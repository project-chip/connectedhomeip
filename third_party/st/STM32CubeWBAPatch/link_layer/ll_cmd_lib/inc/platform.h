/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/platform.h#1 $*/
/**
 ********************************************************************************
 * @file    platform.h
 * @brief   this file include all helper functions in radio.c which is not described in Openthread
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


#ifndef INCLUDE_PLATFORM_H_
#define INCLUDE_PLATFORM_H_

#include "radio.h"
#include "common_types.h"

/*====================================   MACROS   =======================================*/
/*
 * The following macros are used for testing purpose only
 */
#define RADIO_TEST 		0
#define ACK_FRAME_TYPE  0x2
#define MAC_ACK_WAIT	864
#define ACK_LNGTH		5
/******************************************************************************************//**
 *  								 Enums
 *********************************************************************************************/
/**
 * @enum radio_state_enum_t
 * @brief the states of the Radio event it is used for coexistence
 *
 */
typedef enum _radio_evnt_type_enum
{
	TX_PCKT_STATE,      ///< Sending packet (not ACK)
	TX_ACK_STATE,       ///< Sending ACK
	RX_PCKT_STATE,		///< Receiving packet (not ACK)
	RX_ACK_STATE,		///< Receiving Ack
	RADIO_ED,			///< Energy detection
	RADIO_SLEEP_STATE,	///< Sleep state
	RADIO_DISABLED_STATE,///< Disable state
#if(SUPPORT_COEXISTENCE)
	TX_AT_STATE  ,       ///< Tx at specific time
	RX_AT_STATE  ,      ///< Rx at specific time
#endif /* SUPPORT_COEXISTENCE */

}radio_event_enum_t;
/******************************************************************************************//**
 *  								 Structures
 *********************************************************************************************/
/**  @ingroup systm_layer
*  @{
*/
/**
 * @struct mac_cbk_dispatch_tbl
 *
 * @brief structure of all callback functions used to notify mac layer after specific event compeletion
 */
struct mac_cbk_dispatch_tbl{
	/**
	 * @brief  callback function called after the end of Energy detection operation
	 *
	 * @param  aInstance:	radio instance
	 * @param  aEnergyScanMaxRssi: the result for ED operation which is RSSI value
	 *
	 * @retval none
	 *
	 * @note  This function is called after handling SM_DONE event in ral through a sequence of calls
	 * 		  ral_sm_done() -> ral_ed_scan_done() -> callback
	 */
	void (*mac_ed_scan_done)(otInstance *aInstance, int8_t aEnergyScanMaxRssi);
	/**
	 * @brief  callback function called after the end of transmission operation
	 *
	 * @param  aInstance:	radio instance
	 * @param  aFrame: 		pointer to the transmitted frame
	 * @param  aAckFrame: 	pointer to the received ack frame (in case of AR bit is detected in aFrame)
	 * @param  aError: 		Error code if happens in Transmission
	 *
	 * @retval none
	 * @note  This function is called after handling SM_DONE event in ral through a sequence of calls
	 * 		  ral_sm_done() -> ral_tx_done() -> callback
	 */
	void (*mac_tx_done)(otInstance *aInstance, otRadioFrame *aFrame, otRadioFrame *aAckFrame, otError aError);
	/**
	 * @brief  callback function called after the end of Reception operation
	 *
	 * @param  aInstance:	radio instance
	 * @param  aFrame: 		pointer to the received frame
	 * @param  aError: 		Error code if happens in transmission
	 *
	 * @retval none
	 * @note  This function is called after handling SM_DONE event in ral through a sequence of calls
	 * 		  ral_sm_done() -> ral_rx_done() -> callback
	 */
	void (*mac_rx_done)(otInstance *aInstance, otRadioFrame *aFrame, otError aError);
	/**
	 * @brief  callback function called after successful start of transmission in ral
	 *
	 * @param  aInstance:	radio instance
	 * @param  aFrame: 		pointer to the transmitted frame
	 * @param  aError: 		Error code if happens in transmission
	 *
	 * @retval none
	 * @note  This function is called after calling proc_radio_tx which is called with calling of otplatRadioTransmit
	 *        in case of no error returned. These errors may be (CCA channel access failure, ral busy due to ongoing transmission ...etc )
	 */
	void (*mac_tx_strtd)(otInstance *aInstance, otRadioFrame *aFrame);

	void (*mac_frm_updtd)(otInstance *aInstance, otRadioFrame *aFrame);

};
/**
 * @}
 *
 */
/******************************************************************************************//**
 *  								 APIs
 *********************************************************************************************/
/**  @ingroup systm_layer_cmn_proc
*  @{
*/
/**
 * @brief  this function is used to initialize radio layer including registeration for ral callbacks,
 *         some filter configurations, some automatic configurations like auto_sleep and auto_ack...etc
 *
 * @retval none
 */
void radio_init(void);
/**
 * @brief  This function called from upper layer init function (mac_init in case of mac integration e.g. Zigbee stack)
 * 		   to register the upper layer callback functions
 *
 * @param  ptr_cbk_dispatch_tbl: 		pointer to callback function to be registered
 *
 * @retval none
 *
 */
void radio_call_back_funcs_init(struct mac_cbk_dispatch_tbl * ptr_cbk_dispatch_tbl);
/**
 * @}
 *
 */

void otDispatch_tbl_init(otInstance *sInstance);
/**
 * @}
 *
 */

/**  @ingroup systm_layer_enc
*  @{
*/
/**
 * @brief  This function is used to encrypt frame pointed to by ptr_pckt
 *
 * @param  ptr_pckt:				pointer to the frame to be encrypted
 * @param  ptr_session_key: 		pointer to used session key
 * @param  ptr_ccm_nonce: 		    pointer to the nonce
 * @param  mic_len: 		    	mic length
 * @param  ad_len: 		   		 	a data length
 * @param  md_len: 		    		m data length
 *
 * @retval status of the encryption process
 *
 */
uint32_t radio_encrypt_pckts( uint8_t *ptr_pckt,
		 const  uint8_t  *ptr_session_key,
		 uint8_t  *ptr_ccm_nonce,
		 uint32_t  mic_len,
		 uint32_t  ad_len,
		 uint32_t  md_len);
/**
 * @}
 *
 */
/**  @ingroup systm_layer_cmn_config
*  @{
*/
/**
 * @brief  This function is used to configure automatic ack response by ral layer
 *         including AIFS, ack frame timeout.. etc
 *
 * @param  auto_ack_state:				Enabled or disabled
 *
 * @retval otError status of configuration
 *
 */
otError radio_set_auto_Ack_state(uint8_t auto_ack_state);
/**
 * @}
 *
 */
/**  @ingroup systm_layer_cmn_proc
*  @{
*/
/**
 * @fn	uint32_t  radio_reset()
 * @brief This API is used to reset radio layer operation which in turns stop all running operations
 */
uint32_t  radio_reset(void);
/**
 * @}
 *
 */
/**
 * @fn	proc_radio_rcv(uint8_t aChannel, uint32_t duration)
 * @brief This API is used to control ral setup for reception.
 * @param  aChannel:				channel to receive on
 * @param  duration:				reception duration in microsecond , 0 means receive forever
 */
otError proc_radio_rcv(uint8_t aChannel ,uint32_t duration);
/**
 * @fn	otError proc_radio_tx(otRadioFrame *aFrame, ble_time_t* strt_time)
 * @brief This API is used to control ral setup for transmission based on CCA procedure.
 * @param  aFrame:				pointer to the Tx frame
 * @param  strt_time:			pointer to Tx request time
 */
otError proc_radio_tx(otRadioFrame *aFrame , ble_time_t* strt_time);
/**
 * @fn	otError proc_radio_ed(otInstance *aInstance, uint8_t aScanChannel, uint16_t aScanDuration)
 * @brief This API is used to control ral setup for energy scan.
 */
otError proc_radio_ed(uint8_t aScanChannel, uint16_t aScanDuration);
/**  @ingroup systm_layer_cmn_config
*  @{
*/
/**
 * @fn	void setPANcoordinator (uint8_t aEnable)
 * @brief This API is used to set Pan coordinator role in HW to be used while applying MAC 802.15.4 filter policies.
 */
void setPANcoordinator(uint8_t aEnable);
/**
 * @fn	void setContRecp (uint8_t aEnable)
 * @brief This API is used to notify ral layer of rx on when idle state.
 */
void setContRecp(uint8_t aEnable);
/**
 * @fn	void enableScanFilters(otInstance *aInstance)
 * @brief This API is used to disable filter policy while performing scan.
 */
void enableScanFilters(otInstance *aInstance);
/**
 * @fn	void disableScanFilters(otInstance *aInstance)
 * @brief This API is used to restore filter policy after the scan is complete.
 */
void disableScanFilters(otInstance *aInstance);

/**
 * @}
 *
 */
/**  @ingroup systm_layer_cmn_proc
*  @{
*/
/**
 * @brief  get the latest  LQI value
 * @retval uint8_t. LQI value
 */
uint8_t radio_GetLQIValue(void);
/**
 * @}
 *
 */
#if RADIO_CSMA
/**  @ingroup systm_layer_cmn_config
*  @{
*/
/**
 * @fn	void set_min_csma_be(uint8_t value)
 * @brief This API is used to set min csma backoff exponent
 * @param  value: 	value for minimum csma backoff exponent
 *
 * @retval none
 */
void set_min_csma_be(uint8_t value);
/**
 * @fn	void set_max_csma_be(uint8_t value)
 * @brief This API is used to set max csma backoff exponent
 * @param  value: 	value for maximum csma backoff exponent
 *
 * @retval none
 */
void set_max_csma_be(uint8_t value);
/**
 * @fn	void set_max_csma_backoff(uint8_t value)
 * @brief This API is used to set max csma backoff counter
 * @param  value: 	value for maximum csma backoff counter
 *
 * @retval none
 */
void set_max_csma_backoff(uint8_t value);
/**
 * @fn	void set_max_full_csma_frm_retries(uint8_t value)
 * @brief This API is used to set custom max full csma frame retries
 * @param  value: 	value for maximum  full csma retrials
 *
 * @retval none
 */
void set_max_full_csma_frm_retries(uint8_t value);
#endif /*end of RADIO_CSMA*/
/**
 * @fn	void set_max_frm_retries(uint8_t value)
 * @brief This API is used to set max frame retries
 * @param  value: 	value for maximum frame retries
 *
 * @retval none
 */
void set_max_frm_retries(uint8_t value);
/**
 * @}
 *
 */
/**  @ingroup systm_layer_cmn_proc
*  @{
*/
/**
 * @fn	uint32_t mac_gen_rnd_num(uint8_t *ptr_rnd, uint8_t len, uint8_t check_cont_rx)
 * @brief This API is used to generate random number
 *
 * @param  ptr_rnd			: Pointer to the output random bytes .
 * @param  len	 			: Number of required random bytes.
 * @param  check_cont_rx 	: this flag is set to check continuous reception .
 */
uint32_t mac_gen_rnd_num(uint8_t *ptr_rnd, uint16_t len, uint8_t check_cont_rx);
/**
 * @}
 *
 */
/**  @ingroup systm_layer_enc
*  @{
*/
/**
 * @brief  This function is used to decrypt frame pointed to by ptr_pckt
 *
 * @param  ptr_pckt:				pointer to the frame to be encrypted
 * @param  ptr_session_key: 		pointer to used session key
 * @param  ptr_ccm_nonce: 		    pointer to the nonce
 * @param  mic_len: 		    	mic length
 * @param  ad_len: 		   		 	a data length
 * @param  md_len: 		    		m data length
 *
 * @retval status of the decryption process
 *
 */
uint32_t radio_decrypt_pckts( uint8_t *ptr_pckt,
		 uint8_t  *ptr_session_key,
		 uint8_t  *ptr_ccm_nonce,
		 uint32_t  mic_len,
		 uint32_t  ad_len,
		 uint32_t  md_len);
/**
 * @}
 *
 */
/**
 * @fn	void radio_mac_rx_done( otRadioFrame *aFrame, otError aError)
 * @brief This API is used to call mac Radio rx done callback after end of reception.
 * @param  aFrame:				pointer to the received frame
 * @param  aError:			reception error
 */
void radio_mac_rx_done(otRadioFrame *aFrame, otError aError);

#if (SUPPORT_ENH_ACK_LINK_METRICS_PROBING_OT_1_2)
/**
 * This method set the Link Metrics noise floor value needed to calculate the link margine
 *
 * @param[in]  noise_floor    The noise floor used by Link Metrics. It should be set to the platform's
 *                            noise floor (measured noise floor, receiver sensitivity or a constant).
 *
 */
void radio_link_metrics_set_noise_floor(int8_t noise_floor);
#endif
#if(SUPPORT_OPENTHREAD_1_2 && CONFIG_MAC_CSL_TRANSMITTER_ENABLE)
/**
 * @fn	void radio_mac_tx_done_error( otRadioFrame *aFrame, otError aError)
 * @brief This API is used to call mac Radio tx done
 * @param  tx_frame:				pointer to the Tx frame
 * @param  aError:			Tx error
 */
void radio_mac_tx_done_error(otRadioFrame * tx_frame,otError aError);
#endif /*(SUPPORT_OPENTHREAD_1_2 && CONFIG_MAC_CSL_TRANSMITTER_ENABLE)*/
#if !SUPPORT_COEXISTENCE
/*##### Random Number Generation Group #####*/
/** @ingroup rnd_gen_functions
 *  @{
 */
 /**
  * @brief  Request new random number.
  *
  * @param  ptr_rnd	: Pointer to the output random bytes .
  * @param  len	 	: Number of required random bytes.
  *
  * @retval Status.
  */
 uint32_t platform_gen_rnd_num(
 	uint8_t *ptr_rnd,
 	uint32_t len);
/**  @}
*/
/**
 *
 * @brief A common wrapper for BLE-ECB and MAC-CCM security modes
 *
 * @param	ptr_pckt		 : Pointer to the data buffer (variable length
 *  in case of CCM mode, 16 bytes in case of ECB mode). The resulting
 *  Encrypted/Decrypted data overwrites this buffer.
 * @param	ptr_key[in]		 : Pointer to the security key buffer (16 bytes).
 * @param	ptr_nonce[in]	 : Pointer to the security nonce buffer (13 bytes
 *  in case of CCM mode, a Null pointer in case of ECB mode).
 * @param	mic_len[in]		 : Length of MIC, supported values are 0, 4, 6,
 *  8, 10, 12, 14, and 16 in case of CCM, 0 only in case of ECB.
 * @param	ad_len[in]		 : Length of Data to be authenticated.
 * @param	md_len[in]		 : Length of Data to be encrypted.
 * @param	key_endian[in]	 : Represents the format of the security key.
 * @param	data_endian[in]	 : Represents the endian format of the data.
 * @param	security_mode[in]: Hardware security mode.
 * @retval Status
 */
uint32_t platform_crypto(uint8_t *ptr_pckt, const uint8_t *ptr_key,
		uint8_t *ptr_nonce, uint32_t mic_len, uint32_t ad_len, uint32_t md_len,
		crypto_endian_enum_t key_endian, crypto_endian_enum_t data_endian,
		security_mode_enum_t security_mode);

/**
 *
 * @brief radio function that will be called to run periodic calibration if tempature has changed
 *
 * @retval Nonr
 */
void radio_run_phy_clbr_on_temp_change(void);
#endif /*!SUPPORT_COEXISTENCE*/

#if (!SUPPORT_COEXISTENCE)
/**
 *
 * @brief   enable or disable LinkLayer Deep sleep mode
 *
 * @param   dp_slp_mode : @ref DEEP_SLEEP_ENABLE enable deep sleep mode ,@ref DEEP_SLEEP_DISABLE go back to sleep mode
 *
 * @retval Status .
 */
otError radio_set_dp_slp_mode(dpslp_state_e dp_slp_mode);
#endif /*end of (!SUPPORT_COEXISTENCE)*/
#if SUPPORT_MAC_PHY_CONT_TESTING_CMDS
/**
 *
 * @brief set the phy continuous modulation and continuous wave modes upon enable
 *
 * @param	type[in]		: the type of the modulation (0: modulation, 1: wave)
 * @param	enable_mode[in]	: if true then enable the selected mode otherwise disable it
 * @param	chnl_num[in]	: channel number to be used in modulation (range: 0 to 15)
 * @param   pwr[in]         : The used power in dBm.
 * @retval Status
 */
otError platform_zigbee_set_phy_cont_modul_mode(uint8_t type, uint8_t enable_mode, uint8_t chnl_num, int8_t pwr);
#endif /*end of SUPPORT_MAC_PHY_CONT_TESTING_CMDS */
#if SUPPORT_ANT_DIV
/**
 *
 * @brief set antenna diversity parameters
 *
 * @param	aInstance[in]	    : radio instance
 * @param	ptr_ant_div_params[in]	: pointer to antenna diversity params structure
 * @retval Status
 */
otError radio_set_ant_div_params(otInstance *aInstance, antenna_diversity_st* ptr_ant_div_params);

/**
 *
 * @brief get antenna diversity parameters
 *
 * @param	aInstance[in]	    : radio instance
 * @param	ptr_ant_div_params[out]	: pointer to antenna diversity params structure
 * @retval None
 */
void radio_get_ant_div_params(otInstance *aInstance, antenna_diversity_st* ptr_ant_div_params);

/**
 *
 * @brief enable/disable antenna diversity
 *
 * @param	aInstance[in]	: radio instance
 * @param	enable[in]	    : enable:1 / disable:0
 * @retval Status
 */
otError radio_set_ant_div_enable(otInstance *aInstance, uint8_t enable);

/**
 *
 * @brief set the default antenna id to be used for transmission and reception
 *
 * @param	aInstance[in]	    : radio instance
 * @param	default_ant_id[in]	: default antenna id
 * @retval Status
 */
otError radio_set_default_ant_id(otInstance *aInstance, uint8_t default_ant_id);

/**
 *
 * @brief set antenna diversity rssi threshold
 *
 * @param	aInstance[in]	    : radio instance
 * @param	rssi_threshold[in]	: rssi threshold to compare with during antenna diversity measurements
 * @retval Status
 */
otError radio_set_ant_div_rssi_threshold(otInstance *aInstance, int8_t rssi_threshold);
#endif /* SUPPORT_ANT_DIV */

#if SUPPORT_CONFIGURABLE_GAIN_FIX
/**
 * @brief  initialize rssi gain fix region and select resistor measured percentage that affects pre-emphasis sequence.
 *
 * @param[in]  region_0x1f_val: absolute gain fix for region 0x1F in dbm.
 * @param[in]  region_0x0f_val: absolute gain fix for region 0x0F in dbm.
 * @param[in]  region_0x0b_val: absolute gain fix for region 0x0B in dbm.
 * @param[in]  region_0x09_val: absolute gain fix for region 0x09 in dbm.
 * @param[in]  r_msur_percent: percentage of the measured resistor value that will be used
 * 				to select the update values in pre-emphasis sequence (range: 0 to 99).
 *
 * @retval NONE
 */
void radio_gain_fix_init(
		uint8_t region_0x1f_val, uint8_t region_0x0f_val,
		uint8_t region_0x0b_val, uint8_t region_0x09_val,
		uint8_t r_msur_percent);

#endif /* SUPPORT_CONFIGURABLE_GAIN_FIX */

#endif /* INCLUDE_PLATFORM_H_ */
/**
 * @}
 *
 */
