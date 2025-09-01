/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/bsp.h#1 $*/

/**
 ********************************************************************************
 * @file    bsp.h
 * @brief   board support package interface wrapper file.
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
#ifndef LL_BSP_H_
#define LL_BSP_H_

#include <stdint.h>
#include "_40nm_reg_files/DWC_ble154combo.h"
#define INTERRUPT_ENABLE	(1<<4)
#define SET_PRIORITY_LEVEL	2
#define PRIORITY_LEVEL_MASK 0x0F

/**
 * This macros specify the different combinations that can be used to enable or disable
 * a specific ISR
 */

#define LL_HIGH_ISR_ONLY  		0x01	// Specify only LL high ISR to be enabled or disabled
#define LL_LOW_ISR_ONLY  		0x02	// Specify only LL LOW ISR to be enabled or disabled
#define SYS_LOW_ISR		 		0x04 	// Specify only system low ISR to be enabled or disabled

#ifndef EBQ_BUILD
#define EBQ_BUILD					0
#endif


#ifndef USE_HCI_TRANSPORT
#define USE_HCI_TRANSPORT 	0
#endif
/**
 * @brief InterruptPriorities Enum.
 * it is used to define the different ISR priorities in the controller
 */
typedef enum InterruptPriorities {
	INT_PRIO_HIGHEST = 0,
	INT_PRIO_HIGH,
	INT_PRIO_LOW,
	INT_PRIO_LOWEST,
	INT_PRIO_MAX
}InterruptPriorities;

/**
 * @brief Event notification state  Enum.
 * it is used to the state of radio activity ,being started or ended
 */
typedef enum EvntNotiState {
	EVNT_START,
	EVNT_END,
	EVNT_NOT_SPECIIFED
}EvntNotiState;

/**
 * @brief periodic calibration state  Enum.
 * it is used to the state of Periodic calibration
 */
typedef enum _PhyClbrState {
	PHY_CLBR_NOT_RUNNING,
	PHY_CLBR_PNDING_OR_RUNNING,
	PHY_CLBR_NOT_KNOWN
}PhyClbrState;

/* MACRO to explicitly disable unused parameter warning */
#define LL_UNUSED(x) (void)(x)

/**
 * @brief link layer bus structure.
 * it defines the callback functions that will be be called by the transport bus driver
 * after the requested operation is done.
 */
typedef struct _ble_ll_bus {
	/** Bus read callback that will be called after the requested number of bytes is read from the bus */
	void (*read)(uint8_t *buffer);
	/** Bus Write callback that will be called after the requested is written to the bus */
	void (*write)(uint8_t *buffer);
} ble_ll_bus;

/* Structure holding the Event timing */
typedef struct Evnt_timing_s{
	uint32_t drift_time; /* The total drift time between the software timer value and the start execution of the function evnt_schdlr_timer_callback */
	uint32_t exec_time;  /* The time to get the event ready for air transmission */
	uint32_t schdling_time; /* The total time to server the completed event and start new cycle of it, the time from longest time of the state machine done isr to till the debug dio DBG_IO_PROFILE_END_DRIFT_TIME is raised */
}Evnt_timing_t;
/**
 * @brief enum holding all debugging gpio
 *
 *
 *
 */
typedef enum Debug_GPIO_e{


	DBG_IO_HCI_READ_DONE                                        ,
	DBG_IO_HCI_RCVD_CMD                                         ,
	DBG_IO_HCI_WRITE_DONE                                       ,
	DBG_IO_SCHDLR_EVNT_UPDATE                                   ,
	DBG_IO_SCHDLR_TIMER_SET                                     ,
	DBG_IO_SCHDLR_PHY_CLBR_TIMER                                ,
	DBG_IO_SCHDLR_EVNT_SKIPPED                                  ,
	DBG_IO_SCHDLR_HNDL_NXT_TRACE                                ,
	DBG_IO_ACTIVE_SCHDLR_NEAR_DETEDTED                          ,
	DBG_IO_ACTIVE_SCHDLR_NEAR_GAP_CHECK                         ,
	DBG_IO_ACTIVE_SCHDLR_NEAR_TIME_CHECK                        ,
	DBG_IO_ACTIVE_SCHDLR_NEAR_TRACE                             ,
	DBG_IO_SCHDLR_EVNT_RGSTR                                    ,
	DBG_IO_SCHDLR_ADD_CONFLICT_Q                                ,
	DBG_IO_SCHDLR_HNDL_MISSED_EVNT                              ,
	DBG_IO_SCHDLR_UNRGSTR_EVNT                                  ,
	DBG_IO_SCHDLR_EXEC_EVNT_TRACE                               ,
	DBG_IO_SCHDLR_EXEC_EVNT_PROFILE								,
	DBG_IO_SCHDLR_EXEC_EVNT_ERROR                               ,
	DBG_IO_SCHDLR_EXEC_EVNT_WINDOW_WIDENING                     ,
	DBG_IO_LLHWC_CMN_CLR_ISR                                    ,
	DBG_IO_LLWCC_CMN_HG_ISR                                     ,
	DBG_IO_LLHWC_CMN_LW_ISR                                     ,
	DBG_IO_LLHWC_CMN_CLR_TIMER_ERROR                            ,
	DBG_IO_LLHWC_LL_ISR                                         ,
	DBG_IO_LLHWC_SPLTMR_SET                                     ,
	DBG_IO_LLHWC_SPLTMR_GET                                     ,
	DBG_IO_LLHWC_LOW_ISR                                        ,
	DBG_IO_LLHWC_STOP_SCN                                       ,
	DBG_IO_LLHWC_WAIT_ENVT_ON_AIR                               ,
	DBG_IO_LLHWC_SET_CONN_EVNT_PARAM                            ,
	DBG_IO_POST_EVNT                                            ,
	DBG_IO_HNDL_ALL_EVNTS                                       ,
	DBG_IO_PROCESS_EVNT                                         ,
	DBG_IO_PROCESS_ISO_DATA                                     ,
	DBG_IO_ALLOC_TX_ISO_EMPTY_PKT                               ,
	DBG_IO_BIG_FREE_EMPTY_PKTS                                  ,
	DBG_IO_RECOMBINE_UNFRMD_DATA_OK                             ,
	DBG_IO_RECOMBINE_UNFRMD_DATA_CRC                            ,
	DBG_IO_RECOMBINE_UNFRMD_DATA_NoRX                           ,
	DBG_IO_RECOMBINE_UNFRMD_DATA_TRACE                          ,
	DBG_IO_ISO_HNDL_SDU                                         ,
	DBG_IO_LL_INTF_INIT                                         ,
	DBG_IO_DATA_TO_CNTRLR                                       ,
	DBG_IO_FREE_LL_PKT_HNDLR                                    ,
	DBG_IO_PHY_INIT_CLBR_TRACE                                  ,
	DBG_IO_PHY_RUNTIME_CLBR_TRACE                               ,
	DBG_IO_PHY_CLBR_ISR											,
	DBG_IO_PHY_INIT_CLBR_SINGLE_CH                              ,
	DBG_IO_PHY_CLBR_STRTD                                       ,
	DBG_IO_PHY_CLBR_EXEC                                        ,
	DBG_IO_RCO_STRT_STOP_RUNTIME_CLBR_ACTV                      ,
	DBG_IO_RCO_STRT_STOP_RUNTIME_RCO_CLBR                       ,
	DBG_IO_STRT_STOP_RUNTIME_RCO_CLBR_SWT                       ,
	DBG_IO_STRT_STOP_RUNTIME_RCO_CLBR_TRACE                     ,
	DBG_IO_RCO_ISR_TRACE                                        ,
	DBG_IO_RCO_ISR_COMPENDATE                                   ,
	DBG_IO_RAL_STRT_TX                                          ,
	DBG_IO_RAL_ISR_TIMER_ERROR                                  ,
	DBG_IO_RAL_ISR_TRACE                                        ,
	DBG_IO_RAL_STOP_OPRTN                                       ,
	DBG_IO_RAL_STRT_RX                                          ,
	DBG_IO_RAL_DONE_CLBK_TX                                     ,
	DBG_IO_RAL_DONE_CLBK_RX                                     ,
	DBG_IO_RAL_DONE_CLBK_ED                                     ,
	DBG_IO_RAL_ED_SCAN                                          ,
	DBG_IO_ERROR_MEM_CAP_EXCED                                  ,
	DBG_IO_ERROR_COMMAND_DISALLOWED                             ,
	DBG_IO_PTA_INIT                                             ,
	DBG_IO_PTA_EN                                               ,
	DBG_IO_LLHWC_PTA_SET_EN                                     ,
	DBG_IO_LLHWC_PTA_SET_PARAMS                                 ,
	DBG_IO_COEX_STRT_ON_IDLE                                    ,
	DBG_IO_COEX_ASK_FOR_AIR                                     ,
	DBG_IO_COEX_TIMER_EVNT_CLBK                                 ,
	DBG_IO_COEX_STRT_ONE_SHOT                                   ,
	DBG_IO_COEX_FORCE_STOP_RX                                   ,


	DBG_IO_LLHWC_ADV_DONE                                       ,
	DBG_IO_LLHWC_SCN_DONE                                       ,
    DBG_IO_LLHWC_INIT_DONE                                      ,
    DBG_IO_LLHWC_CONN_DONE                                      ,

	DBG_IO_LLHWC_CIG_DONE                                       ,
	DBG_IO_LLHWC_BIG_DONE                                       ,
	DBG_IO_OS_TMR_CREATE                                        ,
	DBG_IO_ADV_EXT_TIMEOUT_CBK                                  ,
	DBG_IO_ADV_EXT_SCN_DUR_CBK                                  ,
	DBG_IO_ADV_EXT_SCN_PERIOD_CBK                               ,
	DBG_IO_ADV_EXT_PRDC_SCN_TIMEOUT_CBK                         ,
	DBG_IO_BIS_SYNC_TIMEOUT_TMR_CBK                             ,
	DBG_IO_BIS_TERM_TMR_CBK                                     ,
	DBG_IO_BIS_TST_MODE_CBK                                     ,
	DBG_IO_BIS_TST_MODE_TMR_CBK                                 ,
	DBG_IO_ISO_POST_TMR_CBK                                     ,
	DBG_IO_ISO_TST_MODE_TMR_CBK                                 ,
	DBG_IO_CONN_POST_TMR_CBK                                    ,
	DBG_IO_EVNT_SCHDLR_TMR_CBK                                  ,
	DBG_IO_HCI_POST_TMR_CBK                                     ,
	DBG_IO_LLCP_POST_TMR_CBK                                    ,
	DBG_IO_LLHWC_ENRGY_DETECT_CBK                               ,
	DBG_IO_PRVCY_POST_TMR_CBK                                   ,

	DBG_IO_ANT_PRPR_TMR_CBK                                     ,

	DBG_IO_COEX_TMR_FRC_STOP_AIR_GRANT_CBK                      ,
	DBG_IO_MLME_RX_EN_TMR_CBK                                   ,
    DBG_IO_MLME_GNRC_TMR_CBK                                    ,
	DBG_IO_MIB_JOIN_LST_TMR_CBK                                 ,
	DBG_IO_MLME_PWR_PRES_TMR_CBK                                ,
	DBG_IO_PRESISTENCE_TMR_CBK                                  ,
	DBG_IO_RADIO_PHY_PRDC_CLBK_TMR_CBK                          ,
	DBG_IO_RADIO_CSMA_TMR_CBK                                   ,
	DBG_IO_RADIO_CSL_RCV_TMR_CBK                                ,
	DBG_IO_ED_TMR_CBK                                           ,
	DBG_IO_DIO_EXT_TMR_CBK                                      ,
	DBG_IO_RCO_CLBR_TMR_CBK
	                                                            ,
	DBG_IO_ADV_EXT_MNGR_ADV_CBK                                 ,
	DBG_IO_ADV_EXT_MNGR_SCN_CBK                                 ,
	DBG_IO_ADV_EXT_MNGR_SCN_ERR_CBK                             ,
    DBG_IO_ADV_EXT_MNGR_PRDC_SCN_CBK                                ,
	DBG_IO_ADV_EXT_MNGR_PRDC_SCN_ERR_CBK                        ,
	DBG_IO_BIG_ADV_CBK                                          ,
	DBG_IO_BIG_ADV_ERR_CBK                                      ,
	DBG_IO_BIG_SYNC_CBK                   	                    ,
	DBG_IO_BIG_SYNC_ERR_CBK                                     ,
	DBG_IO_ISO_CIS_PKT_TRNSM_RECEIVED_CBK                       ,
	DBG_IO_ISO_CIG_ERR_CBK                                      ,
	DBG_IO_CONN_PKT_TRNSM_RECEIVED_CBK                          ,
	DBG_IO_PRDC_CLBR_EXTRL_CBK                                  ,
	DBG_IO_PTR_PRDC_ADV_SYNC_CBK                                ,
	DBG_IO_NCONN_SCN_CBK                                        ,
	DBG_IO_NCONN_ADV_CBK                                        ,
	DBG_IO_NCONN_INIT_CBK                                       ,
	DBG_IO_ANT_RADIO_CMPLT_EVNT_CBK                             ,
	DBG_IO_ANT_STACK_EVNT_CBK                                   ,
	DBG_IO_ADV_EXT_PROCESS_TMOUT_EVNT_CBK                       ,
	DBG_IO_ADV_EXT_MNGR_SCN_DUR_EVNT                            ,
	DBG_IO_ADV_EXT_MNGR_SCN_PERIODIC_EVNT                       ,
	DBG_IO_ADV_EXT_MNGR_PRDC_SCN_TMOUT_EVNT                     ,
	DBG_IO_ADV_EXT_MNGR_PRDC_SCN_CNCEL_EVNT                     ,
	DBG_IO_BIS_MNGR_BIG_TERM_CBK                                ,
	DBG_IO_BIS_MNGR_SYNC_TMOUT_CBK                              ,
	DBG_IO_ISOAL_MNGR_SDU_GEN                                   ,
	DBG_IO_ISO_MNGR_CIS_PROCESS_EVNT_CBK                        ,

	DBG_IO_CONN_MNGR_PROCESS_EVNT_CLBK                          ,
	DBG_IO_CONN_MNGR_UPDT_CONN_PARAM_CBK                        ,
	DBG_IO_CONN_MNGR_DATA_LEN_UPDT_CBK							,
	DBG_IO_EVNT_SCHDLR_HW_EVNT_CMPLT                            ,

	DBG_IO_HCI_EVENT_HNDLR                                      ,

	DBG_IO_MLME_TMRS_CBK                                        ,
	DBG_IO_DIRECT_TX_EVNT_CBK                                   ,
	DBG_IO_INDIRECT_PKT_TOUR_CBK                                ,
	DBG_IO_RADIO_CSMA_TMR                                       ,
	DBG_IO_RAL_SM_DONE_EVNT_CBK                                 ,
	DBG_IO_ED_TMR_HNDL                                          ,
	DBG_IO_OS_TMR_EVNT_CBK                                      ,
	DBG_IO_PROFILE_MARKER_PHY_WAKEUP_TIME                       ,
	DBG_IO_PROFILE_MARKER_BLOCKING_PHY_WAKEUP_TIME              ,
	DBG_IO_PROFILE_END_DRIFT_TIME                               ,
	DBG_IO_PROC_RADIO_RCV										,
	DBG_IO_EVNT_TIME_UPDT										,
	DBG_IO_MAC_RECEIVE_DONE										,
	DBG_IO_MAC_TX_DONE											,
	DBG_IO_RADIO_APPLY_CSMA										,
	DBG_IO_RADIO_TRANSMIT										,
	DBG_IO_PROC_RADIO_TX										,
	DBG_IO_RAL_TX_DONE											,
	DBG_IO_RAL_TX_DONE_INCREMENT_BACKOFF_COUNT					,
	DBG_IO_RAL_TX_DONE_RST_BACKOFF_COUNT						,
	DBG_IO_RAL_CONTINUE_RX										,
	DBG_IO_RAL_PERFORM_CCA										,
	DBG_IO_RAL_ENABLE_TRANSMITTER								,
	DBG_IO_LLHWC_GET_CH_IDX_ALGO_2 ,
	DBG_IO_ADV_EXT_MNGR_PAWR_ADV_SE_CBK                         ,
	DBG_IO_ADV_EXT_MNGR_PAWR_ADV_SE_ERR_CBK                     ,
	DBG_IO_ADV_EXT_MNGR_PAWR_SCN_ERR_CBK                        ,

	DBG_IO_LLHWC_SET_ADV_PAWR_SE_PARAM							,
	DBG_IO_LLHWC_ADV_PAWR_SE_DONE								,
	DBG_IO_LLHWC_SET_PAWR_RSP_PARAM								,
	DBG_IO_LLHWC_ADV_PAWR_RSP_DONE								,
	DBG_IO_LLHWC_ADV_PAWR_RSP_DONE_RCV_PCKT						,
	DBG_IO_LLHWC_ADV_PAWR_RSP_DONE_FREE_PCKT					,
	DBG_IO_LLHWC_PAWR_PING_PONG_HNDL							,
	DBG_IO_LLHWC_PAWR_PING_PONG_RCV_PCKT						,
	DBG_IO_LLHWC_PAWR_ADV_STOP_RSPS								,
	DBG_IO_LLHWC_PAWR_SYNC_SET_PARAM							,
	DBG_IO_LLHWC_PAWR_SYNC_DONE									,
	DBG_IO_LLHWC_PAWR_SYNC_SEND_RSP								,

	DBG_IO_PAWR_ADV_SE_MISS_RSP									,
	DBG_IO_PAWR_ADV_FORCE_RP									,
	DBG_IO_PAWR_ADV_PUSH_STRT_TIM_FORW							,
	DBG_IO_PAWR_ADV_RSP_NEAR									,
	DBG_IO_EVNT_STRT_TIM_PUSHED									,
	DBG_IO_PAWR_ADV_RSP_SWITCH_SE								,
	DBG_IO_PAWR_ADV_QUEUE_WIN_UPDT								,
	DBG_IO_PAWR_SYNC_REFUSE_INST_RSP							,
	DBG_IO_PAWR_SYNC_ABOUT_TIMEOUT								,
	DBG_IO_PAWR_SYNC_INST_RSP_TOO_LATE							,
	DBG_IO_PAWR_SYNC_EXEC_SKIPPED								,
	DBG_IO_NULL_PKT_STATUS										,
	DBG_IO_PAWR_MULTIPLE_EVNTS_MISSED							,
	DBG_IO_PAWR_CHM_UPDT_END									,
	DBG_IO_LLHWC_CMN_INIT                                       ,
	DBG_IO_RADIO_SET_PENDING_TX_FULL							,
	DBG_IO_RADIO_SET_PENDING_TX_CONTINUE						,
	DBG_IO_RADIO_HANDLE_PENDING_TX								,
	DBG_IO_RAL_AD_SET_MEASUREMENT_STATE							,
    DBG_IO_PROFILE_CS_GEN                                       ,
    DBG_IO_PROFILE_CS_CHNL_SHUFFLING                            ,
	Debug_GPIO_num

}Debug_GPIO_t;



/**
*  @ingroup BSP_APIS
*  @{
*/
/**
 * @brief   Bus initialization Function
 *
 * 	this function is used to initialize the used transport bus and link functions is @ref _ble_ll_bus to the ISRs of bus driver
 * @param  op[in]   : pointer to @ref _ble_ll_bus structure that stores the bus callback functions
 * @retval None
 *
 */
void bus_init(ble_ll_bus * op);

/**
 * @brief   Bus read Function
 * This function is used read @ref size bytes from the Bus in the given @ref buffer
 * @note this function is asynchronous, it is expected that Bus read callback function is called to indicate that the requested  data is available
 * the read callback function will be set by calling @ref bus_init
 * @param   *buffer [in]: pointer to the buffer of the data
 * @param   size [in]: size of bytes read from the buffer
 *
 * @retval None
 */
void bus_read(uint8_t *buffer, uint16_t size);

/**
 * @brief   Bus write Function
 * This function is used read @ref size bytes from the Bus in the given @ref buffer
 * @note this function is asynchronous, it is expected that Bus will call read callback function when wrriting data is done
 * the Write callback function will be set by calling @ref bus_init
 * @param   *buffer[in]: pointer to the buffer of the data
 * @param   size[in]: size of bytes written to the buffer
 *
 * @retval None
 */
void bus_write(uint8_t *buffer, uint16_t size);

/**
 * @brief   logger port initialization
 *
 *this function is used to initalizer the logger
 * @param  None
 * @retval None
 *
 */
void logger_init(void);

/**
 * @brief   logger write
 * Thin function is used to log the data described by the input parameters
 * @param   *buffer: pointer to the buffer of the data to be written to logger
 * @param   size: size of bytes to be logged from the buffer
 *
 * @retval None
 */
void logger_write(uint8_t *buffer, uint32_t size);

/**
 * @brief   enable interrupt request function
 * This function enable the MCU interrupt ,after calling this function the LL code can be interrupted by the controller
 * @param   None
 *
 * @retval None
 */
extern void enable_irq(
			void
);
/**
 * @brief   disable interrupt request function
 * This function disable the MCU interrupt ,after calling this function the LL code must not be interrupted as it is in critical section
 * @param   None
 *
 * @retval None
 */
extern void disable_irq(
			void
);
/**
 * @brief this function is used to enable a specific ISR
 * @param[in]  isr_type that holds specific ISR to be enabled by this function
 * 				BIT[0] for LL_HIGH_ISR
 * 				BIT[1] for LL_LOW_ISR
 * 				BIT[2] for SYS_LOW_ISR
 * @return None
 */
void enable_specific_irq(uint8_t isr_type );


/**
 * @brief this function is used to disable a specific ISR
 * @param[in]  isr_type that holds specific ISR to be disabled by this function
 * 				BIT[0] for LL_HIGH_ISR
 * 				BIT[1] for LL_LOW_ISR
 * 				BIT[2] for SYS_LOW_ISR
 * @return None
 */
void disable_specific_irq(uint8_t isr_type );

/**
 * @brief   broad  initialization Function
 *
 * 	this function is used to initialize the used MCU
 * @param  op[in]   : pointer to @ref _ble_ll_bus structure that stores the bus callback functions
 * @retval 0 if SUCCESS
 * otherwise Not SUCESS
 *
 */
int bsp_init(void);
/**
 * @brief   dealy us  Function
 *
 * 	this function is  microsecond delay function
 * @param  op[in]   : Nunmber of microseconds that function this function execution should take
 * @retval  None
 *
 */
void bsp_delay_us(uint32_t delay);

/**
 * @brief   interrupt enable  Function
 *
 * 	this function is used to enabled and register ISR for the given interrupt line
 * @param  intrNum[in]   : number to the interrupt line to be enabled
 * @param  intr_cb[in]   : pionter to ISR function the will be called when this interupt is fired
 * @retval  None
 *
 */
int bsp_intr_enable (uint32_t intrNum, void (*intr_cb)(void));
/**
 * @brief   interrupt set pri  Function
 *
 * 	this function is used to set the interrupt priority and register ISR for the given interrupt line
 * @param  intrNum[in]   : number to the configured interrupt line
 * @param  intr_cb[in]   : pionter to ISR function the will be called when this interupt is fired
 * @param  intpri[in]    : the priority oto used for the given interrupt
 * @retval  None
 *
 */
int bsp_intr_set_pri(uint32_t intrNum, void (*intr_cb)(void), int32_t intpri);



/* */
/**
 *
 * @brief  is in LL BLE memory  function
 * it Checks whether the given pointer in BLE Memory or outside it  if the pointer points to a location in BLE memory
 * @param   ptr [in] pointer to check if in BLE memory or not
 *
 * @retval 1   the given pointer is in LL BLE memory
 * @retval 0   the given pointer is not in LL BLE memory
 */
int bsp_is_ptr_in_ble_mem(void* ptr);


/**
 * @brief  make the mcu sleep in a certain power mode according to its idle time.
 *
 * @param  None.
 *
 * @retval None.
 */
void bsp_mcu_slp(void);


/**
 * @brief	Clear GPIO pin output value
 *
 * @param	enable: enable/disable flag
 *
 * @retval None.
 */

void bsp_gpio_clear(uint8_t gpio_num);


/**
 * @brief	Set GPIO pin output value
 *
 * @param	enable: enable/disable flag
 *
 * @retval None.
 */
void bsp_gpio_set(uint8_t gpio_num);


/**
 * @brief	Enables/Disables the bus clock.
 *
 * @param	enable: enable/disable flag
 *
 * @retval None.
 */
void bsp_control_hclk(uint8_t enable);

/**
 * @brief	Enables/Disables the active clock.
 *
 * @param	enable: enable/disable flag
 *
 * @retval None.
 */
void bsp_control_aclk(uint8_t enable);


/**
 * @brief	Notification that LL FW will start or end a radio activity .
 *
 * @param	enable: EVNT_START , radio activity started
 * 				  : Evnt_END     Radio event completed
  *
 * @retval None.
 */
void bsp_evnt_not(EvntNotiState enable);

/**
 * @brief	Notification that LL FW will start or end RCO Calibration .
 *
 *@note this is an optional wrapper that used to inform the upper layer of the state of RCO calibration.
 *@note the upper layer may ignore this wrapper
 * @param	enable: EVNT_START , RCO calibration will be started
 * 				  : Evnt_END     RCO calibration has  completed
  *
 * @retval None.
 */
void bsp_rco_clbr_not(EvntNotiState enable);

/**
 * @brief used to assert/trigger the low priority interrupt from the SW.
 *
 * @param priority: if 1 then this SW ISR should be treated as if it was High priority HW ISR
 *
 * @retval None.
 */
void bsp_switch_to_lw_isr(uint8_t priority);

/**
 * @brief  wait for bus clock ready signal
 *
 * A platform that has more accurate information about the readiness of the bus clock should implement this function to
 * avoid redundant delay while reading sleep timer .
 *
 * @note this function will be called only if @ref USE_SOC_ACCURATE_BUS_CLK_READY_API is set to one otherwise LL FW will wait for a change in sleep timer reading.
 *
 * @param None.
 *
 * @retval None.
 */
void bsp_wait_for_busclkrdy(void);
/**
 * @brief used to start temperature calculations
 * if the upper layer has informed the link layer by the existence of temperature sensor through @ref ll_intf_set_temperature_sensor_stat() or llhwc_cmn_set_temperature_sensor_stat()  .
 * New temperature will be requested in the following cases:
 * 1- at initialization when the @ref ll_intf_set_temperature_sensor_state or llhwc_cmn_set_temperature_sensor_stat is called
 * 2- before any radio event
 * Once temperature is ready the upper layer should call @ref ll_intf_set_temperature_value() or llhwc_cmn_set_temperature_value() to inform Link Layer with new Temperature value
 * @retval None.
 */
void bsp_request_temperature(void);

/**
 * @brief a function to set a certain gpio pin.
 *
 * @param gpio[in]: one of the gpios defined in Debug_GPIO_t enum to be set
 *
 * @retval None.
 *
 * @note : some of the signals can be mapped to physical hardware and some may not be connected to a physical GPIO based in availability.
 */
void bsp_debug_gpio_set(Debug_GPIO_t gpio);

/**
 * @brief a function to clear a certain gpio pin.
 *
 * @param gpio[in]: one of the gpios defined in Debug_GPIO_t enum to be cleared
 *
 * @retval None.
 *
 * @note : some of the signals can be mapped to physical hardware and some may not be connected to a physical GPIO based in availability.
 */
void bsp_debug_gpio_clear(Debug_GPIO_t gpio);

/**
 * @brief a function to toggle a certain gpio pin.
 *
 * @param gpio[in]: one of the gpios defined in Debug_GPIO_t enum to be toggled
 *
 * @retval None.
 *
 * @note : some of the signals can be mapped to physical hardware and some may not be connected to a physical GPIO based in availability.
 */
void bsp_debug_gpio_toggle(Debug_GPIO_t gpio);

/**
 * @brief a function to inform the upper layer by state of periodic calibration state.
 *
 * @param state[in]: Value from @ref PhyClbrState, NO_PHY_CLBR_NEEDED  when the calibration is completed , PHY_CLBR_PNDING_OR_RUNNING when calibration is started or about to start from background task
 *
 * @retval None.
 */
void bsp_set_phy_clbr_state(PhyClbrState state);

/**
 * @brief a function to notify the upper layer to switch the clock.
 *
 * @param evnt_timing[in]: Evnt_timing_t pointer to structure contains drift time , execution time and scheduling time
 *
 * @retval None.
 */
void bsp_evnt_schldr_timing_update_not(Evnt_timing_t * p_evnt_timing);
/**
 * @}
 */

int logCons(void* devHandle, char* logStr);
void* logFileInit(const char * fileName);
int logFileClose(void* fileHandle);
int logFile(void* devHandle, char* logStr);
int logUart(void* devHandle, char* logStr);


void bsp_assert_log(uint8_t condition, uint8_t severity, const char *ptr_func_name,  const int line);
void bsp_assert(uint8_t condition, uint8_t severity);



#endif /* LL_BSP_H_ */
