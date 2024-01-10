/***************************************************************************//**
 * @file
 * @brief NVM3 definition of the default data structures.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "nvm3.h"
#include "nvm3_hal_flash.h"
#include "nvm3_default_config.h"

#include "sl_wifi_device.h"

extern char linker_nvm_begin;
__attribute__((used)) uint8_t nvm3_default_storage[NVM3_DEFAULT_NVM_SIZE] __attribute__ ((section(".simee")));
#define NVM3_BASE (&linker_nvm_begin)

// NVM3 Default Instance Max Object Size
#define NVM3_DEFAULT_MAX_OBJECT_SIZE 4096

// WiFi Client Interface
#define SL_NET_WIFI_CLIENT_INTERFACE (1 << 3)

static nvm3_Handle_t  nvm3_defaultHandleData;

#if (NVM3_DEFAULT_CACHE_SIZE != 0)
static nvm3_CacheEntry_t defaultCache[NVM3_DEFAULT_CACHE_SIZE];
#endif


static nvm3_Init_t nvm3_defaultInitData =
{
  (nvm3_HalPtr_t)NVM3_BASE,
  0,
#if (NVM3_DEFAULT_CACHE_SIZE != 0)
  defaultCache,
#else
  NULL,
#endif
  NVM3_DEFAULT_CACHE_SIZE,
  NVM3_DEFAULT_MAX_OBJECT_SIZE,
  NVM3_DEFAULT_REPACK_HEADROOM,
  &nvm3_halFlashHandle,
};

static const sl_wifi_device_configuration_t station_init_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode              = SL_SI91X_CLIENT_MODE,
                   .coex_mode              = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map        = (SL_SI91X_FEAT_SECURITY_PSK | SL_SI91X_FEAT_AGGREGATION),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_HTTP_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID | SL_SI91X_TCP_IP_FEAT_OTAF
                                              | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT),
                   .custom_feature_bit_map = SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID,
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS |
#ifndef SLI_SI91X_MCU_INTERFACE
                      SL_SI91X_RAM_LEVEL_NWP_ALL_MCU_ZERO
#else
                      SL_SI91X_RAM_LEVEL_NWP_MEDIUM_MCU_MEDIUM
#endif
                      ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_FEAT_SSL_HIGH_PERFORMANCE | SL_SI91X_EXT_TCP_IP_SSL_16K_RECORD
                      | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
                   .config_feature_bit_map  = 0 }
};

//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------

nvm3_Handle_t *nvm3_defaultHandle = &nvm3_defaultHandleData;

nvm3_Init_t *nvm3_defaultInit = &nvm3_defaultInitData;

Ecode_t nvm3_initDefault(void)
{
  return nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);
}

Ecode_t nvm3_deinitDefault(void)
{
  return nvm3_close(nvm3_defaultHandle);
}

void setNvm3End(uint32_t end_addr)
{
  uint32_t size = NVM3_DEFAULT_NVM_SIZE;
  uint32_t nvm3_start_addr = (end_addr - size);
  nvm3_defaultInit->nvmAdr = (nvm3_HalPtr_t)nvm3_start_addr;
  nvm3_defaultInit->nvmSize = size;
  sl_status_t status      = SL_STATUS_OK;
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  nvm3_initDefault();
}