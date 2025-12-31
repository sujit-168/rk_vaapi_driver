# Changelog - Rockchip VA-API Driver

All notable changes and fixes during the development of the `rockchip` VA-API driver.

## [v1.0.0] - 2026-01-01

### Added
- **vaCreateSurfaces2 Support**: Implemented the modern surface creation interface to support FFmpeg 8.0+ attribute negotiation.
- **Image Derivation**: Implemented `vaDeriveImage` and `vaMapBuffer` to enable zero-copy data upload via FFmpeg's `hwupload` filter.
- **Fallback Coded Buffer Detection**: Added logic to automatically locate the bitstream output buffer if not explicitly provided in `vaRenderPicture`, preventing `invalid VAContextID` errors.
- **Extended Rate Control**: Added support for `VA_RC_CQP` (Constant Quality) mode to satisfy FFmpeg defaults.
- **Memory Type Export**: Corrected `vaQuerySurfaceAttributes` to advertise `VA_SURFACE_ATTRIB_MEM_TYPE_VA` and `DRM_PRIME` support.

### Fixed
- **Compilation Errors**:
    - Resolved implicit declaration of `rk_packet_free` in middleware.
    - Fixed `rk_QueryConfigAttributes` signature mismatch with `libva 1.7.0` spec.
    - Corrected non-existent `num_planes` member usage in `VAImageFormat`.
    - Restored missing header includes (`stdio.h`, `va_drmcommon.h`) in `surface.c`.
    - Replaced deprecated/undeclared `VA_STATUS_ERROR_BUFFER_OVERFLOW` with standard `VA_STATUS_ERROR_NOT_ENOUGH_BUFFER`.
- **Logic Errors**:
    - Fixed "Attribute Not Supported" error by properly flagging `VASurfaceAttrib` bitmasks.
    - Fixed filter link failures by properly advertising `NV12` support constraints.
- **Stability**:
    - Fixed potential memory leak warnings in MPP by improving buffer group deinitialization.
    - Removed verbose debug logging for production readiness.

### Infrastructure
- Created comprehensive `README.md` and `Makefile`.
- Established `rk_vaapi_int.h` for internal state tracking of surfaces, contexts, and images.
