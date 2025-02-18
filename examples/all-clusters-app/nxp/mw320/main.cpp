/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
// #include "FreeRTOS.h"
// #include "task.h"

#include <lib/shell/Engine.h>

#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/SetupPayload.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>

// #include <lib/support/RandUtils.h>   //==> rm from TE7.5
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <static-supported-modes-manager.h>
#include <static-supported-temperature-levels.h>

#include <app/InteractionModelEngine.h>

#include <ChipShellCollection.h>

// cr++
#if (defined(CONFIG_CHIP_MW320_REAL_FACTORY_DATA) && (CONFIG_CHIP_MW320_REAL_FACTORY_DATA == 1))
#include "FactoryDataProvider.h"
#else
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#endif // if CONFIG_CHIP_MW320_REAL_FACTORY_DATA
// cr--
// ota++
#include "app/clusters/ota-requestor/BDXDownloader.h"
#include "app/clusters/ota-requestor/DefaultOTARequestor.h"
#include "app/clusters/ota-requestor/DefaultOTARequestorDriver.h"
#include "app/clusters/ota-requestor/DefaultOTARequestorStorage.h"
// #include <app/clusters/ota-requestor/DefaultOTARequestorUserConsent.h>
#include "platform/nxp/mw320/OTAImageProcessorImpl.h"
// #include "app/clusters/ota-requestor/OTARequestorDriver.h"

// for ota module test
#include "mw320_ota.h"

// ota--
#include "app/clusters/bindings/BindingManager.h"
#include "binding-handler.h"

/* platform specific */
#include "board.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "pin_mux.h"

#include <wm_os.h>
extern "C" {
#include "boot_flags.h"
#include "cli.h"
#include "dhcp-server.h"
#include "iperf.h"
#include "mflash_drv.h"
#include "network_flash_storage.h"
#include "partition.h"
#include "ping.h"
#include "wlan.h"
#include "wm_net.h"
}
#include "fsl_aes.h"
#include "lpm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_AES AES
#define CONNECTION_INFO_FILENAME "connection_info.dat"
#define SSID_FNAME "ssid_fname"
#define PSK_FNAME "psk_fname"

#define VERSION_STR "mw320-2.9.10-005"
enum
{
    MCUXPRESSO_WIFI_CLI,
    MATTER_SHELL,
    MAX_SELECTION,
};
static int Matter_Selection = MAX_SELECTION;
#define RUN_RST_LT_DELAY 10
static const char TAG[] = "mw320";

/*******************************************************************************
 * Variables
 ******************************************************************************/
static SemaphoreHandle_t aesLock;
static struct wlan_network sta_network;
static struct wlan_network uap_network;

chip::app::Clusters::TemperatureControl::AppSupportedTemperatureLevelsDelegate sAppSupportedTemperatureLevelsDelegate;
chip::app::Clusters::ModeSelect::StaticSupportedModesManager sStaticSupportedModesManager;

const int TASK_MAIN_PRIO         = OS_PRIO_3;
const int TASK_MAIN_STACK_SIZE   = 800;
portSTACK_TYPE * task_main_stack = NULL;
TaskHandle_t task_main_task_handler;

uint8_t * __FACTORY_DATA_START;
uint32_t __FACTORY_DATA_SIZE;

#if CHIP_ENABLE_OPENTHREAD
extern "C" {
#include <openthread/platform/platform-softdevice.h>
}
#endif // CHIP_ENABLE_OPENTHREAD

using namespace chip;
using namespace chip::Credentials;
using namespace ::chip::app;
using namespace chip::Shell;
using namespace chip::DeviceLayer;

// ota ++
using chip::BDXDownloader;
using chip::DefaultOTARequestor;
using chip::OTADownloader;
using chip::OTAImageProcessorImpl;
using chip::OTAImageProgress;

DefaultOTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
chip::DeviceLayer::DefaultOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;
// chip::ota::DefaultOTARequestorUserConsent gUserConsentProvider;
// static chip::ota::UserConsentState gUserConsentState = chip::ota::UserConsentState::kGranted;

void InitOTARequestor(void)
{
    // Initialize and interconnect the Requestor and Image Processor objects -- START
    SetRequestorInstance(&gRequestorCore);

    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());

    // Set server instance used for session establishment
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);

    // WARNING: this is probably not realistic to know such details of the image or to even have an OTADownloader instantiated at
    // the beginning of program execution. We're using hardcoded values here for now since this is a reference application.
    // TODO: instatiate and initialize these values when QueryImageResponse tells us an image is available
    // TODO: add API for OTARequestor to pass QueryImageResponse info to the application to use for OTADownloader init
    // OTAImageProcessor ipParams;
    // ipParams.imageFile = "dnld_img.txt"_span;
    // gImageProcessor.SetOTAImageProcessorParams(ipParams);
    gImageProcessor.SetOTADownloader(&gDownloader);

    // Connect the Downloader and Image Processor objects
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);
    /*
        if (gUserConsentState != chip::ota::UserConsentState::kUnknown)
        {
            gUserConsentProvider.SetUserConsentState(gUserConsentState);
            gRequestorUser.SetUserConsentDelegate(&gUserConsentProvider);
        }
    */
    // Initialize and interconnect the Requestor and Image Processor objects -- END
}

const char * mw320_get_verstr(void)
{
    return VERSION_STR;
}

void save_network(char * ssid, char * pwd);
void save_network(char * ssid, char * pwd)
{
    int ret;

    ret = save_wifi_network((char *) SSID_FNAME, (uint8_t *) ssid, strlen(ssid) + 1);
    if (ret != WM_SUCCESS)
    {
        PRINTF("Error: write ssid to flash failed\r\n");
    }

    ret = save_wifi_network((char *) PSK_FNAME, (uint8_t *) pwd, strlen(pwd) + 1);
    if (ret != WM_SUCCESS)
    {
        PRINTF("Error: write psk to flash failed\r\n");
    }

    return;
}

// ota --

namespace {
static void rst_args_lt(System::Layer * aSystemLayer, void * aAppState);
}

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

volatile int g_ButtonPress = 0;
bool need2sync_sw_attr     = false;

void sw2_handle(bool frm_clk)
{
    static uint8_t click_cnt = 0;
    static uint8_t run_times = 0;

    if (frm_clk == true)
    {
        // Called while user clicks the button
        click_cnt++;
        PRINTF(" (%d times) \r\n", click_cnt);
        return;
    }
    // Called regularlly from a thread every 500ms
    run_times++;
    if (click_cnt > 4)
    {
        // More than 4 clicks within the last second => erase the saved parameters
        PRINTF("--> enough clicks (%d times) => resetting the saved parameters \r\n", click_cnt);
        ::erase_all_params();
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(RUN_RST_LT_DELAY), rst_args_lt, nullptr);
        click_cnt = 0;
    }
    if (run_times >= 2)
    {
        // Called twice with gap==500ms
        click_cnt = 0;
        run_times = 0;
    }

    return;
}

void GPIO_IRQHandler(void)
{
    uint32_t intrval = GPIO_PortGetInterruptFlags(GPIO, GPIO_PORT(BOARD_SW1_GPIO_PIN));

    // Clear the interrupt
    GPIO_PortClearInterruptFlags(GPIO, GPIO_PORT(BOARD_SW1_GPIO_PIN), intrval);
    // Check which sw tiggers the interrupt
    if (intrval & 1UL << GPIO_PORT_PIN(BOARD_SW1_GPIO_PIN))
    {
        PRINTF("SW_1 click => do switch handler\r\n");
        /* Change state of button. */
        g_ButtonPress++;
        need2sync_sw_attr = true;
    }
    else if (intrval & 1UL << GPIO_PORT_PIN(BOARD_SW2_GPIO_PIN))
    {
        PRINTF("SW_2 click \r\n");
        sw2_handle(true);
    }
    SDK_ISR_EXIT_BARRIER;
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*
Protocols::InteractionModel::Status emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                   EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                   uint8_t * buffer, uint16_t maxReadLength, int32_t index)
{
    PRINTF("====> emberAfExternalAttributeReadCallback\r\n");

    if(clusterId == Clusters::Switch::Id) {
        *buffer = g_ButtonPress;
    }
    return Protocols::InteractionModel::Status::Success;
}
*/

namespace {
typedef enum
{
    chip_srv_all,
    dns_srv,
    srv_type_max
} srv_type_t;

typedef enum
{
    led_yellow,
    led_amber,
    led_max
} led_id_t;

static void run_chip_srv(System::Layer * aSystemLayer, void * aAppState);
static void run_dnssrv(System::Layer * aSystemLayer, void * aAppState);
static void run_update_chipsrv(srv_type_t srv_type);
static void led_on_off(led_id_t lt_id, bool is_on);
bool is_connected = false;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void load_network(char * ssid, char * pwd);

/*
static void saveProfile(int argc, char **argv);
static void loadProfile(int argc, char **argv);
static void resetProfile(int argc, char **argv);

static void wlanIeeePowerSave(int argc, char **argv);
static void wlanDeepSleep(int argc, char **argv);
static void mcuPowerMode(int argc, char **argv);


static struct cli_command saveload[] = {
    {"save-profile", "<profile_name>", saveProfile},
    {"load-profile", NULL, loadProfile},
    {"reset-profile", NULL, resetProfile},
};

static struct cli_command wlanPower[] = {
    {"wlan-ieee-power-save", "<on/off> <wakeup condition>", wlanIeeePowerSave},
    {"wlan-deepsleep", "<on/off>", wlanDeepSleep},
};

static struct cli_command mcuPower[] = {
    {"mcu-power-mode", "<pm0/pm1/pm2/pm4> [<pm2_io_exclude_mask>]", mcuPowerMode},
};
*/

TaskHandle_t sShellTaskHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/
static status_t APP_AES_Lock(void)
{
    if (pdTRUE == xSemaphoreTakeRecursive(aesLock, portMAX_DELAY))
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

static void APP_AES_Unlock(void)
{
    xSemaphoreGiveRecursive(aesLock);
}

static void load_network(char * ssid, char * pwd)
{
    int ret;
    unsigned char ssid_buf[IEEEtypes_SSID_SIZE + 1];
    unsigned char psk_buf[WLAN_PSK_MAX_LENGTH];
    uint32_t len;

    len = IEEEtypes_SSID_SIZE + 1;
    ret = get_saved_wifi_network((char *) SSID_FNAME, ssid_buf, &len);
    if (ret != WM_SUCCESS)
    {
        PRINTF("Error: Read saved SSID\r\n");
        strcpy(ssid, "");
    }
    else
    {
        PRINTF("saved_ssid: [%s]\r\n", ssid_buf);
        strcpy(ssid, (const char *) ssid_buf);
    }

    len = WLAN_PSK_MAX_LENGTH;
    ret = get_saved_wifi_network((char *) PSK_FNAME, psk_buf, &len);
    if (ret != WM_SUCCESS)
    {
        PRINTF("Error: Read saved PSK\r\n");
        strcpy(pwd, "");
    }
    else
    {
        PRINTF("saved_psk: [%s]\r\n", psk_buf);
        strcpy(pwd, (const char *) psk_buf);
    }
}

/*
static void saveProfile(int argc, char **argv)
{
    int ret;
    struct wlan_network network;

    if (argc < 2)
    {
        PRINTF("Usage: %s <profile_name>\r\n", argv[0]);
        PRINTF("Error: specify network to save\r\n");
        return;
    }

    ret = wlan_get_network_byname(argv[1], &network);
    if (ret != WM_SUCCESS)
    {
        PRINTF("Error: network not found\r\n");
    }
    else
    {
        ret = save_wifi_network((char *)CONNECTION_INFO_FILENAME, (uint8_t *)&network, sizeof(network));
        if (ret != WM_SUCCESS)
        {
            PRINTF("Error: write network to flash failed\r\n");
        }
    }
}

static void loadProfile(int argc, char **argv)
{
    int ret;
    struct wlan_network network;
    uint32_t len = sizeof(network);

    ret = get_saved_wifi_network((char *)CONNECTION_INFO_FILENAME, (uint8_t *)&network, &len);
    if (ret != WM_SUCCESS || len != sizeof(network))
    {
        PRINTF("Error: No network saved\r\n");
    }
    else
    {
        ret = wlan_add_network(&network);
        if (ret != WM_SUCCESS)
        {
            PRINTF("Error: network data corrupted or network already added\r\n");
        }
    }
}

static void resetProfile(int argc, char **argv)
{
    int ret;

    ret = reset_saved_wifi_network((char *)CONNECTION_INFO_FILENAME);
    if (ret != WM_SUCCESS)
    {
        PRINTF("Error: Reset profile failed\r\n");
    }
}

static void wlanIeeePowerSave(int argc, char **argv)
{
    bool on  = false;
    bool off = false;
    uint32_t cond;
    int ret;

    if (argc >= 2)
    {
        on  = (strcmp(argv[1], "on") == 0);
        off = (strcmp(argv[1], "off") == 0);
    }
    if ((argc < 2) || (!on && !off) || (on && argc < 3))
    {
        PRINTF("Usage: %s <on/off> [<wakeup condition>]\r\n", argv[0]);
        PRINTF("       wakeup condictions needed by \"on\" command:\r\n");
        PRINTF("       bit0=1: broadcast data\r\n");
        PRINTF("       bit1=1: unicast data\r\n");
        PRINTF("       bit2=1: mac events\r\n");
        PRINTF("       bit3=1: multicast data\r\n");
        PRINTF("       bit4=1: arp broadcast data\r\n");
        PRINTF("       bit6=1: management frame\r\n");
        return;
    }

    if (on)
    {
        cond = strtoul(argv[2], NULL, 0);
        ret  = wlan_ieeeps_on(cond);
    }
    else
    {
        ret = wlan_ieeeps_off();
    }

    if (ret != WM_SUCCESS)
    {
        PRINTF("Cannot request IEEE power save mode change!\r\n");
    }
    else
    {
        PRINTF("IEEE power save mode change requested!\r\n");
    }
}

static void wlanDeepSleep(int argc, char **argv)
{
    bool on;
    int ret;

    if ((argc < 2) || ((strcmp(argv[1], "on") != 0) && (strcmp(argv[1], "off") != 0)))
    {
        PRINTF("Usage: %s <on/off>\r\n", argv[0]);
        PRINTF("Error: specify deep sleep on or off.\r\n");
        return;
    }

    on = (strcmp(argv[1], "on") == 0);
    if (on)
    {
        ret = wlan_deepsleepps_on();
    }
    else
    {
        ret = wlan_deepsleepps_off();
    }

    if (ret != WM_SUCCESS)
    {
        PRINTF("Cannot request deep sleep mode change!\r\n");
    }
    else
    {
        PRINTF("Deep sleep mode change requested!\r\n");
    }
}


static void mcuPowerMode(int argc, char **argv)
{
    uint32_t excludeIo = 0U;

    if ((argc < 2) || (strlen(argv[1]) != 3) || (argv[1][0] != 'p') || (argv[1][1] != 'm') || (argv[1][2] < '0') ||
        (argv[1][2] > '4') || (argv[1][2] == '3'))
    {
        PRINTF("Usage: %s <pm0/pm1/pm2/pm4> [<pm2_io_exclude_mask>]\r\n", argv[0]);
        PRINTF("       pm2_io_exclude_mask: bitmask of io domains to keep on in PM2.\r\n");
        PRINTF("                            e.g. 0x5 means VDDIO0 and VDDIO2 will not be powered off in PM2\r\n");
        PRINTF("Error: specify power mode to enter.\r\n");
        return;
    }

    if (argv[1][2] - '0' == 2U)
    {
        if (argc < 3)
        {
            PRINTF("Error: PM2 need 3rd parameter.\r\n");
            return;
        }
        else
        {
            excludeIo = strtoul(argv[2], NULL, 0);
        }
    }

    LPM_SetPowerMode(argv[1][2] - '0', excludeIo);
}
*/

static void mcuInitPower(void)
{
    lpm_config_t config = {
        /* System PM2/PM3 less than 50 ms will be skipped. */
        .threshold = 50U,
        /* SFLL config and  RC32M setup takes approx 14 ms. */
        .latency          = 15U,
        .enableWakeupPin0 = true,
        .enableWakeupPin1 = true,
        .handler          = NULL,
    };

    LPM_Init(&config);
}

/* Callback Function passed to WLAN Connection Manager. The callback function
 * gets called when there are WLAN Events that need to be handled by the
 * application.
 */
int wlan_event_callback(enum wlan_event_reason reason, void * data)
{
    int ret;
    struct wlan_ip_config addr;
    char ip[16];
    static int auth_fail = 0;

    //    PRINTF("[%s] WLAN: received event %d\r\n", __FUNCTION__, reason);
    switch (reason)
    {
    case WLAN_REASON_INITIALIZED:
        // PRINTF("app_cb: WLAN initialized\r\n");
#ifdef MCUXPRESSO_WIFI_CLI
        ret = wlan_basic_cli_init();
        if (ret != WM_SUCCESS)
        {
            PRINTF("Failed to initialize BASIC WLAN CLIs\r\n");
            return 0;
        }

        ret = wlan_cli_init();
        if (ret != WM_SUCCESS)
        {
            PRINTF("Failed to initialize WLAN CLIs\r\n");
            return 0;
        }
        PRINTF("WLAN CLIs are initialized\r\n");

        ret = ping_cli_init();
        if (ret != WM_SUCCESS)
        {
            PRINTF("Failed to initialize PING CLI\r\n");
            return 0;
        }

        ret = iperf_cli_init();
        if (ret != WM_SUCCESS)
        {
            PRINTF("Failed to initialize IPERF CLI\r\n");
            return 0;
        }
#endif
        ret = dhcpd_cli_init();
        if (ret != WM_SUCCESS)
        {
            //                PRINTF("Failed to initialize DHCP Server CLI\r\n");
            return 0;
        }
#ifdef MCUXPRESSO_WIFI_CLI
        if (cli_register_commands(saveload, sizeof(saveload) / sizeof(struct cli_command)))
        {
            return -WM_FAIL;
        }

        if (cli_register_commands(wlanPower, sizeof(wlanPower) / sizeof(struct cli_command)))
        {
            return -WM_FAIL;
        }

        if (cli_register_commands(mcuPower, sizeof(mcuPower) / sizeof(struct cli_command)))
        {
            return -WM_FAIL;
        }

        PRINTF("CLIs Available:\r\n");
        if (Matter_Selection == MCUXPRESSO_WIFI_CLI)
            help_command(0, NULL);
#endif
        break;
    case WLAN_REASON_INITIALIZATION_FAILED:
        // PRINTF("app_cb: WLAN: initialization failed\r\n");
        break;
    case WLAN_REASON_SUCCESS:
        // PRINTF("app_cb: WLAN: connected to network\r\n");
        ret = wlan_get_address(&addr);
        if (ret != WM_SUCCESS)
        {
            // PRINTF("failed to get IP address\r\n");
            return 0;
        }

        net_inet_ntoa(addr.ipv4.address, ip);

        ret = wlan_get_current_network(&sta_network);
        if (ret != WM_SUCCESS)
        {
            // PRINTF("Failed to get External AP network\r\n");
            return 0;
        }

        PRINTF("Connected to following BSS:\r\n");
        PRINTF("SSID = [%s], IP = [%s]\r\n", sta_network.ssid, ip);
        save_network(sta_network.ssid, sta_network.security.psk);

#ifdef CONFIG_IPV6
        {
            int i;
            (void) PRINTF("\r\n\tIPv6 Addresses\r\n");
            for (i = 0; i < MAX_IPV6_ADDRESSES; i++)
            {
                if (sta_network.ip.ipv6[i].addr_state != IP6_ADDR_INVALID)
                {
                    (void) PRINTF("\t%-13s:\t%s (%s)\r\n", ipv6_addr_type_to_desc(&(sta_network.ip.ipv6[i])),
                                  inet6_ntoa(sta_network.ip.ipv6[i].address),
                                  ipv6_addr_state_to_desc(sta_network.ip.ipv6[i].addr_state));
                }
            }
            (void) PRINTF("\r\n");
        }
#endif
        auth_fail    = 0;
        is_connected = true;
        run_update_chipsrv(dns_srv);

        if (is_uap_started())
        {
            wlan_get_current_uap_network(&uap_network);
            ret = wlan_stop_network(uap_network.name);
            /*			    if (ret != WM_SUCCESS)
                                            PRINTF("Error: unable to stop network\r\n");
                                            else
                                                    PRINTF("stop uAP, SSID = [%s]\r\n", uap_network.ssid);
            */
        }
        break;
    case WLAN_REASON_CONNECT_FAILED:
        //            PRINTF("app_cb: WLAN: connect failed\r\n");
        break;
    case WLAN_REASON_NETWORK_NOT_FOUND:
        //            PRINTF("app_cb: WLAN: network not found\r\n");
        break;
    case WLAN_REASON_NETWORK_AUTH_FAILED:
        //            PRINTF("app_cb: WLAN: network authentication failed\r\n");
        auth_fail++;
        if (auth_fail >= 3)
        {
            //                PRINTF("Authentication Failed. Disconnecting ... \r\n");
            wlan_disconnect();
            auth_fail = 0;
        }
        break;
    case WLAN_REASON_ADDRESS_SUCCESS:
        //            PRINTF("network mgr: DHCP new lease\r\n");
        break;
    case WLAN_REASON_ADDRESS_FAILED:
        //            PRINTF("app_cb: failed to obtain an IP address\r\n");
        break;
    case WLAN_REASON_USER_DISCONNECT:
        //            PRINTF("app_cb: disconnected\r\n");
        auth_fail = 0;
        break;
    case WLAN_REASON_LINK_LOST:
        is_connected = false;
        run_update_chipsrv(dns_srv);
        //            PRINTF("app_cb: WLAN: link lost\r\n");
        break;
    case WLAN_REASON_CHAN_SWITCH:
        //            PRINTF("app_cb: WLAN: channel switch\r\n");
        break;
    case WLAN_REASON_UAP_SUCCESS:
        //            PRINTF("app_cb: WLAN: UAP Started\r\n");
        ret = wlan_get_current_uap_network(&uap_network);

        if (ret != WM_SUCCESS)
        {
            PRINTF("Failed to get Soft AP network\r\n");
            return 0;
        }

        //            PRINTF("Soft AP \"%s\" started successfully\r\n", uap_network.ssid);
        if (dhcp_server_start(net_get_uap_handle()))
            PRINTF("Error in starting dhcp server\r\n");
        //            PRINTF("DHCP Server started successfully\r\n");
        break;
    case WLAN_REASON_UAP_CLIENT_ASSOC:
        PRINTF("app_cb: WLAN: UAP a Client Associated\r\n");
        //            PRINTF("Client => ");
        //            print_mac((const char *)data);
        //            PRINTF("Associated with Soft AP\r\n");
        break;
    case WLAN_REASON_UAP_CLIENT_DISSOC:
        //            PRINTF("app_cb: WLAN: UAP a Client Dissociated\r\n");
        //            PRINTF("Client => ");
        //            print_mac((const char *)data);
        //            PRINTF("Dis-Associated from Soft AP\r\n");
        break;
    case WLAN_REASON_UAP_STOPPED:
        //            PRINTF("app_cb: WLAN: UAP Stopped\r\n");
        //            PRINTF("Soft AP \"%s\" stopped successfully\r\n", uap_network.ssid);
        dhcp_server_stop();
        //            PRINTF("DHCP Server stopped successfully\r\n");
        break;
    case WLAN_REASON_PS_ENTER:
        //            PRINTF("app_cb: WLAN: PS_ENTER\r\n");
        break;
    case WLAN_REASON_PS_EXIT:
        //            PRINTF("app_cb: WLAN: PS EXIT\r\n");
        break;
    default:
        PRINTF("app_cb: WLAN: Unknown Event: %d\r\n", reason);
    }
    return 0;
}

#if 0
char profile[8] = "mw320";
char ssid[32] = "matter_mw320";
char psk[64] = "12345678";
char network_ip[15] = "192.168.2.1";
char network_netmask[15] = "255.255.255.0";
const uint8_t kOptionalDefaultStringTag1       = 1;
const uint8_t kOptionalDefaultStringTag2       = 2;
const uint8_t kOptionalDefaultStringTag3       = 3;

std::string createSetupPayload()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::string result;
    std::string kOptionalDefaultStringValue1 = "IP:";
    std::string kOptionalDefaultStringValue2 = "SSID:";
    std::string kOptionalDefaultStringValue3 = "Key:";
    uint16_t discriminator;

    kOptionalDefaultStringValue1.append( network_ip, sizeof(network_ip) );
    kOptionalDefaultStringValue2.append( ssid, sizeof(ssid) );
    kOptionalDefaultStringValue3.append( psk, sizeof(psk) );
    err = ConfigurationMgr().GetSetupDiscriminator(discriminator);
    if (err != CHIP_NO_ERROR)
    {
        PRINTF("[%s]: Couldn't get discriminator: %s\r\n", __FUNCTION__, ErrorStr(err));
        return result;
    }

    uint32_t setupPINCode;
    err = ConfigurationMgr().GetSetupPinCode(setupPINCode);
    if (err != CHIP_NO_ERROR)
    {
        PRINTF("[%s]: Couldn't get setupPINCode: %s\r\n", __FUNCTION__, ErrorStr(err));
        return result;
    }

    uint16_t vendorId;
    err = ConfigurationMgr().GetVendorId(vendorId);
    if (err != CHIP_NO_ERROR)
    {
        PRINTF("[%s]: Couldn't get vendorId: %s\r\n", __FUNCTION__, ErrorStr(err));
        return result;
    }

    uint16_t productId;
    err = ConfigurationMgr().GetProductId(productId);
    if (err != CHIP_NO_ERROR)
    {
        PRINTF("[%s]: Couldn't get productId: %s\r\n", __FUNCTION__, ErrorStr(err));
        return result;
    }
    SetupPayload payload;
    payload.version               = 0;
    payload.discriminator         = discriminator;
    payload.setUpPINCode          = setupPINCode;
    payload.rendezvousInformation.SetValue(chip::RendezvousInformationFlag::kBLE);
    payload.vendorID              = vendorId;
    payload.productID             = productId;

    err = payload.addOptionalVendorData(kOptionalDefaultStringTag1, kOptionalDefaultStringValue1);
    if (err != CHIP_NO_ERROR)
    {
        PRINTF("[%s]: Couldn't add payload Vnedor string %d \r\n", __FUNCTION__, kOptionalDefaultStringTag1);
    }
    err = payload.addOptionalVendorData(kOptionalDefaultStringTag2, kOptionalDefaultStringValue2);
    if (err != CHIP_NO_ERROR)
    {
        PRINTF("[%s]: Couldn't add payload Vnedor string %d \r\n", __FUNCTION__, kOptionalDefaultStringTag2);
    }
    err = payload.addOptionalVendorData(kOptionalDefaultStringTag3, kOptionalDefaultStringValue3);
    if (err != CHIP_NO_ERROR)
    {
        PRINTF("[%s]: Couldn't add payload Vnedor string %d \r\n", __FUNCTION__, kOptionalDefaultStringTag3);
    }

    QRCodeSetupPayloadGenerator generator(payload);
    size_t tlvDataLen = sizeof(kOptionalDefaultStringValue1)+sizeof(kOptionalDefaultStringValue2)+sizeof(kOptionalDefaultStringValue3);
    uint8_t tlvDataStart[tlvDataLen];
    err = generator.payloadBase38Representation(result, tlvDataStart, tlvDataLen);

    if (err != CHIP_NO_ERROR)
    {
        PRINTF("[%s]: Couldn't get payload string %d \r\n", __FUNCTION__, err);
    }
    return result;
}
#endif // 0

#if 0
void demo_init(void)
{
	struct wlan_network network;
    int ret = 0;

	// add uAP profile
    memset(&network, 0, sizeof(struct wlan_network));
    memcpy(network.name, profile, strlen(profile));

	memcpy(network.ssid, ssid, strlen(ssid));
	network.channel = 1;

	network.ip.ipv4.address = net_inet_aton(network_ip);
    network.ip.ipv4.gw      = net_inet_aton(network_ip);
    network.ip.ipv4.netmask = net_inet_aton(network_netmask);
	network.ip.ipv4.addr_type = ADDR_TYPE_STATIC;

	network.security.psk_len = strlen(psk);
    strcpy(network.security.psk, psk);
	network.security.type = WLAN_SECURITY_WPA2;

	network.role = WLAN_BSS_ROLE_UAP;

    ret = wlan_add_network(&network);
    switch (ret)
    {
        case WM_SUCCESS:
            PRINTF("Added \"%s\"\r\n", network.name);
            break;
        case -WM_E_INVAL:
            PRINTF("Error: network already exists or invalid arguments\r\n");
            break;
        case -WM_E_NOMEM:
            PRINTF("Error: network list is full\r\n");
            break;
        case WLAN_ERROR_STATE:
            PRINTF("Error: can't add networks in this state\r\n");
            break;
        default:
            PRINTF(
                "Error: unable to add network for unknown"
                " reason\r\n");
            break;
    }

	// start uAP
    ret = wlan_start_network(profile);
    if (ret != WM_SUCCESS)
        PRINTF("Error: unable to start network\r\n");
	else
		PRINTF("start uAP ssid: %s\r\n", network.ssid);

}
#endif // 0

void task_main(void * param)
{
#if 0
    int32_t result = 0;
    flash_desc_t fl;
    struct partition_entry *p, *f1, *f2;
    short history = 0;
    uint32_t *wififw;
#ifdef CONFIG_USE_PSM
    struct partition_entry *psm;
#endif

    mcuInitPower();

    boot_init();

    mflash_drv_init();

    PRINTF("[%s]: Initialize CLI\r\n", __FUNCTION__);
    result = cli_init();
    if (WM_SUCCESS != result)
    {
        assert(false);
    }

    PRINTF("[%s]: Initialize WLAN Driver\r\n", __FUNCTION__);
    result = part_init();
    if (WM_SUCCESS != result)
    {
        assert(false);
    }

#ifdef CONFIG_USE_PSM
    psm = part_get_layout_by_id(FC_COMP_PSM, NULL);
    part_to_flash_desc(psm, &fl);
#else
    fl.fl_dev   = 0U;
    fl.fl_start = MFLASH_FILE_BASEADDR;
    fl.fl_size  = MFLASH_FILE_SIZE;
#endif
    init_flash_storage((char *)CONNECTION_INFO_FILENAME, &fl);

    f1 = part_get_layout_by_id(FC_COMP_WLAN_FW, &history);
    f2 = part_get_layout_by_id(FC_COMP_WLAN_FW, &history);

    if (f1 && f2)
    {
        p = part_get_active_partition(f1, f2);
    }
    else if (!f1 && f2)
    {
        p = f2;
    }
    else if (!f2 && f1)
    {
        p = f1;
    }
    else
    {
        PRINTF("[%s]: Wi-Fi Firmware not detected\r\n", __FUNCTION__);
        p = NULL;
    }

    if (p != NULL)
    {
        part_to_flash_desc(p, &fl);
        wififw = (uint32_t *)mflash_drv_phys2log(fl.fl_start, fl.fl_size);
        assert(wififw != NULL);
        /* First word in WIFI firmware is magic number. */
        assert(*wififw == (('W' << 0) | ('L' << 8) | ('F' << 16) | ('W' << 24)));

        /* Initialize WIFI Driver */
        /* Second word in WIFI firmware is WIFI firmware length in bytes. */
        /* Real WIFI binary starts from 3rd word. */
        result = wlan_init((const uint8_t *)(wififw + 2U), *(wififw + 1U));
        if (WM_SUCCESS != result)
        {
            assert(false);
        }
        PRINTF("[%s]: wlan_init success \r\n", __FUNCTION__);

        result = wlan_start(wlan_event_callback);
        if (WM_SUCCESS != result)
        {
            assert(false);
        }
    }

    while (true)
    {
        /* wait for interface up */
        os_thread_sleep(os_msec_to_ticks(5000));
		PRINTF("[%s]: looping\r\n", __FUNCTION__);
    }
#endif // 0
}

static void run_chip_srv(System::Layer * aSystemLayer, void * aAppState)
{
    // Init ZCL Data Model and CHIP App Server
    {
        // Initialize device attestation config
#if (defined(CONFIG_CHIP_MW320_REAL_FACTORY_DATA) && (CONFIG_CHIP_MW320_REAL_FACTORY_DATA == 1))
        FactoryDataProvider::GetDefaultInstance().Init();
#if (CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER == 1)
        SetDeviceInstanceInfoProvider(&FactoryDataProvider::GetDefaultInstance());
#endif // USE_LOCAL_DEVICEINSTANCEINFOPROVIDER
        SetDeviceAttestationCredentialsProvider(&FactoryDataProvider::GetDefaultInstance());
        SetCommissionableDataProvider(&FactoryDataProvider::GetDefaultInstance());
#else
        SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif // CONFIG_CHIP_MW320_REAL_FACTORY_DATA
    }
    {
        //    chip::Server::GetInstance().Init();
        // uint16_t securePort   = CHIP_PORT;
        // uint16_t unsecurePort = CHIP_UDC_PORT;

        // PRINTF("==> call chip::Server() \r\n");
        // chip::Server::GetInstance().Init(nullptr, securePort, unsecurePort);

        static chip::CommonCaseDeviceServerInitParams initParams;
        (void) initParams.InitializeStaticResourcesBeforeServerInit();
        initParams.dataModelProvider = CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
        chip::Server::GetInstance().Init(initParams);
        PRINTF("Done to call chip::Server() \r\n");
    }
    // ota ++
    {
        InitOTARequestor();
#if (MW320_OTA_TEST == 1)
        // for ota module test
        mw320_fw_update_test();
#endif // MW320_OTA_TEST
    }
    // ota --
    // binding ++
    InitBindingHandlers();
    // binding --

    chip::app::Clusters::TemperatureControl::SetInstance(&sAppSupportedTemperatureLevelsDelegate);
    chip::app::Clusters::ModeSelect::setSupportedModesManager(&sStaticSupportedModesManager);

    return;
}

static void run_dnssrv(System::Layer * aSystemLayer, void * aAppState)
{
    chip::app::DnssdServer::Instance().StartServer();
    if (is_connected == true)
    {
        led_on_off(led_amber, true);
    }
    else
    {
        led_on_off(led_amber, false);
    }
    return;
}

#define RUN_CHIPSRV_DELAY 1
static void run_update_chipsrv(srv_type_t srv_type)
{
    switch (srv_type)
    {
    case chip_srv_all:
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(RUN_CHIPSRV_DELAY), run_chip_srv, nullptr);
        break;
    case dns_srv:
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(RUN_CHIPSRV_DELAY), run_dnssrv, nullptr);
        break;
    default:
        return;
    }
    return;
}

//=============================================================================
// Light behaior while resetting the saved arguments
//
static void rst_args_lt(System::Layer * aSystemLayer, void * aAppState)
{
    // PRINTF("%s(), Turn on lights \r\n", __FUNCTION__);
    led_on_off(led_amber, true);
    led_on_off(led_yellow, true);
    // sleep 3 second
    // PRINTF("%s(), sleep 3 seconds \r\n", __FUNCTION__);
    os_thread_sleep(os_msec_to_ticks(3000));
    // PRINTF("%s(), Turn off lights \r\n", __FUNCTION__);
    led_on_off(led_amber, false);
    led_on_off(led_yellow, false);
    return;
}

void task_test_main(void * param)
{
    while (true)
    {
        /* wait for interface up */
        os_thread_sleep(os_msec_to_ticks(500));
        /*PRINTF("[%s]: looping\r\n", __FUNCTION__);*/
        if (need2sync_sw_attr == true)
        {
            static bool is_on = false;
            uint16_t value    = g_ButtonPress & 0x1;
            is_on             = !is_on;
            value             = (uint16_t) is_on;
            // sync-up the switch attribute:
            PRINTF("--> update CurrentPosition [%d] \r\n", value);
            Clusters::Switch::Attributes::CurrentPosition::Set(1, value);
#ifdef SUPPORT_MANUAL_CTRL
#error "Not implemented"
            // TODO: previous code was trying to write a OnOff cluster attribute id to a switch attribute, generally
            //       not working. Determine if this should maybe be
            //       OnOff::Attributes::OnOff::Set(1, is_on) or similar
#endif // SUPPORT_MANUAL_CTRL

            need2sync_sw_attr = false;
        }
        // =============================
        // Call sw2_handle to clear click_count if needed
        sw2_handle(false);
        // =============================
    }
    return;
}

void init_mw320_sdk()
{
    flash_desc_t fl;
    struct partition_entry *p, *f1, *f2;
    short history = 0;
    uint32_t * wififw;
    struct partition_entry * psm;
    struct partition_entry * manu_dat;
    uint8_t * pmfdat;

    PRINTF("=> init mw320 sdk \r\n");
    PRINTF("call mcuInitPower() \r\n");
    mcuInitPower();
    boot_init();
    mflash_drv_init();
    cli_init();
    part_init();

    psm = part_get_layout_by_id(FC_COMP_PSM, NULL);
    part_to_flash_desc(psm, &fl);
    init_flash_storage((char *) CONNECTION_INFO_FILENAME, &fl);
    PRINTF("[PSM]: (start, len)=(0x%x, 0x%x)\r\n", fl.fl_start, fl.fl_size);

    manu_dat = part_get_layout_by_id(FC_COMP_USER_APP, NULL);
    part_to_flash_desc(manu_dat, &fl);
    PRINTF("[Manufacture_Data]: (start, len)=(0x%x, 0x%x)\r\n", fl.fl_start, fl.fl_size);
    pmfdat               = (uint8_t *) mflash_drv_phys2log(fl.fl_start, fl.fl_size);
    __FACTORY_DATA_START = pmfdat;
    __FACTORY_DATA_SIZE  = (uint32_t) fl.fl_size;

    f1 = part_get_layout_by_id(FC_COMP_WLAN_FW, &history);
    f2 = part_get_layout_by_id(FC_COMP_WLAN_FW, &history);
    if (f1 && f2)
    {
        p = part_get_active_partition(f1, f2);
    }
    else if (!f1 && f2)
    {
        p = f2;
    }
    else if (!f2 && f1)
    {
        p = f1;
    }
    else
    {
        // PRINTF("[%s]: Wi-Fi Firmware not detected\r\n", __FUNCTION__);
        p = NULL;
    }
    if (p != NULL)
    {
        part_to_flash_desc(p, &fl);
        wififw = (uint32_t *) mflash_drv_phys2log(fl.fl_start, fl.fl_size);
        // assert(wififw != NULL);
        /* First word in WIFI firmware is magic number. */
        assert(*wififw == (('W' << 0) | ('L' << 8) | ('F' << 16) | ('W' << 24)));
        wlan_init((const uint8_t *) (wififw + 2U), *(wififw + 1U));
        // PRINTF("[%s]: wlan_init success \r\n", __FUNCTION__);
        wlan_start(wlan_event_callback);
        // demo_init();
        os_thread_sleep(os_msec_to_ticks(5000));
    }
    PRINTF(" mw320 init complete! \r\n");

    return;
}

void ShellCLIMain(void * pvParameter)
{
    const int rc = streamer_init(streamer_get());
    if (rc != 0)
    {
        ChipLogError(Shell, "Streamer initialization failed: %d", rc);
        return;
    }

    PRINTF("version: [%s] \r\n", VERSION_STR);

    // Initialize the SDK components
    init_mw320_sdk();

    ChipLogDetail(Shell, "Initializing CHIP shell commands: %d", rc);

    chip::Platform::MemoryInit();
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlag::kOnNetwork);
    chip::DeviceLayer::PlatformMgr().StartEventLoopTask();
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    chip::DeviceLayer::ConnectivityManagerImpl().StartWiFiManagement();
#endif

    cmd_misc_init();
    // cmd_otcli_init();
    ChipLogDetail(Shell, "Run CHIP shell Task: %d", rc);

    //    std::string qrCodeText = createSetupPayload();
    //    PRINTF("SetupQRCode: [%s]\r\n", qrCodeText.c_str());
    {
        char def_ssid[IEEEtypes_SSID_SIZE + 1];
        char def_psk[WLAN_PSK_MAX_LENGTH];
        load_network(def_ssid, def_psk);

        if ((strlen(def_ssid) <= 0) || (strlen(def_psk) <= 0))
        {
            // No saved connected_ap_info => Using the default ssid/password
            strcpy(def_ssid, "nxp_matter");
            strcpy(def_psk, "nxp12345");
        }
        PRINTF("Connecting to [%s, %s] \r\n", def_ssid, def_psk);

        // TODO: ConnectivityMgrImpl is the platform implementation of ConnectivityMgr layer.
        // Application should use the APIs defined src/include/platform to talk to the Matter
        // platfrom layer, instead of calling into the functions defined in the platform implemenation.
        // ConnectivityMgrImpl().ProvisionWiFiNetwork(def_ssid, def_psk);
    }

    // Run CHIP servers
    run_update_chipsrv(chip_srv_all);

    Engine::Root().RunMainLoop();
}

static void led_on_off(led_id_t lt_id, bool is_on)
{
    GPIO_Type * pgpio;
    uint32_t gpio_pin;

    // Configure the GPIO / PIN
    switch (lt_id)
    {
    case led_amber:
        pgpio    = BOARD_LED_AMBER_GPIO;
        gpio_pin = BOARD_LED_AMBER_GPIO_PIN;
        break;
    case led_yellow:
    default: // Note: led_yellow as default
        pgpio    = BOARD_LED_YELLOW_GPIO;
        gpio_pin = BOARD_LED_YELLOW_GPIO_PIN;
    }
    // Do on/off the LED
    if (is_on == true)
    {
        // PRINTF("led on\r\n");
        GPIO_PortClear(pgpio, GPIO_PORT(gpio_pin), 1u << GPIO_PORT_PIN(gpio_pin));
    }
    else
    {
        // PRINTF("led off\r\n");
        GPIO_PortSet(pgpio, GPIO_PORT(gpio_pin), 1u << GPIO_PORT_PIN(gpio_pin));
    }
    return;
}

} // namespace

int StartShellTask(void)
{
    int ret = 0;

    // Start Shell task.
    switch (Matter_Selection)
    {
    case MCUXPRESSO_WIFI_CLI:
#ifdef MCUXPRESSO_WIFI_CLI
        if (xTaskCreate(task_main, "main", TASK_MAIN_STACK_SIZE, task_main_stack, TASK_MAIN_PRIO, &task_main_task_handler) !=
            pdPASS)
        {
            ret = -1;
        }
        break;
#endif
    case MATTER_SHELL:
    default:
        if (xTaskCreate(ShellCLIMain, "SHELL", TASK_MAIN_STACK_SIZE, NULL, TASK_MAIN_PRIO, &sShellTaskHandle) != pdPASS)
        {
            ret = -1;
        }
        if (xTaskCreate(task_test_main, "testmain", TASK_MAIN_STACK_SIZE, task_main_stack, TASK_MAIN_PRIO,
                        &task_main_task_handler) != pdPASS)
        {
            PRINTF("Failed to crete task_test_main() \r\n");
            ret = -1;
        }
        break;
    }

    return ret;
}

#define gpio_led_cfg(base, pin, cfg)                                                                                               \
    {                                                                                                                              \
        GPIO_PinInit(base, pin, cfg);                                                                                              \
        GPIO_PortSet(base, GPIO_PORT(pin), 1u << GPIO_PORT_PIN(pin));                                                              \
    }

#define gpio_sw_cfg(base, pin, cfg, irq, trig)                                                                                     \
    {                                                                                                                              \
        GPIO_PinInit(base, pin, cfg);                                                                                              \
        GPIO_PinSetInterruptConfig(base, pin, trig);                                                                               \
        GPIO_PortEnableInterrupts(base, GPIO_PORT(pin), 1UL << GPIO_PORT_PIN(pin));                                                \
        EnableIRQ(irq);                                                                                                            \
    }

void gpio_init(void)
{
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput,
        0,
    };
    gpio_pin_config_t sw_config = {
        kGPIO_DigitalInput,
        0,
    };

    /* Init output amber led gpio off */
    gpio_led_cfg(BOARD_LED_AMBER_GPIO, BOARD_LED_AMBER_GPIO_PIN, &led_config);

    /* Init output yellow led gpio off */
    gpio_led_cfg(BOARD_LED_YELLOW_GPIO, BOARD_LED_YELLOW_GPIO_PIN, &led_config);

    /* Init/config input sw_1 GPIO. */
    gpio_sw_cfg(BOARD_SW1_GPIO, BOARD_SW1_GPIO_PIN, &sw_config, BOARD_SW1_IRQ, kGPIO_InterruptFallingEdge);

    /* Init/config input sw_2 GPIO. */
    gpio_sw_cfg(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN, &sw_config, BOARD_SW2_IRQ, kGPIO_InterruptFallingEdge);
    return;
}

int main(void)
{
    //    char ch;
    //    unsigned int bp;
    //    unsigned int mw320_sec = 9000000;
    //    unsigned int default_ch;
    //    unsigned int default_1= 0;
    //    unsigned int default_2= 0;

    /* Initialize platform */
    // BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();

    BOARD_InitDebugConsole();
#ifdef MCUXPRESSO_WIFI_CLI
    PRINTF("\nPlease Select [1/2] => 1. MCUXpress format 2. Matter format. \r\n");
    do
    {
        ch = GETCHAR();
        PUTCHAR(ch);
        if (ch == '1')
            Matter_Selection = MCUXPRESSO_WIFI_CLI;
        else if (ch == '2')
            Matter_Selection = MATTER_SHELL;
    } while (ch != '\r');
    if (Matter_Selection == MAX_SELECTION)
        Matter_Selection = MATTER_SHELL;
    PRINTF("\n\n[%s]:  MW320 %s .\r\n", __FUNCTION__,
           (Matter_Selection == MCUXPRESSO_WIFI_CLI) ? "MCUXPresso WiFi CLI" : "Matter Shell");
#else
#ifdef CONFIGURE_UAP
    PRINTF("\nDo you want to use the default SSID and key for mw320 uAP? [y/n]\r\n");
    do
    {
        ch = GETCHAR();
        PUTCHAR(ch);
        if (ch == 'n')
        {
            PRINTF("\nPlease input your SSID: [ 1 ~ 32 characters]\r\n");
            bp = 0;
            do
            {
                ssid[bp] = GETCHAR();
                PUTCHAR(ssid[bp]);
                bp++;
                if (bp > sizeof(ssid))
                {
                    PRINTF("\n ERROR: your SSID length=%d is larger than %d \r\n", bp, sizeof(ssid));
                    return 0;
                }
            } while (ssid[bp - 1] != '\r');
            ssid[bp - 1] = '\0';
            PRINTF("\nPlease input your KEY: [ 8 ~ 63 characters]\r\n");
            bp = 0;
            do
            {
                psk[bp] = GETCHAR();
                PUTCHAR(psk[bp]);
                bp++;
                if (bp > sizeof(psk))
                {
                    PRINTF("\n ERROR: your KEY length=%d is larger than %d \r\n", bp, sizeof(psk));
                    return 0;
                }
            } while (psk[bp - 1] != '\r');
            psk[bp - 1] = '\0';
            if ((bp - 1) < 8)
            {
                PRINTF("\n ERROR: KEY length=%d is less than 8 \r\n", (bp - 1));
                return 0;
            }
            break;
        }
        if (ch == '\r')
        {
            break;
        }
    } while (ch != 'y');
#endif
#endif
    //    PRINTF("\nMW320 uAP SSID=%s key=%s ip=%s \r\n", ssid, psk, network_ip);

    CLOCK_EnableXtal32K(kCLOCK_Osc32k_External);
    CLOCK_AttachClk(kXTAL32K_to_RTC);

    aesLock = xSemaphoreCreateRecursiveMutex();
    assert(aesLock != NULL);

    AES_Init(APP_AES);
    AES_SetLockFunc(APP_AES_Lock, APP_AES_Unlock);
    gpio_init();

    StartShellTask();

    /* Start FreeRTOS */
    vTaskStartScheduler();

    return 0;
}

bool lowPowerClusterSleep()
{
    return true;
}

static void OnOnOffPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    using namespace Clusters::OnOff::Attributes;

    VerifyOrExit(attributeId == OnOff::Id, ChipLogError(DeviceLayer, "Unhandled Attribute ID: '0x%04lx", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2, ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    led_on_off(led_yellow, (*value != 0) ? true : false);

exit:
    return;
}

static void OnSwitchAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    using namespace Clusters::Switch::Attributes;

    //	auto * pimEngine = chip::app::InteractionModelEngine::GetInstance();
    //	bool do_sendrpt = false;

    VerifyOrExit(attributeId == CurrentPosition::Id, ChipLogError(DeviceLayer, "Unhandled Attribute ID: '0x%04lx", attributeId));
    // Send the switch status report now
/*
        for (uint32_t i = 0 ; i<pimEngine->GetNumActiveReadHandlers() ; i++) {
                ReadHandler * phandler = pimEngine->ActiveHandlerAt(i);
                if (phandler->IsType(chip::app::ReadHandler::InteractionType::Subscribe) &&
                        (phandler->IsGeneratingReports() || phandler->IsAwaitingReportResponse())) {
                        phandler->ForceDirtyState();
                        do_sendrpt = true;
                        break;
                }
        }
        if (do_sendrpt == true) {
                ConcreteEventPath event_path(endpointId, Clusters::Switch::Id, 0);
                pimEngine->GetReportingEngine().ScheduleEventDelivery(event_path, chip::app::EventOptions::Type::kUrgent,
   sizeof(uint16_t));
        }
*/
exit:
    return;
}

uint32_t identifyTimerCount;
constexpr uint32_t kIdentifyTimerDelayMS = 250;
typedef struct _Identify_Timer
{
    EndpointId ep;
    uint32_t identifyTimerCount;
} Identify_Time_t;
Identify_Time_t id_time[MAX_ENDPOINT_COUNT];

void IdentifyTimerHandler(System::Layer * systemLayer, void * appState)
{
    using namespace Clusters::Identify::Attributes;

    Identify_Time_t * pidt = (Identify_Time_t *) appState;
    PRINTF(" -> %s(%u, %u) \r\n", __FUNCTION__, pidt->ep, pidt->identifyTimerCount);
    if (pidt->identifyTimerCount)
    {
        pidt->identifyTimerCount--;
        IdentifyTime::Set(pidt->ep, pidt->identifyTimerCount);
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), IdentifyTimerHandler, pidt);
    }
}

static void OnIdentifyPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    using namespace Clusters::Identify::Attributes;

    VerifyOrExit(attributeId == IdentifyTime::Id,
                 ChipLogError(DeviceLayer, "[%s] Unhandled Attribute ID: '0x%04lx", TAG, attributeId));
    VerifyOrExit((endpointId < MAX_ENDPOINT_COUNT),
                 ChipLogError(DeviceLayer, "[%s] EndPoint > max: [%u, %u]", TAG, endpointId, MAX_ENDPOINT_COUNT));
    if (id_time[endpointId].identifyTimerCount != *value)
    {
        id_time[endpointId].ep                 = endpointId;
        id_time[endpointId].identifyTimerCount = *value;
        PRINTF("-> Identify: %u \r\n", id_time[endpointId].identifyTimerCount);
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), IdentifyTimerHandler, &id_time[endpointId]);
    }

exit:
    return;
}

/*
        Callback to receive the cluster modification event
*/
void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & path, uint8_t type, uint16_t size, uint8_t * value)
{
    PRINTF("==> MatterPostAttributeChangeCallback, cluster: %x, attr: %x, size: %d \r\n", path.mClusterId, path.mAttributeId, size);
    // path.mEndpointId, path.mClusterId, path.mAttributeId, mask, type, size, value
    switch (path.mClusterId)
    {
    case Clusters::OnOff::Id:
        OnOnOffPostAttributeChangeCallback(path.mEndpointId, path.mAttributeId, value);
        break;
    case Clusters::Switch::Id:
        OnSwitchAttributeChangeCallback(path.mEndpointId, path.mAttributeId, value);
        // SwitchToggleOnOff();
        // Trigger to send on/off/toggle command to the bound devices
        chip::BindingManager::GetInstance().NotifyBoundClusterChanged(1, chip::app::Clusters::OnOff::Id, nullptr);
        break;
    case Clusters::Identify::Id:
        OnIdentifyPostAttributeChangeCallback(path.mEndpointId, path.mAttributeId, value);
        break;
    default:
        break;
    }
    return;
}

Protocols::InteractionModel::Status emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                                          const EmberAfAttributeMetadata * attributeMetadata,
                                                                          uint8_t * buffer)
{
    PRINTF("====> %s() \r\n", __FUNCTION__);
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                                         const EmberAfAttributeMetadata * attributeMetadata,
                                                                         uint8_t * buffer, uint16_t maxReadLength)
{
    // Added for the pairing of TE9 to report the commission_info
    // default function (in callback-stub.cpp)
    //
    PRINTF("-> %s()\n\r", __FUNCTION__);
    return Protocols::InteractionModel::Status::Success;
}
