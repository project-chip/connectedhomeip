/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/ll_intf_cmn.h#1 $*/
/**
 ********************************************************************************
 * @file    ll_intf_cmn.h
 * @brief   This file includes declaration of common interfaces of MAC only and BLE/COEXISTENCE APIs.
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
#ifndef INCLUDE_LL_INTF_CMN_H_
#define INCLUDE_LL_INTF_CMN_H_
#include "common_types.h"
/*========================================================================================================*/
/*================================  Configure LL Context Control Command =================================*/
/*========================================================================================================*/
/** @ingroup  ll_common_interface_functions
 *  @{
 */
/**
 * @brief Used to configure the LL contexts, where:
 * 			1. For bare-metal:
 * 			- High ISR is executed in the ISR context
 * 			- Low ISR can be executed in the high ISR context, or switched to low ISR context
 * 			2. For RTOS:
 * 			- High ISR is executed in the ISR context
 * 			- Low ISR is executed in the thread of the "linkLayerHighPrioTask"
 *
 * @param allow_low_isr : [in] Configuration parameter for the context of the low ISR in the bare-metal model. Range is [0,1].
 * 								0: Low ISR code is executed in the same context of the high ISR.
 * 								1: Low ISR code is executed in the context of the low ISR (by configuring a low priority interrupt that is triggered by FW).
 * @param run_post_evnt_frm_isr : [in] Configuration parameter to decide whether the scheduling of the next BLE event is done in the low ISR context or to be handled by the LL main thread. Range is [0,1].
 * 								0: BLE next event scheduling is handled in the LL main thread.
 * 								1: BLE next event scheduling is handled in the low ISR context.
 *
 * @retval ble_state_t : Command status
 */
ble_stat_t ll_intf_cmn_config_ll_ctx_params(uint8_t allow_low_isr, uint8_t run_post_evnt_frm_isr);
/*========================================================================================================*/
/*==================================  LE Select Sleep Clock Source =======================================*/
/*========================================================================================================*/
#if (USE_NON_ACCURATE_32K_SLEEP_CLK)
/**
 * @brief Used to select the source that drives the sleep clock, whether to use an external crystal oscillator or an integrated RC oscillator (RCO).
 *
 * @param  slp_clk_src 				: [in] Indicate which source to drive the sleep clock. 0: Crystal Oscillator (default). 1: RC0
 * @param  ptr_slp_clk_freq_value 	: [out] Indicate the nominal frequency value of the sleep clock.
 *
 * @retval ble_stat_t : Command status to be sent to the Host.
 */
ble_stat_t ll_intf_cmn_le_select_slp_clk_src(uint8_t slp_clk_src, uint16_t *ptr_slp_clk_freq_value);
/*========================================================================================================*/
/*===============================  LE Set RCO Calibration Event Parameters ===============================*/
/*========================================================================================================*/
/**
 * @brief Used to configure the runtime RCO calibration event parameters.
 *
 * @param  rco_clbr_event_duration : [in] Indicate the number of sleep clock cycles for performing the RCO calibration process.
 * @param  rco_clbr_event_interval : [in] Indicate the periodicity of running the runtime RCO calibration event.
 *
 * @retval None.
 */
ble_stat_t ll_intf_cmn_le_set_rco_clbr_evnt_params(uint8_t rco_clbr_event_duration, uint32_t rco_clbr_event_interval);
#endif
/*========================================================================================================*/
/*======================================  LE Select TX_Power Table =======================================*/
/*========================================================================================================*/
/**
 * @brief Used to specify the used power table and its size based on the selected TX_Power table ID.
 *
 * @param  tx_power_table_id : [in] Selected TX_Power table ID.
 *
 * @retval Status 		 : 0: SUCCESS. Otherwise: Error code.
 */
uint8_t ll_intf_cmn_select_tx_power_table(uint8_t tx_power_table_id);
/**
 * @brief flag to the LL the existence of a temperature sensor
 * @retval None
 */
void ll_intf_cmn_set_temperature_sensor_state(void);
/**
 * @brief set the current temperature
 * @param temperature		:	The current temperature
 * @retval None
 */
uint32_t  ll_intf_cmn_set_temperature_value(uint32_t temperature);
/*========================================================================================================*/
/*====================================  Random Number Generation Group ===================================*/
/*========================================================================================================*/
 /**
  * @brief  Request new random number.
  *
  * @param  ptr_rnd	: Pointer to the output random bytes .
  * @param  len	 	: Number of required random bytes.
  *
  * @retval Status.
  */
uint32_t ll_intf_cmn_gen_rnd_num(uint8_t *ptr_rnd, uint32_t len);
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
uint32_t ll_intf_cmn_crypto(uint8_t *ptr_pckt, const uint8_t *ptr_key,
		uint8_t *ptr_nonce, uint32_t mic_len, uint32_t ad_len, uint32_t md_len,
		crypto_endian_enum_t key_endian, crypto_endian_enum_t data_endian,
		security_mode_enum_t security_mode);
/*===============  LL Common Interface LE Set Deep Sleep Mode  ===============*/
/**
 * @brief Switches the controller to and from Deep Sleep mode.
 * @param [in]  dp_slp_mode: Input according to dpslp_state_e.
 *                               DEEP_SLEEP_ENABLE  -> Activate Deep Sleep.
 *                               DEEP_SLEEP_DISABLE -> Deactivate Deep Sleep.
 * @retval Status.
 */
ble_stat_t ll_intf_cmn_le_set_dp_slp_mode(uint8_t dp_slp_mode);
#if SUPPORT_CONFIGURABLE_GAIN_FIX
/**
 * @brief  initialize rssi gain fix region and select resistor measured
 *			percentage that affects pre-emphasis sequence.
 *
 * @param  region_0x1f_val: absolute gain fix for region 0x1F in dbm.
 * @param  region_0x0f_val: absolute gain fix for region 0x0F in dbm.
 * @param  region_0x0b_val: absolute gain fix for region 0x0B in dbm.
 * @param  region_0x09_val: absolute gain fix for region 0x09 in dbm.
 * @param  r_msur_percent: percentage of the measured resistor value that will be used
 * 				to select the update values in pre-emphasis sequence (range: 0 to 99).
 *
 * @retval None.
 */
void ll_intf_cmn_gain_fix_init(
		uint8_t region_0x1f_val, uint8_t region_0x0f_val,
		uint8_t region_0x0b_val, uint8_t region_0x09_val,
		uint8_t r_msur_percent);
#endif /* SUPPORT_CONFIGURABLE_GAIN_FIX */
/** @}
*/

#endif /* INCLUDE_LL_INTF_CMN_H_ */
