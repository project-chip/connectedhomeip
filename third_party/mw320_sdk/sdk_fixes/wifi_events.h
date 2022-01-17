/*
 *  Copyright 2008-2020 NXP
 *
 *  NXP CONFIDENTIAL
 *  The source code contained or described herein and all documents related to
 *  the source code ("Materials") are owned by NXP, its
 *  suppliers and/or its licensors. Title to the Materials remains with NXP,
 *  its suppliers and/or its licensors. The Materials contain
 *  trade secrets and proprietary and confidential information of NXP, its
 *  suppliers and/or its licensors. The Materials are protected by worldwide copyright
 *  and trade secret laws and treaty provisions. No part of the Materials may be
 *  used, copied, reproduced, modified, published, uploaded, posted,
 *  transmitted, distributed, or disclosed in any way without NXP's prior
 *  express written permission.
 *
 *  No license under any patent, copyright, trade secret or other intellectual
 *  property right is granted to or conferred upon you by disclosure or delivery
 *  of the Materials, either expressly, by implication, inducement, estoppel or
 *  otherwise. Any license under such intellectual property rights must be
 *  express and approved by NXP in writing.
 *
 */

/*! \file wifi_events.h
 * \brief Wi-Fi events
 */

#ifndef __WIFI_EVENTS_H__
#define __WIFI_EVENTS_H__

/** Wifi events */
enum wifi_event
{
    /** uAP Started */
    WIFI_EVENT_UAP_STARTED = 0,
    /** uAP Client Assoc */
    WIFI_EVENT_UAP_CLIENT_ASSOC,
    /** uAP Client De-authentication */
    WIFI_EVENT_UAP_CLIENT_DEAUTH,
    /** uAP Network Address Configuration */
    WIFI_EVENT_UAP_NET_ADDR_CONFIG,
    /** uAP Stopped */
    WIFI_EVENT_UAP_STOPPED,
    /** uAP Last */
    WIFI_EVENT_UAP_LAST,
    /* All the uAP related events need to be above and STA related events
     * below */
    /** Scan Result */
    WIFI_EVENT_SCAN_RESULT,
    /** Get hardware spec */
    WIFI_EVENT_GET_HW_SPEC,
    /** Association */
    WIFI_EVENT_ASSOCIATION,
    /** PMK */
    WIFI_EVENT_PMK,
    /** Authentication */
    WIFI_EVENT_AUTHENTICATION,
    /** Disassociation */
    WIFI_EVENT_DISASSOCIATION,
    /** De-authentication */
    WIFI_EVENT_DEAUTHENTICATION,
    /** Link Loss */
    WIFI_EVENT_LINK_LOSS,
    /** Network station address configuration */
    WIFI_EVENT_NET_STA_ADDR_CONFIG,
    /** Network interface configuration */
    WIFI_EVENT_NET_INTERFACE_CONFIG,
    /** WEP configuration */
    WIFI_EVENT_WEP_CONFIG,
    /** MAC address configuration */
    WIFI_EVENT_MAC_ADDR_CONFIG,
    /** Network DHCP configuration */
    WIFI_EVENT_NET_DHCP_CONFIG,
    /** Supplicant PMK */
    WIFI_EVENT_SUPPLICANT_PMK,
    /** Sleep */
    WIFI_EVENT_SLEEP,
    /** Awake */
    WIFI_EVENT_AWAKE,
    /** IEEE PS */
    WIFI_EVENT_IEEE_PS,
    /** Deep Sleep */
    WIFI_EVENT_DEEP_SLEEP,
    /** PS Invalid */
    WIFI_EVENT_PS_INVALID,
    /** HS configuration */
    WIFI_EVENT_HS_CONFIG,
    /** Error Multicast */
    WIFI_EVENT_ERR_MULTICAST,
    /** error Unicast */
    WIFI_EVENT_ERR_UNICAST,
    /* Add Block Ack */
    /** 802.11N add block ack */
    WIFI_EVENT_11N_ADDBA,
    /** 802.11N block Ack stream timeout */
    WIFI_EVENT_11N_BA_STREAM_TIMEOUT,
    /** 802.11n Delete block add */
    WIFI_EVENT_11N_DELBA,
    /** 802.11n aggregation control */
    WIFI_EVENT_11N_AGGR_CTRL,
    /** Channel Switch Announcement */
    WIFI_EVENT_CHAN_SWITCH_ANN,
    /** Channel Switch */
    WIFI_EVENT_CHAN_SWITCH,
#ifdef CONFIG_IPV6
    /** IPv6 address state change */
    WIFI_EVENT_NET_IPV6_CONFIG,
#endif
#ifdef CONFIG_WIFI_FW_DEBUG
    /* WiFi FW Debug Info */
    WIFI_EVENT_FW_DEBUG_INFO,
#endif
    /** Event to indicate end of Wi-Fi events */
    WIFI_EVENT_LAST,
    /* other events can be added after this, however this must
       be the last event in the wifi module */
};

/** WiFi Event Reason */
enum wifi_event_reason
{
    /** Success */
    WIFI_EVENT_REASON_SUCCESS,
    /** Timeout */
    WIFI_EVENT_REASON_TIMEOUT,
    /** Failure */
    WIFI_EVENT_REASON_FAILURE,
};

/** Network wireless BSS Type */
enum wlan_bss_type
{
    /** Station */
    WLAN_BSS_TYPE_STA = 0,
    /** uAP */
    WLAN_BSS_TYPE_UAP = 1,
    /** Any */
    WLAN_BSS_TYPE_ANY = 0xff,
};

/** Network wireless BSS Role */
enum wlan_bss_role
{
    /** Infrastructure network. The system will act as a station connected
     *  to an Access Point. */
    WLAN_BSS_ROLE_STA = 0,
    /** uAP (micro-AP) network.  The system will act as an uAP node to
     * which other Wireless clients can connect. */
    WLAN_BSS_ROLE_UAP = 1,
    /** Either Infrastructure network or micro-AP network */
    WLAN_BSS_ROLE_ANY = 0xff,
};

/** This enum defines various wakeup events
 * for which wakeup will occur */
enum wifi_wakeup_event_t
{
    /** Wakeup on broadcast  */
    WIFI_WAKE_ON_ALL_BROADCAST = 1,
    /** Wakeup on unicast  */
    WIFI_WAKE_ON_UNICAST = 1 << 1,
    /** Wakeup on MAC event  */
    WIFI_WAKE_ON_MAC_EVENT = 1 << 2,
    /** Wakeup on multicast  */
    WIFI_WAKE_ON_MULTICAST = 1 << 3,
    /** Wakeup on ARP broadcast  */
    WIFI_WAKE_ON_ARP_BROADCAST = 1 << 4,
    /** Wakeup on receiving a management frame  */
    WIFI_WAKE_ON_MGMT_FRAME = 1 << 6,
};

#endif /*__WIFI_EVENTS_H__*/
