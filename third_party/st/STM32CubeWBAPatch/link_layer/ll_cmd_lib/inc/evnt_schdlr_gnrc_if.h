/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/evnt_schdlr_gnrc_if.h#1 $*/
/**
 ********************************************************************************
 * @file    evnt_schdlr_gnrc_if.h
 * @brief   This file contains all the functions prototypes for the evnt_schdlr.c
 * that are used by other layers to interface with the scheduler.
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

 /**
 *  @ingroup gnrc_schdlr_intf
 *  @brief Provides APIs to register an external event in the system, all operations for utilizing the generic interface are part of the APIs in evnt_schdlr_gnrc_if.h header file.
 *  *  @{
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INCLUDE_EVNT_GNRC_SCHDLR_IF_H_
#define INCLUDE_EVNT_GNRC_SCHDLR_IF_H_


/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#include "bsp.h"
#include "os_wrapper.h"
#include "common_types.h"

typedef void* ext_evnt_hndl_t;


/**
  * @brief Generic External Event node that holds the the external event information.
  * @note All parameters that are related to time assumed to be in microseconds
  */
typedef struct _extrnl_evnt_st_t{
	/** Event must end before this point in time (us)  it is  an optional parameter and
	 * it should set with zero if not used by stack.*/
	uint64_t 					deadline;

	/** Earliest time the event can start (us) , it is  an optional parameter and
	 * it should set with zero if not used by stack.it is not logical to have periodic event with max start time.
	 *  So, it will be neglected and never checked in case of periodic event.*/
	uint64_t 					strt_min;
	/** Latest time the event can start (us), it is  an optional parameter and
	 * it should set with zero if not used by stack.*/
	uint64_t 					strt_max;
	/** Minimum amount of time that must be allocated to the event (us) , */
	uint32_t 					durn_min;
	/** Maximum amount of time that the event is requesting (us), it is  an optional parameter and
	 * it should set with zero if not used by stack.*/
	uint32_t 					durn_max;
	/** Periodicity of the event (us), 0: Not Periodic
	 * if the value is not zero , it should be multiple of 1250 us to be aligned with BLE events.*/
	uint32_t 					prdc_intrvl;
	/** Priority of the event  from @ref _extrnl_evnt_priority_e*/
	extrnl_evnt_priority_e 		priority;
	/** Event blocked or not, and reason if blocked from @ref _extrnl_evnt_state_e */
	extrnl_evnt_state_e			blocked;
	/** Pointer to private data that should passed with the call back function */
	void* 						ptr_priv;
	/** Event Started Callback Function , this function will be called at the allocated time to do the job related to
	 *  executing the event, it must not be NULL.
	 *  At the end of given grant, @ref evnt_schdlr_gnrc_evnt_cmplt function should be called to inform scheduler
	 *  that the event is finished, and PHY should not be accessed after that call until a new grant is given
	 *   from event scheduler. @ref evnt_strtd_cbk should handle calling of @ref  evnt_schdlr_gnrc_evnt_cmplt  via timer or
	 *   at the end of execution if the execution is blocking
	 *
	 *   if @ref slot_durn is set to zero "0" , event scheduler has given the on idle event unlimited grant.
	 *    It will be aborted later with a calling to evnt_abortd_cbk function.*/
	uint32_t    				(*evnt_strtd_cbk)(ext_evnt_hndl_t evnt_hndl,
						uint32_t slot_durn, void* priv_data_ptr);
	/** Event Blocked Callback Function , called if the event is blocked due deadline or other reasons as
	 *  in @ref blocked_state .it could be NULL if not used by stack
	 *  if it is not NULL ,it will be called when event exceeded the given maximum start time or the given deadline while scheduling the event*/
	uint32_t    				(*evnt_blckd_cbk)(extrnl_evnt_state_e blocked_state);
	/** Event Aborted Callback Function. it could be NULL if not used by stack.  it will be called when event execution is aborted.
	 * the event @ref EXTRNL_ON_IDLE is the only event type that can be aborted so it must not be NULL for this event type
	 * when it is  called from event scheduler, the stack should stop all running operation that access phy,
	 *  no need to call @ref evnt_schdlr_gnrc_evnt_cmpltafter calling this callback as it is called from scheduler itself.
	 * */
	uint32_t    				(*evnt_abortd_cbk)(void);
	/** Event coexistence error Callback Function. it will be called when @ref EXTRNL_GNRC event execution returned error.
	 *  when  @ref evnt_strtd_cbk of @ref EXTRNL_GNRC failed at execution for any reason, this callback will be  called from event scheduler,
	 *  it'll send the returned error to ral_tx_done to check if there will retransmission of failed packet or send the error to upper layers,
	 * */
	void    			     	(*coex_error_cbk)(uint32_t error);
} extrnl_evnt_st_t;
#if(SUPPORT_COEXISTENCE || SUPPORT_GNRC_SCHDLR_IF)
/**
 * @brief  request duration extension of previous registered external generic event.
 *
 * @param  evnt_hndl_t [in]				: Event handle of registered event.
 * @param  updated_priority [in]		: Updated priority of the event during new extend request.
 * @param  flxbl_req [in]				: Flexible request flag,
 * 										  True: scheduler will try to extend duration with available duration up to requested deadline
 * 										  False: scheduler will try to extend duration with requested duration only (full requested duration or not)
 * @param  reqstd_deadline [in and out]	: Pointer to new end time that the event request to extend the grant to it.
 * It is also will be used as an output to report the given grant end time if the request is successfully accepted..
 * @retval Status (0: SUCCESS, 0xXX: ERROR_CODE from @ref ble_stat_t).
 */
uint32_t evnt_schdlr_extend_gnrc_evnt_durn(ext_evnt_hndl_t evnt_hndl, extrnl_evnt_priority_e updated_priority,
		uint8_t flxbl_req, uint64_t *reqstd_deadline);

/**
 * @brief  register external generic event.
 * This function is used to register external generic event in event scheduler.all required information to schedule this event are contained in @ref p_extrnl_evnt_st parameter
 * @param  p_extrnl_evnt_st [in]	   : Pointer to external event structure.
 * @retval evnt_hndl_t                 : The event handle of registered event.
 * @retval NULL                        : means that the event is not registered.
 */
ext_evnt_hndl_t evnt_schdlr_rgstr_gnrc_evnt(extrnl_evnt_st_t* p_extrnl_evnt_st);

/**
 * @brief  un-register external generic/ onidle  event.
 * This function is used to remove external generic event from event scheduler.
 * @param  evnt_hndl [in]	: Event handle of generic event to be removed from scheduler.
 * @retval Status (0: SUCCESS, 0xXX: ERROR_CODE from @ref ble_stat_t).
 */
uint32_t  evnt_schdlr_unrgstr_gnrc_evnt(ext_evnt_hndl_t evnt_hndl);
/**
 * @brief  register external on idle event.
 *
 * the external on idle event is low priority event that will be executed in scheduler idle time.
 * if multiple on idle events are registered the scheduler will divide the idle on them in round robin manner.
 * @note All parameters in in @ ref _extrnl_evnt_st_t will be ignored  except ptr_priv parameter,
 *  as it will be passed to event started call back
 * @note @ref evnt_strtd_cbk and @ref evnt_abortd_cbk pointers must not be NULL,
 * as event scheduler will call evnt_abortd_cbk to stop current running on idle event
 * if other higher event wants to access air while on idle event has grant to access it.
 *
 * @note There is no need to call this function more than once as the event scheduler
 * stores the event and will call it every time it has a window to start on idle event
 *
 * @param  p_extrnl_evnt_st [in]	   : Pointer to external event structure.
 * @retval evnt_hndl_t                 : The event handle of registered event.
 * @retval NULL                        : means that the event is not registered.
 */
ext_evnt_hndl_t evnt_schdlr_rgstr_on_idle_evnt(extrnl_evnt_st_t* p_extrnl_evnt_st);
#if SUPPORT_COEXISTENCE
/**
 * @brief  Disable aborting the on idle event given by the handle @ref on_idle_evnt_hdl.  the user should call @ref evnt_schdlr_gnrc_evnt_cmplt after the event is completed
 *
 * @note the  usage of this function should  be limited to the cases where the event can't be aborted like MAC DTM. and the user should return the grant to scheduler as fast as possible.
 * if this function is called where the event is granted, scheduler will ignore all other event till the grant is return to scheduler. it is not recommended to use this function
 *
 * @param  on_idle_evnt_hdl [in]	   : handle of the on the idle event to disable aborting it
 * @retval None
 */
void evnt_schdlr_disable_idle_abort(ext_evnt_hndl_t on_idle_evnt_hdl);
#endif /* SUPPORT_COEXISTENCE */
#endif /* (#if SUPPORT_COEXISTENCE || SUPPORT_GNRC_SCHDLR_IF) */

/**
 * @brief  external generic event complete.
 *
 * This function is used to return the grant back to event scheduler
 * at the end of duration given at the started function from event scheduler @ref _extrnl_evnt_st_t for more info
 *
 * @param  evnt_hndl_t [in]	: Event handle of registered event.
 * @retval Status (0: SUCCESS, 0xXX: ERROR_CODE from @ref ble_stat_t).
 */
uint32_t evnt_schdlr_gnrc_evnt_cmplt(ext_evnt_hndl_t evnt_hndl);
#if(SUPPORT_COEXISTENCE || SUPPORT_GNRC_SCHDLR_IF)
/**
 * @brief  Confirm the event currently has grant to access phy from event scheduler.
 *
 * @param  evnt_hndl [in]	: pointer of event scheduler handle.
 * @retval 1: grant given.
 * @retval 0: no grant given.
 */
uint8_t evnt_schdlr_confrm_grant(void * evnt_hndl);
#endif /* (SUPPORT_COEXISTENCE || SUPPORT_GNRC_SCHDLR_IF) */

#endif /* INCLUDE_EVNT_SCHDLR_IF_H */
/**
 * @}
 */
