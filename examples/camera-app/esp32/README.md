# Matter ESP32 Lighting Example

This example demonstrates the Matter Lighting application on ESP platforms.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/platforms/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/platforms/esp32/build_app_and_commission.md)
guides to get started.

### Building example for Target ESP32-C6

-   Export KVS_SDK_PATH

    ```
    export KVS_SDK_PATH=/path/to/connectedhomeip/third_party/amazon-kinesis-video-streams-webrtc-sdk-c/repo
    ```

-   To build for Matter Over Wi-Fi

    ```
    idf.py set-target esp32c6 build
    ```
