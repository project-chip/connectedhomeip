/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/ral.h#1 $*/
/**
 ********************************************************************************
 * @file    ral.h
 * @brief   The file include description for the RAL (Radio Abstraction Layer) interfaces and call backs,
 * 			RAL provides functionalities to start, stop and manage different types of events [Transmission - Reception - Energy scanning].
 *
 *
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

#ifndef INCLUDE_RAL_H_
#define INCLUDE_RAL_H_

#include "common_types.h"
#include "os_wrapper.h"
#include "mac_host_intf.h"
#include "instance.h"
#include "radio.h"
#if SUPPORT_ENH_ACK_LINK_METRICS_PROBING_OT_1_2
#include "platform.h"
#endif
/* Defines */
/* @brief: An index used in multi-instance for specifying which instance to perform operation on */
typedef uint8_t ral_instance_t;
/* @brief: Defined for each ral event to identify event's current state
 *
 *
 *						ral event allocation
 *						  ==============
 *ral_add_tx_fifo() _ _ _ =  RAL_IDLE  =
 *						  ==============
 *
 *																					      ==============
 *											      			_ _(Requires ack)_ _ _ _ _ _ _= RAL_RX_ACK =
 *						ral event allocation     		  /             			      ==============
 *						  =============                	 /
 * ral_start_tx() _ _ _ _ = RAL_TX_PKT =_ _ ral_isr()_ _/
 *						  =============		   			\
 *						  					    		 \								  ==============
 *						  					     		  \_ _(Doesn't require ack)_ _ _ _= RAL_TX_PKT =
 *																						  ==============
 *
 *																				      	  ==============
 *											      			_ _(Requires ack)_ _ _ _ _ _ _= RAL_TX_ACK =
 *						ral event allocation     		  /             			      ==============
 *						  =============                	 /
 * ral_start_rx() _ _ _ _ = RAL_RX_PKT =_ _ ral_isr()_ _/
 *						  =============		   			\
 *						  					    		 \								  ==============
 *						  					     		  \_ _(Doesn't require ack)_ _ _ _= RAL_RX_PKT =
 *																						  ==============
 *						ral event allocation
 *						  ==============
 *ral_ed_scan() _ _ _ _ _ =   RAL_ED   =
 *						  ==============
 *
 *
 *
 * */
typedef enum ral_event_state_enum {
	RAL_IDLE,
	RAL_RX_PKT,
	RAL_TX_PKT,
	RAL_RX_ACK,
	RAL_TX_ACK,
	RAL_ED
} ral_event_state_enum_t;
/* @brief: used for ral configuration assignment */
typedef enum ral_state_enum {
	RAL_DISABLE = 0,
	RAL_ENABLE = 1
} ral_state_enum_t;

/* @brief: Error codes defined in ral casted to Openthread codes in radio */
typedef enum ral_error_enum {
    RAL_ERROR_NONE = 0,
    RAL_ERROR_FAILED = 1,
    RAL_ERROR_DROP = 2,
    RAL_ERROR_NO_BUFS = 3,
    RAL_ERROR_BUSY = 5,
    RAL_ERROR_INVALID_PARAMETERS = 7,
    RAL_ERROR_SECURITY = 8,
    RAL_ERROR_NO_ADDRESS = 10,
    RAL_ERROR_ABORT = 11,
    RAL_ERROR_NOT_SUPPORTED = 12,
    RAL_ERROR_INVALID_STATE = 13,
    RAL_ERROR_NO_ACK = 14,
    RAL_ERROR_CCA_FAILURE = 15,
    RAL_ERROR_FCS = 17,
    RAL_ERROR_NO_FRAME_RECEIVED = 18,
    RAL_ERROR_INVALID_SOURCE_ADDRESS = 20,
    RAL_ERROR_DESTINATION_ADDRESS_FILTERED = 22,
	RAL_ERROR_TIMER_ISR = 23,
#if SUPPORT_ENH_ACK_LINK_METRICS_PROBING_OT_1_2
	RAL_ERROR_LINK_METRICS_INVALID_ARGS,
	RAL_ERROR_LINK_METRICS_NOT_FOUND,
	RAL_ERROR_LINK_METRICS_NO_BUF,
#endif /* SUPPORT_ENH_ACK_LINK_METRICS_PROBING_OT_1_2 */
#if SUPPORT_ANT_DIV
	RAL_ERROR_AD_NOT_IN_CONFIG_STATE,
#endif /* SUPPORT_ANT_DIV */
    RAL_ERROR_GENERIC = 255
} ral_error_enum_t;

/* @brief: Define power states for ral where:
 *
 * RAL_POWER_SLEEP: power state of the ral when not executing any event.
 * RAL_POWER_ACTIVE: power state of the ral before starting of any event.
 * */
typedef enum ral_power_state_enum {
	RAL_POWER_SLEEP,
	RAL_POWER_ACTIVE
} ral_power_state_enum_t;

/* @brief: Define new TX retry type:
 *
 * CONTINUE_CSMA_RETRY: continue of CSMA retry from csma_backoff_count.
 * START_NEW_FULL_TX_RETRY: start new TX retry from frm_retries_count.
 * */
typedef enum tx_new_retry_enum {
	CONTINUE_CSMA_RETRY,
	START_NEW_FULL_TX_RETRY
}tx_new_retry_enum_t;

/* @brief: Define pkt source for transmission when call ral_start_tx() to be added properly in ral pkt.
 *
 * RAL_SOURCE_PACKET: used in case of single packet transmission mode the pkt is passed in ral_start_tx() and then copied to the allocated event.
 *
 *   */
typedef enum ral_pkt_src_enum {
#if SUPPORT_A_MAC
/* RAL_SOURCE_FIFO: this mode is defined only for A_MAC. And used in case of burst transmission each packet is copied to a ral event using ral_add_tx_fifo().
 * 					all ral events linked to each other in a linked list which is attached to the ral instance initialized by the application.
 */
	RAL_SOURCE_FIFO,
#endif
	RAL_SOURCE_PACKET
} ral_pkt_src_enum_t;
/* @brief: Define type of acknowledgment packet used in MAC 802.15.4.*/
typedef enum ral_ack_type_enum {
	RAL_NO_ACK,
#if SUPPORT_A_MAC
	RAL_ACK_CUSTOM,
#endif
	RAL_ACK_MAC
} ral_ack_type_enum_t;
/* @brief: Define ral time structure that contains fine and base */
typedef ble_time_t ral_time_st;

/**
 * @struct ral_pkt_st
 * @brief ral packet structure
 *
 * This structure contains the parameters of the sent/received ral packet
 *
 */
typedef struct _ral_pkt_st {
	uint8_t * ptr_pyld;						/* pointer to packet */
	ral_time_st time_stamp;					/* exact time in which the packet transmitted/received */
	uint16_t pyld_len;						/* packet length */
	uint8_t channel;						/* channel at which the packet will be transmitted */
	uint8_t rxchannelaftertxdone;			/* The RX channel after frame TX is done (after all frame retries - ack received, or timeout, or abort).*/
    union
    {
        struct
        {
        	ral_pkt_src_enum_t pkt_src;		/* source of transmitted packet */
#if SUPPORT_RADIO_SECURITY_OT_1_2
        	uint8_t *sec_key;				/* pointer to the security key used in sec processing */
			uint8_t is_sec_proc_by_radio;	/* flag to mark if security processed by radio or not*/
#endif
        	int8_t tx_power;				/* power of transmitted packet */
        	uint8_t last_tx_pkt;			/* last transmitted packet flag */
        	uint8_t csl_frame;			   /* True only if the current TX frame is a CSL frame */
        } tx_info;

        struct
        {
#if SUPPORT_RADIO_SECURITY_OT_1_2
        	uint32_t ack_frm_cntr;  		/* frame counter used for secured Enhanced ack */
        	uint8_t ack_key_id;				/* key index used for secured Enhanced ack */
        	uint8_t is_sec_enh_ack; 		/* flag to mark usage of secured Enhanced ack */
#endif
            int8_t  rssi;					/* received signal strength indicator */
            uint8_t lqi;					/* link quality indicator */
            uint8_t ackFrmPending;			/* This indicates if this frame was acknowledged with frame pending set */
        } rx_info;
    } tx_rx_u;
} ral_pkt_st;

/**
 * @struct ral_evnt_info_st
 * @brief event information for RAL
 *
 * This structure contains the full information of the transmitted/received event
 */
typedef struct _ral_evnt_info_st{
	struct _ral_evnt_info_st * ptr_nxt_evnt;			/* pointer to next event, used in case of FIFO transmission */
	union
	{
		struct
		{
			ral_pkt_st * ptr_pkt;						/* pointer to data packet */
			ral_pkt_st * ptr_ack_pkt;					/* pointer to ack packet used by this event */
		} pkt_info;
		struct
		{
			uint32_t ed_scn_durn;						/* Energy detection scan duration */
			int8_t ed_max_rssi;							/* Energy detection max rssi value */
		} ed_info;
	} ral_evnt_info_u;
	uint32_t ral_status_mask;							/* HW error mask of the received packet */
	ral_instance_t ral_instance;						/* ral identifier of this event */
	ral_event_state_enum_t event_state;					/* event type */
	ral_error_enum_t evnt_error;						/* event error passed to higher layers */
#if (SUPPORT_PTA)
	uint8_t tx_pta_counter;								/* Either the PTA Tx or Rx reject counter depending on the packet type */
	uint8_t rx_pta_counter;								/* Either the PTA Tx or Rx reject counter depending on the packet type */
#endif /* SUPPORT_PTA */
} ral_evnt_info_st;

/**
 * @struct ral_mac_fltr_confg_st
 * @brief MAC filteration parameters
 *
 * MAC filteration parameters used by HW to filter received packets.
 * Used only by MAC 802.15.4
 *  */
typedef struct _ral_mac_fltr_confg_st {
	uint8_t ext_addr[EXT_ADDRESS_LENGTH];		/* the device extended address to compare the received address with */
	uint16_t short_addr;						/* the device short address to compare the received address with */
	uint16_t pan_id;							/* the device pan id to compare the received pan id in the packet with */
	ral_state_enum_t mac_fltr_state;			/* MAC filter state. If disabled so the promiscuous mode is enabled */
	uint8_t mac_implicit_broadcast;				/* MAC ImplictBoradcast PIB that is set in MAC layer  */
	uint8_t is_pan_coord;						/* the first bit used to determine if the device is pan coordinator or not
	 	 	 	 	 	 	 	 	 	 	 	 	the second bit is indicating if this instance is currently performing scanning or not */
} ral_mac_fltr_confg_st;

/**
 * @struct ral_ack_rspnd_fltr_confg_st
 * @brief filter before sending ack configuration
 *
 * this structure contains the parameters used to filter the received packet.
 * and respond to this packet with ack or not
 *  */
typedef struct _ral_ack_rspnd_fltr_confg_st {
#if SUPPORT_A_MAC
	uint8_t * ptr_comp_value;				/* Pointer to value to be compared with received data */
	uint8_t byte_index;						/* Position of the byte start to be compared in the received data */
	uint8_t byte_len;						/* Number of bytes to be compared in the received data */
#endif
	ral_state_enum_t ack_fltr_state;		/* Enable/Disable packet filtration before sending ack */
} ral_ack_rspnd_fltr_confg_st;
/**
 * @struct ral_ack_req_confg_st
 * @brief ack request bit configuration
 *
 * this structure contains information of ack request bit configuration
 *  */
typedef struct _ral_ack_req_confg_st {
	ral_state_enum_t ack_req_bit_state;	/* Enable/Disable Ack request bit check */
	uint8_t byte_index;					/* Byte index of Ack request bit in tx/rx packet */
	uint8_t bit_index;					/* Bit index of Ack request bit in tx/rx packet */
} ral_ack_req_confg_st;

/**
 * @struct ral_auto_ack_confg_st
 * @brief RAL ack data
 *
 * this structure contains the parameters of the acknowledgment configuration.
 * Configure of the received ack (in case of transmission) or configure the transmitted ack (in case of data reception)
 *  */
typedef struct _ral_auto_ack_confg_st {
	ral_ack_rspnd_fltr_confg_st rspnd_fltr_confg;		/* Contains configured filters applied to received packet to determine whether to send ack or not */
	ral_ack_req_confg_st ack_req_confg;					/* Contains configured ack request bit configuration*/
	uint16_t auto_tx_ack_turnaround;					/*time in micro second between rcvd packet and tx ack */
	uint16_t auto_rx_ack_turnaround;					/*time in micro second between tx packet and rcvd ack */
	uint16_t auto_rx_ack_timeout;						/*timeout in microseconds to wait for rcvd ack*/
	uint16_t auto_rx_enh_ack_timeout;					/*timeout in microseconds to wait for rcvd enhanced ack*/
	ral_ack_type_enum_t ack_type;						/*ACK type*/
	ral_state_enum_t auto_tx_ack_state;					/*Enable/Disable automatic transmitted ack*/
	ral_state_enum_t auto_rx_ack_state;					/*Enable/Disable automatic rcvd ack*/
} ral_auto_ack_confg_st;

#if SUPPORT_A_MAC
typedef struct _ral_a_mac_params_st {
	ral_auto_ack_confg_st auto_ack_config;
	uint16_t ifs;
	ral_phy_rate_enum_t phy_rate;
}ral_a_mac_params_st;
#endif /*SUPPORT_A_MAC*/
/**
 * @struct ral_coex_info_st
 * @brief RAL event infomration in case of coexistence
 *  */
typedef struct _ral_coex_info_st {
	void * evnt_hndl;				/* pointer to RAL event handle given from event scheduler after registration */
	ble_time_t grant_end_time;		/* end time in sleep timer steps of grant given from event scheduler */
} ral_coex_info_st;

#if SUPPORT_ENH_ACK_LINK_METRICS_PROBING_OT_1_2 || CONFIG_MAC_CSL_RECEIVER_ENABLE
/**
 * @struct mac_address_st
 * @brief structure carrying information about address and address mode of a device
 *  */
typedef struct mac_address_{
	uint8_t * ptr_address;
	mac_addrs_mode_enum_t address_mode;
} mac_address_st;

#if SUPPORT_ENH_ACK_LINK_METRICS_PROBING_OT_1_2
/**
 * @struct link_metrics_info_st
 * @brief RAL Copy of radio otLinkMetrics structure
 *  */
typedef struct link_metrics_info_
{
    uint8_t mPduCount 	: 1;   ///< Pdu count.
    uint8_t mLqi 	  	: 1;   ///< Link Quality Indicator.
    uint8_t mLinkMargin : 1;   ///< Link Margin.
    uint8_t mRssi 		: 1;   ///< Received Signal Strength Indicator.
    uint8_t mReserved 	: 1;   ///< Reserved, this is for reference device.
} link_metrics_info_st;

typedef struct  link_metric_data_info_st_* p_link_metric_data_info_st;
/**
 * @struct link_metric_data_info_st
 * @brief Link metrics initiator node
 *  */
typedef struct link_metric_data_info_st_{

	p_link_metric_data_info_st ptr_nxt_node;
	// Initiator Info
	struct{
		uint8_t extended_address[EXT_ADDRESS_LENGTH];
		uint16_t short_address;
	} initiator_address_field_st;
	link_metrics_info_st initiator_link_metrics;

} link_metric_data_info_st;
#endif // SUPPORT_ENH_ACK_LINK_METRICS_PROBING_OT_1_2
#endif // SUPPORT_ENH_ACK_LINK_METRICS_PROBING_OT_1_2 || CONFIG_MAC_CSL_RECEIVER_ENABLE

/* Callback API */
/**
 * @struct ral_cbk_dispatch_tbl_st
 * @brief call back functions
 *
 * This structure contains pointer to call back functions which should be called in ISR after Tx/Rx/ED is done
 *  */
typedef struct _ral_cbk_dispatch_tbl_st {
	void (*ral_tx_done)(ral_instance_t ral_instance, ral_pkt_st * ptr_tx_pkt, ral_pkt_st * ptr_ack_pkt, ral_error_enum_t tx_error);
	void (*ral_rx_done)(ral_instance_t ral_instance, ral_pkt_st * ptr_rx_pkt, ral_error_enum_t rx_error);
	void (*ral_ed_scan_done)(ral_instance_t ral_instance, uint32_t scan_durn, int8_t max_rssi);
#if (!SUPPORT_COEXISTENCE)
	void (*ral_resume_rx_after_tmr_err)(void);
#endif /*end of (!SUPPORT_COEXISTENCE)*/
#if SUPPORT_A_MAC
	/*This callback will be called when device receives packet that require custom ack*/
	void (*ral_configure_custom_ack)(ral_instance_t ral_instance,  uint8_t *ptr_ack_pkt, uint16_t*ack_len,uint8_t * ptr_rx_pkt);
#endif /*end of SUPPORT_A_MAC*/
} ral_cbk_dispatch_tbl_st;
/* Definition for hardware control flags */
//--------------------------------------------
/*
 * Tx PHY Rate:
 * 0:125k
 * 1:250k
 * 2:1M
 * 3:2M
 *
 * */
#define DEFAULT_PHY_RATE					1
/* :
 * */
#define DEFAULT_PHY_TX_LOWLTNCY				0
/* :
 * */
#define DEFAULT_PHY_RX_LOWLTNCY				0
/*
 * Bypass CRC check [used for testing purpose]
 * 1'b1 : Bypassing CRC check on the received packet.
 * 1'b0 : perform CRC check on the received packet.
 * */
#define DEFAULT_TXPP_BYPASS_CRC				0
/* :
 * */
#define DEFAULT_SCAN_BCN					0
/*
 * Packet processor configuration
 * 1'b1 : RX PP will discard packet when mac_prmsicsmode is FALSE and generate error flags
 * 1'b0 : RX PP will not discard packet when mac_prmiscsmode is FALSE and will only generate error flags
 * */
#define DEFAULT_DROP_ON_ERR					1
/*
* PAN Coordinator identifier (used in third-level filtering when only source PAN ID is present in the received packet):
* 1'b1 - The device is PAN coordinator
* 1'b0 - The device is not PAN coordinator
 *
 * */
#define DEFAULT_PANCOORD					0
/*
* Indicates whether frames without a destination PAN ID and a destination address are to treated
* as though they are addressed to the broadcast PAN ID and broadcast short address.
 *
 * */
#define DEFAULT_MACIMPLICITBROADCAST		0
/* :
 * */
#define DEFAULT_MACGRPRXMODE				0
/*
 * MAC Promiscuous Mode - Indication of whether the MAC sublayer is in a promiscuous (receive all) mode.
 * A value of TRUE indicates that the MAC sublayer accepts all frames received from the PHY.
 * Shall be used for reception second-level filtering requirements
 * */
#define DEFAULT_MAC_PRMISCMOD				0
//--------------------------------------------
/*@enum
 *@breif: enumerator that defines bit shift amount for control flags used to configure HW
 * */
enum control_flags_shift {
	PHY_RATE_SHIFT,
	PHY_TX_LOWLTNCY_SHIFT=2,
	PHY_RX_LOWLTNCY_SHIFT,
	TXPP_BYPASS_CRC_SHIFT,
	SCAN_BCN_SHIFT,
	DROP_ON_ERR_SHIFT,
	PANCOORD_SHIFT,
	MACIMPLICITBROADCAST_SHIFT,
	MACGRPRXMODE_SHIFT,
	MAC_PRMISCMOD_SHIFT,
	VDDH_PA_SHIFT,
	EPA_DISABLE_SHIFT = 15
};
/*@enum
 *@breif: enumerator that defines bit shift amount for errors returned after event completion
 * */
enum error_flags_shift{
	TIMEOUT_FLAG_SHIFT = 2,  /* bit location for timeout flag reception error */
	ADDMODE_ERR_SHIFT,		 /* bit location for Address Mode reception error. asserted when either of the following is asserted (dstaddr_err,srcpanid_err,dstpanid_err)*/
	RX_ERR_SHIFT,			 /* bit location for rx operation error. asserted when either rx_err or ppdu_err is asserted */
	PPDU_ERR_SHIFT,			 /* bit location for PPDU reception error. asserted when any of the following is asserted
								(ack_err, crc_err,dstaddr_err,srcpanid_err,dstpanid_err,frmvrsn_err,frmtype_err,frmlngth_err)*/
	FRMLNGTH_ERR_SHIFT,		 /* bit location for Frame Length reception error. Asserted when received frame length is one of the reserved values mac_prmiscmode=0 and
	 	 	 	 	 	 	 	decoded MAC header indicates MAC frame longer than received frame length*/
	FRMTYPE_ERR_SHIFT,		 /* bit location for Frame type reception error asserted when received frame type is one of the reserved values.
								Available only when mac_prmiscmode=0 */
	FRMVRSN_ERR_SHIFT,		 /* bit location for Frame version reception error asserted when received frame version is neither 2'b00 not 2'b01.
								Available only when mac_prmiscmode=0 */
	DSTPANID_ERR_SHIFT,		 /* bit location for Destination PAN ID reception error. Available only when mac_prmiscmode=0 */
	SRCPANID_ERR_SHIFT,		 /* bit location for Source PAN ID reception error. Available only when mac_prmiscmode=0 */
	ACK_OK_SHIFT,			 /* bit location for Acknowledgment received correctly flag */
	ACK_ERR_SHIFT,			 /* bit location for Acknowledgment received with errors flag */
	CRC_ERR_SHIFT,			 /* bit location for CRC error indicator flag*/
	DSTADDR_ERR_SHIFT,		 /* bit location for Destination Address reception error. Available only when mac_prmiscmode=0 */
	SEC_ERR_SHIFT = 30		 /* bit location for security processing error occurred in case of secured Enhanced Ack  */
};


/* Start of frame delimiter length as defined in OQPSK phy in MAC 802.15.4 std*/
#define DEFAULT_MAC_SFD_LENGTH 			1
/* Preample length as defined in OQPSK phy in MAC 802.15.4 std*/
#define DEFAULT_MAC_PREAMBLE_LENGTH 	4
/* Start of frame delimiter value as defined in OQPSK phy in MAC 802.15.4 std*/
#define DEFAULT_MAC_SFD_VALUE			0xA7
/* Preample value as defined in OQPSK phy in MAC 802.15.4 std*/
#define DEFAULT_MAC_PEAMBLE_VALUE		0x0

#if(SUPPORT_A_MAC)
/* 1M preamble and SFD used only in A_MAC */
/* Custom start of frame delimiter length used in 1M phy_rate*/
#define DEFAULT_A_MAC_SFD_LENGTH_1M 		4
/* Custom preample length used in 1M phy rate*/
#define DEFAULT_A_MAC_PREAMBLE_LENGTH_1M 	1
/* Custom start of frame delimiter value used in 1M phy_rate*/
#define DEFAULT_A_MAC_SFD_VALUE_1M			0x71764129
/* Custom preample value used in 1M phy rate*/
#define DEFAULT_A_MAC_PEAMBLE_VALUE_1M		0xAA

/* 2M preamble and SFD used only in A_MAC */
/* Custom start of frame delimiter length used in 2M phy_rate*/
#define DEFAULT_A_MAC_SFD_LENGTH_2M 		4
/* Custom preample length used in 2M phy rate*/
#define DEFAULT_A_MAC_PREAMBLE_LENGTH_2M 	2
/* Custom start of frame delimiter value used in 2M phy_rate*/
#define DEFAULT_A_MAC_SFD_VALUE_2M			0x71764129
/* Custom preample value used in 2M phy rate*/
#define DEFAULT_A_MAC_PEAMBLE_VALUE_2M		0xAAAA
#endif

/* Bitfield sizes defined for MAC descriptor */
#define MAC_SFD_VALUE_SIZE 				32
#define MAC_PREAMBLE_VAL_SIZE			32
#define	MAC_PANID_SIZE					16
#define MAC_SHORTADDR_SIZE				16
#define EUI64ADD_LSW_SIZE				32
#define EUI64ADD_MSW_SIZE				32
#define MAC_EXTADDR_LSW_SIZE			32
#define MAC_EXTADDR_MSW_SIZE			32
#define ERROR_FLAGS_SIZE				15
#define RX_FRAME_LEN_SIZE				7
#define DEBUG_PORTS_SIZE				5
#define POINTER_TO_CURRENT_TX_SIZE		16
#define POINTER_TO_CURRENT_RX_SIZE		16
#define FRMLNGTH_SIZE					7
#define MAC_SFD_LEN_SIZE				3
#define MAC_PREAMBLE_LEN_SIZE			3
#define SEQNUM_SIZE						8
#define TX_MAC_LATENCY_SIZE				6
#define CONTROL_FLAGS_SIZE				16
#define PHY_DRV_SEQ_STRT_ADDR_SIZE		7
#define PHY_DRV_SEQ_END_ADDR_SIZE		7
#define PHY_DRV_RSSI_VALUE_SIZE		    16
#define PHY_DRV_LQI_VALUE_SIZE		    8
/*
 * @struct
 * @brief: contains Hardware descriptor fields that:
 * 		   1- required to be filled before start of event.
 * 		   2- need to be checked after the end of event.
 * */
typedef struct llhwc_mac_evnt_info_mem_st {
	uint32_t mac_sfd_value			:MAC_SFD_VALUE_SIZE; 		/* Start of frame delimiter value used by serializer and deserializer*/
	uint32_t mac_preamble_val		:MAC_PREAMBLE_VAL_SIZE;		/* Preamble value used by serializer */
	uint32_t mac_panid				:MAC_PANID_SIZE;			/* The identifier of the PAN on which the device is operating.used for third-level filtering  */
	uint32_t mac_shortaddr			:MAC_SHORTADDR_SIZE;		/* The address that the device uses to communicate in the PAN.used for fourth-level filtering */
	uint32_t eui64add_LSW			:EUI64ADD_LSW_SIZE;			/* Least significant word for EUI Extended address defined in 2015 */
	uint32_t eui64add_MSW			:EUI64ADD_MSW_SIZE;			/* Most significant word for EUI Extended address defined in 2015 */
	uint32_t mac_extaddr_LSW		:MAC_EXTADDR_LSW_SIZE;		/* Least significant word for device Extended address */
	uint32_t mac_extaddr_MSW		:MAC_EXTADDR_MSW_SIZE;		/* Most significant word for device Extended address */
	uint32_t error_flags			:ERROR_FLAGS_SIZE;			/* Error flags set by hardware to indicate filteration errors*/
	uint32_t 						:1;
	uint32_t rx_frm_len				:RX_FRAME_LEN_SIZE;			/* length of the received frame */
	uint32_t debug_ports			:DEBUG_PORTS_SIZE;			/* debugging ports defined for ST */
	uint32_t 						:4;
	uint32_t rssi_out				:PHY_DRV_RSSI_VALUE_SIZE;
	uint32_t LQI				    :PHY_DRV_LQI_VALUE_SIZE;
	uint32_t   				        :8;
	uint32_t frmlngth				:FRMLNGTH_SIZE;				/* length of the transmitted MAC frame */
	uint32_t						:1;
	uint32_t mac_sfd_len			:MAC_SFD_LEN_SIZE;			/* SFD length: 1 octet */
	uint32_t mac_preamble_len		:MAC_PREAMBLE_LEN_SIZE;		/* Preamble length: 4 octets  */
	uint32_t						:2;
	uint32_t seqnum					:SEQNUM_SIZE;				/* Sequence number compared against sequence number extracted from mac header of a received ACK frame*/
	uint32_t tx_latency				:TX_MAC_LATENCY_SIZE;		/* This field sets the required time for the TX path to flush the last bit on the air */
	uint32_t						:2;
	uint32_t control_flags			:CONTROL_FLAGS_SIZE;		/* Flags used to Enable/Disable features in HW */
	uint32_t phy_drv_seq_strt_addr	:PHY_DRV_SEQ_STRT_ADDR_SIZE;/* pointer to start address for sequencer ram */
	uint32_t						:1;
	uint32_t phy_drv_seq_end_addr	:PHY_DRV_SEQ_END_ADDR_SIZE;/* pointer to end address for sequencer ram */
	uint32_t						:1;
	uint32_t Pointer_To_current_TX	:POINTER_TO_CURRENT_TX_SIZE; /* pointer to the packet in shared memory to be transmitted */
	uint32_t Pointer_To_current_RX	:POINTER_TO_CURRENT_RX_SIZE; /* pointer to shared memory place at which the packet will be received  */

}llhwc_mac_evnt_info_mem_t;
#if SUPPORT_RADIO_SECURITY_OT_1_2
typedef struct sec_update_desc_st_{
	uint8_t * ptr_key;
	uint32_t frm_cntr;
	uint32_t hdr_len;
	uint8_t  mic_len;
	uint8_t sec_lvl;
} sec_update_desc_st;
#endif
/*
 * brief: pointer to mac descriptor
 * */
extern llhwc_mac_evnt_info_mem_t* g_mac_event_info;

/**  @ingroup ral_intf_cmn
*  @{
*/
/* Generic APIs  ----------------------------------------------------------------------------------- */
/**
 *
 *
 * @brief	radio abstraction layer initialize
 *
 * @param   ptr_cbk_dispatch_tbl : [in] pointer to callbacks dispatch table
 *
 * @retval ral instance associated to this context that should be used with any ral interface
 */
ral_instance_t ral_init(ral_cbk_dispatch_tbl_st * ptr_cbk_dispatch_tbl);

/**
 *
 *
 * @brief	ral power switch either sleep or active
 *
 * @param   ral_instance  : [in] ral instance
 * @param   power_state   : [in] new power state
 * @param   ptr_coex_info : [in] pointer to current coexistence parameters
 *
 * @retval RAL_ERROR_NONE if power state changed successfully
 */
ral_error_enum_t ral_power_switch(ral_instance_t ral_instance,
		ral_power_state_enum_t power_state, ral_coex_info_st * ptr_coex_info);

/**
 *
 *
 * @brief	get current power state
 *
 * @param   ral_instance : [in] ral instance
 *
 * @retval current power state RAL_POWER_SLEEP or RAL_POWER_ACTIVE
 */
ral_power_state_enum_t ral_get_power_state(ral_instance_t ral_instance);

/**
 *
 *
 * @brief	get current event state and current channel if not idle
 *
 * @param   curr_ral_instance : [out] current ral instance if not idle
 * @param   curr_event_channel : [out] current event channel if not idle
 *
 * @retval current event state RX, TX, ED, IDLE
 */
ral_event_state_enum_t ral_get_current_event_state(ral_instance_t * curr_ral_instance, uint8_t * curr_event_channel);

#if SUPPORT_ANT_DIV
/**
 * @fn ral_set_ant_div_params
 *
 * @brief	set antenna diversity feature parameters
 *
 * @param   ral_instance       : [in] used ral instance
 * @param   ptr_ant_div_params : [in] pointer to antenna diversity parameters structure
 *
 * @retval RAL_ERROR_NONE if antenna diversity parameters are set correctly
 */
ral_error_enum_t ral_set_ant_div_params(ral_instance_t ral_instance, antenna_diversity_st* ptr_ant_div_params);

/**
 * @fn ral_get_ant_div_params
 *
 * @brief	get antenna diversity feature parameters
 *
 * @param   ral_instance       : [in] used ral instance
 * @param   ptr_ant_div_params : [out] pointer to antenna diversity parameters structure
 *
 * @retval None
 */
void ral_get_ant_div_params(ral_instance_t ral_instance, antenna_diversity_st* ptr_ant_div_params);

/**
 * @fn ral_set_ant_div_enable
 *
 * @brief	enable/disable antenna diversity
 *
 * @param   ral_instance   : [in] used ral instance
 * @param   enable         : [in] enable:1 / disable:0
 *
 * @retval RAL_ERROR_NONE if antenna diversity is enabled/disabled correctly
 */
ral_error_enum_t ral_set_ant_div_enable(ral_instance_t ral_instance, uint8_t enable);

/**
 * @fn ral_set_default_ant_id
 *
 * @brief	set the default antenna id to be used for transmission and reception
 *
 * @param   ral_instance    : [in] used ral instance
 * @param   default_ant_id  : [in] the antenna id to be used as default
 *
 * @retval RAL_ERROR_NONE if default antenna ID is set correctly
 */
ral_error_enum_t ral_set_default_ant_id(ral_instance_t ral_instance, uint8_t default_ant_id);

/**
 * @fn ral_set_ant_div_rssi_threshold
 *
 * @brief	set antenna diversity rssi threshold
 *
 * @param   ral_instance     : [in] used ral instance
 * @param   rssi_threshold   : [in] rssi threshold to compare with during antenna diversity measurements
 *
 * @retval RAL_ERROR_NONE if antenna diversity RSSI threshold is set correctly
 */
ral_error_enum_t ral_set_ant_div_rssi_threshold(ral_instance_t ral_instance, int8_t rssi_threshold);
#endif /* SUPPORT_ANT_DIV */

/**
 * @}
 */
#if((!SUPPORT_COEXISTENCE && DEFAULT_PHY_CALIBRATION_PERIOD))
/**
 *
 *
 * @brief	Execute PHY periodic calibration
 *
 *
 * @retval NONE
 */
void ral_exec_phy_prdc_clbr(void);
#endif
/**  @ingroup ral_intf_cmn
*  @{
*/
#if SUPPORT_MAC
#if SUPPORT_A_MAC
/**
 *
 *
 * @brief	set phy rate for transmission/reception
 *
 * @param   ral_instance : [in] used ral instance
 * @param   phy_rate : [in] new phy rate [1M/2M/256K/125K]
 *
 * @retval RAL_ERROR_NONE if phy rate changed successfully
 */
ral_error_enum_t ral_set_rate(ral_instance_t ral_instance, ral_phy_rate_enum_t phy_rate);
/**
 *
 * @brief	set minimum interframe spacing between successive transmission/reception
 *
 * @param   ral_instance : [in] used ral instance
 * @param   min_ifs : [in] new minimum interframe spacing in microsecond
 *
 * @retval None.
 */
void ral_set_min_ifs(ral_instance_t ral_instance, uint16_t min_ifs);
#endif


/**
 *
 *
 * @brief	set interframe spacing between successive transmission/reception
 *
 * @param   ral_instance : [in] used ral instance
 * @param   ifs : [in] new interframe spacing in microsecond
 *
 * @retval RAL_ERROR_NONE if interframe spacing changed successfully
 */
#if SUPPORT_A_MAC
ral_error_enum_t ral_set_ifs(ral_instance_t ral_instance, uint16_t ifs);
#else
#if SUPPORT_ZIGBEE_PHY_CERTIFICATION
ral_error_enum_t ral_set_ifs(ral_instance_t ral_instance, uint16_t ifs);
#endif /* SUPPORT_ZIGBEE_PHY_CERTIFICATION */
#endif
#endif
/**
 * @}
 */
/**  @ingroup ral_intf_tx
*  @{
*/
/* Transmission APIs ----------------------------------------------------------------------------------- */
/**
 *
 *
 * @brief	start packet transmission
 * 			This function responsible for preparation for transmission of a packet by allocating and preparing a new ral event/ral pkt to be executed by HW.
 * 			After completion of the Transmission event or if stop operation ral_tx_done() will be called carrying the status of event.
 *
 * @param   ral_instance 	  : [in] ral instance
 * @param   pkt_src 	 	  : [in] transmission packet source, FIFO based or Packet based
 * @param   ptr_pkt 	 	  : [in] pointer to transmitted packet if packet source is Packet based only
 * @param   ptr_start_time 	  : [in] pointer to start time structure which contains start time of transmission
 * 									 if NULL function will use the current time get from llhwc_slptmr_get
 * @param   periodic_interval : [in] periodic interval in microsecond, 0 means not periodic
 * @param   ptr_coex_info 	  : [in] pointer to current coexistence parameters
 *
 * @retval RAL_ERROR_NONE if transmission started successfully
 * 		   RAL_ERROR_INVALID_PARAMETERS if the passed parameters doesn't make sense e.g. starting fifo mode but ptr_fifo_head = NULL
 * 		   RAL_ERROR_BUSY if there is a transmission event that already started and not ended yet
 *
 * @note: ral_tx_done won't be called unless emngr_handle_all_events() is called to call ral_sm_done which will call the ral_tx_done call back
 *
 */
ral_error_enum_t ral_start_tx(ral_instance_t ral_instance, ral_pkt_src_enum_t pkt_src, ral_pkt_st * ptr_pkt,
		ral_time_st * ptr_start_time, uint32_t periodic_interval, ral_coex_info_st * ptr_coex_info);

/**
 *
 *
 * @brief	abort current transmission
 * 			This function is used for testing purpose
 *
 * @param   ral_instance : [in] ral instance
 *
 * @retval RAL_ERROR_NONE if transmission aborted successfully
 */
ral_error_enum_t ral_abort_tx(ral_instance_t ral_instance);

#if SUPPORT_A_MAC
/**
 *
 *
 * @brief	add packet to transmission FIFO
 *
 * @param   ral_instance : [in] ral instance
 * @param   ptr_pkt 	 : [in] pointer to input packet allocated by user application
 *
 * @retval RAL_ERROR_NONE if packet added to FIFO successfully
 */
ral_error_enum_t ral_add_tx_fifo(ral_instance_t ral_instance, ral_pkt_st * ptr_pkt);

/**
 * @brief	flush current transmission FIFO
 *
 * @param   ral_instance : [in] ral instance
 *
 * @retval RAL_ERROR_NONE if FIFO flushed successfully
 */
ral_error_enum_t ral_flush_fifo(ral_instance_t ral_instance);
#endif

/**
 * @brief	get transmission packet to prepare data in before starting transmission
 * 			must be used in case of packet transmission mode, The packet is allocated from TX/RX shared memory.
 *
 * @param   ral_instance : [in] ral instance
 *
 * @retval pointer to allocated ral packet
 */
ral_pkt_st * ral_get_tx_buf(ral_instance_t ral_instance);
/**
 * @}
 */
/**  @ingroup ral_intf_ed
*  @{
*/
/**
 * @brief	perform Clear Channel Assessment on selected channel
 *
 * @param   channel 		: [in] channel to perform CCA on.
 * @param   ptr_coex_info 	: [in] pointer to current coexistence parameters
 * @param   energyThreshold : [in] the minimum value of RSSI to mark the channel busy ,
 *          it used only in case of phy 2.00a_tc. defined by cca_change_threshold_seq for other phys.
 * @param   ral_instance	: [in] ral instance
 * @retval RAL_ERROR_NONE if no traffic on air, RAL_ERROR_CCA_FAILURE otherwise
 */
ral_error_enum_t ral_perform_cca(uint8_t channel, ral_coex_info_st * ptr_coex_info, int8_t energyThreshold
	, ral_instance_t ral_instance
);
/**
 * @}
 */
/**  @ingroup ral_intf_rx
*  @{
*/
/* Reception APIs  ----------------------------------------------------------------------------------- */
/**
 * @brief	start packet reception
 * 			This function responsible for preparation for reception of a packet by allocating and preparing a new ral event/ral pkt to be executed by HW.
 * 			After completion of the Reception event or if stop operation ral_rx_done() will be called carrying the status of event and the packet received.
 *
 *
 * @param   ral_instance 		: [in] ral instance
 * @param   rx_channel 			: [in] channel to receive on
 * @param   ptr_start_time 		: [in] pointer to start time structure which contains start time of reception
 * @param   timeout 			: [in] timeout of reception in microsecond from the start of reception
 * @param   periodic_interval 	: [in] periodic interval in microsecond, 0 means not periodic
 * @param   ptr_coex_info 		: [in] pointer to current coexistence parameters
 *
 * @retval RAL_ERROR_NONE if reception started successfully
 *  	   RAL_ERROR_INVALID_PARAMETERS if the passed parameters doesn't make sense e.g. if the reception channel out of MAC band
 * 		   RAL_ERROR_BUSY if there is a transmission event that already started and not ended yet
 *
 * @note: ral_rx_done won't be called unless emngr_handle_all_events() is called to call ral_sm_done which will call the ral_rx_done call back
 */
ral_error_enum_t ral_start_rx(ral_instance_t ral_instance, uint8_t rx_channel, ral_time_st * ptr_start_time,
								uint32_t timeout, uint32_t periodic_interval, ral_coex_info_st * ptr_coex_info);

/**
 * @brief	abort current reception
 * 			This function is used for tseting purpose
 *
 * @param   ral_instance : [in] ral instance
 *
 * @retval RAL_ERROR_NONE if reception aborted successfully
 */
ral_error_enum_t ral_abort_rx(ral_instance_t ral_instance);
/**
 * @}
 */
/**  @ingroup ral_intf_cmn
*  @{
*/
/**
 * @brief	set automatic continuous reception after each event state
 *
 * @param   ral_instance 	: [in] ral instance
 * @param   cont_recp_state : [in] continuous reception state, Enable or Disable
 * 							       Enable: means that when no transmission device will always receive
 *
 * @retval RAL_ERROR_NONE if new state saved successfully
 */
ral_error_enum_t ral_set_cont_recp_state(ral_instance_t ral_instance, ral_state_enum_t cont_recp_state);

/**
 * @brief	This function used to Enable/Disable automatic switching to sleep mode after finish each event.
 * 			set automatic sleep after each event state in case of continuous reception is disabled as if enabled
 * 			ral shall schedule reception event from ral_isr() using active timer
 *
 * @param   ral_instance 	 : [in] ral instance
 * @param   auto_sleep_state : [in] automatic sleep state, Enable or Disable
 *
 * @retval RAL_ERROR_NONE if new state saved successfully
 */
ral_error_enum_t ral_set_auto_sleep_state(ral_instance_t ral_instance, ral_state_enum_t auto_sleep_state);
/**
 * @brief	This function used to get the state of automatic switching to sleep mode
 *
 * @param   ral_instance 	 : [in] ral instance
 *
 * @retval automatic sleep state, Enable or Disable
 */
ral_state_enum_t ral_get_auto_sleep_state(ral_instance_t ral_instance);
/**
 * @brief	configure mac filter in rtl while reception.
 * 			when filter is disabled means reception in promiscuous mode.
 *
 * @param   ral_instance 	   : [in] ral instance
 * @param   ptr_mac_fltr_confg : [in] pointer to mac filter configuration
 *
 * @retval RAL_ERROR_NONE if new configuration saved successfully
 */
ral_error_enum_t ral_confg_mac_fltr(ral_instance_t ral_instance, ral_mac_fltr_confg_st * ptr_mac_fltr_confg);
/**
 * @brief	configure mac filter in scan mode
 *
 *
 * @param   ral_instance : [in] ral instance
 * @param   Enable 	     : [in] set or clear scan mode
 *
 * @retval RAL_ERROR_NONE if new configuration saved successfully
 */
ral_error_enum_t ral_set_scan_filter(ral_instance_t ral_instance, uint8_t Enable);

/* Auto Acknowledge APIs ----------------------------------------------------------------------------------- */
/**
 * @brief	configure automatic ack after packet transmission/reception
 *
 * @param   ral_instance 	   : [in] ral instance
 * @param   ptr_auto_ack_confg : [in] pointer to new automatic ack configuration
 *
 * @retval RAL_ERROR_NONE if new configuration saved successfully
 */
ral_error_enum_t ral_confg_auto_ack(ral_instance_t ral_instance, ral_auto_ack_confg_st * ptr_auto_ack_confg);

/**
 * @brief	temporary pause/resume automatic ack state only after transmission/reception
 *			this function is used for testing and custom purposes.
 *
 * @param   ral_instance 	  : [in] ral instance
 * @param   auto_tx_ack_state : [in] new state of automatic ack transmission
 * @param   auto_rx_ack_state : [in] new state of automatic ack reception
 *
 * @retval RAL_ERROR_NONE if new configuration saved successfully
 */
ral_error_enum_t ral_pause_auto_ack(ral_instance_t ral_instance, ral_state_enum_t auto_tx_ack_state, ral_state_enum_t auto_rx_ack_state);
/**
 * @}
 */

/**
 * @brief	set enhanced ack Header IE data
 *
 * @param   ral_instance : [in] ral instance
 * @param   ptr_hdr_ie 	 : [in] pointer to header IE data to be included in enhanced ack
 * @param   hdr_ie_len 	 : [in] header IE data length
 *
 * @retval RAL_ERROR_NONE if new configuration saved successfully
 */
ral_error_enum_t ral_set_enh_ack_hdr_ie(ral_instance_t ral_instance, uint8_t * ptr_hdr_ie, uint8_t hdr_ie_len);


/**  @ingroup ral_intf_cmn
*  @{
*/
/**
 * @brief	Enable/Disable source address match feature.
 *			If disabled, the ral must set the "frame pending" on all acks to data request commands.
 * 			If enabled, the ral uses the source address match table to determine whether to set or clear the
 * 			"frame pending" bit in an ack to a data request command.
 *
 * @param   ral_instance 	: [in] ral instance
 * @param   src_match_state : [in] source address match state
 *
 * @retval void
 */
void ral_set_src_match_state(ral_instance_t ral_instance, ral_state_enum_t src_match_state);

/**
 * @brief	Add a short address to the source address match table
 *
 * @param   ral_instance  : [in] ral instance
 * @param   short_address : [in] short address to be added
 *
 * @retval RAL_ERROR_NONE if added successfully
 */
ral_error_enum_t ral_add_src_match_short(ral_instance_t ral_instance, const uint16_t short_address);

/**
 * @brief	Add an extended address to the source address match table
 *
 * @param   ral_instance : [in] ral instance
 * @param   ptr_ext_addr : [in] pointer to extended address to be added
 *
 * @retval RAL_ERROR_NONE if added successfully
 */
ral_error_enum_t ral_add_src_match_ext(ral_instance_t ral_instance, const uint8_t * ptr_ext_addr);

/**
 * @brief	Remove a short address from the source address match table
 *
 * @param   ral_instance  : [in] ral instance
 * @param   short_address : [in] short address to be removed
 *
 * @retval RAL_ERROR_NONE if removed successfully
 */
ral_error_enum_t ral_clr_src_match_short(ral_instance_t ral_instance, const uint16_t short_address);

/**
 * @brief	Remove an extended address from the source address match table
 *
 * @param   ral_instance : [in] ral instance
 * @param   ptr_ext_addr : [in] pointer to extended address to be removed
 *
 * @retval RAL_ERROR_NONE if removed successfully
 */
ral_error_enum_t ral_clr_src_match_ext(ral_instance_t ral_instance, const uint8_t * ptr_ext_addr);

/**
 * @brief	Remove all short addresses from the source address match table
 *
 * @param   ral_instance : [in] used ral instance
 *
 * @retval void
 */
void ral_clr_all_src_match_short(ral_instance_t ral_instance);

/**
 * @brief	Remove all extended addresses from the source address match table
 *
 * @param   ral_instance : [in] used ral instance
 *
 * @retval void
 */
void ral_clr_all_src_match_ext(ral_instance_t ral_instance);
/**
 * @}
 */
/* Energy Detect API  ----------------------------------------------------------------------------------- */
/**  @ingroup ral_intf_ed
*  @{
*/
/**
 *
 * @brief	This function is used to terminate the ED

 * @retval None
 *
 */
void ral_reset_ed(void);
/**
 * @brief	start energy detection scan on specific channel,
 * 			ral_ed_scan_done callback is called to notify upper layer that the energy scan is complete.
 *
 * @param   ral_instance : [in] ral instance
 * @param   scan_channel : [in] The channel to perform the energy scan on
 * @param   scan_duration : [in] scan duration in microsecond
 * @param   ptr_coex_info : [in] pointer to current coexistence parameters
 *
 * @retval RAL_ERROR_NONE if energy detection scanning started successfully
 *
 * @note: ral_ed_scan_done won't be called unless emngr_handle_all_events() is called to call ral_sm_done_cbk which will call ral_ed_scan_done
 */
ral_error_enum_t ral_ed_scan(ral_instance_t ral_instance, uint8_t scan_channel, uint32_t scan_duration,
		ral_coex_info_st * ptr_coex_info);
/**
 * @}
 */
#if SUPPORT_ZIGBEE_PHY_CERTIFICATION
/**
 * @brief  get the current LQI of the last received packet
 * @param   last_rssi : [in] RSSI of the last received packet
 * @retval uint8_t. LQI value
 */
uint8_t ral_dtmGetLQIValue(int8_t last_rssi);
#endif /*SUPPORT_ZIGBEE_PHY_CERTIFICATION*/

/**
 *
 * @brief	This function is used check whether the ral is about to transmit ack or not
 * it will return true only if the state machine dine interrupt is set and the frame header is parsed and it indicates that the ack is requested
 * @retval 1: Ack is about to be transmitted
 * @retval 0:  No Ack is to be transmitted
 *
 */
uint8_t ral_is_about_to_transmit_ack(void);
/**
 *
 * @brief	This function is used start the triggering of pre tx sequence from sequence ram as early as possible to save time in case of transmitting ack

 * @retval None
 *
 */
void ral_early_perpare_phy_to_tx_ack(void);
/**
 *
 * @brief	This function is the phy driver isr handler in case of mac event ,
 it is used restore the phy state after triggering of  the pre interpact  sequence from sequence ram

 * @retval None
 *
 */
void ral_handle_phy_driver_isr(void);
#if SUPPORT_RADIO_SECURITY_OT_1_2
/**
 * @brief  This function is used to update frame counter sustained in ral instance
 * 		   This function is called only in case of radio support OT_RADIO_CAPS_TRANSMIT_SEC
 * 		   is called from otPlatRadioSetMacFrameCounter()
 *
 * @param  instance    			:	Ral instance
 * @param  mac_frm_cntr			:	frame counter passed by upper layers
 *
 * @retval None
 */
void ral_update_mac_frm_cntr(ral_instance_t instance, uint32_t mac_frm_cntr);
/**
 * @brief  This function is used to update frame counter sustained in ral instance only if the new value larger than the old one
 * 		   This function is called only in case of radio support OT_RADIO_CAPS_TRANSMIT_SEC
 * 		   is called from otPlatRadioSetMacFrameCounterIfLarger()
 *
 * @param  instance    			:	Ral instance
 * @param  mac_frm_cntr			:	frame counter passed by upper layers
 *
 * @retval None
 */
void ral_update_larger_mac_frm_cntr(ral_instance_t instance, uint32_t mac_frm_cntr);
/**
 * @brief  This function is used to update keys and keyId sustained in ral instance
 * 		   This function is called only in case of radio support OT_RADIO_CAPS_TRANSMIT_SEC
 * 		   is called from otPlatRadioSetMacKey()
 *
 * @param  instance     :	Ral instance
 * @param  aKeyId		:	Key index is used for comparison in case of keyId mode '1'
 * 							To select between different keys.
 * 							------------
 * 							for example:
 * 							------------
 * 							if keyId (key index) equivalent to the received in MHR
 * 							Then the used key is the current key.
 *
 * 							But if it isn't equivalent due to any connected neighbor's key index
 * 							mismatch the key generator in upper layers shall be notified to take action
 * 							based on the relation between the different key index
 *
 * 							------------
 * 							for example:
 * 							-----------
 * 							if keyId (key index) less than the received in MHR. this means that the
 * 							communicating device has already generate a new key and started to rotate keys.
 * 							so the next key is the key that shall be used in this situation.
 *
 * 							This kind of process synchronization beside another timing triggered events for
 * 							new key generation and rotation allows a difference of only one between keyIds.
 *
 * 							see thread specification under security section subsection of "Key Rotation"
 *
 * @param  aPrevKey		:	key used in case of keyid of received MHR less than the keyid sustained by ral_instance by one.
 * @param  aCurrKey		:	key used in case of keyid of received MHR equivalent to the keyid sustained by ral_instance.
 * @param  aNextKey		:	key used in case of keyid of received MHR greater than the keyid sustained by ral_instance by one.
 *
 * @retval None
 */
void ral_update_mac_keys(   ral_instance_t    instance,
							uint8_t             aKeyId,
							const uint8_t    *aPrevKey,
							const uint8_t    *aCurrKey,
							const uint8_t    *aNextKey );
/**
 * @brief  This function is used to get the current key of an instance
 * 		   This function is called only in case of radio support OT_RADIO_CAPS_TRANSMIT_SEC
 *
 * @param  instance    :	Ral instance
 *
 * @retval uint8_t*    : 	pointer to the current key
 */
const otMacKeyMaterial  * ral_get_inst_curr_key(ral_instance_t ral_instance);
/**
 * @brief  This function is used to get the previous key of an instance
 * 		   This function is called only in case of radio support OT_RADIO_CAPS_TRANSMIT_SEC
 *
 * @param  instance    :	Ral instance
 *
 * @retval uint8_t*    : 	pointer to the previous key sustained by ral layer
 */
uint8_t * ral_get_inst_prev_key(ral_instance_t ral_instance);
/**
 * @brief  This function is used to get the frame counter of an instance
 * 		   This function is called only in case of radio support OT_RADIO_CAPS_TRANSMIT_SEC
 *
 * @param  instance    :	Ral instance
 *
 * @retval uint32_t    : 	frame counter value sustained by ral layer
 */
uint32_t ral_get_inst_frm_cntr(ral_instance_t ral_instance);
/**
 * @brief  This function is used to get the key index of an instance
 * 		   This function is called only in case of radio support OT_RADIO_CAPS_TRANSMIT_SEC
 *
 * @param  instance    :	Ral instance
 *
 * @retval uint8_t     : 	key index sustained by ral layer
 */
uint8_t ral_get_inst_keyId(ral_instance_t ral_instance);

#endif /*SUPPORT_RADIO_SECURITY_OT_1_2*/
#if SUPPORT_ENH_ACK_LINK_METRICS_PROBING_OT_1_2
/**
 * @brief  This function is used to configure [remove / add / modify] initiator device.
 *
 * @param  short_addr    :[in]	mac_address_st carries address and adress mode for initiator
 * @param  ptr_ext_addr  :[in]	pointer to the matching node
 * @param  link_metrics  :[in]	pointer to the matching node
 *
 * @retval ral_error_enum_t : status
 * 							  RAL_ERROR_NONE						: successfully configured.
 * 							  RAL_ERROR_LINK_METRICS_INVALID_ARGS	: in case of ptr_ext_addr NULL.
 * 							  RAL_ERROR_LINK_METRICS_NOT_FOUND		: in case of remove non-existing node.
 * 							  RAL_ERROR_LINK_METRICS_NO_BUF			: in case of not enough supported nodes.
 */
ral_error_enum_t ral_config_enh_ack_probing(uint16_t short_addr, const uint8_t * ptr_ext_addr, void* link_metrics);
/**
 * This method set the Link Metrics noise floor value needed to calculate the link margine
 *
 * @param[in]  noise_floor    The noise floor used by Link Metrics. It should be set to the platform's
 *                            noise floor (measured noise floor, receiver sensitivity or a constant).
 *
 */
void ral_link_metrics_set_noise_floor(int8_t noise_floor);
#endif /*SUPPORT_ENH_ACK_LINK_METRICS_PROBING_OT_1_2*/
/**
 * @brief  set the value of the openthread base time. this value will be subtracted from all timing values sent / received to openthread
 * @param [in]  : base time value
 * @retval None
 */

void ral_set_ot_base_slp_time_value(uint32_t time);
/**
 * @brief  get the value of the openthread base time, the value nay be set through @ref ral_set_ot_base_slp_time_value
 * @param  None
 * @retval uint64_t. base time value
 */

uint64_t ral_get_ot_base_slp_time_value(void);
/**
 * @brief  Convert the value of sleep timer to openthread time
 * @param  time [in]      : sleep timer value to be converted to openthread time
 * @note if openthread is not integrated, @ref ral_ot_base_slp_time  is set to zero, no conversion will take place
 * @retval uint64_t. the converted time value
 */

uint64_t ral_cnvert_slp_tim_to_ot_tim(uint32_t time);
/**
 * @brief  Convert the value of  openthread time to sleep timer value
 * @param  time [in]      : openthread time value to be converted to sleep timer
 * @note if openthread is not integrated, @ref ral_ot_base_slp_time  is set to zero, no conversion will take place
 * @retval uint32_t. the converted time value
 */

uint32_t ral_cnvert_ot_tim_to_slp_tim(uint64_t time);

#if SUPPORT_OPENTHREAD_1_2
/**
 * @brief  Convert the  microsecond time to  the proper value the should be set for teh sleep timer to start the event
 * @param  curr_time [in]      : current sleep timer value to be converted to openthread time
 * @param  base_time [in]       : base time in microsecond  to be  be converted
 * @param  delay_time [in]      :  microsecond delay  from the base time
 * @retval uint32_t. the converted sleep timer set point to be used in setting active timer
 */
uint32_t ral_cnvrt_req_time_to_set_point(uint32_t curr_time, uint32_t base_tim , uint32_t delay_time);
#if CONFIG_MAC_CSL_RECEIVER_ENABLE
/**
 * @brief	set CSL receiver parameters to enable/ Disable CSL.
 *
 * @param   ral_instance     : [in] ral instance
 * @param   cslPeriod        : [in] CSL period  to be included in CSL header IE or , 0 to disable CSL
 * @param   csl_short_addr   : [in] The short source address of CSL receiver's peer.
 * @param   ptr_csl_ext_addr : [in] pointer to the parent extended address
 *
 * @retval None
 *
 */
void ral_set_csl_rcv_param(ral_instance_t ral_instance, uint32_t cslPeriod ,uint16_t csl_short_addr, uint8_t* ptr_csl_ext_addr);
/**
 * @brief	set CSL receiver next sample time to be used in calculating phase. the sample time points to the time of he next sample window
 *
 * @param   ral_instance     : [in] ral instance
 * @param   cslSampleTime        : [in] the lsb part of sample time in us
 *
 * @retval None
 *
 */
void ral_set_csl_sample_time(ral_instance_t ral_instance, uint32_t cslSampleTime);
/**
 * @brief	check whether the RAL is receiving within the CSL sample window
 *
 * @param   None
 *
 * @retval uint8_t TRUE 1 if the receiving in CSL window
 * @retval uint8_t FALSE 0  if not receiving in CSL windoe
 *
 */
uint8_t ral_is_rcv_in_csl_smple_wndw(void);
/**
 * @brief	check and add CSL header ie to any outgoing frame if csl receiver  is enabled.
 *
 * @param   ral_instance : [in] ral instance
 * @param   frame_ptr    : [in] pointer the frame to be transmitted , either enhanced ack or any other frame.
 * @param   ie_index     : [in,out] index of there first byte of CSL header IE. if the CSL header IE will be included , It will be incremented with the length of csl header ie
 * @param   data_ptr     : [in] data pointer it point  to the parent address in case of enhanced ack , and frame total length for TX frames
 * @param   enh_ack_flag : [in] flag to indicate whether  enhanced ack or new Tx Frame
 * @param   addr_mode    : [in] if enhanced ack then indicate the parent address is short or extendded
 *
 * @retval None
 *
 */
void ral_hndl_csl_hdr_ie(ral_instance_t ral_instance, uint8_t *frame_ptr,uint8_t *ie_index, uint8_t * data_ptr , uint8_t enh_ack_flag, uint8_t addr_mode);
#endif /*CONFIG_MAC_CSL_RECEIVER_ENABLE*/
#endif /* SUPPORT_OPENTHREAD_1_2 */
/**
 * @brief	Set the CCA Energy Detection threshold in phy
 *
 * @param   threshold  : the CCA Energy Detection threshold value to be set
 *
 * @retval OT_ERROR_NONE     if the the threshold is successfully set
 * @retval OT_ERROR_FAILED   if the given value is out of range
 *
 */
otError ral_set_cca_ed_threshold(int8_t threshold);
/**
 * @brief	Get the CCA Energy Detection threshold in phy
 *
 * @param   None.
 *
 * @retval int8_t  CCA Energy Detection threshold value
 *
 */
int8_t ral_get_cca_ed_threshold(void);
#if (SUPPORT_COEXISTENCE)
/**
 * @brief	radio csma error callback.
 *
 * @param   tx_error     : [in] status error returned from this transmission
 *
 * @retval None
 *
 */
void radio_coex_tx_error_cbk(uint32_t error);
#endif /*end of (SUPPORT_COEXISTENCE)*/
#if (RADIO_CSMA) &&(!SUPPORT_COEXISTENCE)
/**
 * @brief	set maximum time to start csma
 *
 * @param   rx_timeout     	: [in] rx timeout time when there's RX while waiting csma backoff delay to fire (value in slp_tmr)
 * @param   max_csma_delay  : [in] max start time to start csma in slp_tmr
 *
 * @retval None
 *
 */
void ral_set_csma_time(
#if ENHANCED_RX_WHILE_CSMA_BACKOFF_DELAY
		ble_time_t rx_timeout,
#endif /*end of ENHANCED_RX_WHILE_CSMA_BACKOFF_DELAY*/
					ble_time_t max_csma_delay);
#endif /*end of (RADIO_CSMA) &&(!SUPPORT_COEXISTENCE)*/
/**
 * @brief	flag indication used to handle frame pending bit in ACK of all packets (set to true) or for ACK of data request command only (set to false)
 *
 * @param   hndle_frm_pending_bit_for_acks     : [in] TRUE means handle pending frame bit in ACK for all frame types, FALSE means handle frame pending bit in ACK for data request command only
 *
 * @retval None
 *
 */
void ral_set_frm_pend_bit_for_acks(uint8_t hndle_frm_pending_bit_for_acks);

#if RADIO_CSMA
/**
 *
 * @brief   set csma enable flag
 *
 * @param   csma_en : [in] value for csma enable flag to be set
 *
 * @retval None .
 */
void radio_set_csma_en(uint8_t csma_en);
/**
 *
 * @brief   get csma enable flag
 *
 * @param   None
 *
 * @retval uint8_t csma enable flag .
 */
uint8_t radio_get_csma_en(void);
#endif /*end of RADIO_CSMA*/
/**
 *
 * @brief   set cca enable flag
 *
 * @param   cca_en 		: [in] value for cca enable flag to be set
 *
 * @retval None .
 */
void radio_set_cca_en(uint8_t cca_en);
/**
 *
 * @brief   get cca enable flag
 *
 * @param   None
 *
 * @retval uint8_t cca enable flag .
 */
uint8_t radio_get_cca_en(void);
/**
 *
 * @brief   set pending TX retry flags
 *
 * @param   evnt_type     	: [in] type of new retry (CONTINUE_CSMA_RETRY , START_NEW_FULL_TX_RETRY)
 * @param   radio_error	    : [in] error returned from previous TX trial
 *
 * @retval None .
 */
void radio_set_tx_retry_pending(tx_new_retry_enum_t evnt_type, otError radio_error);
/**
 *
 * @brief   handle pending tx retry event
 *
 * @param   None
 *
 * @retval 	None .
 */
void radio_handle_pnding_tx_retry_event(void);
/**
 *
 * @brief   check if there's pending TX retry waiting to be executed
 *
 * @param   None
 *
 * @retval uint8_t value of pending_tx_retry_flag .
 */
uint8_t radio_is_tx_retry_event_pending(void);
/**
 *
 * @brief   Set MAC implicit boradcast PIB from MAC layer to be used in filteration
 *
 * @param   ImplicitBroadcast: [in] Value for MAC implicit boradcast PIB to be set
 *
 * @retval 	None .
 */
void radio_set_implicitbroadcast(uint8_t ImplicitBroadcast);
/**
 *
 * @brief   Set MAC implicit boradcast PIB from radio layer to be used in filteration
 *
 * @param   ImplicitBroadcast: [in] Value for MAC implicit boradcast PIB to be set
 *
 * @retval 	None .
 */
void ral_set_implicitbroadcast(ral_instance_t ral_instance, uint8_t ImplicitBroadcast);
/**
 * @fn ed_timer_hndl
 *
 * @brief	energy detection timer event handle
 *
 * @param   ptr_info : [in] pointer to current ral context
 *
 * @retval void
 */
void ed_timer_hndl(void* ptr_info);

#if SUPPORT_MAC_PHY_CONT_TESTING_CMDS
/**
 *
 * @brief set the phy continuous modulation and continuous wave modes
 * 	upon enable, if the selected mode is already enabled and likewise
 * 	in disabling, the change will take no effect
 *
 * @param   ral_instance : [in] ral instance
 *
 * @param	type[in]		: the type of the modulation (0: modulation, 1: wave)
 *
 * @param	enable_mode[in]	: if true then enable the selected mode otherwise disable it
 *
 * @param	chnl_num[in]	: channel number to be used in modulation (range: 0 to 15)
 *
 * @param   tx_pwr[in]         : The used power in dBm.
 *
 * @retval Status
 */
void ral_phy_set_zigbee_phy_cont_test_mode(ral_instance_t instance, uint8_t type, uint8_t enable_mode, uint8_t chnl_num, int8_t tx_pwr);
#endif /*end of SUPPORT_MAC_PHY_CONT_TESTING_CMDS */

#if SUPPORT_A_MAC
/**
 *
 * @fn ral_get_a_mac_params
 *
 * @brief   get parameters used in augmented MAC  (IFS, phy_rate, auto_ACK_config)
 *
 * @param   ral_instance : [in] ral instance
 *
 * @param   a_mac_params : [out] current augmented MAC parameters
 *
 * @retval Status
 */
ral_error_enum_t ral_get_a_mac_params(ral_instance_t ral_instance,ral_a_mac_params_st* a_mac_params);
#endif /*SUPPORT_A_MAC*/

#endif /* INCLUDE_RAL_H_ */
/**
 * @}
 */
