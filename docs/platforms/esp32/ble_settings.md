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

    // Max length is 31 bytes
    // Enter data in (length, type, value) format
    // 0x05 - length of data
    // 0x09 - Type (Complete Local Name)
    // 0x61, 0x62, 0x63, 0x64 - Data (a,b,c,d)
    uint8_t scanResponse[] = { 0x05, 0x09, 0x61, 0x62, 0x63, 0x64};

    chip::ByteSpan data(scanResponse);
    CHIP_ERROR err = chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureScanResponseData(data);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to configure scan response, err:%" CHIP_ERROR_FORMAT, err.Format());
    }
}


```

Note: Scan response should be configure before `InitServer`.
