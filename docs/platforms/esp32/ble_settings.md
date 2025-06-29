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

## Nimble: additional custom GATT services

The `ConfigureExtraServices` API is used to configure additional services
alongside the Matter services. This API allows users to add their own custom
services for provisioning or other purposes.

### Usage

```
/* Service access callback */
static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                           struct ble_gatt_access_ctxt *ctxt, void *arg);

/* Service UUID */
static const ble_uuid128_t gatt_svr_svc_uuid =
    BLE_UUID128_INIT(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12,
                     0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

/* A characteristic that can be subscribed to */
static uint16_t gatt_svr_chr_val_handle;
static const ble_uuid128_t gatt_svr_chr_uuid =
    BLE_UUID128_INIT(0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
                     0x22, 0x22, 0x22, 0x22, 0x33, 0x33, 0x33, 0x33);

/* A custom descriptor */
static const ble_uuid128_t gatt_svr_dsc_uuid =
    BLE_UUID128_INIT(0x01, 0x01, 0x01, 0x01, 0x12, 0x12, 0x12, 0x12,
                     0x23, 0x23, 0x23, 0x23, 0x34, 0x34, 0x34, 0x34);

{
    std::vector<struct ble_gatt_svc_def> gatt_svr_svcs = {
        {
            /*** Service ***/
            .type = BLE_GATT_SVC_TYPE_PRIMARY,
            .uuid = &gatt_svr_svc_uuid.u,
            .characteristics = (struct ble_gatt_chr_def[])
            { {
                    /*** This characteristic can be subscribed to by writing 0x00 and 0x01 to the CCCD ***/
                    .uuid = &gatt_svr_chr_uuid.u,
                    .access_cb = gatt_svc_access,
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_INDICATE,
                    .val_handle = &gatt_svr_chr_val_handle,
                    .descriptors = (struct ble_gatt_dsc_def[]) {
                        {
                            .uuid = &gatt_svr_dsc_uuid.u,
                            .att_flags = BLE_ATT_F_READ,
                            .access_cb = gatt_svc_access,
                        }, {
                            0, /* No more descriptors in this characteristic */
                        }
                    },
                }, {
                    0, /* No more characteristics in this service. */
                }
            },
        },
    };

    /* Add Extra service after Matter service */
    CHIP_ERROR err = chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureExtraServices(gatt_svr_svcs, ture);
}
```

Note: Extra service should be configure before `InitServer`.

## Nimble: multiple BLE advertisement

The chips that support BLE 5.0 features can advertise their custom BLE GAP
advertisement alongside Matter's BLE advertisement if `CONFIG_BT_NIMBLE_EXT_ADV`
is enabled and `CONFIG_BT_NIMBLE_MAX_EXT_ADV_INSTANCES` is a value greater
than 1.

### Usage

```
static uint8_t connectable_adv_pattern[] = {
    0x02, 0x01, 0x06,
    0x03, 0x03, 0xab, 0xcd,
    0x03, 0x03, 0x18, 0x11,
    0x12, 0X09, 'n', 'i', 'm', 'b', 'l', 'e', '-', 'c', 'o', 'n', 'n', 'e', 't', 'a', 'b', 'l', 'e'
};

/* GAP event handler */
static int ble_multi_adv_gap_event(struct ble_gap_event *event, void *arg);

{
    /* Use instance except 0 as Matter advertisement uses instance 0 */
    uint8_t instance = 1;
    struct ble_gap_ext_adv_params params;
    int size_pattern = sizeof(connectable_adv_pattern) / sizeof(connectable_adv_pattern[0]);

    memset (&params, 0, sizeof(params));

    params.connectable = 1;
    params.scannable = 1;
    params.own_addr_type = BLE_OWN_ADDR_RANDOM;
    params.sid = 1;
    params.primary_phy = BLE_HCI_LE_PHY_1M;
    params.secondary_phy = BLE_HCI_LE_PHY_1M;
    params.tx_power = 127;

    int rc;
    struct os_mbuf *data;
    int size_pattern = sizeof(legacy_dur_adv_pattern) / sizeof(legacy_dur_adv_pattern[0]);

    if (ble_gap_ext_adv_active(instance)) {
        ESP_LOGI(tag, "Instance already advertising");
        return;
    }

    rc = ble_gap_ext_adv_configure(instance, params, NULL,
                                   ble_multi_adv_gap_event, NULL);
    assert (rc == 0);

    /* get mbuf for adv data */
    data = os_msys_get_pkthdr(size_pattern, 0);
    assert(data);

    /* fill mbuf with adv data */
    rc = os_mbuf_append(data, legacy_dur_adv_pattern, size_pattern);
    assert(rc == 0);

    rc = ble_gap_ext_adv_set_data(instance, data);
    assert (rc == 0);

    /* start advertising */
    rc = ble_gap_ext_adv_start(instance, 500, 0);
    assert (rc == 0);
}
```

Note: The custom additional advertisement should be configured after BLE stack
is started.
