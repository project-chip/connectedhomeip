/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * Includes
 */
#include "rsi_board.h"
#include "rsi_pll.h"
#include "rsi_rom_clks.h"
#include "silabs_utils.h"
#include "sli_siwx917_soc.h"

#define SOC_PLL_REF_FREQUENCY 32000000 /* PLL input REFERENCE clock 32MHZ */

// Note: Change this macro to required PLL frequency in hertz
#define PS4_SOC_FREQ 180000000 /* PLL out clock 180MHz */
#define SWITCH_QSPI_TO_SOC_PLL
#define ICACHE_DISABLE
#define DEBUG_DISABLE

/* QSPI clock config params */
#define INTF_PLL_500_CTRL_VALUE 0xD900
#define INTF_PLL_CLK 80000000 /* PLL out clock 80 MHz */

#define PMU_GOOD_TIME 31  /*Duration in us*/
#define XTAL_GOOD_TIME 31 /*Duration in us*/

/*Pre-fetch and regestring */
#define ICACHE2_ADDR_TRANSLATE_1_REG *(volatile uint32_t *) (0x20280000 + 0x24)
#define MISC_CFG_SRAM_REDUNDANCY_CTRL *(volatile uint32_t *) (0x46008000 + 0x18)
#define MISC_CONFIG_MISC_CTRL1 *(volatile uint32_t *) (0x46008000 + 0x44)
#define MISC_QUASI_SYNC_MODE *(volatile uint32_t *) (0x46008000 + 0x84)

/**
 * @fn           void soc_pll_config()
 * @brief        This function to configure clock for SiWx917 SoC (80MHz)
 *               Configure the PLL frequency and Switch M4 clock to PLL clock for speed operations
 *
 * @param[in]    none
 * @param[out]   none
 * @return       int
 * @section description
 * configure clock for SiWx917 SoC
 *
 */
int soc_pll_config(void)
{
    int32_t status = RSI_OK;

    RSI_CLK_SocPllLockConfig(1, 1, 7);

    RSI_CLK_SocPllRefClkConfig(2);

    RSI_CLK_M4SocClkConfig(M4CLK, M4_ULPREFCLK, 0);

    /*Enable fre-fetch and register if SOC-PLL frequency is more than or equal to 120M*/
#if (PS4_SOC_FREQ >= 120000000)
    ICACHE2_ADDR_TRANSLATE_1_REG  = BIT(21);
    MISC_CFG_SRAM_REDUNDANCY_CTRL = BIT(4);
    MISC_CONFIG_MISC_CTRL1 |= BIT(4);
#if !(defined WISE_AOC_4)
    MISC_QUASI_SYNC_MODE |= BIT(6);
    MISC_QUASI_SYNC_MODE |= (BIT(6) | BIT(7));
#endif /* !WISE_AOC_4 */
#endif /* (PS4_SOC_FREQ > 120000000) */

    RSI_CLK_SetSocPllFreq(M4CLK, PS4_SOC_FREQ, SOC_PLL_REF_FREQUENCY);

    RSI_CLK_M4SocClkConfig(M4CLK, M4_SOCPLLCLK, 0);

#ifdef SWITCH_QSPI_TO_SOC_PLL
    /* program intf pll to 160Mhz */
    SPI_MEM_MAP_PLL(INTF_PLL_500_CTRL_REG9) = INTF_PLL_500_CTRL_VALUE;
    status                                  = RSI_CLK_SetIntfPllFreq(M4CLK, INTF_PLL_CLK, SOC_PLL_REF_FREQUENCY);
    if (status != RSI_OK)
    {
        SILABS_LOG("Failed to Config Interface PLL Clock, status:%d", status);
    }
    else
    {
        SILABS_LOG("Configured Interface PLL Clock to %d", INTF_PLL_CLK);
    }

    RSI_CLK_QspiClkConfig(M4CLK, QSPI_INTFPLLCLK, 0, 0, 1);
#endif /* SWITCH_QSPI_TO_SOC_PLL */

    return 0;
}

/*==============================================*/
/**
 * @fn           void RSI_Wakeupsw_config()
 * @brief        This function Initializes the platform
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @section description
 * This function initializes the platform
 *
 */
void RSI_Wakeupsw_config(void)
{
    /*Enable the REN*/
    RSI_NPSSGPIO_InputBufferEn(NPSS_GPIO_2, 1);

    /*Configure the NPSS GPIO mode to wake up  */
    RSI_NPSSGPIO_SetPinMux(NPSS_GPIO_2, NPSSGPIO_PIN_MUX_MODE2);

    /*Configure the NPSS GPIO direction to input */
    RSI_NPSSGPIO_SetDir(NPSS_GPIO_2, NPSS_GPIO_DIR_OUTPUT);

    /* Enables fall edge interrupt detection for UULP_VBAT_GPIO_0 */
    RSI_NPSSGPIO_SetIntFallEdgeEnable(NPSS_GPIO_2_INTR);

    /* Un mask the NPSS GPIO interrupt*/
    RSI_NPSSGPIO_IntrUnMask(NPSS_GPIO_2_INTR);

    /*Select wake up sources */
    RSI_PS_SetWkpSources(GPIO_BASED_WAKEUP);

    /* clear NPSS GPIO interrupt*/
    RSI_NPSSGPIO_ClrIntr(NPSS_GPIO_2_INTR);

    /*Enable the NPSS GPIO interrupt slot*/
    NVIC_EnableIRQ(NPSS_TO_MCU_GPIO_INTR_IRQn);

    NVIC_SetPriority(NPSS_TO_MCU_GPIO_INTR_IRQn, 7);
}

void RSI_Wakeupsw_config_gpio0(void)
{
    /*Configure the NPSS GPIO mode to wake up  */
    RSI_NPSSGPIO_SetPinMux(NPSS_GPIO_0, NPSSGPIO_PIN_MUX_MODE2);

    /*Configure the NPSS GPIO direction to input */
    RSI_NPSSGPIO_SetDir(NPSS_GPIO_0, NPSS_GPIO_DIR_INPUT);

    /*Configure the NPSS GPIO interrupt polarity */
    RSI_NPSSGPIO_SetPolarity(NPSS_GPIO_0, NPSS_GPIO_INTR_HIGH);

    /*Enable the REN*/
    RSI_NPSSGPIO_InputBufferEn(NPSS_GPIO_0, 1);

    /* Set the GPIO to wake from deep sleep */
    RSI_NPSSGPIO_SetWkpGpio(NPSS_GPIO_0_INTR);

    /* Enables fall edge interrupt detection for UULP_VBAT_GPIO_0 */
    RSI_NPSSGPIO_SetIntFallEdgeEnable(NPSS_GPIO_0_INTR);

    /* Un mask the NPSS GPIO interrupt*/
    RSI_NPSSGPIO_IntrUnMask(NPSS_GPIO_0_INTR);

    /*Select wake up sources */
    RSI_PS_SetWkpSources(GPIO_BASED_WAKEUP);

    /* clear NPSS GPIO interrupt*/
    RSI_NPSSGPIO_ClrIntr(NPSS_GPIO_0_INTR);

    // 21 being the NPSS_TO_MCU_GPIO_INTR_IRQn
    NVIC_EnableIRQ(NPSS_TO_MCU_GPIO_INTR_IRQn);
    NVIC_SetPriority(NPSS_TO_MCU_GPIO_INTR_IRQn, 7);
}

/*==============================================*/
/**
 * @fn           void rsi_hal_board_init()
 * @brief        This function Initializes the platform
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @section description
 * This function initializes the platform
 *
 */
void rsi_hal_board_init(void)
{
    SystemCoreClockUpdate();

    // initialize the LED pins
    RSI_Board_Init();

    /* configure clock for SiWx917 SoC */
    soc_pll_config();
    // SILABS_LOG("%s, soc_pll_config, SystemCoreClock=%d\n", __func__, SystemCoreClock);

#ifdef COMMON_FLASH_EN
    /* Before TA going to power save mode ,set m4ss_ref_clk_mux_ctrl ,tass_ref_clk_mux_ctrl,
    AON domain power supply controls form TA to M4 */
    RSI_Set_Cntrls_To_M4();
#endif
#ifdef DEBUG_UART
    DEBUGINIT();
#endif
}