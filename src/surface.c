#include "rk_vaapi_int.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <va/va_drmcommon.h>

VAStatus rk_DeriveImage(
    VADriverContextP ctx,
    VASurfaceID surface_id,
    VAImage *image
) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    rk_va_surface *surf = NULL;

    pthread_mutex_lock(&drv->mutex);
    for (int i = 0; i < MAX_SURFACES; i++) {
        if (drv->surfaces[i].in_use && drv->surfaces[i].id == surface_id) {
            surf = &drv->surfaces[i];
            break;
        }
    }

    if (!surf) {
        pthread_mutex_unlock(&drv->mutex);
        return VA_STATUS_ERROR_INVALID_SURFACE;
    }

    /* Create a buffer to represent this surface's data */
    rk_va_buffer *buf = NULL;
    for (int i = 0; i < MAX_BUFFERS; i++) {
        if (!drv->buffers[i].in_use) {
            buf = &drv->buffers[i];
            buf->in_use = 1;
            buf->id = ++drv->next_buffer_id;
            buf->type = VAImageBufferType;
            buf->size = surf->buffer.size;
            buf->data = surf->buffer.data; /* DIRECT POINTER */
            buf->derived_surface = surface_id;
            break;
        }
    }

    if (!buf) {
        pthread_mutex_unlock(&drv->mutex);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    /* Create the Image object */
    rk_va_image *img = NULL;
    for (int i = 0; i < MAX_IMAGES; i++) {
        if (!drv->images[i].in_use) {
            img = &drv->images[i];
            img->in_use = 1;
            img->id = ++drv->next_image_id;
            img->buf_id = buf->id;
            
            img->image.image_id = img->id;
            img->image.buf = buf->id;
            img->image.format.fourcc = VA_FOURCC_NV12;
            img->image.width = surf->buffer.width;
            img->image.height = surf->buffer.height;
            img->image.data_size = surf->buffer.size;
            img->image.num_planes = 2;
            img->image.pitches[0] = surf->buffer.hor_stride;
            img->image.offsets[0] = 0;
            img->image.pitches[1] = surf->buffer.hor_stride;
            img->image.offsets[1] = surf->buffer.hor_stride * surf->buffer.ver_stride;
            
            *image = img->image;
            break;
        }
    }

    pthread_mutex_unlock(&drv->mutex);
    return img ? VA_STATUS_SUCCESS : VA_STATUS_ERROR_ALLOCATION_FAILED;
}

VAStatus rk_DestroyImage(VADriverContextP ctx, VAImageID image) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    pthread_mutex_lock(&drv->mutex);
    for (int i = 0; i < MAX_IMAGES; i++) {
        if (drv->images[i].in_use && drv->images[i].id == image) {
            /* Also destroy the associated buffer if it was derived */
            for (int j = 0; j < MAX_BUFFERS; j++) {
                if (drv->buffers[j].in_use && drv->buffers[j].id == drv->images[i].buf_id) {
                    if (drv->buffers[j].derived_surface != 0) {
                        /* Do NOT free drv->buffers[j].data, it belongs to the surface */
                        drv->buffers[j].in_use = 0;
                    }
                }
            }
            drv->images[i].in_use = 0;
            break;
        }
    }
    pthread_mutex_unlock(&drv->mutex);
    return VA_STATUS_SUCCESS;
}
VAStatus rk_CreateSurfaces(
    VADriverContextP ctx,
    int width,
    int height,
    int format,
    int num_surfaces,
    VASurfaceID *surfaces
) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    pthread_mutex_lock(&drv->mutex);

    int count = 0;
    for (int i = 0; i < MAX_SURFACES && count < num_surfaces; i++) {
        if (!drv->surfaces[i].in_use) {
            RK_PixelFormat rk_fmt = RK_FMT_NV12; /* Default for VA-API */
            if (rk_buffer_alloc(&drv->surfaces[i].buffer, width, height, rk_fmt) == RK_HW_SUCCESS) {
                drv->surfaces[i].in_use = 1;
                drv->surfaces[i].id = ++drv->next_surface_id;
                surfaces[count++] = drv->surfaces[i].id;
            }
        }
    }

    pthread_mutex_unlock(&drv->mutex);

    if (count < num_surfaces) return VA_STATUS_ERROR_ALLOCATION_FAILED;
    return VA_STATUS_SUCCESS;
}

VAStatus rk_CreateSurfaces2(
    VADriverContextP ctx,
    unsigned int format,
    unsigned int width,
    unsigned int height,
    VASurfaceID *surfaces,
    unsigned int num_surfaces,
    VASurfaceAttrib *attrib_list,
    unsigned int num_attribs
) {
    /* For now, just delegate to the standard version and ignore attributes */
    /* Many attributes are advisory (like memory type DMABUF) */
    return rk_CreateSurfaces(ctx, width, height, format, num_surfaces, surfaces);
}

VAStatus rk_DestroySurfaces(
    VADriverContextP ctx,
    VASurfaceID *surface_list,
    int num_surfaces
) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    pthread_mutex_lock(&drv->mutex);

    for (int s = 0; s < num_surfaces; s++) {
        for (int i = 0; i < MAX_SURFACES; i++) {
            if (drv->surfaces[i].in_use && drv->surfaces[i].id == surface_list[s]) {
                rk_buffer_free(&drv->surfaces[i].buffer);
                drv->surfaces[i].in_use = 0;
                break;
            }
        }
    }

    pthread_mutex_unlock(&drv->mutex);
    return VA_STATUS_SUCCESS;
}

VAStatus rk_QuerySurfaceAttributes(
    VADriverContextP ctx,
    VAConfigID config_id,
    VASurfaceAttrib *attrib_list,
    unsigned int *num_attribs
) {
    if (!attrib_list) {
        *num_attribs = 4;
        return VA_STATUS_SUCCESS;
    }

    int i = 0;
    attrib_list[i].type = VASurfaceAttribPixelFormat;
    attrib_list[i].value.type = VAGenericValueTypeInteger;
    attrib_list[i].value.value.i = VA_FOURCC_NV12;
    attrib_list[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
    i++;

    attrib_list[i].type = VASurfaceAttribMemoryType;
    attrib_list[i].value.type = VAGenericValueTypeInteger;
    attrib_list[i].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
    attrib_list[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
    i++;

    attrib_list[i].type = VASurfaceAttribMaxWidth;
    attrib_list[i].value.type = VAGenericValueTypeInteger;
    attrib_list[i].value.value.i = 4096;
    attrib_list[i].flags = VA_SURFACE_ATTRIB_GETTABLE;
    i++;

    attrib_list[i].type = VASurfaceAttribMaxHeight;
    attrib_list[i].value.type = VAGenericValueTypeInteger;
    attrib_list[i].value.value.i = 4096;
    attrib_list[i].flags = VA_SURFACE_ATTRIB_GETTABLE;
    i++;

    *num_attribs = i;
    return VA_STATUS_SUCCESS;
}
