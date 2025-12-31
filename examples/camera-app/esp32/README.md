# Matter ESP32 Camera Example

This example demonstrates the Matter Camera application on ESP platforms.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/platforms/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/platforms/esp32/build_app_and_commission.md)
guides to get started.

# Split Mode Camera Example

This pair of examples demonstrates a **two-device split architecture** for ESP32 Camera, where signaling and media streaming are separated across two processors for optimal power efficiency.

## Architecture Overview

The split mode consists of two separate firmware images:

### 1. **esp32_camera** (ESP32-C6)
- **Role**: Matter camera with KVS signaling integration
- **Responsibilities**:
  - AWS KVS WebRTC signaling
  - Bridge communication with media adapter
  - Always-on connectivity for instant responsiveness

### 2. **media_adapter** (ESP32-P4)
- **Role**: Media streaming device
- **Implementation**: Uses the `streaming_only` example from `${KVS_SDK_PATH}/esp_port/examples/streaming_only`
- **Responsibilities**:
  - Video/audio capture and encoding
  - WebRTC media streaming
  - Power-optimized operation (sleeps when not streaming)
  - Receives signaling commands via bridge from esp32_camera

## Hardware Requirements

- **ESP32-P4 Function EV Board** (required)
  - Contains both ESP32-P4 and ESP32-C6 processors
  - Built-in camera support
  - SDIO communication between processors

## System Architecture

```
┌─────────────────┐      SDIO Bridge     ┌─────────────────┐
│    ESP32-C6     │◄────────────────────►│    ESP32-P4     │
│  (esp32_camera) │      Communication   │ (media_adapter) │
│                 │                      │                 │
│ ┌─────────────┐ │                      │ ┌─────────────┐ │
│ │ Matter      │ │                      │ │ H.264       │ │
│ │             │ │                      │ │ Encoder     │ │
│ │             │ │                      │ │             │ │
│ │ AWS KVS     │ │                      │ │ Camera      │ │
│ │ Signaling   │ │                      │ │ Interface   │ │
│ └─────────────┘ │                      │ └─────────────┘ │
└─────────────────┘                      └─────────────────┘
        ▲                                        ▲
        │                                        │
        ▼                                        ▼
   (Signaling)                              Video/Audio
                                             Hardware
```

## Quick Start

### Prerequisites

- IDF version: v5.4.1
- [ESP32-P4 Function EV Board](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32p4/esp32-p4-function-ev-board/user_guide.html)
- Amazon Kinesis Video Streams WebRTC SDK repository:

  ```
    export KVS_SDK_PATH=/path/to/connectedhomeip/third_party/amazon-kinesis-video-streams-webrtc-sdk-c/repo
  ```

### Build and Flash Instructions

⚠️ **Important**: This requires **TWO separate firmware flashes** on the same ESP32-P4 Function EV Board.

#### Step 1: Flash esp32_camera (ESP32-C6)

This handles AWS KVS signaling and Matter integration.

```bash
idf.py set-target esp32c6
idf.py build
idf.py -p [PORT] flash monitor
```

**Note**: ESP32-C6 does not have an onboard UART port. You will need to use ESP-Prog or any other JTAG.

#### Step 2: Flash media_adapter (ESP32-P4)

This handles video/audio streaming. The firmware is the `streaming_only` example from the KVS SDK.

```bash
cd ${KVS_SDK_PATH}/esp_port/examples/streaming_only
idf.py set-target esp32p4
idf.py menuconfig
# Go to Component config -> ESP System Settings -> Channel for console output
# (X) USB Serial/JTAG Controller # For ESP32-P4 Function_EV_Board V1.2 OR V1.5
# (X) Default: UART0 # For ESP32-P4 Function_EV_Board V1.4
idf.py build
idf.py -p [PORT] flash monitor
```

**Note**: If the console selection is wrong, you will only see the initial bootloader logs. Please change the console as instructed above and reflash the app to see the complete logs.

⚠️ **Caution**: Currently, due to flash size limitations of ESP32-C6 onboard the ESP32-P4 Function EV Board, the `ota_1` partition (see [`partitions.csv`](partitions.csv)) is disabled and the size of the `ota_0` partition is increased. This prevents the firmware from performing OTA updates. Hence, this configuration is not recommended for production use.