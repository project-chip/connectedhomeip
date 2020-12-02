/*! *********************************************************************************
 * \defgroup CONTROLLER - Controller Interface
 * @{
 ********************************************************************************** */
/*!
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2017 NXP
*
* \file
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of Freescale Semiconductor, Inc. nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _CONTROLLER_INTERFACE_H_
#define _CONTROLLER_INTERFACE_H_

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "PWR_Interface.h"
#include "fsl_xcvr.h"
#include "fsl_os_abstraction.h"
#include "Panic.h"

/************************************************************************************
*************************************************************************************
* Structures/Data Types
*************************************************************************************
************************************************************************************/

///HCI Debug user extended function command parameters - vendor specific
struct hci_user_ext_func_cmd;

/// Description of unloaded RAM area content
struct unloaded_area_tag
{
    // status error
    uint32_t error;
};

struct app_cfg
{
    // Platform
    struct unloaded_area_tag* unloaded_area;
    void (*plf_reset_cb)(void);
    void (*plf_panic_cb)( panicId_t id, uint32_t location, uint32_t extra1, uint32_t extra2 );
    void (*get_ble_mac_addr)(uint8_t* aOutUid16B, uint8_t* pOutLen);
    int32_t (*get_temperature)(void);

    // HCI extend function
    uint8_t (*hci_user_ext_func)(struct hci_user_ext_func_cmd * params);
    int (*hci_common_callback)(uint16_t const msgid, void const *param, uint16_t const dest_id, uint16_t const src_id);

    // Callback for Controller packet statistics
    void (*app_pkt_statistic)(uint16_t rx_st, uint16_t conhdl);

    // Debug and Diagnostics
    uint8_t IoDbgMode;
    void (*IoSet)(int pinid, int val);
    void (*DiagEna)(void);

    // Callback to indicate ble inactivity duration
    void (*bleInactivityCallback)(uint32_t time);
    // Callback to signal new activity time
    int (*bleNewActivityCallback)(uint32_t time);
    // Callback to signal link layer wake up ended
    void (*bleWakeupEndCallback)(void);
    // Callback to get the slave procol state
    bool_t (*bleIsDynSlaveProtocolActive)(void);

    int (*prepareOperation)(uint8_t operationType);
    int (*processOperation)();
    int (*stopOperation)();
};

struct fw_cfg
{
    // BLE controller configuration
    uint8_t   ble_con_max;
    uint8_t   ble_whitelist_max;
    uint8_t   ble_resol_addr_list_max;
    uint8_t   ble_duplicate_filter_max;
    uint8_t   prog_latency_def;
    uint16_t  lld_util_min_instant_con_evt;
    uint16_t  min_instant_con_evt;
    uint16_t  adv_pdu_int;
    uint8_t*  llc_state;

    // BLE controller heap configuration
    uint32_t* rwip_heap_env;
    uint32_t* rwip_heap_msg;
    uint32_t* rwip_heap_non_ret;
    uint16_t  rwip_heap_env_size;
    uint16_t  rwip_heap_msg_size;
    uint16_t  rwip_heap_non_ret_size;

    // BLE controller Exchange memory configuration
    uint32_t em_ble_start;
    uint32_t em_ble_wpb_offset;
    uint32_t em_ble_wpv_offset;
    uint32_t em_ble_ral_offset;
    uint32_t em_ble_tx_desc_offset;
    uint32_t em_ble_rx_desc_offset;
    uint32_t em_ble_tx_buffer_ctrl_offset;
    uint32_t em_ble_tx_buffer_data_offset;
    uint32_t em_ble_tx_buf_data_cnt;
    uint32_t em_ble_rx_buffer_offset;
    uint32_t em_ble_rx_buffer_size;
    uint32_t em_ble_rx_buffer_cnt;
    uint32_t em_ble_end;

    // Interrupt mask
    uint32_t int_mask0;
    uint32_t int_mask1;

    // Controller behavior with invalid pdu
    uint32_t invalid_pdu_handling;

};

/* low power configuration for sleep and 32k clock housekeeping */
struct lp_cfg
{
    uint32_t wakeup_delay;         /*!< Number of 32k clock ticks */
    uint32_t wakeup_advance;       /*!< Number of 32k clock ticks */
    uint8_t  timebase_compensate;  /*!< Duration in microseconds used to offset computation */
    bool_t   sw_32k_calib_enable;  /*!< true: software calibrates 32k clock, 32k RCO has to use this mode.
                                        false: do not use software to calibrate 32k clock, 32k XTAL usually uses this mode. */
    // background sleep timer setting
    uint32_t bg_sleep_duration_external;
    uint32_t bg_sleep_duration_periodic;
};

struct xcvr_api
{
    xcvrStatus_t (*xcvr_wake_up_init)(void);
    int16_t (*xcvr_read_rssi)(data_rate_t rate);
    uint32_t (*xcvr_get_recal_duration)(void);
    void (*xcvr_recalibrate)(void);
    void (*xcvr_temperature_update)(int32_t temperature);
    void (*xcvr_lockup_check_and_abort_radio)(void);
    void (*xcvr_disable_ble_fast_tx)(void);
    void (*xcvr_enable_ble_fast_tx)(bool_t keepPll);
    int8_t (*xcvr_set_tx_pwr)(int8_t tx_pwr);
    void (*xcvr_register_rf_activity_cb)(void*);
};

struct coex_api
{
    uint32_t (*coex_register)(void *callback);
    uint32_t (*coex_request_access)(uint32_t newState);
    uint32_t (*coex_change_access)(uint32_t newState);
    void     (*coex_release_access)(void);
    void     (*coex_enable)(void);
    void     (*coex_disable)(void);
};



struct pdm_api
{
    uint32_t (*get_local_name)(uint8_t * name);
    uint32_t (*set_local_name)(const uint8_t * name);
    uint32_t (*get_class_of_device)(uint8_t * aclass);
    uint32_t (*set_class_of_device)(const uint8_t * aclass);
};

struct fwk_cfg
{
    // Low power configuration
    const struct lp_cfg lp_cfg;

    // XCVR API
    const struct xcvr_api xcvr_api;

    // MWS API
    const struct coex_api coex_api;

    const bool_t controllerUsePdm;
    const struct pdm_api  pdm_api;
};

struct dyn_cfg
{
    // Low power
    PWR_clock_32k_hk_t* lp_dyn;

    // Flags
    volatile uint8_t flags;

    // Seed for firmware's srand()
    uint32_t fw_seed;
};

struct ble_config_st
{
    const struct app_cfg *app;
    const struct fw_cfg  *fw;
    const struct fwk_cfg *fwk;
    struct dyn_cfg       *dyn;
};

/*! @brief Power modes. */
typedef enum _power_mode
{
    kPmActive,     /*!< CPU is executing */
    kPmSleep,      /*!< CPU clock is gated */
    kPmPowerDown0, /*!< Power is shut down except for always on domain, 32k clock and selected wakeup source */
    kPmPowerDown1  /*!< Power is shut down except for always on domain and selected wakeup source */
} power_mode_t;


/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void Controller_TaskEventInit(osaEventId_t event_id, const uint8_t use_rtos);
void Controller_TaskHandler(void* handler);

/*
 * @brief Send deep sleep command to controller
 * @return: the sleep mode the controller will enter
 */
power_mode_t BLE_sleep(void);

/*
 * @brief Get the sleep mode allowed by BLE Controller
 *        No sleep command is executed.
 * @return: the sleep mode the controller will enter
 */
power_mode_t BLE_get_sleep_mode(void);

/*
 * @brief Informs the controller deep sleep is not yet permitted as wake-up is not finished.
 * The BLE controller will allow deep sleep once wake-up is finished.
 */
void BLE_prevent_sleep_on_wakeup(void);

/*
 * @brief Informs the controller deep sleep is permitted.
 */
void BLE_enable_sleep(void);

/*
 * @brief Informs the controller deep sleep is NOT permitted.
 */
void BLE_disable_sleep(void);

/*
 * @brief Enable the hardware WiFi coexistence mechanism in BLE controller.
 */
void BLE_EnableWlanCoex(void);

/*
 * @brief Save Link layer registers before power modes.
 */
void BLE_SaveRegisters(void);

/*
 * @brief Restore Link layer registers After power modes.
 */
void BLE_RestoreRegisters(void);

/*
 * @brief Configure Controller with application parameters
 */
void BLE_ControllerConfig(struct ble_config_st *cfg);

/*
 * @brief Get time in us before next BLE event
 */
uint32_t BLE_TimeBeforeNextBleEvent(void);

/************************************************************************************
 ************************************************************************************
 *                  Common header section, for all platforms                        *
 ************************************************************************************
 ***********************************************************************************/

typedef bleResult_t (*gHostHciRecvCallback_t)
(
    hciPacketType_t packetType,
    void* pHciPacket,
    uint16_t hciPacketLength
);

/*! *********************************************************************************
 * \brief  Performs initialization of the Controller.
 * \param[in]  callback HCI Host Receive Callback
 *
 * \return osaStatus_Success or osaStatus_Error
 ********************************************************************************** */
osaStatus_t Controller_Init(gHostHciRecvCallback_t callback);

/*! *********************************************************************************
 * \brief  Controller Receive Interface
 * \param[in]  packetType HCI packet Type
 * \param[in]  pPacket    data buffer      
 * \param[in]  packetSize data buffer length 
 *
 * \return gBleSuccess_c, gBleOutOfMemory_c or gBleInvalidParameter_c
 ********************************************************************************** */
bleResult_t Hci_SendPacketToController( hciPacketType_t packetType, void* pPacket,
                                        uint16_t packetSize);
                                        
#ifdef __cplusplus
}
#endif

#endif /* _CONTROLLER_INTERFACE_H_ */

/*! *********************************************************************************
* @}
********************************************************************************** */

