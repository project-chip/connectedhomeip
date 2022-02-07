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
//#include "FreeRTOS.h"
//#include "task.h"

#include <lib/shell/Engine.h>

#include <platform/CHIPDeviceLayer.h>
#include <app/server/OnboardingCodesUtil.h>
#include <setup_payload/SetupPayload.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/RandUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <app/server/Server.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>

#include <ChipShellCollection.h>

/* platform specific */
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"

#include <wm_os.h>
extern "C" {
#include "wm_net.h"
#include "wlan.h"
#include "mflash_drv.h"
#include "dhcp-server.h"
#include "cli.h"
#include "ping.h"
#include "iperf.h"
#include "partition.h"
#include "boot_flags.h"
#include "network_flash_storage.h"
}
#include "lpm.h"
#include "fsl_aes.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_AES AES
#define CONNECTION_INFO_FILENAME "connection_info.dat"

/*******************************************************************************
 * Variables
 ******************************************************************************/
static SemaphoreHandle_t aesLock;
static struct wlan_network sta_network;
static struct wlan_network uap_network;

const int TASK_MAIN_PRIO       = OS_PRIO_3;
const int TASK_MAIN_STACK_SIZE = 800;
portSTACK_TYPE *task_main_stack = NULL;
TaskHandle_t task_main_task_handler;

#if CHIP_ENABLE_OPENTHREAD
extern "C" {
#include <openthread/platform/platform-softdevice.h>
}
#endif // CHIP_ENABLE_OPENTHREAD


using namespace chip;
using namespace chip::Shell;
using namespace chip::DeviceLayer;

void test_led_on(int argc, char **argv)
{
    PRINTF("led on\r\n");

	GPIO_PortClear(BOARD_LED_YELLOW_GPIO, GPIO_PORT(BOARD_LED_YELLOW_GPIO_PIN), 1u << GPIO_PORT_PIN(BOARD_LED_YELLOW_GPIO_PIN));
	GPIO_PortClear(BOARD_LED_AMBER_GPIO, GPIO_PORT(BOARD_LED_AMBER_GPIO_PIN), 1u << GPIO_PORT_PIN(BOARD_LED_AMBER_GPIO_PIN));
}

void test_led_off(int argc, char **argv)
{
    PRINTF("led off\r\n");

	GPIO_PortSet(BOARD_LED_YELLOW_GPIO, GPIO_PORT(BOARD_LED_YELLOW_GPIO_PIN), 1u << GPIO_PORT_PIN(BOARD_LED_YELLOW_GPIO_PIN));
	GPIO_PortSet(BOARD_LED_AMBER_GPIO, GPIO_PORT(BOARD_LED_AMBER_GPIO_PIN), 1u << GPIO_PORT_PIN(BOARD_LED_AMBER_GPIO_PIN));
}

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint16_t size, uint8_t * value)
{
    if (clusterId != ZCL_ON_OFF_CLUSTER_ID)
    {
        return;
    }

    if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
    {
        return;
    }

    if (*value)
    {
        test_led_on(0, NULL);
    }
    else
    {
        test_led_off(0, NULL);
    }
}

namespace {

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
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
int wlan_event_callback(enum wlan_event_reason reason, void *data)
{
    int ret;
    struct wlan_ip_config addr;
    char ip[16];
    static int auth_fail = 0;

    PRINTF("[%s] WLAN: received event %d\r\n", __FUNCTION__, reason);
    switch (reason)
    {
        case WLAN_REASON_INITIALIZED:
            PRINTF("app_cb: WLAN initialized\r\n");
            ret = dhcpd_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize DHCP Server CLI\r\n");
                return 0;
            }
            break;
        case WLAN_REASON_INITIALIZATION_FAILED:
            PRINTF("app_cb: WLAN: initialization failed\r\n");
            break;
        case WLAN_REASON_SUCCESS:
            PRINTF("app_cb: WLAN: connected to network\r\n");
            ret = wlan_get_address(&addr);
            if (ret != WM_SUCCESS)
            {
                PRINTF("failed to get IP address\r\n");
                return 0;
            }

            net_inet_ntoa(addr.ipv4.address, ip);

            ret = wlan_get_current_network(&sta_network);
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to get External AP network\r\n");
                return 0;
            }

            PRINTF("Connected to following BSS:\r\n");
            PRINTF("SSID = [%s], IP = [%s]\r\n", sta_network.ssid, ip);
#ifdef CONFIG_IPV6
            {
                int i;
                (void)PRINTF("\r\n\tIPv6 Addresses\r\n");
                for (i = 0; i < MAX_IPV6_ADDRESSES; i++)
                {
                    if (sta_network.ip.ipv6[i].addr_state != IP6_ADDR_INVALID)
                    {
                        (void)PRINTF("\t%-13s:\t%s (%s)\r\n", ipv6_addr_type_to_desc(&(sta_network.ip.ipv6[i])),
                             inet6_ntoa(sta_network.ip.ipv6[i].address), ipv6_addr_state_to_desc(sta_network.ip.ipv6[i].addr_state));
                    }
                }
                (void)PRINTF("\r\n");
            }
#endif
            auth_fail = 0;

			if (is_uap_started())
			{
				wlan_get_current_uap_network(&uap_network);
			    ret = wlan_stop_network(uap_network.name);
			    if (ret != WM_SUCCESS)
			        PRINTF("Error: unable to stop network\r\n");
				else
					PRINTF("stop uAP, SSID = [%s]\r\n", uap_network.ssid);
			}
            break;
        case WLAN_REASON_CONNECT_FAILED:
            PRINTF("app_cb: WLAN: connect failed\r\n");
            break;
        case WLAN_REASON_NETWORK_NOT_FOUND:
            PRINTF("app_cb: WLAN: network not found\r\n");
            break;
        case WLAN_REASON_NETWORK_AUTH_FAILED:
            PRINTF("app_cb: WLAN: network authentication failed\r\n");
            auth_fail++;
            if (auth_fail >= 3)
            {
                PRINTF("Authentication Failed. Disconnecting ... \r\n");
                wlan_disconnect();
                auth_fail = 0;
            }
            break;
        case WLAN_REASON_ADDRESS_SUCCESS:
            PRINTF("network mgr: DHCP new lease\r\n");
            break;
        case WLAN_REASON_ADDRESS_FAILED:
            PRINTF("app_cb: failed to obtain an IP address\r\n");
            break;
        case WLAN_REASON_USER_DISCONNECT:
            PRINTF("app_cb: disconnected\r\n");
            auth_fail = 0;
            break;
        case WLAN_REASON_LINK_LOST:
            PRINTF("app_cb: WLAN: link lost\r\n");
            break;
        case WLAN_REASON_CHAN_SWITCH:
            PRINTF("app_cb: WLAN: channel switch\r\n");
            break;
        case WLAN_REASON_UAP_SUCCESS:
            PRINTF("app_cb: WLAN: UAP Started\r\n");
            ret = wlan_get_current_uap_network(&uap_network);

            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to get Soft AP network\r\n");
                return 0;
            }

            PRINTF("Soft AP \"%s\" started successfully\r\n", uap_network.ssid);
            if (dhcp_server_start(net_get_uap_handle()))
                PRINTF("Error in starting dhcp server\r\n");
            PRINTF("DHCP Server started successfully\r\n");
            break;
        case WLAN_REASON_UAP_CLIENT_ASSOC:
            PRINTF("app_cb: WLAN: UAP a Client Associated\r\n");
            PRINTF("Client => ");
            print_mac((const char *)data);
            PRINTF("Associated with Soft AP\r\n");
            break;
        case WLAN_REASON_UAP_CLIENT_DISSOC:
            PRINTF("app_cb: WLAN: UAP a Client Dissociated\r\n");
            PRINTF("Client => ");
            print_mac((const char *)data);
            PRINTF("Dis-Associated from Soft AP\r\n");
            break;
        case WLAN_REASON_UAP_STOPPED:
            PRINTF("app_cb: WLAN: UAP Stopped\r\n");
            PRINTF("Soft AP \"%s\" stopped successfully\r\n", uap_network.ssid);
            dhcp_server_stop();
            PRINTF("DHCP Server stopped successfully\r\n");
             break;
        case WLAN_REASON_PS_ENTER:
            PRINTF("app_cb: WLAN: PS_ENTER\r\n");
            break;
        case WLAN_REASON_PS_EXIT:
            PRINTF("app_cb: WLAN: PS EXIT\r\n");
            break;
        default:
            PRINTF("app_cb: WLAN: Unknown Event: %d\r\n", reason);
    }
    return 0;
}

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
    payload.rendezvousInformation = RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE);
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

void uAP_init(void)
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

void gpio_init(void)
{
	gpio_pin_config_t led_config1 = {kGPIO_DigitalOutput, 0, };
    gpio_pin_config_t led_config2 = {kGPIO_DigitalOutput, 0, };
    gpio_pin_config_t sw_config = {kGPIO_DigitalInput, 0, };

    GPIO_PinInit(BOARD_LED_AMBER_GPIO, BOARD_LED_AMBER_GPIO_PIN, &led_config1);
    GPIO_PinInit(BOARD_LED_YELLOW_GPIO, BOARD_LED_YELLOW_GPIO_PIN, &led_config2);

	/* Init output LED GPIO off */
	GPIO_PortSet(BOARD_LED_YELLOW_GPIO, GPIO_PORT(BOARD_LED_YELLOW_GPIO_PIN), 1u << GPIO_PORT_PIN(BOARD_LED_YELLOW_GPIO_PIN));
	GPIO_PortSet(BOARD_LED_AMBER_GPIO, GPIO_PORT(BOARD_LED_AMBER_GPIO_PIN), 1u << GPIO_PORT_PIN(BOARD_LED_AMBER_GPIO_PIN));
	/* Init input switch GPIO. */
    GPIO_PinSetInterruptConfig(BOARD_SW1_GPIO, BOARD_SW1_GPIO_PIN, kGPIO_InterruptFallingEdge);
    GPIO_PortEnableInterrupts(BOARD_SW1_GPIO, GPIO_PORT(BOARD_SW1_GPIO_PIN), 1UL << GPIO_PORT_PIN(BOARD_SW1_GPIO_PIN));
    EnableIRQ(BOARD_SW1_IRQ);
    GPIO_PinInit(BOARD_SW1_GPIO, BOARD_SW1_GPIO_PIN, &sw_config);
}

void ShellCLIMain(void * pvParameter)
{
    flash_desc_t fl;
    struct partition_entry *p, *f1, *f2;
    short history = 0;
    uint32_t *wififw;
    struct partition_entry *psm;

    const int rc = streamer_init(streamer_get());
    if (rc != 0)
    {
        ChipLogError(Shell, "Streamer initialization failed: %d", rc);
        return;
    }

    ChipLogDetail(Shell, "Initializing CHIP shell commands: %d", rc);

    chip::Platform::MemoryInit();
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlag::kBLE);
    chip::DeviceLayer::PlatformMgr().StartEventLoopTask();
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    chip::DeviceLayer::ConnectivityManagerImpl().StartWiFiManagement();
#endif

    cmd_misc_init();
    //cmd_otcli_init();
    //cmd_ping_init();
    //cmd_send_init();

    ChipLogDetail(Shell, "Run CHIP shell Task: %d", rc);

    mcuInitPower();
    boot_init();
    mflash_drv_init();
    cli_init();
    part_init();
    psm = part_get_layout_by_id(FC_COMP_PSM, NULL);
    part_to_flash_desc(psm, &fl);
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

        wlan_init((const uint8_t *)(wififw + 2U), *(wififw + 1U));
        wlan_start(wlan_event_callback);
		uAP_init();
        os_thread_sleep(os_msec_to_ticks(5000));
    }

    std::string qrCodeText = createSetupPayload();
    PRINTF("SetupQRCode: [%s]\r\n", qrCodeText.c_str());

    // Init ZCL Data Model and CHIP App Server
    chip::Server::GetInstance().Init();

    Engine::Root().RunMainLoop();
}

} // namespace


int StartShellTask(void)
{
    int ret = 0;

    // Start Shell task.
    if (xTaskCreate(ShellCLIMain, "SHELL", TASK_MAIN_STACK_SIZE, NULL, TASK_MAIN_PRIO, &sShellTaskHandle)!= pdPASS)
    {
        ret = -1;
    }

    return ret;
}

int main(void)
{
    char ch;
    unsigned int bp;

    /* Initialize platform */
    //BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();

    BOARD_InitDebugConsole();
    PRINTF("\nMW320 uAP SSID=%s key=%s ip=%s \r\n", ssid, psk, network_ip);

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
