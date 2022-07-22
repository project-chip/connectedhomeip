# PID allocation for example apps

Unless specifically overridden by the platform, example apps in this SDK use the
Example credentials implementation in `DeviceAttestationCredsExample.cpp`.

The SDK holds example certificates for VID `0xFFF1` and any PID in
`0x8000-0x801F`. The device VID and PID supplied by the basic information
cluster must correspond to the VID/PID given in the certificate for the device
to pass verification.

Certificates are selected using the value in
CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID. The vendor ID for every example app is the
same because they are all signed by the same PAI (vendor id `0xFFF1`).

In order to allow some differentiation between the various example apps, each
app is assigned a PID from the list below:

| App                     | PID      |
| ----------------------- | -------- |
| All Clusters            | `0x8001` |
| Bridge                  | `0x8002` |
| Door Lock               | `0x8003` |
| Light switch            | `0x8004` |
| Lighting                | `0x8005` |
| Lock                    | `0x8006` |
| OTA provider            | `0x8007` |
| OTA requestor           | `0x8008` |
| Persistent Storage      | `0x8009` |
| Pigweed                 | `0x800B` |
| Pump                    | `0x800A` |
| Pump Controller         | `0x8011` |
| Shell                   | `0x8012` |
| Temperature measurement | `0x800D` |
| Thermostat              | `0x800E` |
| TV                      | `0x800F` |
| Window                  | `0x8010` |
