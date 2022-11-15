/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#ifdef RSI_M4_INTERFACE
#include "rsi_driver.h"
#include "rsi_m4.h"
#include <core_cm4.h>

#ifndef ROM_WIRELESS

/** @addtogroup ROM
* @{
*/
/*==============================================*/
/**
 * @fn           void ROM_WL_rsi_raise_pkt_pending_interrupt_to_ta(void)
 * @brief        Raise the packet pending interrupt to TA
 * @param[in]    void  
 * @return       void
 */
void ROM_WL_rsi_raise_pkt_pending_interrupt_to_ta(void)
{

  // Write the packet pending interrupt to TA register
  M4SS_P2P_INTR_SET_REG = TX_PKT_PENDING_INTERRUPT;

  return;
}

/*==============================================*/
/**
 * @fn          int32_t ROM_WL_rsi_send_pkt_to_ta(rsi_m4ta_desc_t *tx_desc)
 * @brief       Programme the shared memory between the M4 and TA to send packet to TA
 * @param[in]   tx_desc -  shared memory descriptors
 * @return      0 - Success \n
 * 			    1 - Failure
 */
int32_t ROM_WL_rsi_send_pkt_to_ta(rsi_m4ta_desc_t *tx_desc)
{

  //raise interrupt to TA
  ROM_WL_rsi_raise_pkt_pending_interrupt_to_ta();

  // return success
  return 0;
}
/*==============================================*/
/**
 * @fn          int ROM_WL_rsi_submit_rx_pkt(global_cb_t *global_cb_p)
 * @brief       Submit receiver packets 
 * @param[in]   global_cb_t -  shared memory descriptors
 * @return      0 - Success \n
 * 			    1 - Failure
 */
int ROM_WL_rsi_submit_rx_pkt(global_cb_t *global_cb_p)
{

  rsi_pkt_t *rx_pkt = NULL;

  rsi_driver_cb_t *rsi_driver_cb = global_cb_p->rsi_driver_cb;
  rsi_m4ta_desc_t *rx_desc       = global_cb_p->rx_desc;

  int8_t *pkt_buffer = NULL;
  //Get commmon cb pointer

  if (M4SS_P2P_INTR_SET_REG & RX_BUFFER_VALID) {
    RSI_ASSERTION(SAPIS_M4_RX_BUFF_ALREDY_VALID, "\nIn submit rx pkt , RX buffer is already valid\n");

    return -2;
  }

  // Allocate packet to receive packet from module
  rx_pkt = ROM_WL_rsi_pkt_alloc(global_cb_p, &rsi_driver_cb->rx_pool);

  if (rx_pkt == NULL) {
    RSI_ASSERTION(SAPIS_M4_DEBUG_OUT, "\nIn submit rx pkt , RX buffer is not available\n");

    global_cb_p->submit_rx_pkt_to_ta = 1;

    return -1;
  }

  pkt_buffer = (int8_t *)&rx_pkt->desc[0];

  // Fill source address in the TX descriptors
  rx_desc[0].addr = (M4_MEMORY_OFFSET_ADDRESS + (uint32_t)pkt_buffer);

  // Fill source address in the TX descriptors
  rx_desc[0].length = (16);

  // Fill source address in the TX descriptors
  rx_desc[1].addr = (M4_MEMORY_OFFSET_ADDRESS + (uint32_t)(pkt_buffer + 16));

  // Fill source address in the TX descriptors
  rx_desc[1].length = (1600);

  ROM_WL_raise_m4_to_ta_interrupt(RX_BUFFER_VALID);

  return 0;
}

/*====================================================*/
/**
 * @fn          rsi_pkt_t* ROM_WL_rsi_frame_read(global_cb_t *global_cb_p)
 * @brief       Read response for all the command and data from module.
 * @param[in]   global_cb_p - pointer to the global control block
 * @return      Packet which is read from the  module
 */

rsi_pkt_t *ROM_WL_rsi_frame_read(global_cb_t *global_cb_p)
{

  return ROM_WL_rsi_dequeue_pkt(global_cb_p, &global_cb_p->rsi_driver_cb->m4_rx_q);
}

/*====================================================*/
/**
 * @fn          int16_t ROM_WL_rsi_frame_write(global_cb_t *global_cb_p, rsi_frame_desc_t *uFrameDscFrame,
 * 									uint8_t *payloadparam,uint16_t size_param)
 * @brief       Process a command to the module.
 * @param[in]   global_cb_p - pointer to the global control block
 * @param[in]   uFrameDscFrame - frame descriptor
 * @param[in]   payloadparam - pointer to the command payload parameter structure
 * @param[in]   size_param - size of the payload for the command
 * @return      0              - Success \n 
 *              Negative Value - Failure
 */

int16_t ROM_WL_rsi_frame_write(global_cb_t *global_cb_p,
                               rsi_frame_desc_t *uFrameDscFrame,
                               uint8_t *payloadparam,
                               uint16_t size_param)
{
  rsi_m4ta_desc_t *tx_desc = global_cb_p->tx_desc;

  if (((uFrameDscFrame->frame_len_queue_no[1]) >> 4) == 0x0) {
    RSI_ASSERTION(SAPIS_M4_TX_INVALID_DESC, "\nIn frame write , Invalid TX frame descriptor\n");

    return -1;
  }

  // Fill source address in the TX descriptors
  tx_desc[0].addr = (M4_MEMORY_OFFSET_ADDRESS + (uint32_t)uFrameDscFrame);

  // Fill source address in the TX descriptors
  tx_desc[0].length = (16);

  // Fill source address in the TX descriptors
  tx_desc[1].addr = (M4_MEMORY_OFFSET_ADDRESS + (uint32_t)payloadparam);

  // Fill source address in the TX descriptors
  tx_desc[1].length = (size_param);

  ROM_WL_rsi_send_pkt_to_ta(&tx_desc[0]);

  return 0;
}
/*====================================================*/
/**
 * @fn          void ROM_WL_raise_m4_to_ta_interrupt(uint32_t interrupt_no)
 * @brief       Set interrupt.
 * @param[in]   interrupt_no - Process of a interrupt number 
 * @return      void 
 */

void ROM_WL_raise_m4_to_ta_interrupt(uint32_t interrupt_no)
{
  M4SS_P2P_INTR_SET_REG = interrupt_no;
}

#if 0
/*====================================================*/
/**
 * @fn          void ROM_WL_clear_m4_to_ta_interrupt(uint32_t interrupt_no)
 * @brief       Clear interrupt raised by M4.
 * @param[in]   interrupt_no - Process of a interrupt number 
 * @return      void
 */
void ROM_WL_clear_m4_to_ta_interrupt(uint32_t interrupt_no)
{
  M4SS_P2P_INTR_CLR_REG = interrupt_no;   
}

/*====================================================*/
/**
 * @fn          void ROM_WL_clear_ta_interrupt_mask()
 * @brief       Clear interrupt raised by M4.
 * @param[in]   void  
 * @return      void
 */
void ROM_WL_clear_ta_interrupt_mask()
{
  TASS_P2P_INTR_MASK_CLR  = ~0;
}

/*====================================================*/
/**
 * @fn          void ROM_WL_set_ta_interrupt_mask()
 * @brief       Process a interrupt mask.
 * @param[in]   void  
 * @return      void 
 */
void ROM_WL_set_ta_interrupt_mask()
{
  TASS_P2P_INTR_MASK_SET  = ~0;
}
#endif
/*====================================================*/
/**
 * @fn          void ROM_WL_mask_ta_interrupt(uint32_t interrupt_no)
 * @brief       Process a interrupt mask.
 * @param[in]   void  
 * @return      void
 */
void ROM_WL_mask_ta_interrupt(uint32_t interrupt_no)
{
  TASS_P2P_INTR_MASK_SET = interrupt_no;
}
/*====================================================*/
/**
 * @fn          void ROM_WL_unmask_ta_interrupt(uint32_t interrupt_no)
 * @brief       Process a interrupt unmask.
 * @param[in]   interrupt_no - Process of a interrupt number 
 * @return      void
 */
void ROM_WL_unmask_ta_interrupt(uint32_t interrupt_no)
{
  TASS_P2P_INTR_MASK_CLR = interrupt_no;
}

#endif
/*====================================================*/
/**
 * @fn          void ROM_WL_clear_ta_to_m4_interrupt(uint32_t interrupt_no)
 * @brief       Clear interrupt raised by TA.
 * @param[in]   interrupt_no - Process of a interrupt number 
 * @return      void 
 */
void ROM_WL_clear_ta_to_m4_interrupt(uint32_t interrupt_no)
{
  TASS_P2P_INTR_CLEAR   = interrupt_no;
  TASS_P2P_INTR_CLR_REG = interrupt_no;
}
/*==============================================*/
/**
 * @fn           void ROM_WL_rsi_transfer_to_ta_done_isr(global_cb_t *global_cb_p)
 * @brief        Called when TA has read the memory content in the shared memory and raises the interrupt to M4
 * @param[in]    global_cb_p - pointer to the global control block
 * @return       void
 */
void ROM_WL_rsi_transfer_to_ta_done_isr(global_cb_t *global_cb_p)
{
  // Unmask TX Event
  ROM_WL_rsi_unmask_event_from_isr(global_cb_p, RSI_TX_EVENT);
}

/*==============================================*/
/**
 * @fn           rsi_pkt_t* ROM_WL_rsi_get_rx_pkt(global_cb_t *global_cb_p)
 * @brief        Read a packet from NWP 
 * @param[in]    global_cb_p - pointer to the global control block
 * @return       void
 */
rsi_pkt_t *ROM_WL_rsi_get_rx_pkt(global_cb_t *global_cb_p)
{

  rsi_m4ta_desc_t *rx_desc = global_cb_p->rx_desc;
  if (rx_desc[0].addr == (uint32_t)NULL) {
    RSI_ASSERTION(SAPIS_M4_RX_BUFF_ADDR_NULL, "\nIn get  rx pkt,Rx Buffer in rx dma desc is NULL\n");
    // ASSERTION
    return NULL;
  }

  return ((rsi_pkt_t *)((rx_desc[0].addr - M4_MEMORY_OFFSET_ADDRESS) - 4));
}
/*==============================================*/
/**
 * @fn           void ROM_WL_rsi_receive_from_ta_done_isr(global_cb_t *global_cb_p)
 * @brief        Called when DMA done for RX packet is received
 * @param[in]    global_cb_p - pointer to the global control block
 * @return       void
 */
void ROM_WL_rsi_receive_from_ta_done_isr(global_cb_t *global_cb_p)
{

  rsi_pkt_t *rx_pkt              = NULL;
  rsi_driver_cb_t *rsi_driver_cb = global_cb_p->rsi_driver_cb;

  rx_pkt = ROM_WL_rsi_get_rx_pkt(global_cb_p);
  if (rx_pkt != NULL) {
    // Enqueue the packet
    ROM_WL_rsi_enqueue_pkt_from_isr(global_cb_p, &rsi_driver_cb->m4_rx_q, rx_pkt);
  } else {
    RSI_ASSERTION(SAPIS_M4_RX_BUFF_NULL_RECIEVED, "\n receive_from_ta_done_isr, Received NULL Packet \n");
  }

  // Set event RX pending from device
  ROM_WL_rsi_set_event_from_isr(global_cb_p, RSI_RX_EVENT);
}

/*==============================================*/
/**
 * @fn           void ROM_WL_rsi_m4_interrupt_isr(global_cb_t *global_cb_p)
 * @brief        Raise the packet pending interrupt to TA
 * @param[in]    global_cb_p - pointer to the global control block
 * @return       void
 */

void ROM_WL_rsi_m4_interrupt_isr(global_cb_t *global_cb_p)
{

  if (TASS_P2P_INTR_CLEAR & TX_PKT_TRANSFER_DONE_INTERRUPT) {

    // Call done interrupt isr
    ROM_WL_rsi_transfer_to_ta_done_isr(global_cb_p);

    // Clear the interrupt
    ROM_WL_clear_ta_to_m4_interrupt(TX_PKT_TRANSFER_DONE_INTERRUPT);

  } else if (TASS_P2P_INTR_CLEAR & RX_PKT_TRANSFER_DONE_INTERRUPT) {

    // Call done interrupt isr
    ROM_WL_rsi_receive_from_ta_done_isr(global_cb_p);

    // Clear the interrupt
    ROM_WL_clear_ta_to_m4_interrupt(RX_PKT_TRANSFER_DONE_INTERRUPT);

  } else {
    RSI_ASSERTION(SAPIS_M4_ISR_UNEXPECTED_INTR, "\nM4 ISR , unexpected interrupt  \n");
  }

  return;
}
#endif
/** @} */