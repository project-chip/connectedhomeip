# Bluetooth Low Energy (BLE)

## Nimble: scan response

The `ConfigureScanResponseData` API is used to configure the scan response data
for advertising in a Bluetooth Low Energy (BLE) application based on the NimBLE
BLE stack. Scan response data is additional data that a BLE peripheral device
can include in its advertising packets to provide more information about itself.
This API allows you to set the scan response data that will be included in the
advertising packets.

### Usage

```
{
    uint8_t scanResponse[31]; // 0x05, 0x09, a, b, c, d
    scanResponse[0] = 0x05;
    scanResponse[1] = 0x09;
    scanResponse[2] = 0x61;
    scanResponse[3] = 0x62;
    scanResponse[4] = 0x63;
    scanResponse[5] = 0x64;
    chip::ByteSpan data(scanResponse);
    CHIP_ERROR err = chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureScanResponseData(data);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to configure scan response, err:%" CHIP_ERROR_FORMAT, err.Format());
    }
}
```

Note: Scan response should be configure before `InitServer`.
