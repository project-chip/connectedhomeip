/*******************************************************************************
 * @file  wfx_sl_ble_init.cpp
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/silabs/BLEManagerImpl.h>

using namespace chip::DeviceLayer::Internal;

// Global Variables
SilabsBleWrapper::BleEvent_t bleEvent;

/*==============================================*/
/**
 * @fn         rsi_ble_on_mtu_event
 * @brief      its invoked when mtu exhange event is received.
 * @param[in]  rsi_ble_mtu, mtu event paramaters.
 * @return     none.
 * @section description
 * This callback function is invoked when  mtu exhange event is received
 */
void SilabsBleWrapper::rsi_ble_on_mtu_event(rsi_ble_event_mtu_t * rsi_ble_mtu)
{
    bleEvent.eventType = BleEventType::RSI_BLE_MTU_EVENT;
    memcpy(&bleEvent.eventData->rsi_ble_mtu, rsi_ble_mtu, sizeof(rsi_ble_event_mtu_t));
    BLEMgrImpl().BlePostEvent(&bleEvent);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_gatt_write_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
void SilabsBleWrapper::rsi_ble_on_gatt_write_event(uint16_t event_id, rsi_ble_event_write_t * rsi_ble_write)
{
    bleEvent.eventType           = BleEventType::RSI_BLE_GATT_WRITE_EVENT;
    bleEvent.eventData->event_id = event_id;
    memcpy(&bleEvent.eventData->rsi_ble_write, rsi_ble_write, sizeof(rsi_ble_event_write_t));
    BLEMgrImpl().BlePostEvent(&bleEvent);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_enhance_conn_status_event
 * @brief      invoked when enhanced connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void SilabsBleWrapper::rsi_ble_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t * resp_enh_conn)
{
    bleEvent.eventType                   = BleEventType::RSI_BLE_CONN_EVENT;
    bleEvent.eventData->connectionHandle = 1;
    bleEvent.eventData->bondingHandle    = 255;
    memcpy(bleEvent.eventData->resp_enh_conn.dev_addr, resp_enh_conn->dev_addr, RSI_DEV_ADDR_LEN);
    BLEMgrImpl().BlePostEvent(&bleEvent);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_disconnect_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates disconnected device information and status
 */
void SilabsBleWrapper::rsi_ble_on_disconnect_event(rsi_ble_event_disconnect_t * resp_disconnect, uint16_t reason)
{
    bleEvent.eventType         = BleEventType::RSI_BLE_DISCONN_EVENT;
    bleEvent.eventData->reason = reason;
    BLEMgrImpl().BlePostEvent(&bleEvent);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_event_indication_confirmation
 * @brief      this function will invoke when received indication confirmation event
 * @param[out] resp_id, response id
 * @param[out] status, status of the response
 * @return     none
 * @section description
 */
void SilabsBleWrapper::rsi_ble_on_event_indication_confirmation(uint16_t resp_status,
                                                                rsi_ble_set_att_resp_t * rsi_ble_event_set_att_rsp)
{
    bleEvent.eventType              = BleEventType::RSI_BLE_GATT_INDICATION_CONFIRMATION;
    bleEvent.eventData->resp_status = resp_status;
    memcpy(&bleEvent.eventData->rsi_ble_event_set_att_rsp, rsi_ble_event_set_att_rsp, sizeof(rsi_ble_set_att_resp_t));
    BLEMgrImpl().BlePostEvent(&bleEvent);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_read_req_event
 * @brief      its invoked when read events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_read, read event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when read events are received
 */
void SilabsBleWrapper::rsi_ble_on_read_req_event(uint16_t event_id, rsi_ble_read_req_t * rsi_ble_read_req)
{
    bleEvent.eventType           = BleEventType::RSI_BLE_EVENT_GATT_RD;
    bleEvent.eventData->event_id = event_id;
    memcpy(&bleEvent.eventData->rsi_ble_read_req, rsi_ble_read_req, sizeof(rsi_ble_read_req_t));
    BLEMgrImpl().BlePostEvent(&bleEvent);
}

/*==============================================*/
/**
 * @fn         rsi_gatt_add_attribute_to_list
 * @brief      This function is used to store characteristic service attribute.
 * @param[in]  p_val, pointer to homekit structure
 * @param[in]  handle, characteristic service attribute handle.
 * @param[in]  data_len, characteristic value length
 * @param[in]  data, characteristic value pointer
 * @param[in]  uuid, characteristic value uuid
 * @return     none.
 * @section description
 * This function is used to store all attribute records
 */
void SilabsBleWrapper::rsi_gatt_add_attribute_to_list(rsi_ble_t * p_val, uint16_t handle, uint16_t data_len, uint8_t * data,
                                                      uuid_t uuid, uint8_t char_prop)
{
    if ((p_val->DATA_ix + data_len) >= BLE_ATT_REC_SIZE)
    { //! Check for max data length for the characteristic value
        return;
    }

    p_val->att_rec_list[p_val->att_rec_list_count].char_uuid     = uuid;
    p_val->att_rec_list[p_val->att_rec_list_count].handle        = handle;
    p_val->att_rec_list[p_val->att_rec_list_count].value_len     = data_len;
    p_val->att_rec_list[p_val->att_rec_list_count].max_value_len = data_len;
    p_val->att_rec_list[p_val->att_rec_list_count].char_val_prop = char_prop;
    memcpy(p_val->DATA + p_val->DATA_ix, data, data_len);
    p_val->att_rec_list[p_val->att_rec_list_count].value = p_val->DATA + p_val->DATA_ix;
    p_val->att_rec_list_count++;
    p_val->DATA_ix += p_val->att_rec_list[p_val->att_rec_list_count].max_value_len;

    return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_add_char_serv_att
 * @brief      this function is used to add characteristic service attribute..
 * @param[in]  serv_handler, service handler.
 * @param[in]  handle, characteristic service attribute handle.
 * @param[in]  val_prop, characteristic value property.
 * @param[in]  att_val_handle, characteristic value handle
 * @param[in]  att_val_uuid, characteristic value uuid
 * @return     none.
 * @section description
 * This function is used at application to add characteristic attribute
 */
void SilabsBleWrapper::rsi_ble_add_char_serv_att(void * serv_handler, uint16_t handle, uint8_t val_prop, uint16_t att_val_handle,
                                                 uuid_t att_val_uuid)
{
    rsi_ble_req_add_att_t new_att = { 0 };

    //! preparing the attribute service structure
    new_att.serv_handler       = serv_handler;
    new_att.handle             = handle;
    new_att.att_uuid.size      = 2;
    new_att.att_uuid.val.val16 = RSI_BLE_CHAR_SERV_UUID;
    new_att.property           = RSI_BLE_ATT_PROPERTY_READ;

    //! preparing the characteristic attribute value
    new_att.data_len = att_val_uuid.size + 4;
    new_att.data[0]  = val_prop;
    rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
    if (new_att.data_len == 6)
    {
        rsi_uint16_to_2bytes(&new_att.data[4], att_val_uuid.val.val16);
    }
    else if (new_att.data_len == 8)
    {
        rsi_uint32_to_4bytes(&new_att.data[4], att_val_uuid.val.val32);
    }
    else if (new_att.data_len == 20)
    {
        memcpy(&new_att.data[4], &att_val_uuid.val.val128, att_val_uuid.size);
    }
    //! Add attribute to the service
    rsi_ble_add_attribute(&new_att);

    return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_add_char_val_att
 * @brief      this function is used to add characteristic value attribute.
 * @param[in]  serv_handler, new service handler.
 * @param[in]  handle, characteristic value attribute handle.
 * @param[in]  att_type_uuid, attribute uuid value.
 * @param[in]  val_prop, characteristic value property.
 * @param[in]  data, characteristic value data pointer.
 * @param[in]  data_len, characteristic value length.
 * @return     none.
 * @section description
 * This function is used at application to create new service.
 */

void SilabsBleWrapper::rsi_ble_add_char_val_att(void * serv_handler, uint16_t handle, uuid_t att_type_uuid, uint8_t val_prop,
                                                uint8_t * data, uint8_t data_len, uint8_t auth_read)
{
    rsi_ble_req_add_att_t new_att = { 0 };
    rsi_ble_t att_list;
    memset(&new_att, 0, sizeof(rsi_ble_req_add_att_t));
    //! preparing the attributes
    new_att.serv_handler  = serv_handler;
    new_att.handle        = handle;
    new_att.config_bitmap = auth_read;
    memcpy(&new_att.att_uuid, &att_type_uuid, sizeof(uuid_t));
    new_att.property = val_prop;

    if (data != NULL)
    {
        memcpy(new_att.data, data, RSI_MIN(sizeof(new_att.data), data_len));
    }

    //! preparing the attribute value
    new_att.data_len = data_len;

    //! add attribute to the service
    rsi_ble_add_attribute(&new_att);

    if ((auth_read == ATT_REC_MAINTAIN_IN_HOST) || (data_len > 20))
    {
        if (data != NULL)
        {
            rsi_gatt_add_attribute_to_list(&att_list, handle, data_len, data, att_type_uuid, val_prop);
        }
    }

    //! check the attribute property with notification/Indication
    if ((val_prop & RSI_BLE_ATT_PROPERTY_NOTIFY) || (val_prop & RSI_BLE_ATT_PROPERTY_INDICATE))
    {
        //! if notification/indication property supports then we need to add client characteristic service.

        //! preparing the client characteristic attribute & values
        memset(&new_att, 0, sizeof(rsi_ble_req_add_att_t));
        new_att.serv_handler       = serv_handler;
        new_att.handle             = handle + 1;
        new_att.att_uuid.size      = 2;
        new_att.att_uuid.val.val16 = RSI_BLE_CLIENT_CHAR_UUID;
        new_att.property           = RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE;
        new_att.data_len           = 2;

        //! add attribute to the service
        rsi_ble_add_attribute(&new_att);
    }

    return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_add_matter_service
 * @brief      this function is used to add service for matter
 * @return     status (uint32_t) 0 for success.
 * @section description
 * This function is used at application to create new service.
 */

uint32_t SilabsBleWrapper::rsi_ble_add_matter_service(void)
{
    uuid_t custom_service                                   = { RSI_BLE_MATTER_CUSTOM_SERVICE_UUID };
    custom_service.size                                     = RSI_BLE_MATTER_CUSTOM_SERVICE_SIZE;
    custom_service.val.val16                                = RSI_BLE_MATTER_CUSTOM_SERVICE_VALUE_16;
    uint8_t data[RSI_BLE_MATTER_CUSTOM_SERVICE_DATA_LENGTH] = { RSI_BLE_MATTER_CUSTOM_SERVICE_DATA };

    static const uuid_t custom_characteristic_RX = {
        .size     = RSI_BLE_CUSTOM_CHARACTERISTIC_RX_SIZE,
        .reserved = { RSI_BLE_CUSTOM_CHARACTERISTIC_RX_RESERVED },
        .val      = { .val128 = { .data1 = { RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_1 },
                                  .data2 = { RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_2 },
                                  .data3 = { RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_3 },
                                  .data4 = { RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_4 } } }
    };

    rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
    rsi_ble_add_service(custom_service, &new_serv_resp);

    // Adding custom characteristic declaration to the custom service
    rsi_ble_add_char_serv_att(
        new_serv_resp.serv_handler, new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_RX_ATTRIBUTE_HANDLE_LOCATION,
        RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ, // Set read, write, write without response
        new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_RX_VALUE_HANDLE_LOCATION, custom_characteristic_RX);

    // Adding characteristic value attribute to the service
    rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                             new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_RX_VALUE_HANDLE_LOCATION, custom_characteristic_RX,
                             RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ, // Set read, write, write without response
                             data, sizeof(data), ATT_REC_IN_HOST);

    static const uuid_t custom_characteristic_TX = {
        .size     = RSI_BLE_CUSTOM_CHARACTERISTIC_TX_SIZE,
        .reserved = { RSI_BLE_CUSTOM_CHARACTERISTIC_TX_RESERVED },
        .val      = { .val128 = { .data1 = { RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_1 },
                                  .data2 = { RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_2 },
                                  .data3 = { RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_3 },
                                  .data4 = { RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_4 } } }
    };

    // Adding custom characteristic declaration to the custom service
    rsi_ble_add_char_serv_att(
        new_serv_resp.serv_handler, new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_TX_ATTRIBUTE_HANDLE_LOCATION,
        RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE | RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ |
            RSI_BLE_ATT_PROPERTY_NOTIFY | RSI_BLE_ATT_PROPERTY_INDICATE, // Set read, write, write without response
        new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_TX_MEASUREMENT_HANDLE_LOCATION, custom_characteristic_TX);

    // Adding characteristic value attribute to the service
    bleEvent.eventData->rsi_ble_measurement_hndl =
        new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_TX_MEASUREMENT_HANDLE_LOCATION;

    // Adding characteristic value attribute to the service
    bleEvent.eventData->rsi_ble_gatt_server_client_config_hndl =
        new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_TX_GATT_SERVER_CLIENT_HANDLE_LOCATION;

    rsi_ble_add_char_val_att(new_serv_resp.serv_handler, bleEvent.eventData->rsi_ble_measurement_hndl, custom_characteristic_TX,
                             RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE | RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ |
                                 RSI_BLE_ATT_PROPERTY_NOTIFY |
                                 RSI_BLE_ATT_PROPERTY_INDICATE, // Set read, write, write without response
                             data, sizeof(data), ATT_REC_MAINTAIN_IN_HOST);

    memset(&data, 0, sizeof(data));
    return 0;
}
