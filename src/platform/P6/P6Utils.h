/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
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

#pragma once

#include "platform/internal/DeviceNetworkInfo.h"
#include "whd_wlioctl.h"
#include <cy_wcm.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <whd_events_int.h>

typedef struct
{
    cy_wcm_ssid_t ssid;           /**< SSID of the Wi-Fi network to join; should be a null-terminated string. */
    cy_wcm_passphrase_t password; /**< Password needed to join the AP; should be a null-terminated string. */
    cy_wcm_security_t security;   /**< Wi-Fi Security. @see cy_wcm_security_t. */
    cy_wcm_mac_t BSSID;
    cy_wcm_ip_setting_t * static_ip_settings;
} wifi_config_sta_t;

typedef struct
{
    cy_wcm_ssid_t ssid;           /**< SSID of the Wi-Fi network to join; should be a null-terminated string. */
    cy_wcm_passphrase_t password; /**< Password needed to join the AP; should be a null-terminated string. */
    cy_wcm_security_t security;   /**< Wi-Fi Security. @see cy_wcm_security_t. */
    uint8_t channel;
    cy_wcm_ip_setting_t ip_settings;
} wifi_config_ap_t;

typedef struct
{
    wifi_config_sta_t sta; /**< configuration of STA */
    wifi_config_ap_t ap;   /**< configuration of AP */
} wifi_config_t;

typedef enum
{
    WIFI_MODE_NULL = 0, /**< null mode */
    WIFI_MODE_STA,      /**< WiFi station mode */
    WIFI_MODE_AP,       /**< WiFi soft-AP mode */
    WIFI_MODE_APSTA,    /**< WiFi station + soft-AP mode */
    WIFI_MODE_MAX
} wifi_mode_t;

typedef enum
{
    WIFI_IF_STA    = CY_WCM_INTERFACE_TYPE_STA,
    WIFI_IF_AP     = CY_WCM_INTERFACE_TYPE_AP,
    WIFI_IF_STA_AP = CY_WCM_INTERFACE_TYPE_AP_STA,
} wifi_interface_t;

/** MACSTAT counters for ucode (corerev >= 40) */
typedef struct
{
    /* MAC counters: 32-bit version of d11.h's macstat_t */
    uint32 txallfrm;         /**< total number of frames sent, incl. Data, ACK, RTS, CTS,
                              * Control Management (includes retransmissions)
                              */
    uint32 txrtsfrm;         /**< number of RTS sent out by the MAC */
    uint32 txctsfrm;         /**< number of CTS sent out by the MAC */
    uint32 txackfrm;         /**< number of ACK frames sent out */
    uint32 txdnlfrm;         /**< number of Null-Data transmission generated from template  */
    uint32 txbcnfrm;         /**< beacons transmitted */
    uint32 txfunfl[6];       /**< per-fifo tx underflows */
    uint32 txampdu;          /**< number of AMPDUs transmitted */
    uint32 txmpdu;           /**< number of MPDUs transmitted */
    uint32 txtplunfl;        /**< Template underflows (mac was too slow to transmit ACK/CTS
                              * or BCN)
                              */
    uint32 txphyerror;       /**< Transmit phy error, type of error is reported in tx-status for
                              * driver enqueued frames
                              */
    uint32 pktengrxducast;   /**< unicast frames rxed by the pkteng code */
    uint32 pktengrxdmcast;   /**< multicast frames rxed by the pkteng code */
    uint32 rxfrmtoolong;     /**< Received frame longer than legal limit (2346 bytes) */
    uint32 rxfrmtooshrt;     /**< Received frame did not contain enough bytes for its frame type */
    uint32 rxanyerr;         /**< Any RX error that is not counted by other counters. */
    uint32 rxbadfcs;         /**< number of frames for which the CRC check failed in the MAC */
    uint32 rxbadplcp;        /**< parity check of the PLCP header failed */
    uint32 rxcrsglitch;      /**< PHY was able to correlate the preamble but not the header */
    uint32 rxstrt;           /**< Number of received frames with a good PLCP
                              * (i.e. passing parity check)
                              */
    uint32 rxdtucastmbss;    /**< number of received DATA frames with good FCS and matching RA */
    uint32 rxmgucastmbss;    /**< number of received mgmt frames with good FCS and matching RA */
    uint32 rxctlucast;       /**< number of received CNTRL frames with good FCS and matching RA */
    uint32 rxrtsucast;       /**< number of unicast RTS addressed to the MAC (good FCS) */
    uint32 rxctsucast;       /**< number of unicast CTS addressed to the MAC (good FCS) */
    uint32 rxackucast;       /**< number of ucast ACKS received (good FCS) */
    uint32 rxdtocast;        /**< number of received DATA frames (good FCS and not matching RA) */
    uint32 rxmgocast;        /**< number of received MGMT frames (good FCS and not matching RA) */
    uint32 rxctlocast;       /**< number of received CNTRL frame (good FCS and not matching RA) */
    uint32 rxrtsocast;       /**< number of received RTS not addressed to the MAC */
    uint32 rxctsocast;       /**< number of received CTS not addressed to the MAC */
    uint32 rxdtmcast;        /**< number of RX Data multicast frames received by the MAC */
    uint32 rxmgmcast;        /**< number of RX Management multicast frames received by the MAC */
    uint32 rxctlmcast;       /**< number of RX Control multicast frames received by the MAC
                              * (unlikely to see these)
                              */
    uint32 rxbeaconmbss;     /**< beacons received from member of BSS */
    uint32 rxdtucastobss;    /**< number of unicast frames addressed to the MAC from
                              * other BSS (WDS FRAME)
                              */
    uint32 rxbeaconobss;     /**< beacons received from other BSS */
    uint32 rxrsptmout;       /**< number of response timeouts for transmitted frames
                              * expecting a response
                              */
    uint32 bcntxcancl;       /**< transmit beacons canceled due to receipt of beacon (IBSS) */
    uint32 rxnodelim;        /**< number of no valid delimiter detected by ampdu parser */
    uint32 rxf0ovfl;         /**< number of receive fifo 0 overflows */
    uint32 rxf1ovfl;         /**< number of receive fifo 1 overflows */
    uint32 rxhlovfl;         /**< number of length / header fifo overflows */
    uint32 missbcn_dbg;      /**< number of beacon missed to receive */
    uint32 pmqovfl;          /**< number of PMQ overflows */
    uint32 rxcgprqfrm;       /**< number of received Probe requests that made it into
                              * the PRQ fifo
                              */
    uint32 rxcgprsqovfl;     /**< Rx Probe Request Que overflow in the AP */
    uint32 txcgprsfail;      /**< Tx Probe Response Fail. AP sent probe response but did
                              * not get ACK
                              */
    uint32 txcgprssuc;       /**< Tx Probe Response Success (ACK was received) */
    uint32 prs_timeout;      /**< number of probe requests that were dropped from the PRQ
                              * fifo because a probe response could not be sent out within
                              * the time limit defined in M_PRS_MAXTIME
                              */
    uint32 txrtsfail;        /**< number of rts transmission failure that reach retry limit */
    uint32 txucast;          /**< number of unicast tx expecting response other than cts/cwcts */
    uint32 txinrtstxop;      /**< number of data frame transmissions during rts txop */
    uint32 rxback;           /**< blockack rxcnt */
    uint32 txback;           /**< blockack txcnt */
    uint32 bphy_rxcrsglitch; /**< PHY count of bphy glitches */
    uint32 rxdrop20s;        /**< drop secondary cnt */
    uint32 rxtoolate;        /**< receive too late */
    uint32 bphy_badplcp;     /**< number of bad PLCP reception on BPHY rate */
                             /* XXX:  All counter variables have to be of uint32. */
} wl_cnt_ge40mcst_v1_t;

typedef struct xtlv
{
    uint16_t id;
    uint16_t len;
    uint8_t data[1];
} xtlv_t;

#define WL_CNT_VER_30 (30)

/* XTLV Format parsing for wl counters support */
#define XTLV_OPTION_ALIGN32 0x0001 /* 32bit alignment of type.len.data */
#define XTLV_OPTION_IDU8 0x0002    /* shorter id */
#define XTLV_OPTION_LENU8 0x0004   /* shorted length */
#define OFFSETOF(type, member) ((uintptr_t) & ((type *) 0)->member)
#define _LTOH16_UA(cp) ((cp)[0] | ((cp)[1] << 8))
#define P6_ALIGN_SIZE(value, align_to) (((value) + (align_to) -1) & ~((align_to) -1))

#define CHK_CNTBUF_DATALEN(cntbuf, ioctl_buflen)                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        if (((wl_cnt_info_t *) cntbuf)->datalen + OFFSETOF(wl_cnt_info_t, data) > ioctl_buflen)                                    \
            printf("%s: IOVAR buffer short!\n", __FUNCTION__);                                                                     \
    } while (0)

typedef struct heap_info
{
    size_t HeapMax;
    size_t HeapUsed;
    size_t HeapFree;
} heap_info_t;

namespace chip {
namespace DeviceLayer {
namespace Internal {

class P6Utils
{
public:
    static CHIP_ERROR IsAPEnabled(bool & apEnabled);
    static CHIP_ERROR IsStationEnabled(bool & staEnabled);
    static bool IsStationProvisioned(void);
    static CHIP_ERROR IsStationConnected(bool & connected);
    static CHIP_ERROR StartWiFiLayer(void);
    static CHIP_ERROR EnableStationMode(void);
    static CHIP_ERROR SetAPMode(bool enabled);
    static int OrderScanResultsByRSSI(const void * _res1, const void * _res2);
    static const char * WiFiModeToStr(wifi_mode_t wifiMode);
    static struct netif * GetNetif(const char * ifKey);
    static struct netif * GetStationNetif(void);
    static bool IsInterfaceUp(const char * ifKey);
    static bool HasIPv6LinkLocalAddress(const char * ifKey);

    static CHIP_ERROR GetWiFiStationProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials);
    static CHIP_ERROR SetWiFiStationProvision(const Internal::DeviceNetworkInfo & netInfo);
    static CHIP_ERROR ClearWiFiStationProvision(void);
    static CHIP_ERROR p6_wifi_get_config(wifi_interface_t interface, wifi_config_t * conf);
    static CHIP_ERROR p6_wifi_set_config(wifi_interface_t interface, wifi_config_t * conf);
    static CHIP_ERROR p6_wifi_disconnect(void);
    static CHIP_ERROR p6_wifi_connect(void);
    static CHIP_ERROR p6_start_ap(void);
    static CHIP_ERROR p6_stop_ap(void);
    // Fills out some common fields in a wifi_config_t. Use interface param to
    // select whether sta or ap config is populated.
    static void populate_wifi_config_t(wifi_config_t * wifi_config, wifi_interface_t interface, const cy_wcm_ssid_t * ssid,
                                       const cy_wcm_passphrase_t * password, cy_wcm_security_t security = CY_WCM_SECURITY_OPEN);
    static CHIP_ERROR OnIPAddressAvailable(void);
    static CHIP_ERROR ping_init(void);
    static void unpack_xtlv_buf(const uint8_t * tlv_buf, uint16_t buflen, wl_cnt_ver_30_t * cnt, wl_cnt_ge40mcst_v1_t * cnt_ge40);
    static void heap_usage(heap_info_t * heap);
    static CHIP_ERROR GetWiFiSSID(char * buf, size_t bufSize);
    static CHIP_ERROR StoreWiFiSSID(char * buf, size_t size);
    static CHIP_ERROR GetWiFiPassword(char * buf, size_t bufSize);
    static CHIP_ERROR StoreWiFiPassword(char * buf, size_t size);
    static CHIP_ERROR GetWiFiSecurityCode(uint32_t & security);
    static CHIP_ERROR StoreWiFiSecurityCode(uint32_t security);
    static CHIP_ERROR wifi_get_mode(uint32_t & security);
    static CHIP_ERROR wifi_set_mode(uint32_t security);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
