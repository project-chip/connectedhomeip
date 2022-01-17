/** @file wifi-internal.h
 *
 *  @brief WLAN Internal API
 *
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

#ifndef __WIFI_INTERNAL_H__
#define __WIFI_INTERNAL_H__

#include <mlan_api.h>

#include <wifi-decl.h>
#include <wifi_events.h>
#include <wm_os.h>

typedef struct
{
    int (*wifi_uap_set_params_p)();
    int (*wifi_uap_downld_domain_params_p)(MrvlIEtypes_DomainParamSet_t * dp);
    int (*wifi_uap_enable_11d_p)();
} wifi_uap_11d_apis_t;

typedef struct mcast_filter
{
    uint8_t mac_addr[MLAN_MAC_ADDR_LENGTH];
    struct mcast_filter * next;
} mcast_filter;

typedef struct
{
    os_thread_t wm_wifi_main_thread;
    os_thread_t wm_wifi_core_thread;
    os_queue_t * wlc_mgr_event_queue;

    void (*data_intput_callback)(const uint8_t interface, const uint8_t * buffer, const uint16_t len);
    void (*amsdu_data_intput_callback)(uint8_t interface, uint8_t * buffer, uint16_t len);
    void (*deliver_packet_above_callback)(t_u8 interface, t_void * lwip_pbuf);
    bool (*wrapper_net_is_ip_or_ipv6_callback)(const t_u8 * buffer);

    os_mutex_t command_lock;
    os_semaphore_t command_resp_sem;
    os_mutex_t mcastf_mutex;

    unsigned last_sent_cmd_msec;

    /* Queue for events/data from low level interface driver */
    os_queue_t io_events;
    os_queue_pool_t io_events_queue_data;

    mcast_filter * start_list;

    /*
     * Usage note:
     * There are a number of API's (for e.g. wifi_get_antenna()) which
     * return some data in the buffer passed by the caller. Most of the
     * time this data needs to be retrieved from the firmware. This
     * retrival happens in a different thread context. Hence, we need
     * to store the buffer pointer passed by the user at a shared
     * location. This pointer to used for this purpose.
     *
     * Note to the developer: Please ensure to set this to NULL after
     * use in the wifi driver thread context.
     */
    void * cmd_resp_priv;
    void * cmd_resp_ioctl;
    /*
     * In continuation with the description written for the
     * cmd_resp_priv member above, the below member indicates the
     * result of the retrieval operation from the firmware.
     */
    int cmd_resp_status;

    /*
     * Store 11D support status in Wi-Fi driver.
     */
    bool enable_11d_support;
    wifi_uap_11d_apis_t * uap_support_11d_apis;
    /*
     * This is updated when user calls the wifi_uap_set_domain_params()
     * functions. This is used later during uAP startup. Since the uAP
     * configuration needs to be done befor uAP is started we keep this
     * cache. This is needed to enable 11d support in uAP.
     */
    MrvlIEtypes_DomainParamSet_t * dp;
    /** Broadcast ssid control */
    t_u8 bcast_ssid_ctl;
    /** beacon period */
    t_u16 beacon_period;
    /** channel switch time to send ECSA */
    t_u8 chan_sw_count;
    /** Sniffer channel number */
    t_u8 chan_num;
    /** HT Capability Info */
    t_u16 ht_cap_info;
#ifdef CONFIG_WIFI_FW_DEBUG
    /** This function mount USB device.
     *
     * return WM_SUCCESS on success
     * return -WM_FAIL on failure.
     */
    int (*wifi_usb_mount_cb)();
    /** This function will open file for writing FW dump.
     *
     * \param[in] test_file_name Name of file to write FW dump data.
     *
     * \return WM_SUCCESS if opening of file is successful.
     * \return -WM_FAIL in case of failure.
     */
    int (*wifi_usb_file_open_cb)(char * test_file_name);
    /** This function will write data to file opened using wifi_usb_file_open_cb()
     *
     * \param[in] data Buffer containing FW dump data.
     * \param[in] data_len Length of data that needs to be written.
     *
     * \return WM_SUCCESS if write is successful
     * \return -WM_FAIL in case of failure.
     */
    int (*wifi_usb_file_write_cb)(uint8_t * data, size_t data_len);
    /** This function will close the file on which FW dump is written.
     *
     * \note This will close file that is opened using wifi_usb_file_open_cb().
     *
     * \return WM_SUCCESS on success.
     * \return -WM_FAIL on failure.
     */
    int (*wifi_usb_file_close_cb)();
#endif
} wm_wifi_t;

extern wm_wifi_t wm_wifi;

struct bus_message
{
    uint16_t event;
    uint16_t reason;
    void * data;
};

PACK_START struct ieee80211_hdr
{
    t_u16 frame_control;
    t_u16 duration_id;
    t_u8 addr1[6];
    t_u8 addr2[6];
    t_u8 addr3[6];
    t_u16 seq_ctrl;
    t_u8 addr4[6];
} PACK_END;

/**
 * This function handles events received from the firmware.
 */
int wifi_handle_fw_event(struct bus_message * msg);

/**
 * This function is used to send events to the upper layer through the
 * message queue registered by the upper layer.
 */
void wifi_event_completion(int type, enum wifi_event_reason result, void * data);

/**
 * Use this function to know whether a split scan is in progress.
 */
bool is_split_scan_complete(void);

/**
 * Waits for Command processing to complete and waits for command response
 */
int wifi_wait_for_cmdresp(void * cmd_resp_priv);

/**
 * Register an event queue
 *
 * This queue is used to send events and command responses to the wifi
 * driver from the stack dispatcher thread.
 */
int bus_register_event_queue(xQueueHandle * event_queue);

/**
 * De-register the event queue.
 */
void bus_deregister_event_queue(void);

/**
 * Register a special queue for WPS
 */
int bus_register_special_queue(xQueueHandle * special_queue);

/**
 * Deregister special queue
 */
void bus_deregister_special_queue(void);

/**
 * Register DATA input function with SDIO driver.
 *
 * This queue is used to DATA frames to the wifi
 * driver from the stack dispatcher thread.
 */
int bus_register_data_input_function(int (*wifi_low_level_input)(const uint8_t interface, const uint8_t * buffer,
                                                                 const uint16_t len));

/**
 * De-register the DATA input function with SDIO driver.
 */
void bus_deregister_data_input_function(void);

/*
 * @internal
 *
 *
 */
int wifi_get_command_lock(void);

/*
 * @internal
 *
 *
 */
int wifi_put_command_lock(void);

/*
 * Process the command response received from the firmware.
 *
 * Change the type of param below to HostCmd_DS_COMMAND after mlan
 * integration complete and then move it to header file.
 */
int wifi_process_cmd_response(HostCmd_DS_COMMAND * resp);

/*
 * @internal
 *
 *
 */
void * wifi_mem_malloc_cmdrespbuf(int size);

/*
 * @internal
 *
 *
 */
void * wifi_malloc_eventbuf(int size);
void wifi_free_eventbuf(void * buffer);

int wifi_mem_cleanup();
void wifi_uap_handle_cmd_resp(HostCmd_DS_COMMAND * resp);

mlan_status wrapper_moal_malloc(t_void * pmoal_handle, t_u32 size, t_u32 flag, t_u8 ** ppbuf);
mlan_status wrapper_moal_mfree(t_void * pmoal_handle, t_u8 * pbuf);

int wifi_sdio_lock();
void wifi_sdio_unlock();
mlan_status wrapper_wlan_cmd_mgmt_ie(int bss_type, void * buffer, unsigned int len, unsigned int action);
#endif /* __WIFI_INTERNAL_H__ */
