#include "rk_vaapi_int.h"
#include <stdlib.h>
#include <string.h>

VAStatus rk_CreateConfig(
    VADriverContextP ctx,
    VAProfile profile,
    VAEntrypoint entrypoint,
    VAConfigAttrib *attrib_list,
    int num_attribs,
    VAConfigID *config_id
) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    pthread_mutex_lock(&drv->mutex);

    for (int i = 0; i < MAX_CONFIGS; i++) {
        if (!drv->configs[i].in_use) {
            drv->configs[i].in_use = 1;
            drv->configs[i].id = ++drv->next_config_id;
            drv->configs[i].profile = profile;
            drv->configs[i].entrypoint = entrypoint;
            *config_id = drv->configs[i].id;
            pthread_mutex_unlock(&drv->mutex);
            return VA_STATUS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&drv->mutex);
    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
}

VAStatus rk_DestroyConfig(VADriverContextP ctx, VAConfigID config_id) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    pthread_mutex_lock(&drv->mutex);
    for (int i = 0; i < MAX_CONFIGS; i++) {
        if (drv->configs[i].in_use && drv->configs[i].id == config_id) {
            drv->configs[i].in_use = 0;
            break;
        }
    }
    pthread_mutex_unlock(&drv->mutex);
    return VA_STATUS_SUCCESS;
}

VAStatus rk_CreateContext(
    VADriverContextP ctx,
    VAConfigID config_id,
    int picture_width,
    int picture_height,
    int flag,
    VASurfaceID *render_targets,
    int num_render_targets,
    VAContextID *context_id
) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    rk_va_config *cfg = NULL;

    pthread_mutex_lock(&drv->mutex);
    for (int i = 0; i < MAX_CONFIGS; i++) {
        if (drv->configs[i].in_use && drv->configs[i].id == config_id) {
            cfg = &drv->configs[i];
            break;
        }
    }
    
    if (!cfg) {
        pthread_mutex_unlock(&drv->mutex);
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }

    for (int i = 0; i < MAX_CONTEXTS; i++) {
        if (!drv->contexts[i].in_use) {
            drv->contexts[i].in_use = 1;
            drv->contexts[i].id = ++drv->next_context_id;
            drv->contexts[i].is_enc = (cfg->entrypoint == VAEntrypointEncSlice);
            
            if (drv->contexts[i].is_enc) {
                RK_EncConfig rk_cfg = {0};
                rk_cfg.width = picture_width;
                rk_cfg.height = picture_height;
                rk_cfg.bit_rate = 1000000; // Default
                rk_cfg.fps_num = 30;
                rk_cfg.fps_den = 1;
                rk_cfg.gop_size = 30;
                rk_cfg.rc_mode = 1; // VBR
                
                RK_CodingType coding = RK_CODING_AVC;
                if (cfg->profile == VAProfileHEVCMain) coding = RK_CODING_HEVC;
                
                drv->contexts[i].enc = rk_encoder_create(coding, &rk_cfg);
            } else {
                RK_CodingType coding = RK_CODING_AVC;
                if (cfg->profile == VAProfileHEVCMain) coding = RK_CODING_HEVC;
                drv->contexts[i].dec = rk_decoder_create(coding);
            }
            
            *context_id = drv->contexts[i].id;
            pthread_mutex_unlock(&drv->mutex);
            return VA_STATUS_SUCCESS;
        }
    }

    pthread_mutex_unlock(&drv->mutex);
    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
}

VAStatus rk_DestroyContext(VADriverContextP ctx, VAContextID context_id) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    pthread_mutex_lock(&drv->mutex);
    for (int i = 0; i < MAX_CONTEXTS; i++) {
        if (drv->contexts[i].in_use && drv->contexts[i].id == context_id) {
            if (drv->contexts[i].is_enc && drv->contexts[i].enc) {
                rk_encoder_destroy(drv->contexts[i].enc);
            } else if (!drv->contexts[i].is_enc && drv->contexts[i].dec) {
                rk_decoder_destroy(drv->contexts[i].dec);
            }
            drv->contexts[i].in_use = 0;
            break;
        }
    }
    pthread_mutex_unlock(&drv->mutex);
    return VA_STATUS_SUCCESS;
}
