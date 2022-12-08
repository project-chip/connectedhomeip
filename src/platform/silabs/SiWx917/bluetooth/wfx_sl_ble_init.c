/*******************************************************************************
 * @file  wfx_sl_ble_init.c
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
/*************************************************************************
 *
 */

/*================================================================================
* @brief : This file contains example application for Wlan Station BLE
* Provisioning
* @section Description :
* This application explains how to get the WLAN connection functionality using
* BLE provisioning.
* Silicon Labs Module starts advertising and with BLE Provisioning the Access Point
* details are fetched.
* Silicon Labs device is configured as a WiFi station and connects to an Access Point.
=================================================================================*/

#include "wfx_sl_ble_init.h"
#include "rsi_ble_config.h"

// application defines
rsi_ble_event_conn_status_t conn_event_to_app;
rsi_ble_t att_list;
sl_wfx_msg_t event_msg;

// Memory to initialize driver
uint8_t bt_global_buf[BT_GLOBAL_BUFF_LEN];
static uint8_t wfx_rsi_drv_buf[WFX_RSI_BUF_SZ];
const uint8_t ShortUUID_CHIPoBLEService[] = { 0xF6, 0xFF };

/* Rsi driver Task will use as its stack */
// StackType_t driverRsiTaskStack[WFX_RSI_WLAN_TASK_SZ] = { 0 };

/* Structure that will hold the TCB of the wfxRsi Task being created. */
// StaticTask_t driverRsiTaskBuffer;

StaticTask_t rsiBLETaskStruct;

/* wfxRsi Task will use as its stack */
StackType_t wfxBLETaskStack[WFX_RSI_TASK_SZ] = { 0 };

int32_t ble_rsi_task(void)
{
    int32_t status;
    uint8_t buf[RSI_RESPONSE_HOLD_BUFF_SIZE];
    //    extern void rsi_hal_board_init(void);
    //
    //    WFX_RSI_LOG("%s: starting(HEAP_SZ = %d)", __func__, SL_HEAP_SIZE);
    //
    //    //! Driver initialization
    //    status = rsi_driver_init(wfx_rsi_drv_buf, WFX_RSI_BUF_SZ);
    //    if ((status < RSI_DRIVER_STATUS) || (status > WFX_RSI_BUF_SZ))
    //    {
    //        WFX_RSI_LOG("%s: error: RSI Driver initialization failed with status: %02x", __func__, status);
    //        return status;
    //    }
    //
    //    WFX_RSI_LOG("%s: rsi_device_init", __func__);
    //
    //    /* ! Redpine module intialisation */
    //    if ((status = rsi_device_init(LOAD_NWP_FW)) != RSI_SUCCESS)
    //    {
    //        WFX_RSI_LOG("%s: error: rsi_device_init failed with status: %02x", __func__, status);
    //        return status;
    //    }
    //    WFX_RSI_LOG("%s: start wireless drv task", __func__);
    //
    //    /*
    //     * Create the driver task
    //     */
    //    wfx_rsi.drv_task = xTaskCreateStatic((TaskFunction_t) rsi_wireless_driver_task, "rsi_drv", WFX_RSI_WLAN_TASK_SZ, NULL,
    //                                         1, driverRsiTaskStack, &driverRsiTaskBuffer);
    //    if (NULL == wfx_rsi.drv_task)
    //    {
    //        WFX_RSI_LOG("%s: error: Create the driver task failed", __func__);
    //        return RSI_ERROR_INVALID_PARAM;
    //    }
    //
    //    /* Initialize WiSeConnect or Module features. */
    //    WFX_RSI_LOG("%s: rsi_wireless_init", __func__);
    //    if ((status = rsi_wireless_init(OPER_MODE_0, RSI_OPERMODE_WLAN_BLE)) != RSI_SUCCESS)
    //    {
    //        WFX_RSI_LOG("%s: error: Initialize WiSeConnect failed with status: %02x", __func__, status);
    //        return status;
    //    }
    //
    //    WFX_RSI_LOG("%s: get FW version..", __func__);
    //
    //    /*
    //     * Get the MAC and other info to let the user know about it.
    //     */
    //    if (rsi_wlan_get(RSI_FW_VERSION, buf, sizeof(buf)) != RSI_SUCCESS)
    //    {
    //        WFX_RSI_LOG("%s: error: rsi_wlan_get(RSI_FW_VERSION) failed with status: %02x", __func__, status);
    //        return status;
    //    }
    //
    //    buf[sizeof(buf) - 1] = 0;
    //    WFX_RSI_LOG("%s: RSI firmware version: %s", __func__, buf);
    //    //! Send feature frame
    //    if ((status = rsi_send_feature_frame()) != RSI_SUCCESS)
    //    {
    //        WFX_RSI_LOG("%s: error: rsi_send_feature_frame failed with status: %02x", __func__, status);
    //        return status;
    //    }
    //
    //    WFX_RSI_LOG("%s: sent rsi_send_feature_frame", __func__);
    //    /* initializes wlan radio parameters and WLAN supplicant parameters.
    //     */
    //    (void) rsi_wlan_radio_init(); /* Required so we can get MAC address */
    //    if ((status = rsi_wlan_get(RSI_MAC_ADDRESS, &wfx_rsi.sta_mac.octet[0], RESP_BUFF_SIZE)) != RSI_SUCCESS)
    //    {
    //        WFX_RSI_LOG("%s: error: rsi_wlan_get failed with status: %02x", __func__, status);
    //        return status;
    //    }

    // registering the GAP callback functions
    rsi_ble_gap_register_callbacks(NULL, NULL, rsi_ble_on_disconnect_event, NULL, NULL, NULL, rsi_ble_on_enhance_conn_status_event,
                                   NULL, NULL, NULL);

    // registering the GATT call back functions
    rsi_ble_gatt_register_callbacks(NULL, NULL, NULL, NULL, NULL, NULL, NULL, rsi_ble_on_gatt_write_event, NULL, NULL, NULL,
                                    rsi_ble_on_mtu_event, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                    rsi_ble_on_event_indication_confirmation, NULL);

    WFX_RSI_LOG("registering rsi_ble_add_service");

    //  Exchange of GATT info with BLE stack
    rsi_ble_add_matter_service();

    //  initializing the application events map
    rsi_ble_app_init_events();

    wfx_rsi.ble_task = xTaskCreateStatic((TaskFunction_t) rsi_ble_event_handling_task, "rsi_ble", WFX_RSI_TASK_SZ, NULL, 2,
                                         wfxBLETaskStack, &rsiBLETaskStruct);
    WFX_RSI_LOG("%s: rsi_task_suspend init_task ", __func__);
    if (wfx_rsi.ble_task == NULL)
    {
        WFX_RSI_LOG("%s: error: failed to create ble task.", __func__);
    }

    WFX_RSI_LOG("%s complete", __func__);
    //    rsi_task_destroy((rsi_task_handle_t *)wfx_rsi.init_task);
    return RSI_SUCCESS;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_init_events
 * @brief      initializes the event parameter.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used during BLE initialization.
 */
void rsi_ble_app_init_events()
{
    event_msg.ble_app_event_map  = 0;
    event_msg.ble_app_event_mask = 0xFFFFFFFF;
    event_msg.ble_app_event_mask = event_msg.ble_app_event_mask; // To suppress warning while compiling
    WFX_RSI_LOG("Function :: rsi_ble_app_init_events");
    return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_clear_event
 * @brief      clears the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to clear the specific event.
 */
void rsi_ble_app_clear_event(uint32_t event_num)
{
    event_msg.event_num = event_num;
    event_msg.ble_app_event_map &= ~BIT(event_num);
    return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_mtu_event
 * @brief      its invoked when mtu exhange event is received.
 * @param[in]  rsi_ble_mtu, mtu event paramaters.
 * @return     none.
 * @section description
 * This callback function is invoked when  mtu exhange event is received
 */
void rsi_ble_on_mtu_event(rsi_ble_event_mtu_t * rsi_ble_mtu)
{
    WFX_RSI_LOG(" RSI_BLE : rsi_ble_on_mtu_event");
    memset(&event_msg.rsi_ble_mtu, 0, sizeof(rsi_ble_event_mtu_t));
    memcpy(&event_msg.rsi_ble_mtu, rsi_ble_mtu, sizeof(rsi_ble_event_mtu_t));
    rsi_ble_app_set_event(RSI_BLE_MTU_EVENT);
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
void rsi_ble_on_gatt_write_event(uint16_t event_id, rsi_ble_event_write_t * rsi_ble_write)
{
    WFX_RSI_LOG(" RSI_BLE : rsi_ble_on_gatt_write_event");
    memset(&event_msg.rsi_ble_write, 0, sizeof(rsi_ble_event_write_t));
    event_msg.event_id = event_id;
    memcpy(&event_msg.rsi_ble_write, rsi_ble_write, sizeof(rsi_ble_event_write_t));
    rsi_ble_app_set_event(RSI_BLE_GATT_WRITE_EVENT);
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
void rsi_ble_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t * resp_enh_conn)
{
    WFX_RSI_LOG(" RSI_BLE : rsi_ble_on_enhance_conn_status_event");
    event_msg.connectionHandle = 1;
    event_msg.bondingHandle    = 255;
    memcpy(event_msg.resp_enh_conn.dev_addr, resp_enh_conn->dev_addr, RSI_DEV_ADDR_LEN);
    rsi_ble_app_set_event(RSI_BLE_CONN_EVENT);
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
void rsi_ble_on_disconnect_event(rsi_ble_event_disconnect_t * resp_disconnect, uint16_t reason)
{
    WFX_RSI_LOG(" RSI_BLE : rsi_ble_on_disconnect_event");
    event_msg.reason = reason;
    memcpy(event_msg.resp_disconnect, resp_disconnect, sizeof(rsi_ble_event_disconnect_t));
    rsi_ble_app_set_event(RSI_BLE_DISCONN_EVENT);
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
void rsi_ble_on_event_indication_confirmation(uint16_t resp_status, rsi_ble_set_att_resp_t * rsi_ble_event_set_att_rsp)
{
    WFX_RSI_LOG(" RSI_BLE : rsi_ble_on_event_indication_confirmation");
    event_msg.resp_status = resp_status;
    memcpy(&event_msg.rsi_ble_event_set_att_rsp, rsi_ble_event_set_att_rsp, sizeof(rsi_ble_set_att_resp_t));
    rsi_ble_app_set_event(RSI_BLE_GATT_INDICATION_CONFIRMATION);
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_get_event
 * @brief      returns the first set event based on priority
 * @param[in]  none.
 * @return     int32_t
 *             > 0  = event number
 *             -1   = not received any event
 * @section description
 * This function returns the highest priority event among all the set events
 */
int32_t rsi_ble_app_get_event(void)
{
    uint32_t ix;

    for (ix = 0; ix < 32; ix++)
    {
        if (event_msg.ble_app_event_map & (1 << ix))
        {
            return ix;
        }
    }

    return (-1);
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_set_event
 * @brief      set the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to set/raise the specific event.
 */
void rsi_ble_app_set_event(uint32_t event_num)
{
    event_msg.ble_app_event_map |= BIT(event_num);
    return;
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
void rsi_gatt_add_attribute_to_list(rsi_ble_t * p_val, uint16_t handle, uint16_t data_len, uint8_t * data, uuid_t uuid,
                                    uint8_t char_prop)
{
    if ((p_val->DATA_ix + data_len) >= BLE_ATT_REC_SIZE)
    { //! Check for max data length for the characteristic value
        LOG_PRINT("\r\n no data memory for att rec values \r\n");
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
void rsi_ble_add_char_serv_att(void * serv_handler, uint16_t handle, uint8_t val_prop, uint16_t att_val_handle, uuid_t att_val_uuid)
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

void rsi_ble_add_char_val_att(void * serv_handler, uint16_t handle, uuid_t att_type_uuid, uint8_t val_prop, uint8_t * data,
                              uint8_t data_len, uint8_t auth_read)
{
    rsi_ble_req_add_att_t new_att = { 0 };

    memset(&new_att, 0, sizeof(rsi_ble_req_add_att_t));
    //! preparing the attributes
    new_att.serv_handler  = serv_handler;
    new_att.handle        = handle;
    new_att.config_bitmap = auth_read;
    memcpy(&new_att.att_uuid, &att_type_uuid, sizeof(uuid_t));
    new_att.property = val_prop;

    if (data != NULL)
        memcpy(new_att.data, data, RSI_MIN(sizeof(new_att.data), data_len));

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

uint32_t rsi_ble_add_matter_service(void)
{
    uuid_t custom_service    = { RSI_BLE_MATTER_CUSTOM_SERVICE_UUID };
    custom_service.size      = RSI_BLE_MATTER_CUSTOM_SERVICE_SIZE;
    custom_service.val.val16 = RSI_BLE_MATTER_CUSTOM_SERVICE_VALUE_16;
    uint8_t data[230]        = { RSI_BLE_MATTER_CUSTOM_SERVICE_DATA };

    static const uuid_t custom_characteristic_RX = { .size             = RSI_BLE_CUSTOM_CHARACTERISTIC_RX_SIZE,
                                                     .reserved         = { RSI_BLE_CUSTOM_CHARACTERISTIC_RX_RESERVED },
                                                     .val.val128.data1 = RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_1,
                                                     .val.val128.data2 = RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_2,
                                                     .val.val128.data3 = RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_3,
                                                     .val.val128.data4 = { RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_4 } };

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

    static const uuid_t custom_characteristic_TX = { .size             = RSI_BLE_CUSTOM_CHARACTERISTIC_TX_SIZE,
                                                     .reserved         = { RSI_BLE_CUSTOM_CHARACTERISTIC_TX_RESERVED },
                                                     .val.val128.data1 = RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_1,
                                                     .val.val128.data2 = RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_2,
                                                     .val.val128.data3 = RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_3,
                                                     .val.val128.data4 = { RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_4 } };

    // Adding custom characteristic declaration to the custom service
    rsi_ble_add_char_serv_att(
        new_serv_resp.serv_handler, new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_TX_ATTRIBUTE_HANDLE_LOCATION,
        RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE | RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ |
            RSI_BLE_ATT_PROPERTY_NOTIFY | RSI_BLE_ATT_PROPERTY_INDICATE, // Set read, write, write without response
        new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_TX_MEASUREMENT_HANDLE_LOCATION, custom_characteristic_TX);

    // Adding characteristic value attribute to the service
    event_msg.rsi_ble_measurement_hndl = new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_TX_MEASUREMENT_HANDLE_LOCATION;

    // Adding characteristic value attribute to the service
    event_msg.rsi_ble_gatt_server_client_config_hndl =
        new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_TX_GATT_SERVER_CLIENT_HANDLE_LOCATION;

    rsi_ble_add_char_val_att(new_serv_resp.serv_handler, event_msg.rsi_ble_measurement_hndl, custom_characteristic_TX,
                             RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE | RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ |
                                 RSI_BLE_ATT_PROPERTY_NOTIFY |
                                 RSI_BLE_ATT_PROPERTY_INDICATE, // Set read, write, write without response
                             data, sizeof(data), ATT_REC_MAINTAIN_IN_HOST);

    memset(&data, 0, sizeof(data));
    return 0;
}