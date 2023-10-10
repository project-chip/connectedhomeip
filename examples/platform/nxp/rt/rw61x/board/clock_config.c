/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_config.h"
#include "board.h"
#include "fsl_clock.h"
#include "fsl_power.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
const clock_avpll_config_t g_avpllConfig_BOARD_BootClockRUN = { .ch1Freq    = kCLOCK_AvPllChFreq12p288m,
                                                                .ch2Freq    = kCLOCK_AvPllChFreq64m,
                                                                .enableCali = true };

/*FUNCTION**********************************************************************
 *
 * Function Name : BOARD_FlexspiClockSafeConfig
 * Description   : FLEXSPI clock source safe configuration weak function.
 *                 Called before clock source(Such as PLL, Main clock) configuration.
 * Note          : Users need override this function to change FLEXSPI clock source to stable source when executing
 *                 code on FLEXSPI memory(XIP). If XIP, the function should runs in RAM and move the FLEXSPI clock
 *source to an stable clock to avoid instruction/data fetch issue during clock updating.
 *END**************************************************************************/
__attribute__((weak)) void BOARD_FlexspiClockSafeConfig(void) {}

/*FUNCTION**********************************************************************
 *
 * Function Name : BOARD_SetFlexspiClock
 * Description   : This function should be overridden if executing code on FLEXSPI memory(XIP).
 *                 To Change FLEXSPI clock, should move to run from RAM and then configure FLEXSPI clock source.
 *                 After the clock is changed and stable,  move back to run on FLEXSPI.
 * Param base    : FLEXSPI peripheral base address.
 * Param src     : FLEXSPI clock source.
 * Param divider : FLEXSPI clock divider.
 *END**************************************************************************/
__attribute__((weak)) void BOARD_SetFlexspiClock(FLEXSPI_Type * base, uint32_t src, uint32_t divider)
{
    CLKCTL0->FLEXSPIFCLKSEL = CLKCTL0_FLEXSPIFCLKSEL_SEL(src);
    CLKCTL0->FLEXSPIFCLKDIV |= CLKCTL0_FLEXSPIFCLKDIV_RESET_MASK; /* Reset the divider counter */
    CLKCTL0->FLEXSPIFCLKDIV = CLKCTL0_FLEXSPIFCLKDIV_DIV(divider - 1);
    while ((CLKCTL0->FLEXSPIFCLKDIV) & CLKCTL0_FLEXSPIFCLKDIV_REQFLAG_MASK)
    {
    }
}

/*******************************************************************************
 ************************ BOARD_InitBootClocks function ************************
 ******************************************************************************/
void BOARD_InitBootClocks(void)
{
    BOARD_BootClockRUN();
}

/*******************************************************************************
 ********************** Configuration BOARD_BootClockRUN ***********************
 ******************************************************************************/
/*******************************************************************************
 * Variables for BOARD_BootClockRUN configuration
 ******************************************************************************/

/*******************************************************************************
 * Code for BOARD_BootClockRUN configuration
 ******************************************************************************/
void BOARD_BootClockRUN(void)
{
    if ((PMU->CAU_SLP_CTRL & PMU_CAU_SLP_CTRL_SOC_SLP_RDY_MASK) == 0U)
    {
        /* LPOSC not enabled, enable it */
        CLOCK_EnableClock(kCLOCK_RefClkCauSlp);
    }
    if ((SYSCTL2->SOURCE_CLK_GATE & SYSCTL2_SOURCE_CLK_GATE_REFCLK_SYS_CG_MASK) != 0U)
    {
        /* REFCLK_SYS not enabled, enable it */
        CLOCK_EnableClock(kCLOCK_RefClkSys);
    }

    /* Initialize T3 clocks and t3pll_mci_48_60m_irc configured to 48.3MHz */
    CLOCK_InitT3RefClk(kCLOCK_T3MciIrc48m);
    /* Enable FFRO */
    CLOCK_EnableClock(kCLOCK_T3PllMciIrcClk);
    /* Enable T3 256M clock and SFRO */
    CLOCK_EnableClock(kCLOCK_T3PllMci256mClk);

    if (BOARD_IS_XIP())
    {
        /* Call function BOARD_FlexspiClockSafeConfig() to move FlexSPI clock to a stable clock source to avoid
           instruction/data fetch issue when updating PLL and Main clock if XIP(execute code on FLEXSPI memory). */
        BOARD_FlexspiClockSafeConfig();
    }

    /* First let M33 run on SOSC */
    CLOCK_AttachClk(kSYSOSC_to_MAIN_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivSysCpuAhbClk, 1);

    /* tcpu_mci_clk configured to 260MHz, tcpu_mci_flexspi_clk 312MHz. */
    CLOCK_InitTcpuRefClk(3120000000UL, kCLOCK_TcpuFlexspiDiv10);
    /* Enable tcpu_mci_clk 260MHz. Keep tcpu_mci_flexspi_clk gated. */
    CLOCK_EnableClock(kCLOCK_TcpuMciClk);

    /* tddr_mci_flexspi_clk 320MHz */
    CLOCK_InitTddrRefClk(kCLOCK_TddrFlexspiDiv10);
    CLOCK_EnableClock(kCLOCK_TddrMciFlexspiClk); /* 320MHz */

    /* Enable AUX0 PLL to 260MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivAux0PllClk, 1U);

    /* Init AVPLL and enable both channels. */
    CLOCK_InitAvPll(&g_avpllConfig_BOARD_BootClockRUN);
    CLOCK_SetClkDiv(kCLOCK_DivAudioPllClk, 1U);

    /* Configure MainPll to 260MHz, then let CM33 run on Main PLL. */
    CLOCK_SetClkDiv(kCLOCK_DivSysCpuAhbClk, 1U);
    CLOCK_SetClkDiv(kCLOCK_DivMainPllClk, 1U);
    CLOCK_AttachClk(kMAIN_PLL_to_MAIN_CLK);

    /* Set SYSTICKFCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivSystickClk, 1U);
    CLOCK_AttachClk(kSYSTICK_DIV_to_SYSTICK_CLK);

    if (BOARD_IS_XIP())
    {
        /* Call function BOARD_SetFlexspiClock() to set clock source to aux0_pll_clk. */
        BOARD_SetFlexspiClock(FLEXSPI, 2U, 2U);
    }

    /* Set PLL FRG clock to 20MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivPllFrgClk, 13U);

    /* Measure main_clk on CLKOUT. Set CLKOUTFCLKDIV divider to value 100 */
    CLOCK_AttachClk(kMAIN_CLK_to_CLKOUT);
    SOCCTRL->TST_TSTBUS_CTRL2 =
        (SOCCTRL->TST_TSTBUS_CTRL2 & ~(SOCCIU_TST_TSTBUS_CTRL2_CLK_OUT_PAGE_SEL_MASK | SOCCIU_TST_TSTBUS_CTRL2_CLK_OUT_SEL_MASK)) |
        SOCCIU_TST_TSTBUS_CTRL2_CLK_OUT_PAGE_SEL(3) | SOCCIU_TST_TSTBUS_CTRL2_CLK_OUT_SEL(14);
    CLOCK_SetClkDiv(kCLOCK_DivClockOut, 100U);

    /* Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_CORE_CLOCK;
}

/*******************************************************************************
 ********************** Configuration BOARD_BootClockLPR ***********************
 ******************************************************************************/
/*******************************************************************************
 * Variables for BOARD_BootClockLPR configuration
 ******************************************************************************/

/*******************************************************************************
 * Code for BOARD_BootClockLPR configuration
 ******************************************************************************/
void BOARD_BootClockLPR(void)
{
    CLOCK_DisableClock(kCLOCK_Pkc);
    CLOCK_DisableClock(kCLOCK_Els);
    CLOCK_DisableClock(kCLOCK_ElsApb);
    CLOCK_DisableClock(kCLOCK_Otp);
    CLOCK_DisableClock(kCLOCK_Wwdt0);
    CLOCK_DisableClock(kCLOCK_Flexcomm0);
    CLOCK_DisableClock(kCLOCK_Flexcomm2);
    CLOCK_DisableClock(kCLOCK_Flexcomm3);
    CLOCK_DisableClock(kCLOCK_Crc);
    CLOCK_DisableClock(kCLOCK_Itrc);

    RESET_SetPeripheralReset(kPKC_RST_SHIFT_RSTn);
    // RESET_SetPeripheralReset(kELS_RST_SHIFT_RSTn);
    RESET_SetPeripheralReset(kELS_APB_RST_SHIFT_RSTn);
    RESET_SetPeripheralReset(kELS_GDET_REF_RST_SHIFT_RSTn);
    RESET_SetPeripheralReset(kOTP_RST_SHIFT_RSTn);
    RESET_SetPeripheralReset(kWWDT_RST_SHIFT_RSTn);
    RESET_SetPeripheralReset(kFC0_RST_SHIFT_RSTn);
    RESET_SetPeripheralReset(kFC2_RST_SHIFT_RSTn);
    RESET_SetPeripheralReset(kFC3_RST_SHIFT_RSTn);
    RESET_SetPeripheralReset(kCRC_RST_SHIFT_RSTn);

    if ((PMU->CAU_SLP_CTRL & PMU_CAU_SLP_CTRL_SOC_SLP_RDY_MASK) == 0U)
    {
        /* LPOSC not enabled, enable it */
        CLOCK_EnableClock(kCLOCK_RefClkCauSlp);
    }
    if ((SYSCTL2->SOURCE_CLK_GATE & SYSCTL2_SOURCE_CLK_GATE_REFCLK_SYS_CG_MASK) != 0U)
    {
        /* REFCLK_SYS not enabled, enable it */
        CLOCK_EnableClock(kCLOCK_RefClkSys);
    }

    if (BOARD_IS_XIP())
    {
        /* Initialize T3 clocks and t3pll_mci_48_60m_irc configured to 48.3MHz */
        CLOCK_InitT3RefClk(kCLOCK_T3MciIrc48m);
        /* Enable T3 256M clock and SFRO */
        CLOCK_EnableClock(kCLOCK_T3PllMci256mClk);

        /* Call function BOARD_FlexspiClockSafeConfig() to move FlexSPI clock to a stable clock source to avoid
           instruction/data fetch issue when updating PLL and Main clock if XIP(execute code on FLEXSPI memory). */
        BOARD_FlexspiClockSafeConfig();
    }
    else
    {
        RESET_ClearPeripheralReset(kFLEXSPI_RST_SHIFT_RSTn);
        BOARD_DeinitFlash(FLEXSPI);
        CLOCK_AttachClk(kNONE_to_FLEXSPI_CLK);
        CLOCK_DisableClock(kCLOCK_Flexspi);
        RESET_SetPeripheralReset(kFLEXSPI_RST_SHIFT_RSTn);
    }

    /* Deinitialize TDDR clocks */
    CLOCK_DeinitTddrRefClk();

    /* First let M33 run on SOSC */
    CLOCK_AttachClk(kSYSOSC_to_MAIN_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivSysCpuAhbClk, 1);

    /* tcpu_mci_clk configured to 260MHz, tcpu_mci_flexspi_clk 312MHz. */
    CLOCK_InitTcpuRefClk(3120000000UL, kCLOCK_TcpuFlexspiDiv10);
    /* Enable tcpu_mci_clk 260MHz. Keep tcpu_mci_flexspi_clk gated. */
    CLOCK_EnableClock(kCLOCK_TcpuMciClk);

    /* Enable AUX0 PLL to 260MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivAux0PllClk, 1U);

    /* Configure MainPll to 260MHz, then let CM33 run on Main PLL. */
    CLOCK_SetClkDiv(kCLOCK_DivSysCpuAhbClk, 1U);
    CLOCK_SetClkDiv(kCLOCK_DivMainPllClk, 1U);
    CLOCK_AttachClk(kMAIN_PLL_to_MAIN_CLK);

    /* Set SYSTICKFCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivSystickClk, 1U);
    CLOCK_AttachClk(kSYSTICK_DIV_to_SYSTICK_CLK);

    if (BOARD_IS_XIP())
    {
        /* Call function BOARD_SetFlexspiClock() to set clock source to aux0_pll_clk. */
        BOARD_SetFlexspiClock(FLEXSPI, 2U, 2U);
    }

    /* Set PLL FRG clock to 20MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivPllFrgClk, 13U);

    /* Deinitialize T3 clocks */
    CLOCK_DeinitT3RefClk();

    /* Deinitialize AVPLL clocks */
    CLOCK_DeinitAvPll();

    /* Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_CORE_CLOCK;
}
