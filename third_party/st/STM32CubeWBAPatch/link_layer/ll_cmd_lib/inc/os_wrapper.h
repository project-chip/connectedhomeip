/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/os_wrapper.h#1 $*/
/**
 ********************************************************************************
 * @file    os_wrapper.h
 * @brief   Wrapper header for OS porting
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

/* Define to prevent recursive inclusion */
#ifndef INCLUDE_OS_WRAPPER_H_
#define INCLUDE_OS_WRAPPER_H_

#include "stdint.h"
/********************* Macros **********************************/
#define POOL_BLOCK_SIZE 		16
#define POOL_TOTAL_BLOCKS_SIZE 	10
#define POOL_INDEX_SIZE 		6

/* Exported  Defines -----------------------------------------------------------*/

#define os_Pool_Def(type) \
os_pool_def_t os_pool_##type

#define os_Pool(type) \
&os_pool_##type

#define os_Pool_Def_extern(type)   \
extern os_pool_def_t os_pool_##type

/* Exported macros ------------------------------------------------------------*/
/* Exported types -------------------------------------------------------------*/

/**
 * @brief Interrupt status.
 */
typedef enum int_state {
	NOT_ACTIVE,
	LINK_LAYER_INTRPT,
	LINK_LAYER_LOW_PRIORITY_INTRPT,
	UART_READ_INTRPT,
	UART_WRITE_INTRPT,
	TIMER_INTRPT,
	MAC_INTRPT,
	TOTAL_INTERRUPTS
} int_state_e;

/**
 * @brief Priority used for thread control.
 */
typedef enum  {
  os_priority_high,
  os_priority_normal,
  os_priority_low,
} os_priority;

/**
 * @brief SW Timer Activity Status.
 */
typedef enum _sw_timer_activity_status_e {
	SW_TIMER_NOT_ACTIVE 	 	= 0x00,
	SW_TIMER_ACTIVE 			= 0x01,
	SW_TIMER_MAY_BE_NOT_ACTIVE	= 0x02
} sw_timer_activity_status_e;

/**
 * @brief SW Timer Type.
 */
typedef enum {
	os_timer_once = 0,        ///< one-shot timer
	os_timer_periodic = 1     ///< repeating timer
} os_timer_type;


/**
 * @brief SW Timer Priority.
 */
typedef enum {
	lw_prio_tmr = 0,        // Low Priority Timer
	hg_prio_tmr = 1     // High Priority Timer
} os_timer_prio;


/**
 * @brief Software Timer State Active, Expired or Stopped
 */
typedef enum {
	osTimerActive, 	/**< @brief Active timer : Timer in the list waiting for its time to fire */
	osTimerExpired, /**< @brief Expired timer: Timer fired and removed form the list, or created and not exist in the list */
	osTimerStopped 	/**< @brief Stopped timer: Timer stopped and removed form the list */
} os_timer_state;

// Thread Management
typedef void (*os_pthread) (void const *argument);
typedef void* os_thread_id;

// Timer Management
typedef void (*t_timer_callbk)(const void*);
typedef void* os_timer_id;
typedef void (*os_timer_activity_cb_t)(sw_timer_activity_status_e timer_activity);
typedef struct sw_timer sw_timer_t;

// Mutex Management
typedef void   os_mutex_def_t;
typedef void * os_mutex_id;

// Semaphore Management
typedef void   os_semaphore_def_t;
typedef void * os_semaphore_id;

/**
 * @brief   Software Timer structure.
 */
struct sw_timer {
	sw_timer_t *stnext; 		/**< @brief Next timer in the timers list.      			*/
	uint32_t vtime; 			/**< @brief value of timer			          				*/
	uint32_t rtime; 			/**< @brief remain time.				          			*/
	t_timer_callbk ptimer; 		/**< @brief Timer callback function pointer.    			*/
	void *argument; 			/**< @brief Timer callback function arguments.  			*/
	uint16_t overflow_flag : 1;
	uint16_t frac_time : 5; 	/** < @brief fraction time of period [0:31] in terms of us   			  */
	uint16_t cycles : 5; 		/** < @brief  cycles [0:31] number of elapsed cycles of periodic timer    */
	uint16_t rem_time : 5; 		/** < @brief  remainder to be added to the fraction [0:31] in terms of us */
	uint8_t state; 				/**< @brief Timer State : Active or Expired or Stopped 	*/
	uint8_t type:1 ; 				/**< @brief Timer Type : one-shot (0) or periodic (1)   */
	uint8_t prio:1 ; /* used to indicate if this timer ISR should be handled from hg isr in case of allow_lw_isr==1 */
};

/**
 * @brief Memory Block Structure
 */
typedef struct _mem_blck_t {
	/*	8 bits					|	 8 bits			| 8 bits	| 8 bits     *
	 *  Free memory chunk flag 	| sub-pool number	| reserved	| handle_id  */
	uint32_t flag;
	struct _mem_blck_t * next;
} mem_blck_t;

/**
 * @brief Memory Pool Block Structure
 */
typedef struct {
	uint32_t blck_size			: POOL_BLOCK_SIZE; 			/* block size */
	uint32_t total_blcks		: POOL_TOTAL_BLOCKS_SIZE;	/* total number of blocks */
	uint32_t indx				: POOL_INDEX_SIZE;			/* pool index (sub-pool number) */
	mem_blck_t* next_blck; 	/* next free block */
} os_pool_def_t;

/* Exported functions ---------------------------------------------------------*/

/**
 * @brief Creates a thread
 *
 * @param thread		Pointer to a function to be executed by the thread
 * @param name			Thread's name
 * @param pri			Thread's priority
 * @param argu			Arguments to be passed to the function executed by the thread
 * @param stack_size	Thread stack size
 *
 * @retval Handle of the created task
 */
os_thread_id os_thread_create(
		os_pthread thread,
		char* name,
		os_priority pri,
		void* argu,
		uint32_t stack_size);

/**
 * @brief Registers an interrupt function corresponding to the passed interrupt ID
 *
 * @param ptr_int_hndlr Interrupt function
 * @param int_id 		Interrupt ID
 */
void intr_hndlr_reg(
		void (*ptr_int_hndlr)(void),
		int_state_e int_id);

/**
 * @brief initialize function to to os_wrapper
 */
void os_wrapper_init(void);

/**
 * @brief reset function to os_wrapper component
 */
void os_wrapper_reset(void);


/**
 * @brief initialize timer function
 */
void os_timer_init(void);

/**
 * @brief initialize timer function
 */
void os_timer_reset(void);

/**  @ingroup SW_TIMER
 * @{
 */
/**
 * @brief  create a new timer
 *
 * @param  p_callbk      pointer to the call_back function.
 * @param  type          os_timer_once for one-shot or os_timer_periodic for periodic behavior.
 * @param  argument      argument to the timer call back function.
 *
 * @retval timer ID for reference by other functions or NULL in case of error.
 */
void* os_timer_create(
	t_timer_callbk p_callbk,
	os_timer_type type,
	void *argument);


/**
 * @brief  set the timer priority
 *
 * @param  timer id
 * @param  tmr_prio: the new priority of the timer in case of allow_lw_isr==1
 *
 * @retval None
 */
void os_timer_set_prio(os_timer_id timer_id ,
		os_timer_prio tmr_prio);

/**
 * @brief  get the timer priority
 *
 * @retval get the priority of the SW timers head
 */
uint8_t os_timer_is_any_near_sw_timer_hg_prio(void);


/**
 * @brief  start a running timer.
 *
 * @param  timer_id      timer Id.
 * @param  steps         number of steps in 31.25 us resolution
 *
 * @retval error code.
 */
int32_t os_timer_start(
	os_timer_id timer_id,
	uint32_t steps);

/**
 * @brief  start a running timer.
 *
 * @param  timer_id      timer Id.
 * @param  time_us       time in us
 *
 * @retval error code.
 */
int32_t os_timer_start_in_us(
	os_timer_id timer_id,
	uint32_t time_us);

/**
 * @brief	stop a running timer.
 *
 * @param	timer_id	  timer Id.
 *
 * @retval error code.
 */
int32_t os_timer_stop(
	os_timer_id timer_id);

/**
 * @brief	free an allocated timer.
 *
 * @param	timer_id	  timer Id.
 *
 * @retval error code.
 */
int32_t os_timer_free(
	os_timer_id timer_id);

/**
 * @brief Stop the timer if it is running and delete it.
 *
 * @param  ptr_timer_id     pointer to the timer ID obtained by  os_timer_create.
 *
 * @retval status code that indicates the execution status of the function.
 */
int32_t os_timer_stop_free(
	os_timer_id *ptr_timer_id);

/**
 * @brief  Stop the timer if it is running and start it with the new value.
 *
 * @param  timer      timer ID obtained by \ref os_timer_create.
 * @param  steps	  steps to set the timer with.
 *
 * @retval status code that indicates the execution status of the function.
 */
int32_t os_timer_set(
	os_timer_id timer,
	uint32_t steps);

/**
 * @brief	get the starte of the timer.
 *
 * @param	timer_id	  timer Id.
 *
 * @retval os_timer_state. Active , Expired, or stopped
 */
os_timer_state os_get_tmr_state(
		os_timer_id timer_id);
/**@}
 * *
 */
/**
 * @brief	Get the number of active SW timers.
 *
 * @retval active_sw_timers_num: The number of currently active SW timers
 */
uint32_t os_timer_get_active_sw_timers_number(void);

/**
 * @brief	Register a callback function to show whether the timer is in use or not.
 *
 * @param	cbk	  : [in] Callback function.
 */
void os_timer_rgstr_timer_activity_cbk(
		os_timer_activity_cb_t cbk);

/**
 * @brief	Gets the remaining time of the first time set to fire, if exists
 *
 * @retval	Remaining time. 0 if no timers exist.
 */
uint64_t os_timer_get_earliest_time(void);

/**
 * @brief  This function calls the proper handling based on the incming interrupt
 *
 * @param  intrpt_fired      current interrupt to be served.
 */
void os_process_isr(
	int_state_e intrpt_fired);

/**
*  @ingroup os_wrappers
*  @{
*/
/**
 * @brief  disables system Interrupts
 */
void os_disable_isr(void);

/**
 * @brief  enables system Interrupts, the imp. should respect the nested disable calls
 */
void os_enable_isr(void);

//  ==== Mutex Management ====

/**
 * @brief  create a new recursive mutex
 *
 * @retval handle to the created mutex
 */
os_mutex_id os_rcrsv_mutex_create(void);

/**
 * @brief  Wait until a mutex becames available
 *
 * @param  mutex_id      mutex id.
 * @param millisec      time-out value, 0 for no time-out.
 *
 * @retval status code , 0 for success
 */
int32_t os_rcrsv_mutex_wait(
		os_mutex_id mutex_id,
		uint32_t millisec);

/**
 * @brief  Release a mutex
 *
 * @param  mutex_id      mutex id.
 *
 * @retval status code, 0 for success
 */
int32_t os_rcrsv_mutex_release(
		os_mutex_id mutex_id);

//  ==== Semaphore Management Functions ====
/**
 * @brief  Create and initialize a semaphore
 *
 * @param  max_count	      The max value to which the semaphore can count.
 * @param  initial_count      initial value assigned to the count.
 *
 * @retval semaphore id for reference
 */
os_semaphore_id os_semaphore_create(
		int32_t max_count,
		int32_t initial_count);

/**
 * @brief  Wait until a semaphore becomes available
 *
 * @param  semaphore_id semaphore id.
 * @param  millisec      time-out value, 0 for no time-out.
 *
 * @retval status code, 0 for success
 */
int32_t os_semaphore_wait(
		os_semaphore_id semaphore_id,
		uint32_t millisec);

/**
 * @brief  Release a semaphore
 *
 * @param  semaphore_id semaphore id.
 *
 * @retval status code, 0 for success
 */
int32_t os_semaphore_release(
		os_semaphore_id semaphore_id);

/**
 * @brief  Release an ISR semaphore
 *
 * @param  semaphore_id semaphore id.
 *
 * @retval status code, 0 for success
 */
int32_t os_semaphore_release_isr(
		os_semaphore_id semaphore_id);

/**
 * @}
 */
/*  ==== Memory Pool Management Functions ====  */

/**
 * @brief Allocates from the passed memory pool
 *
 * @param pool Pointer to the pool to allocate from
 *
 * @retval Pointer at the allocated block
 */
void * os_mem_pool_alloc(
		os_pool_def_t * pool);

/**
 * @brief Frees from the passed memory pool
 *
 * @param block Pointer at the block that will be freed
 */
void os_mem_pool_free(
		void *block);

/**
 * @brief Allocates from the shared memory pool
 *
 * @param pool Pointer to the pool to allocate from
 *
 * @retval Pointer at the allocated block
 */
void* os_shrd_mem_alloc(
		os_pool_def_t * pool);

/**
 * @fn uint8_t os_wrapper_is_rtos_used()
 *
 * @brief This function used to detect whether RTOS configuration is enabled or not.
 * @param None.
 * @return is_rtos_enabled : TRUE: RTOS enabled. FALSE: otherwise
 */
uint8_t os_wrapper_is_rtos_used(void);

#endif /* INCLUDE_CONN_MNGR_H_ */
