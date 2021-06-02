/*! *********************************************************************************
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2019 NXP
 * All rights reserved.
 *
 * \file
 *
 * This is a source file which implements the driver for the MX25R8035F memory.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */
#include "Eeprom.h"
#include "EmbeddedTypes.h"
#include "FunctionLib.h"

#include "Panic.h"
#include "fsl_os_abstraction.h"

#if gEepromType_d == gEepromDevice_MX25R8035F_c

#include "fsl_spifi.h"
#if gLoggingActive_d
#include "dbg_logging.h"
#endif
#include "fsl_debug_console.h"

/*! *********************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
********************************************************************************** */
#ifndef gEepromWriteEnable_d
#define gEepromWriteEnable_d 1
#endif

static osaSemaphoreId_t mExtEepromSemaphoreId;

#define IS_WORD_ALIGNED(x) (((uint32_t)(x) &0x3) == 0)
#define IS_PAGE_ALIGNED(x) (((uint32_t)(x) & (EEPROM_PAGE_SIZE - 1)) == 0)
#define SECTOR_ADDR(x) (((uint32_t)(x) / EEPROM_SECTOR_SIZE) * EEPROM_SECTOR_SIZE)
#define IS_SECTOR_ALIGNED(x) (((uint32_t)(x) & (EEPROM_SECTOR_SIZE - 1)) == 0)
#define SECTOR_NUMBER(x) ((uint32_t)(x) / EEPROM_SECTOR_SIZE)

#define BLOCK_ADDR(x) (((uint32_t)(x) / EEPROM_BLOCK_SIZE) * EEPROM_BLOCK_SIZE)
#define IS_BLOCK_ALIGNED(x) (((uint32_t)(x) & (EEPROM_BLOCK_SIZE - 1)) == 0)
#define BLOCK_NUMBER(x) ((uint32_t)(x) / EEPROM_BLOCK_SIZE)

#define EEPROM_BLOCK2_SIZE (EEPROM_BLOCK_SIZE * 2)
#define BLOCK2_ADDR(x) (((uint32_t)(x) / EEPROM_BLOCK2_SIZE) * EEPROM_BLOCK2_SIZE)
#define IS_BLOCK2_ALIGNED(x) (((uint32_t)(x) & (EEPROM_BLOCK2_SIZE - 1)) == 0)

/* Control Commands */
#define EEPROM_RDSR 0x05
#define EEPROM_RDSFDR 0x5A
#define EEPROM_WREN 0x06
#define EEPROM_WRDI 0x04
#define EEPROM_RDCR 0x15
#define EEPROM_WRSR 0x01
#define EEPROM_RDID 0x9F
#define EEPROM_RES 0xAB
#define EEPROM_REMS 0x90
#define EEPROM_DP 0xB9

/* Read Commands */
#define EEPROM_READ 0x03
#define EEPROM_FAST_READ 0x0B
#define EEPROM_2READ 0xBB
#define EEPROM_DREAD 0x3B
#define EEPROM_4READ 0xEB
#define EEPROM_QREAD 0x6B

/* Erase Commands */
#define EEPROM_ERASE_4K 0x20
#define EEPROM_ERASE_32K 0x52
#define EEPROM_ERASE_64K 0xD8
#define EEPROM_ERASE_ALL 0x60

/* Write Commands */
#define EEPROM_WRITE_PAGE 0x02
#define EEPROM_WRITE_QPAGE 0x38

/* Status Flags */
#define EEPROM_BUSY_FLAG_MASK 0x01
#define EEPROM_WEL_FLAG_MASK 0x02

#define EEPROM_PAGE_SIZE (256)
#define EEPROM_SECTOR_SIZE (4 * 1024)
#define EEPROM_BLOCK_SIZE (32 * 1024)
#define EEPROM_PAGE_MASK (EEPROM_PAGE_SIZE - 1)

/* address mask */
#define ADDRESS_MASK 0x000000FF

/* SPI config */
#ifndef gEepromSpiInstance_c
#define gEepromSpiInstance_c 1
#endif

/* Commands (see command array below) */
#define CMD_SPIFI_DREAD (0)
#define CMD_SPIFI_DPROGRAM_PAGE (1)
#define CMD_SPIFI_QREAD (2)
#define CMD_SPIFI_QPROGRAM_PAGE (3)
#define CMD_SPIFI_GET_STATUS (4)
#define CMD_SPIFI_ERASE_SECTOR (5)
#define CMD_SPIFI_ERASE_BLOCK_32K (6)
#define CMD_SPIFI_ERASE_ALL (7)
#define CMD_SPIFI_WRITE_ENABLE (8)
#define CMD_SPIFI_WRITE_REGISTER (9)
#define CMD_SPIFI_ERASE_BLOCK_64K (10)
#define CMD_SPIFI_READ_ID (11)
#define CMD_SPIFI_DP (12)
#define CMD_SPIFI_RES (13)
#define CMD_SPIFI_NUM (14)

// Flash type: including XT25F08B and MX25R8035F
#define FLASH_MX25R8035F_ID 0x1428C2
#define FLASH_XT25F08B_ID 0x14400B
#define FLASH_UNKNOWN_ID 0xffffff

/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/
#if (gFlashEraseDuringWrite == 1)
static ee_err_t EEPROM_PrepareForWrite(uint32_t NoOfBytes, uint32_t Addr);
#endif
#if gEepromWriteEnable_d
static ee_err_t EEPROM_WritePage(uint32_t NoOfBytes, uint32_t Addr, uint8_t * Outbuf);
static void EEPROM_WriteEnable(void);
#endif
static uint32_t EEPROM_ReadStatusReq(void);
static uint32_t EEPROM_ReadIDReq(void);
static uint32_t EEPROM_ReadResReq(void);

#define MX25_SR_WIP_POS 0 /* Write In Progress */
#define MX25_SR_WEL_POS 1 /* Write Enable Latch */
#define MX25_SR_BP_POS 2  /* Level of Protected block  */
#define MX25_SR_BP_WIDTH 4
#define MX25_SR_BP_MASK (((1 << MX25_SR_BP_WIDTH) - 1) << MX25_SR_BP_POS)
#define MX25_SR_QE_POS 6 /* Non Volatile  */

#define MX25_CR1_TB_POS 3 /* Top-Bottom protection selected */
#define MX25_CR1_DC_POS 6 /* Dummy Cycle */

#define MX25_CR2_LH_POS 1 /* LowPower / HighPerformance  */

#define MX25R8035_CFG_STATUS_QUAD_MODE BIT(MX25_SR_QE_POS)

#define MX25R8035_CFG_REG2_HI_PERF_MODE (BIT(MX25_CR2_LH_POS) << 16)

/*! *********************************************************************************
*************************************************************************************
* Private Memory Declarations
*************************************************************************************
********************************************************************************** */

static uint8_t initialized = 0;

static spifi_command_t command[CMD_SPIFI_NUM] = {
    { EEPROM_PAGE_SIZE, false, kSPIFI_DataInput, 1, kSPIFI_CommandDataQuad, kSPIFI_CommandOpcodeAddrThreeBytes,
      EEPROM_DREAD }, /* DREAD (1I/2O)*/
    { EEPROM_PAGE_SIZE, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeAddrThreeBytes,
      EEPROM_WRITE_PAGE }, /* pp  (dual page program)*/
    { EEPROM_PAGE_SIZE, false, kSPIFI_DataInput, 1, kSPIFI_CommandDataQuad, kSPIFI_CommandOpcodeAddrThreeBytes,
      EEPROM_QREAD }, /* QREAD (1I/4O) */
    { EEPROM_PAGE_SIZE, false, kSPIFI_DataOutput, 0, kSPIFI_CommandOpcodeSerial, kSPIFI_CommandOpcodeAddrThreeBytes,
      EEPROM_WRITE_QPAGE },                                                                            /* 4PP (quad page program) */
    { 1, false, kSPIFI_DataInput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, EEPROM_RDSR }, /* Read status register */
    { 0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeAddrThreeBytes,
      EEPROM_ERASE_4K }, /* Sector erase */
    { 0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeAddrThreeBytes,
      EEPROM_ERASE_32K },                                                                                    /* Block 32k erase */
    { 0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, EEPROM_ERASE_ALL }, /* Chip erase */
    { 0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, EEPROM_WREN },      /* Write enable */
    { 3, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, EEPROM_WRSR }, /* Write Status register */
    { 0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeAddrThreeBytes,
      EEPROM_ERASE_64K },                                                                              /* Block 64k erase */
    { 3, false, kSPIFI_DataInput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, EEPROM_RDID }, /* Read ID register */
    { 0, false, kSPIFI_DataInput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, EEPROM_DP },   /* Low Power */
    { 4, false, kSPIFI_DataInput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly,
      EEPROM_RES }, /* Read  Electronic Signature */

}; /* Write status register */

#if defined gFlashBlockBitmap_d
typedef struct
{
    uint8_t EraseBitmap[gEepromParams_TotalSize_c / EEPROM_SECTOR_SIZE / 8];
} EepromContext_t;

static EepromContext_t mHandle;
#endif

#if gEepromSupportReset
static uint8_t sectorRamBuffer[gEepromParams_SectorSize_c];
#endif

static uint32_t eEpromFlashID = FLASH_UNKNOWN_ID;

/*! *********************************************************************************
*************************************************************************************
* Public Functions
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
 * \brief   This function initializes external memory.
 *
 * \return MEM_SUCCESS_c if initialization is successful. (It's always successful).
 *
 ********************************************************************************** */
ee_err_t EEPROM_Init(void)
{
    ee_err_t status     = ee_ok;
    bool_t resReqCalled = FALSE;

    if ((mExtEepromSemaphoreId == NULL) && ((mExtEepromSemaphoreId = OSA_SemaphoreCreate(1)) == NULL))
    {
        panic(ID_PANIC(0, 0), (uint32_t) EEPROM_Init, 0, 0);
    }

#if defined gFlashBlockBitmap_d
    uint32_t i;

    /* Mark Flash as Unerased */
    for (i = 0; i < NumberOfElements(mHandle.EraseBitmap); i++)
    {
        mHandle.EraseBitmap[i] = 0;
    }
#endif

    if (!initialized)
    {
        EEPROM_DBG_LOG("");

        spifi_config_t config = { 0 };

        /* Enable DIOs */
        BOARD_InitSPIFI();

        /* Initialize SPIFI base driver */
        SPIFI_GetDefaultConfig(&config);

        if (CHIP_USING_SPIFI_DUAL_MODE())
        {
            config.dualMode = kSPIFI_DualMode;
        }
        SPIFI_Init(SPIFI, &config);

        EEPROM_WriteEnable();

        if (eEpromFlashID == FLASH_UNKNOWN_ID)
        {
            /* Read the Electronic Signature
             * It allows to release from Power-Down in case of FLASH_XT25F08B_ID
             */
            EEPROM_ReadResReq();
            eEpromFlashID = EEPROM_ReadIDReq();
            resReqCalled  = TRUE;
        }

        switch (eEpromFlashID)
        {
        case FLASH_XT25F08B_ID:
            /* Read the Electronic Signature
             * It allows to release from Power-Down in case of FLASH_XT25F08B_ID
             */
            if (!resReqCalled)
            {
                EEPROM_ReadResReq();
            }
            command[CMD_SPIFI_WRITE_REGISTER].dataLen = 2;
            SPIFI_SetCommand(SPIFI, &command[CMD_SPIFI_WRITE_REGISTER]);
            SPIFI_WriteDataHalfword(SPIFI, 0x0200);
            break;
        case FLASH_MX25R8035F_ID: {
            SPIFI_SetCommand(SPIFI, &command[CMD_SPIFI_WRITE_REGISTER]);
            /* Enable ultra low power mode */
            uint32_t cfg_word = 0x000000; /* 24 bit register */

            if (config.dualMode == kSPIFI_QuadMode)
            {
                /* insert dummy cycles for Quad mode operation */
                cfg_word |= MX25R8035_CFG_STATUS_QUAD_MODE;
            }
#ifdef gSpiFiHiPerfMode_d
            cfg_word |= MX25R8035_CFG_REG2_HI_PERF_MODE;
#endif
            SPIFI_WritePartialWord(SPIFI, cfg_word, command[CMD_SPIFI_WRITE_REGISTER].dataLen);
        }
        break;
        default:
            status = ee_error;
            break;
        }

        if (status != ee_ok)
        {
            SPIFI_Deinit(SPIFI);
            SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRLSET0_SPIFI_CLK_SET_MASK;
            BOARD_SetSpiFi_LowPowerEnter();
        }
        else
        {
            initialized = 1;
        }
        while (EEPROM_isBusy())
            ;
    }

    return status;
}

/******************************************************************************
 * NAME: EEPROM_DeInit
 * DESCRIPTION: De-Initializes the EEPROM peripheral
 * PARAMETERS: None
 * RETURN: ee_ok - if the EEPROM has been de-initialized successfully
 *         ee_error - otherwise
 ******************************************************************************/
ee_err_t EEPROM_DeInit(void)
{
    if (initialized)
    {
        OSA_SemaphoreWait(mExtEepromSemaphoreId, osaWaitForever_c);

        SPIFI_SetCommand(SPIFI, &command[CMD_SPIFI_DP]);
        SPIFI_Deinit(SPIFI);
        SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRLSET0_SPIFI_CLK_SET_MASK;
        BOARD_SetSpiFi_LowPowerEnter();
        initialized = 0;

        OSA_SemaphorePost(mExtEepromSemaphoreId);
        OSA_SemaphoreDestroy(mExtEepromSemaphoreId);
    }
    return ee_ok;
}

/*! *********************************************************************************
 * \brief   Erase all memory to 0xFF
 *
 * \return ee_err_t.
 *
 ********************************************************************************** */
#if gEepromWriteEnable_d
ee_err_t EEPROM_ChipErase(void)
{
#if defined gFlashBlockBitmap_d
    uint32_t i;
#endif

    EEPROM_DBG_LOG("");

    OSA_SemaphoreWait(mExtEepromSemaphoreId, osaWaitForever_c);

    /* Wait for idle state : check before operation in order to let previous
     * operation terminate rather than blocking */
    while (EEPROM_isBusy())
        ;

    /* Enable write */
    EEPROM_WriteEnable();

    /* Erase command */
    SPIFI_SetCommand(SPIFI, &command[CMD_SPIFI_ERASE_ALL]);

    OSA_SemaphorePost(mExtEepromSemaphoreId);

#if defined gFlashBlockBitmap_d
    /* Mark Flash as erased */
    for (i = 0; i < NumberOfElements(mHandle.EraseBitmap); i++)
    {
        mHandle.EraseBitmap[i] = 0xFF;
    }
#endif

    return ee_ok;
}
#endif

/*! *********************************************************************************
 * \brief   Erase a block of memory to 0xFF
 *
 * \param[in] Addr      Start memory address
 * \param[in] size      The size of the block: either 4096 or 32768
 *
 * \return ee_err_t.
 *
 ********************************************************************************** */
#if gEepromWriteEnable_d
ee_err_t EEPROM_EraseBlock(uint32_t Addr, uint32_t block_size)
{
    uint8_t cmd;
#if defined gFlashBlockBitmap_d
    uint32_t i = Addr / block_size;
#endif

    EEPROM_DBG_LOG("");

    OSA_SemaphoreWait(mExtEepromSemaphoreId, osaWaitForever_c);

    while (EEPROM_isBusy())
        ;

    EEPROM_WriteEnable();

    switch (block_size)
    {
    case EEPROM_SECTOR_SIZE:
        cmd = CMD_SPIFI_ERASE_SECTOR;
#if defined gFlashBlockBitmap_d
        mHandle.EraseBitmap[i / 8] |= 1 << (i % 8);
#endif
        break;
    case EEPROM_BLOCK_SIZE:
        cmd = CMD_SPIFI_ERASE_BLOCK_32K;
#if defined gFlashBlockBitmap_d
        mHandle.EraseBitmap[i] = 0xFF;
#endif
        break;
    case EEPROM_BLOCK2_SIZE:
        cmd = CMD_SPIFI_ERASE_BLOCK_64K;
#if defined gFlashBlockBitmap_d
        i                          = Addr / (block_size / 2);
        mHandle.EraseBitmap[i]     = 0xFF;
        mHandle.EraseBitmap[i + 1] = 0xFF;
#endif
        break;
    default:
        OSA_SemaphorePost(mExtEepromSemaphoreId);
        return ee_error;
    }

    /* Set block address: any address within the block is acceptable to the MX25R8035F */
    SPIFI_SetCommandAddress(SPIFI, FSL_FEATURE_SPIFI_START_ADDR + Addr);

    /* Erase sector or block */
    SPIFI_SetCommand(SPIFI, &command[cmd]);

    OSA_SemaphorePost(mExtEepromSemaphoreId);

    return ee_ok;
}

ee_err_t EEPROM_EraseArea(uint32_t * Addr, int32_t * size, bool non_blocking)
{
    ee_err_t status = ee_error;
    uint32_t sz;
    int32_t remain_sz   = (int32_t) *size;
    uint32_t erase_addr = *Addr;

    EEPROM_DBG_LOG("");

    do
    {
        bool skip = false;

#if defined gFlashBlockBitmap_d
        uint32_t block_nb;
        uint32_t sect_nb;
#endif
        if (!IS_SECTOR_ALIGNED(erase_addr))
        {
            status = ee_error;
            break;
        }
        if (remain_sz == 0)
        {
            status = ee_error;
            break;
        }
        erase_addr = SECTOR_ADDR(Addr);
        for (erase_addr = *Addr; remain_sz > 0;)
        {
            OSA_SemaphoreWait(mExtEepromSemaphoreId, osaWaitForever_c);

            if (non_blocking && EEPROM_isBusy())
            {
                status = ee_busy;
                OSA_SemaphorePost(mExtEepromSemaphoreId);
                break;
            }

            OSA_SemaphorePost(mExtEepromSemaphoreId);

#if defined gFlashBlockBitmap_d
            block_nb = BLOCK_NUMBER(erase_addr);
#endif
            sz = EEPROM_SECTOR_SIZE;

            if ((IS_BLOCK2_ALIGNED(erase_addr) && (remain_sz >= EEPROM_BLOCK2_SIZE))) /* EEPROM_BLOCK_SIZE*2 */
            {
#if defined gFlashBlockBitmap_d
                if ((mHandle.EraseBitmap[block_nb]) != 0xff)
                {
                    sz = EEPROM_BLOCK_SIZE;
                    if ((mHandle.EraseBitmap[block_nb + 1]) != 0xff) /* group 2 consecutive 32k blocks in one single 64k block */
                        sz += EEPROM_BLOCK_SIZE;
                }
                else
                    skip = true;
#else
                sz = EEPROM_BLOCK2_SIZE;
#endif /* gFlashBlockBitmap_d */
            }
            else if ((IS_BLOCK_ALIGNED(erase_addr) && (remain_sz >= EEPROM_BLOCK_SIZE))) /* EEPROM_BLOCK_SIZE */
            {
#if defined gFlashBlockBitmap_d
                if ((mHandle.EraseBitmap[block_nb]) == 0)
                    sz = EEPROM_BLOCK_SIZE;
                else
                    skip = true;
#else
                sz = EEPROM_BLOCK_SIZE;
#endif /* gFlashBlockBitmap_d */
            }
            else /* necessarily 4k aligned */
            {
#if defined gFlashBlockBitmap_d
                sect_nb = SECTOR_NUMBER(erase_addr);
                if ((mHandle.EraseBitmap[block_nb] & (1 << (sect_nb % 8))) == 0)
                    sz = EEPROM_SECTOR_SIZE;
                else
                    skip = true;
#else
#endif /* gFlashBlockBitmap_d */
            }
            if (!skip)
            {
                EEPROM_DBG_LOG("Erasing block Addr=%x sz=%d", erase_addr, sz);
                status = EEPROM_EraseBlock(erase_addr, sz);
                if (status != ee_ok)
                    break;
            }
            erase_addr += sz;
            remain_sz -= sz;
        }
        status = ee_ok;
    } while (0);

    *Addr = erase_addr;
    *size = remain_sz;
    return status;
}

ee_err_t EEPROM_EraseNextBlock(uint32_t Addr, uint32_t size)
{
    ee_err_t status     = ee_error;
    uint8_t alignment   = 0;
    uint32_t block_size = EEPROM_SECTOR_SIZE;
    uint32_t erase_addr;
    EEPROM_DBG_LOG("");

    do
    {
        if (size <= EEPROM_SECTOR_SIZE)
        {
            block_size = EEPROM_SECTOR_SIZE;
            alignment  = 12;
        }
        else if (size <= EEPROM_BLOCK_SIZE)
        {
            block_size = EEPROM_BLOCK_SIZE;
            alignment  = 15;
        }
        else
        {
            block_size = EEPROM_BLOCK2_SIZE;
            alignment  = 16;
        }
        erase_addr = Addr;
        erase_addr = ((erase_addr + (block_size - 1)) >> alignment << alignment);
        if ((size != 0) && (erase_addr < (Addr + size)))
        {
            status = EEPROM_EraseBlock(erase_addr, block_size);
            if (status != ee_ok)
                break;
            // while (EEPROM_isBusy());
        }
        status = ee_ok;
        ;
    } while (0);
    return status;
}

ee_err_t EEPROM_SectorAlignmentAfterReset(uint32_t Addr)
{
    ee_err_t status = ee_error;
#if gEepromSupportReset
    uint32_t nbBytesToCopy = 0;
    do
    {
        /* The address given should be aligned on EEPROM_PAGE_SIZE */
        if (Addr % EEPROM_PAGE_SIZE != 0)
            break;
        nbBytesToCopy = Addr % gEepromParams_SectorSize_c;
        /* If the address is aligned on gEepromParams_SectorSize_c, nothing to do */
        if (nbBytesToCopy != 0)
        {
            /* copy only the necessary bytes in RAM*/
            if (EEPROM_ReadData(nbBytesToCopy, Addr - nbBytesToCopy, sectorRamBuffer) != ee_ok)
                break;
            /* Erase the sector */
            if (EEPROM_EraseBlock(Addr - nbBytesToCopy, EEPROM_SECTOR_SIZE) != ee_ok)
                break;
            /* Write the necessary bytes */
            if (EEPROM_WriteData(nbBytesToCopy, Addr - nbBytesToCopy, sectorRamBuffer) != ee_ok)
                break;
        }
        status = ee_ok;
    } while (0);
#endif
    return status;
}

#endif /* gEepromWriteEnable_d */

/*! *********************************************************************************
 * \brief   Write a data buffer into the external memory, at a given address
 *
 * \param[in] NoOfBytes Number of bytes to write
 * \param[in] Addr      Start memory address
 * \param[in] inbuf     Pointer to the data
 *
 * \return ee_err_t.
 *
 ********************************************************************************** */
#if gEepromWriteEnable_d
ee_err_t EEPROM_WriteData(uint32_t NoOfBytes, uint32_t Addr, uint8_t * Outbuf)
{
    ee_err_t retval = ee_ok;

    uint32_t bytes;

    if (NoOfBytes > 0)
    {
        /* Will erase the sectors needed if they have not been erased during this
         * session. A bit useless as it doesn't cope with an ongoing OTA that spans
         * several sessions and it can't spot when you're overwriting data that is
         * already there */
#if (gFlashEraseDuringWrite == 1)
        retval = EEPROM_PrepareForWrite(NoOfBytes, Addr);
#endif

        if (retval == ee_ok)
        {
            while ((Addr & EEPROM_PAGE_MASK) + NoOfBytes > EEPROM_PAGE_MASK)
            {
                bytes = EEPROM_PAGE_SIZE - (Addr & EEPROM_PAGE_MASK);
                EEPROM_DBG_LOG("");
                retval = EEPROM_WritePage(bytes, Addr, Outbuf);
                NoOfBytes -= bytes;
                Addr += bytes;
                Outbuf += bytes;

                if (retval != ee_ok)
                {
                    break;
                }
            }

            retval = EEPROM_WritePage(NoOfBytes, Addr, Outbuf);
        }
    }
    return retval;
}
#endif /* gEepromWriteEnable_d */

void EEPROM_SetRead(void)
{

    OSA_SemaphoreWait(mExtEepromSemaphoreId, osaWaitForever_c);

    while (EEPROM_isBusy())
        ;

    /* Set start address */
    SPIFI_SetCommandAddress(SPIFI, FSL_FEATURE_SPIFI_START_ADDR);

    /* Enable read */
    if (CHIP_USING_SPIFI_DUAL_MODE())
    {
        SPIFI_SetMemoryCommand(SPIFI, &command[CMD_SPIFI_DREAD]);
    }
    else
    {
        SPIFI_SetMemoryCommand(SPIFI, &command[CMD_SPIFI_QREAD]);
    }

    OSA_SemaphorePost(mExtEepromSemaphoreId);
}

/*! *********************************************************************************
 * \brief   Reads a data buffer from the external memory, from a given address
 *
 * \param[in] NoOfBytes Number of bytes to read
 * \param[in] Addr      Start memory address
 * \param[in] inbuf     Pointer to a location where to store the read data
 *
 * \return ee_err_t.
 *
 ********************************************************************************** */
ee_err_t EEPROM_ReadData(uint16_t NoOfBytes, uint32_t Addr, uint8_t * inbuf)
{

    OSA_SemaphoreWait(mExtEepromSemaphoreId, osaWaitForever_c);

    while (EEPROM_isBusy())
        ;

    /* Set start address */
    SPIFI_SetCommandAddress(SPIFI, FSL_FEATURE_SPIFI_START_ADDR + Addr);

    /* Enable read */
    if (CHIP_USING_SPIFI_DUAL_MODE())
    {
        SPIFI_SetMemoryCommand(SPIFI, &command[CMD_SPIFI_DREAD]);
    }
    else
    {
        SPIFI_SetMemoryCommand(SPIFI, &command[CMD_SPIFI_QREAD]);
    }
    uint8_t * flash_addr = (uint8_t *) (FSL_FEATURE_SPIFI_START_ADDR + Addr);

    FLib_MemCpy((void *) inbuf, (void *) flash_addr, NoOfBytes);

    /* Reset the SPIFI to switch to command mode */
    SPIFI_ResetCommand(SPIFI);

    OSA_SemaphorePost(mExtEepromSemaphoreId);

    return ee_ok;
}

/*! *********************************************************************************
 * \brief   Check if the memory controller is busy
 *
 * \return TRUE/FALSE.
 *
 ********************************************************************************** */
uint8_t EEPROM_isBusy(void)
{
    return (EEPROM_ReadStatusReq() & EEPROM_BUSY_FLAG_MASK) == EEPROM_BUSY_FLAG_MASK;
}

/*! *********************************************************************************
*************************************************************************************
* Private Functions
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
 * \brief   Read the memory controller status register
 *
 * \return status register.
 *
 ********************************************************************************** */
static uint32_t EEPROM_ReadStatusReq(void)
{
    SPIFI_SetCommand(SPIFI, &command[CMD_SPIFI_GET_STATUS]);
    while ((SPIFI->STAT & SPIFI_STAT_INTRQ_MASK) == 0U)
    {
    }
    return SPIFI_ReadPartialWord(SPIFI, command[CMD_SPIFI_GET_STATUS].dataLen);
}

/*! *********************************************************************************
 * \brief   Read ID register
 *
 * \return ID register.
 *
 ********************************************************************************** */
static uint32_t EEPROM_ReadIDReq(void)
{
    SPIFI_SetCommand(SPIFI, &command[CMD_SPIFI_READ_ID]);
    while ((SPIFI->STAT & SPIFI_STAT_INTRQ_MASK) == 0U)
    {
    }
    return SPIFI_ReadPartialWord(SPIFI, command[CMD_SPIFI_READ_ID].dataLen);
}

/*! *********************************************************************************
 * \brief   Read Electronic Signature
 *
 * \return Electronic Signature.
 *
 ********************************************************************************** */
static uint32_t EEPROM_ReadResReq(void)
{
    uint32_t result = 0;
    SPIFI_SetCommand(SPIFI, &command[CMD_SPIFI_RES]);
    while ((SPIFI->STAT & SPIFI_STAT_INTRQ_MASK) == 0U)
    {
    }
    result = SPIFI_ReadPartialWord(SPIFI, command[CMD_SPIFI_RES].dataLen);
    /*
     * The spec of FLASH_XT25F08B_ID requires a time duration of a least 20us before accepting
     * any new command after a release from Power-Down
     */
    CLOCK_uDelay(21);
    return result;
}

/*! *********************************************************************************
 * \brief   Enabled Write/Erase access for the next operation
 *
 ********************************************************************************** */
#if gEepromWriteEnable_d
static void EEPROM_WriteEnable(void)
{
    SPIFI_SetCommand(SPIFI, &command[CMD_SPIFI_WRITE_ENABLE]);
}
#endif /* gEepromWriteEnable_d */

/*! *********************************************************************************
 * \brief   Prepare the memory for write operations (bits must be in erased state)
 *
 * \param[in] NoOfBytes Number of bytes to write
 * \param[in] Addr      Start memory address
 *
 * \return ee_err_t.
 *
 ********************************************************************************** */
#if (gFlashEraseDuringWrite == 1)
static ee_err_t EEPROM_PrepareForWrite(uint32_t NoOfBytes, uint32_t Addr)
{
    uint32_t i;
    ee_err_t ret = ee_ok;
    uint32_t startBlk, endBlk;

    /* Obtain the range of sectors */
    startBlk = Addr / EEPROM_SECTOR_SIZE;
    endBlk   = (Addr + NoOfBytes - 1) / EEPROM_SECTOR_SIZE;

    /* Check if each sector needs erase */
    for (i = startBlk; i <= endBlk; i++)
    {
        if ((mHandle.EraseBitmap[i / 8] & (1 << (i % 8))) == 0)
        {
            ret = EEPROM_EraseBlock(i * EEPROM_SECTOR_SIZE, EEPROM_SECTOR_SIZE);

            if (ret != ee_ok)
            {
                break;
            }
        }
    }

    return ret;
}
#endif /* (gFlashEraseDuringWrite == 1) */

#if gEepromWriteEnable_d

void EepromWritePage(uint32_t NoOfBytes, uint32_t Addr, uint8_t * Outbuf)
{
    EEPROM_DBG_LOG("Addr=%x", Addr);
#ifdef gFlashBlockBitmap_d
    int sector_index;
    uint32_t startBlk, endBlk;

    /* Obtain the range of sectors */
    startBlk = Addr / EEPROM_SECTOR_SIZE;
    endBlk   = (Addr + NoOfBytes - 1) / EEPROM_SECTOR_SIZE;

    /* Mark  each sector as written */
    for (sector_index = startBlk; sector_index <= endBlk; sector_index++)
    {
        mHandle.EraseBitmap[sector_index / 8] &= ~(1 << (sector_index % 8));
    }
#endif /* gFlashBlockBitmap_d */

    OSA_SemaphoreWait(mExtEepromSemaphoreId, osaWaitForever_c);

    EEPROM_WriteEnable();
    SPIFI_SetCommandAddress(SPIFI, Addr + FSL_FEATURE_SPIFI_START_ADDR);

    if (CHIP_USING_SPIFI_DUAL_MODE())
    {
        command[CMD_SPIFI_DPROGRAM_PAGE].dataLen = NoOfBytes;
        SPIFI_SetCommand(SPIFI, &command[CMD_SPIFI_DPROGRAM_PAGE]);
    }
    else
    {
        command[CMD_SPIFI_QPROGRAM_PAGE].dataLen = NoOfBytes;
        SPIFI_SetCommand(SPIFI, &command[CMD_SPIFI_QPROGRAM_PAGE]);
    }
    SPIFI_WriteBuffer(SPIFI, Outbuf, NoOfBytes);

    OSA_SemaphorePost(mExtEepromSemaphoreId);
}

/*! *********************************************************************************
 * \brief   Writes maximum 256 bytes into a memory page
 *
 * \param[in] NoOfBytes Number of bytes to write into thr page
 * \param[in] Addr      Start memory address
 * \param[in] Outbuf    Pointer to the data to be written
 *
 * \return ee_err_t.
 *
 ********************************************************************************** */
static ee_err_t EEPROM_WritePage(uint32_t NoOfBytes, uint32_t Addr, uint8_t * Outbuf)
{
    EEPROM_DBG_LOG("");

    if (NoOfBytes > 0)
    {
        OSA_SemaphoreWait(mExtEepromSemaphoreId, osaWaitForever_c);

        while (EEPROM_isBusy())
            ;

        OSA_SemaphorePost(mExtEepromSemaphoreId);

        EepromWritePage(NoOfBytes, Addr, Outbuf);
    }

    return ee_ok;
}

#endif /* gEepromWriteEnable_d */
#endif /* gEepromType_d == gEepromDevice_MX25R8035F_c */
