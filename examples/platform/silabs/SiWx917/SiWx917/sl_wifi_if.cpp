/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "silabs_utils.h"
#include "sl_status.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "sl_board_configuration.h"
extern "C" {
#include "sl_si91x_types.h"
#include "sl_wifi_constants.h"
#include "sl_wifi_types.h"
#include "sl_wlan_config.h"
}
#include "task.h"

#if (EXP_BOARD)
#include "rsi_bt_common_apis.h"
#endif

#include "ble_config.h"

#if SL_ICD_ENABLED && SLI_SI91X_MCU_INTERFACE
#include "rsi_rom_power_save.h"
#include "sl_si91x_button_pin_config.h"
extern "C" {
#include "sl_si91x_driver.h"
#include "sl_si91x_m4_ps.h"
}

// TODO: should be removed once we are getting the press interrupt for button 0 with sleep
#define BUTTON_PRESSED 1
bool btn0_pressed = false;
#endif // SL_ICD_ENABLED && SLI_SI91X_MCU_INTERFACE

#include "dhcp_client.h"
#include "wfx_host_events.h"
#include "wfx_rsi.h"
#define ADV_SCAN_THRESHOLD -40
#define ADV_RSSI_TOLERANCE_THRESHOLD 5
#define ADV_ACTIVE_SCAN_DURATION 15
#define ADV_PASSIVE_SCAN_DURATION 20
#define ADV_MULTIPROBE 1
#define ADV_SCAN_PERIODICITY 10

// TODO: Confirm that this value works for size and timing
#define WFX_QUEUE_SIZE 10
extern "C" {
#include "sl_net.h"
#include "sl_si91x_host_interface.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "wfx_host_events.h"
#if SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_trng.h"
#define TRNGKEY_SIZE 4
#endif // SLI_SI91X_MCU_INTERFACE
} // extern "C" {

WfxRsi_t wfx_rsi;

/* Declare a variable to hold the data associated with the created event group. */
StaticEventGroup_t rsiDriverEventGroup;

bool hasNotifiedIPV6 = false;
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
bool hasNotifiedIPV4 = false;
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
bool hasNotifiedWifiConnectivity = false;

/* Declare a flag to differentiate between after boot-up first IP connection or reconnection */
bool is_wifi_disconnection_event = false;

/* Declare a variable to hold connection time intervals */
uint32_t retryInterval                 = WLAN_MIN_RETRY_TIMER_MS;
volatile bool scan_results_complete    = false;
volatile bool bg_scan_results_complete = false;

// TODO: Figure out why we actually need this, we are already handling failure and retries somewhere else.
#define WIFI_SCAN_TIMEOUT_TICK 10000

extern osSemaphoreId_t sl_rs_ble_init_sem;

/*
 * This file implements the interface to the wifi sdk
 */

static wfx_wifi_scan_ext_t temp_reset;

volatile sl_status_t callback_status = SL_STATUS_OK;

// Scan semaphore
static osSemaphoreId_t sScanSemaphore;
// DHCP Poll timer
static osTimerId_t sDHCPTimer;
static osMessageQueueId_t sWifiEventQueue = NULL;

static void DHCPTimerEventHandler(void * arg)
{
    WfxEvent_t event;
    event.eventType = WFX_EVT_DHCP_POLL;
    WfxPostEvent(&event);
}

static void CancelDHCPTimer()
{
    osStatus_t status;

    // Check if timer started
    if (!osTimerIsRunning(sDHCPTimer))
    {
        SILABS_LOG("CancelDHCPTimer: timer not running");
        return;
    }

    status = osTimerStop(sDHCPTimer);
    if (status != osOK)
    {
        SILABS_LOG("CancelDHCPTimer: failed to stop timer with status: %d", status);
    }
}

static void StartDHCPTimer(uint32_t timeout)
{
    osStatus_t status;

    // Cancel timer if already started
    CancelDHCPTimer();

    status = osTimerStart(sDHCPTimer, pdMS_TO_TICKS(timeout));
    if (status != osOK)
    {
        SILABS_LOG("StartDHCPTimer: failed to start timer with status: %d", status);
    }
}

/******************************************************************
 * @fn   int32_t wfx_rsi_get_ap_info(wfx_wifi_scan_result_t *ap)
 * @brief
 *       Getting the AP details
 * @param[in] ap: access point
 * @return
 *        status
 *********************************************************************/
int32_t wfx_rsi_get_ap_info(wfx_wifi_scan_result_t * ap)
{
    sl_status_t status = SL_STATUS_OK;
    int32_t rssi       = 0;
    ap->security       = wfx_rsi.sec.security;
    ap->chan           = wfx_rsi.ap_chan;
    memcpy(&ap->bssid[0], &wfx_rsi.ap_mac.octet[0], BSSID_MAX_STR_LEN);
    sl_wifi_get_signal_strength(SL_WIFI_CLIENT_INTERFACE, &rssi);
    ap->rssi = rssi;
    return status;
}

/******************************************************************
 * @fn   int32_t wfx_rsi_get_ap_ext(wfx_wifi_scan_ext_t *extra_info)
 * @brief
 *       Getting the AP extra details
 * @param[in] extra info: access point extra information
 * @return
 *        status
 *********************************************************************/
int32_t wfx_rsi_get_ap_ext(wfx_wifi_scan_ext_t * extra_info)
{
    sl_status_t status        = SL_STATUS_OK;
    sl_wifi_statistics_t test = { 0 };
    status                    = sl_wifi_get_statistics(SL_WIFI_CLIENT_INTERFACE, &test);
    VERIFY_STATUS_AND_RETURN(status);
    extra_info->beacon_lost_count = test.beacon_lost_count - temp_reset.beacon_lost_count;
    extra_info->beacon_rx_count   = test.beacon_rx_count - temp_reset.beacon_rx_count;
    extra_info->mcast_rx_count    = test.mcast_rx_count - temp_reset.mcast_rx_count;
    extra_info->mcast_tx_count    = test.mcast_tx_count - temp_reset.mcast_tx_count;
    extra_info->ucast_rx_count    = test.ucast_rx_count - temp_reset.ucast_rx_count;
    extra_info->ucast_tx_count    = test.ucast_tx_count - temp_reset.ucast_tx_count;
    extra_info->overrun_count     = test.overrun_count - temp_reset.overrun_count;
    return status;
}

/******************************************************************
 * @fn   int32_t wfx_rsi_reset_count()
 * @brief
 *       Getting the driver reset count
 * @param[in] None
 * @return
 *        status
 *********************************************************************/
int32_t wfx_rsi_reset_count()
{
    sl_wifi_statistics_t test = { 0 };
    sl_status_t status        = SL_STATUS_OK;
    status                    = sl_wifi_get_statistics(SL_WIFI_CLIENT_INTERFACE, &test);
    VERIFY_STATUS_AND_RETURN(status);
    temp_reset.beacon_lost_count = test.beacon_lost_count;
    temp_reset.beacon_rx_count   = test.beacon_rx_count;
    temp_reset.mcast_rx_count    = test.mcast_rx_count;
    temp_reset.mcast_tx_count    = test.mcast_tx_count;
    temp_reset.ucast_rx_count    = test.ucast_rx_count;
    temp_reset.ucast_tx_count    = test.ucast_tx_count;
    temp_reset.overrun_count     = test.overrun_count;
    return status;
}

/******************************************************************
 * @fn   wfx_rsi_disconnect()
 * @brief
 *       Getting the driver disconnect status
 * @param[in] None
 * @return
 *        status
 *********************************************************************/
int32_t wfx_rsi_disconnect()
{
    return sl_wifi_disconnect(SL_WIFI_CLIENT_INTERFACE);
}

sl_status_t join_callback_handler(sl_wifi_event_t event, char * result, uint32_t result_length, void * arg)
{
    WfxEvent_t WfxEvent;

    wfx_rsi.dev_state &= ~(WFX_RSI_ST_STA_CONNECTING);
    if (SL_WIFI_CHECK_IF_EVENT_FAILED(event))
    {
        SILABS_LOG("F: Join Event received with %u bytes payload\n", result_length);
        callback_status = *(sl_status_t *) result;
        wfx_rsi.dev_state &= ~(WFX_RSI_ST_STA_CONNECTED);
        is_wifi_disconnection_event = true;
        wfx_retry_interval_handler(is_wifi_disconnection_event, wfx_rsi.join_retries++);
        if (is_wifi_disconnection_event || wfx_rsi.join_retries <= WFX_RSI_CONFIG_MAX_JOIN)
        {
            WfxEvent.eventType = WFX_EVT_STA_START_JOIN;
            WfxPostEvent(&WfxEvent);
        }
        return SL_STATUS_FAIL;
    }
    /*
     * Join was complete - Do the DHCP
     */
    memset(&temp_reset, 0, sizeof(wfx_wifi_scan_ext_t));
    SILABS_LOG("join_callback_handler: join completed.");
    SILABS_LOG("%c: Join Event received with %u bytes payload\n", *result, result_length);

    WfxEvent.eventType = WFX_EVT_STA_CONN;
    WfxPostEvent(&WfxEvent);
    wfx_rsi.join_retries = 0;
    retryInterval        = WLAN_MIN_RETRY_TIMER_MS;
    if (is_wifi_disconnection_event)
    {
        is_wifi_disconnection_event = false;
    }
    callback_status = SL_STATUS_OK;
    return SL_STATUS_OK;
}

#if SL_ICD_ENABLED

#if SLI_SI91X_MCU_INTERFACE
/******************************************************************
 * @fn   sl_wfx_host_si91x_sleep_wakeup()
 * @brief
 *       M4 going to sleep
 *
 * @param[in] None
 * @return
 *        None
 *********************************************************************/
void sl_wfx_host_si91x_sleep_wakeup()
{
    if (wfx_rsi.dev_state & WFX_RSI_ST_SLEEP_READY)
    {
        // TODO: should be removed once we are getting the press interrupt for button 0 with sleep
        if (!RSI_NPSSGPIO_GetPin(SL_BUTTON_BTN0_PIN) && !btn0_pressed)
        {
            sl_button_on_change(SL_BUTTON_BTN0_NUMBER, BUTTON_PRESSED);
            btn0_pressed = true;
        }
        if (RSI_NPSSGPIO_GetPin(SL_BUTTON_BTN0_PIN))
        {
#ifdef DISPLAY_ENABLED
            // if LCD is enabled, power down the lcd before setting the M4 to sleep
            sl_si91x_hardware_setup();
#endif
            btn0_pressed = false;
            /* Configure RAM Usage and Retention Size */
            sl_si91x_m4_sleep_wakeup();
#if SILABS_LOG_ENABLED
            silabsInitLog();
#endif
        }
    }
}
#endif // SLI_SI91X_MCU_INTERFACE

/******************************************************************
 * @fn   wfx_rsi_power_save()
 * @brief
 *       Setting the RS911x in DTIM sleep based mode
 *
 * @param[in] sl_si91x_ble_state : State to set for the BLE
              sl_si91x_wifi_state : State to set for the WiFi
 * @return
 *        None
 *********************************************************************/
int32_t wfx_rsi_power_save(rsi_power_save_profile_mode_t sl_si91x_ble_state, sl_si91x_performance_profile_t sl_si91x_wifi_state)
{
    int32_t status;

    status = rsi_bt_power_save_profile(sl_si91x_ble_state, 0);
    if (status != RSI_SUCCESS)
    {
        SILABS_LOG("BT Powersave Config Failed, Error Code : 0x%lX", status);
        return status;
    }
    sl_wifi_performance_profile_t wifi_profile = { .profile = sl_si91x_wifi_state };
    status                                     = sl_wifi_set_performance_profile(&wifi_profile);
    if (status != RSI_SUCCESS)
    {
        SILABS_LOG("Powersave Config Failed, Error Code : 0x%lX", status);
        return status;
    }
    if (sl_si91x_wifi_state == HIGH_PERFORMANCE)
    {
        wfx_rsi.dev_state &= ~(WFX_RSI_ST_SLEEP_READY);
    }
    else
    {
        wfx_rsi.dev_state |= WFX_RSI_ST_SLEEP_READY;
    }
    return status;
}
#endif /* SL_ICD_ENABLED */

/*************************************************************************************
 * @fn  static int32_t wfx_wifi_rsi_init(void)
 * @brief
 *      Wifi initialization called from app main
 * @param[in]  None
 * @return
 *        None
 *****************************************************************************************/
int32_t wfx_wifi_rsi_init(void)
{
    SILABS_LOG("wfx_wifi_rsi_init started");
    sl_status_t status;
    status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
    if (status != SL_STATUS_OK)
    {
        return status;
    }

    // Create Sempaphore for scan
    sScanSemaphore = osSemaphoreNew(1, 0, NULL);
    if (sScanSemaphore == NULL)
    {
        return SL_STATUS_ALLOCATION_FAILED;
    }
    // Create the message queue
    sWifiEventQueue = osMessageQueueNew(WFX_QUEUE_SIZE, sizeof(WfxEvent_t), NULL);
    if (sWifiEventQueue == NULL)
    {
        return SL_STATUS_ALLOCATION_FAILED;
    }

    // Create timer for DHCP polling
    // TODO: Use LWIP timer instead of creating a new one here
    sDHCPTimer = osTimerNew(DHCPTimerEventHandler, osTimerPeriodic, NULL, NULL);
    if (sDHCPTimer == NULL)
    {
        return SL_STATUS_ALLOCATION_FAILED;
    }

    return status;
}

/*************************************************************************************
 * @fn  static void sl_print_firmware_version(sl_wifi_firmware_version_t*)
 * @brief
 *      To print the firmware version
 * @param[in]  sl_wifi_firmware_version_t*
 * @return
 *        None
 *****************************************************************************************/
static void sl_print_firmware_version(sl_wifi_firmware_version_t * firmware_version)
{
    SILABS_LOG("Firmware version is: %x%x.%d.%d.%d.%d.%d.%d", firmware_version->chip_id, firmware_version->rom_id,
               firmware_version->major, firmware_version->minor, firmware_version->security_version, firmware_version->patch_num,
               firmware_version->customer_id, firmware_version->build_num);
}

/*************************************************************************************
 * @fn  static int32_t wfx_rsi_init(void)
 * @brief
 *      driver initialization
 * @param[in]  None
 * @return
 *        None
 *****************************************************************************************/
static sl_status_t wfx_rsi_init(void)
{
    sl_status_t status;

#ifndef SLI_SI91X_MCU_INTERFACE
    status = wfx_wifi_rsi_init();
    if (status != SL_STATUS_OK)
    {
        SILABS_LOG("wfx_rsi_init failed %x", status);
        return status;
    }
#else // For SoC
#if SL_ICD_ENABLED
    uint8_t xtal_enable = 1;
    status              = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_XTAL, 1, &xtal_enable, 0, NULL);
    if (status != SL_STATUS_OK)
    {
        SILABS_LOG("Failed to bring m4_ta_secure_handshake: 0x%lx\r\n", status);
        return status;
    }
#endif /* SL_ICD_ENABLED */
#endif /* SLI_SI91X_MCU_INTERFACE */

    sl_wifi_firmware_version_t version = { 0 };
    status                             = sl_wifi_get_firmware_version(&version);
    if (status != SL_STATUS_OK)
    {
        SILABS_LOG("Get fw version failed:");
        sl_print_firmware_version(&version);
        return status;
    }
    sl_print_firmware_version(&version);

    status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, (sl_mac_address_t *) &wfx_rsi.sta_mac.octet[0]);
    if (status != SL_STATUS_OK)
    {
        SILABS_LOG("sl_wifi_get_mac_address failed: %x", status);
        return status;
    }

    const uint32_t trngKey[TRNGKEY_SIZE] = { 0x16157E2B, 0xA6D2AE28, 0x8815F7AB, 0x3C4FCF09 };

    // To check the Entropy of TRNG and verify TRNG functioning.
    status = sl_si91x_trng_entropy();
    if (status != SL_STATUS_OK)
    {
        SILABS_LOG("TRNG Entropy Failed");
        return status;
    }

    // Initiate and program the key required for TRNG hardware engine
    status = sl_si91x_trng_program_key((uint32_t *) trngKey, TRNGKEY_SIZE);
    if (status != SL_STATUS_OK)
    {
        SILABS_LOG("TRNG Key Programming Failed");
        return status;
    }

    wfx_rsi.events = xEventGroupCreateStatic(&rsiDriverEventGroup);
    wfx_rsi.dev_state |= WFX_RSI_ST_DEV_READY;
    osSemaphoreRelease(sl_rs_ble_init_sem);
    return status;
}

/*************************************************************************************
 * @fn  void wfx_show_err(char *msg)
 * @brief
 *      driver shows error message
 * @param[in]  msg
 * @return
 *        None
 *****************************************************************************************/
void wfx_show_err(char * msg)
{
    SILABS_LOG("wfx_show_err: message: %d", msg);
}

sl_status_t scan_callback_handler(sl_wifi_event_t event, sl_wifi_scan_result_t * scan_result, uint32_t result_length, void * arg)
{
    if (SL_WIFI_CHECK_IF_EVENT_FAILED(event))
    {
        callback_status       = *(sl_status_t *) scan_result;
        scan_results_complete = true;
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
        wfx_rsi.sec.security = WFX_SEC_WPA3;
#else
        wfx_rsi.sec.security = WFX_SEC_WPA2;
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */

        osSemaphoreRelease(sScanSemaphore);
        return SL_STATUS_FAIL;
    }
    wfx_rsi.sec.security = WFX_SEC_UNSPECIFIED;
    wfx_rsi.ap_chan      = scan_result->scan_info[0].rf_channel;
    memcpy(&wfx_rsi.ap_mac.octet, scan_result->scan_info[0].bssid, BSSID_MAX_STR_LEN);
    switch (scan_result->scan_info[0].security_mode)
    {
    case SL_WIFI_OPEN:
        wfx_rsi.sec.security = WFX_SEC_NONE;
        break;
    case SL_WIFI_WPA:
    case SL_WIFI_WPA_ENTERPRISE:
    case SL_WIFI_WPA_WPA2_MIXED:
        wfx_rsi.sec.security = WFX_SEC_WPA;
        break;
    case SL_WIFI_WPA2:
    case SL_WIFI_WPA2_ENTERPRISE:
        wfx_rsi.sec.security = WFX_SEC_WPA2;
        break;
    case SL_WIFI_WEP:
        wfx_rsi.sec.security = WFX_SEC_WEP;
        break;
    case SL_WIFI_WPA3_TRANSITION:
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
    case SL_WIFI_WPA3:
        wfx_rsi.sec.security = WFX_SEC_WPA3;
#else
        wfx_rsi.sec.security = WFX_SEC_WPA2;
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
        break;
    default:
        wfx_rsi.sec.security = WFX_SEC_UNSPECIFIED;
        break;
    }
    wfx_rsi.dev_state &= ~WFX_RSI_ST_SCANSTARTED;
    scan_results_complete = true;

    osSemaphoreRelease(sScanSemaphore);
    return SL_STATUS_OK;
}
sl_status_t show_scan_results(sl_wifi_scan_result_t * scan_result)
{
    SL_WIFI_ARGS_CHECK_NULL_POINTER(scan_result);
    int x;
    wfx_wifi_scan_result_t ap;
    for (x = 0; x < (int) scan_result->scan_count; x++)
    {
        strcpy(&ap.ssid[0], (char *) &scan_result->scan_info[x].ssid);
        if (wfx_rsi.scan_ssid)
        {
            SILABS_LOG("SCAN SSID: %s , ap scan: %s", wfx_rsi.scan_ssid, ap.ssid);
            if (strcmp(wfx_rsi.scan_ssid, ap.ssid) == CMP_SUCCESS)
            {
                ap.security = static_cast<wfx_sec_t>(scan_result->scan_info[x].security_mode);
                ap.rssi     = (-1) * scan_result->scan_info[x].rssi_val;
                memcpy(&ap.bssid[0], &scan_result->scan_info[x].bssid[0], BSSID_MAX_STR_LEN);
                (*wfx_rsi.scan_cb)(&ap);
                break;
            }
        }
        else
        {
            ap.security = static_cast<wfx_sec_t>(scan_result->scan_info[x].security_mode);
            ap.rssi     = (-1) * scan_result->scan_info[x].rssi_val;
            memcpy(&ap.bssid[0], &scan_result->scan_info[x].bssid[0], BSSID_MAX_STR_LEN);
            (*wfx_rsi.scan_cb)(&ap);
        }
    }
    wfx_rsi.dev_state &= ~WFX_RSI_ST_SCANSTARTED;
    (*wfx_rsi.scan_cb)((wfx_wifi_scan_result_t *) 0);
    wfx_rsi.scan_cb = (void (*)(wfx_wifi_scan_result_t *)) 0;
    if (wfx_rsi.scan_ssid)
    {
        vPortFree(wfx_rsi.scan_ssid);
        wfx_rsi.scan_ssid = (char *) 0;
    }
    return SL_STATUS_OK;
}
sl_status_t bg_scan_callback_handler(sl_wifi_event_t event, sl_wifi_scan_result_t * result, uint32_t result_length, void * arg)
{
    callback_status          = show_scan_results(result);
    bg_scan_results_complete = true;
    osSemaphoreRelease(sScanSemaphore);
    return SL_STATUS_OK;
}
/***************************************************************************************
 * @fn   static void wfx_rsi_save_ap_info()
 * @brief
 *       Saving the details of the AP
 * @param[in]  None
 * @return
 *       None
 *******************************************************************************************/
static void wfx_rsi_save_ap_info() // translation
{
    sl_status_t status = SL_STATUS_OK;
#ifndef EXP_BOARD // TODO: this changes will be reverted back after the SDK team fix the scan API
    sl_wifi_scan_configuration_t wifi_scan_configuration = default_wifi_scan_configuration;
#endif
    sl_wifi_ssid_t ssid_arg;
    ssid_arg.length = strlen(wfx_rsi.sec.ssid);
    memcpy(ssid_arg.value, (int8_t *) &wfx_rsi.sec.ssid[0], ssid_arg.length);
    sl_wifi_set_scan_callback(scan_callback_handler, NULL);
    scan_results_complete = false;
#ifndef EXP_BOARD
    // TODO: this changes will be reverted back after the SDK team fix the scan API
    status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &ssid_arg, &wifi_scan_configuration);
#endif
    if (SL_STATUS_IN_PROGRESS == status)
    {
        osSemaphoreAcquire(sScanSemaphore, WIFI_SCAN_TIMEOUT_TICK);
    }
}

/********************************************************************************************
 * @fn   static void wfx_rsi_do_join(void)
 * @brief
 *        Start an async Join command
 * @return
 *        None
 **********************************************************************************************/
static sl_status_t wfx_rsi_do_join(void)
{
    sl_status_t status = SL_STATUS_OK;
    sl_wifi_security_t connect_security_mode;
    WfxEvent_t event;
    switch (wfx_rsi.sec.security)
    {
    case WFX_SEC_WEP:
        connect_security_mode = SL_WIFI_WEP;
        break;
    case WFX_SEC_WPA:
    case WFX_SEC_WPA2:
        connect_security_mode = SL_WIFI_WPA_WPA2_MIXED;
        break;
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
    case WFX_SEC_WPA3:
        connect_security_mode = SL_WIFI_WPA3_TRANSITION;
        break;
#endif /*WIFI_ENABLE_SECURITY_WPA3_TRANSITION*/
    case WFX_SEC_NONE:
        connect_security_mode = SL_WIFI_OPEN;
        break;
    default:
        SILABS_LOG("error: unknown security type.");
        return status;
    }

    if (wfx_rsi.dev_state & (WFX_RSI_ST_STA_CONNECTING | WFX_RSI_ST_STA_CONNECTED))
    {
        SILABS_LOG("%s: not joining - already in progress", __func__);
    }
    else
    {
        SILABS_LOG("%s: WLAN: connecting to %s, sec=%d", __func__, &wfx_rsi.sec.ssid[0], wfx_rsi.sec.security);

        /*
         * Join the network
         */
        /* TODO - make the WFX_SECURITY_xxx - same as RSI_xxx
         * Right now it's done by hand - we need something better
         */
        wfx_rsi.dev_state |= WFX_RSI_ST_STA_CONNECTING;

        sl_wifi_set_join_callback(join_callback_handler, NULL);

#if SL_ICD_ENABLED
        // Setting the listen interval to 0 which will set it to DTIM interval
        sl_wifi_listen_interval_t sleep_interval = { .listen_interval = 0 };
        status                                   = sl_wifi_set_listen_interval(SL_WIFI_CLIENT_INTERFACE, sleep_interval);

        sl_wifi_advanced_client_configuration_t client_config = { .max_retry_attempts = 5 };
        sl_wifi_set_advanced_client_configuration(SL_WIFI_CLIENT_INTERFACE, &client_config);
#endif // SL_ICD_ENABLED
        /* Try to connect Wifi with given Credentials
         * untill there is a success or maximum number of tries allowed
         */

        /* Call rsi connect call with given ssid and password
         * And check there is a success
         */
        sl_wifi_credential_t cred;
        memset(&cred, 0, sizeof(sl_wifi_credential_t));
        cred.type = SL_WIFI_PSK_CREDENTIAL;
        memcpy(cred.psk.value, &wfx_rsi.sec.passkey[0], strlen(wfx_rsi.sec.passkey));
        sl_net_credential_id_t id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
        status = sl_net_set_credential(id, SL_NET_WIFI_PSK, &wfx_rsi.sec.passkey[0], strlen(wfx_rsi.sec.passkey));
        if (SL_STATUS_OK != status)
        {
            SILABS_LOG("wfx_rsi_do_join: RSI callback register join failed with status: %02x", status);
            return status;
        }

        sl_wifi_client_configuration_t ap = { 0 };
        uint32_t timeout_ms               = 0;

        ap.ssid.length = strlen(wfx_rsi.sec.ssid);
        memcpy(ap.ssid.value, (int8_t *) &wfx_rsi.sec.ssid[0], ap.ssid.length);
        ap.security      = connect_security_mode;
        ap.encryption    = SL_WIFI_NO_ENCRYPTION;
        ap.credential_id = id;
        if ((status = sl_wifi_connect(SL_WIFI_CLIENT_INTERFACE, &ap, timeout_ms)) == SL_STATUS_IN_PROGRESS)
        {
            callback_status = SL_STATUS_IN_PROGRESS;
            while (callback_status == SL_STATUS_IN_PROGRESS)
            {
                osThreadYield();
            }
            status = callback_status;
        }
        else
        {
            if (is_wifi_disconnection_event || wfx_rsi.join_retries <= WFX_RSI_CONFIG_MAX_JOIN)
            {
                SILABS_LOG("wfx_rsi_do_join: Wifi connect failed with status: %x", status);
                SILABS_LOG("wfx_rsi_do_join: starting JOIN to %s after %d tries\n", (char *) &wfx_rsi.sec.ssid[0],
                           wfx_rsi.join_retries);
                wfx_rsi.join_retries += 1;
                wfx_rsi.dev_state &= ~(WFX_RSI_ST_STA_CONNECTING | WFX_RSI_ST_STA_CONNECTED);
                wfx_retry_interval_handler(is_wifi_disconnection_event, wfx_rsi.join_retries);
                if (is_wifi_disconnection_event || wfx_rsi.join_retries <= MAX_JOIN_RETRIES_COUNT)
                {
                    event.eventType = WFX_EVT_STA_START_JOIN;
                    WfxPostEvent(&event);
                }
            }
        }
    }
    return status;
}

/// NotifyConnectivity
/// @brief Notify the application about the connectivity status if it has not been notified yet.
///        Helper function for HandleDHCPPolling.
void NotifyConnectivity()
{
    if (!hasNotifiedWifiConnectivity)
    {
        wfx_connected_notify(CONNECTION_STATUS_SUCCESS, &wfx_rsi.ap_mac);
        hasNotifiedWifiConnectivity = true;
    }
}

void HandleDHCPPolling()
{
    struct netif * sta_netif;
    WfxEvent_t event;

    sta_netif = wfx_get_netif(SL_WFX_STA_INTERFACE);
    if (sta_netif == NULL)
    {
        // TODO: Notify the application that the interface is not set up or Chipdie here because we are in an unkonwn state
        SILABS_LOG("HandleDHCPPolling: failed to get STA netif");
        return;
    }
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
    uint8_t dhcp_state = dhcpclient_poll(sta_netif);
    if (dhcp_state == DHCP_ADDRESS_ASSIGNED && !hasNotifiedIPV4)
    {
        wfx_dhcp_got_ipv4((uint32_t) sta_netif->ip_addr.u_addr.ip4.addr);
        hasNotifiedIPV4 = true;
        NotifyConnectivity();
    }
    else if (dhcp_state == DHCP_OFF)
    {
        wfx_ip_changed_notify(IP_STATUS_FAIL);
        hasNotifiedIPV4 = false;
    }
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
    /* Checks if the assigned IPv6 address is preferred by evaluating
     * the first block of IPv6 address ( block 0)
     */
    if ((ip6_addr_ispreferred(netif_ip6_addr_state(sta_netif, 0))) && !hasNotifiedIPV6)
    {
        wfx_ipv6_notify(GET_IPV6_SUCCESS);
        hasNotifiedIPV6 = true;
        event.eventType = WFX_EVT_STA_DHCP_DONE;
        WfxPostEvent(&event);
        NotifyConnectivity();
    }
}

void WfxPostEvent(WfxEvent_t * event)
{
    sl_status_t status = osMessageQueuePut(sWifiEventQueue, event, 0, 0);

    if (status != osOK)
    {
        SILABS_LOG("WfxPostEvent: failed to post event with status: %d", status);
        // TODO: Handle error, requeue event depending on queue size or notify relevant task, Chipdie, etc.
    }
}

/// ResetDHCPNotificationFlags
/// @brief Reset the flags that are used to notify the application about DHCP connectivity
///        and emits a WFX_EVT_STA_DO_DHCP event to trigger DHCP polling checks. Helper function for ProcessEvent.
void ResetDHCPNotificationFlags()
{
    WfxEvent_t outEvent;

#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
    hasNotifiedIPV4 = false;
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4
    hasNotifiedIPV6             = false;
    hasNotifiedWifiConnectivity = false;

    outEvent.eventType = WFX_EVT_STA_DO_DHCP;
    WfxPostEvent(&outEvent);
}

void ProcessEvent(WfxEvent_t inEvent)
{
    // Process event
    switch (inEvent.eventType)
    {
    case WFX_EVT_STA_CONN:
        SILABS_LOG("%s: starting LwIP STA", __func__);
        wfx_rsi.dev_state |= WFX_RSI_ST_STA_CONNECTED;
        ResetDHCPNotificationFlags();
        wfx_lwip_set_sta_link_up();
        /* We need to get AP Mac - TODO */
        // Uncomment once the hook into MATTER is moved to IP connectivty instead
        // of AP connectivity.
        // wfx_connected_notify(0, &wfx_rsi.ap_mac); // This
        // is independant of IP connectivity.
        break;
    case WFX_EVT_STA_DISCONN:
        // TODO: This event is not being posted anywhere, seems to be a dead code or we are missing something
        wfx_rsi.dev_state &=
            ~(WFX_RSI_ST_STA_READY | WFX_RSI_ST_STA_CONNECTING | WFX_RSI_ST_STA_CONNECTED | WFX_RSI_ST_STA_DHCP_DONE);
        SILABS_LOG("%s: disconnect notify", __func__);
        /* TODO: Implement disconnect notify */
        ResetDHCPNotificationFlags();
        wfx_lwip_set_sta_link_down(); // Internally dhcpclient_poll(netif) ->
                                      // wfx_ip_changed_notify(0) for IPV4
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
        wfx_ip_changed_notify(IP_STATUS_FAIL);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
        wfx_ipv6_notify(GET_IPV6_FAIL);
        break;
    case WFX_EVT_AP_START:
        // TODO: Currently unimplemented
        break;
    case WFX_EVT_AP_STOP:
        // TODO: Currently unimplemented
        break;
    case WFX_EVT_SCAN:
#ifdef SL_WFX_CONFIG_SCAN
        if (!(wfx_rsi.dev_state & WFX_RSI_ST_SCANSTARTED))
        {
            SILABS_LOG("%s: start SSID scan", __func__);
            sl_wifi_scan_configuration_t wifi_scan_configuration;
            memset(&wifi_scan_configuration, 0, sizeof(sl_wifi_scan_configuration_t));

            // TODO: Add scan logic
            sl_wifi_advanced_scan_configuration_t advanced_scan_configuration = { 0 };
            int32_t status;
            advanced_scan_configuration.active_channel_time  = ADV_ACTIVE_SCAN_DURATION;
            advanced_scan_configuration.passive_channel_time = ADV_PASSIVE_SCAN_DURATION;
            advanced_scan_configuration.trigger_level        = ADV_SCAN_THRESHOLD;
            advanced_scan_configuration.trigger_level_change = ADV_RSSI_TOLERANCE_THRESHOLD;
            advanced_scan_configuration.enable_multi_probe   = ADV_MULTIPROBE;
            status = sl_wifi_set_advanced_scan_configuration(&advanced_scan_configuration);
            if (SL_STATUS_OK != status)
            {
                // TODO: Seems like Chipdie should be called here, the device should be initialized here
                SILABS_LOG("Failed to set advanced scan configuration with status: %d", status);
                return;
            }

            if (wfx_rsi.dev_state & WFX_RSI_ST_STA_CONNECTED)
            {
                /* Terminate with end of scan which is no ap sent back */
                wifi_scan_configuration.type                   = SL_WIFI_SCAN_TYPE_ADV_SCAN;
                wifi_scan_configuration.periodic_scan_interval = ADV_SCAN_PERIODICITY;
            }
            else
            {
                wifi_scan_configuration = default_wifi_scan_configuration;
            }
            sl_wifi_set_scan_callback(bg_scan_callback_handler, NULL);
            scan_results_complete = false;
            wfx_rsi.dev_state |= WFX_RSI_ST_SCANSTARTED;
            status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, NULL, &wifi_scan_configuration);
            if (SL_STATUS_IN_PROGRESS == status)
            {
                osSemaphoreAcquire(sScanSemaphore, WIFI_SCAN_TIMEOUT_TICK);
            }
        }
        break;
#endif /* SL_WFX_CONFIG_SCAN */
    case WFX_EVT_STA_START_JOIN:
        // saving the AP related info
        wfx_rsi_save_ap_info();
        // Joining to the network
        wfx_rsi_do_join();
        break;
    case WFX_EVT_STA_DO_DHCP:
        StartDHCPTimer(WFX_RSI_DHCP_POLL_INTERVAL);
        break;
    case WFX_EVT_STA_DHCP_DONE:
        CancelDHCPTimer();
        break;
    case WFX_EVT_DHCP_POLL:
        HandleDHCPPolling();
    default:
        break;
    }
}

/*********************************************************************************
 * @fn  void wfx_rsi_task(void *arg)
 * @brief
 * The main WLAN task - started by wfx_wifi_start () that interfaces with RSI.
 * The rest of RSI stuff come in call-backs.
 * The initialization has been already done.
 * @param[in] arg:
 * @return
 *       None
 **********************************************************************************/
/* ARGSUSED */
void wfx_rsi_task(void * arg)
{
    EventBits_t flags;
    (void) arg;
    sl_status_t status = wfx_rsi_init();

    WfxEvent_t wfxEvent;
    if (status != RSI_SUCCESS)
    {
        SILABS_LOG("wfx_rsi_task: error: wfx_rsi_init with status: %02x", status);
        return;
    }
    wfx_lwip_start();
    wfx_started_notify();

    SILABS_LOG("wfx_rsi_task: starting event loop");
    for (;;)
    {
        status = osMessageQueueGet(sWifiEventQueue, &wfxEvent, NULL, osWaitForever);
        if (status == osOK)
        {
            ProcessEvent(wfxEvent);
        }
        else
        {
            // TODO: Everywhere in this file(and related) SILABS_LOG ---> Chiplog
            SILABS_LOG("Failed to get event with status: %x", status);
        }
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
/********************************************************************************************
 * @fn   void wfx_dhcp_got_ipv4(uint32_t ip)
 * @brief
 *        Acquire the new ip address
 * @param[in] ip: internet protocol
 * @return
 *        None
 **********************************************************************************************/
void wfx_dhcp_got_ipv4(uint32_t ip)
{
    /*
     * Acquire the new IP address
     */
    wfx_rsi.ip4_addr[0] = (ip) &HEX_VALUE_FF;
    wfx_rsi.ip4_addr[1] = (ip >> 8) & HEX_VALUE_FF;
    wfx_rsi.ip4_addr[2] = (ip >> 16) & HEX_VALUE_FF;
    wfx_rsi.ip4_addr[3] = (ip >> 24) & HEX_VALUE_FF;
    SILABS_LOG("%s: DHCP OK: IP=%d.%d.%d.%d", __func__, wfx_rsi.ip4_addr[0], wfx_rsi.ip4_addr[1], wfx_rsi.ip4_addr[2],
               wfx_rsi.ip4_addr[3]);
    /* Notify the Connectivity Manager - via the app */
    wfx_rsi.dev_state |= WFX_RSI_ST_STA_DHCP_DONE;
    wfx_ip_changed_notify(IP_STATUS_SUCCESS);
    wfx_rsi.dev_state |= WFX_RSI_ST_STA_READY;
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

/********************************************************************************************
 * @fn   void wfx_rsi_pkt_add_data(void *p, uint8_t *buf, uint16_t len, uint16_t off)
 * @brief
 *       add the data into packet
 * @param[in]  p:
 * @param[in]  buf:
 * @param[in]  len:
 * @param[in]  off:
 * @return
 *        None
 **********************************************************************************************/
void wfx_rsi_pkt_add_data(void * p, uint8_t * buf, uint16_t len, uint16_t off)
{
    sl_si91x_packet_t * pkt;
    pkt = (sl_si91x_packet_t *) p;
    memcpy(((char *) pkt->data) + off, buf, len);
}

#if !EXP_BOARD
/********************************************************************************************
 * @fn   int32_t wfx_rsi_send_data(void *p, uint16_t len)
 * @brief
 *       Driver send a data
 * @param[in]  p:
 * @param[in]  len:
 * @return
 *        None
 **********************************************************************************************/
int32_t wfx_rsi_send_data(void * p, uint16_t len)
{
    int32_t status;
    sl_wifi_buffer_t * buffer;
    buffer = (sl_wifi_buffer_t *) p;

    if (sl_si91x_driver_send_data_packet(SI91X_WLAN_CMD_QUEUE, buffer, RSI_SEND_RAW_DATA_RESPONSE_WAIT_TIME))
    {
        SILABS_LOG("*ERR*EN-RSI:Send fail");
        return ERR_IF;
    }
    return status;
}

#endif
