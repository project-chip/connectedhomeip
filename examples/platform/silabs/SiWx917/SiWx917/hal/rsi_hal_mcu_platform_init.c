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
#include "siwx917_utils.h"
#include "system_RS1xxxx.h"

#define SOC_PLL_REF_FREQUENCY 32000000 /* PLL input REFERENCE clock 32MHZ */

// Note: Change this macro to required PLL frequency in hertz
#define PS4_SOC_FREQ 80000000 /* PLL out clock 80MHz */
#define SWITCH_QSPI_TO_SOC_PLL
#define ICACHE_DISABLE
#define DEBUG_DISABLE

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
    RSI_CLK_QspiClkConfig(M4CLK, QSPI_INTFPLLCLK, 0, 0, 0);
#endif /* SWITCH_QSPI_TO_SOC_PLL */

    return 0;
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

    /* configure clock for SiWx917 SoC */
    soc_pll_config();
    SILABS_LOG("%s, soc_pll_config, SystemCoreClock=%d\n", __func__, SystemCoreClock);

#ifdef COMMON_FLASH_EN
    /* Before TA going to power save mode ,set m4ss_ref_clk_mux_ctrl ,tass_ref_clk_mux_ctrl,
    AON domain power supply controls form TA to M4 */
    RSI_Set_Cntrls_To_M4();
#endif
#ifdef DEBUG_UART
    DEBUGINIT();
#endif
}
