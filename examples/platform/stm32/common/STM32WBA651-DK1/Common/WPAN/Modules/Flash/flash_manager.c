/**
  ******************************************************************************
  * @file    flash_manager.c
  * @author  MCD Application Team
  * @brief   The Flash Manager module provides an interface to write raw data
  *          from SRAM to FLASH
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
#include "flash_manager.h"
#include "rf_timing_synchro.h"
#include "flash_driver.h"
#include "utilities_conf.h"

#include "stm32wbaxx_hal.h"

/* Debug */
#include "log_module.h"

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* State of the background process */
typedef enum FM_BackGround_States
{
  FM_BKGND_NOWINDOW_FLASHOP,
  FM_BKGND_WINDOWED_FLASHOP,
}FM_BackGround_States_t;

/* Flash operation type */
typedef enum
{
  FM_WRITE_OP,
  FM_ERASE_OP
} FM_FlashOp_t;

/**
 * @brief Flash operation configuration struct
 */
typedef struct FM_FlashOpConfig
{
  uint32_t *writeSrc;
  uint32_t *writeDest;
  int32_t  writeSize;
  uint32_t eraseFirstSect;
  uint32_t eraseNbrSect;
}FM_FlashOpConfig_t;

/* Private defines -----------------------------------------------------------*/
#define FLASH_PAGE_NBR    (FLASH_SIZE / FLASH_PAGE_SIZE)
#define FLASH_WRITE_BLOCK_SIZE  4U
#define ALIGNMENT_32   0x00000003
#define ALIGNMENT_128  0x0000000F

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
  * @brief Semaphore on Flash
  */
static bool busy_flash_sem = FALSE;

/**
  * @brief Indicates if the flash manager module is available or not
  */
static bool flash_manager_busy = FALSE;

/**
  * @brief Parameters for Flash Write command
  */
static bool fm_window_granted = FALSE;

/**
  * @brief Callback node list for pending flash operation request
  */
static tListNode fm_cb_pending_list;

/**
  * @brief Flag indicating if pending node list has been initialized
  */
static bool fm_cb_pending_list_init = FALSE;

/**
  * @brief Pointer to current flash operation requester's callback
  */
static void (*fm_running_cb)(FM_FlashOp_Status_t);

/**
  * @brief Type of current flash operation (Write/Erase)
  */
static FM_FlashOp_t fm_flashop;

/**
  * @brief Parameters for Flash operation
  */
static FM_FlashOpConfig_t fm_flashop_parameters;

/**
 * @brief State of the Background process
 */
static FM_BackGround_States_t FM_CurrentBackGroundState;

/* Private function prototypes -----------------------------------------------*/

static FM_Cmd_Status_t FM_CheckFlashManagerState(FM_CallbackNode_t *CallbackNode);
static void FM_WindowAllowed_Callback(void);

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Request the Flash Manager module to initiate a Flash Write operation
  * @param  Src: Address of the data to be stored in FLASH. It shall be 32bits aligned
  * @param  Dest: Address where the data shall be written. It shall be 128bits aligned
  * @param  Size: This is the size of data to be written in Flash.
                  The size is a multiple of 32bits (size = 1 means 32bits)
  * @param  CallbackNode: Pointer to the callback node for storage in list
  * @retval FM_Cmd_Status_t: Status of the Flash Manager module
  */
FM_Cmd_Status_t FM_Write(uint32_t *Src, uint32_t *Dest, int32_t Size, FM_CallbackNode_t *CallbackNode)
{
  FM_Cmd_Status_t status;

  if (((uint32_t)Dest < FLASH_BASE) || ((uint32_t)Dest > (FLASH_BASE + FLASH_SIZE))
                                    || (((uint32_t)Dest + Size) > (FLASH_BASE + FLASH_SIZE)))
  {
    LOG_ERROR_SYSTEM("\r\nFM_Write - Destination address not part of the flash");

    /* Destination address not part of the flash */
    return FM_ERROR;
  }

  if (((uint32_t) Src & ALIGNMENT_32) || ((uint32_t) Dest & ALIGNMENT_128))
  {
    LOG_ERROR_SYSTEM("\r\nFM_Write - Source or destination address not properly aligned");

    /* Source or destination address not properly aligned */
    return FM_ERROR;
  }

  status = FM_CheckFlashManagerState(CallbackNode);

  if (status == FM_OK)
  { /* Flash manager is available */

    /* Save Write parameters */
    fm_flashop_parameters.writeSrc = Src;
    fm_flashop_parameters.writeDest = Dest;
    fm_flashop_parameters.writeSize = Size;

    fm_flashop = FM_WRITE_OP;

    FM_CurrentBackGroundState = FM_BKGND_NOWINDOW_FLASHOP;

    /* Window request to be executed in background */
    FM_ProcessRequest();
  }

  LOG_INFO_SYSTEM("\r\nFM_Write - Returned value : %d", status);

  return status;
}

/**
  * @brief  Request the Flash Manager module to initiate a Flash Write operation
  * @param  Src: Address of the data to be stored in FLASH. It shall be 32bits aligned
  *              Compared to FM_write, removes the 32bits alignment constraint.
  * @param  Dest: Address where the data shall be written. It shall be 128bits aligned
  * @param  Size: This is the size of data to be written in Flash.
                  The size is a multiple of 32bits (size = 1 means 32bits)
  * @param  CallbackNode: Pointer to the callback node for storage in list
  * @retval FM_Cmd_Status_t: Status of the Flash Manager module
  */
FM_Cmd_Status_t FM_Write_ext(uint32_t *Src, uint32_t *Dest, int32_t Size, FM_CallbackNode_t *CallbackNode)
{
  FM_Cmd_Status_t status;

  if (((uint32_t)Dest < FLASH_BASE) || ((uint32_t)Dest > (FLASH_BASE + FLASH_SIZE))
                                    || (((uint32_t)Dest + Size) > (FLASH_BASE + FLASH_SIZE)))
  {
    LOG_ERROR_SYSTEM("\r\nFM_Write - Destination address not part of the flash");

    /* Destination address not part of the flash */
    return FM_ERROR;
  }

  if ((uint32_t) Dest & ALIGNMENT_128)
  {
    LOG_ERROR_SYSTEM("\r\nFM_Write - Source or destination address not properly aligned");

    /* Source or destination address not properly aligned */
    return FM_ERROR;
  }

  status = FM_CheckFlashManagerState(CallbackNode);

  if (status == FM_OK)
  { /* Flash manager is available */

    /* Save Write parameters */
    fm_flashop_parameters.writeSrc = Src;
    fm_flashop_parameters.writeDest = Dest;
    fm_flashop_parameters.writeSize = Size;

    fm_flashop = FM_WRITE_OP;

    FM_CurrentBackGroundState = FM_BKGND_NOWINDOW_FLASHOP;

    /* Window request to be executed in background */
    FM_ProcessRequest();
  }

  LOG_INFO_SYSTEM("\r\nFM_Write - Returned value : %d", status);

  return status;
}

/**
  * @brief  Request the Flash Manager module to initiate a Flash Erase operation
  * @param  FirstSect: Index of the first sector to erase
  * @param  NbrSect: Number of sector to erase
  * @param  CallbackNode: Pointer to the callback node for storage in list
  * @retval FM_Cmd_Status_t: Status of the Flash Manager module
  */
FM_Cmd_Status_t FM_Erase(uint32_t FirstSect, uint32_t NbrSect, FM_CallbackNode_t *CallbackNode)
{
  FM_Cmd_Status_t status;

  if ((FirstSect > FLASH_PAGE_NBR) || ((FirstSect + NbrSect) > FLASH_PAGE_NBR))
  {
    LOG_ERROR_SYSTEM("\r\nFM_Erase - Inconsistent request");

    /* Inconsistent request */
    return FM_ERROR;
  }

  if (NbrSect == 0)
  {
    LOG_ERROR_SYSTEM("\r\nFM_Erase - Inconsistent request");

    /* Inconsistent request */
    return FM_ERROR;
  }

  status = FM_CheckFlashManagerState(CallbackNode);

  if (status == FM_OK)
  { /* Flash manager is available */

    /* Save Erase parameters */
    fm_flashop_parameters.eraseFirstSect = FirstSect;
    fm_flashop_parameters.eraseNbrSect = NbrSect;

    fm_flashop = FM_ERASE_OP;

    FM_CurrentBackGroundState = FM_BKGND_NOWINDOW_FLASHOP;

    /* Window request to be executed in background */
    FM_ProcessRequest();
  }

  LOG_INFO_SYSTEM("\r\nFM_Erase - Returned value : %d", status);

  return status;
}

/**
  * @brief  Execute Flash Manager background tasks
  * @param  None
  * @retval None
  */
void FM_BackgroundProcess (void)
{
  static uint32_t duration;
  bool flashop_complete = false;
  FD_FlashOp_Status_t fdReturnValue = FD_FLASHOP_SUCCESS;
  FM_CallbackNode_t *pCbNode = NULL;

  switch (FM_CurrentBackGroundState)
  {
    case FM_BKGND_NOWINDOW_FLASHOP:
    {
      LOG_INFO_SYSTEM("\r\nFM_BackgroundProcess - Case FM_BKGND_NOWINDOW_FLASHOP");

      if (fm_flashop == FM_WRITE_OP)
      {
        LOG_INFO_SYSTEM("\r\nFM_BackgroundProcess - Case FM_BKGND_NOWINDOW_FLASHOP - Write operation");

        /* Update duration time value */
        duration = TIME_WINDOW_WRITE_REQUEST;

        /* Set the next possible state - App could stop at anytime no window operation */
        FM_CurrentBackGroundState = FM_BKGND_WINDOWED_FLASHOP;

        HAL_FLASH_Unlock();

        while((fm_flashop_parameters.writeSize > 0) &&
              (fdReturnValue == FD_FLASHOP_SUCCESS))
        {
          fdReturnValue = FD_WriteData((uint32_t) fm_flashop_parameters.writeDest,
                                       (uint32_t) fm_flashop_parameters.writeSrc);

          if (fdReturnValue == FD_FLASHOP_SUCCESS)
          {
            fm_flashop_parameters.writeDest += FLASH_WRITE_BLOCK_SIZE;
            fm_flashop_parameters.writeSrc += FLASH_WRITE_BLOCK_SIZE;
            fm_flashop_parameters.writeSize -= FLASH_WRITE_BLOCK_SIZE;
          }
        }

        HAL_FLASH_Lock();

        /* Is write over ? */
        if (fm_flashop_parameters.writeSize <= 0)
        {
          flashop_complete = true;
        }
      }
      else
      {
        LOG_INFO_SYSTEM("\r\nFM_BackgroundProcess - Case FM_BKGND_NOWINDOW_FLASHOP - Erase operation");

        /* Update duration time value */
        duration = TIME_WINDOW_ERASE_REQUEST;

        /* Set the next possible state */
        FM_CurrentBackGroundState = FM_BKGND_WINDOWED_FLASHOP;

        HAL_FLASH_Unlock();

        while((fm_flashop_parameters.eraseNbrSect > 0) &&
              (fdReturnValue == FD_FLASHOP_SUCCESS))
        {
          fdReturnValue = FD_EraseSectors(fm_flashop_parameters.eraseFirstSect);

          if (fdReturnValue == FD_FLASHOP_SUCCESS)
          {
            fm_flashop_parameters.eraseNbrSect--;
            fm_flashop_parameters.eraseFirstSect++;
          }
        }

        HAL_FLASH_Lock();

        if (fm_flashop_parameters.eraseNbrSect == 0)
        {
          flashop_complete = true;
        }
      }
      break;
    }

    case FM_BKGND_WINDOWED_FLASHOP:
    {
      LOG_INFO_SYSTEM("\r\nFM_BackgroundProcess - Case FM_BKGND_WINDOWED_FLASHOP");

      if (fm_window_granted == false)
      {
        LOG_INFO_SYSTEM("\r\nFM_BackgroundProcess - Case FM_BKGND_WINDOWED_FLASHOP - No time window granted yet, request one");

        /* No time window granted yet, request one */
        RFTS_ReqWindow(duration, &FM_WindowAllowed_Callback);
      }
      else
      {
        LOG_INFO_SYSTEM("\r\nFM_BackgroundProcess - Case FM_BKGND_WINDOWED_FLASHOP - Time window granted");

        if (fm_flashop == FM_WRITE_OP)
        {
          /* Flash Write operation */
          LOG_INFO_SYSTEM("\r\nFM_BackgroundProcess - Case FM_BKGND_WINDOWED_FLASHOP - Write operation");

          HAL_FLASH_Unlock();

          while((fm_flashop_parameters.writeSize > 0) &&
                (FD_WriteData((uint32_t) fm_flashop_parameters.writeDest,
                              (uint32_t) fm_flashop_parameters.writeSrc) == FD_FLASHOP_SUCCESS))
          {
              fm_flashop_parameters.writeDest += FLASH_WRITE_BLOCK_SIZE;
              fm_flashop_parameters.writeSrc += FLASH_WRITE_BLOCK_SIZE;
              fm_flashop_parameters.writeSize -= FLASH_WRITE_BLOCK_SIZE;
          }

          if (fm_flashop_parameters.writeSize <= 0)
          {
            flashop_complete = true;
          }

      HAL_FLASH_Lock();

    }
    else
    {
      /* Flash Erase operation */
      LOG_INFO_SYSTEM("\r\nFM_BackgroundProcess - Case FM_BKGND_WINDOWED_FLASHOP - Erase operation");

      HAL_FLASH_Unlock();

      /* Erase only one sector in a single time window */
      if (FD_EraseSectors(fm_flashop_parameters.eraseFirstSect) == FD_FLASHOP_SUCCESS)
      {
        fm_flashop_parameters.eraseNbrSect--;
        fm_flashop_parameters.eraseFirstSect++;
      }

      if (fm_flashop_parameters.eraseNbrSect == 0)
      {
        flashop_complete = true;
      }

      HAL_FLASH_Lock();
    }

        /* Release the time window */
        RFTS_RelWindow();

        /* Indicate that there is no more window */
        fm_window_granted = false;
      }

      break;
    }

    default:
    {
      /* Nothing to do here */
      break;
    }
  }

  if (flashop_complete == true)
  {
    UTILS_ENTER_CRITICAL_SECTION();

    /* Release semaphore on flash */
    busy_flash_sem = false;

    /* Set Flash Manager busy */
    flash_manager_busy = false;

    UTILS_EXIT_CRITICAL_SECTION();

    /* Invoke the running callback if present */
    if (fm_running_cb != NULL)
    {
      fm_running_cb(FM_OPERATION_COMPLETE);
    }

    /* notify pending requesters */
    while((LST_is_empty (&fm_cb_pending_list) == false) &&
          (busy_flash_sem == false) && (flash_manager_busy == false))
    {
      LST_remove_head (&fm_cb_pending_list, (tListNode**)&pCbNode);
      pCbNode->Callback(FM_OPERATION_AVAILABLE);
    }
  }
  else
  {
    /* Flash operation not complete yet */
    LOG_INFO_SYSTEM("\r\nFM_BackgroundProcess - Flash operation not complete yet, request a new time window");

    /* Request a new time window */
    RFTS_ReqWindow(duration, &FM_WindowAllowed_Callback);
  }
}

/**
  * @brief  Check if the Flash Manager is busy or available
  * @param  CallbackNode: Pointer to the callback node for storage in list
  * @retval FM_Cmd_Status_t: Status of the Flash Manager module
  */
static FM_Cmd_Status_t FM_CheckFlashManagerState(FM_CallbackNode_t *CallbackNode)
{
  bool fm_process_cmd = false;
  FM_Cmd_Status_t status = FM_ERROR;

  /* Check if semaphore on flash is available */
  UTILS_ENTER_CRITICAL_SECTION();

  /* Initialize pending list if not done */
  if (fm_cb_pending_list_init == false)
  {
    LST_init_head(&fm_cb_pending_list);
    fm_cb_pending_list_init = true;
  }
  /* Check if semaphore on flash is available */
  if (busy_flash_sem == false)
  { /* Check if Flash Manager is already busy */
    if (flash_manager_busy == false)
    {
      busy_flash_sem = true; /* Get semaphore on flash */
      flash_manager_busy = true; /* Set Flash Manager busy */
      fm_process_cmd = true;
    }
    else
    {
      fm_process_cmd = false;
    }
  }
  else
  {
    fm_process_cmd = false;
  }
  UTILS_EXIT_CRITICAL_SECTION();

  if (fm_process_cmd == false)
  { /* Flash manager busy */

    /* Append callback to the pending list */
    if ((CallbackNode != NULL) && (CallbackNode->Callback != NULL))
    {
      LST_insert_tail(&fm_cb_pending_list, &(CallbackNode->NodeList));
    }

    status = FM_BUSY;
  }
  else
  { /* Flash manager is available */

    if ((CallbackNode != NULL) && (CallbackNode->Callback != NULL))
    {
      UTILS_ENTER_CRITICAL_SECTION();

      fm_running_cb = CallbackNode->Callback;

      UTILS_EXIT_CRITICAL_SECTION();
    }
    else
    {
      UTILS_ENTER_CRITICAL_SECTION();

      fm_running_cb = NULL;

      UTILS_EXIT_CRITICAL_SECTION();
    }

    status = FM_OK;
  }
  return status;
}

/**
  * @brief  Callback called by RF Timing Synchro module when a time window is available
  * @param  None
  * @retval None
  */
static void FM_WindowAllowed_Callback(void)
{
  fm_window_granted = true;

  LOG_INFO_SYSTEM("\r\nFM_WindowAllowed_Callback");

  /* Flash operation to be executed in background */
  FM_ProcessRequest();
}
