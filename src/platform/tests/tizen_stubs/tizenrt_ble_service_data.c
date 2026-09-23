/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * TizenRT BLE service-data parser excerpt for host fuzzing.
 * Source:
 * https://github.com/Samsung/TizenRT/blob/47cbbc77a52bad51925925bb50e5e218b86b4edf/framework/src/bluetooth/src/bluetooth-adapter.c
 */

#include "bluetooth.h"

TIZENRT_STUB_NO_SANITIZE
int bt_adapter_le_get_scan_result_service_data_list(const bt_adapter_le_device_scan_result_info_s * info,
                                                    bt_adapter_le_packet_type_e pkt_type, bt_adapter_le_service_data_s ** data_list,
                                                    int * count)
{
    BT_CHECK_LE_SUPPORT();
    BT_CHECK_INIT_STATUS();
    BT_CHECK_INPUT_PARAMETER(info); /* LCOV_EXCL_START */
    BT_CHECK_INPUT_PARAMETER(count);

    int adv_length     = 0;
    char * adv_data    = NULL;
    char * remain_data = NULL;
    int remain_len     = 0;
    int field_len      = 0;
    int data_count     = 0;
    int data_index     = 0;

    if (pkt_type == BT_ADAPTER_LE_PACKET_ADVERTISING)
    {
        adv_data   = info->adv_data;
        adv_length = info->adv_data_len;
    }
    else if (pkt_type == BT_ADAPTER_LE_PACKET_SCAN_RESPONSE)
    {
        adv_data   = info->scan_data;
        adv_length = info->scan_data_len;
    }
    else
        return BT_ERROR_INVALID_PARAMETER;

    if (!adv_data || adv_length < 3)
        return BT_ERROR_NO_DATA;

    remain_data = adv_data;
    remain_len  = adv_length;
    field_len   = 0;
    while (remain_len > 0)
    {
        field_len = remain_data[0];
        if (remain_data[1] == BT_ADAPTER_LE_ADVERTISING_DATA_SERVICE_DATA)
            data_count++;

        remain_len = remain_len - field_len - 1;
        remain_data += field_len + 1;
    }

    if (data_count == 0)
        return BT_ERROR_NO_DATA;

    *data_list = calloc(1, sizeof(bt_adapter_le_service_data_s) * data_count);
    if (*data_list == NULL)
        return BT_ERROR_OUT_OF_MEMORY;

    *count = data_count;

    remain_data = adv_data;
    remain_len  = adv_length;
    field_len   = 0;
    while (remain_len > 0)
    {
        field_len = remain_data[0];
        if (remain_data[1] == BT_ADAPTER_LE_ADVERTISING_DATA_SERVICE_DATA)
        {
            (*data_list)[data_index].service_uuid = calloc(1, sizeof(char) * 4 + 1);
            if ((*data_list)[data_index].service_uuid == NULL)
                return BT_ERROR_OUT_OF_MEMORY;

            snprintf((*data_list)[data_index].service_uuid, 5, "%2.2X%2.2X", remain_data[3], remain_data[2]);

#ifdef GLIB_SUPPORTED
            (*data_list)[data_index].service_data = g_memdup(&remain_data[4], field_len - 3);
#else
            (*data_list)[data_index].service_data = calloc(1, field_len - 3);
            if ((*data_list)[data_index].service_data == NULL)
                return BT_ERROR_OUT_OF_MEMORY;

            memcpy((*data_list)[data_index].service_data, &remain_data[4], field_len - 3);
#endif
            (*data_list)[data_index].service_data_len = field_len - 3;

            data_index++;
        }

        remain_len = remain_len - field_len - 1;
        remain_data += field_len + 1;
    }

    return BT_ERROR_NONE; /* LCOV_EXCL_STOP */
}

TIZENRT_STUB_NO_SANITIZE
int bt_adapter_le_free_service_data_list(bt_adapter_le_service_data_s * data_list, int count)
{
    int i;

    BT_CHECK_LE_SUPPORT();
    BT_CHECK_INIT_STATUS();
    BT_CHECK_INPUT_PARAMETER(data_list); /* LCOV_EXCL_START */

    for (i = 0; i < count; i++)
    {
        free(data_list[i].service_uuid);
        free(data_list[i].service_data);
    }
    free(data_list);

    return BT_ERROR_NONE; /* LCOV_EXCL_STOP */
}
