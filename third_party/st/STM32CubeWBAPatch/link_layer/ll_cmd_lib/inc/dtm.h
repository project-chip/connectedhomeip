/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/dtm.h#1 $*/
/**
 ********************************************************************************
 * @file    dtm.h
 * @brief   header file for  Synopsys zigbee phy testing APIs
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

#ifndef MAC_CONTROLLER_INC_DTM_H_
#define MAC_CONTROLLER_INC_DTM_H_

/********************************************************************************************
 *  								Includes
 *********************************************************************************************/

#include "ral.h"

/*=========================================== MACROS ======================================*/
#define aMaxSIFSFrameSize 			18
#define macMinSIFSPeriod 			192
#define macMinLIFSPeriod 			640
#define MAX_PHY_PACKET_SIZE 		127
#define macMaxIFSPeriod 			1000
#define MAX_ERROR_PER_SINGLE_TX		5
/*============================= Enumerations ================================*/
/**
 * @brief:  Current DTM State
 */
typedef enum z_dtm_mode {
	Z_DTM_STOPPED = 0,
	Z_DTM_TX = 1,
	Z_DTM_RX = 2,
	Z_DTM_TX_ACK = 3,
	Z_DTM_CHECK_TX_ERROR = 4
} z_dtm_mode_e;

/*============================= Structures ================================*/
/**
 * @brief:  type to represent the current state of DTM TX.
 */
typedef struct Tx_Info
{
	uint16_t TxTotal; // Total number of transmitted Frames (with/without errors)
	uint16_t NoErr;  // Number of frames that are transmitted successfully
	uint16_t ErrNoAck; // Number of frames that are requesting ack and no Ack is received after transmission
	uint16_t ErrOther; // Number of frames that are transmitted with errors excluding the no ack error
} TxInfo_s;

/**
 * @brief:  type to represent the current state of DTM RX.
 */
typedef struct Rx_Info
{
	uint16_t RxTotal;// Total number of received Frames (with/without errors)
	uint16_t NoErr; // Number of frames that are successfully received
	uint16_t ErrFcs; // Number of frames that are received with invalid FCS
	uint16_t ErrOther; // Number of frames that are received with errors excluding the invalid FCS error
	uint8_t	lqi; // LQI  of the received Frame
} RxInfo_s;

/**
 * @brief:  structure to store all information related to DTM.
 */
typedef struct dtmInfo_st
{
	z_dtm_mode_e curr_mode;  // Current State (TX,RX,STopped)

	union{
		TxInfo_s TxInfo; // DTM information in case of TX and TX_ACK
		RxInfo_s RxInfo;  // DTM information in case of RX
	} tx_rx_info_u;

} dtmInfo_t;

/** @ingroup  mac_dtm
*  @{
*/
/*=========================================== API  ============================================*/
/**
 * @brief  this function is used to initialize or rest DTM , it also stops the current running DTM
 * @retval None.
 */
void dtmReset(void);

/**
 * @brief  get the the state of DTM (running or stopped)
 * @retval uin8_t . TRUE if DTM is running ,
 * 					FALSE if DTM is stopped
 */
uint8_t dtmIsEnabled(void);


/**
 * @brief  this function is used to start continuous DTM transmission with the given input parameters until the  dtmStop() API is called
 * @param  mPsdu [in]    : pointer to MAC DTM FRame  including MAC header.
 * @param  mLength [in]  : Length of the frame to be transmitted.
 * @param  mChannel [in] : Channel to transmit the frame on .
 * @param  IFS [in]      : Interframe spacing in us
 * @retval mac_status_enum_t. MAC error State of starting transmission
 */
mac_status_enum_t dtmStartTransmit(uint8_t * mPsdu, uint8_t mLength, uint8_t mChannel, uint16_t IFS);

/**
 * @brief  this function is used to start transmission of a single DTM Frame that is requesting ACK  with the given input parameters
 * @param  mPsdu [in]    : pointer to MAC DTM FRame  including MAC header.
 * @param  mLength [in]  : Length of the frame to be transmitted.
 * @param  mChannel [in] : Channel to transmit the frame on .
 * @param  IFS [in]      : Interframe spacing in us
 * @retval mac_status_enum_t. MAC error State of starting transmission
 */
mac_status_enum_t dtmStartTransmitwithAck(uint8_t * mPsdu, uint8_t mLength, uint8_t mChannel, uint16_t IFS);
/**
 * @brief  this function is used to stop the current Running DTM  (TX or RX)
 * @retval mac_status_enum_t. MAC error State of stopping DTM
 */
mac_status_enum_t dtmStop(uint16_t *num_rec_pckts, uint8_t *lqi);

/**
 * @brief  this function is used to start continuous DTM reception on the given channel
 * @param  mChannel [in] : Channel to receive on .
 * @retval mac_status_enum_t. MAC error State of starting transmission
 */
mac_status_enum_t dtmStartReceive(uint8_t aChannel);

/**
 * @brief  this function is used to start CCA on the given channel
 * @param  mChannel [in] : Channel to perform CCA  on .
 * @retval mac_status_enum_t. MAC error State of starting reception
 */
mac_status_enum_t dtmPerformCCA(uint8_t channel);


/**
 * @brief  get the current LQI of the last received packet
 * @retval uint8_t. LQI value
 */
uint8_t dtmGetLQIValue(void);
/**
 * @}
 */
/**
 * @brief  check if there is more DTM frames to transmit
 * @retval uin8_t . TRUE  Continue DTM transmission ,
 * 					FALSE Stop DTM transmission
 */
uint8_t dtmCheckMoreTx(void);

/**
 * @brief  check if that dtm is in reception state
 * @retval uin8_t . TRUE  DTM in reception mode ,
 * 					FALSE DTM is not in reception mode
 */
uint8_t dtmCheckRxState(void);

/**
 * @brief  Done function that should be called after each DTM event
 * @param  aFrame    	    : [in] pointer to MAC transmitted/Received Frame.
 * @param  aError			: [in] status error of overall transmission (success , No_ack, and other errors).
 * @retval None.
 */
void dtmRadioDone(otRadioFrame *aFrame, otError aError);

/**
* @brief  check if in Z_DTM_CHECK_TX_ERROR transmission mode or not
* @param  None.
* @retval uin8_t . TRUE  in Z_DTM_CHECK_TX_ERROR transmission mode ,
* 					FALSE Not in Z_DTM_CHECK_TX_ERROR transmission mode
*/
uint8_t dtmCheckTxErrorState(void);
/**
* @brief  count errors which happens in single tx in Z_DTM_CHECK_TX_ERROR transmission mode
* @param  tx_error : [in] type of error to be counted for this trial
* @retval None.
*/
void dtmCheckTxErrorCount(otError tx_error);
/**
* @brief  gives stored errors and its count that happens in single tx in Z_DTM_CHECK_TX_ERROR transmission mode
* @param  errors       : [in] pointer to array to point to array of stored errors in Z_DTM_CHECK_TX_ERROR transmission mode
* @param  error_count  : [in] pointer that will have number of errors that stored in errors array
* @retval None.
*/
void dtmTxCheckErrorCountDone(uint8_t ** errors, uint8_t *error_count);
/**
* @brief  start transmission in Z_DTM_CHECK_TX_ERROR transmission mode
* @param  mac_hndl           : [in] MAC instance handle
* @param  mPsdu              : [in] pointer to tx packet
* @param  mLength            : [in] length of tx packet
* @param  mChannel           : [in] channel that packet will be sent on
* @param  IFS                : [in] used Inter-frame-spacing
* @param  csma_en            : [in] flag to indicate to apply csma in this transmission if max_csma_retry_count != 0
* @param  backoff_count_max  : [in] maximum number of tx trials in every full csma trial
* @param  frame_retry        : [in] maximum number of trials after failure that not related to cca_failure
* @param  ack_req            : [in] indicate if this packet require ack or not
* @retval mac_status_enum_t.
*/
mac_status_enum_t dtmTxSpecificErrorCount(uint32_t mac_hndl, uint8_t * mPsdu, uint8_t mLength, uint8_t mChannel, uint16_t IFS,
		uint8_t csma_en, uint8_t backoff_count_max, uint8_t frame_retry, uint8_t ack_req);

/**
* @brief  reset tx_check_err_arr array and its counter for Z_DTM_CHECK_TX_ERROR mode
* @param  None.
* @retval None.
*/
void dtmTxErrorCountReset(void);

#endif /* MAC_CONTROLLER_INC_DTM_H_ */
