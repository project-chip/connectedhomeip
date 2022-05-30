# CHIP controller

There are currently 4 implementations of the CHIP device controller with varying
support for certain CHIP features.

## Implementations

### iOS

The iOS chip-tool is located in [../darwin/CHIPTool](../darwin/CHIPTool).

### Android

The Android chip-tool is located in [../android/CHIPTool](../android/CHIPTool).

### POSIX CLI

The POSIX CLI chip-tool is located in
[../../examples/chip-tool](../../examples/chip-tool).

### Darwin CLI

The POSIX CLI chip-tool is located in
[../../examples/darwin-framework-tool](../../examples/chip-tool).

### Python

The Python chip-device-ctrl is located in
[../controller/python/](../controller/python).

## Feature Overview

| Onboarding | iOS | Android | CLI | Python |
| ---------- | --- | ------- | --- | ------ |
| Setup code | y?  | n       | y   | y      |
| QR code    | y   | y       | n   | n      |
| NFC        | y   | y       | n   | n      |

| Provisioning | iOS    | Android | CLI | Python |
| ------------ | ------ | ------- | --- | ------ |
| Soft-AP      | y      | n       | y   | y      |
| WiFi-BLE     | y?     | y       | y   | y      |
| Thread-BLE   | n(\*0) | y(\*1)  | n   | y(\*1) |

| Commands/Clusters   | iOS | Android | CLI | Python |
| ------------------- | --- | ------- | --- | ------ |
| Echo Client         | y?  | y       | y   | y      |
| On/Off Client       | y   | y       | y   | y      |
| LevelControl Client | ?   | y       | y   | y      |
| Identify Client     | ?   | n(\*2)  | y   | y      |
| Scene Client        | n?  | n(\*2)  | y   | y      |
| Group Client        | n?  | n(\*2)  | y   | y      |
| Binding Client      | n?  | n(\*2)  | y   | y      |

| Attributes/Clusters | iOS | Android | CLI | Python |
| ------------------- | --- | ------- | --- | ------ |
| Read                | n   | n(\*2)  | y   | y      |
| Write               | n   | n(\*2)  | y   | n      |

|                  | iOS | Android | CLI | Python |
| ---------------- | --- | ------- | --- | ------ |
| Multiple Devices | y   | n       | ?   | y      |

Notes:

(\*0) <https://github.com/project-chip/connectedhomeip/pull/4829>

(\*1) Only static commissioning, not MeshCoP.

(\*2) Locally a patch exists, could be upstreamed.
