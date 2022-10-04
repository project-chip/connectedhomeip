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

#ifndef _WFX_MSGS_H_
#define _WFX_MSGS_H_
/*
 * Taken from sl_wfx firmware - so I can re-use.
 * I need to do a better job than to use this stuff
 * in the CPP files of Matter
 */
typedef struct
{
    uint8_t octet[6]; ///< Table to store a MAC address
} sl_wfx_mac_address_t;
/**
 * @brief General Message header structure
 *
 */
typedef struct __attribute__((__packed__)) sl_wfx_header_s
{
    uint16_t length; ///< Message length in bytes including this uint16_t.
                     ///< Maximum value is 8188 but maximum Request size is FW dependent and reported in the
                     ///< ::sl_wfx_startup_ind_body_t::size_inp_ch_buf.
    uint8_t id;      ///< Contains the message Id indexed by sl_wfx_general_commands_ids_t or sl_wfx_message_ids_t.
    uint8_t info;    ///< TODO comment missing
} sl_wfx_header_t;

/**
 * @brief Generic message structure for all requests, confirmations and indications
 *
 */
typedef struct __attribute__((__packed__)) sl_wfx_generic_message_s
{
    sl_wfx_header_t header; ///< 4 bytes header
    uint8_t body[];         ///< variable size payload of the message
} sl_wfx_generic_message_t;
#define SL_WFX_OPN_SIZE 14
#define SL_WFX_UID_SIZE 8
#define SL_WFX_DISABLED_CHANNEL_LIST_SIZE 2
#define SL_WFX_FIRMWARE_LABEL_SIZE 128
/**
 * @brief Startup Indication message.
 * This is the first message sent to the host to confirm boot success.
 * It gives detailed information on the HW and FW versions and capabilities
 */
typedef struct __attribute__((__packed__)) sl_wfx_startup_ind_body_s
{
    uint32_t
        status; ///< Initialization status. A value of zero indicates the boot is completed successfully  (see enum sl_wfx_status_t)
    uint16_t hardware_id; ///<=RO misc_read_reg7 register value
#if 0                     /* Not used in RS911x for now - use stuff here for the port */
  uint8_t  opn[SL_WFX_OPN_SIZE];                           ///<=OTP part_OPN
  uint8_t  uid[SL_WFX_UID_SIZE];                           ///<=OTP UID
  uint16_t num_inp_ch_bufs;                                ///<Number of buffers available for request messages.
  uint16_t size_inp_ch_buf;                                ///<TX Buffer size in bytes=request message max size.
  uint8_t  num_links_aP;                                   ///<number of STA that are supported in AP mode
  uint8_t  num_interfaces;                                 ///<number of interfaces (WIFI link : STA or AP) that can be created by the user
  uint8_t  mac_addr[2][SL_WFX_MAC_ADDR_SIZE];              ///<MAC addresses derived from OTP
  uint8_t  api_version_minor;
  uint8_t  api_version_major;
  sl_wfx_capabilities_t capabilities;                      ///<List some FW options
  uint8_t  firmware_build;
  uint8_t  firmware_minor;
  uint8_t  firmware_major;
  uint8_t  firmware_type;                                  ///<See enum sl_wfx_fw_type_t
  uint8_t  disabled_channel_list[SL_WFX_DISABLED_CHANNEL_LIST_SIZE]; ///<=OTP Disabled channel list info
  sl_wfx_otp_regul_sel_mode_info_t regul_sel_mode_info;    ///<OTP region selection mode info
  sl_wfx_otp_phy_info_t otp_phy_info;                      ///<info on OTP backoff tables used to enforce the different DFS regulations.
  uint32_t supported_rate_mask;                            ///<A bit mask that indicates which rates are supported by the Physical layer. See enum api_rate_index.
  uint8_t  firmware_label[SL_WFX_FIRMWARE_LABEL_SIZE];     ///<Null terminated text string describing the loaded FW.
#else
    uint8_t mac_addr[6];
#endif
} sl_wfx_startup_ind_body_t;

typedef struct __attribute__((__packed__)) sl_wfx_startup_ind_s
{
    sl_wfx_header_t header;
    sl_wfx_startup_ind_body_t body;
} sl_wfx_startup_ind_t;
/**
 * @brief Indication message body for sl_wfx_connect_ind_t.
 */
typedef struct __attribute__((__packed__)) sl_wfx_connect_ind_body_s
{
    /**
     * @brief Status of the connection request.
     * @details <B>WFM_STATUS_SUCCESS</B>: the connection request was completed successfully.
     *          <BR><B>any other value</B>: the connection request failed.
     *          <BR>See sl_wfx_fmac_status_t for enumeration values.
     */
    uint32_t status;
    /**
     * @brief MAC address of the connected access point.
     */
    uint8_t mac[6];
    /**
     * @brief Channel of the connected access point.
     * @details <B>1 - 13</B>: Channel number.
     */
    uint16_t channel;
    /**
     * @brief Beacon Interval of the connected access point.
     */
    uint8_t beacon_interval;
    /**
     * @brief DTIM period of the connected access point.
     * @details <B>1 - 255</B>: DTIM period.
     */
    uint8_t dtim_period;
    /**
     * @brief Maximum PHY data rate supported by the connection.
     * @details See sl_wfx_rate_index_t for enumeration values.
     */
    uint16_t max_phy_rate;
} sl_wfx_connect_ind_body_t;

/**
 * @brief Indication message used to signal the completion of a connection operation.
 * @details The device will send this indication to signal the connection request initiated
 *          with sl_wfx_connect_req_t has been completed. The indication is also sent when
 *          the device autonomously roams to another access point.
 * @ingroup WFM_GROUP_MODE_IDLE
 */
typedef struct __attribute__((__packed__)) sl_wfx_connect_ind_s
{
    /** Common message header. */
    sl_wfx_header_t header;
    /** Indication message body. */
    sl_wfx_connect_ind_body_t body;
} sl_wfx_connect_ind_t;
/**
 * @brief Indication message body for sl_wfx_disconnect_ind_t.
 */
typedef struct __attribute__((__packed__)) sl_wfx_disconnect_ind_body_s
{
    /**
     * @brief MAC address of the access point.
     */
    uint8_t mac[6];
    /**
     * @brief Reason for disconnection.
     * @details <B>WFM_DISCONNECTED_REASON_UNSPECIFIED</B>: The device disconnected because of an internal error.
     *          <BR><B>WFM_DISCONNECTED_REASON_AP_LOST</B>: The device lost the AP beacons for too long.
     *          <BR><B>WFM_DISCONNECTED_REASON_REJECTED</B>: The device was disconnected by the AP.
     *          <BR><B>WFM_DISCONNECTED_REASON_LEAVING_BSS</B>: Disconnection was requested through the device API.
     *          <BR><B>WFM_DISCONNECTED_REASON_WPA_COUNTERMEASURES</B>: WPA countermeasures triggered a disconnection
     *          <BR>See sl_wfx_disconnected_reason_t for enumeration values.
     */
    uint16_t reason;
} sl_wfx_disconnect_ind_body_t;

/**
 * @brief Indication message used to signal the completion of a disconnection operation.
 * @details The device will send this indication to signal the disconnection request initiated
 *          with sl_wfx_disconnect_req_t has been completed. The indication is also sent when
 *          the device has lost the connection to an access point and has been unable to regain it.
 * @ingroup WFM_GROUP_MODE_STA
 */
typedef struct __attribute__((__packed__)) sl_wfx_disconnect_ind_s
{
    /** Common message header. */
    sl_wfx_header_t header;
    /** Indication message body. */
    sl_wfx_disconnect_ind_body_t body;
} sl_wfx_disconnect_ind_t;

#endif /* _WFX_MSGS_H_ */
