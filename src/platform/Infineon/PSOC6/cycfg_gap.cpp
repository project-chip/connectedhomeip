#include "cycfg_gap.h"

/* Device address */
const wiced_bt_device_address_t cy_bt_device_address = { 0x00, 0xA0, 0x50, 0x00, 0x00, 0x00 };

const uint8_t cy_bt_adv_packet_elem_0[1]           = { 0x06 };
const uint8_t cy_bt_adv_packet_elem_1[7]           = { 0x62, 0x6C, 0x65, 0x50, 0x72, 0x6F, 0x76 };
const uint8_t cy_bt_adv_packet_elem_2[16]          = { 0x5B, 0x19, 0xBA, 0xE4, 0xE4, 0x52, 0xA9, 0x96,
                                                       0xF1, 0x4A, 0x84, 0xC8, 0x09, 0x4D, 0xC0, 0x21 };
wiced_bt_ble_advert_elem_t cy_bt_adv_packet_data[] = {
    /* Flags */
    {
        .p_data      = (uint8_t *) cy_bt_adv_packet_elem_0,
        .len         = 1,
        .advert_type = BTM_BLE_ADVERT_TYPE_FLAG,
    },
    /* Complete local name */
    {
        .p_data      = (uint8_t *) cy_bt_adv_packet_elem_1,
        .len         = 7,
        .advert_type = BTM_BLE_ADVERT_TYPE_NAME_COMPLETE,
    },
    /* Complete list of 128-bit UUIDs available */
    {
        .p_data      = (uint8_t *) cy_bt_adv_packet_elem_2,
        .len         = 16,
        .advert_type = BTM_BLE_ADVERT_TYPE_128SRV_COMPLETE,
    },
};
