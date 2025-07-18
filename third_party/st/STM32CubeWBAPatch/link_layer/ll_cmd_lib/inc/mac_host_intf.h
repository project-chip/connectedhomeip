/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/mac_host_intf.h#1 $*/
/**
 ********************************************************************************
 * @file    mac_host_intf.h
 * @brief   The file include description for the MAC services interface and call backs ,
 * 			MAC layer provide software implementation for 802.15.4 standard primitives
 *
 ******************************************************************************
 * @copy
 * This Synopsys DWC Bluetooth Low Energy Combo Link Layer/MAC software and
 * associated documentation ( hereinafter the "Software") is an unsupported
 * proprietary work of Synopsys, Inc. unless otherwise expressly agreed to in
 * writing between Synopsys and you. The Software IS NOT an item of Licensed
 * Software or a Licensed Product under any End User Software License Agreement
 * or Agreement for Licensed Products with Synopsys or any supplement thereto.
 * Synopsys is a registered trademark of Synopsys, Inc. Other names included in
 * the SOFTWARE may be the trademarks of their respective owners.
 *
 * Synopsys MIT License:
 * Copyright (c) 2020-Present Synopsys, Inc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * the Software), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING, BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE ARISING FROM,
 * OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * */

#ifndef INCLUDE_MAC_HOST_INTF_H_
#define INCLUDE_MAC_HOST_INTF_H_
#include "common_types.h"
#include "ll_fw_config.h"
#include "rfd_dev_config.h"
#include "error.h"
#include  "stdint.h"
/*====================================   MACROS   =======================================*/
/* Size in octets of frame counter used in security processing */
#define FRM_CNTR_SIZE 				4
/* Size in octets of key src in case of key id mode 2 used in security processing */
#define KEY_SRC_SIZE_MOD_2 			4
/* Size in octets of key src in case of key id mode 3 used in security processing */
#define KEY_SRC_SIZE_MOD_3 			8
/* Security Key ID Modes  */
#define MODE_ZERO					0
#define MODE_ONE					1
#define MODE_TWO					2
#define MODE_THREE					3

/**security parameters */
#if SUPPORT_SEC
#define KEY_SOURCE_SIZE    				 8
#define KEY_ID_LOOKUP_DSCRP_LIST_SIZE    3
#define MAX_NUM_OF_DEVICES_USE_KEY       8
#define KEY_USAGE_LIST_SIZE              5
#define KEY_SIZE                        16
#define MAC_KEY_TBL_STRUCT_SIZE_PLUS_OVERHEAD	(sizeof(key_dscrp_st_t) + 7) /*adding 7 bytes for header parameters ( 1 opcode + 2 length + 1 mac_handle + 1 pib_attributes + 1 pib_attribute_index + 1 pib_attribute_length )*/
#endif /*SUPPORT_SEC */


/* The maximum number of octets that can be transmitted in the MAC Payload field (MAX_PHY_PACKET_SIZE  MIN_PDU_OVERHEAD)*/
#define MAX_MAC_SAFE_PAYLOAD_SIZE       118
#define	MAX_HDR_IE_SIZE				3
/* EB payload IE length = 19 + 2 = 21 octets
 * Tx power IE length = 6 octets */
#define MAX_ZIGBEE_EB_IE_LEN				27 // maximum allowed IE Size for Enhanced Beacon
/*
 * False vendor specific IE: 5 octets without any vendor specific info only OUI
 * False MLME nested: 3 for our testing <not random length>
 * used for zigbee certification
 * */
#define MAX_ADDITIONAL_ZIGBEE_PYLDIE_SIZE	8
/* Rejoin IE  = 12 + 3 hdr = 15 octets
 * Tx power IE length = 1 + 3 hdr octets
 * Vendor specific IE hdr = 5 octets
 * */
#define MAX_ZIGBEE_EBR_IE_LEN				(20 + MAX_ADDITIONAL_ZIGBEE_PYLDIE_SIZE)


#define MAX_BEACON_FRAME_PENDING_ADDRESSES  7
/**
 * @brief Enumeration representing any error codes that could result from calling a MAC API
 **/
typedef enum {
	BEACON_LOSS = 0xE0,
	CHANNEL_ACCESS_FAILURE = 0xE1,
	COUNTER_ERROR = 0xDB,
	DENIED = 0xE2,
	DISABLE_TRX_FAILURE = 0xE3,
	SECURITY_ERROR = 0xE4,
	FRAME_TOO_LONG = 0xE5,
	IMPROPER_KEY_TYPE = 0xDC,
	IMPROPER_SECURITY_LEVEL = 0xDD,
	INVALID_ADDRESS = 0xF5,
	INVALID_GTS = 0xE6,
	INVALID_HANDLE = 0xE7,
	INVALID_INDEX = 0xF9,
	INVALID_PARAMETER = 0xE8,
	NO_ACK = 0xE9,
	NO_BEACON = 0xEA,
	NO_DATA = 0xEB,
	NO_SHORT_ADDRESS = 0xEC,
	ON_TIME_TOO_LONG = 0xF6,
	OUT_OF_CAP = 0xED,
	PAN_ID_CONFLICT = 0xEE,
	PAST_TIME = 0xF7,
	READ_ONLY = 0xFB,
	REALIGNMENT = 0xEF,
	SCAN_IN_PROGRESS = 0xFC,
	SUPERFRAME_OVERLAP = 0xFD,
	TRACKING_OFF = 0xF8,
	TRANSACTION_EXPIRED = 0xF0,
	TRANSACTION_OVERFLOW = 0xF1,
	TX_ACTIVE = 0xF2,
	UNAVAILABLE_KEY = 0xF3,
	UNSUPPORTED_ATTRIBUTE = 0xF4,
	UNSUPPORTED_LEGACY = 0xDE,
	UNSUPPORTED_SECURITY = 0xDF,
	MAC_LIMIT_REACHED = 0xFA,
	UNAVAILABLE_DEVICE,
	UNAVAILABLE_SECURITY_LEVEL,
	RANGING_NOT_SUPPORTED,
	INTERNAL_ERROR,
	CONDITINALLY_PASSED,
	MAC_STATUS_SUCCESS = 0,
} mac_status_enum_t;

/* MAC PIBs IDs */
typedef enum {
	ACK_WAIT_DUR_ID = 0x40,
	ASSOC_PAN_COORD_ID = 0x56,
	ASSOC_PERMIT_ID = 0x41,
	AUTO_REQ_ID = 0x42,
	BEACON_PAY_ID = 0x45,
	BECON_PAY_LEN_ID = 0x46,
	BEACON_ORDER_ID = 0x47,
	BEACON_TX_TIME_ID = 0x48,
	BSN_ID = 0x49,
	COORD_EXT_ADDR_ID = 0x4A,
	COORD_SHRT_ADDR_ID = 0x4B,
	DSN_ID = 0x4C,
	MAX_BE_ID = 0x57,
	MAX_CSMA_ID = 0x4E,
	MAX_FRAME_WAIT_TIME_ID = 0x58,
	MAX_FRAME_RETRY_ID = 0x59,
	MIN_BE_ID = 0x4F,
	PAN_ID_ID = 0x50,
	PROMIS_MODE_ID = 0x51,
	RSP_WAIT_TIME_ID = 0x5A,
	RX_WHEN_IDLE_ID = 0x52,
	SEC_ENABLED_ID = 0X5D,
	SHORT_ADDR_ID = 0X53,
	SUPER_FRAME_ORD_ID = 0X54,
	SYNC_SYMB_OFFS_ID = 0X5B,
	TIME_STAMP_SUPP_ID = 0X5C,
	PERSISTEN_TIME_ID = 0X55,
	PHY_CHANNEL_ID = 0x00,
	PHY_CHANNEL_SUPPORTED = 0x01,
	PHY_CCA_MODE = 0x03,
	PHY_CURRENT_PAGE = 0x04,
	PHY_MAX_FRAME_DUR = 0x05,
	PHY_SHR_DUR = 0x06,
	PHY_SYMBOLS_PER_OCTET = 0x07,
	KEY_TBL_ID = 0x71,
	DEVICE_TBL_ID = 0x73,
	SEC_LVL_TBL_ID = 0x75,
	FRM_CNTR_ID = 0x77,
	AUTO_REQ_SEC_LVL_ID = 0x78,
	AUTO_REQ_KEY_MODE_ID = 0x79,
	AUTO_REQ_KEY_SRC_ID = 0x7A,
	AUTO_REQ_KEY_INDX_ID = 0x7B,
	DFLT_KEY_SRC_ID = 0x7C,
	PAN_COORD_EXT_ADDR_ID = 0x7D,
	PAN_COORD_SHRT_ADDR_ID = 0x7E,
	/* for the sake of testing used only for customization for setting ext address instead of random by the
	 * values of the security test vectors */
	CUSTOM_IEEE_EXTD_ADDRS = 0x7F,
	EB_HDR_IE_LIST_ID = 0x80,
	EB_PYLD_IE_LIST_ID = 0x81,
	EB_FLTR_Enbld_ID = 0x82,
	EBSN_ID = 0x83,
	EB_AUTO_SA_ID = 0x84,
	EBR_PRMT_JOIN_ID = 0x85,
	EBR_FLTRS_ID = 0x86,
	EBR_LQ_ID = 0x87,
	EBR_PRCNT_FLTR_ID = 0x88,
	EBR_AUTO_RSPND_ID = 0x89,
	MAX_FULL_CSMA_FRAME_RETRY_ID = 0x8A,
	IMPLICIT_BROADCAST_ID = 0x8B,
	MIB_JOINING_IEEE_LIST_ID = 0xA0,
	MIB_JOINING_POLICY_ID = 0xA1,
	MIB_EXPIRY_INTRVL_ID = 0xA2,
	MIB_EXPIRY_INTRVL_COUNTDOWN_ID = 0xA3
} mac_pib_id;

/**
 * @brief enum represents device address mode defined in 802.15.4 std
 **/
typedef enum {
	NO_ADDRS,
	RESERVED,
	SHORT_ADDRS_16,
	EXT_ADDRS_64,
	NOT_USED =0xFF
} mac_addrs_mode_enum_t;

/**
 * @brief enum represents scanning types defined in 802.15.4 std
 **/
typedef enum {
	ED_SCAN, MAC_ACTIV_SCAN, MAC_PASS_SCAN, ORPH_SCAN, ENHANCED_ACTIV_SCAN
} scn_type_t;

/**
 * @brief enum represents Beacon Frame types defined in 802.15.4 std
 **/
typedef enum {
	NORMAL_BEACON = 0x00, ENHANCED_BEACON = 0x01
} bcon_typ_t;

typedef enum {
	ASSOC_REQ = 0x01,
	ASSOC_RSP = 0x02,
	DISASSOC_NOTF = 0x03,
	DATA_REQ = 0x04,
	PAN_ID_CONF_NOTF = 0x05,
	ORPH_NOTF = 0x06,
	BEACON_REQ = 0x07,
	COORD_REALIGN = 0x08
} mac_cmd_frm_t;

/**
 * @brief enum represents Frame types defined in 802.15.4 std
 **/
typedef enum {
	BEACON = 0x00, DATA = 0x01, ACK = 0x02, COMMAND = 0x03, FT_RESERVED = 0x04, FT_MULTIPURPOSE = 0x5, FT_FRAK = 0x6, FT_EXTENDED = 0x7, NO_FRAME=0xFF
} mac_frm_t;



/**
 * @brief indicates the actual frame protection that is provided. This value can be adapted on a frame-by-frame basis and allows for
 *        varying levels of data authenticity (to allow minimization of security overhead in transmitted frames where required) and for
 *        optional data confidentiality. The cryptographic protection offered by the various security levels is shown in Table 95.
 *        When nontrivial protection is required, replay protection is always provided.
 **/
typedef enum security_level_enum {
	no_security,
	mic_32_sec_level,
	mic_64_sec_level,
	mic_128_sec_level,
	enc_sec_level,
	enc_mic_32_sec_level,
	enc_mic_64_sec_level,
	enc_mic_128_sec_level,
	sec_lvls_num
} sec_level_enum_t;
/**
 * @brief struct conatins PIB attribute data
 **/
typedef struct attr_arr_e {
	uint8_t* data;
	uint16_t length;
} attr_arr_t;
/**
 * @brief union contains PIB attribute value [bool/ single int / octet string]
 **/
typedef union attr_value_e {
	uint32_t attr_int;
	uint32_t attr_bool;
	attr_arr_t attr_array;
} attr_val_t;

/**
 * @brief structure represents common security parameters
 **/
typedef struct prim_sec_param_e {
	uint32_t frm_cntr;
	uint8_t key_src[KEY_SRC_SIZE_MOD_3];
	sec_level_enum_t sec_level;
	uint8_t key_indx;
	uint8_t key_id_mod;
} prim_sec_param_st;

#if SUPPORT_SEC
typedef union _dev_addrs_un
{
	uint8_t  dev_addrs_arr[EXT_ADDRESS_LENGTH];
	uint16_t shrt_addrs;

}dev_addrs_un_t;

/**
 * @brief Security device descriptor list element structure
 *
 **/
typedef struct _device_dscrp_st
{
	uint16_t      device_pan_id;
	uint16_t      device_short_addrs;
	uint8_t       dev_extd_addrs_arr[EXT_ADDRESS_LENGTH];
	uint32_t      incoming_frame_counter;
	uint8_t       exempt_min_sec_level;
	uint8_t		  tbl_set_flag; /*TRUE means that this entry is used, FALSE means this entry not used*/
}device_dscrp_st_t;

typedef struct _key_id_lookup_dscrp_st
{
	uint32_t key_index ; /* allows unique identification of different keys with the same originator */
	dev_addrs_un_t un_dev_addrs;
	uint16_t dev_pan_id;
	uint8_t  key_id_mode;        /* The mode used to for this descriptor. */
	uint8_t key_src_arr[KEY_SOURCE_SIZE]; /* indicates the originator of a group key. Present only if KeyIdMode is equal to 0x02 or 0x03 */
	uint8_t  enum_device_addrs_mode; //range values from  mac_addrs_mode_enum_t enum (should be set to NOT_USED=0xFF when this entry isn't being used )

}key_id_lookup_dscrp_st_t;

/* indicating the frame types with which this key may be used */
typedef struct _key_usage_dscrp_st
{
	uint8_t     enum_mac_frm_type;  //range values from mac_frm_t enum
	uint8_t 	enum_cmd_frm_id;	//range value from mac_cmd_frm_t enum
}key_usage_dscrp_st_t;

/**
 * @brief Security key descriptor list element structure
 *
 **/
typedef struct _key_dscrp_st
{
	key_id_lookup_dscrp_st_t key_id_lookup_dscrp_list[KEY_ID_LOOKUP_DSCRP_LIST_SIZE];
	device_dscrp_st_t        device_dscrp_list[MAX_NUM_OF_DEVICES_USE_KEY];
	key_usage_dscrp_st_t     key_usage_dscrp_st_list[KEY_USAGE_LIST_SIZE];
	uint8_t                  key_arr[KEY_SIZE];
	uint8_t		  			 tbl_set_flag; /*TRUE means that this entry is used, FALSE means this entry not used*/
}key_dscrp_st_t;

/**
 * @brief enum represents Security table types that can be used in mlme-remove-sec_table
 **/
typedef enum security_tbl_enum {
	MAC_SEC_LEVEL_TABLE,
	MAC_DEVICE_TABLE,
	MAC_KEY_TABLE,
	KEY_ID_LOOKUP_DSCRP_LIST,
	DEVICE_DSCRP_LIST,
	KEY_USAGE_DSCRP_ST_LIST
} sec_tbl_enum_t;

#endif /*SUPPORT_SEC */

/**
 * @brief structure represents pan descriptor parameters
 **/
typedef struct pan_descr_e {
#if SUPPORT_SEC
	/* Security Information */
	prim_sec_param_st sec_params;
#endif
	uint32_t time_stamp;
	/** The address of the coordinator as specified in the received beacon frame */
	uint8_t coord_addr[EXT_ADDRESS_LENGTH];
	/** The PAN ID of the coordinator as specified in the received beacon frame */
	uint16_t coord_pan_id;
	/** Superframe specification */
	uint16_t super_frm_spec;
	/** The coordinator addressing mode corresponding to the received beacon frame */
	uint8_t coord_addr_mod;
	/** The current channel number occupied by the network */
	uint8_t logic_chanl;
	/** The current channel page occupied by the network */
	uint8_t chnl_pge;
	/** The LQI at which the network beacon was received */
	uint8_t link_qual;
	/* Set to one  if the beacon is from the PAN coordinator that is accepting GTS requests*/
	uint8_t gts_perm;
	/* SUCCESS if there was no error in the security processing of the frame otherwise check error code */
	uint8_t sec_fail_status;
} pan_descr_st;

/**
 * @brief structure represents MLME-SCAN.confirm parameters
 **/
typedef struct mlme_scan_conf_param_e {
	/** A list of the channels given in the request which were not scanned */
	uint32_t unscan_chnls;
	/** The list of energy measurements, one for each channel searched during an ED scan */
	uint8_t* enrgy_detec_lst;
	/** The list of PAN descriptors, one for each beacon found during an active or passive scan
	 if macAutoRequest is set to TRUE */
	pan_descr_st* pan_descr_lst;
	/** Scan type */
	scn_type_t scn_type;
	/** The status of the scan request */
	uint8_t status;
	/** The channel page on which the scan was performed */
	uint8_t chnl_pge;
	/** The number of elements returned in the appropriate result lists */
	uint8_t rslt_size;
} mlme_scn_cmf_param_st;

/**
 * @brief structure represents MLME-BEACON-NOTIFY.indication parameters
 **/
typedef struct mlme_bcon_notfy_params_e {
	/** Pointer to the PANDescriptor for the received Beacon frame.*/
	pan_descr_st* pan_desc_ptr;
	uint8_t addr_list [EXT_ADDRESS_LENGTH * MAX_BEACON_FRAME_PENDING_ADDRESSES + MAX_BEACON_FRAME_PENDING_ADDRESSES * EXT_ADDRESS_LENGTH] ;
	/** The set of octets comprising the beacon payload to be transferred from the MAC
	 *sublayer entity to the next higher layer */
	uint8_t* sdu;
	/** Pointer to header IEs */
	uint8_t* ptr_hdr_ie_list;
	/**Pointer to payload IEs */
	uint8_t* ptr_pyld_ie_list;
	/** Indicates a beacon(value 0x00) or enhanced beacon (value 0x01) was received.*/
	uint8_t bcon_type;
	/** Number of header IEs  */
	uint8_t hdr_ie_list_count;
	/** Number of payload IEs*/
	uint8_t pyld_ie_list_count;
	/** Bits (0-2) indicates Number of Short Addresses Pending ,
	  * Bits (4-6) indicates Number of Extended Addresses Pending Bits 3,7 are reserved */
	uint8_t pend_addrs;
	/** The beacon sequence number (0x00 - 0xff) */
	uint8_t bsn;
	/** BSN used for Enhanced Beacon frames (0x00 - 0xff) */
	uint8_t ebsn;
	/** The number of octets contained in the beacon payload of the beacon frame
	 * received by the MAC sublayer */
	uint8_t sdu_length;
} mlme_bcon_notfy_params_st;

/**
 * @brief structure represents pan MLME-SYNC-LOSS.indication parameters
 **/
typedef struct mlme_sync_loss_params_e {
	/** Security Information */
	prim_sec_param_st sec_params;
	/** The PAN ID with which the device lost synchronization or to which it was realigned.*/
	uint16_t pan_id;
	/** The reason that synchronization was lost */
	uint8_t loss_reason;
	/** The channel number on which the device lost synchronization or to which it was realigned */
	uint8_t chnl_num;
	/** The channel page on which the device lost synchronization or to which it was realigned */
	uint8_t chnl_pge;
} mlme_sync_loss_params_st_t;

/* MCPS-DATA.indication parameters  primitive is generated by the MAC sublayer and issued to the SSCS on receipt
 of a data frame at the local MAC sublayer entity */
typedef struct {
	/* Optional. The time, in symbols, at which the data were received */
	uint32_t timestamp;
	/* The individual device address of the entity from which the MSDU was received.*/
	uint8_t src_addrs[EXT_ADDRESS_LENGTH];
	/* The individual device address of the entity to which the MSDU is being transferred */
	uint8_t dstn_addrs[EXT_ADDRESS_LENGTH];
	/* The 16-bit PAN identifier of the entity from which the MSDU was received. */
	uint16_t src_pan_id;
	/* The 16-bit PAN identifier of the entity to which the MSDU is being transferred. */
	uint16_t dstn_pan_id;
	/* The destination addressing mode for this primitive corresponding to the received MPDU. This value can take one of the
	 following values:
	 0x00 = no address, 0x01 = reserved, 0x02 = 16-bit short address, 0x03 = 64-bit extended address. */
	mac_addrs_mode_enum_t enum_dstn_addrs_mode;
	/* The source addressing mode for this primitive corresponding to the received MPDU.. This value can take one of the following values:
	 * 0x00 = no address, 0x01 = reserved, 0x02 = 16-bit short address, 0x03 = 64-bit extended address.*/
	mac_addrs_mode_enum_t enum_src_addrs_mode;
	/* The number of octets contained in the MSDU being indicated by the MAC sublayer entity.*/
	uint8_t msdu_len;
	/* The set of octets forming the MSDU being indicated by the MAC sublayer entity..*/
	uint8_t *ptr_msdu;
	/*  The DSN of the received data frame.*/
	uint8_t dsn;
	/* link quality value */
	uint8_t mpdu_link_qlty;
	/* RSSI */
	uint8_t mpdu_rssi;
	/* The security level purportedly used by the received data frame */
	sec_level_enum_t enum_mcps_security_level;
#if SUPPORT_SEC
	/* The mode used to identify the key purportedly used by the originator of the received frame */
	uint8_t key_id_mode;
	/* The index of the key purportedly used by the originator of the received frame */
	uint8_t key_index;
	/* The originator of the key purportedly used by the originator of the received frame */
	uint8_t key_src[8];
#endif
} mcps_indicate_params_st_t;

/* MLME Association Status */
typedef enum mlme_assoc_status_enum {
	ASSOCIATION_SUCCESS = 0x00, AT_CAPACITY = 0x01, ACCESS_DENIED = 0x02,
} mlme_assoc_status_enum_t;

#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_ASSOCIATION_IND_RSP)
/**
 * @brief structure represents parameters for MLME-ASSOCIATE.indication
 **/
typedef struct mlme_assoc_ind_params_e {
	prim_sec_param_st sec_params; /**< security information */
	uint8_t dev_addr[EXT_ADDRESS_LENGTH]; /**< The address of the device requesting association */
	uint8_t cap_info; /**< The operational capabilities of the device requesting association */
} mlme_assoc_ind_param_st;

/**
 * @brief structure represents MLME-ASSOCIATE.response input parameters
 **/
typedef struct mlme_assoc_res_params_e {
	uint8_t dev_addr[EXT_ADDRESS_LENGTH]; /**< The address of the device requesting association.*/
	uint16_t dev_short_addr; /**< The short address allocated by the coordinator */
	mlme_assoc_status_enum_t status; /**< The status of the association attemp */
	prim_sec_param_st sec_params; /**< Security Information */
} mlme_assoc_res_param_st;
#endif /* (FFD_DEVICE_CONFIG || RFD_SUPPORT_ASSOCIATION_IND_RSP) */


/**
 * @brief structure represents MLME-ASSOCIATE.request input parameters
 **/
typedef struct mlme_assoc_req_params_e {
	mac_addrs_mode_enum_t coord_addr_mod; /**< Coordinator addressing mode */
	uint8_t coord_addr[EXT_ADDRESS_LENGTH]; /**< The address of the coordinator with which to associate */
	uint16_t coord_pan_id; /**< The identifier of the PAN with which to associate */
	uint8_t chnl_pge; /**< The channel page on which to attempt association */
	uint8_t chann_num; /**< The channel number on which to attempt association */
	uint8_t capab_info; /**< Specifies the operational capabilities of the associating device */
	prim_sec_param_st sec_params; /**< Security information */
} mlme_assoc_req_param_st;


/**
 * @brief structure represents MLME-DISASSOCIATE.Request input parameters
 **/
typedef struct mlme_disassoc_req_params_e {
	prim_sec_param_st sec_params; /**< security information */
	uint8_t dev_addr[EXT_ADDRESS_LENGTH]; /**<The address of the device to which to send the command */
	uint16_t dev_pan_id; /**<The PAN ID of the device to which to send the command */
	uint8_t tx_indirect; /**<set to one if the Disassociation Notification command is to be sent indirectly */
	mac_addrs_mode_enum_t addr_mod; /**<The addressing mode of the device to which to send the command */
	uint8_t reason; /**<The reason for the disassociation */
} mlme_disassoc_req_param_st;

/**
 * @brief structure represents MLME-SCAN.request input parameters
 **/
typedef struct mlme_scn_req_params_e {
	uint32_t 				scn_chnls; /**< The channel numbers to be scanned */
	uint8_t* 				ptr_hdr_ie_list; /**< Pointer to header IE */
	uint8_t* 				ptr_pyld_ie_list; /**< Pointer to payload IE */
	prim_sec_param_st 		sec_params; /**< Security Information */
	scn_type_t 				scn_type; /**< Indicates the type of scan performed */
	uint8_t 				scn_dur; /**< time to spend scanning on each channel*/
	uint8_t 				chnl_pge; /**<The channel page on which to perform the scan */
	uint8_t 				hdr_ie_list_count; /**< Number of header IE */
	uint8_t 				pyld_ie_list_count; /**< Number of payload IE */
	uint8_t 				sn_supr; /**<Set to one if the sequence number is suppressed in the frame */
	uint8_t					ie_present; /**<Information Element present(either header or payload IE */
	uint8_t					ie_hdr_total_len; /* total length of the header IE including its headers */
	uint8_t					ie_payld_total_len; /* total length of the payload IE including its headers */
} mlme_scn_req_param_st;

/**
 * @brief structure represents MLME-POLL.request input parameters
 **/
typedef struct mlme_poll_req_params_e {
	mac_addrs_mode_enum_t coord_addr_mod;/**< coordinator address mode */
	uint8_t coord_addr[EXT_ADDRESS_LENGTH];/**< coordinator address based on the mode */
	prim_sec_param_st sec_params;/**< Security Information */
	uint16_t coord_pan_id;/**< PanId for coordinator */
} mlme_poll_req_param_st;

#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_START_PRIM)
/**
 * @brief structure represents MLME-Start.Request input parameters
 **/
typedef struct mlme_start_req_params_e {
	prim_sec_param_st coord_realign_sec_parms;/**< Security information to be used in coordinator realignment command */
	prim_sec_param_st beacn_sec_params;/**< Security information to be used in Beacon frame */
	uint8_t* ptr_hdr_ie_list;/**< pointer to header IEs that will be attached to the beacon frame */
	uint8_t* ptr_pyld_ie_list;/**< pointer to payload IEs that will be attached to the beacon frame */
	uint32_t start_time;/**< This parameter is ignored for non-beacon enabled */
	uint32_t pan_coord;/**< Flag to indicate the role of this device (coordinator/pan-coordinator) */
	uint32_t batt_life_ext; /**<This parameter is ignored in non-beacon enabled network*/
	uint32_t coord_realign; /**<This flag indicate if coordinator realignment is to be transmitted*/
	uint16_t pan_id;/**<The PAN identifier to be used by thedevic*/
	uint8_t logic_chanl;/**< Channel to transmit beacon frame or coordinator realignment command frame on*/
	uint8_t chnl_pge;/**<The channel page on which to begin listen and transmit*/
	uint8_t bcon_ord;/**<value of 15 indicates that the coordinator will not transmit periodic beacons (non-beacon enabled)*/
	uint8_t super_frm_ord;/**< ignored in case of non-beacon enabled network */
	uint8_t ie_present; /**< flag to indicate existence of IEs in the sent frame*/
	uint8_t hdr_ie_list_count;/**< number of header IEs passed */
	uint8_t pyld_ie_list_count;/**< number of payload IEs passed */
	uint8_t	ie_hdr_total_len; /**< total length of the header IE including its headers */
	uint8_t	ie_payld_total_len; /**< total length of the payload IE including its headers */

} mlme_start_req_param_st;
#endif /* FFD_DEVICE_CONFIG */
/**
 * @brief Structure contains all the information required for MLME-DISASSOCIATE.confirm primitive
 **/
typedef struct mlme_disassoc_cfm_params_e {
	mac_status_enum_t status; /**< Status of disassociation operation */
	mac_addrs_mode_enum_t dev_addr_mode; /**< Device addressing mode */
	uint16_t dev_pan_id; /**< Device PAN ID */
	uint8_t dev_addr[EXT_ADDRESS_LENGTH]; /**< Device address*/
} mlme_disassoc_cfm_params_st;

/**
 * @brief Structure contains all the information required for MLME-DISASSOCIATE.indication primitive
 **/
typedef struct mlme_disassoc_ind_e {
	uint8_t dev_addr[EXT_ADDRESS_LENGTH]; /**< The address of the device requesting disassociation */
	uint8_t reason; /**< The reason for the disassociation */
	prim_sec_param_st beacn_sec_params; /** <Security Information */
} mlme_disassoc_ind_st;

/*******************************************mcps_data_types ***********************/
/*
 * @enum
 * @brief enumeration that describes tx options passed in mcps-data.request
 * */
typedef enum mcps_tx_options_mask_enum {
	ack_transm_msk = 0x01, /* acknowledged transmission is required */
	indirect_transm_shift = 0x02, /* indirect transmission bit shift */
	indirect_transm_msk = 0x04 /* indirect transmission */
} mcps_tx_options_mask_enum_t;
/**
 * @brief Structure contains all the input parameters for a the MCPS-DATA.Request
 **/
typedef struct st_mcps_data_req_params {
	/** The source addressing mode for this primitive and subsequent MPDU. This value can take one of the following values:
	 * 0x00 = no address, 0x01 = reserved, 0x02 = 16-bit short address, 0x03 = 64-bit extended address.*/
	mac_addrs_mode_enum_t enum_src_addrs_mode;
	/** The destination addressing mode for this primitive and subsequent MPDU. This value can take one of the
	 following values:
	 0x00 = no address, 0x01 = reserved, 0x02 = 16-bit short address, 0x03 = 64-bit extended address. */
	mac_addrs_mode_enum_t enum_dstn_addrs_mode;
	/** The 16-bit PAN identifier of the entity to which the MSDU is being transferred. */
	uint16_t dstn_pan_id;
	/** The individual device address of the entity to which the MSDU is being transferred */
	uint8_t dstn_addrs[8];
	/** The number of octets contained in the MSDU to be transmitted by the MAC sublayer entity.*/
	uint8_t msdu_len;
	/** The set of octets forming the MSDU to be transmitted by the MAC sublayer entity.*/
	uint8_t *ptr_msdu;
	/**  The handle associated with the MSDU to be transmitted by the MAC sublayer entity*/
	uint8_t msdu_hndl;
	/** The 3 bits (b0, b1, b2) indicate the transmission options for this MSDU.  For b0, 1 = acknowledged transmission,
	 0 = unacknowledged transmission, For b1, 1 = GTS transmission, 0 = CAP transmission for a bcon-enabled PAN,
	 For b2, 1 = indirect transmission, 0 = direct transmission. For a nonbcon-enabled PAN, bit b1 should always be set to 0. */
	uint8_t tx_options;
	/** The security level to be used */
	sec_level_enum_t enum_mcps_security_level;
	/** The mode used to identify the key to be used */
	uint8_t key_id_mode;
	/** The originator of the key to be used (see 7.6.2.4.1). This parameter is ignored if the KeyIdMode parameter is ignored or set to 0x00 */
	uint8_t key_src[8];
	/** The index of the key to be used (see 7.6.2.4.2). This parameter is ignored if the KeyIdMode parameter is ignored or set to 0x00. */
	uint8_t key_index;
} mcps_data_req_params_st_t;
/*
 * @struct
 * @brief contains the private data to be used in persistent timer call back
 * */
typedef struct persis_tmr_data_e {
	void *ptr_mac_cntx;
	void *ptr_persist_data_loc;
} persis_tmr_data_st;

/* MCPS-DATA cfm parameters */
typedef struct {
	uint32_t timestamp; /* Optional. The time, in symbols, at which the data were transmitted */
	uint8_t msdu_hndl; /* The handle associated with the MSDU being cfmed */
	uint8_t enum_data_tx_status; /* The status of the last MSDU transmission */
} mcps_data_cfm_params_st_t;

/**
 * @brief Structure contains all information related to MLME-COMM-STATUS.indication primitive
 **/
typedef struct {
	/* Security Information */
	prim_sec_param_st sec_params;
	/** Source address */
	uint8_t src_add[EXT_ADDRESS_LENGTH];
	/**The address of the device for which the frame was intended */
	uint8_t dst_addr[EXT_ADDRESS_LENGTH];
	/** The PAN ID of the device from which the frame was received or to which the frame was being sent */
	uint16_t pan_id;
	/** Source addressing mode */
	uint8_t src_addr_mode;
	/** Destination addressing mode */
	uint8_t dst_addr_mode;
	/** The communications status */
	uint8_t status;
} mlme_comm_status_st_t;

#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_ORPHAN_IND_RSP)
/*
 * @struct
 * @brief Structure contains all information related to MLME-ORPHAN.indication primitive
 * */
typedef struct {
	/* orphan device address */
	uint8_t orphan_addr[EXT_ADDRESS_LENGTH];
	/* security Information for orphan notification command */
	prim_sec_param_st sec_params;
} mlme_orphan_ind_st_t;
/*
 * @struct
 * @brief Structure contains all information related to MLME-ORPHAN.response primitive
 * */
typedef struct {
	/* security Information for orphan response frame */
	prim_sec_param_st sec_params;
	/* orphan device address */
	uint8_t orphan_addr[EXT_ADDRESS_LENGTH];
	/* The short address allocated to the orphaned device if it is associated with this coordinator.
	 * The special short address 0xfffe indicates that no short address was allocated,and the device
	 * will use its extended address in all communications. If the device was not associated with
	 * this coordinator, this field will contain the value 0xffff and be ignored on receipt */
	uint16_t shrt_addr;
	/* TRUE if the orphaned device is associated with this coordinator or FALSE otherwise.
	 * @note:If False function will ignore the call for MLME-ORPHAN.Response */
	uint8_t assoc_member;

} mlme_orphan_rsp_st_t;
#endif /* (FFD_DEVICE_CONFIG || RFD_SUPPORT_ORPHAN_IND_RSP) */

#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_SEND_BEACON)
/*
 * @struct
 * @brief Structure contains all information related to MLME-BEAON-Req.Indication primitive
 * */
typedef struct mlme_bcon_req_ind_params_e {
	uint8_t src_addr[EXT_ADDRESS_LENGTH]; /**< source address for the device sending beacon request */
	uint8_t ptr_hdr_ie_list[MAX_HDR_IE_SIZE]; /**< parsed header IEs from beacon request frame */
	uint8_t ptr_pyld_ie_list[MAX_ZIGBEE_EBR_IE_LEN];/**< parsed payload IEs from beacon request frame*/
	uint16_t dst_pan_id;/**< PANID for the device receiving beacon request */
	uint8_t bcon_type;/**< Type of beacon frame required to be sent on response for beacon request Beacon/Enhanced Beacon*/
	uint8_t src_addr_mode;/**< source address mode for th device sending beacon request */
	uint8_t hdr_ie_list_count; /**<count of header IEs after filtering the unrecognized IEs*/
	uint8_t pyld_ie_list_count; /**<count of payload IEs after filtering the unrecognized IEs */
	uint8_t pyld_ie_total_len; /**<length of header IEs after filtering the unrecognized IEs*/
	uint8_t hdr_ie_total_len;  /**<length of payload IEs after filtering the unrecognized HDR IEs */
} mlme_bcon_req_ind_params_st;
/*
 * @struct
 * @brief Structure contains all information related to MLME-BEAON-SEND.Request primitive
 * */
typedef struct st_mlme_bcon_send_req_params {
	uint8_t 				key_src[KEY_SRC_SIZE_MOD_3]; /**< key source for Security information */
	uint8_t 				dstn_addrs[EXT_ADDRESS_LENGTH];/**< address of the device to send beacon to in response of beacon request*/
	uint8_t* 				ptr_hdr_ie_list;/**< pointer to header IEs to be attached to beacon frame in case of Enhanced beacon */
	uint8_t* 				ptr_pyld_ie_list;/**< pointer to payload IEs to be attached to beacon frame in case of Enhanced beacon */
	bcon_typ_t 				enum_bcon_typ;/**< beacon type: Enhanced Beacon / Beacon */
	mac_addrs_mode_enum_t 	enum_dstn_addrs_mode;/**< destination address mode */
	mac_addrs_mode_enum_t 	enum_src_addrs_mode;/**< source address mode */
	sec_level_enum_t 		enum_mcps_security_level;/**< Security level for security information */
	uint8_t 				chnnl_num;/**< channel to transmit beacon frame on */
	uint8_t 				chnnl_page;/**< channel page to transmit beacon frame on */
	uint8_t					super_frm_ord;/**< ignored in non-beacon enabled*/
	uint8_t 				hdr_ie_list_count;/**< count of header IEs */
	uint8_t 				pyld_ie_list_count;/**< count of payload IEs */
	uint8_t 				hdr_ie_total_len;/**< total len of header IEs */
	uint8_t 				pyld_ie_total_len;/**< total len of header IEs*/
	uint8_t 				key_id_mode;/**< key id mode used for security processing */
	uint8_t 				key_index;/**< key index mode used for security processing */
	uint8_t 				bsn_supr;/**< beacon sequence number suppression flag used to suppress SN*/
} mlme_bcon_send_req_params_st_t;

#endif
/*Structure representing all the call backs that could be called from MAC to the upper host*/
struct mac_dispatch_tbl {

	/*========================================================================================================*/
	/*================================================= APIs =================================================*/
	/*========================================================================================================*/
	/*===== Rx Enable Confirm Callback =====*/
	/**
	 * @brief  MLME-RX-ENABLE primitive callback
	 *
	 * @param  status			: [in] indicates the status of rx enable or disable operation
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 *
	 * @retval None.
	 */
	void (*mlme_rx_enable_cfm)(void* mac_cntx_ptr, uint8_t status);

	/*===== Reset Confirm Callback =====*/
	/**
	 * @brief  MLME-RESET primitive callback
	 *
	 * @param  status			: [in] indicates the status of reset operation
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 *
	 * @retval None.
	 */
	void (*mlme_rst_cfm)(void* mac_cntx_ptr, uint8_t status);
#if SUPPORT_SEC
	/*===== Remove Sec Level Table Confirm Callback =====*/
	/**
	 * @brief  MLME-REMOVE-SEC-TABLE.CONFIRM primitive callback
	 *
	 * @param  status			: [in] indicates the status of reset operation
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 * @param   tbl_type	 	: [in] Indicate type of removed table
	 *
	 * @retval None.
	 */
	void(*mlme_rmv_sec_tbl_cfm)(void* mac_cntx_ptr, uint8_t status, uint8_t tbl_type);
#endif /*#end of SUPPORT_SEC*/
	/*===== Set Confirm Callback =====*/
	/**
	 * @brief  MLME-SET.CONFIRM primitive callback
	 *
	 * @param  status			: [in] indicates the status of reset operation
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 * @param  pib_attr_id		: [in] Identifier of PibAttribute sent in set request primitive
	 * @param  pib_attr_indx	: [in] Index for MAC PIBAttributes
	 *
	 * @retval None.
	 */
	void (*mlme_set_cfm)(void* mac_cntx_ptr, uint8_t status, uint8_t pib_attr_id, uint8_t pib_attr_indx);

	/*===== Get Confirm Callback =====*/
	/**
	 * @brief  MLME-GET.CONFIRM primitive callback
	 *
	 * @param  status			: [in] indicates the status of reset operation
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 * @param  pib_attr_id		: [in] PIB attribute ID
	 * @param  pib_attr_indx	: [in] PIB attribute index
	 * @param  pib_attr_val		: [in] PIB value
	 *
	 * @retval None.
	 */
	void (*mlme_get_cfm)(void* mac_cntx_ptr, uint8_t status,
			uint8_t pib_attr_id, uint8_t pib_attr_indx,
			attr_val_t* pib_attr_val);
#if RADIO_CSMA
	/**
	 * @brief  MLME-SET-ENABLE-CSMA.CONFIRM primitive callback
	 *
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 * @param  status			: [in] indicates the status of reset operation
	 *
	 * @retval None.
	 */
	void (*mlme_set_csma_en_cfm)(void* mac_cntx_ptr, uint8_t status);
#endif /*end of RADIO_CSMA*/
	/**
	 * @brief  MLME-SET-ENABLE-CCA.CONFIRM primitive callback
	 *
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 * @param  status			: [in] indicates the status of reset operation
	 *
	 * @retval None.
	 */
	void (*mlme_set_cca_en_cfm)(void* mac_cntx_ptr, uint8_t status);
	/**
	 * @brief  MLME-SET-CCA-THRESHOLD.CONFIRM primitive callback
	 *
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 * @param  status			: [in] indicates the status of reset operation
	 * @param cca_thresold		: [in] indicates cca threshold that set
	 *
	 * @retval None.
	 */
	void (*mlme_set_cca_threshold_cfm)(void* mac_cntx_ptr, uint8_t status, int8_t cca_thresold);
	/**
	 * @brief  MLME-GET-CCA-THRESHOLD.CONFIRM primitive callback
	 *
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 * @param  status			: [in] indicates the status of reset operation
	 * @param cca_thresold		: [in] indicates currently used cca threshold
	 *
	 * @retval None.
	 */
	void (*mlme_get_cca_threshold_cfm)(void* mac_cntx_ptr, uint8_t status, int8_t cca_thresold);
#if SUPPORT_ANT_DIV
	/**
	 * @brief  MLME-SET-ANT-DIV-PARAMS.CONFIRM primitive callback
	 *
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 * @param  status			: [in] indicates the status of reset operation
	 * @param cca_thresold		: [in] indicates currently used cca threshold
	 *
	 * @retval None.
	 */
	void (*mlme_set_ant_div_params_cfm)(void* mac_cntx_ptr, uint8_t status, antenna_diversity_st* ptr_ant_div_params);
	/**
	 * @brief  MLME-SET-ANT-DIV-EN.CONFIRM primitive callback
	 *
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 * @param  status			: [in] indicates the status of reset operation
	 * @param enable		    : [in] enable/disable antenna diversity
	 *
	 * @retval None.
	 */
	void (*mlme_set_ant_div_en_cfm)(void* mac_cntx_ptr, uint8_t status, uint8_t enable);
	/**
	 * @brief  MLME-SET-ANT-DIV-EN.CONFIRM primitive callback
	 *
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 * @param  status			: [in] indicates the status of reset operation
	 * @param default_ant_id	: [in] indicates the antenna id to be used as default
	 *
	 * @retval None.
	 */
	void (*mlme_set_default_ant_id_cfm)(void* mac_cntx_ptr, uint8_t status, uint8_t default_ant_id);
	/**
	 * @brief  MLME-SET-ANT-DIV-EN.CONFIRM primitive callback
	 *
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 * @param  status			: [in] indicates the status of reset operation
	 * @param rssi_threshold	: [in] indicates good quality rssi threshold
	 *
	 * @retval None.
	 */
	void (*mlme_set_ant_div_rssi_threshold_cfm)(void* mac_cntx_ptr, uint8_t status, int8_t rssi_threshold);
#endif /* SUPPORT_ANT_DIV */
#if SUPPORT_SEC
	/*===== Get key table Confirm Callback =====*/
	/**
	 * @brief  MLME-GET_KEY_TBL.CONFIRM primitive callback
	 *
	 * @param  status			: [in] indicates the status of get operation
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 * @param  pib_attr_id		: [in] PIB attribute ID
	 * @param  pib_attr_indx	: [in] PIB attribute index
	 * @param  pib_attr_val		: [in] PIB value
	 *
	 * @retval None.
	 */
	void (*mlme_get_key_tbl_cfm)(void* mac_cntx_ptr, uint8_t status,
			uint8_t pib_attr_id, uint8_t pib_attr_indx,
			attr_val_t* pib_attr_val);
#endif /*end of SUPPORT_SEC*/
	/*===== Poll Confirm Callback =====*/
	/**
	 * @brief  MLME-POLL.CONFIRM confirm callback
	 *
	 * @param  status			: [in] indicates the status of poll request
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 *
	 * @retval None.
	 */
	void (*mlme_poll_cfm)(void* mac_cntx_ptr, uint8_t status);

	/*===== Scan Confirm Callback =====*/
	/**
	 * @brief  MLME-SCAN.CONFIRM primitive callback
	 *
	 * @param  mac_cntx_ptr				: [in] indicate the used mac context
	 * @param  mlme_scan_cmf_params		: [in] scan confirm parameters returned to host
	 *
	 * @retval None.
	 */
	void (*mlme_scn_cfm)(void* mac_cntx_ptr,
			mlme_scn_cmf_param_st* mlme_scan_cmf_params);
	/*===== assoc Confirm Callback =====*/
	/**
	 * @brief  MLME_ASSOCIATION.CONFIRM primitive callback
	 *
	 * @param  status			: [in] indicates the status of assoc request
	 * @param  short_addr		: [in] indicates device short address received in assoc confirm
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 * @param  sec_params		: [in] security parameters
	 *
	 * @retval None.
	 */
	void (*mlme_assoc_cfm)(void* mac_cntx_ptr, uint8_t status,
			uint16_t short_addr, prim_sec_param_st* ptr_sec_params);

	/*===== Beacon Notify Callback =====*/
	/**
	 * @brief  MLME-BEACON-NOTIFY.INDICATION primitive callback
	 *
	 * @param  mac_cntx_ptr				: [in] indicate the used mac context
	 * @param  ptr_bcon_notify_params	: [in] beacon notify parameters
	 *
	 * @retval None.
	 */
	void (*mlme_bcon_notfy)(void* mac_cntx_ptr,
			mlme_bcon_notfy_params_st* ptr_bcon_notify_params);

	/*===== MCPS Data Indication Callback =====*/
	/**
	 * @brief  MCPS-DATA.INDICATION primitive callback
	 *
	 * @param  mac_cntx_ptr				: [in] indicate the used mac context
	 * @param  ptr_mcps_data_ind		: [in] data indication parameters
	 *
	 * @retval None.
	 */
	void (*mcps_data_ind)(void* mac_cntx_ptr,
			mcps_indicate_params_st_t* ptr_mcps_data_ind);

	/*===== MLME Sync Loss Indication Callback =====*/
	/**
	 * @brief  MLME-SYNC-LOSS.Indication primitive callback
	 *
	 * @param  mac_cntx_ptr				: [in] indicate the used mac context
	 * @param  ptr_st_mlme_sync_loss	: [in] sync loss indication parameters
	 *
	 * @retval None.
	 */
	void (*mlme_sync_loss_ind)(void* mac_cntx_ptr,
			mlme_sync_loss_params_st_t* ptr_st_mlme_sync_loss);

#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_START_PRIM)
	/*===== MLME Start Confirm Callback =====*/
	/**
	 * @brief  MLME-START.CONFIRM primitive callback
	 *
	 * @param  status			: [in] indicates the status of start operation
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 *
	 * @retval None.
	 */
	void (*mlme_strt_cfm)(void* mac_cntx_ptr, uint8_t status);
#endif /* (FFD_DEVICE_CONFIG || RFD_SUPPORT_START_PRIM) */

	/*===== MCPS Data Confirm Callback =====*/
	/**
	 * @brief  MCPS-DATA.CONFIRM primitive callback
	 *
	 * @param  mac_cntx_ptr				 : [in] indicate the used mac context
	 * @param  mcps_data_cfm_params_st_t : [in] data confirm parameters
	 *
	 * @retval None.
	 */
	void (*mcps_data_cfm)(void* mac_cntx_ptr,
			mcps_data_cfm_params_st_t* ptr_mcps_data_ind);
#if	(FFD_DEVICE_CONFIG || RFD_SUPPORT_ASSOCIATION_IND_RSP)
	/*===== MLME Poll Indication Callback =====*/
	/**
	 * @brief  MLME-POLL.indication primitive callback
	 *
	 * @param  mac_cntx_ptr			: [in] indicate the used mac context
	 * @param  addr_mode 			: [in] device address mode of received data request
	 * @param  dev_addr 			: [in] pointer to device address of received data request
	 *
	 * @retval None.
	 */
	void (*mlme_poll_ind)(void* mac_cntx_ptr, uint8_t addr_mode,
			uint8_t* dev_addr);
#endif /* (FFD_DEVICE_CONFIG || RFD_SUPPORT_ASSOCIATION_IND_RSP) */
	/* MLME Comm Status Indication */
	/**
	 * @brief  MLME-COMM-STATUS.Indication primitive callback
	 *
	 * @param  mac_cntx_ptr				: [in] indicate the used mac context
	 * @param  ptr_mlme_comm_status 	: [in] communication status primitive parameters
	 *
	 * @retval None.
	 */
	void (*mlme_comm_status)(void* mac_cntx_ptr,
			mlme_comm_status_st_t *ptr_mlme_comm_status);

	/* MLME Disassociation Confirm */
	/**
	 * @brief   MLME-Disassociate.confirm primitive callback
	 *
	 * @param  mac_cntx_ptr				 	: [in] indicate the used mac context
	 * @param  mlme_disassoc_cfm_params_st  : [in] disassoc confirm parameters
	 *
	 * @retval None.
	 */
	void (*mlme_disassoc_cfm)(void* mac_cntx_ptr,
			mlme_disassoc_cfm_params_st *ptr_mlme_disassoc_cfm_params);

	/* MLME Disassociation Indication */
	/**
	 * @brief  MLME-DISASSOCIATION.INDICATION primitive callback
	 *
	 * @param  mac_cntx_ptr				 	: [in] indicate the used mac context
	 * @param  ptr_mlme_disassoc_ind_params : [in] disassoc indication parameters
	 *
	 * @retval None.
	 */
	void (*mlme_disassoc_ind)(void* mac_cntx_ptr,
			mlme_disassoc_ind_st *ptr_mlme_disassoc_ind_params);

#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_ASSOCIATION_IND_RSP)
	/*===== assoc Indication Callback =====*/
	/**
	 * @brief  MLME-ASSOCIATION.Indication callback
	 *
	 * @param  mlme_assoc_ind_param_st		: [in] indicates the parameters of assoc request
	 *
	 * @param  mac_cntx_ptr		: [in] indicate the used mac context
	 *
	 * @retval None.
	 */
	void (*mlme_assoc_ind)(void* mac_cntx_ptr,
			mlme_assoc_ind_param_st* mlme_assoc_ind_params);
#endif /* (FFD_DEVICE_CONFIG || RFD_SUPPORT_ASSOCIATION_IND_RSP) */

#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_DATA_PURGE)
	/* MCPS Purge Confirm */
	/**
	 * @brief  MCPS-PURGE.confirm is generated as response for MCPS-PURGE.request primitive
	 *
	 * @param  mac_cntx_ptr	: [in] indicate the used mac context
	 * @param  msdu_hndl    : [in] msdu handle
	 * @param  status	    : [in] status for the purge request
	 *
	 * @retval None.
	 */
	void (*mcps_purge_cfm)(void* mac_cntx_ptr,
			uint8_t msdu_hndl, uint8_t status);
#endif /* (FFD_DEVICE_CONFIG || RFD_SUPPORT_DATA_PURGE) */

#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_ORPHAN_IND_RSP)
	/* MLME Orphan Indication */
	/**
	 * @brief  MLME-orphan.indication primitive callback
	 *
	 * @param  mac_cntx_ptr				 	: [in] indicate the used mac context
	 * @param  ptr_mlme_orphan_ind_params   : [in] orphan indication parameters
	 *
	 * @retval None.
	 */
	void (*mlme_orphan_ind)(void* mac_cntx_ptr,
			mlme_orphan_ind_st_t *ptr_mlme_orphan_ind_params);
#endif /* (FFD_DEVICE_CONFIG || RFD_SUPPORT_ORPHAN_IND_RSP) */

#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_SEND_BEACON)
	/* MLME Beacon Request Indication */
	/**
	 * @brief  MLME-BEACON-Request.indication primitive callback
	 *
	 * @param  mac_cntx_ptr				 	: [in] indicate the used mac context
	 * @param  ptr_bcon_req_ind_params   	: [in] beacon request indication parameters
	 *
	 * @retval None.
	 */
	void (*mlme_bcon_req_ind)(void* mac_cntx_ptr,
			mlme_bcon_req_ind_params_st *ptr_bcon_req_ind_params);

	/*  MLME Beacon Send Confirm */
	/**
	 * @brief  MLME-BEACON.confirm primitive callback
	 *
	 * @param  status	    		: [in] status of beacon send request
	 * @param  mac_cntx_ptr			: [in] indicate the used mac context
	 *
	 * @retval None.
	 */
	void (*mlme_bcon_cfm)(void* mac_cntx_ptr, uint8_t status);

#endif /* (FFD_DEVICE_CONFIG || RFD_SUPPORT_SEND_BEACON) */
	/* Set pwr info confirm  */
	/**
	 * @brief  returns the status and the information re-quested by the MLME-SET-POWER-INFORMATION-TABLE.request.
	 *
	 * @param  mac_cntx_ptr	: [in] indicate the used mac context
	 * @param  status	    : [in] status error for the information re-quested
	 *
	 * @retval None.
	 */
	void (*mlme_set_pwr_info_table_cfm)(void* ptr_mac_cntx, uint8_t status);

	/* Get pwr info confirm  */
	/**
	 * @brief  returns the status and the information re-quested by the MLME-GET-POWER-INFORMATION-TABLE.request.
	 *
	 * @param  mac_cntx_ptr	: [in] indicate the used mac context
	 * @param  status	    : [in] status error for the information re-quested
	 * @param  short_addrs 	: [in] Short address of the link pair to transmit the packet to.
	 * @param  ptr_ext_addrs: [in] Pointer to Extended (IEEE) address of the link pair to transmit the packet to.
	 * @param  tx_pwr_level : [in] Tx power level of the link pair to transmit the packet to.
	 * @param  last_rssi_level : [in] RSSI of last packet received on the link pair
	 * @param  nwk_negotiated  : [in] Flag = 0 during the joining / rejoining process , 1 after joining/rejoining
	 *
	 * @retval None.
	 */
	void (*mlme_get_pwr_info_table_cfm)(void* ptr_mac_cntx, uint8_t status, uint16_t short_addrs, uint8_t  *ptr_ext_addrs,
			                            int8_t tx_pwr_level, int8_t  last_rssi_level, uint8_t nwk_negotiated);
#if SUPPORT_ZIGBEE_PHY_CERTIFICATION
/**
 * @fn check tx error confirm
 *
 * @brief   This function is used to notify upper layer upon transmission by number or trials of expected error
 *
 * @param  mac_cntx_ptr 		: [in] pointer to mac context
 * @param  last_error			: [in] status error overall transmission
 * @param  error_arr 			: [in] status error for expected error
 * @param  error_count 			: [in] counts the number of expected error in this transmission
 *
 * @retval None .
 */
	void (*check_tx_cfm)(void* mac_cntx_ptr, uint8_t last_error, uint8_t* error_arr, uint8_t error_count);
#endif /*end of SUPPORT_ZIGBEE_PHY_CERTIFICATION*/
};

/** @ingroup  mac_intf
 *  @{
 */
/* ============================== MAC Init API ========================== */
/**
 * @brief   Mac Initialization function, it must be invoked once at the beginning
 *
 * @param   mac_hndl[in] : the MAC instance handle
 * @param   ptr_dispatch_tbl[in] : pointer to the dispatch table
 *
 * @retval mac_status_enum_t :  status to be sent to the Host
 */
mac_status_enum_t mac_init(uint32_t* mac_hndl,
		struct mac_dispatch_tbl* ptr_dispatch_tbl);


/*=========================================================================================================*/
/*
 * For all mlme requests/ responses that requires packet transmission (excluding set/get requests), and
 * for all MCPS Data requests mlme function defined in mac layer will do the needed checks and prepare for
 * proper packet tranmission and then post DIRECT_DATA_TX_EVENT event to event manager which will handle
 * the posted event after call emngr_hndl_all_events by calling direct_tx_evnt_cbk which will issue
 * otpltRadioTransmit.
 *
 *================          ================================   |    _ _ _  _ _ _ _ _ _ _ _
 * MLME-XX.Req/Res_ _ _  _> post DIRECT_DATA_TX_EVENT event    |    \ Main Thread          \
 *================<_ _ _ _ _================================   |. . .\ Emngr_hndl_all_events\
 * 				   <Return>     							   |	  \_ _ _ _ _ _ _ _ _ _ _ \ ---> direct_tx_evnt_cbk
 *
 * */
/*=========================================================================================================*/
/* ============================== MLME Requests ======================================================== */
#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_SEND_BEACON)
/**
 * @brief   beacon send request primitive.
 *
 * @param   mac_hndl[in] : the MAC instance handle
 *
 * @param  	ptr_st_bcon_send_req_params[in]: pointer to the beacon send request parameters
 *
 * @retval  mac_status_enum_t :  status to be sent to the Host
 *
 * @note 	this function will post DIRECT_DATA_TX_EVENT event that will be handled
 *			when call emngr_handle_all_events to call the cbk direct_tx_evnt_cbk
 */
mac_status_enum_t mlme_bcon_send_req(uint32_t mac_hndl,
		mlme_bcon_send_req_params_st_t *ptr_st_bcon_send_req_params);
#endif
/**
 * @brief   used by a device to request an association with a coordinator,
 *			mlme_assoc_cfm callback is used to inform the next higher layer
 *			whether its request to associate was successful or unsuccessful.
 *
 * @param   mac_hndl[in] : the MAC instance handle
 *
 * @param  	ptr_st_assoc_req_param[in]: pointer to the association request parameters
 *
 * @retval  mac_status_enum_t :  status to be sent to the Host
 *
 * @note 	this function will post DIRECT_DATA_TX_EVENT event that will be handled
 *			when call emngr_handle_all_events to call the cbk direct_tx_evnt_cbk
 */
mac_status_enum_t mlme_assoc_req(uint32_t mac_hndl,
		mlme_assoc_req_param_st *ptr_st_assoc_req_param);

#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_ASSOCIATION_IND_RSP)
/**
 * @brief   association response primitive.
 *
 * @param   mac_hndl[in] : the MAC instance handle
 *
 * @param  	ptr_st_assoc_req_param[in]: pointer to the association request parameters
 *
 * @retval 	None
 *
 * @note 	this function will post DIRECT_DATA_TX_EVENT event that will be handled
 *			when call emngr_handle_all_events to call the cbk direct_tx_evnt_cbk
 */
mac_status_enum_t mlme_assoc_res(uint32_t mac_hndl,
		mlme_assoc_res_param_st *ptr_st_assoc_res_param);
#endif /* (FFD_DEVICE_CONFIG || RFD_SUPPORT_ASSOCIATION_IND_RSP) */

/**
 * @brief   Request to disassociate from a PAN or used by the coordinator to disassociate a device from a PAN,
 * 			 mlme_disassoc_cfm callback will be issued with data stored in structure  mlme_disassoc_cfm_params_e
 *
 * @param   mac_hndl : [in] the MAC instance handle
 * @param  	mlme_disassoc_req_param : [in] pointer to disassociation request primitive params
 *
 * @retval 	mac_status_enum_t :  status to be sent to the Host
 *
 * @note 	this function will post DIRECT_DATA_TX_EVENT event that will be handled
 *			when call emngr_handle_all_events to call the cbk direct_tx_evnt_cbk
 */
mac_status_enum_t mlme_disassoc_req(uint32_t mac_hndl,
		mlme_disassoc_req_param_st* mlme_disassoc_req_param);

/**
 * @brief   mlme get request for accessing PIB attributes
 *
 * @param   mac_hndl	 : [in] the MAC instance handle
 * @param  	pib_attr     : [in] PIB attribute ID
 * @param   pib_attr_indx: [in] PIB attribute index
 * @param	pib_attr_value: [out] PIB value
 *
 * @retval void
 */
void mlme_get_req(uint32_t mac_hndl, uint8_t pib_attr_id, uint8_t pib_attr_indx,
		attr_val_t *pib_attr_val);

/**
 * @brief   mlme reset request for reset everything
 *
 * @param   mac_hndl	: [in] the MAC instance handle
 * @param  	set_def_pib : [in] set default PIB flag
 *
 * @retval void
 */
void mlme_rst_req(uint32_t mac_hndl, uint8_t set_def_pib);

/**
 *
 * @brief   destroy all the mac handles registered
 *
 * @param   none
 *
 * @retval void
 */
void mac_destroy(void);

/**
 *
 * @brief   mlme scn request issued to start one of [Active/ Enhanced Active / Passive / Energy Detection / Orphan] Scan type.
 *
 * @param   mac_hndl			 : [in] the MAC instance handle
 * @param  	mlme_req_prim_params : [in] pointer to request primitive params
 *
 * @retval 	void
 *
 * @note   	this function will post DIRECT_DATA_TX_EVENT event that will be handled
 *			when call emngr_handle_all_events to call the cbk direct_tx_evnt_cbk
 *			in cases of Active/Enhanced Active/Orphan Scaning only
 */
mac_status_enum_t mlme_scn_req(uint32_t mac_hndl,
		mlme_scn_req_param_st* mlme_scn_req_params);

#if SUPPORT_SEC
/**
 *
 * @brief   remove a mac_sec_level_table specified by pib_attr_indx
 *
 * @param   mac_hndl 	 : [in] the MAC instance handle
 * @param   tbl_type	 : [in] Indicate type of removed table
 * @param   tbl_index	 : [in] Indicate index of SecTable of main security attribute to be removed
 * @param   tbl_sub_index: [in] Indicate index of SecTable of sub security attribute to be removed.
 *
 * @retval void
 */
void mlme_rmv_sec_table(uint32_t mac_hndl, uint8_t tbl_type, uint8_t tbl_index, uint8_t tbl_sub_index);
#endif /*end of SUPPORT_SEC*/
/**
 *
 * @brief   set a value of PIB attribute
 *
 * @param   mac_hndl 	 : [in] the MAC instance handle
 * @param  	pib_attr     : [in] PIB attribute ID
 * @param   pib_attr_indx: [in] PIB attribute index
 * @param	pib_attr_value: [in] PIB value
 *
 * @retval void
 */
void mlme_set_req(uint32_t mac_hndl, uint8_t pib_attr_id, uint8_t pib_attr_indx,
		attr_val_t* pib_attr_val);

#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_START_PRIM)
/**
 *
 * @brief   MLME-START.request is used by the PAN coordinator to initiate a new PAN or to begin
 *			using a new superframe configuration,mlme_strt_cfm callback is called to send
 *			MLME-START.confirm to the upper layers
 *			As stated in standard the impact of new updates will be added immediately in case
 *			of idle state or after transmission of beacon in case a beacon prepared to be sent.
 *
 * @param   mac_hndl : [in] the MAC instance handle
 * @param  	mlme_req_prim_params : [in] pointer to request primitive params
 *
 * @retval status
 */
mac_status_enum_t mlme_strt_req(uint32_t mac_hndl,
		mlme_start_req_param_st* ptr_strt_req_params);
#endif /* (FFD_DEVICE_CONFIG || RFD_SUPPORT_START_PRIM) */

/**
 *
 * @brief   The MLME-POLL.request primitive prompts the device to request data from the coordinator,
 *			 mlme_poll_cfm callback will called by MLME to send MLME-POLL.confirm to the upper layers
 *
 *
 * @param   mac_hndl[in] : the MAC instance handle
 * @param  	ptr_st_poll_req_param[in]: pointer to the polling request parameters
 *
 * @retval  mac_status_enum_t
 *
 * @note 	this function will post DIRECT_DATA_TX_EVENT event that will be handled
 *				  when call emngr_handle_all_events to call the cbk direct_tx_evnt_cbk
 */
mac_status_enum_t mlme_poll_req(uint32_t mac_hndl, mlme_poll_req_param_st *ptr_st_poll_req_param);

#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_ORPHAN_IND_RSP)
/**
 * @brief   used to respond to orphan notification command to inform the orphan device if associatied
 * 			about its exchanged information during association.
 *
 * @param   mac_hndl[in] : the MAC instance handle
 * @param  	ptr_st_orphan_rsp_param[in]: pointer to the orphan response parameters
 *
 * @retval  mac_status_enum_t
 *
 * @note    this function will post DIRECT_DATA_TX_EVENT event that will be handled
 *			when call emngr_handle_all_events to call the cbk direct_tx_evnt_cbk
 */
mac_status_enum_t mlme_orphan_rsp(uint32_t mac_hndl, mlme_orphan_rsp_st_t *ptr_st_orphan_rsp_param);
#endif /* (FFD_DEVICE_CONFIG || RFD_SUPPORT_ORPHAN_IND_RSP) */
/* ============================== MLME APIs ========================== */

/**************************************************MCPS-SAP public functions*******************************************/
/**
 * @brief  requests the transfer of a data SPDU (i.e., MSDU) from a local SSCS
 *         entity to a single peer SSCS entity.
 *
 * @param   ptr_st_data_req_params[in]	: pointer to the data request parameters
 * @param   mac_hndl[in]	            : the MAC instance handle that initiated data send request
 *
 * @retval 	mac_status_enum_t            : the results of a request.
 *
 * @note 	this function will post DIRECT_DATA_TX_EVENT event that will be handled
 *			when call emngr_handle_all_events to call the cbk direct_tx_evnt_cbk
 */
mac_status_enum_t mcps_data_send_req(mcps_data_req_params_st_t *ptr_st_mcps_data_req_params,  uint32_t mac_hndl);
#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_DATA_PURGE)
/**
 * @brief  The MCPS-PURGE.request primitive allows the next
 * 		higher layer to purge an MSDU from the transaction queue ,After request is completed
 * 		 mcps_purge_cfm callback is called to send MCPS-PURGE.confirm to upper layer
 *
 * @param   mac_hndl[in]	: The MAC instance handle
 * @param   msdu_hndl[in]	: The handle of the MSDU
 *
 * @retval void
 */
void mcps_purge_req(uint32_t mac_hndl, uint8_t msdu_hndl);
#endif /* (FFD_DEVICE_CONFIG || RFD_SUPPORT_DATA_PURGE) */
/**
 *
 * @brief   enable/disable RX for a given duration
 *
 * @param    mac_hndl      	  : [in] the MAC instance handle
 * @param    RxOnDuration     : [in] Duration to enable RX with it starting from the current time , 0 means disable.
 * @param    RangingRxControl : [in] Ranging RX control it should be set to 0 " RANGING_OFF ".
 *
 * @retval mac_status_enum_t :  status to be sent to the Host.
 */
mac_status_enum_t mlme_rx_enable_req(uint32_t mac_hndl,
								uint32_t    RxOnDuration,
								uint8_t     RangingRxControl);
/**
 *
 * @brief   adds new entry to the Power Control Information table or update an existing entry
 *
 * @param   mac_hndl 		 : [in] the MAC instance handle
 * @param   short_addrs 	 : [in] Short address of the link pair to transmit the packet to.
 * @param   ptr_ext_addrs 	 : [in] Pointer to Extended (IEEE) address of the link pair to transmit the packet to.
 * @param   tx_pwr_level 	 : [in] Tx power level of the link pair to transmit the packet to.
 * @param   last_rssi_level  : [in] RSSI of last packet received on the link pair
 * @param   nwk_negotiated   : [in] Flag = 0 during the joining / rejoining process , 1 after joining/rejoining
 * @param	cfm_flag		 : [in] Flag = 1 means that cfm is to be sent / = 0 otherwise
 *
 * @retval mac_status_enum_t :  status to be sent to the Host.
 */
mac_status_enum_t mlme_set_pwr_info_table_req(uint32_t mac_hndl,
								uint16_t    short_addrs,
								uint8_t     *ptr_ext_addrs,
								int8_t      tx_pwr_level,
								int8_t      last_rssi_level,
								uint8_t     nwk_negotiated,
								uint8_t 	cfm_flag);
/**
 *
 * @brief   primitive returns the Power Control Information entry for the link pair
 *
 * @param   mac_hndl 		: [in] the MAC instance handle
 * @param    short_addrs 	: [in] Short address of the link pair.
 * @param    ptr_ext_addrs  : [in] Pointer to Extended (IEEE) address of the link.
 *
 * @retval None .
 */
void mlme_get_pwr_info_table_req(uint32_t mac_hndl, mac_addrs_mode_enum_t enum_addr_mode, uint8_t  *ptr_addrs);

#if (FFD_DEVICE_CONFIG || RFD_SUPPORT_SEND_BEACON)
/**
 * @brief   send beacon frame
 *
 * @param   mac_hndl	 [in] : the MAC instance handle
 * @param   enum_bcon_typ[in] : beacon type [Beacon / Enhanced Beacon]
 * @param   bcon_tx_pwr  [in] : tx power to be added in the tx poweer IE
 * @param   ptr_st_bcon_send_req_params [in] : pointer to send beacon parameters
 *
 * @retval mac_status_enum_t
 *
 * @note 	this function will post DIRECT_DATA_TX_EVENT event that will be handled
 *		    when call emngr_handle_all_events to call the cbk direct_tx_evnt_cbk
 */
mac_status_enum_t mlme_send_bcon_frm(uint32_t mac_hndl, bcon_typ_t enum_bcon_typ,
		int8_t bcon_tx_pwr, mlme_bcon_send_req_params_st_t *ptr_st_bcon_send_req_params);
#endif
#if (SUPPORT_MAC_HCI_UART)
/**
 * @fn mlme_read_ieee_ext_addr
 *
 * @brief   read ext address
 *
 * @param   mac_hndl[in] : the MAC instance handle
 * @param   ext_addr[in] : extended address
 */
void mlme_read_ieee_ext_addr(uint32_t mac_hndl, uint8_t* ext_addr);
#endif /* SUPPORT_MAC_HCI_UART */

#if RADIO_CSMA
/**
 *
 * @brief   set csma enable flag
 *
 * @param   csma_en 		: [in] value for csma enable flag to be set
 *
 * @retval None .
 */
void mac_set_csma_en(uint8_t csma_en);
#endif /*end of RADIO_CSMA*/
/**
 *
 * @brief   set cca enable flag
 *
 * @param   cca_en 		: [in] value for cca enable flag to be set
 *
 * @retval None .
 */
void mac_set_cca_en(uint8_t cca_en);
/**
 * @fn mac_set_cca_threshold
 *
 * @brief   set cca threshold
 *
 * @param   mac_hndl	 : [in] the MAC instance handle
 * @param   cca_thresold : [in] value of cca threshold that set
 *
 * @retval mac_status_enum_t .
 */
mac_status_enum_t mac_set_cca_threshold(uint32_t mac_hndl, int8_t cca_thresold);
/**
 * @fn mac_get_cca_threshold
 *
 * @brief   get cca threshold
 *
 * @param   mac_hndl	 : [in] the MAC instance handle
 * @param   cca_thresold : [in] pointer to value of cca threshold
 *
 * @retval mac_status_enum_t .
 */
mac_status_enum_t mac_get_cca_threshold(uint32_t mac_hndl, int8_t * cca_thresold);

#if SUPPORT_ANT_DIV
/**
 * @fn mac_set_ant_div_params
 *
 * @brief   set antenna diversity parameters
 *
 * @param   mac_hndl	   : [in] the MAC instance handle
 * @param   ptr_ant_div_params : [in] pointer to antenna diversity params structure
 *
 * @retval mac_status_enum_t .
 */
mac_status_enum_t mac_set_ant_div_params(uint32_t mac_hndl, antenna_diversity_st* ptr_ant_div_params);

/**
 * @fn mac_get_ant_div_params
 *
 * @brief   get antenna diversity parameters
 *
 * @param   mac_hndl	   : [in] the MAC instance handle
 * @param   ptr_ant_div_params : [out] pointer to antenna diversity params structure
 *
 * @retval None .
 */
void mac_get_ant_div_params(uint32_t mac_hndl, antenna_diversity_st* ptr_ant_div_params);

/**
 * @fn mac_set_default_ant_id
 *
 * @brief   set the default antenna id to be used for transmission and reception
 *
 * @param   mac_hndl	    : [in] the MAC instance handle
 * @param   default_ant_id  : [in] the antenna id to be used as default
 *
 * @retval mac_status_enum_t .
 */
mac_status_enum_t mac_set_ant_div_enable(uint32_t mac_hndl, uint8_t enable);

/**
 * @fn mac_set_default_ant_id
 *
 * @brief   enable/disable antenna diversity
 *
 * @param   mac_hndl	: [in] the MAC instance handle
 * @param   enable      : [in] enable:1 / disable:0
 *
 * @retval mac_status_enum_t .
 */
mac_status_enum_t mac_set_default_ant_id(uint32_t mac_hndl, uint8_t enable);

/**
 * @fn mac_set_ant_div_rssi_threshold
 *
 * @brief   set antenna diversity rssi threshold
 *
 * @param   mac_hndl	     : [in] the MAC instance handle
 * @param   rssi_threshold   : [in] rssi threshold to compare with during antenna diversity measurements
 *
 * @retval mac_status_enum_t .
 */
mac_status_enum_t mac_set_ant_div_rssi_threshold(uint32_t mac_hndl, int8_t rssi_threshold);
#endif /* SUPPORT_ANT_DIV */
#endif /* INCLUDE_MAC_HOST_INTF_H_ */
/**
 * @}
 */
