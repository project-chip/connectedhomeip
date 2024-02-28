/*******************************************************************************
* @file  sl_net_default_values.h
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#pragma once

#include "sl_net_wifi_types.h"

#if defined(__GNUC__)
// Ignore warning of unused variables. It is expected that some or all of these are unused
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#ifdef SLI_SI91X_ENABLE_IPV6
#define REQUIRED_IP_TYPE SL_IPV6
#else
#define REQUIRED_IP_TYPE SL_IPV4
#endif

#ifndef DEFAULT_WIFI_CLIENT_PROFILE_SSID
#define DEFAULT_WIFI_CLIENT_PROFILE_SSID "YOUR_AP_SSID"
#endif

#ifndef DEFAULT_WIFI_CLIENT_CREDENTIAL
#define DEFAULT_WIFI_CLIENT_CREDENTIAL "YOUR_AP_PASSPHRASE"
#endif

#ifndef DEFAULT_WIFI_AP_PROFILE_SSID
#define DEFAULT_WIFI_AP_PROFILE_SSID "MY_AP_SSID"
#endif

#ifndef DEFAULT_WIFI_AP_CREDENTIAL
#define DEFAULT_WIFI_AP_CREDENTIAL "MY_AP_PASSPHRASE"
#endif

#ifndef DEFAULT_WIFI_CLIENT_SECURITY_TYPE
#define DEFAULT_WIFI_CLIENT_SECURITY_TYPE SL_WIFI_WPA2
#endif

#ifndef DEFAULT_WIFI_CLIENT_ENCRYPTION_TYPE
#define DEFAULT_WIFI_CLIENT_ENCRYPTION_TYPE SL_WIFI_CCMP_ENCRYPTION
#endif

//! IP address of the module
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#ifndef DEFAULT_WIFI_MODULE_IP_ADDRESS
#define DEFAULT_WIFI_MODULE_IP_ADDRESS 0x0A0AA8C0
#endif

//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#ifndef DEFAULT_WIFI_SN_MASK_ADDRESS
#define DEFAULT_WIFI_SN_MASK_ADDRESS 0x00FFFFFF
#endif

//! IP address of Gateway
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#ifndef DEFAULT_WIFI_GATEWAY_ADDRESS
#define DEFAULT_WIFI_GATEWAY_ADDRESS 0x0A0AA8C0
#endif

//! Wi-Fi BTR Channel
#define SL_CHANNEL_NO 14

//! Wi-Fi BTR default Tx power
#define SL_TX_POWER 127

//! Wi-Fi BTR config default values
#define DEFAULT_RETRANSMIT_COUNT 15
#define DEFAULT_QOS_BE_CWMIN     4
#define DEFAULT_QOS_BE_CWMAX     6
#define DEFAULT_QOS_BE_AIFSN     3
#define DEFAULT_QOS_BK_CWMIN     4
#define DEFAULT_QOS_BK_CWMAX     10
#define DEFAULT_QOS_BK_AIFSN     7
#define DEFAULT_QOS_VI_CWMIN     3
#define DEFAULT_QOS_VI_CWMAX     4
#define DEFAULT_QOS_VI_AIFSN     1
#define DEFAULT_QOS_VO_CWMIN     2
#define DEFAULT_QOS_VO_CWMAX     3
#define DEFAULT_QOS_VO_AIFSN     1

//! Wi-Fi BTR option to store peer information in MAC layer
#define MAC_PEER_DS_SUPPORT          1
#define FEAT_BTR_MAC_PEER_DS_SUPPORT BIT(13)

#define DEFAULT_WIFI_CLIENT_PROFILE \
  (sl_net_wifi_client_profile_t)    \
  {                                 \
    .config = { \
        .ssid.value = DEFAULT_WIFI_CLIENT_PROFILE_SSID, \
        .ssid.length = sizeof(DEFAULT_WIFI_CLIENT_PROFILE_SSID)-1, \
        .channel.channel = SL_WIFI_AUTO_CHANNEL, \
        .channel.band = SL_WIFI_AUTO_BAND, \
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH, \
        .bssid = {{0}}, \
        .bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE, \
        .security = DEFAULT_WIFI_CLIENT_SECURITY_TYPE, \
        .encryption = DEFAULT_WIFI_CLIENT_ENCRYPTION_TYPE, \
        .client_options = 0, \
        .credential_id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID, \
    }, \
    .ip = { \
        .mode = SL_IP_MANAGEMENT_DHCP, \
        .type = REQUIRED_IP_TYPE, \
        .host_name = NULL, \
        .ip = {{{0}}}, \
    }                  \
  }

#define DEFAULT_WIFI_ACCESS_POINT_PROFILE \
  (sl_net_wifi_ap_profile_t)              \
  {                                       \
    .config = { \
        .ssid.value = DEFAULT_WIFI_AP_PROFILE_SSID, \
        .ssid.length = sizeof(DEFAULT_WIFI_AP_PROFILE_SSID)-1, \
        .channel.channel = SL_WIFI_AUTO_CHANNEL, \
        .channel.band = SL_WIFI_AUTO_BAND, \
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH, \
        .security = SL_WIFI_WPA2, \
        .encryption = SL_WIFI_CCMP_ENCRYPTION, \
        .rate_protocol = SL_WIFI_RATE_PROTOCOL_AUTO, \
        .options = 0, \
        .credential_id = SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID, \
        .keepalive_type = SL_SI91X_AP_NULL_BASED_KEEP_ALIVE, \
        .beacon_interval = 100, \
        .client_idle_timeout = 0xFF, \
        .dtim_beacon_count = 3, \
        .maximum_clients = 3, \
        .beacon_stop = 0, \
        .tdi_flags =SL_WIFI_TDI_NONE, \
        .is_11n_enabled = 0, \
    }, \
    .ip = { \
      .mode      = SL_IP_MANAGEMENT_STATIC_IP, \
      .type      = SL_IPV4, \
      .host_name = NULL, \
      .ip        = { \
         .v4.ip_address.value = DEFAULT_WIFI_MODULE_IP_ADDRESS, \
         .v4.gateway.value    = DEFAULT_WIFI_GATEWAY_ADDRESS, \
         .v4.netmask.value    = DEFAULT_WIFI_SN_MASK_ADDRESS \
      }, \
    }                        \
  }

#define DEFAULT_WIFI_BTR_PROFILE                                                  \
  (sl_net_wifi_btr_profile_t)                                                     \
  {                                                                               \
    .config = {                                                                   \
      .btr_chan_info.chan_info        = { .channel   = SL_CHANNEL_NO,             \
                                          .band      = SL_WIFI_BAND_2_4GHZ,       \
                                          .bandwidth = SL_WIFI_BANDWIDTH_20MHz }, \
      .btr_chan_info.tx_power         = SL_TX_POWER,                              \
      .config_params.set              = 1,                                        \
      .config_params.retransmit_count = DEFAULT_RETRANSMIT_COUNT,                 \
      .config_params.cw_params[0]     = { .cwmin = DEFAULT_QOS_BE_CWMIN,          \
                                          .cwmax = DEFAULT_QOS_BE_CWMAX,          \
                                          .aifsn = DEFAULT_QOS_BE_AIFSN },        \
      .config_params.cw_params[1]     = { .cwmin = DEFAULT_QOS_BK_CWMIN,          \
                                          .cwmax = DEFAULT_QOS_BK_CWMAX,          \
                                          .aifsn = DEFAULT_QOS_BK_AIFSN },        \
      .config_params.cw_params[2]     = { .cwmin = DEFAULT_QOS_VI_CWMIN,          \
                                          .cwmax = DEFAULT_QOS_VI_CWMAX,          \
                                          .aifsn = DEFAULT_QOS_VI_AIFSN },        \
      .config_params.cw_params[3]     = { .cwmin = DEFAULT_QOS_VO_CWMIN,          \
                                          .cwmax = DEFAULT_QOS_VO_CWMAX,          \
                                          .aifsn = DEFAULT_QOS_VO_AIFSN }         \
    }                                                                             \
  }

static sl_net_wifi_psk_credential_entry_t default_wifi_client_credential = {
  .type        = SL_NET_WIFI_PSK,
  .data_length = sizeof(DEFAULT_WIFI_CLIENT_CREDENTIAL) - 1,
  .data        = DEFAULT_WIFI_CLIENT_CREDENTIAL
};

static sl_net_wifi_psk_credential_entry_t default_wifi_ap_credential = { .type = SL_NET_WIFI_PSK,
                                                                         .data_length =
                                                                           sizeof(DEFAULT_WIFI_AP_CREDENTIAL) - 1,
                                                                         .data = DEFAULT_WIFI_AP_CREDENTIAL };
#if defined(__GNUC__)
// Restore GCC diagnostics
#pragma GCC diagnostic pop
#endif
