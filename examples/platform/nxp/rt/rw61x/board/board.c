/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_cache.h"
#include "fsl_clock.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_flexspi.h"
#include "fsl_io_mux.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_FLEXSPI_DLL_LOCK_RETRY (10)

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Initialize debug console. */
void BOARD_InitDebugConsole(void)
{
    uint32_t uartClkSrcFreq = 0;

    /* attach FRG0 clock to FLEXCOMM3 (debug console) */
    CLOCK_SetFRGClock(BOARD_DEBUG_UART_FRG_CLK);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ;
    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

static status_t flexspi_hyper_ram_read_id(FLEXSPI_Type * base, uint16_t * buffer)
{
    flexspi_transfer_t flashXfer;
    status_t status;

    /* Write data */
    flashXfer.deviceAddress = 0x00;
    flashXfer.port          = kFLEXSPI_PortB1;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = 15;
    flashXfer.data          = (uint32_t *) buffer;
    flashXfer.dataSize      = 2;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    return status;
}

static status_t flexspi_hyper_ram_read_register(FLEXSPI_Type * base, uint32_t address, uint16_t * buffer)
{
    flexspi_transfer_t flashXfer;
    status_t status;

    /* Write data */
    flashXfer.deviceAddress = address;
    flashXfer.port          = kFLEXSPI_PortB1;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = 13;
    flashXfer.data          = (uint32_t *) buffer;
    flashXfer.dataSize      = 2;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    return status;
}

static status_t flexspi_hyper_ram_write_register(FLEXSPI_Type * base, uint32_t address, uint16_t * buffer)
{
    flexspi_transfer_t flashXfer;
    status_t status;

    /* Write data */
    flashXfer.deviceAddress = address;
    flashXfer.port          = kFLEXSPI_PortB1;
    flashXfer.cmdType       = kFLEXSPI_Write;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = 14;
    flashXfer.data          = (uint32_t *) buffer;
    flashXfer.dataSize      = 2;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    return status;
}

/* Initialize psram. */
status_t BOARD_InitPsRam(void)
{
    flexspi_device_config_t deviceconfig = {
        .flexspiRootClk       = 320000000, /* 320MHZ SPI serial clock, DDR serial clock 160M */
        .isSck2Enabled        = false,
        .flashSize            = 0x1000, /* 32Mb/KByte */
        .addressShift         = true,
        .CSIntervalUnit       = kFLEXSPI_CsIntervalUnit1SckCycle,
        .CSInterval           = 5,
        .CSHoldTime           = 2,
        .CSSetupTime          = 3,
        .dataValidTime        = 1,
        .columnspace          = 9 + 5, /* CA:9 + CA_SHIFT:5 */
        .enableWordAddress    = false,
        .AWRSeqIndex          = 12,
        .AWRSeqNumber         = 1,
        .ARDSeqIndex          = 11,
        .ARDSeqNumber         = 1,
        .AHBWriteWaitUnit     = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
        .AHBWriteWaitInterval = 0,
        .enableWriteMask      = true,
    };

    uint32_t customLUT[20] = {
        /* Read Data */
        [0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DDR, kFLEXSPI_4PAD, 0xAA, kFLEXSPI_Command_DDR, kFLEXSPI_4PAD, 0x00),
        [1] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_4PAD, 16, kFLEXSPI_Command_CADDR_DDR, kFLEXSPI_4PAD, 16),
        [2] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DUMMY_DDR, kFLEXSPI_4PAD, 28, kFLEXSPI_Command_READ_DDR, kFLEXSPI_4PAD, 0x01),

        /* Write Data */
        [4] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DDR, kFLEXSPI_4PAD, 0x22, kFLEXSPI_Command_DDR, kFLEXSPI_4PAD, 0x00),
        [5] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_4PAD, 16, kFLEXSPI_Command_CADDR_DDR, kFLEXSPI_4PAD, 16),
        [6] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DUMMY_DDR, kFLEXSPI_4PAD, 28, kFLEXSPI_Command_WRITE_DDR, kFLEXSPI_4PAD, 0x01),

        /* Read Register */
        [8]  = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DDR, kFLEXSPI_4PAD, 0xCC, kFLEXSPI_Command_DDR, kFLEXSPI_4PAD, 0x00),
        [9]  = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_4PAD, 16, kFLEXSPI_Command_CADDR_DDR, kFLEXSPI_4PAD, 16),
        [10] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DUMMY_DDR, kFLEXSPI_4PAD, 12, kFLEXSPI_Command_READ_DDR, kFLEXSPI_4PAD, 0x01),

        /* Write Register */
        [12] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DDR, kFLEXSPI_4PAD, 0x66, kFLEXSPI_Command_DDR, kFLEXSPI_4PAD, 0x00),
        [13] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_4PAD, 16, kFLEXSPI_Command_CADDR_DDR, kFLEXSPI_4PAD, 16),
        [14] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_DDR, kFLEXSPI_4PAD, 0x01, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00),

        /* Read ID */
        [16] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, 0xE0, kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_4PAD, 16),
        [17] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_CADDR_DDR, kFLEXSPI_4PAD, 16, kFLEXSPI_Command_DUMMY_RWDS_DDR, kFLEXSPI_4PAD, 0x08),
        [18] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_DDR, kFLEXSPI_4PAD, 0x01, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00),
    };

    flexspi_config_t config;
#if BOARD_ENABLE_PSRAM_CACHE
    cache64_config_t cacheCfg;
#endif
    status_t status = kStatus_Success;

    if (!BOARD_IS_XIP()) /* FlexSPI not initialized */
    {
        CLOCK_EnableClock(kCLOCK_Flexspi);
        RESET_ClearPeripheralReset(kFLEXSPI_RST_SHIFT_RSTn);
        BOARD_SetFlexspiClock(FLEXSPI, 5U, 1U);

        /* Get FLEXSPI default settings and configure the flexspi. */
        FLEXSPI_GetDefaultConfig(&config);

        /* Init FLEXSPI. */
        config.rxSampleClock      = kFLEXSPI_ReadSampleClkExternalInputFromDqsPad;
        config.rxSampleClockPortB = kFLEXSPI_ReadSampleClkExternalInputFromDqsPad;
        config.rxSampleClockDiff  = true;
        /*Set AHB buffer size for reading data through AHB bus. */
        config.ahbConfig.enableAHBPrefetch    = true;
        config.ahbConfig.enableAHBBufferable  = true;
        config.ahbConfig.enableAHBCachable    = true;
        config.ahbConfig.enableReadAddressOpt = true;
        for (uint8_t i = 1; i < FSL_FEATURE_FLEXSPI_AHB_BUFFER_COUNT - 1; i++)
        {
            config.ahbConfig.buffer[i].bufferSize = 0;
        }
        /* FlexSPI has total 1KB RX buffer.
         * Set DMA0 master to use AHB Rx Buffer0.
         */
        config.ahbConfig.buffer[0].masterIndex    = 10;  /* GDMA */
        config.ahbConfig.buffer[0].bufferSize     = 512; /* Allocate 512B bytes for DMA0 */
        config.ahbConfig.buffer[0].enablePrefetch = true;
        config.ahbConfig.buffer[0].priority       = 0;
        /* All other masters use last buffer with 512B bytes. */
        config.ahbConfig.buffer[FSL_FEATURE_FLEXSPI_AHB_BUFFER_COUNT - 1].bufferSize = 512;
#if !(defined(FSL_FEATURE_FLEXSPI_HAS_NO_MCR0_COMBINATIONEN) && FSL_FEATURE_FLEXSPI_HAS_NO_MCR0_COMBINATIONEN)
        config.enableCombination = false;
#endif
        FLEXSPI_Init(BOARD_FLEXSPI_PSRAM, &config);
    }

    /* Configure flash settings according to serial flash feature. */
    FLEXSPI_SetFlashConfig(BOARD_FLEXSPI_PSRAM, &deviceconfig, kFLEXSPI_PortB1);

    /* Update bottom LUT table (44-63). */
    FLEXSPI_UpdateLUT(BOARD_FLEXSPI_PSRAM, 44U, customLUT, ARRAY_SIZE(customLUT));

    /* Do software reset. */
    FLEXSPI_SoftwareReset(BOARD_FLEXSPI_PSRAM);

    /* Read identification: the Manufacturer ID of ISSI's PSRAM(IS66/67WVQ8M4DALL) is 0x03U  */
    uint16_t identification = 0x00U;
    uint16_t registerVal    = 0x00U;
    status                  = flexspi_hyper_ram_read_id(BOARD_FLEXSPI_PSRAM, &identification);
    if ((status != kStatus_Success) || (identification & 0x03U) != 0x03U)
    {
        status = kStatus_Fail;
    }

    /* Read configuration register: the default setting is 0xF052(see table 6.1 Configuration Register in
       PSRAM's(IS66/67WVQ8M4DALL) datasheet), which Latency code(CR[7:4]) is 0101b, which supported max frequency is
       200MHz.*/
    status = flexspi_hyper_ram_read_register(BOARD_FLEXSPI_PSRAM, 0x04U << 9, &registerVal);
    if ((status != kStatus_Success) || registerVal != 0xF052)
    {
        status = kStatus_Fail;
    }

    /* Initial access latency configuration, which is located in bit3 of CR. */
    registerVal |= 0x01UL << 3;

    /* Write configuration register: */
    status = flexspi_hyper_ram_write_register(BOARD_FLEXSPI_PSRAM, 0x04U << 9, &registerVal);
    if ((status != kStatus_Success) || registerVal != 0xF05A)
    {
        status = kStatus_Fail;
    }

    /* Reset */
    registerVal = 0x00U;

    /* Read configuration register: changes default Variable Latency into Fixed Latency: 0xF05A.
       Note: FlexSPI only supports fixed latency mode for ISSI's psram. */
    status = flexspi_hyper_ram_read_register(BOARD_FLEXSPI_PSRAM, 0x04U << 9, &registerVal);
    if ((status != kStatus_Success) || registerVal != 0xF05A)
    {
        status = kStatus_Fail;
    }

#if BOARD_ENABLE_PSRAM_CACHE
    CACHE64_GetDefaultConfig(&cacheCfg);
    /* Suppose:
       Flash on PC bus starting from 0x08000000, controlled by cache 0.
       PSRAM on PS bus starting from 0x28000000, controlled by cache 1.
     */
    CACHE64_Init(CACHE64_POLSEL1, &cacheCfg);
    CACHE64_EnableWriteBuffer(CACHE64_CTRL1, true);
    CACHE64_EnableCache(CACHE64_CTRL1);
#endif

    return status;
}

void BOARD_InitSleepPinConfig(void)
{
    int32_t i;

    /* Set all non-AON pins output low level in sleep mode. */
    for (i = 0; i < 22; i++)
    {
        IO_MUX_SetPinOutLevelInSleep(i, IO_MUX_SleepPinLevelLow);
    }
    for (i = 28; i < 64; i++)
    {
        IO_MUX_SetPinOutLevelInSleep(i, IO_MUX_SleepPinLevelLow);
    }

    /* Set RF_CNTL 0-3 output low level in sleep mode. */
    for (i = 0; i < 4; i++)
    {
        IO_MUX_SetRfPinOutLevelInSleep(i, IO_MUX_SleepPinLevelLow);
    }
}

void BOARD_DeinitFlash(FLEXSPI_Type * base)
{
    /* Enable FLEXSPI clock again */
    CLKCTL0->PSCCTL0_SET = CLKCTL0_PSCCTL0_SET_FLEXSPI0_MASK;

    /* Enable FLEXSPI module */
    base->MCR0 &= ~FLEXSPI_MCR0_MDIS_MASK;

    /* Wait until FLEXSPI is not busy */
    while (!((base->STS0 & FLEXSPI_STS0_ARBIDLE_MASK) && (base->STS0 & FLEXSPI_STS0_SEQIDLE_MASK)))
    {
    }
    /* Disable module during the reset procedure */
    base->MCR0 |= FLEXSPI_MCR0_MDIS_MASK;
}

void BOARD_InitFlash(FLEXSPI_Type * base)
{
    uint32_t status;
    uint32_t lastStatus;
    uint32_t retry;

    /* Loopback from DQS pad can maximize RD board flash speed. */
    if ((base->MCR0 & FLEXSPI_MCR0_RXCLKSRC_MASK) != FLEXSPI_MCR0_RXCLKSRC(1))
    {
        base->MCR0 = (base->MCR0 & ~FLEXSPI_MCR0_RXCLKSRC_MASK) | FLEXSPI_MCR0_RXCLKSRC(1);
    }
    /* If serial root clock is >= 100 MHz, DLLEN set to 1, OVRDEN set to 0, then SLVDLYTARGET setting of 0x0 is
     * recommended. */
    base->DLLCR[0] = 0x1U;

    /* Enable FLEXSPI module */
    base->MCR0 &= ~FLEXSPI_MCR0_MDIS_MASK;

    base->MCR0 |= FLEXSPI_MCR0_SWRESET_MASK;
    while (base->MCR0 & FLEXSPI_MCR0_SWRESET_MASK)
    {
    }

    /* Need to wait DLL locked if DLL enabled */
    if (0U != (base->DLLCR[0] & FLEXSPI_DLLCR_DLLEN_MASK))
    {
        lastStatus = base->STS2;
        retry      = BOARD_FLEXSPI_DLL_LOCK_RETRY;
        /* Wait slave delay line locked and slave reference delay line locked. */
        do
        {
            status = base->STS2;
            if ((status & (FLEXSPI_STS2_AREFLOCK_MASK | FLEXSPI_STS2_ASLVLOCK_MASK)) ==
                (FLEXSPI_STS2_AREFLOCK_MASK | FLEXSPI_STS2_ASLVLOCK_MASK))
            {
                /* Locked */
                retry = 100;
                break;
            }
            else if (status == lastStatus)
            {
                /* Same delay cell number in calibration */
                retry--;
            }
            else
            {
                retry      = BOARD_FLEXSPI_DLL_LOCK_RETRY;
                lastStatus = status;
            }
        } while (retry > 0);
        /* According to ERR011377, need to delay at least 100 NOPs to ensure the DLL is locked. */
        for (; retry > 0U; retry--)
        {
            __NOP();
        }
    }
}

/* BOARD_SetFlexspiClock run in RAM used to configure FlexSPI clock source and divider when XIP. */
void BOARD_SetFlexspiClock(FLEXSPI_Type * base, uint32_t src, uint32_t divider)
{
    if ((CLKCTL0->FLEXSPIFCLKSEL != CLKCTL0_FLEXSPIFCLKSEL_SEL(src)) ||
        ((CLKCTL0->FLEXSPIFCLKDIV & CLKCTL0_FLEXSPIFCLKDIV_DIV_MASK) != (divider - 1)))
    {
        /* Always deinit FLEXSPI and init FLEXSPI for the flash to make sure the flash works correctly after the
         FLEXSPI root clock changed as the default FLEXSPI configuration may does not work for the new root clock
         frequency. */
        BOARD_DeinitFlash(base);

        /* Disable clock before changing clock source */
        CLKCTL0->PSCCTL0_CLR = CLKCTL0_PSCCTL0_CLR_FLEXSPI0_MASK;
        /* Update flexspi clock. */
        CLKCTL0->FLEXSPIFCLKSEL = CLKCTL0_FLEXSPIFCLKSEL_SEL(src);
        CLKCTL0->FLEXSPIFCLKDIV |= CLKCTL0_FLEXSPIFCLKDIV_RESET_MASK; /* Reset the divider counter */
        CLKCTL0->FLEXSPIFCLKDIV = CLKCTL0_FLEXSPIFCLKDIV_DIV(divider - 1);
        while ((CLKCTL0->FLEXSPIFCLKDIV) & CLKCTL0_FLEXSPIFCLKDIV_REQFLAG_MASK)
        {
        }
        /* Enable FLEXSPI clock again */
        CLKCTL0->PSCCTL0_SET = CLKCTL0_PSCCTL0_SET_FLEXSPI0_MASK;

        BOARD_InitFlash(base);
    }
}

/* This function is used to change FlexSPI clock to a stable source before clock sources(Such as PLL and Main clock)
 * updating in case XIP(execute code on FLEXSPI memory.) */
void BOARD_FlexspiClockSafeConfig(void)
{
    /* Move FLEXSPI clock source to T3 256m / 2 to avoid instruction/data fetch issue in XIP when
     * updating PLL and main clock.
     */
    BOARD_SetFlexspiClock(FLEXSPI, 6U, 2U);
}

void BOARD_CLIAttachClk(void)
{
    /* attach FRG3 clock to FLEXCOMM3 (debug console) */
    CLOCK_SetFRGClock(BOARD_CLI_FRG_CLK);
    CLOCK_AttachClk(BOARD_CLI_CLK_ATTACH);
}
