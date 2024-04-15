/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sdmmc_config.h"
#include "fsl_iomuxc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_SDCardPowerControl(bool enable);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*!brief sdmmc dma buffer */
AT_NONCACHEABLE_SECTION_ALIGN(static uint32_t s_sdmmcHostDmaBuffer[BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE],
                              SDMMCHOST_DMA_DESCRIPTOR_BUFFER_ALIGN_SIZE);
#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
/* two cache line length for sdmmc host driver maintain unalign transfer */
SDK_ALIGN(static uint8_t s_sdmmcCacheLineAlignBuffer[BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE * 2U], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);
#endif
#if defined(SDIO_ENABLED) || defined(SD_ENABLED)
static sd_detect_card_t s_cd;
static sd_io_voltage_t s_ioVoltage = {
    .type = BOARD_SDMMC_SD_IO_VOLTAGE_CONTROL_TYPE,
    .func = NULL,
};
#endif
static sdmmchost_t s_host;

#ifdef SDIO_ENABLED
static sdio_card_int_t s_sdioInt;
#endif

GPIO_HANDLE_DEFINE(s_CardDetectGpioHandle);
GPIO_HANDLE_DEFINE(s_PowerResetGpioHandle);

/*******************************************************************************
 * Code
 ******************************************************************************/
uint32_t BOARD_USDHC1ClockConfiguration(void)
{
    clock_root_config_t rootCfg = { 0 };
    /* SYS PLL2 528MHz. */
    const clock_sys_pll2_config_t sysPll2Config = {
        .ssEnable = false,
    };

    CLOCK_InitSysPll2(&sysPll2Config);
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd2, 24);

    rootCfg.mux = 4;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Usdhc1, &rootCfg);

    return CLOCK_GetRootClockFreq(kCLOCK_Root_Usdhc1);
}

#if __CORTEX_M == 7
void BOARD_USDHC_Errata(void)
{
    /* ERR050396
     * Errata description:
     * AXI to AHB conversion for CM7 AHBS port (port to access CM7 to TCM) is by a NIC301 block, instead of XHB400
     * block. NIC301 doesn’t support sparse write conversion. Any AXI to AHB conversion need XHB400, not by NIC. This
     * will result in data corruption in case of AXI sparse write reaches the NIC301 ahead of AHBS. Errata workaround:
     * For uSDHC, don’t set the bit#1 of IOMUXC_GPR28 (AXI transaction is cacheable), if write data to TCM aligned in 4
     * bytes; No such write access limitation for OCRAM or external RAM
     */
    IOMUXC_GPR->GPR28 &= (~IOMUXC_GPR_GPR28_AWCACHE_USDHC_MASK);
}
#endif

#if defined(SDIO_ENABLED) || defined(SD_ENABLED)
bool BOARD_SDCardGetDetectStatus(void)
{
    uint8_t pinState;

    if (HAL_GpioGetInput(s_CardDetectGpioHandle, &pinState) == kStatus_HAL_GpioSuccess)
    {
        if (pinState == BOARD_SDMMC_SD_CD_INSERT_LEVEL)
        {
            return true;
        }
    }

    return false;
}

void SDMMC_SD_CD_Callback(void * param)
{
    if (s_cd.callback != NULL)
    {
        s_cd.callback(BOARD_SDCardGetDetectStatus(), s_cd.userData);
    }
}

void BOARD_SDCardDAT3PullFunction(uint32_t status)
{
    if (status == kSD_DAT3PullDown)
    {
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_05_USDHC1_DATA3,
                            0xCU); /* no pull in IOMUX configuration, 100K pull down externally is required. */
        /* power reset the card to clear DAT3 legacy status */
        BOARD_SDCardPowerControl(false);
        SDK_DelayAtLeastUs(1000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
        /* make sure the card is power on for DAT3 pull up */
        BOARD_SDCardPowerControl(true);
        /* power on delay */
        SDK_DelayAtLeastUs(1000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    }
    else
    {
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_05_USDHC1_DATA3, 0x4U);
    }
}

void BOARD_SDCardDetectInit(sd_cd_t cd, void * userData)
{
    uint8_t pinState;

    /* install card detect callback */
    s_cd.cdDebounce_ms = BOARD_SDMMC_SD_CARD_DETECT_DEBOUNCE_DELAY_MS;
    s_cd.type          = BOARD_SDMMC_SD_CD_TYPE;
    s_cd.cardDetected  = BOARD_SDCardGetDetectStatus;
    s_cd.callback      = cd;
    s_cd.userData      = userData;

    if (BOARD_SDMMC_SD_CD_TYPE == kSD_DetectCardByGpioCD)
    {
        hal_gpio_pin_config_t sw_config = {
            kHAL_GpioDirectionIn,
            0,
            BOARD_SDMMC_SD_CD_GPIO_PORT,
            BOARD_SDMMC_SD_CD_GPIO_PIN,
        };
        HAL_GpioInit(s_CardDetectGpioHandle, &sw_config);
        HAL_GpioSetTriggerMode(s_CardDetectGpioHandle, BOARD_SDMMC_SD_CD_INTTERUPT_TYPE);
        HAL_GpioInstallCallback(s_CardDetectGpioHandle, SDMMC_SD_CD_Callback, NULL);

        if (HAL_GpioGetInput(s_CardDetectGpioHandle, &pinState) == kStatus_HAL_GpioSuccess)
        {
            if (pinState == BOARD_SDMMC_SD_CD_INSERT_LEVEL)
            {
                if (cd != NULL)
                {
                    cd(true, userData);
                }
            }
        }
    }

    /* register DAT3 pull function switch function pointer */
    if (BOARD_SDMMC_SD_CD_TYPE == kSD_DetectCardByHostDATA3)
    {
        s_cd.dat3PullFunc = BOARD_SDCardDAT3PullFunction;
        BOARD_SDCardPowerControl(true);
    }
}

void BOARD_SDCardPowerResetInit(void)
{
    hal_gpio_pin_config_t sw_config = {
        kHAL_GpioDirectionOut,
        1,
        BOARD_SDMMC_SD_POWER_RESET_GPIO_PORT,
        BOARD_SDMMC_SD_POWER_RESET_GPIO_PIN,
    };
    HAL_GpioInit(s_PowerResetGpioHandle, &sw_config);
}

void BOARD_SDCardPowerControl(bool enable)
{
    if (enable)
    {
        HAL_GpioSetOutput(s_PowerResetGpioHandle, 0);
    }
    else
    {
        HAL_GpioSetOutput(s_PowerResetGpioHandle, 1);
    }
}
#endif

#ifdef SD_ENABLED
void BOARD_SD_Config(void * card, sd_cd_t cd, uint32_t hostIRQPriority, void * userData)
{
    assert(card);

    s_host.dmaDesBuffer         = s_sdmmcHostDmaBuffer;
    s_host.dmaDesBufferWordsNum = BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;
#if ((defined __DCACHE_PRESENT) && __DCACHE_PRESENT) || (defined FSL_FEATURE_HAS_L1CACHE && FSL_FEATURE_HAS_L1CACHE)
    s_host.enableCacheControl = BOARD_SDMMC_HOST_CACHE_CONTROL;
#endif
#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
    s_host.cacheAlignBuffer     = s_sdmmcCacheLineAlignBuffer;
    s_host.cacheAlignBufferSize = BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE * 2U;
#endif

    ((sd_card_t *) card)->host                                = &s_host;
    ((sd_card_t *) card)->host->hostController.base           = BOARD_SDMMC_SD_HOST_BASEADDR;
    ((sd_card_t *) card)->host->hostController.sourceClock_Hz = BOARD_USDHC1ClockConfiguration();

    ((sd_card_t *) card)->usrParam.cd         = &s_cd;
    ((sd_card_t *) card)->usrParam.pwr        = BOARD_SDCardPowerControl;
    ((sd_card_t *) card)->usrParam.ioStrength = NULL;
    ((sd_card_t *) card)->usrParam.ioVoltage  = &s_ioVoltage;
    ((sd_card_t *) card)->usrParam.maxFreq    = BOARD_SDMMC_SD_HOST_SUPPORT_SDR104_FREQ;

    BOARD_SDCardPowerResetInit();

    BOARD_SDCardDetectInit(cd, userData);

    NVIC_SetPriority(BOARD_SDMMC_SD_HOST_IRQ, hostIRQPriority);

#if __CORTEX_M == 7
    BOARD_USDHC_Errata();
#endif
}
#endif

#ifdef SDIO_ENABLED
void BOARD_SDIO_Config(void * card, sd_cd_t cd, uint32_t hostIRQPriority, sdio_int_t cardInt)
{
    assert(card);

    s_host.dmaDesBuffer         = s_sdmmcHostDmaBuffer;
    s_host.dmaDesBufferWordsNum = BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;
#if ((defined __DCACHE_PRESENT) && __DCACHE_PRESENT) || (defined FSL_FEATURE_HAS_L1CACHE && FSL_FEATURE_HAS_L1CACHE)
    s_host.enableCacheControl = BOARD_SDMMC_HOST_CACHE_CONTROL;
#endif
#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
    s_host.cacheAlignBuffer     = s_sdmmcCacheLineAlignBuffer;
    s_host.cacheAlignBufferSize = BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE * 2U;
#endif

    ((sdio_card_t *) card)->host                                = &s_host;
    ((sdio_card_t *) card)->host->hostController.base           = BOARD_SDMMC_SDIO_HOST_BASEADDR;
    ((sdio_card_t *) card)->host->hostController.sourceClock_Hz = BOARD_USDHC1ClockConfiguration();

    ((sdio_card_t *) card)->usrParam.cd         = &s_cd;
    ((sdio_card_t *) card)->usrParam.pwr        = BOARD_SDCardPowerControl;
    ((sdio_card_t *) card)->usrParam.ioStrength = NULL;
    ((sdio_card_t *) card)->usrParam.ioVoltage  = &s_ioVoltage;
    ((sdio_card_t *) card)->usrParam.maxFreq    = BOARD_SDMMC_SD_HOST_SUPPORT_SDR104_FREQ;
    if (cardInt != NULL)
    {
        s_sdioInt.cardInterrupt                  = cardInt;
        ((sdio_card_t *) card)->usrParam.sdioInt = &s_sdioInt;
    }

    BOARD_SDCardPowerResetInit();
    BOARD_SDCardDetectInit(cd, NULL);

    NVIC_SetPriority(BOARD_SDMMC_SDIO_HOST_IRQ, hostIRQPriority);

#if __CORTEX_M == 7
    BOARD_USDHC_Errata();
#endif
}
#endif

#ifdef MMC_ENABLED
void BOARD_MMC_Config(void * card, uint32_t hostIRQPriority)
{
    assert(card);

    s_host.dmaDesBuffer         = s_sdmmcHostDmaBuffer;
    s_host.dmaDesBufferWordsNum = BOARD_SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;
#if ((defined __DCACHE_PRESENT) && __DCACHE_PRESENT) || (defined FSL_FEATURE_HAS_L1CACHE && FSL_FEATURE_HAS_L1CACHE)
    s_host.enableCacheControl = BOARD_SDMMC_HOST_CACHE_CONTROL;
#endif
#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
    s_host.cacheAlignBuffer     = s_sdmmcCacheLineAlignBuffer;
    s_host.cacheAlignBufferSize = BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE * 2U;
#endif

    ((mmc_card_t *) card)->host                                = &s_host;
    ((mmc_card_t *) card)->host->hostController.base           = BOARD_SDMMC_MMC_HOST_BASEADDR;
    ((mmc_card_t *) card)->host->hostController.sourceClock_Hz = BOARD_USDHC1ClockConfiguration();
    ((mmc_card_t *) card)->host->tuningType                    = BOARD_SDMMC_MMC_TUNING_TYPE;
    ((mmc_card_t *) card)->usrParam.ioStrength                 = NULL;
    ((mmc_card_t *) card)->usrParam.maxFreq                    = BOARD_SDMMC_MMC_HOST_SUPPORT_HS200_FREQ;

    ((mmc_card_t *) card)->usrParam.capability |= BOARD_SDMMC_MMC_SUPPORT_8_BIT_DATA_WIDTH;

    ((mmc_card_t *) card)->hostVoltageWindowVCC  = BOARD_SDMMC_MMC_VCC_SUPPLY;
    ((mmc_card_t *) card)->hostVoltageWindowVCCQ = BOARD_SDMMC_MMC_VCCQ_SUPPLY;

    NVIC_SetPriority(BOARD_SDMMC_MMC_HOST_IRQ, hostIRQPriority);

#if __CORTEX_M == 7
    BOARD_USDHC_Errata();
#endif
}
#endif
