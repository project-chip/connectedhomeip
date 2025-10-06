/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/pta.h#1 $*/
/**
 ******************************************************************************
 * @file    pta.h
 * @brief   This file contains all prototypes for the public PTA APIs
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

#ifndef PTA_H_
#define PTA_H_
#include "common_types.h"
#if (SUPPORT_PTA)

#define PTA_HCI_TESTING 0

/***************************** PUBLIC ENUMERATIONS *****************************/
/**
 * @brief Enumeration holding the PTA enable and disable.
 */
typedef enum {
	PTA_DISABLED = 0,
	PTA_ENABLED,
} pta_state;

#if (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION || (SUPPORT_LE_PERIODIC_ADVERTISING && (SUPPORT_EXPLCT_OBSERVER_ROLE || SUPPORT_SYNC_ISOCHRONOUS)))
/**
 * @brief Enumeration holding the event types passed to the BLE_SetLinkCoexPriority().
 */
typedef enum {
	PTA_LINK_COEX_CONN,
	PTA_LINK_COEX_PRDC_SCAN,
} pta_link_coex_event_type;
#endif /* (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION || \
		  (SUPPORT_LE_PERIODIC_ADVERTISING && (SUPPORT_EXPLCT_OBSERVER_ROLE || SUPPORT_SYNC_ISOCHRONOUS))) */

#if (SUPPORT_BRD_ISOCHRONOUS || SUPPORT_SYNC_ISOCHRONOUS || (SUPPORT_CONNECTED_ISOCHRONOUS && (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION)))
/**
 * @brief Enumeration holding the event types passed to the BLE_SetISOCoexPriority().
 */
typedef enum {
	PTA_ISO_CIG,
	PTA_ISO_BIG,
} pta_iso_type;
#endif /* (SUPPORT_BRD_ISOCHRONOUS || SUPPORT_SYNC_ISOCHRONOUS || \
		  (SUPPORT_CONNECTED_ISOCHRONOUS && (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION))) */


/**
 * @brief Enumeration holding all the error codes for the PTA interfaces
 */
typedef enum pta_error {
	PTA_ERROR_SUCCESS,
	PTA_ERROR_PTA_NOT_ENABLED,
	PTA_ERROR_INVALID_PRIORITY_CONF,
	PTA_ERROR_UNKNOWN_CONN_HANDLE,
	PTA_ERROR_UNKNOWN_PRDC_SYNC_HANDLE,
	PTA_ERROR_UNKNOWN_CIG_HANDLE,
	PTA_ERROR_UNKNOWN_BIG_HANDLE,
	PTA_ERROR_INVALID_NBR_OF_PKTS,
	PTA_ERROR_INVALID_TIMEOUT,
	PTA_ERROR_INVALID_REQUEST_TO_EVENT_TIME,
	PTA_ERROR_INVALID_PTA_STATE,
	PTA_ERROR_INIT_ALREADY_CALLED,
	PTA_ERROR_PTA_ENABLED_IN_INIT,
	PTA_ERROR_PTA_INIT_NOT_CALLED,
} pta_error;

/***************************** Functions Prototypes *****************************/
/*##### PTA' Group #####*/
/** @ingroup  pta_functions
 *  @{
 */
/**
 * @brief Used to enable and disable the PTA.
 * @note This API can be called directly to enable/disable the PTA feature. In case the BLE controller is supported, beside this API,
 * 			there is an option to enable/disable the PTA feature through the "HCI_CMD_OCF_PTA_ENABLE" custom HCI command.
 *
 * @param enable: [in] 0: Disable. 1: Enable.
 *
 * @retval pta_error : 	PTA_ERROR_INVALID_PTA_STATE:
 * 							- If enable is passed while the PTA is already enabled.
 * 							- If disabled is passed while the PTA is already disabled.
 * 							- If a value outside of the pta_state enumeration is passed.
 * 						PTA_ERROR_SUCCESS : Otherwise.
 */
pta_error pta_enable(
		pta_state enable);

/**
 * @brief Used to initialize the PTA feature. The PHY sequences are configured with
 * 		  respect to the "request_to_event_time" parameter.
 * @note This API can be called directly to initialize the PTA feature. In case the BLE controller is supported, beside this API,
 * 			there is an option to initialize the PTA feature through the "HCI_CMD_OCF_PTA_INIT" custom HCI command.
 *
 * @param request_to_event_time : [in] Time between the request signal assertion
 * 									  and beginning of event on air.
 *
 * @retval pta_error : 	PTA_DISABLED: If PTA is disabled.
 * 					   	PTA_ERROR_INVALID_REQUEST_TO_EVENT_TIME: If the request_to_event_time is greater than the minimum time of the TX/RX interpacket time.
 * 					   	PTA_ERROR_SUCCESS : Otherwise.
 */
pta_error pta_init(
		uint8_t request_to_event_time);

/***************************** BLE Functions *****************************/
#if (SUPPORT_BLE)
#if (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION || \
	(SUPPORT_LE_EXTENDED_ADVERTISING && SUPPORT_LE_PERIODIC_ADVERTISING && (SUPPORT_EXPLCT_OBSERVER_ROLE || SUPPORT_SYNC_ISOCHRONOUS)))
/**
 * @brief Used to configure the priority of the ACL and Periodic Scan events.
 * @note This API can be called directly to configure the priority of the ACL and Periodic Scan events. In case the BLE controller is supported, beside this API,
 * 			there is an option to configure the priority of the ACL and Periodic Scan events through the "HCI_CMD_OCF_PTA_BLE_SetLinkCoexPriority" custom HCI command.
 *
 * @param event_type	: [in] Either ACL or Periodic Scan event.
 * @param handle		: [in] ACL Handle or Periodic Scan Handle.
 * @param priority		: [in] Determines the state of each priority mode.
 * @param priority_mask	: [in] Determines which priorities are in effect in
 * 							  the priority variable.
 * @param acl_multi_slot_nbr_of_packets	: [in] Number of protected slots.
 * @param link_loss_limit_timeout		: [in] Timeout percentage for link loss mode
 *
 * @retval pta_error : 	PTA_DISABLED: If PTA is disabled.
 * 						PTA_ERROR_INVALID_PRIORITY_CONF:
 * 							- If Forced configuration = 11
 * 							- priority_mask did not mask away the reserved bits in priority
 * 							- priority_mask masked one bit only of the first two bits in the priority.
 * 							- event_type has invalid event type.
 * 							- ACL Multi Slot is used with periodic sync event.
 * 							- ACL Multi Slot is used with Link Loss Limit.
 * 						PTA_ERROR_UNKNOWN_CONN_HANDLE		: Connection Handle not found.
 * 						PTA_ERROR_UNKNOWN_PRDC_SYNC_HANDLE	: Periodic Sync Handle not found.
 * 						PTA_ERROR_INVALID_NBR_OF_PKTS		: acl_multi_slot_nbr_of_slots > PACKETS_PER_EVENT_MAX
 * 			 			PTA_ERROR_INVALID_TIMEOUT			: link_loss_limit_timeout < 0 or > 100.
 * 			 			PTA_ERROR_SUCCESS					: Otherwise.
 */
pta_error pta_set_link_coex_priority(
		pta_link_coex_event_type event_type,
		uint16_t handle,
		uint32_t priority,
		uint32_t priority_mask,
		uint8_t acl_multi_slot_nbr_of_packets,
		uint8_t link_loss_limit_timeout);
#endif /* (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION || \
		  (SUPPORT_LE_EXTENDED_ADVERTISING && SUPPORT_LE_PERIODIC_ADVERTISING && (SUPPORT_EXPLCT_OBSERVER_ROLE || SUPPORT_SYNC_ISOCHRONOUS))) */

#if (SUPPORT_BRD_ISOCHRONOUS || SUPPORT_SYNC_ISOCHRONOUS || SUPPORT_CONNECTED_ISOCHRONOUS)
/**
 * @brief Used to configure the priority of the BIG and CIG events.
 * @note This API can be called directly to configure the priority of the BIG and CIG events. In case the BLE controller is supported, beside this API,
 * 			there is an option to configure the priority of the BIG and CIG events through the "HCI_CMD_OCF_PTA_BLE_SetISOCoexPriority" custom HCI command.
 *
 * @param iso_type		: [in] Either BIG or CIG event.
 * @param group_id		: [in] Isochronous Group ID.
 * @param priority		: [in] Determines the state of each priority mode.
 * @param priority_mask	: [in] Determines which priorities are in effect in
 * 							  the priority variable.
 * @param link_loss_limit_timeout : [in] Timeout percentage for link loss mode
 *
 * @retval pta_error : 	PTA_DISABLED: If PTA is disabled.
 * 						PTA_ERROR_INVALID_PRIORITY_CONF:
 * 							- If Forced configuration = 11
 * 							- priority_mask did not mask away the reserved bits in priority
 * 							- priority_mask masked one bit only of the first two bits in the priority.
 * 							- iso_type has invalid event type.
 * 							- Protect All or Protect RTN is used with BIG.
 * 							- Link Loss Limit is configured for Broadcaster Role for BIG.
 * 						PTA_ERROR_UNKNOWN_BIG_HANDLE : BIG ID not found.
 * 						PTA_ERROR_UNKNOWN_CIG_HANDLE : CIG ID not found.
 * 						PTA_ERROR_INVALID_TIMEOUT	 : link_loss_limit_timeout < 0 or > 100.
 * 						PTA_ERROR_SUCCESS			 : Otherwise.
 */
pta_error pta_set_iso_coex_priority(
		pta_iso_type iso_type,
		uint8_t group_id,
		uint32_t priority,
		uint32_t priority_mask,
		uint8_t link_loss_limit_timeout);
#endif /* (SUPPORT_BRD_ISOCHRONOUS || SUPPORT_SYNC_ISOCHRONOUS || SUPPORT_CONNECTED_ISOCHRONOUS) */
#endif /* SUPPORT_BLE */

/**
 * @brief Used to configure the priority of the generic BLE events.
 * @note This API can be called directly to configure the priority of the generic BLE events. In case the BLE controller is supported, beside this API,
 * 			there is an option to configure the priority of the generic BLE events through the "HCI_CMD_OCF_PTA_BLE_SetCoexPriority" custom HCI command.
 *
 * @param priority		: [in] Determines the state of each priority mode.
 * @param priority_mask	: [in] Determines which priorities are in effect in
 * 							  the priority variable.
 *
 * @retval 	pta_error : PTA_DISABLED: If PTA is disabled.
 * 						PTA_ERROR_INVALID_PRIORITY_CONF:
 * 							- If Forced configuration = 11
 * 							- priority_mask did not mask away the reserved bits in priority
 * 							- priority_mask masked one bit only of the first two bits in the priority.
 * 						PTA_ERROR_SUCCESS : Otherwise.
 */
pta_error pta_set_coex_priority(
		uint32_t priority,
		uint32_t priority_mask);

/***************************** MAC Functions *****************************/
#if (SUPPORT_MAC)
/**
 * @brief Used to configure the priority of the MAC Packets.
 * @note This API can be called directly to configure the priority of the MAC Packets. Beside this API,
 * 			there is an option to configure the priority of the MAC Packets through the "PTA_SET_PRIORITY" MAC custom HCI command.
 *
 * @param priority		: [in] Determines the state of each priority mode.
 * @param priority_mask	: [in] Determines which priorities are in effect in
 * 							  the priority variable.
 *
 * @retval pta_error :	PTA_DISABLED: If PTA is disabled.
 * 						PTA_ERROR_INVALID_PRIORITY_CONF:
 * 							- If Forced configuration = 11
 * 							- priority_mask did not mask away the reserved bits in priority
 * 							- priority_mask masked one bit only of the first two bits in the priority.
 * 			 			PTA_ERROR_SUCCESS : Otherwise.
 */
pta_error pta_set_mac_coex_priority(
		uint32_t priority,
		uint32_t priority_mask);
#endif /* SUPPORT_MAC */
/** @}
*/
#endif /* SUPPORT_PTA */
#endif /* PTA_H_ */
