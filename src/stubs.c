#include <va/va_backend.h>

VAStatus rk_stub_unimplemented(VADriverContextP ctx) {
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus rk_QueryConfigAttributes(VADriverContextP ctx, VAConfigID config_id, VAProfile *profile, VAEntrypoint *entrypoint, VAConfigAttrib *attrib_list, int *num_attribs) {
    if (!attrib_list) {
        *num_attribs = 1;
        return VA_STATUS_SUCCESS;
    }
    attrib_list[0].type = VAConfigAttribRTFormat;
    attrib_list[0].value = VA_RT_FORMAT_YUV420;
    *num_attribs = 1;
    return VA_STATUS_SUCCESS;
}

VAStatus rk_GetConfigAttributes(VADriverContextP ctx, VAProfile profile, VAEntrypoint entrypoint, VAConfigAttrib *attrib_list, int num_attribs) {
    for (int i = 0; i < num_attribs; i++) {
        switch (attrib_list[i].type) {
            case VAConfigAttribRTFormat:
                attrib_list[i].value = VA_RT_FORMAT_YUV420;
                break;
            case VAConfigAttribRateControl:
                attrib_list[i].value = VA_RC_VBR | VA_RC_CBR | VA_RC_CQP;
                break;
            case VAConfigAttribEncPackedHeaders:
                attrib_list[i].value = VA_ENC_PACKED_HEADER_NONE;
                break;
            case VAConfigAttribEncMaxRefFrames:
                attrib_list[i].value = 1;
                break;
            default:
                /* For optional attributes, just don't touch them or set to 0 */
                break;
        }
    }
    return VA_STATUS_SUCCESS;
}

VAStatus rk_SyncSurface(VADriverContextP ctx, VASurfaceID render_target) { return VA_STATUS_SUCCESS; }
VAStatus rk_QuerySurfaceStatus(VADriverContextP ctx, VASurfaceID render_target, VASurfaceStatus *status) {
    *status = VASurfaceReady;
    return VA_STATUS_SUCCESS;
}

VAStatus rk_PutSurface(VADriverContextP ctx, VASurfaceID surface, void *draw, short srcx, short srcy, unsigned short srcw, unsigned short srch, short destx, short desty, unsigned short destw, unsigned short desth, VARectangle *cliprects, unsigned int number_cliprects, unsigned int flags) {
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus rk_QueryImageFormats(VADriverContextP ctx, VAImageFormat *format_list, int *num_formats) {
    if (!format_list) {
        *num_formats = 1;
        return VA_STATUS_SUCCESS;
    }
    format_list[0].fourcc = VA_FOURCC_NV12;
    format_list[0].bits_per_pixel = 12;
    *num_formats = 1;
    return VA_STATUS_SUCCESS;
}

VAStatus rk_CreateImage(VADriverContextP ctx, VAImageFormat *format, int width, int height, VAImage *image) { return VA_STATUS_ERROR_UNIMPLEMENTED; }
VAStatus rk_SetImagePalette(VADriverContextP ctx, VAImageID image, unsigned char *palette) { return VA_STATUS_ERROR_UNIMPLEMENTED; }
VAStatus rk_GetImage(VADriverContextP ctx, VASurfaceID surface, int x, int y, unsigned int width, unsigned int height, VAImageID image) { return VA_STATUS_ERROR_UNIMPLEMENTED; }
VAStatus rk_PutImage(VADriverContextP ctx, VASurfaceID surface, VAImageID image, int src_x, int src_y, unsigned int src_width, unsigned int src_height, int dest_x, int dest_y, unsigned int dest_width, unsigned int dest_height) { return VA_STATUS_ERROR_UNIMPLEMENTED; }

VAStatus rk_QuerySubpictureFormats(VADriverContextP ctx, VAImageFormat *format_list, unsigned int *flags, unsigned int *num_formats) {
    *num_formats = 0;
    return VA_STATUS_SUCCESS;
}

VAStatus rk_CreateSubpicture(VADriverContextP ctx, VAImageID image, VASubpictureID *subpicture) { return VA_STATUS_ERROR_UNIMPLEMENTED; }
VAStatus rk_DestroySubpicture(VADriverContextP ctx, VASubpictureID subpicture) { return VA_STATUS_SUCCESS; }
VAStatus rk_SetSubpictureImage(VADriverContextP ctx, VASubpictureID subpicture, VAImageID image) { return VA_STATUS_ERROR_UNIMPLEMENTED; }
VAStatus rk_SetSubpictureChromakey(VADriverContextP ctx, VASubpictureID subpicture, unsigned int chromakey_min, unsigned int chromakey_max, unsigned int chromakey_mask) { return VA_STATUS_ERROR_UNIMPLEMENTED; }
VAStatus rk_SetSubpictureGlobalAlpha(VADriverContextP ctx, VASubpictureID subpicture, float global_alpha) { return VA_STATUS_ERROR_UNIMPLEMENTED; }
VAStatus rk_AssociateSubpicture(VADriverContextP ctx, VASubpictureID subpicture, VASurfaceID *target_surfaces, int num_surfaces, short src_x, short src_y, unsigned short src_width, unsigned short src_height, short dest_x, short dest_y, unsigned short dest_width, unsigned short dest_height, unsigned int flags) { return VA_STATUS_ERROR_UNIMPLEMENTED; }
VAStatus rk_DeassociateSubpicture(VADriverContextP ctx, VASubpictureID subpicture, VASurfaceID *target_surfaces, int num_surfaces) { return VA_STATUS_SUCCESS; }

VAStatus rk_QueryDisplayAttributes(VADriverContextP ctx, VADisplayAttribute *attr_list, int *num_attributes) {
    *num_attributes = 0;
    return VA_STATUS_SUCCESS;
}

VAStatus rk_GetDisplayAttributes(VADriverContextP ctx, VADisplayAttribute *attr_list, int num_attributes) { return VA_STATUS_SUCCESS; }
VAStatus rk_SetDisplayAttributes(VADriverContextP ctx, VADisplayAttribute *attr_list, int num_attributes) { return VA_STATUS_SUCCESS; }
