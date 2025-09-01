/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/event_manager.h#1 $*/
/**
 ********************************************************************************
 * @file    event_manager.h
 * @brief   This file contains all the functions prototypes for the event_manager.c.
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
#ifndef INCLUDE_EVENT_MANAGER_H_
#define INCLUDE_EVENT_MANAGER_H_
#include <stdint.h>
#include "ll_fw_config.h"
#include "common_types.h"

/* Defination ----------------------------------------------------------------*/
/*Event manager events*/
#define EVENT_PENDING		1
#define HANDLE_IS_FULL		77
#define NO_EVENT_PENDING	0


#define EVENT_NOT_BUSY		0
#define EVENT_BUSY			1

#if (NUM_OF_CTSM_EMNGR_HNDLS > 3)
#error "NUM_OF_CTSM_EMNGR_HNDLS shouldn't exceed 3 !"
#endif



typedef uint8_t (*conditional_cbk)(void * em_data , void * caller_data);



/**
 * @brief Enum event manager handler types.
 */
	/* Event Manager IDs */
typedef enum {
#if SUPPORT_BLE
	LLHWC_EVENT,
	PRDC_CLBR_EVENT,
#if ((SUPPORT_MASTER_CONNECTION) || (SUPPORT_SLAVE_CONNECTION))
	CONN_EVENT, /*connection event handler ID*/
	CONN_PARAM_UPDATE_EVENT, /* handler for connection parameter update initiated by link layer */
#if ((SUPPORT_CONNECTED_ISOCHRONOUS) && (SUPPORT_MASTER_CONNECTION))
	CONN_DATA_LEN_UPDATE_EVENT, /* handler for connection data length update initiated by link layer */
#endif /*SUPPORT_CONNECTED_ISOCHRONOUS && SUPPORT_MASTER_CONNECTION*/
#if(SUPPORT_CONNECTED_ISOCHRONOUS &&( SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION))
	CIS_EVENT,
#endif /*SUPPORT_CONNECTED_ISOCHRONOUS*/
#endif /* SUPPORT_MASTER_CONNECTION) || (SUPPORT_SLAVE_CONNECTION */
#if(SUPPORT_BRD_ISOCHRONOUS || SUPPORT_SYNC_ISOCHRONOUS)
	BIS_TERM_EVENT,
#if(SUPPORT_SYNC_ISOCHRONOUS)
	BIS_SYNC_TIMEOUT_EVENT,
#endif /* SUPPORT_SYNC_ISOCHRONOUS */
#endif /* SUPPORT_BRD_ISOCHRONOUS || SUPPORT_SYNC_ISOCHRONOUS */
#endif /*SUPPORT_BLE*/
#if SUPPORT_BLE
	HCI_HANDLER, /* handler for the HCI events; handling events from Host to HCI*/
#endif /*SUPPORT_BLE*/
#if SUPPORT_BLE
#if SUPPORT_LE_EXTENDED_ADVERTISING
	ADV_TIMEOUT_EVENT, 	/*handler for advertising extended timeout feature */
	SCN_DURATION_EVENT,	/*handler for extended scanning duration */
	SCN_PERIOD_EVENT, 	/*handler for extended scanning period */
#if SUPPORT_LE_PERIODIC_ADVERTISING
	PRDC_SCAN_TIMEOUT_EVENT, /*handler for periodic scan sync timeout */
	PRDC_SCAN_CANCEL_EVENT,
#endif /* SUPPORT_LE_PERIODIC_ADVERTISING */
#if SUPPORT_LE_PAWR_ADVERTISER_ROLE
	PAWR_SEND_FRST_REQ,
#endif /* SUPPORT_LE_PAWR_ADVERTISER_ROLE */
#endif /* SUPPORT_LE_EXTENDED_ADVERTISING */
#endif /*SUPPORT_BLE*/
#if SUPPORT_COEXISTENCE
	COEX_TIMER_EVENT,
#endif
#if SUPPORT_MAC
	RADIO_MAC_TX_DONE_EVENT,
	RAL_SM_DONE_EVENT,
	MAC_SM_DONE_EVENT,
	ED_TMR_EVENT,
#endif /*SUPPORT_MAC*/
#if ((SUPPORT_BLE)||(SUPPORT_MAC_HCI_UART)||(SUPPORT_ANT_HCI_UART) || (SUPPORT_AUG_MAC_HCI_UART))
	HCI_TRANSPORT_HANDLER, /* handler for the HCI transport events; handling events from HCI to Host */
#endif /*SUPPORT_BLE*/
#if (SUPPORT_HCI_EVENT_ONLY)
	GENERIC_EVENT,
#if SUPPORT_SYNC_ISOCHRONOUS || SUPPORT_CONNECTED_ISOCHRONOUS
	ISO_DATA_EVENT,
#endif /* SUPPORT_SYNC_ISOCHRONOUS || SUPPORT_CONNECTED_ISOCHRONOUS */
#if SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION
	ACL_DATA_EVENT,
#endif /* SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION */
#if SUPPORT_EXPLCT_OBSERVER_ROLE || SUPPORT_MASTER_CONNECTION || SUPPORT_SYNC_ISOCHRONOUS  || (SUPPORT_AOA_AOD && SUPPORT_SLAVE_CONNECTION)
	ADV_REPORT_EVENT,
#endif /* SUPPORT_EXPLCT_OBSERVER_ROLE || SUPPORT_MASTER_CONNECTION || SUPPORT_SYNC_ISOCHRONOUS  || (SUPPORT_AOA_AOD && SUPPORT_SLAVE_CONNECTION) */
#if (SUPPORT_CONNECTED_ISOCHRONOUS && (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION) || (SUPPORT_BRD_ISOCHRONOUS) || (SUPPORT_SYNC_ISOCHRONOUS))
	HCI_SYNC_EVENT,
#endif /* SYNC_EVENT */
#if END_OF_RADIO_ACTIVITY_REPORTING
	HCI_RADIO_ACTIVITY_EVENT,
#endif /* END_OF_RADIO_ACTIVITY_REPORTING */
#endif /* SUPPORT_HCI_EVENT_ONLY */
#if (SUPPORT_CHANNEL_SOUNDING &&( SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION))
	CS_EVENT_REMOVE,
#endif /*(SUPPORT_CHANNEL_SOUNDING &&( SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION))*/
#if ((SUPPORT_MAC) && (MAC_LAYER_BUILD))
	MLME_TIMER_EVENT,
	DIRECT_DATA_TX_EVENT,
	INDIRECT_DATA_TIMEOUT_EVENT,
#if SUPPORT_MAC_HCI_UART
	MAC_HCI_HANDLER,
#endif /* SUPPORT_MAC_HCI_UART */
#endif/*((SUPPORT_MAC) && (MAC_LAYER_BUILD)) */
#if ((!SUPPORT_COEXISTENCE) &&  (SUPPORT_OPENTHREAD_1_2 && CONFIG_MAC_CSL_RECEIVER_ENABLE))
	CSL_RCV_TMR_EVENT,
#endif /*((!SUPPORT_COEXISTENCE) &&  (SUPPORT_OPENTHREAD_1_2 && CONFIG_MAC_CSL_RECEIVER_ENABLE))*/
#if(!SUPPORT_COEXISTENCE && DEFAULT_PHY_CALIBRATION_PERIOD && SUPPORT_MAC)
	PRDC_CLBR_TMR_EVENT,
#endif
#if SUPPORT_AUG_MAC_HCI_UART
	AUG_MAC_HCI_HANDLER,
#endif/*SUPPORT_AUG_MAC_HCI_UART */
#if SUPPORT_ANT
	ANT_RADIO_CMPLT_EVENT,
	ANT_STACK_TASK_EVENT,
#if SUPPORT_ANT_HCI_UART
	ANT_HCI_HANDLER,
#endif /*SUPPORT_ANT_TESTING */
#endif  /* SUPPORT_ANT */
#if (NUM_OF_CTSM_EMNGR_HNDLS >= 1)
	CUSTOM_HANDLE_1,
#endif
#if (NUM_OF_CTSM_EMNGR_HNDLS >= 2)
	CUSTOM_HANDLE_2,
#endif
#if (NUM_OF_CTSM_EMNGR_HNDLS >= 3)
	CUSTOM_HANDLE_3,
#endif
	MAX_EM_HANDLE
} handler_t;
/* Exported functions ------------------------------------------------------- */



/**
 * @brief  Used to initialize the event manager component.
 *
 * @param  None.
 *
 * @retval None.
 */
void emngr_init(void);

/**
 * @brief  Used to reset the event manager structure.
 *
 * @param  None.
 *
 * @retval Status : 0: SUCCESS. Otherwise: FAILED.
 */
int emngr_reset(void);

/** @ingroup event_manager_functions
 * @{
 */
/**
 * @brief  Used to initialize a certain handle in the event manager.
 *
 * @param  id  : [in] ID of the handle to be initialized. [Range: 0 to 255].
 * @param  max : [in] Maximum number of events that can be added to this handle. [Range: 0 to 255].
 * @param  call_back_fun : [in] Pointer to the function that will process the events of that handle.
 *
 * @retval Status : 0: SUCCESS. Otherwise: FAILED.
 */
int emngr_handle_init(unsigned char id, unsigned char max,
	void (*call_back_fun)(void *));

/**
 * @brief  Used to remove a certain handle from the event manager.
 *
 * @param  id : [in] ID of the handle to be removed. [Range: 0 to 255].
 *
 * @retval Status : 0: SUCCESS. Otherwise: FAILED.
 */
int emngr_handle_remove(
	unsigned char id);

/**
 * @brief  Used to post an event to a certain event manager handle that is identified by the handle ID.
 *
 * @param  id 		: [in] ID of the handle to which the event will be posted. [Range: 0 to 255].
 * @param  event 	: [in] Void pointer pointing to the data related to the posted event.
 *
 * @retval Status : 0: SUCCESS. Otherwise: FAILED.
 */
int emngr_post_event(unsigned char id, void *event);

/**
 * @brief  Used to post an event to a certain event manager handle that is identified by the handle ID.
 * 		   This Event will be handled first
 *
 * @param  id 		: [in] ID of the handle to which the event will be posted. [Range: 0 to 255].
 * @param  event 	: [in] Void pointer pointing to the data related to the posted event.
 *
 * @retval Status : 0: SUCCESS. Otherwise: FAILED.
 */
int emngr_post_event_first(unsigned char id ,void *event);

/**
 * @brief  Used to get an event from a certain event manager handle that is identified by the handle ID.
 *
 * @param  id : [in] ID of the handle whose events are to be gotten. [Range: 0 to 255].
 *
 * @retval event handle : Void pointer pointing to the returned event.
 */
void * emngr_get_event(unsigned char id);

/**
 * @brief  Used to process an event from a a certain event manager handle that is identified by the handle ID, by first getting the event (in case the handle contains any) then processing it through the associated callback function.
 *
 * @param  id : [in] ID of the handle whose events are to be processed. [Range: 0 to 255].
 *
 * @retval Status : 0: SUCCESS. Otherwise: FAILED.
 */
int emngr_process_event(unsigned char id);

/**
 * @brief  Used to return the number of events in the event manager.
 *
 * @param  None.
 *
 * @retval Events Number.
 */
int emngr_get_total_events(void);

/**
 * @brief  Used to return the number of events in a certain handle that is identified by the handle ID.
 *
 * @param  id : [in] ID of the handle whose events number is to be returned. [Range: 0 to 255].
 *
 * @retval Events Number per handle.
 */
int emngr_get_handle_events(unsigned char id);

/**
 * @brief  Used to set a flag "busy_flag" to determine whether a certain handle is busy processing an event.
 *
 * @param  id 			: [in] ID of the handle whose "busy_flag" parameter value is to be set. [Range: 0 to 255].
 * @param  busy_flag  	: [in] Value to be set to the "busy_flag" parameter. 0: EVENT_NOT_BUSY. 1: EVENT_BUSY.
 *
 * @retval None.
 */
void emngr_set_event_handle_busy(unsigned char id, unsigned char busy_flag);

/**
 * @brief  Used to return the state of the event "busy_flag", which indicates whether the events of a certain handle, identified by the handle ID, are being currently processed.
 *
 * @param  id : [in] ID of the handle whose events are being currently processed. [Range: 0 to 255].
 *
 * @retval busy_flag state : 0: EVENT_NOT_BUSY. 1: EVENT_BUSY.
 */
int emngr_is_event_busy(
		unsigned char id);

/**
 * @brief  Used to loop through all the registered handles in the event manager and process their events, if any exists.
 *
 * @param None.
 *
 * @retval None.
 */
void emngr_handle_all_events(void);

/**
 * @brief  Used to process an event of a certain event manager handle that is identified by the handle ID.
 *
 * @param  id : [in] ID of the handle whose event is to be processed. [Range: 0 to 255].
 *
 * @retval None.
 */
void emngr_handle_event(handler_t id);

/**
 * @brief  Used to return a pointer to the first event in a certain event manager handle, identified by the handle ID, without dequeuing that event.
 *
 * @param  id : [in] ID of the handle whose first event is to be returned without being removed from the handle queue. [Range: 0 to 255].
 *
 * @retval : void Pointer to the first event of the specified handle.
 */
void * emngr_peak_frst_event(unsigned char id);

/**
 * @brief  Used to delete an event from  a certain event manager handle, identified by the handle ID.
 *
 * @param  id 	: [in] ID of the handle whose one of its events is to be deleted. [Range: 0 to 255].
 * @param  data : [in] Pointer to the event data to be used for searching for the event to be deleted out of all the events in the specified handle.
 *
 * @retval handle : Void pointer to the event data of the event node that is being deleted.
 */
void * emngr_del_event(unsigned char id, void *data);


/* @breif Used to remove a certain event within a queue that satisfies the condition set by cbk
 * 		  the cbk can delete priv data if it has allocated data.
 * @param id: [In] ID of the queue to search for
 * @param only_one_event: [In] if False delete all events that satisfies the condition
 * @param conitional_data: [In] Optional pointer to pass to cbk along with priv data
 * @param cbk: [In] A callback function that returns True if the event priv data satisfies a condition
 * @retval True if any event was found and removed
 * */
uint8_t emngr_remove_conditional_event( uint8_t id, uint8_t only_one_event,
		void* conditional_data,  conditional_cbk cbk);


/*
 * @brief Check if MCU can sleep in the case that all events in the event manager are busy and
 * 		  of type that allows sleep while busy.
 * @retval True if MCU can sleep.
 * */
uint8_t emngr_can_mcu_sleep(void);

/* @breif Used to process a certain event within a queue that satisfies the condition set by cbk
 * @param id: [In] ID of the queue to search for
 * @param only_one_event: [In] if False process all events that satisfies the condition
 * @param conitional_data: [In] Optional pointer to pass to cbk along with priv data
 * @param cbk: [In] A callback function that returns True if the event priv data satisfies a condition
 * */
uint8_t emngr_process_conditional_event( uint8_t id, uint8_t only_one_event,
		void* conditional_data,  conditional_cbk cbk);

/**
 * @}
 */

#endif /* INCLUDE_EVENT_MANAGER_H_ */
