/**
  ******************************************************************************
  * @file    rf_timing_synchro.c
  * @author  MCD Application Team
  * @brief   The RF Timing Synchronization module provides an interface to
  *          synchronize the flash processing versus the RF activity to make
  *          sure the RF timing is not broken
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>

#include "rf_timing_synchro.h"
#include "evnt_schdlr_gnrc_if.h"
#include "utilities_conf.h"
#include "stm32_timer.h"
#include "flash_driver.h"

/* Global variables ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

#if (DISABLE_RFTS_EXT_EVNT_HNDLR == 0u)
/**
  * @brief Pointer to time window requester's callback
  */
static void (*req_callback)(void);

/**
  * @brief Indicates if a time window has already been requested or not
  */
static bool rfts_window_req_pending = FALSE;

/**
  * @brief Timer used by the RFTS module to prevent time window overrun
  */
static UTIL_TIMER_Object_t rfts_timer;

/**
  * @brief Firmware Link Layer external event handler
  */
static ext_evnt_hndl_t ext_event_handler;

/* Private function prototypes -----------------------------------------------*/

static void RFTS_WindowAllowed_Callback(void);
static void RFTS_Timeout_Callback(void* Argument);
static uint32_t event_started_callback(ext_evnt_hndl_t evnt_hndl, uint32_t slot_durn, void* priv_data_ptr);
#endif /* (DISABLE_RFTS_EXT_EVNT_HNDLR == 0u) */

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Request a time window to the Firmware Link Layer
  * @param  duration: Duration in us of the time window requested
  * @param  Callback: Callback to be called when time window is allocated
  * @retval RFTS_Cmd_Status_t: Success or failure of the window request
  */
RFTS_Cmd_Status_t RFTS_ReqWindow(uint32_t Duration, void (*Callback)(void))
{
#if (DISABLE_RFTS_EXT_EVNT_HNDLR == 0u)
  extrnl_evnt_st_t extrnl_evnt_config;
  bool req_pending = false;

  if (Callback == NULL)
  { /* Prevent use of uninitialized callback */
    return RFTS_WINDOW_REQ_FAILED;
  }

  /* Check no request is already pending */
  UTILS_ENTER_CRITICAL_SECTION();
  if (rfts_window_req_pending == true)
  {
    req_pending = true;
  }
  else
  {
    rfts_window_req_pending = true;
  }
  UTILS_EXIT_CRITICAL_SECTION();
  if (req_pending == true)
  { /* A window request is already pending */
    return RFTS_WINDOW_REQ_FAILED;
  }

  /* Register requester's callback */
  req_callback = Callback;

  /* Submit request to Firmware Link Layer */
  extrnl_evnt_config.deadline = 0;
  extrnl_evnt_config.strt_min = 0;
  extrnl_evnt_config.strt_max = 0;
  extrnl_evnt_config.durn_min = Duration;
  extrnl_evnt_config.durn_max = 0;
  extrnl_evnt_config.prdc_intrvl = 0;
  extrnl_evnt_config.priority = PRIORITY_DEFAULT;
  extrnl_evnt_config.blocked = STATE_NOT_BLOCKED;
  extrnl_evnt_config.ptr_priv = NULL;
  extrnl_evnt_config.evnt_strtd_cbk = &event_started_callback;
  extrnl_evnt_config.evnt_blckd_cbk = NULL;
  extrnl_evnt_config.evnt_abortd_cbk = NULL;

  UTIL_TIMER_Create(&rfts_timer,
                    (Duration/1000),
                    UTIL_TIMER_ONESHOT,
                    &RFTS_Timeout_Callback,
                    NULL);

  ext_event_handler = evnt_schdlr_rgstr_gnrc_evnt(&extrnl_evnt_config);
  if (ext_event_handler == NULL)
  {
    UTILS_ENTER_CRITICAL_SECTION();
    rfts_window_req_pending = false;
    UTILS_EXIT_CRITICAL_SECTION();

    return RFTS_WINDOW_REQ_FAILED;
  }
#endif /* (DISABLE_RFTS_EXT_EVNT_HNDLR == 0u) */

  return RFTS_CMD_OK;
}

/**
  * @brief  Execute necessary tasks to allow the time window to be released
  * @param  None
  * @retval RFTS_Cmd_Status_t: Success or error in the window release procedure
  */
RFTS_Cmd_Status_t RFTS_RelWindow(void)
{
#if (DISABLE_RFTS_EXT_EVNT_HNDLR == 0u)
  RFTS_Cmd_Status_t status;

  /* Stop RFTS module window overrun control timer */
  UTIL_TIMER_Stop(&rfts_timer);

  /* Inform Firmware Link Layer that time window can be released */
  if (evnt_schdlr_gnrc_evnt_cmplt(ext_event_handler) == 0)
  {
    status = RFTS_CMD_OK;
  }
  else
  {
    status = RFTS_WINDOW_REL_ERROR;
  }

  /* Forbid flash operation */
  FD_SetStatus(FD_FLASHACCESS_RFTS, LL_FLASH_DISABLE);

  UTILS_ENTER_CRITICAL_SECTION();
  rfts_window_req_pending = false;
  UTILS_EXIT_CRITICAL_SECTION();

  return status;
#else
  return RFTS_CMD_OK;
#endif /* (DISABLE_RFTS_EXT_EVNT_HNDLR == 0u) */
}

#if (DISABLE_RFTS_EXT_EVNT_HNDLR == 0u)
/**
  * @brief  Callback called by Firmware Link Layer when a time window is available
  * @note   This callback is supposed to be called under interrupt
  * @param  None
  * @retval None
  */
static void RFTS_WindowAllowed_Callback(void)
{
  /* Allow flash operation */
  FD_SetStatus(FD_FLASHACCESS_RFTS, LL_FLASH_ENABLE);

  /* Start timer preventing window overrun */
  UTIL_TIMER_Start(&rfts_timer);

  /* Call back requester to inform time window is available */
  req_callback();
}

/**
  * @brief  Callback triggered by a timeout when the allocated window time is elapsed
  * @note   This callback is supposed to be called under interrupt
  * @param  None
  * @retval None
  */
static void RFTS_Timeout_Callback(void* Argument)
{
  /* Forbid flash operation */
  FD_SetStatus(FD_FLASHACCESS_RFTS, LL_FLASH_DISABLE);

  UTILS_ENTER_CRITICAL_SECTION();
  rfts_window_req_pending = false;
  UTILS_EXIT_CRITICAL_SECTION();
}

static uint32_t event_started_callback(ext_evnt_hndl_t evnt_hndl, uint32_t slot_durn, void* priv_data_ptr)
{
  RFTS_WindowAllowed_Callback();
  return 0;
}
#endif /* (DISABLE_RFTS_EXT_EVNT_HNDLR == 0u) */
