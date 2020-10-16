## Source Code

### Directory Structure

The CHIP src directory is structured as follows:

| File / Folder | Contents                                           |
| ------------- | -------------------------------------------------- |
| app           | Application Layer -- Zigbee Cluster Library (ZCL)  |
| ble           | BLE Layer -- Bluetooth Transport Protocol (BTP)    |
| controller    | Controller API                                     |
| crypto        | Cryptography libraries                             |
| darwin        | Darwin Framework (iOS and macOS)                   |
| include       | Public headers                                     |
| inet          | Network Layer -- TCP and UDP endpoints             |
| lib           | Core and Support libraries                         |
| lwip          | Lightweight IP adaptation (to third_party library) |
| platform      | Device Layer -- platform portability adaptations   |
| qrcodetool    | QR code tool                                       |
| setup_payload | QR code setup data encode / decode library         |
| system        | System Layer -- common APIs for mem, work, etc.    |
| test_driver   | Framework for on-device testing                    |
