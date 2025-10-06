/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/hci.h#1 $*/
/**
 ********************************************************************************
 * @file    hci.h
 * @brief   This file contains all the functions prototypes for the hci.h.
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

/** @defgroup  hci_intf HCI Layer
 *  @ingroup BLE_STACK_API
 *  @brief Provide APIs to interface with HCI layer, like HCI layer initialization, allocation and free of HCI buffer header, process the received HCI command packet ...etc, HCI APIS are defined in hci.h header file.
 *  @{
 */
/* Define to prevnt recursive inclusion */
#ifndef INCLUDE_HCI_H
#define INCLUDE_HCI_H

/* Includes ---------------------------------------------------------------------*/

#include <stdint.h>
#include "bsp.h"
#include "ll_intf.h"

/* Exported  Enumerations -----------------------------------------------------*/

/**
 * @brief Enumeration holding the types of the returned commands in response
 * 		  to a received HCI command.
 */
typedef enum {
	HCI_RETURN_COMMAND_TYPE_COMPLETE,
	HCI_RETURN_COMMAND_TYPE_STATUS,
} hci_return_command_type;

/* Exported  Defines -----------------------------------------------------------*/

typedef uint8_t (*hci_trnsprt_cbk)(
	ble_buff_hdr_t *ptr_evnt_hdr);

/** @ingroup  ext_hci_cmds External HCI Commands
 * @{
 */
typedef ble_stat_t (*ble_ext_custm_cb_t) (uint16_t ocf, uint8_t *pckt_p, uint8_t *evnt_pckt_p, uint8_t* params_length, hci_return_command_type* return_command_type);
/**@}
 */

/* Exported functions ---------------------------------------------------------*/

/**
 * @brief Initialize the HCI layer and Registers a callback function to the upper layer
 *
 * @param p_trnsprt_cbk : [in] callback function
 *
 * @retval always returns SUCCESS
 */
ble_stat_t ll_hci_init(
	hci_trnsprt_cbk p_trnsprt_cbk);

/**
 * @brief  get a pointer to the HCI dispatch table.
 *
 * @param  p_p_dispatch_tbl : [out] pointer to be filled by the address of the HCI dispatch table.
 */
void hci_get_dis_tbl(
	const struct hci_dispatch_tbl** p_p_dispatch_tbl);

#if !SUPPORT_HCI_EVENT_ONLY || SUPPORT_HCI_EVENT_ONLY_TESTING
#if (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION)
/**
 * @brief  Process the received ACL data packet from the host.
 *
 * @param  ptr_buff_hdr  : [in] Pointer to the HCI command packet.
 *
 * @retval ble_stat_t  : Command status to be sent to the Host.
 */
ble_stat_t hci_rcvd_acl_data_pckt_hndlr(
		ble_buff_hdr_t *ptr_buff_hdr);
#endif /* (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION) */

#if ((SUPPORT_CONNECTED_ISOCHRONOUS && (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION)) || (SUPPORT_BRD_ISOCHRONOUS))
/**
 * @brief  Process the received ISO data packet from the host.
 *
 * @param  ptr_buff_hdr  : [in] Pointer to the HCI command packet.
 *
 * @retval ble_stat_t  : Command status to be sent to the Host.
 */
ble_stat_t hci_rcvd_iso_data_pckt_hndlr(
		ble_buff_hdr_t *ptr_buff_hdr);
#endif /* ((SUPPORT_CONNECTED_ISOCHRONOUS && (SUPPORT_MASTER_CONNECTION || SUPPORT_SLAVE_CONNECTION)) || SUPPORT_BRD_ISOCHRONOUS) */

/**
 * @brief  Process the received HCI command packet from the host.
 *
 * @param  ptr_rcvd_pckt_strt   : [in] Pointer to the HCI command packet.
 * @param  rcvd_pckt_len   		: [in] Length of the HCI command packet.
 * @retval ble_stat_t : Command status to be sent to the Host.
 */
ble_stat_t hci_rcvd_cmd_pckt_hndlr(
	uint8_t *ptr_rcvd_pckt_strt,
	uint16_t rcvd_pckt_len);

#endif /* !SUPPORT_HCI_EVENT_ONLY || SUPPORT_HCI_EVENT_ONLY_TESTING */

/**
 * @brief  allocate a message structure to be sent to the HCI layer through event manager.
 *
 * @retval ble_buff_hdr_t* : pointer to the buffer header allocated.
 */
ble_buff_hdr_t* hci_alloc_msg(void);

/**
 * @brief  free an allocated message structure
 *
 * @param  ptr_hci_msg  : [in] Pointer to the message to be freed.
 */
void hci_free_msg(
		ble_buff_hdr_t *ptr_hci_msg);
/**
 * @}
 */

/** @ingroup  ext_hci_cmds External HCI Commands
 * @{
 */
/**
 * @brief Registers a custom callback function to be called when the
 * 		  hci_cstm_pckt_hndlr() cannot resolve the OCF.
 *
 * @param ext_custm_cbk :[IN] Pointer to the callback function.
 *
 * @retval False if the cbk is null, True otherwise.
 */
uint8_t hci_rgstr_ble_external_custom_cbk(ble_ext_custm_cb_t ext_custm_cbk);

/**@}
 */


#if SUPPORT_HCI_EVENT_ONLY

/**
 * @brief  Initialize event manager data/events queue call backs
 *
 * @retval ble_stat_t  None
 */
void hci_init_events_queues(void);
/**
 * @brief  Post data/event to appropriate queue. This function is called
 * 		   in case of sending data/events to host
 *
 * @param  ptr_evnt_hdr	data/event ble_buff_hdr to send
 *
 * @retval ble_stat_t  None
 */
uint8_t hci_queue_send_pckt(ble_buff_hdr_t *ptr_evnt_hdr);

/*
 * @brief register callback to be called sending data to host
 * @param upper_layer_cbk:  host callback
 * @retval None
 * */
void hci_rgstr_hst_cbk(hst_cbk cbk);
/*
 * @brief register callback to be called on LL queue is full
 * 		  This function should be called at initialization
 * @param cbk:  host callback
 * @retval None
 * */
void hci_rgstr_hst_cbk_ll_queue_full(hst_cbk_queue_full cbk);

/*
 * @brief sets the LE event mask in hci event only configuration
 * @param event_mask : [In] an array of 8 bytes representing new event mask
 * @retval: 		   None
 * */
void hci_ll_set_le_event_mask(uint8_t event_mask[8]);


/*
 * @brief sets the event mask in hci event only configuration
 * @param event_mask : [In] an array of 8 bytes representing new event mask
 * @retval: 		   None
 * */
void hci_ll_set_event_mask(uint8_t event_mask[8]);


/*
 * @brief sets the page 2 event mask in hci event only configuration
 * @param event_mask : [In] an array of 8 bytes representing new event mask
 * @retval: 		   None
 * */
void hci_ll_set_event_mask_page2(uint8_t event_mask[8]);

/*
 * @brief sets the custom event mask in hci event only configuration
 * @param cstm_evnt_mask : [In] custom event mask bitfield
 * @retval: 		  		None
 * */
void hci_ll_set_custom_event_mask(uint8_t cstm_evnt_mask);

#endif /* SUPPORT_HCI_EVENT_ONLY */

#endif /* INCLUDE_HCI_H */
