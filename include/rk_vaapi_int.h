#ifndef RK_VAAPI_INT_H
#define RK_VAAPI_INT_H

#include <va/va_backend.h>
#include "rk_hw_base.h"
#include <pthread.h>

#define MAX_SURFACES  128
#define MAX_CONTEXTS  16
#define MAX_CONFIGS   16

#define MAX_BUFFERS   256
#define MAX_IMAGES    128

typedef struct {
    VAImageID id;
    VAImage image;
    VABufferID buf_id;
    int in_use;
} rk_va_image;

typedef struct {
    VABufferID id;
    VABufferType type;
    void *data;
    uint32_t size;
    int in_use;
    VASurfaceID derived_surface; /* If this buffer is derived from a surface */
} rk_va_buffer;

typedef struct {
    VASurfaceID id;
    RK_Buffer buffer;
    int in_use;
} rk_va_surface;

typedef struct {
    VAContextID id;
    RK_EncCtx enc;
    RK_DecCtx dec;
    VASurfaceID current_render_target;
    VABufferID active_coded_buffer;
    int is_enc;
    int in_use;
} rk_va_context;

typedef struct {
    VAConfigID id;
    VAProfile profile;
    VAEntrypoint entrypoint;
    int in_use;
} rk_va_config;

typedef struct {
    int drm_fd;
    pthread_mutex_t mutex;
    
    rk_va_surface surfaces[MAX_SURFACES];
    rk_va_context contexts[MAX_CONTEXTS];
    rk_va_config  configs[MAX_CONFIGS];
    rk_va_buffer  buffers[MAX_BUFFERS];
    rk_va_image   images[MAX_IMAGES];
    
    uint32_t next_surface_id;
    uint32_t next_context_id;
    uint32_t next_config_id;
    uint32_t next_buffer_id;
    uint32_t next_image_id;
} rk_va_driver_data;

#endif
