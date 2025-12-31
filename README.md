# Rockchip VA-API Driver (rk_vaapi_driver)

A hardware-accelerated Video Acceleration API (VA-API) driver for Rockchip SoCs, acting as a standard-compliant wrapper for the Rockchip Media Process Platform (MPP).

## Overview

This driver enables **zero-modification** integration of Rockchip hardware video acceleration with standard Linux multimedia tools. By implementing the VA-API backend, it allows official, unmodified builds of applications like `FFmpeg`, `VLC`, and `GStreamer` to utilize the Rockchip VPU for encoding and decoding.

### Key Features
- **Standard-Compliant**: Implements the Linux VA-API 1.x interface.
- **Middleware Driven**: Leverages the `rk_hw_base` middleware for robust MPP/RGA interaction.
- **Zero-Copy Architecture**: Uses hardware-backed buffers for efficient frame management.
- **Universal Compatibility**: Works with official FFmpeg binaries without custom patches.

## Supported Profiles & Entrypoints

Verified via `vainfo`:
- **H.264 (AVC)**: Baseline, Main, High (Dec/Enc)
- **H.265 (HEVC)**: Main (Dec/Enc)

## Prerequisites

- **FFmpeg**: Official build (e.g., from `apt`)
- **libva-dev**: `sudo apt install libva-dev vainfo`
- **rk_hw_base**: Our intermediate middleware library.
- **Rockchip Libraries**: `librockchip_mpp` and `librga`.

## Build Instructions

```bash
cd rk_vaapi_driver
make clean && make
```
This produces `lib/rockchip_drv_video.so`.

## Usage & Configuration

To use the driver, you must tell `libva` where to find it and where to find the supporting middleware.

```bash
# Set driver name and path
export LIBVA_DRIVER_NAME=rockchip
export LIBVA_DRIVERS_PATH=/path/to/rk_vaapi_driver/lib

# Ensure the middleware library is reachable
export LD_LIBRARY_PATH=/path/to/rk_hw_base/lib:$LD_LIBRARY_PATH
```

### Verification
Check if the driver is correctly loaded:
```bash
vainfo
```

### FFmpeg Example (Hardware Encoding)
```bash
ffmpeg -vaapi_device /dev/dri/renderD128 \
       -i input.mp4 \
       -vf 'format=nv12,hwupload' \
       -c:v h264_vaapi \
       output_vaapi.mp4
```

## Current Status (Development Phase)

- [x] Phase 1: Driver Skeleton and `__vaDriverInit_1_0` entry point.
- [x] Phase 2: Surface and Hardware Buffer mapping.
- [x] Phase 3: Context and VTable wiring.
- [x] Phase 4: Data Path Integration (Bitstream retrieval).
- [ ] Phase 5: Advanced Parameter Parsing (SPS/PPS, Bitrate control mapping).

## License
MIT
