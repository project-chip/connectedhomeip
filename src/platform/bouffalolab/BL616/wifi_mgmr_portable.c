#include <bl616.h>
#include <bl616_glb.h>

#include <FreeRTOS.h>
#include <lwip/tcpip.h>

#include <bl_fw_api.h>
#undef __INLINE
#undef __PACKED
#include <wifi_mgmr.h>

#include <wifi_mgmr_portable.h>

#define WIFI_STACK_SIZE (1536)
#define TASK_PRIORITY_FW (16)

static TaskHandle_t wifi_fw_task;
static netif_ext_callback_t netifExtCallback;

struct netif * deviceInterface_getNetif(void)
{
    LOCK_TCPIP_CORE();
    struct netif * net_if = netif_find("wl1");
    UNLOCK_TCPIP_CORE();

    return net_if;
}

int wifi_start_scan(const uint8_t * ssid, uint32_t length)
{
    wifi_mgmr_scan_params_t config;

    memset(&config, 0, sizeof(wifi_mgmr_scan_params_t));
    if (length && length <= MGMR_SSID_LEN)
    {
        memcpy(config.ssid_array, ssid, length);
        config.ssid_length = length;
    }

    return wifi_mgmr_sta_scan(&config);
}

void wifi_start_firmware_task(void)
{
    memset(&netifExtCallback, 0, sizeof(netifExtCallback));

    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_IP_WIFI_PHY | GLB_AHB_CLOCK_IP_WIFI_MAC_PHY | GLB_AHB_CLOCK_IP_WIFI_PLATFORM);
    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_WIFI);

    extern void interrupt0_handler(void);
    bflb_irq_attach(WIFI_IRQn, (irq_callback) interrupt0_handler, NULL);
    bflb_irq_enable(WIFI_IRQn);

    LOCK_TCPIP_CORE();
    netif_add_ext_callback(&netifExtCallback, network_netif_ext_callback);
    UNLOCK_TCPIP_CORE();

    xTaskCreate(wifi_main, (char *) "fw", WIFI_STACK_SIZE, NULL, TASK_PRIORITY_FW, &wifi_fw_task);
}
