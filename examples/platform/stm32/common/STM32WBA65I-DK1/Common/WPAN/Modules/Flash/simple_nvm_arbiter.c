/**
  ******************************************************************************
  * @file    simple_nvm_arbiter.c
  * @author  MCD Application Team
  * @brief   The Simple NVM arbiter module provides an interface to write and/or
  *          restore data from SRAM to FLASH with use of NVMs.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

/* Memset */
#include <string.h>

/* Own header files */
#include "simple_nvm_arbiter.h"
#include "simple_nvm_arbiter_common.h"
#include "simple_nvm_arbiter_conf.h"

/* Target defines */
#include "stm32wbaxx_hal.h"

/* Needed modules */
#include "flash_driver.h"
#include "flash_manager.h"
#include "crc_ctrl.h"
#include "crc_ctrl_conf.h"

/* Tools */
#include "stm_list.h"
#include "utilities_common.h"

/* Debug */
#include "log_module.h"

/* Global variables ----------------------------------------------------------*/
/* Error handler */
extern void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/
/* Alignment 128bits for NVM Start address */
#define SNVMA_MASK_ALIGNMENT_128  0x0000000Fu

/* Alignment 32bits for Buffer addresses */
#define SNVMA_MASK_ALIGNMENT_32   0x00000003u

/* Flash memory erased content */
#define SNVMA_ERASED_CONTENT  0xFFFFu

/* Private macros ------------------------------------------------------------*/
/* Align an address on a 128 bits boundary */
#define SNVMA_ALIGN_128(p_Addr)  \
  ((p_Addr + SNVMA_MASK_ALIGNMENT_128) & ~SNVMA_MASK_ALIGNMENT_128)

/* Private typedef -----------------------------------------------------------*/

/* Flash operation steps */
typedef enum SNVMA_FlashOpSteps
{
  SNVMA_HEADER_WRITE,
  SNVMA_BUFFER_WRITE,
  SNVMA_ERASE_BANK,
  SNVMA_RETRY_WRITE
}SNVMA_FlashOpSteps_t;

/* Flash operation information */
typedef struct __PACKED SNVMA_FlashOpInfo
{
  /* Id of the NVM in use */
  uint8_t NvmId;
  /* Id of the buffer in use - Shall be set to 0 on first write */
  uint8_t BufferId;
  /* State of the flash operation */
  SNVMA_FlashOpSteps_t FlashOpState;
}SNVMA_FlashOpInfo_t;

/* Bank header */
typedef __PACKED_STRUCT __ALIGNED(16) SNVMA_BankHeader
{
  /* Unused value - Only for padding */
  uint8_t Empty;
  /* Bank counter - Indicates the freshness of the bank. The greater, the fresher */
  uint8_t Counter;
  /* Integrity check - CRC16 */
  uint16_t Crc;
  /* Buffer ID #1 */
  uint8_t BufferId1;
  /* Size used by user ID #1 */
  uint16_t SizeId1;
  /* Buffer ID #2 */
  uint8_t BufferId2;
  /* Size used by user ID #2 */
  uint16_t SizeId2;
  /* Buffer ID #3 */
  uint8_t BufferId3;
  /* Size used by user ID #3 */
  uint16_t SizeId3;
  /* Buffer ID #4 */
  uint8_t BufferId4;
  /* Size used by user ID #4 */
  uint16_t SizeId4;
}SNVMA_BankHeader_t;

/* Private variables ---------------------------------------------------------*/
/* Flag for module initialization */
static uint8_t SNVMA_ModuleInit = FALSE;

/* Flag for command pending */
static uint8_t SNVMA_CommandPending = FALSE;

/* Bitmask for write operation */
static uint32_t SNVMA_IdBitmask = 0x00000000;

/* Bank header for write operation */
static SNVMA_BankHeader_t SNVMA_WriteBankHeader;

/* Callback struct for Flash manager */
static FM_CallbackNode_t SNVMA_FlashCallback;

/* Information of the ongoing flash operation */
static SNVMA_FlashOpInfo_t SNVMA_FlashInfo;

/* Representation of the Bank configuration */
static SNVMA_BankElt_t SNVMA_BankConfiguration[SNVMA_NUMBER_OF_BANKS];

/* Callback prototypes -----------------------------------------------*/

/**
 * @brief Callback to be invoked by flash manager
 *
 * @details Informs about the result of the flash operation
 *
 * @param Status: Status of the flash operation
 *
 */
void SNVMA_FlashManagerCallback(FM_FlashOp_Status_t Status);

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief Verify the credibility of a bank header
 *
 * @param p_BankStartAddress: Bank start address to verify header
 *
 * @return State of the Header
 * @retval TRUE: Header seems to be good
 * @retval FALSE: Header is not good
 */
static inline uint8_t IsHeaderOk (const uint32_t * const p_BankStartAddress, const uint8_t NvmId);

/**
 * @brief Verify the integrity of a bank
 *
 * @param p_BankStartAddress: Start address of the bank
 *
 * @return State of the CRC
 * @retval TRUE: CRC is OK
 * @retval FALSE: CRC is NOK
 */
static inline uint8_t IsCrcOk (const uint32_t * const p_BankStartAddress);

/**
 * @brief Verify that the source and the destination content are the same
 *
 * @param p_Source: Original content address
 * @param p_Destination: Content address to compare
 * @param Size: Size of the content to compare
 *
 * @return Result of a comparison between source and destination
 * @retval TRUE: Content is the same
 * @retval FALSE: Content is not the same
 */
static inline uint8_t IsSameContent (uint32_t * p_Source, uint32_t * p_Destination, uint32_t Size);

/**
 * @brief Determine which bank is the newest
 *
 * @param p_FirstBank: Pointer onto the first bank
 * @param p_SecondBankStartAddress: Pointer onto the second bank
 *
 * @return Pointer on the newest bank
 */
static inline SNVMA_BankElt_t * GetNewestBank (SNVMA_BankElt_t * p_FirstBank,
                                               SNVMA_BankElt_t * p_SecondBank);

/**
 * @brief Erase the sector(s) given in parameter
 *
 * @param SectorId: Sector ID to erase
 * @param SectorNumber: Number of sectors to erase
 *
 * @return State of the operation
 * @retval TRUE: Op. OK
 * @retval FALSE: Op. NOK
 */
static inline uint8_t EraseSector (const uint32_t SectorId, const uint32_t SectorNumber);

/**
 * @brief Start a write sequence with the header creation and write request
 *
 * @param NvmId: Id of the NVM in use
 *
 * @return State of the flash write operation
 */
static inline FM_Cmd_Status_t StartFlashWrite (const uint8_t NvmId);

/**
 * @brief Invoke all the NVM pending buffer registered callbacks
 *
 * @param NvmId: Id of the NVM in use
 * @param CallbackStatus: Status to propagate
 *
 * @return None
 */
static inline void InvokeBufferCallback (const uint8_t NvmId, const SNVMA_Callback_Status_t CallbackStatus);

/* Functions Definition ------------------------------------------------------*/
SNVMA_Cmd_Status_t SNVMA_Init (const uint32_t * p_NvmStartAddress)
{
  SNVMA_Cmd_Status_t error = SNVMA_ERROR_NOK;

  uint16_t bankNbr = 0x00;
  uint16_t bankConfIdx = 0x00;
  uint32_t bankPrint = 0x00;
  uint32_t nvmOffset = 0x00;
  uint32_t addressOffset = 0x00;

  CRCCTRL_Cmd_Status_t crcCtrlStatus = CRCCTRL_UNKNOWN;

  SNVMA_BankElt_t * p_currentRestoreBank = NULL;

  /* Check if not already initialized */
  if (SNVMA_ModuleInit == TRUE)
  {
    error = SNVMA_ERROR_ALREADY_INIT;
  }
  /* Check NVM Null pointer */
  else if (p_NvmStartAddress == NULL)
  {
    error = SNVMA_ERROR_NVM_NULL;
  }
  /* Check 128bits alignment */
  else if (((uint32_t)p_NvmStartAddress & SNVMA_MASK_ALIGNMENT_128) != 0x00000000)
  {
    error = SNVMA_ERROR_NVM_NOT_ALIGNED;
  }
  /* Check that configuration will not overlap flash */
  else if ((SNVMA_NUMBER_OF_SECTOR_NEEDED > FLASH_PAGE_NB) ||
           ((uint32_t)p_NvmStartAddress + (SNVMA_NUMBER_OF_SECTOR_NEEDED * FLASH_PAGE_SIZE) >
            (FLASH_BASE_NS + FLASH_SIZE)))
  {
    error = SNVMA_ERROR_NVM_OVERLAP_FLASH;
  }
  else
  {
    /* Init the crc handle */
    crcCtrlStatus = CRCCTRL_RegisterHandle(&SNVMA_Handle);

    if ((CRCCTRL_OK != crcCtrlStatus) &&
        (CRCCTRL_HANDLE_ALREADY_REGISTERED != crcCtrlStatus))
    {
      error = SNVMA_ERROR_CRC_INIT;
    }
    else
    {
      /* Enter critical section */
      UTILS_ENTER_CRITICAL_SECTION();

      /* Prepare flash manager callback */
      SNVMA_FlashCallback.Callback = SNVMA_FlashManagerCallback;

      /* Compute nvm implantation offset */
      nvmOffset = (uint32_t)p_NvmStartAddress - FLASH_BASE_NS;

      /* Ensure all variable are initialized: Banks */
      memset ((void *)SNVMA_BankConfiguration,
              0x00,
              (sizeof(SNVMA_BankElt_t) * SNVMA_NUMBER_OF_BANKS));

      /* For each NVM */
      for (uint8_t nvmIdx = 0x00;
           (nvmIdx < SNVMA_NVM_NUMBER) && (error == SNVMA_ERROR_NOK);
           nvmIdx++)
      {
        /* Updates levels for bank numbers and bank foot print */
        bankNbr += SNVMA_NvmConfiguration[nvmIdx].BankNumber;
        bankPrint += (SNVMA_NvmConfiguration[nvmIdx].BankSize * SNVMA_NvmConfiguration[nvmIdx].BankNumber);

        /* Check the Bank configuration, Is number of banks ok ? */
        if ((SNVMA_NvmConfiguration[nvmIdx].BankNumber < SNVMA_MIN_NUMBER_BANK) ||
            (bankNbr > SNVMA_NUMBER_OF_BANKS))
        {
          error = SNVMA_ERROR_BANK_NUMBER;
        }
        /* Is bank size ok ? */
        else if ((SNVMA_NvmConfiguration[nvmIdx].BankSize == 0x00) ||
                 (bankPrint > SNVMA_NUMBER_OF_SECTOR_NEEDED))
        {
          error = SNVMA_ERROR_BANK_SIZE;
        }
        else
        {
          p_currentRestoreBank = NULL;

          /* Get the first bank element of our NVM - Will be used as the bank list of the NVM */
          SNVMA_NvmConfiguration[nvmIdx].p_BankList = &SNVMA_BankConfiguration[bankConfIdx];

          /* For each bank ... */
          for (uint8_t bankIdx = 0x00;
              bankIdx < SNVMA_NvmConfiguration[nvmIdx].BankNumber;
              bankIdx++)
          {
            /* ... compute bank addresses */
            SNVMA_BankConfiguration[bankConfIdx].p_StartAddr = (uint32_t *)((uint32_t)p_NvmStartAddress + addressOffset);

            /* Shall this bank be the one in use */
            if (IsHeaderOk (SNVMA_BankConfiguration[bankConfIdx].p_StartAddr,
                            nvmIdx) == FALSE)
            {
              /* Erase the bank */
              while (EraseSector (((nvmOffset + addressOffset) / FLASH_PAGE_SIZE),
                                  SNVMA_NvmConfiguration[nvmIdx].BankSize) == FALSE);

              LOG_ERROR_SYSTEM("\r\nSNVMA_Init - Corrupted banks erases [IsHeaderOk]");
            }
            /* Check if CRC OK */
            else if (IsCrcOk (SNVMA_BankConfiguration[bankConfIdx].p_StartAddr) == FALSE)
            {
              /* Erase the bank */
              while (EraseSector (((nvmOffset + addressOffset) / FLASH_PAGE_SIZE),
                                  SNVMA_NvmConfiguration[nvmIdx].BankSize) == FALSE);

              LOG_ERROR_SYSTEM("\r\nSNVMA_Init - Corrupted banks erases [IsCrcOk]");
            }
            /* Valid bank */
            else
            {
              /* Compute buffer addresses in the bank */
              if (SNVMA_NvmConfiguration[nvmIdx].p_BankForRestore == NULL)
              {
                SNVMA_NvmConfiguration[nvmIdx].p_BankForRestore = &SNVMA_BankConfiguration[bankConfIdx];
              }
              else
              {
                /* Get the current bank in use for this NVM */
                p_currentRestoreBank = SNVMA_NvmConfiguration[nvmIdx].p_BankForRestore;

                /* Already have a valid bank in use, determine which bank is the newest */
                SNVMA_NvmConfiguration[nvmIdx].p_BankForRestore = GetNewestBank (p_currentRestoreBank,
                                                                                &SNVMA_BankConfiguration[bankConfIdx]);

                if (SNVMA_NvmConfiguration[nvmIdx].p_BankForRestore == p_currentRestoreBank)
                {
                  /* Erase &SNVMA_BankConfiguration[bankConfIdx] */
                  while (EraseSector (((nvmOffset + addressOffset) / FLASH_PAGE_SIZE),
                                      SNVMA_NvmConfiguration[nvmIdx].BankSize) == FALSE);
                }
                else
                {
                  /* Erase p_currentRestoreBank */
                  while (EraseSector ((((uint32_t)p_currentRestoreBank->p_StartAddr - FLASH_BASE_NS) / FLASH_PAGE_SIZE),
                                      SNVMA_NvmConfiguration[nvmIdx].BankSize) == FALSE);
                }
              }
            }

            /* Determine the next write bank, is there any bank for restore ? */
            if (SNVMA_NvmConfiguration[nvmIdx].p_BankForRestore == NULL)
            {
              /* The bank for write will be the first of list */
              SNVMA_NvmConfiguration[nvmIdx].p_BankForWrite = &SNVMA_NvmConfiguration[nvmIdx].p_BankList[0];
            }
            /* Is the bank to restore the last one in the bank list */
            else if (SNVMA_NvmConfiguration[nvmIdx].p_BankForRestore ==
                    &SNVMA_NvmConfiguration[nvmIdx].p_BankList[SNVMA_NvmConfiguration[nvmIdx].BankNumber - 1])
            {
              /* The bank for write will be the first of list */
              SNVMA_NvmConfiguration[nvmIdx].p_BankForWrite = &SNVMA_NvmConfiguration[nvmIdx].p_BankList[0];
            }
            else
            {
              /* The bank for write will be the one after the restore list */
              SNVMA_NvmConfiguration[nvmIdx].p_BankForWrite = SNVMA_NvmConfiguration[nvmIdx].p_BankForRestore + 1;
            }

            /* Add the bank size to the address offset */
            addressOffset = addressOffset + ((SNVMA_NvmConfiguration[nvmIdx].BankSize * FLASH_PAGE_SIZE));

            /* Update bank conf index */
            bankConfIdx++;
          }
        }
      }

      if (error == SNVMA_ERROR_NOK)
      {
        /* Init is over, all OK */
        SNVMA_ModuleInit = TRUE;

        error = SNVMA_ERROR_OK;
      }

      /* Leave critical section */
      UTILS_EXIT_CRITICAL_SECTION ();
    }
  }

  return error;
}

SNVMA_Cmd_Status_t SNVMA_Register (const SNVMA_BufferId_t BufferId,
                                   const uint32_t * p_BufferAddress,
                                   const uint32_t BufferSize)
{
  SNVMA_Cmd_Status_t error = SNVMA_ERROR_NOK;

  uint8_t idxBuf = 0x00;
  uint8_t nvmId = (uint8_t)(BufferId / SNVMA_MAX_NUMBER_BUFFER);

  uint32_t offSet = sizeof (SNVMA_BankHeader_t);
  uint32_t neededSpace = SNVMA_ALIGN_128 ((BufferSize * sizeof (uint32_t)));

  /* Check if Module is initialized */
  if (SNVMA_ModuleInit == FALSE)
  {
    error = SNVMA_ERROR_NOT_INIT;
  }
  /* Check if there is no command pending */
  else if (SNVMA_CommandPending == TRUE)
  {
    error = SNVMA_ERROR_CMD_PENDING;
  }
  /* Check if buffer ID is in the configuration range */
  else if (BufferId >= SNVMA_BufferId_Max)
  {
    error = SNVMA_ERROR_BUFFERID_NOT_KNOWN;
  }
  /* Check for NULL buffer pointer */
  else if (p_BufferAddress == NULL)
  {
    error = SNVMA_ERROR_BUFFER_NULL;
  }
  /* Check buffer alignment - Shall be 32bits */
  else if (((uint32_t)p_BufferAddress & SNVMA_MASK_ALIGNMENT_32) != 0x00000000u)
  {
    error = SNVMA_ERROR_BUFFER_NOT_ALIGNED;
  }
  /* Check buffer size */
  else if (BufferSize == 0x00)
  {
    error = SNVMA_ERROR_BUFFER_SIZE;
  }
  /* Check if registration is still possible */
  else if ((SNVMA_NvmConfiguration[nvmId].a_Buffers[0].p_Addr != NULL) &&
           (SNVMA_NvmConfiguration[nvmId].a_Buffers[1].p_Addr != NULL) &&
           (SNVMA_NvmConfiguration[nvmId].a_Buffers[2].p_Addr != NULL) &&
           (SNVMA_NvmConfiguration[nvmId].a_Buffers[3].p_Addr != NULL))
  {
    error = SNVMA_ERROR_NVM_BUFFER_FULL;
  }
  else
  {
    /* Check if registration will overflow the bank */
    for (uint8_t cnt = 0x00;
         (cnt < SNVMA_MAX_NUMBER_BUFFER) && (error == SNVMA_ERROR_NOK);
         cnt++)
    {
      /* Compute occupied space */
      if (SNVMA_NvmConfiguration[nvmId].a_Buffers[cnt].p_Addr != NULL)
      {
        neededSpace = neededSpace + SNVMA_ALIGN_128 ((SNVMA_NvmConfiguration[nvmId].a_Buffers[cnt].Size *
                                                      sizeof (uint32_t)));

        if (((neededSpace + offSet)) > (SNVMA_NvmConfiguration[nvmId].BankSize * FLASH_PAGE_SIZE))
        {
          error = SNVMA_ERROR_BUFFER_SIZE;
        }
      }
    }

    if (error == SNVMA_ERROR_NOK)
    {
      /* Enter critical section */
      UTILS_ENTER_CRITICAL_SECTION();

      /* Compute the index in the buffer array */
      idxBuf = BufferId % SNVMA_MAX_NUMBER_BUFFER;

      /* Register the buffer */
      SNVMA_NvmConfiguration[nvmId].a_Buffers[idxBuf].p_Addr = (uint32_t *)p_BufferAddress;
      SNVMA_NvmConfiguration[nvmId].a_Buffers[idxBuf].Size = BufferSize;

      /* Update buffer write addr for the whole bank */
      for (uint8_t cnt = 0x00;
           cnt < SNVMA_MAX_NUMBER_BUFFER;
           cnt++)
      {
        if (SNVMA_NvmConfiguration[nvmId].a_Buffers[cnt].p_Addr != NULL)
        {
          /* Compute buffer address in bank - It shall be align 128bits */
          SNVMA_NvmConfiguration[nvmId].p_BankForWrite->ap_BufferAddr[cnt] =
            (uint32_t *)(((uint32_t)SNVMA_NvmConfiguration[nvmId].p_BankForWrite->p_StartAddr) + offSet);

          /* Update offset value */
          offSet = offSet + SNVMA_ALIGN_128 ((SNVMA_NvmConfiguration[nvmId].a_Buffers[cnt].Size *
                                              sizeof (uint32_t)));
        }
      }

      /* Leave critical section */
      UTILS_EXIT_CRITICAL_SECTION ();

      error = SNVMA_ERROR_OK;
    }
  }

  LOG_ERROR_SYSTEM("\r\nSNVMA_Register returned %d", (uint8_t)error);

  return error;
}

SNVMA_Cmd_Status_t SNVMA_Restore (const SNVMA_BufferId_t BufferId)
{
  SNVMA_Cmd_Status_t error = SNVMA_ERROR_NOK;

  uint8_t nvmId =  (uint8_t)(BufferId / SNVMA_MAX_NUMBER_BUFFER);
  uint8_t idxBuf = (uint8_t)(BufferId % SNVMA_MAX_NUMBER_BUFFER);

  uint32_t * p_bufferFlashAddr = NULL;
  uint32_t paddingOffset = 0x00;

  SNVMA_BankHeader_t * p_bankHeader = NULL;

  /* Check if initialized */
  if (SNVMA_ModuleInit == FALSE)
  {
    error = SNVMA_ERROR_NOT_INIT;
  }
  /* Check if there is no command pending */
  else if (SNVMA_CommandPending == TRUE)
  {
    error = SNVMA_ERROR_CMD_PENDING;
  }
  /* Check buffer id */
  else if (BufferId >= SNVMA_BufferId_Max)
  {
    error = SNVMA_ERROR_BUFFERID_NOT_KNOWN;
  }
  /* Check if buffer is registered */
  else if (SNVMA_NvmConfiguration[nvmId].a_Buffers[idxBuf].p_Addr == NULL)
  {
    error = SNVMA_ERROR_BUFFERID_NOT_REGISTERED;
  }
  /* Check if there is a bank in use - To restore from */
  else if (SNVMA_NvmConfiguration[nvmId].p_BankForRestore == NULL)
  {
    error = SNVMA_ERROR_NVM_BANK_EMPTY;
  }
  else
  {
    /* Enter critical section */
    UTILS_ENTER_CRITICAL_SECTION();

    /* Set that a command is pending */
    SNVMA_CommandPending = TRUE;

    /* Check bank integrity - Header plausibility */
    if (IsHeaderOk (SNVMA_NvmConfiguration[nvmId].p_BankForRestore->p_StartAddr,
                    nvmId) == FALSE)
    {
      error = SNVMA_ERROR_NVM_BANK_CORRUPTED;
    }
    /* Check bank integrity - CRC value */
    else if (IsCrcOk (SNVMA_NvmConfiguration[nvmId].p_BankForRestore->p_StartAddr) == FALSE)
    {
      error = SNVMA_ERROR_NVM_BANK_CORRUPTED;
    }
    else
    {
      /* Get the bank header */
      p_bankHeader = (SNVMA_BankHeader_t *)SNVMA_NvmConfiguration[nvmId].p_BankForRestore->p_StartAddr;

      /* Compute the address of the buffer in flash */
      if ((BufferId == p_bankHeader->BufferId1) &&
          (SNVMA_NvmConfiguration[nvmId].a_Buffers[idxBuf].Size == p_bankHeader->SizeId1))
      {
        paddingOffset = sizeof (SNVMA_BankHeader_t);
      }
      else if ((BufferId == p_bankHeader->BufferId2) &&
               (SNVMA_NvmConfiguration[nvmId].a_Buffers[idxBuf].Size == p_bankHeader->SizeId2))
      {
        paddingOffset = sizeof (SNVMA_BankHeader_t) +
                        SNVMA_ALIGN_128((p_bankHeader->SizeId1 * sizeof (uint32_t)));

      }
      else if ((BufferId == p_bankHeader->BufferId3) &&
               (SNVMA_NvmConfiguration[nvmId].a_Buffers[idxBuf].Size == p_bankHeader->SizeId3))
      {
        paddingOffset = sizeof (SNVMA_BankHeader_t) +
                        SNVMA_ALIGN_128((p_bankHeader->SizeId1 * sizeof (uint32_t))) +
                        SNVMA_ALIGN_128((p_bankHeader->SizeId2 * sizeof (uint32_t)));
      }
      else if ((BufferId == p_bankHeader->BufferId4) &&
               (SNVMA_NvmConfiguration[nvmId].a_Buffers[idxBuf].Size == p_bankHeader->SizeId4))
      {
        paddingOffset = sizeof (SNVMA_BankHeader_t) +
                        SNVMA_ALIGN_128((p_bankHeader->SizeId1 * sizeof (uint32_t))) +
                        SNVMA_ALIGN_128((p_bankHeader->SizeId2 * sizeof (uint32_t))) +
                        SNVMA_ALIGN_128((p_bankHeader->SizeId3 * sizeof (uint32_t)));
      }
      else
      {
        /* Buffer config in header is not the same as the one requested/registered */
        error = SNVMA_ERROR_BUFFER_CONFIG_MISSMATCH;
      }

      /* All OK, proceed to restore */
      if (error == SNVMA_ERROR_NOK)
      {
        p_bufferFlashAddr = (uint32_t *)((uint32_t)SNVMA_NvmConfiguration[nvmId].p_BankForRestore->p_StartAddr +
                                         paddingOffset);

        /* Restore the data in RAM */
        memcpy ((void *)SNVMA_NvmConfiguration[nvmId].a_Buffers[idxBuf].p_Addr,
                (void *)p_bufferFlashAddr,
                (SNVMA_NvmConfiguration[nvmId].a_Buffers[idxBuf].Size * sizeof (uint32_t)));

        error = SNVMA_ERROR_OK;
      }
    }

    /* Release the pending flag */
    SNVMA_CommandPending = FALSE;

    /* Leave critical section */
    UTILS_EXIT_CRITICAL_SECTION ();
  }

  LOG_ERROR_SYSTEM("\r\nSNVMA_Restore returned %d", (uint8_t)error);

  return error;
}

SNVMA_Cmd_Status_t SNVMA_Write (const SNVMA_BufferId_t BufferId,
                                void (* Callback) (SNVMA_Callback_Status_t))
{
  SNVMA_Cmd_Status_t error = SNVMA_ERROR_NOK;

  uint8_t nvmId =  (uint8_t)(BufferId / SNVMA_MAX_NUMBER_BUFFER);
  uint8_t idxBuf = (uint8_t)(BufferId % SNVMA_MAX_NUMBER_BUFFER);

  /* Check if initialized */
  if (SNVMA_ModuleInit == FALSE)
  {
    error = SNVMA_ERROR_NOT_INIT;
  }
  /* Check buffer id */
  else if (BufferId >= SNVMA_BufferId_Max)
  {
    error = SNVMA_ERROR_BUFFERID_NOT_KNOWN;
  }
  /* Check if buffer is registered */
  else if (SNVMA_NvmConfiguration[nvmId].a_Buffers[idxBuf].p_Addr == NULL)
  {
    error = SNVMA_ERROR_BUFFERID_NOT_REGISTERED;
  }
  else
  {
    /* Enter critical section */
    UTILS_ENTER_CRITICAL_SECTION();

    /* Set the impacted NVM */
    SNVMA_IdBitmask |= (1u << nvmId);

    LOG_INFO_SYSTEM("\r\nSNVMA_Write - Impacted NVM : %d", SNVMA_IdBitmask);

    /* Store the pending buffer ... */
    SNVMA_NvmConfiguration[nvmId].PendingBufferWriteOp |= (1u << idxBuf);

    LOG_INFO_SYSTEM("\r\nSNVMA_Write - Pending buffer : %d", (uint8_t)(1u << idxBuf));

    /* ... and the callback - Can be NULL */
    SNVMA_NvmConfiguration[nvmId].a_Callback[idxBuf] = Callback;

    /* Leave critical section */
    UTILS_EXIT_CRITICAL_SECTION ();

    /* Check if there is only one operation on going */
    if (SNVMA_CommandPending == FALSE)
    {
      /* Enter critical section */
      UTILS_ENTER_CRITICAL_SECTION();

      /* Set that a command is pending */
      SNVMA_CommandPending = TRUE;
      
      /* Flash op started */
      SNVMA_FlashInfo.NvmId = nvmId;
      SNVMA_FlashInfo.BufferId = idxBuf;
      SNVMA_FlashInfo.FlashOpState = SNVMA_HEADER_WRITE;

      /* Set request active */
      SNVMA_NvmConfiguration[nvmId].PendingBufferWriteOp =
        (SNVMA_NvmConfiguration[nvmId].PendingBufferWriteOp << SNVMA_MAX_NUMBER_BUFFER) & 0xF0u;

      /* Leave critical section */
      UTILS_EXIT_CRITICAL_SECTION ();

      /* Check flash write status */
      if (StartFlashWrite (nvmId) == FM_ERROR)
      {
        /* Enter critical section */
        UTILS_ENTER_CRITICAL_SECTION();

        /* Reset command pending flag */
        SNVMA_CommandPending = FALSE;

        /* Clean flags */
        SNVMA_IdBitmask &= ~(1u << nvmId);
        SNVMA_NvmConfiguration[nvmId].PendingBufferWriteOp = 0u;
        
        /* Clean flash operation information */
        memset ((void *)&SNVMA_FlashInfo,
                0x00,
                sizeof (SNVMA_FlashOpInfo_t));

        for (uint8_t cnt = 0x00;
             cnt < SNVMA_MAX_NUMBER_BUFFER;
             cnt++)
        {
          SNVMA_NvmConfiguration[nvmId].a_Callback[cnt] = NULL;
        }

        /* Leave critical section */
        UTILS_EXIT_CRITICAL_SECTION ();

        error = SNVMA_ERROR_FLASH_ERROR;
      }
      else
      {  
        LOG_INFO_SYSTEM("\r\nSNVMA_Write - Flash operation started (Header write request) : %d", (uint8_t)SNVMA_NvmConfiguration[nvmId].PendingBufferWriteOp);

        error = SNVMA_ERROR_OK;
      }
    }
    else
    {
      /* Request information are registered, it will dealt later on */
      error = SNVMA_ERROR_OK;
    }
  }

  return error;
}

/* Callback Definition ------------------------------------------------------*/
void SNVMA_FlashManagerCallback(FM_FlashOp_Status_t Status)
{  
  FM_Cmd_Status_t flashFunRet = FM_ERROR;

  static uint8_t buffCnt = 0x00;

  static SNVMA_BankElt_t * tmpBank = NULL;

  /* Check Flash operation state */
  switch (SNVMA_FlashInfo.FlashOpState)
  {
    case SNVMA_HEADER_WRITE:
    {
      LOG_INFO_SYSTEM("\r\nSNVMA_FlashManagerCallback - Flash operation state : SNVMA_HEADER_WRITE");

      /* Check flash operation status */
      if (Status == FM_OPERATION_COMPLETE)
      {
        /* Check that header has been written correctly */
        if (IsSameContent ((uint32_t *)&SNVMA_WriteBankHeader,
                           (uint32_t *)SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite->p_StartAddr,
                           (sizeof (SNVMA_BankHeader_t) / sizeof (uint32_t))) == FALSE)
        {
          /* Enter critical section */
          UTILS_ENTER_CRITICAL_SECTION();

          /* Reschedule the header write operation but first erase the bank */
          SNVMA_FlashInfo.FlashOpState = SNVMA_RETRY_WRITE;

          /* Leave critical section */
          UTILS_EXIT_CRITICAL_SECTION ();

          flashFunRet = FM_Erase ((((uint32_t)
                                    SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].
                                      p_BankForWrite->p_StartAddr - FLASH_BASE_NS) / FLASH_PAGE_SIZE),
                                    SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].BankSize,
                                    &SNVMA_FlashCallback);

          /* Check flash operation */
          if (flashFunRet == FM_ERROR)
          {
            /* Notify buffers callbacks */
            InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_FAILED);

            /* Enter critical section */
            UTILS_ENTER_CRITICAL_SECTION();

            /* Clear the NVM bitmask */
            SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

            /* Reset command pending flag */
            SNVMA_CommandPending = FALSE;

            /* Leave critical section */
            UTILS_EXIT_CRITICAL_SECTION ();
          }
        }
        else
        {
          /* Enter critical section */
          UTILS_ENTER_CRITICAL_SECTION();
          
          /* Header has been properly written, update operation information */
          SNVMA_FlashInfo.FlashOpState = SNVMA_BUFFER_WRITE;

          /* Reset the buffer write counter */
          buffCnt = 0x00;

          /* Leave critical section */
          UTILS_EXIT_CRITICAL_SECTION ();

          /* Execute the first buffer write */
          flashFunRet = FM_Write (
            SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].a_Buffers[SNVMA_FlashInfo.BufferId].p_Addr,
            SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite->ap_BufferAddr[SNVMA_FlashInfo.BufferId],
            SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].a_Buffers[SNVMA_FlashInfo.BufferId].Size,
            &SNVMA_FlashCallback);

          /* Check flash operation */
          if (flashFunRet == FM_ERROR)
          {
            /* Notify buffers callbacks */
            InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_FAILED);

            /* Enter critical section */
            UTILS_ENTER_CRITICAL_SECTION();

            /* Clear the NVM bitmask */
            SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

            /* Reset command pending flag */
            SNVMA_CommandPending = FALSE;

            /* Leave critical section */
            UTILS_EXIT_CRITICAL_SECTION ();
          }
        }
      }
      /* Status == FM_OPERATION_AVAILABLE */
      else
      {
        LOG_INFO_SYSTEM("\r\nSNVMA_FlashManagerCallback - Flash operation state : SNVMA_RETRY_WRITE - Retry write operation");

        /* Retry write operation of the header */
        flashFunRet = FM_Write ((uint32_t *)&SNVMA_WriteBankHeader,
                                SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite->p_StartAddr,
                                (sizeof (SNVMA_BankHeader_t) / sizeof (uint32_t)),
                                &SNVMA_FlashCallback);

        /* Check flash operation */
        if (flashFunRet == FM_ERROR)
        {
          /* Notify buffers callbacks */
          InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_FAILED);

          /* Enter critical section */
          UTILS_ENTER_CRITICAL_SECTION();

          /* Clear the NVM bitmask */
          SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

          /* Reset command pending flag */
          SNVMA_CommandPending = FALSE;

          /* Leave critical section */
          UTILS_EXIT_CRITICAL_SECTION ();
        }
      }

      break;
    }

    case SNVMA_BUFFER_WRITE:
    {
      LOG_INFO_SYSTEM("\r\nSNVMA_FlashManagerCallback - Flash operation state : SNVMA_BUFFER_WRITE");

      /* Check flash operation status */
      if (Status == FM_OPERATION_COMPLETE)
      {
        /* Enter critical section */
        UTILS_ENTER_CRITICAL_SECTION();

        /* Update buffer Id */
        do
        {
          /* Search for the buffer to write - Not null */
          if (SNVMA_FlashInfo.BufferId < (SNVMA_MAX_NUMBER_BUFFER - 1))
          {
            SNVMA_FlashInfo.BufferId++;
          }
          else
          {
            SNVMA_FlashInfo.BufferId = 0x00;
          }

          buffCnt++;
        } while ((SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].a_Buffers[SNVMA_FlashInfo.BufferId].p_Addr == NULL) &&
                 (buffCnt < SNVMA_MAX_NUMBER_BUFFER));

        /* Leave critical section */
        UTILS_EXIT_CRITICAL_SECTION ();

        /* Check if there is still some buffer to write */
        if (buffCnt < SNVMA_MAX_NUMBER_BUFFER)
        {
          flashFunRet = FM_Write (
            SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].a_Buffers[SNVMA_FlashInfo.BufferId].p_Addr,
            SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite->ap_BufferAddr[SNVMA_FlashInfo.BufferId],
            SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].a_Buffers[SNVMA_FlashInfo.BufferId].Size,
            &SNVMA_FlashCallback);

          /* Check flash operation */
          if (flashFunRet == FM_ERROR)
          {
            /* Notify buffers callbacks */
            InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_FAILED);

            /* Enter critical section */
            UTILS_ENTER_CRITICAL_SECTION();

            /* Clear the NVM bitmask */
            SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

            /* Reset command pending flag */
            SNVMA_CommandPending = FALSE;

            /* Leave critical section */
            UTILS_EXIT_CRITICAL_SECTION ();
          }
        }
        /* Buffer write is over */
        else
        {
          /* Check the integrity of the whole write operation */
          if (IsCrcOk (SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite->p_StartAddr) == FALSE)
          {
            /* Enter critical section */
            UTILS_ENTER_CRITICAL_SECTION();

            /* Reschedule the whole write operation but first erase the bank */
            SNVMA_FlashInfo.FlashOpState = SNVMA_RETRY_WRITE;

            /* Leave critical section */
            UTILS_EXIT_CRITICAL_SECTION ();

            flashFunRet = FM_Erase ((((uint32_t)
                                      SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite->p_StartAddr -
                                        FLASH_BASE_NS) / FLASH_PAGE_SIZE),
                                      SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].BankSize,
                                      &SNVMA_FlashCallback);

            /* Check flash operation */
            if (flashFunRet == FM_ERROR)
            {
              /* Notify buffers callbacks */
              InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_FAILED);

              /* Enter critical section */
              UTILS_ENTER_CRITICAL_SECTION();

              /* Clear the NVM bitmask */
              SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

              /* Reset command pending flag */
              SNVMA_CommandPending = FALSE;

              /* Leave critical section */
              UTILS_EXIT_CRITICAL_SECTION ();
            }
          }
          else
          {
            /* Enter critical section */
            UTILS_ENTER_CRITICAL_SECTION();

            /* Pursue with a bank swap */
            tmpBank = SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForRestore;

            /* Make the restore bank pointing the new valid bank */
            SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForRestore =
              SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite;

            /* Check if it is the last bank element of the list */
            if (SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite ==
                &SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].
                  p_BankList[(SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].BankNumber - 1)])
            {
              SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite =
                &SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankList[0x00];
            }
            else
            {
              SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite++;
            }

            /* Update buffer addresses from old write bank */
            for (uint8_t cnt = 0x00;
                 cnt < SNVMA_MAX_NUMBER_BUFFER;
                 cnt++)
            {
              if (SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForRestore->ap_BufferAddr[cnt] != NULL)
              {
                SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite->ap_BufferAddr[cnt] =
                  SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForRestore->ap_BufferAddr[cnt] -
                  SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForRestore->p_StartAddr +
                  SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite->p_StartAddr;
              }
            }

            /* Leave critical section */
            UTILS_EXIT_CRITICAL_SECTION ();

            /* Erase the old restore bank */
            if (tmpBank != NULL)
            {
              /* Enter critical section */
              UTILS_ENTER_CRITICAL_SECTION();

              /* Update flash operation information */
              SNVMA_FlashInfo.FlashOpState = SNVMA_ERASE_BANK;
			  
              /* Leave critical section */
              UTILS_EXIT_CRITICAL_SECTION ();
              
              flashFunRet = FM_Erase ((((uint32_t)tmpBank->p_StartAddr - FLASH_BASE_NS) / FLASH_PAGE_SIZE),
                                                  SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].BankSize,
                                                  &SNVMA_FlashCallback);

              /* Check flash operation */
              if (flashFunRet == FM_ERROR)
              {
                /* Notify buffers callbacks */
                InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_FAILED);

                /* Enter critical section */
                UTILS_ENTER_CRITICAL_SECTION();

                /* Clear the NVM bitmask */
                SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

                /* Reset command pending flag */
                SNVMA_CommandPending = FALSE;

                /* Leave critical section */
                UTILS_EXIT_CRITICAL_SECTION ();
              }
            }
            else
            {
              /* Notify buffers callbacks */
              InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_COMPLETE);

              /* Is there any new request ? */
              if (SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp == 0x00)
              {
                /* Enter critical section */
                UTILS_ENTER_CRITICAL_SECTION();

                /* No more action on this NVM, clear the NVM bitmask */
                SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

                /* Leave critical section */
                UTILS_EXIT_CRITICAL_SECTION ();
              }

              /* Check whether there is another pending requests */
              if (SNVMA_IdBitmask != 0x00000000)
              {
                /* Determine which NVM is impacted */
                for (uint8_t cnt = 0x00;
                     cnt < SNVMA_MAX_NUMBER_NVM;
                     cnt++)
                {
                  if ((SNVMA_IdBitmask & (1u << cnt)) != 0x00)
                  {
                    /* Enter critical section */
                    UTILS_ENTER_CRITICAL_SECTION();

                    /* Update flash information */
                    SNVMA_FlashInfo.NvmId = cnt;
                    SNVMA_FlashInfo.FlashOpState = SNVMA_HEADER_WRITE;

                    /* Determine which buffer is impacted */
                    for (uint8_t idx = 0x00;
                        idx < SNVMA_MAX_NUMBER_BUFFER;
                        idx++)
                    {
                      if ((SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp & (1u << idx)) != 0x00)
                      {
                        SNVMA_FlashInfo.BufferId = idx;

                        break;
                      }
                    }

                    /* Leave critical section */
                    UTILS_EXIT_CRITICAL_SECTION ();

                    break;
                  }
                }

                LOG_INFO_SYSTEM("\r\nSNVMA_FlashManagerCallback - Flash operation state : SNVMA_BUFFER_WRITE - Start the pending write operation");

                /* Start the pending write operation */
                flashFunRet = StartFlashWrite (SNVMA_FlashInfo.NvmId);

                /* Check flash operation */
                if (flashFunRet == FM_ERROR)
                {
                  /* Notify buffers callbacks */
                  InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_FAILED);

                  /* Enter critical section */
                  UTILS_ENTER_CRITICAL_SECTION();

                  /* Clear the NVM bitmask */
                  SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

                  /* Reset command pending flag */
                  SNVMA_CommandPending = FALSE;

                  /* Leave critical section */
                  UTILS_EXIT_CRITICAL_SECTION ();
                }
                else
                {
                  /* Enter critical section */
                  UTILS_ENTER_CRITICAL_SECTION();

                  /* Set requests active */
                  SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp |=
                    (SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp << SNVMA_MAX_NUMBER_BUFFER);

                  /* Erase pendings requests */
                  SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp &= 0xF0;

                  /* Leave critical section */
                  UTILS_EXIT_CRITICAL_SECTION ();
                }
              }
              /* No more stuff to do */
              else
              {
                /* Enter critical section */
                UTILS_ENTER_CRITICAL_SECTION();

                /* Reset command pending flag */
                SNVMA_CommandPending = FALSE;

                /* Leave critical section */
                UTILS_EXIT_CRITICAL_SECTION ();
              }
            }
          }
        }
      }
      /* Status == FM_OPERATION_AVAILABLE */
      else
      {
        LOG_INFO_SYSTEM("\r\nSNVMA_FlashManagerCallback - Flash operation state : SNVMA_BUFFER_WRITE - Retry write operation");

        /* Retry the buffer write */
        flashFunRet = FM_Write (
          SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].a_Buffers[SNVMA_FlashInfo.BufferId].p_Addr,
          SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite->ap_BufferAddr[SNVMA_FlashInfo.BufferId],
          SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].a_Buffers[SNVMA_FlashInfo.BufferId].Size,
          &SNVMA_FlashCallback);

        /* Check flash operation */
        if (flashFunRet == FM_ERROR)
        {
          /* Notify buffers callbacks */
          InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_FAILED);

          /* Enter critical section */
          UTILS_ENTER_CRITICAL_SECTION();

          /* Clear the NVM bitmask */
          SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

          /* Reset command pending flag */
          SNVMA_CommandPending = FALSE;

          /* Leave critical section */
          UTILS_EXIT_CRITICAL_SECTION ();
        }
      }
      break;
    }

    case SNVMA_ERASE_BANK:
    {
      LOG_INFO_SYSTEM("\r\nSNVMA_FlashManagerCallback - Flash operation state : SNVMA_ERASE_BANK");

      /* Check flash operation status */
      if (Status == FM_OPERATION_COMPLETE)
      {
        /* Notify buffers callbacks */
        InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_COMPLETE);
        
        /* Is there any new request ? */
        if (SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp == 0x00)
        {
          /* Enter critical section */
          UTILS_ENTER_CRITICAL_SECTION();

          /* No more action on this NVM, clear the NVM bitmask */
          SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

          /* Leave critical section */
          UTILS_EXIT_CRITICAL_SECTION ();
        }

        /* Check whether there is another pending requests */
        if (SNVMA_IdBitmask != 0x00000000)
        {
          /* Determine which NVM is impacted */
          for (uint8_t cnt = 0x00;
                cnt < SNVMA_MAX_NUMBER_NVM;
                cnt++)
          {
            if ((SNVMA_IdBitmask & (1u << cnt)) != 0x00)
            {
              /* Enter critical section */
              UTILS_ENTER_CRITICAL_SECTION();

              /* Update flash information */
              SNVMA_FlashInfo.NvmId = cnt;
              SNVMA_FlashInfo.FlashOpState = SNVMA_HEADER_WRITE;
              
              /* Determine which buffer is impacted */
              for (uint8_t idx = 0x00;
                   idx < SNVMA_MAX_NUMBER_BUFFER;
                   idx++)
              {
                if ((SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp & (1u << idx)) != 0x00)
                {
                  SNVMA_FlashInfo.BufferId = idx;

                  break;
                }
              }

              /* Leave critical section */
              UTILS_EXIT_CRITICAL_SECTION ();

              break;
            }
          }

          LOG_INFO_SYSTEM("\r\nSNVMA_FlashManagerCallback - Flash operation state : SNVMA_ERASE_BANK - Start the pending write operation");

          /* Start the pending write operation */
          flashFunRet = StartFlashWrite (SNVMA_FlashInfo.NvmId);

          /* Check flash operation */
          if (flashFunRet == FM_ERROR)
          {
            /* Notify buffers callbacks */
            InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_FAILED);

            /* Enter critical section */
            UTILS_ENTER_CRITICAL_SECTION();

            /* Clear the NVM bitmask */
            SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

            /* Reset command pending flag */
            SNVMA_CommandPending = FALSE;

            /* Leave critical section */
            UTILS_EXIT_CRITICAL_SECTION ();
          }
          else
          {
            /* Enter critical section */
            UTILS_ENTER_CRITICAL_SECTION();

            /* Set requests active */
            SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp |=
              (SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp << SNVMA_MAX_NUMBER_BUFFER);

            /* Erase pendings requests */
            SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp &= 0xF0;

            /* Leave critical section */
            UTILS_EXIT_CRITICAL_SECTION ();
          }
        }
        /* No more stuff to do */
        else
        {
          /* Enter critical section */
          UTILS_ENTER_CRITICAL_SECTION();

          /* Reset command pending flag */
          SNVMA_CommandPending = FALSE;

          /* Leave critical section */
          UTILS_EXIT_CRITICAL_SECTION ();
        }
      }
      /* Status == FM_OPERATION_AVAILABLE */
      else
      {
        flashFunRet = FM_Erase ((((uint32_t)tmpBank->p_StartAddr - FLASH_PAGE_SIZE) / FLASH_PAGE_SIZE),
                                            SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].BankSize,
                                            &SNVMA_FlashCallback);

        /* Check flash operation */
        if (flashFunRet == FM_ERROR)
        {
          /* Notify buffers callbacks */
          InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_FAILED);

          /* Enter critical section */
          UTILS_ENTER_CRITICAL_SECTION();

          /* Clear the NVM bitmask */
          SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

         /* Reset command pending flag */
          SNVMA_CommandPending = FALSE;

          /* Leave critical section */
          UTILS_EXIT_CRITICAL_SECTION ();
        }
      }

      break;
    }

    case SNVMA_RETRY_WRITE:
    {
      LOG_INFO_SYSTEM("\r\nSNVMA_FlashManagerCallback - Flash operation state : SNVMA_RETRY_WRITE");

      /* Check flash operation status */
      if (Status == FM_OPERATION_COMPLETE)
      {
        /* Enter critical section */
        UTILS_ENTER_CRITICAL_SECTION();

        /* Erase OK, restart the write from the start */
        SNVMA_FlashInfo.FlashOpState = SNVMA_HEADER_WRITE;

        /* Determine which buffer is impacted */
        for (uint8_t idx = 0x00;
              idx < SNVMA_MAX_NUMBER_BUFFER;
              idx++)
        {
          if ((SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp & (1u << idx)) != 0x00)
          {

            SNVMA_FlashInfo.BufferId = idx;

            break;
          }
        }

        /* Leave critical section */
        UTILS_EXIT_CRITICAL_SECTION ();

        /* Retry write operation of the header */
        flashFunRet = StartFlashWrite (SNVMA_FlashInfo.NvmId);

        /* Check flash operation */
        if (flashFunRet == FM_ERROR)
        {
          /* Notify buffers callbacks */
          InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_FAILED);

          /* Enter critical section */
          UTILS_ENTER_CRITICAL_SECTION();

          /* Clear the NVM bitmask */
          SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

          /* Reset command pending flag */
          SNVMA_CommandPending = FALSE;

          /* Leave critical section */
          UTILS_EXIT_CRITICAL_SECTION ();
        }
        else
        {
          /* Enter critical section */
          UTILS_ENTER_CRITICAL_SECTION();

          /* Set requests active */
          SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp |=
            (SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp << SNVMA_MAX_NUMBER_BUFFER);

          /* Erase pendings requests */
          SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].PendingBufferWriteOp &= 0xF0;

          /* Leave critical section */
          UTILS_EXIT_CRITICAL_SECTION ();
        }
      }
      /* Status == FM_OPERATION_AVAILABLE */
      else
      {
        LOG_INFO_SYSTEM("\r\nSNVMA_FlashManagerCallback - Flash operation state : SNVMA_RETRY_WRITE - Retry erase operation");

        /* Retry erase operation */
        flashFunRet = FM_Erase ((((uint32_t)SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].p_BankForWrite->p_StartAddr -
                                            FLASH_PAGE_SIZE) / FLASH_PAGE_SIZE),
                                            SNVMA_NvmConfiguration[SNVMA_FlashInfo.NvmId].BankSize,
                                            &SNVMA_FlashCallback);

        /* Check flash operation */
        if (flashFunRet == FM_ERROR)
        {
          /* Notify buffers callbacks */
          InvokeBufferCallback (SNVMA_FlashInfo.NvmId, SNVMA_OPERATION_FAILED);

          /* Enter critical section */
          UTILS_ENTER_CRITICAL_SECTION();

          /* Clear the NVM bitmask */
          SNVMA_IdBitmask &= ~(1u << SNVMA_FlashInfo.NvmId);

          /* Reset command pending flag */
          SNVMA_CommandPending = FALSE;

          /* Leave critical section */
          UTILS_EXIT_CRITICAL_SECTION ();
        }
      }

      break;
    }

    default:
    {
      /* Do nothing */
      break;
    }
  }
}

/* Private functions Definition ------------------------------------------------------*/

uint8_t IsHeaderOk (const uint32_t * const p_BankStartAddress, const uint8_t NvmId)
{
  uint8_t error = FALSE;

  /* Check all the struct members to verify if the header could be any good */
  /* First, check if it is not erased */
  if ((((SNVMA_BankHeader_t *)p_BankStartAddress)->Empty == (uint8_t)SNVMA_ERASED_CONTENT) &&
      (((SNVMA_BankHeader_t *)p_BankStartAddress)->Counter == (uint8_t)SNVMA_ERASED_CONTENT) &&
      (((SNVMA_BankHeader_t *)p_BankStartAddress)->Crc == SNVMA_ERASED_CONTENT) &&
      (((SNVMA_BankHeader_t *)p_BankStartAddress)->BufferId1 == (uint8_t)SNVMA_ERASED_CONTENT) &&
      (((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId1 == SNVMA_ERASED_CONTENT) &&
      (((SNVMA_BankHeader_t *)p_BankStartAddress)->BufferId2 == (uint8_t)SNVMA_ERASED_CONTENT) &&
      (((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId2 == SNVMA_ERASED_CONTENT) &&
      (((SNVMA_BankHeader_t *)p_BankStartAddress)->BufferId3 == (uint8_t)SNVMA_ERASED_CONTENT) &&
      (((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId3 == SNVMA_ERASED_CONTENT) &&
      (((SNVMA_BankHeader_t *)p_BankStartAddress)->BufferId4 == (uint8_t)SNVMA_ERASED_CONTENT) &&
      (((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId4 == SNVMA_ERASED_CONTENT))
  {
    error = FALSE;
  }
  /* Check that written sizes does not overlap the bank */
  else if ((((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId1 +
            ((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId2 +
            ((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId3 +
            ((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId4) >
           ((SNVMA_NvmConfiguration[NvmId].BankSize * FLASH_PAGE_SIZE) / sizeof (uint32_t)))
  {
    error = FALSE;
  }
  /* Now check IDs range */
  else if (((SNVMA_BankHeader_t *)p_BankStartAddress)->BufferId1 != (NvmId * SNVMA_MAX_NUMBER_BUFFER))
  {
    error = FALSE;
  }
  else if (((SNVMA_BankHeader_t *)p_BankStartAddress)->BufferId2 != ((NvmId * SNVMA_MAX_NUMBER_BUFFER) + 1))
  {
    error = FALSE;
  }
  else if (((SNVMA_BankHeader_t *)p_BankStartAddress)->BufferId3 != ((NvmId * SNVMA_MAX_NUMBER_BUFFER) + 2))
  {
    error = FALSE;
  }
  else if (((SNVMA_BankHeader_t *)p_BankStartAddress)->BufferId4 != ((NvmId * SNVMA_MAX_NUMBER_BUFFER) + 3))
  {
    error = FALSE;
  }
  else
  {
    /* Header can be good */
    error = TRUE;
  }
  
  return error;
}

uint8_t IsCrcOk (const uint32_t * const p_BankStartAddress)
{
  uint8_t error = FALSE;

  uint32_t crcComputedValue = 0x00;
  uint32_t * payloadAddr = (uint32_t *)((uint32_t)(p_BankStartAddress) + sizeof (SNVMA_BankHeader_t));
  uint32_t offSet = 0x00;
  uint32_t cnt = 0x00;
  CRCCTRL_Cmd_Status_t eReturn;

  LOG_INFO_SYSTEM("\r\nStart of CRC computation");

  /* Compute CRC for every buffer */
  while (cnt < SNVMA_MAX_NUMBER_BUFFER)
  {
    switch (cnt)
    {
      case 0:
      {
        if (0x00 != ((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId1)
        {
          eReturn = CRCCTRL_Calculate (&SNVMA_Handle,
                                       payloadAddr,
                                       ((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId1,
                                       &crcComputedValue);

          if (CRCCTRL_OK == eReturn)
          {
            offSet = SNVMA_ALIGN_128(((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId1 * sizeof (uint32_t));

            cnt++;
          }
          else if (CRCCTRL_BUSY == eReturn)
          {
            /* Do nothing */
          }
          else
          {
            Error_Handler();
          }
        }
        else
        {
          cnt++;
        }
        break;
      }

      case 1:
      {
        if (0x00 != ((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId2)
        {
          eReturn = CRCCTRL_Accumulate (&SNVMA_Handle,
                                        (uint32_t *)((uint32_t)payloadAddr + offSet),
                                        ((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId2,
                                        &crcComputedValue);

          if (CRCCTRL_OK == eReturn)
          {
            offSet += SNVMA_ALIGN_128(((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId2 * sizeof (uint32_t));

            cnt++;
          }
          else if (CRCCTRL_BUSY == eReturn)
          {
            /* Do nothing */
          }
          else
          {
            Error_Handler();
          }
        }
        else
        {
          cnt++;
        }
        break;
      }

      case 2:
      {
        if (0x00 != ((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId3)
        {
          eReturn = CRCCTRL_Accumulate (&SNVMA_Handle,
                                        (uint32_t *)((uint32_t)payloadAddr + offSet),
                                        ((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId3,
                                        &crcComputedValue);

          if (CRCCTRL_OK == eReturn)
          {
            offSet += SNVMA_ALIGN_128(((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId3 * sizeof (uint32_t));

            cnt++;
          }
          else if (CRCCTRL_BUSY == eReturn)
          {
            /* Do nothing */
          }
          else
          {
            Error_Handler();
          }
        }
        else
        {
          cnt++;
        }
        break;
      }

      case 3:
      {
        if (0x00 != ((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId4)
        {
          eReturn = CRCCTRL_Accumulate (&SNVMA_Handle,
                                        (uint32_t *)((uint32_t)payloadAddr + offSet),
                                        ((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId4,
                                        &crcComputedValue);

          if (CRCCTRL_OK == eReturn)
          {
            offSet += SNVMA_ALIGN_128(((SNVMA_BankHeader_t *)p_BankStartAddress)->SizeId4 * sizeof (uint32_t));

            cnt++;
          }
          else if (CRCCTRL_BUSY == eReturn)
          {
            /* Do nothing */
          }
          else
          {
            Error_Handler();
          }
        }
        else
        {
          cnt++;
        }
        break;
      }

      default:
      {
        /* Should never be reached */
        break;
      }
    }
  }

  /* Compare the CRC values */
  if (crcComputedValue == ((SNVMA_BankHeader_t *)p_BankStartAddress)->Crc)
  {
    error = TRUE;
  }

  LOG_INFO_SYSTEM("\r\nEnd of CRC computation, value : %d", crcComputedValue);

  return error;
}

uint8_t IsSameContent (uint32_t * p_Source, uint32_t * p_Destination, uint32_t Size)
{
  uint8_t error = TRUE;

  /* Compare both contents */
  for (uint32_t cnt = 0x00;
       (cnt < Size) && (error == TRUE);
       cnt++)
  {
    if (p_Source[cnt] != p_Destination[cnt])
    {
      error = FALSE;
    }
  }

  return error;
}

SNVMA_BankElt_t * GetNewestBank (SNVMA_BankElt_t * p_FirstBank,
                                 SNVMA_BankElt_t * p_SecondBank)
{
  SNVMA_BankElt_t * error = p_FirstBank;

  SNVMA_BankHeader_t * p_FirstBankHeader = (SNVMA_BankHeader_t *)p_FirstBank->p_StartAddr;
  SNVMA_BankHeader_t * p_SecondBankHeader = (SNVMA_BankHeader_t *)p_SecondBank->p_StartAddr;

  /* Check which has the greatest counter value */
  if (p_FirstBankHeader->Counter <
      p_SecondBankHeader->Counter)
  {
    if ((p_SecondBankHeader->Counter -
         p_FirstBankHeader->Counter) != 0xFF)
    {
      error = p_SecondBank;
    }
  }

  return error;
}

uint8_t EraseSector (const uint32_t SectorId, const uint32_t SectorNumber)
{
  uint8_t error = FALSE;

  uint32_t sectorIdx = 0x00;

  FD_FlashOp_Status_t funcError = FD_FLASHOP_SUCCESS;

  if (0u != SectorNumber)
  {
    /* Setup erase index */
    sectorIdx = (SectorNumber - 1u);
    
    HAL_FLASH_Unlock();

    while ((SectorNumber > sectorIdx) && (FD_FLASHOP_SUCCESS == funcError))
    {
      funcError = FD_EraseSectors ((SectorId + sectorIdx));

      if (FD_FLASHOP_SUCCESS == funcError)
      {
        sectorIdx++;
      }
    }

    HAL_FLASH_Lock();

    /* Check if operation OK */
    if ((SectorNumber <= sectorIdx) &&
        (FD_FLASHOP_SUCCESS == funcError))
    {
      error = TRUE;
    }
  }

  return error;
}

FM_Cmd_Status_t StartFlashWrite (const uint8_t NvmId)
{
  FM_Cmd_Status_t error = FM_ERROR;

  uint32_t crcValue = 0x00;
  uint32_t cnt = 0x00;
  CRCCTRL_Cmd_Status_t eReturn;

  /* Reset value of SNVMA_BankHeader */
  memset ((void *)&SNVMA_WriteBankHeader,
          0x00,
          sizeof (SNVMA_BankHeader_t));

  /* Compute CRC for every buffer */
  while (cnt < SNVMA_MAX_NUMBER_BUFFER)
  {
    if (SNVMA_NvmConfiguration[NvmId].a_Buffers[cnt].p_Addr != NULL)
    {
      /* First crc computation */
      if (crcValue == 0x00)
      {
        eReturn = CRCCTRL_Calculate (&SNVMA_Handle,
                                      SNVMA_NvmConfiguration[NvmId].a_Buffers[cnt].p_Addr,
                                      SNVMA_NvmConfiguration[NvmId].a_Buffers[cnt].Size,
                                      &crcValue);

        if (CRCCTRL_OK == eReturn)
        {
          cnt++;
        }
        else if (CRCCTRL_BUSY == eReturn)
        {
          continue;
        }
        else
        {
          Error_Handler();
        }
      }
      else
      {
        eReturn = CRCCTRL_Accumulate (&SNVMA_Handle,
                                      SNVMA_NvmConfiguration[NvmId].a_Buffers[cnt].p_Addr,
                                      SNVMA_NvmConfiguration[NvmId].a_Buffers[cnt].Size,
                                      &crcValue);

        if (CRCCTRL_OK == eReturn)
        {
          cnt++;
        }
        else if (CRCCTRL_BUSY == eReturn)
        {
          continue;
        }
        else
        {
          Error_Handler();
        }
      }
    }
    else
    {
      /* Keep going */
      cnt++;
    }
  }

  /* -- Build the header -- */

  /* Enter critical section */
  UTILS_ENTER_CRITICAL_SECTION();

  /* Check if there is an old bank to pursue counter value */
  if (SNVMA_NvmConfiguration[NvmId].p_BankForRestore != NULL)
  {
    SNVMA_WriteBankHeader.Counter = (((SNVMA_BankHeader_t *)
                                      (SNVMA_NvmConfiguration[NvmId].p_BankForRestore->p_StartAddr))->Counter) + 1;
  }

  /* Update CRC value */
  SNVMA_WriteBankHeader.Crc = (uint16_t)(crcValue & 0x0000FFFF);

  /* Update buffers info */
  SNVMA_WriteBankHeader.BufferId1 = (NvmId * SNVMA_MAX_NUMBER_BUFFER);
  SNVMA_WriteBankHeader.SizeId1 = SNVMA_NvmConfiguration[NvmId].a_Buffers[0x00].Size;
  SNVMA_WriteBankHeader.BufferId2 = (NvmId * SNVMA_MAX_NUMBER_BUFFER) + 1u;
  SNVMA_WriteBankHeader.SizeId2 = SNVMA_NvmConfiguration[NvmId].a_Buffers[0x01].Size;
  SNVMA_WriteBankHeader.BufferId3 = (NvmId * SNVMA_MAX_NUMBER_BUFFER) + 2u;
  SNVMA_WriteBankHeader.SizeId3 = SNVMA_NvmConfiguration[NvmId].a_Buffers[0x02].Size;
  SNVMA_WriteBankHeader.BufferId4 = (NvmId * SNVMA_MAX_NUMBER_BUFFER) + 3u;
  SNVMA_WriteBankHeader.SizeId4 = SNVMA_NvmConfiguration[NvmId].a_Buffers[0x03].Size;

  /* Leave critical section */
  UTILS_EXIT_CRITICAL_SECTION ();
    
  /* Write the header */
  error = FM_Write ((uint32_t *)&SNVMA_WriteBankHeader,
                                SNVMA_NvmConfiguration[NvmId].p_BankForWrite->p_StartAddr,
                                (sizeof (SNVMA_BankHeader_t) / sizeof (uint32_t)),
                                &SNVMA_FlashCallback);

  return error;
}

void InvokeBufferCallback (const uint8_t NvmId, const SNVMA_Callback_Status_t CallbackStatus)
{
  uint8_t pendingShift = 0x00;
  uint8_t tmpMask = 0x00;

  for (pendingShift = 0x00;
       ((SNVMA_NvmConfiguration[NvmId].PendingBufferWriteOp != 0x00) &&
        (pendingShift < SNVMA_MAX_NUMBER_BUFFER));
       pendingShift++)
  {
    tmpMask = SNVMA_NvmConfiguration[NvmId].PendingBufferWriteOp &
              (1u << (pendingShift + SNVMA_MAX_NUMBER_BUFFER));

    /* Is there an active write request ? */
    if (tmpMask != 0x00)
    {
      /* Has a new request come ? */
      if ((SNVMA_NvmConfiguration[NvmId].PendingBufferWriteOp & (1u << pendingShift)) == 0x00)
      {
        if (SNVMA_NvmConfiguration[NvmId].a_Callback[pendingShift] != NULL)
        {
          /* Invoke callback */
          SNVMA_NvmConfiguration[NvmId].a_Callback[pendingShift] (CallbackStatus);

          LOG_INFO_SYSTEM("\r\nSNVMA - InvokeBufferCallback for NVM ID : %d\n", NvmId);

          /* Enter critical section */
          UTILS_ENTER_CRITICAL_SECTION();

          /* Clear callback list */
          SNVMA_NvmConfiguration[NvmId].a_Callback[pendingShift] = NULL;

          /* Leave critical section */
          UTILS_EXIT_CRITICAL_SECTION ();
        }

        /* Enter critical section */
        UTILS_ENTER_CRITICAL_SECTION();

        /* Remove this pending buffer from the active list */
        SNVMA_NvmConfiguration[NvmId].PendingBufferWriteOp &= ~(1u << (pendingShift + SNVMA_MAX_NUMBER_BUFFER));

        /* Leave critical section */
        UTILS_EXIT_CRITICAL_SECTION ();
      }
      else
      {
        /* Do not notify, a retry shall be planned */
      }
    }
  }
}
