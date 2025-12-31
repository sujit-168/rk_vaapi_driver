#include "rk_vaapi_int.h"
#include <stdlib.h>
#include <string.h>

VAStatus rk_CreateBuffer(
    VADriverContextP ctx,
    VAContextID context_id,
    VABufferType type,
    unsigned int size,
    unsigned int num_elements,
    void *data,
    VABufferID *buf_id
) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    pthread_mutex_lock(&drv->mutex);

    for (int i = 0; i < MAX_BUFFERS; i++) {
        if (!drv->buffers[i].in_use) {
            drv->buffers[i].in_use = 1;
            drv->buffers[i].id = ++drv->next_buffer_id;
            drv->buffers[i].type = type;
            drv->buffers[i].size = size * num_elements;
            drv->buffers[i].data = malloc(drv->buffers[i].size);
            
            if (data) {
                memcpy(drv->buffers[i].data, data, drv->buffers[i].size);
            }
            
            *buf_id = drv->buffers[i].id;
            pthread_mutex_unlock(&drv->mutex);
            return VA_STATUS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&drv->mutex);
    return VA_STATUS_ERROR_ALLOCATION_FAILED;
}

VAStatus rk_DestroyBuffer(VADriverContextP ctx, VABufferID buf_id) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    pthread_mutex_lock(&drv->mutex);
    for (int i = 0; i < MAX_BUFFERS; i++) {
        if (drv->buffers[i].in_use && drv->buffers[i].id == buf_id) {
            if (drv->buffers[i].data) free(drv->buffers[i].data);
            drv->buffers[i].in_use = 0;
            break;
        }
    }
    pthread_mutex_unlock(&drv->mutex);
    return VA_STATUS_SUCCESS;
}

VAStatus rk_MapBuffer(VADriverContextP ctx, VABufferID buf_id, void **pbuf) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    pthread_mutex_lock(&drv->mutex);
    for (int i = 0; i < MAX_BUFFERS; i++) {
        if (drv->buffers[i].in_use && drv->buffers[i].id == buf_id) {
            *pbuf = drv->buffers[i].data;
            pthread_mutex_unlock(&drv->mutex);
            return VA_STATUS_SUCCESS;
        }
    }
    pthread_mutex_unlock(&drv->mutex);
    return VA_STATUS_ERROR_INVALID_BUFFER;
}

VAStatus rk_UnmapBuffer(VADriverContextP ctx, VABufferID buf_id) {
    return VA_STATUS_SUCCESS;
}

VAStatus rk_BufferSetNumElements(VADriverContextP ctx, VABufferID buf_id, unsigned int num_elements) {
    return VA_STATUS_SUCCESS;
}
