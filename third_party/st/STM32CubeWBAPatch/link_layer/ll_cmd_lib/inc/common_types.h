/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/common_types.h#1 $*/
/**
 ********************************************************************************
 * @file    common_types.h
 * @brief   This file contains common includes for BLE FW LL.
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

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_
#include "_40nm_reg_files/DWC_ble154combo.h"
#include "bsp.h"
#include "ll_fw_config.h"
#include "mem_intf.h"
#include "os_wrapper.h"


/********************************************************************** */
/****************** BLE MAC COMBO Configurations*********************** */
/********************************************************************** */
//#ifdef BLE
#define SUPPORT_BLE									1
//#else
//#define SUPPORT_BLE									0
//#endif

#ifdef MAC
#define SUPPORT_MAC									1
#else
#define SUPPORT_MAC									0
#endif
#ifdef ANT_ROLE
#define SUPPORT_ANT								    1

#ifdef SUPPORT_ANT_TESTING
#define SUPPORT_ANT_HCI_UART								    1
#else
#define SUPPORT_ANT_HCI_UART								    0
#endif /* SUPPORT_ANT_HCI_UART*/

#else
#define SUPPORT_ANT								    0
#define SUPPORT_ANT_HCI_UART                                0
#endif

#ifdef MAC_LAYER
#define MAC_LAYER_BUILD 1
#else
#define MAC_LAYER_BUILD 0
#endif

#ifdef SUPPORT_MAC_HCI_UART
#define SUPPORT_MAC_HCI_UART       1
#else
#define SUPPORT_MAC_HCI_UART       0
#endif

#ifdef SUPPORT_AUG_MAC_HCI_UART
#define SUPPORT_AUG_MAC_HCI_UART       1
#else
#define SUPPORT_AUG_MAC_HCI_UART       0
#endif

#if((!SUPPORT_BLE)&&(SUPPORT_MAC || SUPPORT_AUG_MAC_HCI_UART)&&(RAL_NUMBER_OF_INSTANCE>1))
#error "BLE controller must be enabled to support MAC multiple Instances"
#endif

#if((SUPPORT_MAC)&&(SUPPORT_AUG_MAC_HCI_UART)&&(RAL_NUMBER_OF_INSTANCE<2))
#error "RAL_NUMBER_OF_INSTANCE must be 2 to support MAC and Augmented MAC"
#endif

#if((!SUPPORT_BLE)&&(SUPPORT_MAC)&&(SUPPORT_ANT))
#error "BLE controller must be enabled to support MAC and ANT Coexistence"
#endif

#define SUPPORT_COEXISTENCE							((SUPPORT_BLE&&SUPPORT_MAC) || (SUPPORT_BLE&&SUPPORT_ANT))
#define SUPPORT_ANT_COEXISTENCE						(SUPPORT_BLE&&SUPPORT_ANT)
/****************** User configuration **********************************/
#define CS_TESTING TRUE

/********************* Macros **********************************/

#ifndef SUCCESS
#define		SUCCESS				0
#endif
#ifndef GENERAL_FAILURE
#define		GENERAL_FAILURE		-1
#endif
#ifndef GENERAL_ERROR_STATUS
#define		GENERAL_ERROR_STATUS		0xFF
#endif

#ifndef TRUE
#define TRUE				1
#endif /* TRUE */

#ifndef FALSE
#define FALSE				0
#endif /* FALSE */

#ifndef NULL
#define NULL				((void *)0)
#endif	/* NULL */

#define UNUSED_VALUE		0
#define UNUSED_PTR			NULL

#define MEMSET(ptr_memory, value, memory_size)				ble_memset(ptr_memory, value, memory_size)
#define MEMCPY(ptr_destination, ptr_source, memory_size)	ble_memcpy(ptr_destination, ptr_source, memory_size)
#define MEMCMP(ptr_destination, ptr_source, memory_size)	ble_memcmp(ptr_destination, ptr_source, memory_size)
#define MEMMOV(ptr_destination, ptr_source, memory_size)	ble_memmov(ptr_destination, ptr_source, memory_size)
#define MEMCPY_N_BYTES(ptr_dest, ptr_src,no_bytes ,keep_endian)	 ble_memcpy_n_bytes(ptr_dest,ptr_src ,no_bytes ,keep_endian)



extern os_mutex_id g_ll_lock;
#define LL_LOCK()	os_rcrsv_mutex_wait(g_ll_lock,0xffffffff)
#define LL_UNLOCK()	os_rcrsv_mutex_release(g_ll_lock)

#ifndef SUPPORT_ANT_DIV
#define SUPPORT_ANT_DIV 0
#endif

#if SUPPORT_MAC
#define RADIO_MAC_TX_DONE_EVENT_MAX     				1
#define RAL_SM_DONE_EVENT_MAX 							RADIO_TX_RX_PACKET_BLOCK_COUNT
#define ED_TIMER_EVENT_MAX								1
#define MAX_MLME_TIMER_EVENT							MAC_NUMBER_OF_INSTANCE
#define MAX_DIRECT_DATA_TX_EVENT						MAC_NUMBER_OF_INSTANCE
#define MAX_INDIRECT_DATA_TIMEOUT_EVENT					MAX_NUMBER_OF_INDIRECT_DATA
#define PRDC_CLBR_TMR_EVENT_MAX 						1
#define CSL_RCV_TMR_EVENT_MAX   						1

/* Size in octets of extended address used in security processing */
#define EXT_ADDRESS_LENGTH								8
#endif /* SUPPORT_MAC */

#if SUPPORT_MAC && SUPPORT_OPENTHREAD_1_2
/* compiler flag to control supporting of CSL transmitter , RADIO TX at specific time , 1  supported , 0 not supported */
#define CONFIG_MAC_CSL_TRANSMITTER_ENABLE                1
/* compiler flag to control supporting of CSL receiver , RADIO RX at specific time, 1  supported , 0 not supported */
#define CONFIG_MAC_CSL_RECEIVER_ENABLE                   1
/* compiler flag to control supporting of Radio security handling */
#define SUPPORT_RADIO_SECURITY_OT_1_2                    1
/* compiler flag to control supporting of Enhanced Ack Link metrics probing  */
#define SUPPORT_ENH_ACK_LINK_METRICS_PROBING_OT_1_2 	 1
/* compiler flag to control supporting of Time sync experimental feature of OT 1.2
 * (it is not a thread 1.2 shall not be added except for with OT 1.2 for testing purpose) */
#define SUPPORT_TIME_SYNC_OT_1_2						 1
#else
#define CONFIG_MAC_CSL_TRANSMITTER_ENABLE                0
#define CONFIG_MAC_CSL_RECEIVER_ENABLE                   0
#define SUPPORT_RADIO_SECURITY_OT_1_2           		 0
#define SUPPORT_ENH_ACK_LINK_METRICS_PROBING_OT_1_2 	 0
#define SUPPORT_TIME_SYNC_OT_1_2						 0
#endif /*SUPPORT_MAC && SUPPORT_OPENTHREAD_1_2 */

/* end of radio activity custom command flag */
#define END_OF_RADIO_ACTIVITY_REPORTING				1 /* Enable\Disable end of radio activity reporting feature. Enable:1 - Disable:0 */

/* Supported PHYs*/
typedef enum {
	LE_NO_CHANGE			= 0x00,
	LE_1M  					= 0x01,
	LE_2M 					= 0x02,
	LE_CODED_S8				= 0x03,
	LE_CODED 				= 0x04,
/*===============  Channel Sounding  ===============*/
#if (SUPPORT_CHANNEL_SOUNDING &&( SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION))
	LE_2M_2BT				= 0x08,
#endif /*SUPPORT_CHANNEL_SOUNDING &&( SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION)*/
#if (SUPPORT_LE_POWER_CONTROL)
	LE_PHY_UNDEFINED		= 0xFC,
	NEW_PHY_CODED_S2		= 0xFD,
	NEW_PHY_CODED_S8		= 0xFE,
	NEW_PHY_CODED_COMBINED	= 0xFF,
#endif /* SUPPORT_LE_POWER_CONTROL */

	/* PHY Recommendations - Combined Values */
#if ((SUPPORT_CONNECTED_ISOCHRONOUS && SUPPORT_MASTER_CONNECTION) || SUPPORT_BRD_ISOCHRONOUS)
	RECOMMEND_1M_2M			= 0x03,
	RECOMMEND_1M_CODED		= 0x05,
	RECOMMEND_2M_CODED		= 0x06,
	RECOMMEND_1M_2M_CODED 	= 0x07,
#endif /* ((SUPPORT_CONNECTED_ISOCHRONOUS && SUPPORT_MASTER_CONNECTION) || SUPPORT_BRD_ISOCHRONOUS) */
} tx_rx_phy_e;

/**
 * @brief time stamp structure.
 */
typedef struct time_st {
	uint32_t time_stamp_base;
	uint16_t time_stamp_fine;
	uint8_t  overflow_flag;
} ble_time_t, *ble_time_p;

typedef enum dpslp_state {
	DEEP_SLEEP_ENABLE  = 0x01,
	DEEP_SLEEP_DISABLE = 0x00
} dpslp_state_e;

/**
 * @enum crypto_endian_enum_t
 * @brief States the available endian formats.
 *
 */
typedef enum {
	CRYPTO_LITTLE_ENDIAN,
	CRYPTO_BIG_ENDIAN
} crypto_endian_enum_t;

/**
 * @enum security_mode_enum_t
 * @brief Contains the available security modes.
 *
 * Note: The enum values should be the same as specified in the register
 *  address header file.
 */
typedef enum {
	ECB_DECRYPT	= 0,
	CCM_DECRYPT	= 1,
	ECB_ENCRYPT	= 2,
	CCM_ENCRYPT	= 3,
	CTR_ENCRYPT = 4,
	CTR_DECRYPT = 5,
	MODES_MAX_NUM
} security_mode_enum_t;

#if SUPPORT_MAC
typedef enum ral_phy_rate_enum {
#if SUPPORT_A_MAC
	RAL_RATE_125K = 0x00,
	RAL_RATE_1M   = 0x02,
	RAL_RATE_2M   = 0x03,
#endif
	RAL_RATE_256K  = 0x01
} ral_phy_rate_enum_t;

#endif /*SUPPORT_MAC*/
/**
 * @enum extrnl_evnt_priority_e
 * @brief External Event priority
 */
typedef enum _extrnl_evnt_priority_e
{
	PRIORITY_DEFAULT,
	PRIORITY_HIGH,
	PRIORITY_CRITICAL
}extrnl_evnt_priority_e;

/**
 * @enum extrnl_evnt_state_e
 * @brief External Event Blocked State and reason
 */
typedef enum _extrnl_evnt_state_e
{
	STATE_BLOCKED_UNKNOWN,
	STATE_BLOCKED_PRIORITY,
	STATE_BLOCKED_CANCELLED,
	STATE_BLOCKED_LATE,
	STATE_BLOCKED_DEADLINE,
	STATE_NOT_BLOCKED
}extrnl_evnt_state_e;

/**
  * @brief Enumeration of the source type used to drive the sleep timer.
  */
typedef enum _slptmr_src_type_e {
	CRYSTAL_OSCILLATOR_SLPTMR	= 0x00,
#if (USE_NON_ACCURATE_32K_SLEEP_CLK)
	RCO_SLPTMR					= 0x01,
	RTC_SLPTMR					= 0x02
#endif /* USE_NON_ACCURATE_32K_SLEEP_CLK */
}slptmr_src_type_e;

/**
  * @brief Enumeration of the antenna diversity interval type.
  */
#if SUPPORT_MAC && SUPPORT_ANT_DIV
typedef enum ant_intrv_type_enum {
	NO_TYPE,
	FIXED_TIME,
	PACKETS_NUMBER
} ant_intrv_type_enum_t;

/*
 * @brief structure that hold antenna diversity parameters information.
 */
typedef struct _antenna_diversity_st{
	ant_intrv_type_enum_t ant_intrv_type;                /* antenna interval type: FIXED_TIME(us) or PACKETS_NUMBER(n) */
	uint32_t ant_intrv_value;                            /* antenna interval value based on type; us for FIXED_TIME, n for PACKETS_NUMBER */
	uint16_t wntd_coord_shrt_addr;	                     /* wanted coordinator/router short address */
	uint8_t wntd_coord_ext_addr[EXT_ADDRESS_LENGTH];	 /* wanted coordinator/router extended address */
	uint8_t max_rx_ack_retries;                          /* max number of retries to receive ack in case of ack error reception*/
} antenna_diversity_st;
#endif /* SUPPORT_MAC && SUPPORT_ANT_DIV */

/*
 * @brief structure that hold some information about the data transmitted across layers.
 */
typedef struct ble_buff_hdr_st {
	/* pointer to buffer allocated in TX/RX buffer*/
	uint8_t *buff_start;
	/* pointer to next ble_buff_hdr node */
	struct ble_buff_hdr_st *next_pkt;
	/* Total buffer size */
	uint16_t total_len;
	/* offset to data location from the packet start pointed to by buff_start */
	uint16_t data_offset;
	/* data size for the data pointed to by buff_start + data_offset*/
	uint16_t data_size;
	/* bit field flags to be used for identifying buffer type data/control buffer,
	 * for data is it a BLE/MAC/ANT, does it contain complete SDU packet of it is a fragment,
	 * if it is a fragment is it the start or continuation or last fragment. */
	uint8_t ble_hdr_flags;
} pkt_buff_hdr_t ,ble_buff_hdr_t, *ble_buff_hdr_p;

#if (SUPPORT_BRD_ISOCHRONOUS || SUPPORT_SYNC_ISOCHRONOUS ||  (SUPPORT_CONNECTED_ISOCHRONOUS && ( SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION)))
/**
 * @brief:  PDU info structure
 */
typedef struct _iso_pdu_buff_hdr_st {
	ble_buff_hdr_t  pkt;	/* PDU Packet */
#if(SUPPORT_CONNECTED_ISOCHRONOUS && (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION))
	uint8_t flsh_tmout_cnt;		/* flush timeout counter */
	uint8_t flsh_tmout_subevnt_cnt;		/* flush timeout subevent number */
	uint8_t rx_flsh_tmout_cnt_updtd;	/* flush timeout count updated or not flag */
#endif //(SUPPORT_CONNECTED_ISOCHRONOUS && (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION))
	uint8_t status;
} iso_pdu_buff_hdr_st;
/**
 * @brief:  SDU info structure
 */
typedef struct _sdu_buf_hdr_st {
	iso_pdu_buff_hdr_st* ptr_last_pdu_buff_hdr;		/* pointer to the last pdu buffer header pointed to this sdu */
	struct _sdu_buf_hdr_st* ptr_nxt_sdu_buff_hdr;	/* next sdu pointer */
	uint32_t*ptr_sdu_buffer;	/* pointer to SDU buffer in system memory for rx*/
	uint32_t time_stamp;	/* Time Stamp associated with this SDU */
	uint32_t time_offset;	/* Time Offset used only in framed SDUs */
	uint16_t pkt_sqnc_num;	/* Packet Sequence Number */
	uint16_t iso_sdu_len;	/* ISO SDU data real length */
	uint8_t  pkt_status_flag;
	uint8_t  pb_flag;      /* PB_flag used in rx */
	/*
	 * the first one will have the value 10 complete sdu until a new sdu fragment will be received then it will be 00 first fragment
	 * the subsequent one will have the value 11 last fragment until a new sdu fragment will be received then it will be 01 continuation fragment
	 *
	 * */
} iso_sdu_buf_hdr_st, *iso_sdu_buf_hdr_p;
#endif  /* (SUPPORT_BRD_ISOCHRONOUS || SUPPORT_SYNC_ISOCHRONOUSs ||  (SUPPORT_CONNECTED_ISOCHRONOUS && ( SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION))) */

#if SUPPORT_LE_PAWR_ADVERTISER_ROLE
/**
 * brief: PAWR host buffer struct
 */
typedef struct _pawr_host_buffer {
	uint8_t buffer[257];
	uint8_t total_data_lenth;
	uint8_t number_of_reports;
}pawr_host_buffer;
#endif /* SUPPORT_LE_PAWR_ADVERTISER_ROLE */


/*
 * @brief   Transport layer event
 */
typedef enum {
	HCI_CMD_PCKT = 0x01,
	HCI_ACL_DATA_PCKT = 0x02,
	HCI_EVNT_PCKT = 0x04,
	HCI_ISO_DATA_PCKT = 0x05,


#if (SUPPORT_MAC && SUPPORT_MAC_HCI_UART)
	HCI_MAC_REQ = 0x0A,
	HCI_MAC_CFM = 0x0B,
	HCI_MAC_KEY_TBL_CFM = 0x0E,
#endif /* SUPPORT_MAC && SUPPORT_MAC_HCI_UART */
#if (SUPPORT_ANT_HCI_UART)
	HCI_ANT_REQ = 0x10,
	HCI_ANT_CFM = 0x11,
#endif /* SUPPORT_ANT_HCI_UART */
#if (SUPPORT_AUG_MAC_HCI_UART)
	AUG_HCI_MAC_REQ = 0x0C,
	AUG_HCI_MAC_CFM = 0x0D,
#endif /* SUPPORT_AUG_MAC_HCI_UART */

} event_t;



/*constant flags to be used with ble_buff_hdr_t:ble_hdr_flags*/
#define BLE_BUFF_HDR_STRT_PKT 			(1<<0)
#define BLE_BUFF_HDR_CNTRL_PKT 			(1<<1)
#define BLE_BUFF_HDR_BUFF_FRGMNTD 		(1<<2)
#define BLE_BUFF_HDR_EVNT_CMD_PCKT		(1<<3)
#define BLE_BUFF_HDR_ACL_DATA_PCKT		(1<<4)

#if ((SUPPORT_CONNECTED_ISOCHRONOUS && (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION)) \
		||(SUPPORT_BRD_ISOCHRONOUS || SUPPORT_SYNC_ISOCHRONOUS))
#define BLE_BUFF_HDR_NULL_PCKT		(1<<3)
/*
 * BLE_BUFF_HDR_ISO_DATA_PCKT_BIT1  : BLE_BUFF_HDR_ISO_DATA_PCKT_BIT0   = Mode
 * 1                                : 1                                    = Unframed CIS Data PDU; end fragment
 * of an SDU or a complete SDU. LLID (0b00) 0                                : 1 = Unframed CIS Data PDU;
 * start or continuation fragment of an SDU.
 * 1                                : 0                                    = Framed CIS Data PDU; one or
 * more segments of an SDU.
 * */
#define BLE_BUFF_HDR_ISO_DATA_PCKT_BIT0		(1<<5)
#define BLE_BUFF_HDR_ISO_DATA_PCKT_BIT1		(1<<6)
#endif /* (SUPPORT_CONNECTED_ISOCHRONOUS && (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION)) */
#if (SUPPORT_MAC && SUPPORT_MAC_HCI_UART)
#define BLE_BUFF_HDR_MAC_CMD_PCK		(1<<6)
#define BLE_BUFF_HDR_MAC_KEY_TBL_CMD_PCK		((1<<7)|(1<<4))
#endif /* (SUPPORT_MAC && SUPPORT_MAC_HCI_UART) */
#if (SUPPORT_ANT_HCI_UART)
#define BLE_BUFF_HDR_ANT_CMD_PCK		(1<<7)
#endif  /* SUPPORT_ANT_HCI_UART */
#if (SUPPORT_AUG_MAC_HCI_UART)
#define BLE_BUFF_HDR_AUG_MAC_CMD_PCK		((1<<7)|(1<<6))
#endif


/**
 * The default PHY periodic calibration period in second. this Macro can be set to any value , Zero means that phy periodic calibration is disabled
 */
#ifndef DEFAULT_PHY_CALIBRATION_PERIOD
#define DEFAULT_PHY_CALIBRATION_PERIOD        		10	/* Time period for PHY calibration = 10s */
#endif /* DEFAULT_PHY_CALIBRATION_PERIOD */

#if defined(PHY_40nm_3_00_a) || defined(PHY_40nm_3_40_a)
#define SUPPORT_MAC_PHY_CONT_TESTING_CMDS 1
#else
#define SUPPORT_MAC_PHY_CONT_TESTING_CMDS 0
#if(SUPPORT_MAC_PHY_CONT_TESTING_CMDS)
#error "SUPPORT_MAC_PHY_CONT_TESTING_CMDS must be enabled for PHY_40nm_3_00_a or PHY_40nm_3_40_a only"
#endif/*end of (SUPPORT_MAC_PHY_CONT_TESTING_CMDS) */
#endif /*end of defined(PHY_40nm_3_00_a) || defined(PHY_40nm_3_40_a) */

#ifndef EXTERNAL_CUSTOM_CMDS
#define EXTERNAL_CUSTOM_CMDS						0	/* Indicates that an external custom HCI commands module exists */
#endif /* EXTERNAL_CUSTOM_CMDS */
#ifndef SUPPORT_ZIGBEE_PHY_CERTIFICATION
#define SUPPORT_ZIGBEE_PHY_CERTIFICATION   0  /* 0 disable , 1 enable .. used to enable support of hci command required to implement zigbee phy Test cases*/
#endif /* SUPPORT_ZIGBEE_PHY_CERTIFICATION */

#if (!USE_HCI_TRANSPORT) && (SUPPORT_BLE)						  /* SUPPORT_HCI_EVENT_ONLY cannot be supported with default HCI_transport */
/* if this marco is enabled it will enable  the below features
 *  -Queue events - ACL - ISO - Reports into different queues
   -Allow host to register callback to refuse current controller event and receive it later with another callback*/
#define SUPPORT_HCI_EVENT_ONLY				1

#else
#define SUPPORT_HCI_EVENT_ONLY				0
#endif/* (!USE_HCI_TRANSPORT) && (SUPPORT_BLE) */

#ifndef SUPPORT_HCI_EVENT_ONLY_TESTING
#define SUPPORT_HCI_EVENT_ONLY_TESTING				0
#endif /* SUPPORT_HCI_EVENT_ONLY_TESTING */

#ifndef SUPPORT_HW_AUDIO_SYNC_SIGNAL
#define SUPPORT_HW_AUDIO_SYNC_SIGNAL       0
#endif /* SUPPORT_HW_AUDIO_SYNC_SIGNAL */

#if SUPPORT_LE_PAWR_SYNC_ROLE
#define SUPPORT_PAWR_CUSTOM_SYNC			1
#else
#define SUPPORT_PAWR_CUSTOM_SYNC			0
#endif /* SUPPORT_LE_PAWR_SYNC_ROLE */

#define PAWR_TESTING						0
#ifndef SUPPORT_LE_PAWR_ADVERTISER_ROLE
#define SUPPORT_LE_PAWR_ADVERTISER_ROLE		0
#endif



#ifndef SUPPORT_TIM_UPDT
#define SUPPORT_TIM_UPDT					1
#endif /* SUPPORT_TIM_UPDT */

#ifndef SUPPORT_RX_DTP_CONTROL
#define SUPPORT_RX_DTP_CONTROL				1 /* Enable\Disable ACL Rx data throughput feature */
#endif /* SUPPORT_RX_DTP_CONTROL */

#ifndef SUPPORT_CUSTOM_ADV_SCAN_TESTING
#define SUPPORT_CUSTOM_ADV_SCAN_TESTING		0
#endif /* SUPPORT_CUSTOM_ADV_SCAN_TESTING */

#ifndef SUPPORT_CHANNEL_SOUNDING
#define SUPPORT_CHANNEL_SOUNDING			0
#endif /* SUPPORT_CHANNEL_SOUNDING */

#ifndef SUPPORT_EXT_FEATURE_SET
#define SUPPORT_EXT_FEATURE_SET 0
#endif /* SUPPORT_EXT_FEATURE_SET */


#ifndef SUPPORT_CONFIGURABLE_GAIN_FIX
#define SUPPORT_CONFIGURABLE_GAIN_FIX				0 /* Enable\Disable configurable gain fix support */
#endif /* SUPPORT_CONFIGURABLE_GAIN_FIX */

#if SUPPORT_CONFIGURABLE_GAIN_FIX
#define PREEMPH_GAIN_COEFF_STEP_SIZE				10 			/* percentage margin of single step */
#define GAIN_FIX_WAKEUP_TIME_OVERHEAD				4 			/* in sleep timer units, the added time overhead from patching all pre-emphasis coefficients */
#else
#define GAIN_FIX_WAKEUP_TIME_OVERHEAD				0
#endif /* SUPPORT_CONFIGURABLE_GAIN_FIX */

#ifndef SUPPORT_PHY_SHUTDOWN_MODE
#if defined(PHY_40nm_3_60_a_tc) || defined(PHY_40nm_3_00_a) || defined(PHY_40nm_3_40_a)
#define SUPPORT_PHY_SHUTDOWN_MODE					1 /* Enable\Disable phpy shutdown mode support */
#else
#define SUPPORT_PHY_SHUTDOWN_MODE					0
#endif /* defined(PHY_40nm_3_60_a_tc) || defined(PHY_40nm_3_00_a) || defined(PHY_40nm_3_40_a) */
#endif /* SUPPORT_PHY_SHUTDOWN_MODE */

#if SUPPORT_PHY_SHUTDOWN_MODE
#define PHY_SHUTDOWN_WAKEUP_TIME_OVERHEAD			2 			/* in sleep timer units, the added time overhead from executing override seqeuences needed in phy shutdown mode */
#else
#define PHY_SHUTDOWN_WAKEUP_TIME_OVERHEAD			0
#endif /* SUPPORT_PHY_SHUTDOWN_MODE */

#ifndef SUPPORT_GNRC_SCHDLR_IF
#define SUPPORT_GNRC_SCHDLR_IF				1
#endif
#ifndef NEAR_AUX_AFTER_EXT_SLEEP_TIMER_SCHEDULING
#define NEAR_AUX_AFTER_EXT_SLEEP_TIMER_SCHEDULING 	0
#endif /* NEAR_AUX_AFTER_EXT_SLEEP_TIMER_SCHEDULING */

#if (SUPPORT_CHANNEL_SOUNDING &&( SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION))
/**
 * @brief holds the possible values for cs roles
 */
typedef enum _cs_role_e
{
	CS_ROLE_INITIATOR,
	CS_ROLE_REFLECTOR
} cs_role_e;

/**
 * @brief holds the possible values for cs step mode types
 */
typedef enum _cs_step_mode_type_e
{
	CS_STEP_MODE_0,
	CS_STEP_MODE_1,
	CS_STEP_MODE_2,
	CS_STEP_MODE_3,
	CS_STEP_MODE_NONE = 0xFF
} cs_step_mode_type_e;

/**
 * @brief cs host buffer structure
 */
typedef struct _cs_host_buffer {
	uint8_t buffer[257]; /*CS_HOST_BUFFER_DATA_MAX (255) + HCI_EVENT_HEADER_LEN(2)*/
}cs_host_buffer;

#endif /*SUPPORT_CHANNEL_SOUNDING &&( SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION)*/

#endif /*COMMON_TYPES_H_*/
