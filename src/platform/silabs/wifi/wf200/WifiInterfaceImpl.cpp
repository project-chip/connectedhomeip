/*
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

#include "FreeRTOS.h"
#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"
#include "event_groups.h"
#include "gpiointerrupt.h"
#include "sl_wfx_cmd_api.h"
#include "sl_wfx_constants.h"
#include "task.h"
#include <app/icd/server/ICDServerConfig.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/silabs/wifi/lwip-support/dhcp_client.h>
#include <platform/silabs/wifi/lwip-support/ethernetif.h>
#include <platform/silabs/wifi/lwip-support/lwip_netif.h>
#include <platform/silabs/wifi/wf200/WifiInterfaceImpl.h>
#include <platform/silabs/wifi/wf200/ncp/efr_spi.h>
#include <platform/silabs/wifi/wf200/ncp/sl_wfx_board.h>
#include <platform/silabs/wifi/wf200/ncp/sl_wfx_host.h>
#include <platform/silabs/wifi/wf200/ncp/sl_wfx_task.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;

// TODO: This is a workaround because we depend on the platform lib which depends on the platform implementation.
//       As such we can't depend on the platform here as well
extern void HandleWFXSystemEvent(sl_wfx_generic_message_t * eventData);

/* wfxRsi Task will use as its stack */
StackType_t wfxEventTaskStack[1024] = { 0 };

/* Structure that will hold the TCB of the wfxRsi Task being created. */
StaticTask_t wfxEventTaskBuffer;

/* Declare a variable to hold the data associated with the created event group. */
StaticEventGroup_t wfxEventGroup;

EventGroupHandle_t sl_wfx_event_group;
TaskHandle_t wfx_events_task_handle;
static WifiInterface::MacAddress ap_mac;
#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
static uint32_t sta_ip;
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
static wfx_wifi_scan_result_t ap_info;

// Set Scan Parameters
#define ACTIVE_CHANNEL_TIME 110
#define PASSIVE_CHANNEL_TIME 0
#define NUM_PROBE_REQUEST 2

/* Wi-Fi bitmask events - for the task */
#define SL_WFX_CONNECT (1 << 1)
#define SL_WFX_DISCONNECT (1 << 2)
#define SL_WFX_START_AP (1 << 3)
#define SL_WFX_STOP_AP (1 << 4)
#define SL_WFX_SCAN_START (1 << 5)
#define SL_WFX_SCAN_COMPLETE (1 << 6)
#define SL_WFX_RETRY_CONNECT (1 << 7)

#define WLAN_TASK_STACK_SIZE (1024)
#define ETH_FRAME (0)
#define AP_START_SUCCESS (0)
#define BITS_TO_WAIT (0)
#define BEACON_1 (0)
#define CHANNEL_LIST ((const uint8_t *) 0)
#define CHANNEL_COUNT (0)
#define IE_DATA ((const uint8_t *) 0)
#define IE_DATA_LENGTH (0)
#define BSSID_SCAN ((const uint8_t *) 0)
#define CHANNEL_0 (0)
#define PREVENT_ROAMING (1)
#define DISABLE_PMF_MODE (0)
#define STA_IP_FAIL (0)
#define WLAN_TASK_PRIORITY (1)

#ifdef SL_WFX_CONFIG_SOFTAP
// Connection parameters
char softap_ssid[32]                   = SOFTAP_SSID_DEFAULT;
char softap_passkey[64]                = SOFTAP_PASSKEY_DEFAULT;
sl_wfx_security_mode_t softap_security = SOFTAP_SECURITY_DEFAULT;
uint8_t softap_channel                 = SOFTAP_CHANNEL_DEFAULT;
#endif

/* station network interface structures */
struct netif * sta_netif;
WifiInterface::WifiCredentials wifi_provision;
#define PUT_COUNTER(name) ChipLogDetail(DeviceLayer, "%-24s %lu", #name, (unsigned long) counters->body.count_##name);

bool hasNotifiedWifiConnectivity = false;

static struct scan_result_holder
{
    struct scan_result_holder * next;
    wfx_wifi_scan_result scan;
} * scan_save;

static uint8_t scan_count = 0;
static ScanCallback scan_cb;              /* user-callback - when scan is done */
static uint8_t * scan_ssid     = nullptr; /* Which one are we scanning for */
static size_t scan_ssid_length = 0;
static void sl_wfx_scan_result_callback(sl_wfx_scan_result_ind_body_t * scan_result);
static void sl_wfx_scan_complete_callback(uint32_t status);

static void sl_wfx_generic_status_callback(sl_wfx_generic_ind_t * frame);

#ifdef SL_WFX_CONFIG_SOFTAP
static void sl_wfx_start_ap_callback(uint32_t status);
static void sl_wfx_stop_ap_callback(void);
static void sl_wfx_client_connected_callback(uint8_t * mac);
static void sl_wfx_ap_client_disconnected_callback(uint32_t status, uint8_t * mac);
static void sl_wfx_ap_client_rejected_callback(uint32_t status, uint8_t * mac);
#endif

extern uint32_t gOverrunCount;

namespace {

// wfx_fmac_driver context
sl_wfx_context_t wifiContext;
chip::BitFlags<WifiInterface::WifiState> wifi_extra;

typedef struct __attribute__((__packed__)) sl_wfx_get_counters_cnf_body_s
{
    uint32_t status;
    uint16_t mib_id;
    uint16_t length;
    uint32_t rcpi;
    uint32_t count_plcp_errors;
    uint32_t count_fcs_errors;
    uint32_t count_tx_packets;
    uint32_t count_rx_packets;
    uint32_t count_rx_packet_errors;
    uint32_t count_rx_decryption_failures;
    uint32_t count_rx_mic_failures;
    uint32_t count_rx_no_key_failures;
    uint32_t count_tx_multicast_frames;
    uint32_t count_tx_frames_success;
    uint32_t count_tx_frame_failures;
    uint32_t count_tx_frames_retried;
    uint32_t count_tx_frames_multi_retried;
    uint32_t count_rx_frame_duplicates;
    uint32_t count_rts_success;
    uint32_t count_rts_failures;
    uint32_t count_ack_failures;
    uint32_t count_rx_multicast_frames;
    uint32_t count_rx_frames_success;
    uint32_t count_rx_cmacicv_errors;
    uint32_t count_rx_cmac_replays;
    uint32_t count_rx_mgmt_ccmp_replays;
    uint32_t count_rx_bipmic_errors;
    uint32_t count_rx_beacon;
    uint32_t count_miss_beacon;
    uint32_t reserved[15];
} sl_wfx_get_counters_cnf_body_t;

typedef struct __attribute__((__packed__)) sl_wfx_get_counters_cnf_s
{
    /** Common message header. */
    sl_wfx_header_t header;
    /** Confirmation message body. */
    sl_wfx_get_counters_cnf_body_t body;
} sl_wfx_get_counters_cnf_t;

typedef struct __attribute__((__packed__)) sl_wfx_mib_req_body_s
{
    uint16_t mib_id; ///< ID of the MIB to be read.
    uint16_t reserved;
} sl_wfx_mib_req_body_t;

typedef struct __attribute__((__packed__)) sl_wfx_header_mib_s
{
    uint16_t length; ///< Message length in bytes including this uint16_t.
                     ///< Maximum value is 8188 but maximum Request size is FW dependent and reported in the
                     ///< ::sl_wfx_startup_ind_body_t::size_inp_ch_buf.
    uint8_t id;      ///< Contains the message Id indexed by sl_wfx_general_commands_ids_t or sl_wfx_message_ids_t.
    uint8_t reserved : 1;
    uint8_t interface : 2;
    uint8_t seqnum : 3;
    uint8_t encrypted : 2;
} sl_wfx_header_mib_t;

typedef struct __attribute__((__packed__)) sl_wfx_mib_req_s
{
    /** Common message header. */
    sl_wfx_header_mib_t header;
    /** Request message body. */
    sl_wfx_mib_req_body_t body;
} sl_wfx_mib_req_t;

sl_wfx_get_counters_cnf_t * counters;

sl_status_t get_all_counters(void)
{
    sl_status_t result;
    uint8_t command_id         = 0x05;
    uint16_t mib_id            = 0x2035;
    sl_wfx_mib_req_t * request = nullptr;
    uint32_t request_length    = SL_WFX_ROUND_UP_EVEN(sizeof(sl_wfx_header_mib_t) + sizeof(sl_wfx_mib_req_body_t));

    result =
        sl_wfx_allocate_command_buffer((sl_wfx_generic_message_t **) &request, command_id, SL_WFX_CONTROL_BUFFER, request_length);

    VerifyOrReturnError(request != nullptr, SL_STATUS_NULL_POINTER);

    request->body.mib_id      = mib_id;
    request->header.interface = 0x2;
    request->header.encrypted = 0x0;

    result = sl_wfx_send_request(command_id, (sl_wfx_generic_message_t *) request, request_length);
    SL_WFX_ERROR_CHECK(result);

    result = sl_wfx_host_wait_for_confirmation(command_id, SL_WFX_DEFAULT_REQUEST_TIMEOUT_MS, (void **) &counters);
    SL_WFX_ERROR_CHECK(result);

    ChipLogDetail(DeviceLayer, "%-24s %12s ", "", "Debug Counters Content");
    ChipLogDetail(DeviceLayer, "%-24s %lu", "rcpi", (unsigned long) counters->body.rcpi);
    PUT_COUNTER(plcp_errors);
    PUT_COUNTER(fcs_errors);
    PUT_COUNTER(tx_packets);
    PUT_COUNTER(rx_packets);
    PUT_COUNTER(rx_packet_errors);
    PUT_COUNTER(rx_decryption_failures);
    PUT_COUNTER(rx_mic_failures);
    PUT_COUNTER(rx_no_key_failures);
    PUT_COUNTER(tx_multicast_frames);
    PUT_COUNTER(tx_frames_success);
    PUT_COUNTER(tx_frame_failures);
    PUT_COUNTER(tx_frames_retried);
    PUT_COUNTER(tx_frames_multi_retried);
    PUT_COUNTER(rx_frame_duplicates);
    PUT_COUNTER(rts_success);
    PUT_COUNTER(rts_failures);
    PUT_COUNTER(ack_failures);
    PUT_COUNTER(rx_multicast_frames);
    PUT_COUNTER(rx_frames_success);
    PUT_COUNTER(rx_cmacicv_errors);
    PUT_COUNTER(rx_cmac_replays);
    PUT_COUNTER(rx_mgmt_ccmp_replays);
    PUT_COUNTER(rx_bipmic_errors);
    PUT_COUNTER(rx_beacon);
    PUT_COUNTER(miss_beacon);

error_handler:

    if (result == SL_STATUS_TIMEOUT)
    {
        if (sl_wfx_context->used_buffers > 0)
        {
            sl_wfx_context->used_buffers--;
        }
    }
    if (request != nullptr)
    {
        sl_wfx_free_command_buffer((sl_wfx_generic_message_t *) request, command_id, SL_WFX_CONTROL_BUFFER);
    }

    return result;
}

/**
 * @brief Convert RCPI to RSSI

 * This function converts the Received Channel Power Indicator (RCPI) to
 * the Received Signal Strength Indicator (RSSI). If the result of the
 * conversion exceeds the range of a int16_t, it will be clamped to the maximum
 * or minimum value of int16_t.

 * @param[in]  rcpi: Received Channel Power Indicator value

 * @return RSSI value
 */
inline int16_t ConvertRcpiToRssi(uint32_t rcpi)
{
    int64_t rssi = (rcpi / 2) - 110;
    // Checking for overflows
    VerifyOrReturnValue(rssi <= std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::max());
    VerifyOrReturnValue(rssi >= std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::min());
    return rssi;
}

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
/**
 * @brief Updates the IPv4 address in the Wi-Fi interface and notifies the application layer about the new IP address.
 *
 * @param[in] ip New IPv4 address
 */
void GotIPv4Address(uint32_t ip)
{
    ChipLogDetail(DeviceLayer, "DHCP IP=%d.%d.%d.%d", (ip & 0xFF), (ip >> 8 & 0xFF), (ip >> 16 & 0xFF), (ip >> 24 & 0xFF));
    sta_ip = ip;

    NotifyIPv4Change(true);
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

} // namespace

/****************************************************************************
 * @brief
 * Called when the driver needs to post an event
 * @param[in]  event_payload:
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_FAIL otherwise
 *****************************************************************************/
extern "C" sl_status_t sl_wfx_host_process_event(sl_wfx_generic_message_t * event_payload)
{
    switch (event_payload->header.id)
    {
    /******** INDICATION ********/
    case SL_WFX_STARTUP_IND_ID: {
        ChipLogProgress(DeviceLayer, "startup completed.");
        HandleWFXSystemEvent(event_payload);
        break;
    }
    case SL_WFX_CONNECT_IND_ID: {
        sl_wfx_connect_ind_t * connect_indication = (sl_wfx_connect_ind_t *) event_payload;
        WifiInterfaceImpl::GetInstance().ConnectionEventCallback(connect_indication->body);
        break;
    }
    case SL_WFX_DISCONNECT_IND_ID: {
        sl_wfx_disconnect_ind_t * disconnect_indication = (sl_wfx_disconnect_ind_t *) event_payload;
        WifiInterfaceImpl::GetInstance().DisconnectionEventCallback(disconnect_indication->body.mac,
                                                                    disconnect_indication->body.reason);
        break;
    }
    case SL_WFX_RECEIVED_IND_ID: {
        sl_wfx_received_ind_t * ethernet_frame = (sl_wfx_received_ind_t *) event_payload;
        if (ethernet_frame->body.frame_type == ETH_FRAME)
        {
            sl_wfx_host_received_frame_callback(ethernet_frame);
        }
        break;
    }
    case SL_WFX_SCAN_RESULT_IND_ID: {
        sl_wfx_scan_result_ind_t * scan_result = (sl_wfx_scan_result_ind_t *) event_payload;
        sl_wfx_scan_result_callback(&scan_result->body);
        break;
    }
    case SL_WFX_SCAN_COMPLETE_IND_ID: {
        sl_wfx_scan_complete_ind_t * scan_complete = (sl_wfx_scan_complete_ind_t *) event_payload;
        sl_wfx_scan_complete_callback(scan_complete->body.status);
        break;
    }
#ifdef SL_WFX_CONFIG_SOFTAP
    case SL_WFX_START_AP_IND_ID: {
        sl_wfx_start_ap_ind_t * start_ap_indication = (sl_wfx_start_ap_ind_t *) event_payload;
        sl_wfx_start_ap_callback(start_ap_indication->body.status);
        break;
    }
    case SL_WFX_STOP_AP_IND_ID: {
        sl_wfx_stop_ap_callback();
        break;
    }
    case SL_WFX_AP_CLIENT_CONNECTED_IND_ID: {
        sl_wfx_ap_client_connected_ind_t * client_connected_indication = (sl_wfx_ap_client_connected_ind_t *) event_payload;
        sl_wfx_client_connected_callback(client_connected_indication->body.mac);
        break;
    }
    case SL_WFX_AP_CLIENT_REJECTED_IND_ID: {
        sl_wfx_ap_client_rejected_ind_t * ap_client_rejected_indication = (sl_wfx_ap_client_rejected_ind_t *) event_payload;
        sl_wfx_ap_client_rejected_callback(ap_client_rejected_indication->body.reason, ap_client_rejected_indication->body.mac);
        break;
    }
    case SL_WFX_AP_CLIENT_DISCONNECTED_IND_ID: {
        sl_wfx_ap_client_disconnected_ind_t * ap_client_disconnected_indication =
            (sl_wfx_ap_client_disconnected_ind_t *) event_payload;
        sl_wfx_ap_client_disconnected_callback(ap_client_disconnected_indication->body.reason,
                                               ap_client_disconnected_indication->body.mac);
        break;
    }
#endif /* SL_WFX_CONFIG_SOFTAP */
#ifdef SL_WFX_USE_SECURE_LINK
    case SL_WFX_SECURELINK_EXCHANGE_PUB_KEYS_IND_ID: {
        if (host_context.waited_event_id != SL_WFX_SECURELINK_EXCHANGE_PUB_KEYS_IND_ID)
        {
            memcpy((void *) &sl_wfx_context->secure_link_exchange_ind, (void *) event_payload, event_payload->header.length);
        }
        break;
    }
#endif
    case SL_WFX_GENERIC_IND_ID: {
        sl_wfx_generic_ind_t * generic_status = (sl_wfx_generic_ind_t *) event_payload;
        sl_wfx_generic_status_callback(generic_status);
        break;
    }
    case SL_WFX_EXCEPTION_IND_ID: {
        sl_wfx_exception_ind_t * firmware_exception = (sl_wfx_exception_ind_t *) event_payload;
        ChipLogError(DeviceLayer, "event: SL_WFX_EXCEPTION_IND_ID");
        ChipLogError(DeviceLayer, "firmware_exception->header.length: %d", firmware_exception->header.length);
        // create a bytespan header.length with exception payload
        ByteSpan exception_byte_span = ByteSpan((uint8_t *) firmware_exception, firmware_exception->header.length);
        ChipLogByteSpan(DeviceLayer, exception_byte_span);
        break;
    }
    case SL_WFX_ERROR_IND_ID: {
        sl_wfx_error_ind_t * firmware_error = (sl_wfx_error_ind_t *) event_payload;
        ChipLogError(DeviceLayer, "event: SL_WFX_ERROR_IND_ID");
        ChipLogError(DeviceLayer, "firmware_error->type: %lu", firmware_error->body.type);
        ChipLogError(DeviceLayer, "firmware_error->header.length: %d", firmware_error->header.length);
        // create a bytespan header.length with error payload
        ByteSpan error_byte_span = ByteSpan((uint8_t *) firmware_error, firmware_error->header.length);
        ChipLogByteSpan(DeviceLayer, error_byte_span);
        break;
    }
    }

    return SL_STATUS_OK;
}

/****************************************************************************
 * @brief
 * Callback for individual scan result
 * @param[in] scan_result: Scan result of all SSID's
 *****************************************************************************/
static void sl_wfx_scan_result_callback(sl_wfx_scan_result_ind_body_t * scan_result)
{

    ChipLogDetail(DeviceLayer, "# %2d %2d  %03d %02X:%02X:%02X:%02X:%02X:%02X  %s", scan_count, scan_result->channel,
                  (ConvertRcpiToRssi(scan_result->rcpi)), scan_result->mac[0], scan_result->mac[1], scan_result->mac[2],
                  scan_result->mac[3], scan_result->mac[4], scan_result->mac[5], scan_result->ssid_def.ssid);

    chip::ByteSpan requestedSsid(scan_ssid, scan_ssid_length);
    chip::ByteSpan scannedSsid(scan_result->ssid_def.ssid, scan_result->ssid_def.ssid_length);

    // Verify that there was no requested SSID or that the SSID matches the requested SSID
    VerifyOrReturn(requestedSsid.empty() || requestedSsid.data_equal(scannedSsid));

    struct scan_result_holder * ap = reinterpret_cast<struct scan_result_holder *>(chip::Platform::MemoryAlloc(sizeof(*ap)));
    VerifyOrReturn(ap != nullptr, ChipLogError(DeviceLayer, "Scan Callback: No Memory for scanned network."));

    // Add Scan to the linked list
    ap->next  = scan_save;
    scan_save = ap;

    // Copy scanned SSID to the output buffer
    chip::MutableByteSpan outputSsid(ap->scan.ssid, WFX_MAX_SSID_LENGTH);
    chip::CopySpanToMutableSpan(scannedSsid, outputSsid);
    ap->scan.ssid_length = outputSsid.size();

    // Set Network Security - We start by WPA3 to set the most secure type
    ap->scan.security = WFX_SEC_UNSPECIFIED;
    if (scan_result->security_mode.wpa3)
    {
        ap->scan.security = WFX_SEC_WPA3;
    }
    else if (scan_result->security_mode.wpa2)
    {
        ap->scan.security = WFX_SEC_WPA2;
    }
    else if (scan_result->security_mode.wpa)
    {
        ap->scan.security = WFX_SEC_WPA;
    }
    else if (scan_result->security_mode.wep)
    {
        ap->scan.security = WFX_SEC_WEP;
    }
    else
    {
        ap->scan.security = WFX_SEC_NONE;
    }

    ap->scan.chan = scan_result->channel;
    ap->scan.rssi = ConvertRcpiToRssi(scan_result->rcpi);

    chip::ByteSpan scannedBssid(scan_result->mac, kWifiMacAddressLength);
    chip::MutableByteSpan outputBssid(ap->scan.bssid, kWifiMacAddressLength);
    chip::CopySpanToMutableSpan(scannedBssid, outputBssid);

    scan_count++;
}

/****************************************************************************
 * @brief
 * Callback for scan complete
 * @param[in] status:Status of WLAN scan api
 *****************************************************************************/
/* ARGSUSED */
static void sl_wfx_scan_complete_callback(uint32_t status)
{
    (void) (status);
    /* Use scan_count value and reset it */
    xEventGroupSetBits(sl_wfx_event_group, SL_WFX_SCAN_COMPLETE);
}

#ifdef SL_WFX_CONFIG_SOFTAP
/****************************************************************************
 * @brief
 * Callback for AP started
 * @param[in]  status: Status of wfx start ap api
 *****************************************************************************/
static void sl_wfx_start_ap_callback(uint32_t status)
{
    VerifyOrReturnLogError(status == AP_START_SUCCESS, CHIP_ERROR_INTERNAL);
    wifi_extra.Set(WifiInterface::WifiState::kAPReady);

    xEventGroupSetBits(sl_wfx_event_group, SL_WFX_START_AP);
}

/****************************************************************************
 * @brief
 * Callback for AP stopped
 *****************************************************************************/
static void sl_wfx_stop_ap_callback(void)
{
    // TODO
    // dhcpserver_clear_stored_mac();
    ChipLogProgress(DeviceLayer, "SoftAP stopped");
    wifi_extra.Clear(WifiInterface::WifiState::kAPReady);

    xEventGroupSetBits(sl_wfx_event_group, SL_WFX_STOP_AP);
}

/****************************************************************************
 * @brief
 * Callback for client connect to AP
 * @param[in]  mac: MAC adress of device
 *****************************************************************************/
static void sl_wfx_client_connected_callback(uint8_t * mac)
{
    ChipLogProgress(DeviceLayer, "Client connected, MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4],
                    mac[5]);
}

/****************************************************************************
 * @brief
 * Callback for client rejected from AP
 * @param[in] status: Status of ap rejected
 * @param[in] mac: MAC adress of device
 *****************************************************************************/
static void sl_wfx_ap_client_rejected_callback(uint32_t status, uint8_t * mac)
{
    ChipLogError(DeviceLayer, "Client rejected, reason: %d, MAC: %02X:%02X:%02X:%02X:%02X:%02X", (int) status, mac[0], mac[1],
                 mac[2], mac[3], mac[4], mac[5]);
}

/****************************************************************************
 * @brief
 * Callback for AP client disconnect
 * @param[in] status: Status of ap dissconnect
 * @param[in]  mac:
 *****************************************************************************/
static void sl_wfx_ap_client_disconnected_callback(uint32_t status, uint8_t * mac)
{
    ChipLogError(DeviceLayer, "Client disconnected, reason: %d, MAC: %02X:%02X:%02X:%02X:%02X:%02X", (int) status, mac[0], mac[1],
                 mac[2], mac[3], mac[4], mac[5]);
}
#endif /* SL_WFX_CONFIG_SOFTAP */

/****************************************************************************
 * @brief
 * Callback for generic status received
 * @param[in] farme:
 *****************************************************************************/
static void sl_wfx_generic_status_callback(sl_wfx_generic_ind_t * frame)
{
    (void) (frame);
}

namespace chip {
namespace DeviceLayer {
namespace Silabs {

WifiInterfaceImpl WifiInterfaceImpl::mInstance;

WifiInterface & WifiInterface::GetInstance()
{
    return WifiInterfaceImpl::GetInstance();
}

CHIP_ERROR WifiInterfaceImpl::InitWiFiStack()
{
    // TODO: This function should include sl_wfx_hw_init() and sl_wfx_init() functions. Only done now to make MatterConfig platform
    // agnostic. (MATTER-4680)
    // Start wfx bus communication task.
    sl_status_t status = wfx_bus_start();
    VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_NO_MEMORY,
                        ChipLogError(DeviceLayer, "wfx_bus_start failed: %lx", status));
    return CHIP_NO_ERROR;
}

CHIP_ERROR WifiInterfaceImpl::GetMacAddress(sl_wfx_interface_t interface, MutableByteSpan & address)
{
    VerifyOrReturnError(address.size() >= kWifiMacAddressLength, CHIP_ERROR_BUFFER_TOO_SMALL);

#ifdef SL_WFX_CONFIG_SOFTAP
    chip::ByteSpan byteSpan((interface == SL_WFX_SOFTAP_INTERFACE) ? wifiContext.mac_addr_1.octet : wifiContext.mac_addr_0.octet);
#else
    chip::ByteSpan byteSpan(wifiContext.mac_addr_0.octet);
#endif

    return CopySpanToMutableSpan(byteSpan, address);
}

CHIP_ERROR WifiInterfaceImpl::StartNetworkScan(chip::ByteSpan ssid, ScanCallback callback)
{
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // SSID Max Length that is supported by the Wi-Fi SDK is 32
    VerifyOrReturnError(ssid.size() <= WFX_MAX_SSID_LENGTH, CHIP_ERROR_INVALID_STRING_LENGTH);

    // Make sure memory is cleared before starting a new scan
    if (scan_ssid)
    {
        chip::Platform::MemoryFree(scan_ssid);
        scan_ssid = nullptr;
    }

    if (ssid.empty())
    {
        scan_ssid_length = 0;
        scan_ssid        = nullptr;
    }
    else
    {
        scan_ssid_length = ssid.size();
        scan_ssid        = reinterpret_cast<uint8_t *>(chip::Platform::MemoryAlloc(scan_ssid_length));
        VerifyOrReturnError(scan_ssid != nullptr, CHIP_ERROR_NO_MEMORY);

        chip::MutableByteSpan scannedSsidSpan(scan_ssid, WFX_MAX_SSID_LENGTH);
        chip::CopySpanToMutableSpan(ssid, scannedSsidSpan);
    }
    scan_cb = callback;

    xEventGroupSetBits(sl_wfx_event_group, SL_WFX_SCAN_START);
    return CHIP_NO_ERROR;
}

CHIP_ERROR WifiInterfaceImpl::StartWifiTask()
{
    if (wifi_extra.Has(WifiInterface::WifiState::kStationInit))
    {
        ChipLogDetail(DeviceLayer, "WIFI: Already started");
        return CHIP_NO_ERROR;
    }
    wifi_extra.Set(WifiInterface::WifiState::kStationInit);

    VerifyOrReturnError(wfx_soft_init() == SL_STATUS_OK, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "Failed to execute the WFX software init."));
    VerifyOrReturnError(InitWf200Platform() == SL_STATUS_OK, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "Failed to execute the WFX HW start."));

    return CHIP_NO_ERROR;
}

void WifiInterfaceImpl::ConfigureStationMode()
{
    wifi_extra.Set(WifiInterface::WifiState::kStationMode);
}

bool WifiInterfaceImpl::IsStationModeEnabled(void)
{
    return wifi_extra.Has(WifiInterface::WifiState::kStationMode);
}

bool WifiInterfaceImpl::IsStationConnected()
{
    return wifi_extra.Has(WifiInterface::WifiState::kStationConnected);
}

bool WifiInterfaceImpl::IsStationReady()
{
    return wifi_extra.Has(WifiInterface::WifiState::kStationInit);
}

CHIP_ERROR WifiInterfaceImpl::TriggerDisconnection(void)
{
    ChipLogProgress(DeviceLayer, "STA-Disconnecting");

    sl_status_t status = sl_wfx_send_disconnect_command();
    VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_INTERNAL);

    wifi_extra.Clear(WifiInterface::WifiState::kStationConnected);

    xEventGroupSetBits(sl_wfx_event_group, SL_WFX_RETRY_CONNECT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR WifiInterfaceImpl::GetAccessPointInfo(wfx_wifi_scan_result_t & info)
{
    uint32_t signal_strength = 0;

    // TODO: The ap_info.ssid isn't populated anywhere. The returned value is always 0.
    chip::ByteSpan apSsidSpan(ap_info.ssid, ap_info.ssid_length);
    chip::MutableByteSpan apSsidMutableSpan(info.ssid, WFX_MAX_SSID_LENGTH);
    chip::CopySpanToMutableSpan(apSsidSpan, apSsidMutableSpan);
    info.ssid_length = apSsidMutableSpan.size();

    // TODO: The ap_info.bssid isn't populated anywhere. The returned value is always 0.
    chip::ByteSpan apBssidSpan(ap_info.bssid, kWifiMacAddressLength);
    chip::MutableByteSpan apBssidMutableSpan(info.bssid, kWifiMacAddressLength);
    chip::CopySpanToMutableSpan(apBssidSpan, apBssidMutableSpan);

    info.security = ap_info.security;
    info.chan     = ap_info.chan;

    sl_status_t status = sl_wfx_get_signal_strength(&signal_strength);
    VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_INTERNAL);

    info.rssi = ConvertRcpiToRssi(signal_strength);

    ChipLogDetail(DeviceLayer, "WIFI:SSID     : %s", ap_info.ssid);
    ChipLogDetail(DeviceLayer, "WIFI:BSSID    : %02x:%02x:%02x:%02x:%02x:%02x", ap_info.bssid[0], ap_info.bssid[1],
                  ap_info.bssid[2], ap_info.bssid[3], ap_info.bssid[4], ap_info.bssid[5]);
    ChipLogDetail(DeviceLayer, "WIFI:security : %d", info.security);
    ChipLogDetail(DeviceLayer, "WIFI:channel  :  %d", info.chan);
    ChipLogDetail(DeviceLayer, "signal_strength: %ld", signal_strength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WifiInterfaceImpl::GetAccessPointExtendedInfo(wfx_wifi_scan_ext_t & info)
{
    sl_status_t status = get_all_counters();
    VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_INTERNAL, ChipLogError(DeviceLayer, "Failed to get the couters"));

    info.beacon_lost_count = counters->body.count_miss_beacon;
    info.beacon_rx_count   = counters->body.count_rx_beacon;
    info.mcast_rx_count    = counters->body.count_rx_multicast_frames;
    info.mcast_tx_count    = counters->body.count_tx_multicast_frames;
    info.ucast_rx_count    = counters->body.count_rx_packets;
    info.ucast_tx_count    = counters->body.count_tx_packets;
    info.overrun_count     = gOverrunCount;

    return CHIP_NO_ERROR;
}

#if CHIP_CONFIG_ENABLE_ICD_SERVER
CHIP_ERROR WifiInterfaceImpl::ConfigurePowerSave()
{
    // TODO: Implement Power save configuration. We do a silent failure to avoid causing problems in higher layers.
    return CHIP_NO_ERROR;
}

CHIP_ERROR WifiInterfaceImpl::ConfigureBroadcastFilter(bool enableBroadcastFilter)
{
    // TODO: Implement Broadcast filtering. We do a silent failure to avoid causing problems in higher layers.
    return CHIP_NO_ERROR;
}
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

CHIP_ERROR WifiInterfaceImpl::ResetCounters()
{
    // TODO: Implement the function
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

void WifiInterfaceImpl::ClearWifiCredentials()
{
    wifi_provision.Clear();
}

CHIP_ERROR WifiInterfaceImpl::GetWifiCredentials(WifiCredentials & credentials)
{
    VerifyOrReturnError(IsWifiProvisioned(), CHIP_ERROR_INCORRECT_STATE);
    credentials = wifi_provision;

    return CHIP_NO_ERROR;
}

bool WifiInterfaceImpl::IsWifiProvisioned()
{
    // TODO: We need a better way of checking if we are provisioned or not
    return wifi_provision.ssid[0] != 0;
}

void WifiInterfaceImpl::SetWifiCredentials(const WifiCredentials & credentials)
{
    wifi_provision = credentials;
}

CHIP_ERROR WifiInterfaceImpl::ConnectToAccessPoint(void)
{
    sl_wfx_security_mode_t connect_security_mode;

    VerifyOrReturnError(IsWifiProvisioned(), CHIP_ERROR_INCORRECT_STATE);
    ChipLogDetail(DeviceLayer, "WIFI:JOIN to %s", wifi_provision.ssid);

    ChipLogDetail(DeviceLayer,
                  "WIFI Scan Paramter set to Active channel time %d, Passive Channel "
                  "Time: %d, Number of prob: %d",
                  ACTIVE_CHANNEL_TIME, PASSIVE_CHANNEL_TIME, NUM_PROBE_REQUEST);
    (void) sl_wfx_set_scan_parameters(ACTIVE_CHANNEL_TIME, PASSIVE_CHANNEL_TIME, NUM_PROBE_REQUEST);
    switch (wifi_provision.security)
    {
    case WFX_SEC_WEP:
        connect_security_mode = sl_wfx_security_mode_e::WFM_SECURITY_MODE_WEP;
        break;
    case WFX_SEC_WPA:
    case WFX_SEC_WPA2:
        connect_security_mode = sl_wfx_security_mode_e::WFM_SECURITY_MODE_WPA2_WPA1_PSK;
        break;
    case WFX_SEC_WPA3:
        connect_security_mode = sl_wfx_security_mode_e::WFM_SECURITY_MODE_WPA3_SAE;
        break;
    case WFX_SEC_NONE:
        connect_security_mode = sl_wfx_security_mode_e::WFM_SECURITY_MODE_OPEN;
        break;
    default:
        ChipLogError(DeviceLayer, "error: unknown security type.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    VerifyOrReturnError(sl_wfx_send_join_command(wifi_provision.ssid, wifi_provision.ssidLength, NULL, CHANNEL_0,
                                                 connect_security_mode, PREVENT_ROAMING, DISABLE_PMF_MODE, wifi_provision.passkey,
                                                 wifi_provision.passkeyLength, NULL, IE_DATA_LENGTH) == SL_STATUS_OK,
                        CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
bool WifiInterfaceImpl::HasAnIPv4Address()
{
    return (sta_ip == STA_IP_FAIL) ? false : true;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4

bool WifiInterfaceImpl::HasAnIPv6Address()
{
    return IsStationConnected();
}

void WifiInterfaceImpl::CancelScanNetworks()
{
    struct scan_result_holder *hp, *next;
    /* Not possible */
    VerifyOrReturn(scan_cb != nullptr);
    sl_wfx_send_stop_scan_command();
    for (hp = scan_save; hp; hp = next)
    {
        next = hp->next;
        chip::Platform::MemoryFree(hp);
    }
    scan_save  = (struct scan_result_holder *) 0;
    scan_count = 0;
    if (scan_ssid)
    {
        chip::Platform::MemoryFree(scan_ssid);
        scan_ssid = nullptr;
    }
    scan_cb = nullptr;
}

void WifiInterfaceImpl::ConnectionEventCallback(sl_wfx_connect_ind_body_t connect_indication_body)
{
    uint8_t * mac   = connect_indication_body.mac;
    uint32_t status = connect_indication_body.status;
    ap_info.chan    = connect_indication_body.channel;
    memcpy(&ap_info.security, &wifi_provision.security, sizeof(wifi_provision.security));
    switch (status)
    {
    case WFM_STATUS_SUCCESS: {
        ChipLogProgress(DeviceLayer, "STA-Connected");
        memcpy(ap_mac.data(), mac, kWifiMacAddressLength);

        wifi_extra.Set(WifiInterface::WifiState::kStationConnected);
        xEventGroupSetBits(sl_wfx_event_group, SL_WFX_CONNECT);
        break;
    }
    case WFM_STATUS_NO_MATCHING_AP: {
        ChipLogError(DeviceLayer, "Connection failed, access point not found");
        break;
    }
    case WFM_STATUS_CONNECTION_ABORTED: {
        ChipLogError(DeviceLayer, "Connection aborted");
        break;
    }
    case WFM_STATUS_CONNECTION_TIMEOUT: {
        ChipLogError(DeviceLayer, "Connection timeout");
        break;
    }
    case WFM_STATUS_CONNECTION_REJECTED_BY_AP: {
        ChipLogError(DeviceLayer, "Connection rejected by the access point");
        break;
    }
    case WFM_STATUS_CONNECTION_AUTH_FAILURE: {
        ChipLogError(DeviceLayer, "Connection authentication failure");
        break;
    }
    default: {
        ChipLogError(DeviceLayer, "Connection attempt error");
    }
    }

    if (status != WFM_STATUS_SUCCESS)
    {
        ScheduleConnectionAttempt();
    }
}

void WifiInterfaceImpl::DisconnectionEventCallback(uint8_t * mac, uint16_t reason)
{
    (void) (mac);
    ChipLogProgress(DeviceLayer, "Disconnected %d", reason);
    wifi_extra.Clear(WifiInterface::WifiState::kStationConnected);

    ScheduleConnectionAttempt();
}

void WifiInterfaceImpl::ProcessEvents(void * arg)
{
    TickType_t last_dhcp_poll, now;
    EventBits_t flags;
    (void) arg;

    sta_netif      = chip::DeviceLayer::Silabs::Lwip::GetNetworkInterface(SL_WFX_STA_INTERFACE);
    last_dhcp_poll = xTaskGetTickCount();
    while (true)
    {
        flags = xEventGroupWaitBits(sl_wfx_event_group,
                                    SL_WFX_CONNECT | SL_WFX_DISCONNECT
#ifdef SL_WFX_CONFIG_SOFTAP
                                        | SL_WFX_START_AP | SL_WFX_STOP_AP
#endif /* SL_WFX_CONFIG_SOFTAP */
                                        | SL_WFX_SCAN_START | SL_WFX_SCAN_COMPLETE | BITS_TO_WAIT,
                                    pdTRUE, pdFALSE, pdMS_TO_TICKS(250)); /* 250 msec delay converted to ticks */
        if (flags & SL_WFX_RETRY_CONNECT)
        {
            ChipLogProgress(DeviceLayer, "sending the connect command");
            WifiInterface::GetInstance().ConnectToAccessPoint();
        }

        if (wifi_extra.Has(WifiInterface::WifiState::kStationConnected))
        {
            if ((now = xTaskGetTickCount()) > (last_dhcp_poll + pdMS_TO_TICKS(250)))
            {
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
                uint8_t dhcp_state = dhcpclient_poll(sta_netif);

                if ((dhcp_state == DHCP_ADDRESS_ASSIGNED) && !WifiInterfaceImpl::GetIstance().HasNotifiedIPv4())
                {
                    WifiInterface::GetInstance().GotIPv4Address((uint32_t) sta_netif->ip_addr.u_addr.ip4.addr);
                    if (!hasNotifiedWifiConnectivity)
                    {
                        ChipLogProgress(DeviceLayer, "will notify WiFi connectivity");
                        WifiInterfaceImpl::GetInstance().NotifyConnection(ap_mac);
                        hasNotifiedWifiConnectivity = true;
                    }
                }
                else if (dhcp_state == DHCP_OFF)
                {
                    NotifyIPv4Change(false);
                }
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4
                if ((ip6_addr_ispreferred(netif_ip6_addr_state(sta_netif, 0))) &&
                    !WifiInterfaceImpl::GetInstance().HasNotifiedIPv6())
                {
                    WifiInterfaceImpl::GetInstance().NotifyIPv6Change(true);
                    if (!hasNotifiedWifiConnectivity)
                    {
                        WifiInterfaceImpl::GetInstance().NotifyConnection(ap_mac);
                        hasNotifiedWifiConnectivity = true;
                    }
                }
                last_dhcp_poll = now;
            }
        }

        if (flags & SL_WFX_CONNECT)
        {
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
            WifiInterfaceImpl::GetInstance().NotifyIPv4Change(false);
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4
            WifiInterfaceImpl::GetInstance().NotifyIPv6Change(false);
            hasNotifiedWifiConnectivity = false;
            ChipLogProgress(DeviceLayer, "connected to AP");
            wifi_extra.Set(WifiInterface::WifiState::kStationConnected);
            chip::DeviceLayer::Silabs::Lwip::SetLwipStationLinkUp();
#if CHIP_CONFIG_ENABLE_ICD_SERVER
            if (!(wifi_extra.Has(WifiInterface::WifiState::kAPReady)))
            {
                // Enable the power save
                ChipLogProgress(DeviceLayer, "WF200 going to DTIM based sleep");
                sl_wfx_set_power_mode(WFM_PM_MODE_DTIM, WFM_PM_POLL_FAST_PS, BEACON_1, 0 /*timeout*/);
                sl_wfx_enable_device_power_save();
            }
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
        }

        if (flags & SL_WFX_DISCONNECT)
        {

#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
            WifiInterfaceImpl::GetInstance().NotifyIPv4Change(false);
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4
            WifiInterfaceImpl::GetInstance().NotifyIPv6Change(false);
            hasNotifiedWifiConnectivity = false;
            wifi_extra.Clear(WifiInterface::WifiState::kStationConnected);
            chip::DeviceLayer::Silabs::Lwip::SetLwipStationLinkDown();
        }

        if (flags & SL_WFX_SCAN_START)
        {

            // Start the Scan
            sl_wfx_ssid_def_t ssid       = { 0 };
            sl_wfx_ssid_def_t * ssidPtr  = nullptr;
            uint16_t nbreScannedNetworks = 0;

            if (scan_ssid)
            {

                chip::ByteSpan requestedSsid(scan_ssid, scan_ssid_length);
                chip::MutableByteSpan outputSsid(ssid.ssid, WFX_MAX_SSID_LENGTH);

                chip::CopySpanToMutableSpan(requestedSsid, outputSsid);
                ssid.ssid_length = outputSsid.size();

                nbreScannedNetworks = 1;
                ssidPtr             = &ssid;
            }
            else
            {
                nbreScannedNetworks = 0;
                ssidPtr             = nullptr;
            }

            ChipLogDetail(DeviceLayer,
                          "WIFI Scan Paramter set to Active channel time %d, Passive "
                          "Channel Time: %d, Number of prob: %d",
                          ACTIVE_CHANNEL_TIME, PASSIVE_CHANNEL_TIME, NUM_PROBE_REQUEST);
            (void) sl_wfx_set_scan_parameters(ACTIVE_CHANNEL_TIME, PASSIVE_CHANNEL_TIME, NUM_PROBE_REQUEST);
            (void) sl_wfx_send_scan_command(WFM_SCAN_MODE_ACTIVE, CHANNEL_LIST,    /* Channel list */
                                            CHANNEL_COUNT,                         /* Scan all chans */
                                            ssidPtr, nbreScannedNetworks, IE_DATA, /* IE we're looking for */
                                            IE_DATA_LENGTH, BSSID_SCAN);
        }
        if (flags & SL_WFX_SCAN_COMPLETE)
        {
            struct scan_result_holder *hp, *next;

            ChipLogDetail(DeviceLayer, "WIFI: Return %d scan results", scan_count);
            for (hp = scan_save; hp; hp = next)
            {
                next = hp->next;
                scan_cb(&hp->scan);
                chip::Platform::MemoryFree(hp);
            }
            scan_cb(nullptr);

            // Clean up
            scan_save  = nullptr;
            scan_count = 0;

            if (scan_ssid)
            {
                chip::Platform::MemoryFree(scan_ssid);
                scan_ssid = NULL;
            }
            scan_cb = nullptr;
        }
    }
}

sl_status_t WifiInterfaceImpl::InitWf200Platform()
{
    sl_status_t status = SL_STATUS_OK;

    VerifyOrReturnValue(!wifi_extra.Has(WifiInterface::WifiState::kStationStarted), SL_STATUS_OK);

    ChipLogDetail(DeviceLayer, "STARTING WF200");

    sl_wfx_host_gpio_init();

    status = wfx_init();
    VerifyOrReturnError(status == SL_STATUS_OK, status, ChipLogError(DeviceLayer, "WF200:init failed: %ld", status));

    /* Initialize the LwIP stack */
    ChipLogDetail(DeviceLayer, "WF200:Start LWIP");
    chip::DeviceLayer::Silabs::Lwip::InitializeLwip();
    NotifyWifiTaskInitialized();

    ChipLogDetail(DeviceLayer, "WF200:ready.");
    wifi_extra.Set(WifiInterface::WifiState::kStationStarted);

    return SL_STATUS_OK;
}

void WifiInterfaceImpl::StartWifiProcessTask()
{
    /* create an event group to track Wi-Fi events */
    sl_wfx_event_group = xEventGroupCreateStatic(&wfxEventGroup);

    wfx_events_task_handle = xTaskCreateStatic(ProcessEvents, "wfx_events", WLAN_TASK_STACK_SIZE, NULL, WLAN_TASK_PRIORITY,
                                               wfxEventTaskStack, &wfxEventTaskBuffer);
    if (NULL == wfx_events_task_handle)
    {
        // TODO: Verify if this needs to be a chip-die or not.
        ChipLogError(DeviceLayer, "Failed to create WFX wfx_events");
    }
}

sl_status_t WifiInterfaceImpl::wfx_init(void)
{
    /* Initialize the WF200 used by the two interfaces */
    StartWifiProcessTask();
    sl_status_t status = sl_wfx_init(&wifiContext);
    ChipLogProgress(DeviceLayer, "FMAC Driver version: %s", FMAC_DRIVER_VERSION_STRING);
    switch (status)
    {
    case SL_STATUS_OK:
        ChipLogProgress(DeviceLayer, "WF200 FW ver:%d.%d.%d [MAC %02x:%02x:%02x-%02x:%02x:%02x]", wifiContext.firmware_major,
                        wifiContext.firmware_minor, wifiContext.firmware_build, wifiContext.mac_addr_0.octet[0],
                        wifiContext.mac_addr_0.octet[1], wifiContext.mac_addr_0.octet[2], wifiContext.mac_addr_0.octet[3],
                        wifiContext.mac_addr_0.octet[4], wifiContext.mac_addr_0.octet[5]);
        ChipLogProgress(DeviceLayer, "WF200 Init OK");

        if (wifi_extra.Has(WifiInterface::WifiState::kStationConnected))
        {
            sl_wfx_send_disconnect_command();
        }

        break;
    case SL_STATUS_WIFI_INVALID_KEY:
        ChipLogError(DeviceLayer, "WF200: F/W keyset invalid");
        break;
    case SL_STATUS_WIFI_FIRMWARE_DOWNLOAD_TIMEOUT:
        ChipLogError(DeviceLayer, "WF200: F/W download timo");
        break;
    case SL_STATUS_TIMEOUT:
        ChipLogError(DeviceLayer, "WF200: Poll for value timo");
        break;
    case SL_STATUS_FAIL:
        ChipLogError(DeviceLayer, "WF200: Error");
        break;
    default:
        ChipLogError(DeviceLayer, "WF200: Unknown");
    }

    return status;
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
