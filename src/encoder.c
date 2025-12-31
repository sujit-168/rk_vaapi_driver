#include "rk_vaapi_int.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

VAStatus rk_BeginPicture(
    VADriverContextP ctx,
    VAContextID context_id,
    VASurfaceID render_target
) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    pthread_mutex_lock(&drv->mutex);
    for (int i = 0; i < MAX_CONTEXTS; i++) {
        if (drv->contexts[i].in_use && drv->contexts[i].id == context_id) {
            drv->contexts[i].current_render_target = render_target;
            pthread_mutex_unlock(&drv->mutex);
            return VA_STATUS_SUCCESS;
        }
    }
    pthread_mutex_unlock(&drv->mutex);
    return VA_STATUS_ERROR_INVALID_CONTEXT;
}

VAStatus rk_RenderPicture(
    VADriverContextP ctx,
    VAContextID context_id,
    VABufferID *buffers,
    int num_buffers
) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    rk_va_context *context = NULL;

    pthread_mutex_lock(&drv->mutex);
    for (int i = 0; i < MAX_CONTEXTS; i++) {
        if (drv->contexts[i].in_use && drv->contexts[i].id == context_id) {
            context = &drv->contexts[i];
            break;
        }
    }

    if (!context) {
        pthread_mutex_unlock(&drv->mutex);
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    /* Look for coded buffer and parameter buffers */
    for (int b = 0; b < num_buffers; b++) {
        for (int i = 0; i < MAX_BUFFERS; i++) {
            if (drv->buffers[i].in_use && drv->buffers[i].id == buffers[b]) {
                if (drv->buffers[i].type == VAEncCodedBufferType) {
                    context->active_coded_buffer = buffers[b];
                } else if (drv->buffers[i].type == VASliceDataBufferType) {
                    /* For decoder: track the bitstream buffer */
                    context->active_coded_buffer = buffers[b]; 
                }
            }
        }
    }

    pthread_mutex_unlock(&drv->mutex);
    return VA_STATUS_SUCCESS;
}

VAStatus rk_EndPicture(
    VADriverContextP ctx,
    VAContextID context_id
) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    rk_va_context *context = NULL;
    rk_va_surface *surface = NULL;
    rk_va_buffer  *coded_buf = NULL;

    pthread_mutex_lock(&drv->mutex);
    for (int i = 0; i < MAX_CONTEXTS; i++) {
        if (drv->contexts[i].in_use && drv->contexts[i].id == context_id) {
            context = &drv->contexts[i];
            break;
        }
    }
    
    if (context) {
        for (int i = 0; i < MAX_SURFACES; i++) {
            if (drv->surfaces[i].in_use && drv->surfaces[i].id == context->current_render_target) {
                surface = &drv->surfaces[i];
                break;
            }
        }
        /* Try to find the active coded buffer */
        for (int i = 0; i < MAX_BUFFERS; i++) {
            if (drv->buffers[i].in_use && drv->buffers[i].id == context->active_coded_buffer) {
                coded_buf = &drv->buffers[i];
                break;
            }
        }
        /* Fallback: If no active coded buffer set via RenderPicture, find the first one */
        if (!coded_buf) {
            for (int i = 0; i < MAX_BUFFERS; i++) {
                if (drv->buffers[i].in_use && drv->buffers[i].type == VAEncCodedBufferType) {
                    coded_buf = &drv->buffers[i];
                    break;
                }
            }
        }
    }
    int is_enc = context->is_enc;
    void *enc_ctx = context->enc;
    void *dec_ctx = context->dec;

    pthread_mutex_unlock(&drv->mutex);

    if (!context || !surface) {
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    if (is_enc) {
        if (!enc_ctx || !coded_buf) {
            return VA_STATUS_ERROR_INVALID_CONTEXT;
        }

        /* Send frame to encoder */
        RK_Frame frame = {0};
        frame.buffer = surface->buffer;
        if (rk_encoder_send_frame(enc_ctx, &frame) != RK_HW_SUCCESS) {
            return VA_STATUS_ERROR_ENCODING_ERROR;
        }

        /* Receive packet and copy to coded buffer */
        RK_Packet packet = {0};
        if (rk_encoder_receive_packet(enc_ctx, &packet) == RK_HW_SUCCESS) {
            if (packet.size <= coded_buf->size - sizeof(VACodedBufferSegment)) {
                VACodedBufferSegment *seg = (VACodedBufferSegment *)coded_buf->data;
                seg->size = packet.size;
                seg->bit_offset = 0;
                seg->status = 0;
                seg->reserved = 0;
                seg->buf = (void *)((uint8_t *)coded_buf->data + sizeof(VACodedBufferSegment));
                seg->next = NULL;
                
                memcpy(seg->buf, packet.data, packet.size);
                rk_packet_free(&packet);
            } else {
                rk_packet_free(&packet);
                return VA_STATUS_ERROR_NOT_ENOUGH_BUFFER;
            }
        }
    } else {
        /* DECODER PATH */
        if (!dec_ctx || !coded_buf) {
            /* If no slice data this time, it might just be a param update */
            return VA_STATUS_SUCCESS;
        }

        RK_Packet packet = {0};
        packet.data = coded_buf->data;
        packet.size = coded_buf->size;
        
        if (rk_decoder_send_packet(dec_ctx, &packet) == RK_HW_SUCCESS) {
            RK_Frame frame = {0};
            if (rk_decoder_receive_frame(dec_ctx, &frame) == RK_HW_SUCCESS) {
                /* Copy decoded data to surface buffer */
                /* In a perfect world, we'd use DMA-BUF export/import to avoid this copy */
                if (frame.buffer.size <= surface->buffer.size) {
                    memcpy(surface->buffer.data, frame.buffer.data, frame.buffer.size);
                }
                /* Note: frame.buffer is managed by MPP, we don't free it here */
            }
        }
    }
    
    return VA_STATUS_SUCCESS;
}
