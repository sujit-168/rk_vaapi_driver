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

## Prerequisites & Dependencies

To build and use this driver, the following components are required:

- **libva (>= 2.x)**: Standard Video Acceleration API library.
- **rk_hw_base**: Our intermediate middleware library. 
  ```bash
  mkdir -p ~/ffmpeg && cd ~/ffmpeg
  git clone https://github.com/sujit-168/rk_hw_base.git
  cd rk_hw_base && make
  ```
- **librockchip_mpp**: Rockchip Media Process Platform.
- **librga**: Rockchip Graphics 2D Accelerator.
- **Standard Linux Build Tools**: `gcc`, `make`, `pkg-config`.

## Build Instructions

```bash
cd rk_vaapi_driver
make clean && make
```
This produces `lib/rockchip_drv_video.so`.

## Usage & Configuration

```bash
# Set driver name and path
export LIBVA_DRIVER_NAME=rockchip
export LIBVA_DRIVERS_PATH=/path/to/rk_vaapi_driver/lib

# Ensure the middleware and rk libraries are reachable
export LD_LIBRARY_PATH=/path/to/rk_hw_base/lib:$LD_LIBRARY_PATH
```

### Verification
```bash
vainfo
```

### FFmpeg Example (Hardware Encoding)
```bash
ffmpeg -vaapi_device /dev/dri/renderD128 \
       -f lavfi -i testsrc=size=1920x1080:rate=30 \
       -vf 'format=nv12,hwupload' \
       -c:v h264_vaapi \
       output.mp4
```

## Acknowledgments

This project stands on the shoulders of giants. Special thanks to:

- **[Intel VA-API Library](https://github.com/intel/libva)**: For defining the standard that makes cross-platform hardware acceleration possible.
- **[Mesa 3D Project](https://www.mesa3d.org/)**: For their excellent reference implementations of VA-API state trackers.
- **[ffmpeg-rockchip](https://github.com/nyanmisaka/ffmpeg-rockchip)**: For providing high-performance implementation patterns for Rockchip hardware.
- **[rk_hw_base](https://github.com/user/rk_hw_base)**: The foundational middleware layer that powers this driver.

## License

MIT
