#include "rk_vaapi_int.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Forward declarations for surface.c */
VAStatus rk_CreateSurfaces(VADriverContextP ctx, int width, int height, int format, int num_surfaces, VASurfaceID *surfaces);
VAStatus rk_CreateSurfaces2(VADriverContextP ctx, unsigned int format, unsigned int width, unsigned int height, VASurfaceID *surfaces, unsigned int num_surfaces, VASurfaceAttrib *attrib_list, unsigned int num_attribs);
VAStatus rk_DestroySurfaces(VADriverContextP ctx, VASurfaceID *surface_list, int num_surfaces);
VAStatus rk_QuerySurfaceAttributes(VADriverContextP ctx, VAConfigID config_id, VASurfaceAttrib *attrib_list, unsigned int *num_attribs);
VAStatus rk_DeriveImage(VADriverContextP ctx, VASurfaceID surface, VAImage *image);
VAStatus rk_DestroyImage(VADriverContextP ctx, VAImageID image);

/* Forward declarations for context.c */
VAStatus rk_CreateConfig(VADriverContextP ctx, VAProfile profile, VAEntrypoint entrypoint, VAConfigAttrib *attrib_list, int num_attribs, VAConfigID *config_id);
VAStatus rk_DestroyConfig(VADriverContextP ctx, VAConfigID config_id);
VAStatus rk_CreateContext(VADriverContextP ctx, VAConfigID config_id, int picture_width, int picture_height, int flag, VASurfaceID *render_targets, int num_render_targets, VAContextID *context_id);
VAStatus rk_DestroyContext(VADriverContextP ctx, VAContextID context_id);

/* Forward declarations for buffer.c */
VAStatus rk_CreateBuffer(VADriverContextP ctx, VAContextID context_id, VABufferType type, unsigned int size, unsigned int num_elements, void *data, VABufferID *buf_id);
VAStatus rk_DestroyBuffer(VADriverContextP ctx, VABufferID buf_id);
VAStatus rk_MapBuffer(VADriverContextP ctx, VABufferID buf_id, void **pbuf);
VAStatus rk_UnmapBuffer(VADriverContextP ctx, VABufferID buf_id);
VAStatus rk_BufferSetNumElements(VADriverContextP ctx, VABufferID buf_id, unsigned int num_elements);

/* Forward declarations for encoder.c */
VAStatus rk_BeginPicture(VADriverContextP ctx, VAContextID context_id, VASurfaceID render_target);
VAStatus rk_RenderPicture(VADriverContextP ctx, VAContextID context_id, VABufferID *buffers, int num_buffers);
VAStatus rk_EndPicture(VADriverContextP ctx, VAContextID context_id);

/* Forward declarations for stubs.c */
VAStatus rk_QueryConfigAttributes(VADriverContextP ctx, VAConfigID config_id, VAProfile *profile, VAEntrypoint *entrypoint, VAConfigAttrib *attrib_list, int *num_attribs);
VAStatus rk_GetConfigAttributes(VADriverContextP ctx, VAProfile profile, VAEntrypoint entrypoint, VAConfigAttrib *attrib_list, int num_attribs);
VAStatus rk_SyncSurface(VADriverContextP ctx, VASurfaceID render_target);
VAStatus rk_QuerySurfaceStatus(VADriverContextP ctx, VASurfaceID render_target, VASurfaceStatus *status);
VAStatus rk_PutSurface(VADriverContextP ctx, VASurfaceID surface, void *draw, short srcx, short srcy, unsigned short srcw, unsigned short srch, short destx, short desty, unsigned short destw, unsigned short desth, VARectangle *cliprects, unsigned int number_cliprects, unsigned int flags);
VAStatus rk_QueryImageFormats(VADriverContextP ctx, VAImageFormat *format_list, int *num_formats);
VAStatus rk_CreateImage(VADriverContextP ctx, VAImageFormat *format, int width, int height, VAImage *image);
VAStatus rk_SetImagePalette(VADriverContextP ctx, VAImageID image, unsigned char *palette);
VAStatus rk_GetImage(VADriverContextP ctx, VASurfaceID surface, int x, int y, unsigned int width, unsigned int height, VAImageID image);
VAStatus rk_PutImage(VADriverContextP ctx, VASurfaceID surface, VAImageID image, int src_x, int src_y, unsigned int src_width, unsigned int src_height, int dest_x, int dest_y, unsigned int dest_width, unsigned int dest_height);
VAStatus rk_QuerySubpictureFormats(VADriverContextP ctx, VAImageFormat *format_list, unsigned int *flags, unsigned int *num_formats);
VAStatus rk_CreateSubpicture(VADriverContextP ctx, VAImageID image, VASubpictureID *subpicture);
VAStatus rk_DestroySubpicture(VADriverContextP ctx, VASubpictureID subpicture);
VAStatus rk_SetSubpictureImage(VADriverContextP ctx, VASubpictureID subpicture, VAImageID image);
VAStatus rk_SetSubpictureChromakey(VADriverContextP ctx, VASubpictureID subpicture, unsigned int chromakey_min, unsigned int chromakey_max, unsigned int chromakey_mask);
VAStatus rk_SetSubpictureGlobalAlpha(VADriverContextP ctx, VASubpictureID subpicture, float global_alpha);
VAStatus rk_AssociateSubpicture(VADriverContextP ctx, VASubpictureID subpicture, VASurfaceID *target_surfaces, int num_surfaces, short src_x, short src_y, unsigned short src_width, unsigned short src_height, short dest_x, short dest_y, unsigned short dest_width, unsigned short dest_height, unsigned int flags);
VAStatus rk_DeassociateSubpicture(VADriverContextP ctx, VASubpictureID subpicture, VASurfaceID *target_surfaces, int num_surfaces);
VAStatus rk_QueryDisplayAttributes(VADriverContextP ctx, VADisplayAttribute *attr_list, int *num_attributes);
VAStatus rk_GetDisplayAttributes(VADriverContextP ctx, VADisplayAttribute *attr_list, int num_attributes);
VAStatus rk_SetDisplayAttributes(VADriverContextP ctx, VADisplayAttribute *attr_list, int num_attributes);

static VAStatus rk_Terminate(VADriverContextP ctx) {
    rk_va_driver_data *drv = (rk_va_driver_data *)ctx->pDriverData;
    pthread_mutex_destroy(&drv->mutex);
    free(drv);
    return VA_STATUS_SUCCESS;
}

static VAStatus rk_QueryConfigProfiles(VADriverContextP ctx, VAProfile *profile_list, int *num_profiles) {
    profile_list[0] = VAProfileH264Baseline;
    profile_list[1] = VAProfileH264Main;
    profile_list[2] = VAProfileH264High;
    profile_list[3] = VAProfileHEVCMain;
    *num_profiles = 4;
    return VA_STATUS_SUCCESS;
}

static VAStatus rk_QueryConfigEntrypoints(VADriverContextP ctx, VAProfile profile, VAEntrypoint *entrypoint_list, int *num_entrypoints) {
    entrypoint_list[0] = VAEntrypointVLD;      /* Decode */
    entrypoint_list[1] = VAEntrypointEncSlice; /* Encode */
    *num_entrypoints = 2;
    return VA_STATUS_SUCCESS;
}

/* Entry point called by libva 1.x (e.g. 1.7.0) */
VAStatus __vaDriverInit_1_0(VADriverContextP ctx) {
    ctx->version_major = VA_MAJOR_VERSION;
    ctx->version_minor = VA_MINOR_VERSION;
    ctx->max_profiles = MAX_CONFIGS;
    ctx->max_entrypoints = 5;
    ctx->max_attributes = 10;
    ctx->max_image_formats = 10;
    ctx->max_subpic_formats = 1;
    ctx->max_display_attributes = 10;
    ctx->str_vendor = "Rockchip MPP Wrapper (rk_hw_base)";

    rk_va_driver_data *drv = calloc(1, sizeof(rk_va_driver_data));
    if (!drv) return VA_STATUS_ERROR_ALLOCATION_FAILED;

    pthread_mutex_init(&drv->mutex, NULL);
    drv->next_surface_id = 0x100;
    drv->next_context_id = 0x200;
    drv->next_config_id  = 0x300;
    drv->next_buffer_id  = 0x400;
    drv->next_image_id   = 0x500;

    ctx->pDriverData = drv;
    
    /* Populate VTable */
    struct VADriverVTable *vtable = ctx->vtable;
    vtable->vaTerminate = rk_Terminate;
    vtable->vaQueryConfigProfiles = rk_QueryConfigProfiles;
    vtable->vaQueryConfigEntrypoints = rk_QueryConfigEntrypoints;
    vtable->vaQuerySurfaceAttributes = rk_QuerySurfaceAttributes;
    vtable->vaCreateSurfaces = rk_CreateSurfaces;
    vtable->vaCreateSurfaces2 = rk_CreateSurfaces2;
    vtable->vaDestroySurfaces = rk_DestroySurfaces;
    vtable->vaCreateConfig = rk_CreateConfig;
    vtable->vaDestroyConfig = rk_DestroyConfig;
    vtable->vaCreateContext = rk_CreateContext;
    vtable->vaDestroyContext = rk_DestroyContext;
    vtable->vaBeginPicture = rk_BeginPicture;
    vtable->vaRenderPicture = rk_RenderPicture;
    vtable->vaEndPicture = rk_EndPicture;
    vtable->vaDeriveImage = rk_DeriveImage;
    vtable->vaDestroyImage = rk_DestroyImage;

    /* Wire up stubs */
    vtable->vaQueryConfigAttributes = rk_QueryConfigAttributes;
    vtable->vaGetConfigAttributes = rk_GetConfigAttributes;
    vtable->vaCreateBuffer = rk_CreateBuffer;
    vtable->vaBufferSetNumElements = rk_BufferSetNumElements;
    vtable->vaMapBuffer = rk_MapBuffer;
    vtable->vaUnmapBuffer = rk_UnmapBuffer;
    vtable->vaDestroyBuffer = rk_DestroyBuffer;
    vtable->vaSyncSurface = rk_SyncSurface;
    vtable->vaQuerySurfaceStatus = rk_QuerySurfaceStatus;
    vtable->vaPutSurface = rk_PutSurface;
    vtable->vaQueryImageFormats = rk_QueryImageFormats;
    vtable->vaCreateImage = rk_CreateImage;
    vtable->vaDeriveImage = rk_DeriveImage;
    vtable->vaDestroyImage = rk_DestroyImage;
    vtable->vaSetImagePalette = rk_SetImagePalette;
    vtable->vaGetImage = rk_GetImage;
    vtable->vaPutImage = rk_PutImage;
    vtable->vaQuerySubpictureFormats = rk_QuerySubpictureFormats;
    vtable->vaCreateSubpicture = rk_CreateSubpicture;
    vtable->vaDestroySubpicture = rk_DestroySubpicture;
    vtable->vaSetSubpictureImage = rk_SetSubpictureImage;
    vtable->vaSetSubpictureChromakey = rk_SetSubpictureChromakey;
    vtable->vaSetSubpictureGlobalAlpha = rk_SetSubpictureGlobalAlpha;
    vtable->vaAssociateSubpicture = rk_AssociateSubpicture;
    vtable->vaDeassociateSubpicture = rk_DeassociateSubpicture;
    vtable->vaQueryDisplayAttributes = rk_QueryDisplayAttributes;
    vtable->vaGetDisplayAttributes = rk_GetDisplayAttributes;
    vtable->vaSetDisplayAttributes = rk_SetDisplayAttributes;

    printf("Rockchip VA-API Driver Initialized (with stubs)\n");

    return VA_STATUS_SUCCESS;
}
