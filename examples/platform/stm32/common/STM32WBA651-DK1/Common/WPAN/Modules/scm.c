/**
  ******************************************************************************
  * @file    scm.c
  * @author  MCD Application Team
  * @brief   Functions for the System Clock Manager.
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
#include "scm.h"
#include "RTDebug.h"
#include "utilities_common.h"

#if (CFG_SCM_SUPPORTED == 1)

__weak void SCM_HSI_CLK_ON(void)
{
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() == 0);
}

__weak void SCM_HSI_CLK_OFF(void)
{

}

/* SCM HSE BEGIN */
__weak void SCM_HSI_SwithSystemClock_Entry(void)
{

}

__weak void SCM_HSI_SwithSystemClock_Exit(void)
{

}
/* SCM HSE END */
/* Private typedef -----------------------------------------------------------*/
#define PLL_INPUTRANGE0_FREQMAX         8000000u  /* 8 MHz is maximum frequency for VCO input range 0 */

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* SCM HSE BEGIN */
static uint8_t SW_HSERDY = 0;
/* SCM HSE END */

RAMCFG_HandleTypeDef sram1_ns =
{
  RAMCFG_SRAM1,           /* Instance */
  HAL_RAMCFG_STATE_READY, /* RAMCFG State */
  0U,                     /* RAMCFG Error Code */
};

RAMCFG_HandleTypeDef sram2_ns =
{
  RAMCFG_SRAM2,           /* Instance */
  HAL_RAMCFG_STATE_READY, /* RAMCFG State */
  0U,                     /* RAMCFG Error Code */
};

static scm_system_clock_t scm_system_clock_config;
static scm_clockconfig_t scm_system_clock_requests[(scm_user_id_t)TOTAL_CLIENT_NUM] = {NO_CLOCK_CONFIG};
static scm_radio_state_t RadioState;

/* Private function prototypes -----------------------------------------------*/
static scm_clockconfig_t scm_getmaxfreq(void);
static void scm_systemclockconfig(void);
static void ConfigStartPll(void);
static void ConfigHwPll(scm_pll_config_t *p_hw_config);
static void SwitchHsePre(scm_hse_hsepre_t hse_pre);
static void SwitchHse16toHse32(void);
static void SwitchHse32toHse16(void);
static void SwitchPlltoHse32(void);
/* SCM HSE BEGIN */
/**
 * @brief Initialize the timer for HSE stabilization
 */
static void SCM_HSE_TimerInit(void);
/* SCM HSE END */

/* Private functions ---------------------------------------------------------*/
OPTIMIZED static scm_clockconfig_t scm_getmaxfreq(void)
{
  uint8_t idx = 0;
  scm_clockconfig_t max = NO_CLOCK_CONFIG;

  for(idx = 0; idx < sizeof(scm_system_clock_requests) ; idx++)
  {
    if(scm_system_clock_requests[idx] > max)
    {
      max = scm_system_clock_requests[idx];
    }
  }

  return max;
}

OPTIMIZED static void scm_systemclockconfig(void)
{
  SYSTEM_DEBUG_SIGNAL_SET(SCM_SYSTEM_CLOCK_CONFIG);

  switch (scm_system_clock_config.targeted_clock_freq)
  {
    case HSE_16MHZ:

      if(LL_RCC_GetSysClkSource() == LL_RCC_SYS_CLKSOURCE_STATUS_PLL1R)
      {
        /* currently running on PLL */
        SwitchPlltoHse32();
      }

      SwitchHse32toHse16();

      /* Ensure time base clock coherency */
      SystemCoreClockUpdate();

      break;

    case HSE_32MHZ:

      if (LL_RCC_HSE_IsEnabledPrescaler())
      {
        /* currently running on HSE16 */
        SwitchHse16toHse32();

        /* Ensure time base clock coherency */
        SystemCoreClockUpdate();
      }
      else if(LL_RCC_GetSysClkSource() == LL_RCC_SYS_CLKSOURCE_STATUS_PLL1R)
      {
        /* currently running on PLL */
        SwitchPlltoHse32();

        /* Ensure time base clock coherency */
        SystemCoreClockUpdate();
      }
      else
      {
        /**
          * The system is already running on HSE32
          * The only case is when the PLL has been requested and
          * aborted before the system switched to PLL
          */

        /* Disable PLL */
        LL_RCC_PLL1_Disable();

        /**
          * Disable PLL1RDY interrupt
          * No need to worry the case when the PLL interrupt
          * may already be pending at this time
          */
        __HAL_RCC_DISABLE_IT(RCC_IT_PLL1RDY);
      }

      break;

    case SYS_PLL:

      if (LL_RCC_HSE_IsEnabledPrescaler())
      {
        /* currently running on HSE16 */
        SwitchHse16toHse32();

        /* Ensure time base clock coherency */
        SystemCoreClockUpdate();
      }

      ConfigStartPll();

      break;

    default:
      break;
  }

  SYSTEM_DEBUG_SIGNAL_RESET(SCM_SYSTEM_CLOCK_CONFIG);
}

OPTIMIZED static void SwitchHsePre(scm_hse_hsepre_t hse_pre)
{
  /* Start HSI */
  SCM_HSI_CLK_ON();
  
  /* SCM HSE BEGIN */
  /* Entry hook for HSI switch */
  SCM_HSI_SwithSystemClock_Entry();
  /* SCM HSE END */

  /* Set HSI as SYSCLK */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI);

  /* Enable HSEON */
  /* SCM HSE BEGIN */
  LL_RCC_HSE_Enable();
  SCM_HSE_WaitUntilReady();
  
  /* Exit hook for HSI switch */
  SCM_HSI_SwithSystemClock_Exit();
  /* SCM HSE END */
  
  /* Set/Clear HSEPRE */
  if(hse_pre == HSEPRE_DISABLE)
  {
    LL_RCC_HSE_DisablePrescaler();
  }
  else
  {
    LL_RCC_HSE_EnablePrescaler();
  }

  /* Set HSE as SYSCLK */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSE);
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSE);

  /* Disable HSI */
  SCM_HSI_CLK_OFF();

#if defined(STM32WBAXX_SI_CUT1_0)
  /* STM32WBA5 Cut1.0 only: if the radio is not active is set to OFF by the hardware. */
  if(isRadioActive() == SCM_RADIO_NOT_ACTIVE)
  {
    /* SCM HSE BEGIN */
    SCM_HSE_Clear_SW_HSERDY();
	/* SCM HSE END */
  }
#endif /* STM32WBAXX_SI_CUT1_0 */
}

OPTIMIZED static void SwitchHse16toHse32(void)
{
  /**
    * Switch from HSE_16MHz to HSE_32MHz
    * 1. Voltage Range1
    * 2. Disable prescaler ==> HSE16 to HSE32
    * 3. Change RAM/FLASH waitstates (no limitation in Rang1)
    * 4. AHB5 Div 1
    */

  /* first switch to VOS1 */
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  while (LL_PWR_IsActiveFlag_VOS() == 0);

  /* Switch to 32Mhz */
  SwitchHsePre(HSEPRE_DISABLE);

  /* Configure flash and SRAMs */
  scm_setwaitstates(HSE32);

  /* Need to set HDIV5 */
  LL_RCC_SetAHB5Divider(LL_RCC_AHB5_DIVIDER_1); /* divided by 1 */
}

OPTIMIZED static void SwitchHse32toHse16(void)
{
  /**
    * Switch from HSE_16MHz to HSE_32MHz
    * 1. AHB5 Div 2
    * 2. Change RAM/FLASH waitstates
    * 3. Disable prescaler ==> HSE16 to HSE32
    * 4. Voltage Range2
    */

  /* Divide HDIV5 by 2 */
  LL_RCC_SetAHB5Divider(LL_RCC_AHB5_DIVIDER_2);

  /* Configure flash and SRAMs before switching to VOS2 */
  scm_setwaitstates(HSE16);

  /* Switch to HSE 16 */
  SwitchHsePre(HSEPRE_ENABLE);

  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);
}

OPTIMIZED static void SwitchPlltoHse32(void)
{
  /**
    * Switch from PLL to HSE_32MHz
    * 1. Switch system clock source to HSE
    * 2. Turn OFF PLL
    * 3. Change RAM/FLASH waitstates (no limitation in Rang1)
    */

  /* Switch to HSE */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSE);
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSE);

  /* Disable PLL */
  LL_RCC_PLL1_Disable();

  /* Configure flash and SRAMs */
  scm_setwaitstates(HSE32);
}

OPTIMIZED static void ConfigStartPll(void)
{
  /* Enable PLL1 output for SYSCLK (PLL1R) */
  LL_RCC_PLL1_EnableDomain_PLL1R();

  /* Configure and start the PLL */
  LL_RCC_PLL1_SetMainSource(LL_RCC_PLL1SOURCE_HSE);

  /* Enable PLL1 */
  __HAL_RCC_PLL1_ENABLE();

  /* PLL1RDY interrupt raised when PLL is enabled */
  __HAL_RCC_ENABLE_IT(RCC_IT_PLL1RDY);
}

static void ConfigHwPll(scm_pll_config_t *p_hw_config)
{
  uint32_t freq_vco_in = 0;

  /* Apply user PLL mode */
  if(p_hw_config->pll_mode == PLL_FRACTIONAL_MODE)
  {
    scm_pll_fractional_update(p_hw_config->PLLFractional);
  }
  else
  {
    /* Integer configuration will be used for PLL mode */
    LL_RCC_PLL1FRACN_Disable();
  }

  /* Apply correct frequency range for VCO_IN */
  /* Note as PLL clock source is always HSE 32MHz, only PLL1M value impact VCO_IN */

  freq_vco_in = 32000000UL/p_hw_config->PLLM;
  if (freq_vco_in > PLL_INPUTRANGE0_FREQMAX)
  {
    freq_vco_in = RCC_PLL_VCOINPUT_RANGE1;
  }
  else
  {
    freq_vco_in = RCC_PLL_VCOINPUT_RANGE0;
  }
  __HAL_RCC_PLL1_VCOINPUTRANGE_CONFIG(freq_vco_in);

  __HAL_RCC_PLL1_CONFIG(RCC_PLLSOURCE_HSE, /* PLL clock source is always HSE 32MHz */
                        p_hw_config->PLLM,
                        p_hw_config->PLLN,
                        p_hw_config->PLLP,
                        p_hw_config->PLLQ,
                        p_hw_config->PLLR
                        );

  LL_RCC_SetAHB5Prescaler(p_hw_config->AHB5_PLL1_CLKDivider);

  /* PLL is now initialized */
  scm_system_clock_config.pll.are_pll_params_initialized = 1;
}

/* SCM HSE BEGIN */
static void SCM_HSE_TimerInit(void)
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM16);

  /* TIM16 interrupt Init */
  NVIC_SetPriority(TIM16_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1, 0));
  NVIC_EnableIRQ(TIM16_IRQn);

  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_DOWN;
  TIM_InitStruct.Autoreload = 3200;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM16, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM16);
  LL_TIM_SetOnePulseMode(TIM16, LL_TIM_ONEPULSEMODE_SINGLE);
}
/* SCM HSE END */

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  System Clock Manager init code
  * @param  None
  * @retval None
  */
OPTIMIZED void scm_init()
{
  /* init scm_system_clock_config with LP config
   * scm_system_clock_config SHALL BE UPDATED BY READING HW CONFIG FROM HAL APIs
   * SHALL BE CALLED AFTER SystemClock_Config()
   **/

  /* Default PLL configuration => no configuration */
  memset(&(scm_system_clock_config.pll), 0, sizeof(scm_pll_config_t));

  /* Reading FLASH and SRAMs waitstates from registers */
  scm_system_clock_config.flash_ws_cfg = __HAL_FLASH_GET_LATENCY();
  scm_system_clock_config.sram_ws_cfg = HAL_RAMCFG_GetWaitState(&sram1_ns);

  /* Link Layer is not active at this stage */
  RadioState = SCM_RADIO_NOT_ACTIVE;

  /* Enable RAMCFG clock */
  __HAL_RCC_RAMCFG_CLK_ENABLE();

  /* SCM HSE BEGIN */
  /* Init SW HSE Flag */
  SCM_HSE_Set_SW_HSERDY();
  
  /* Init timer for HSE stabilization measurement */
  SCM_HSE_TimerInit ();
  /* SCM HSE END */

  /* Reading system core clock configuration from registers */
  switch(LL_RCC_GetSysClkSource())
  {
    case LL_RCC_SYS_CLKSOURCE_STATUS_HSI:
      /* HSI system clock configuration is not supported on SCM module as radio activity is not possible.
       * Switch to HSE_16MHz required.
       */

      /* Target system clock frequency is now HSE_16MHZ */
      scm_system_clock_config.targeted_clock_freq = HSE_16MHZ;

      /* Enable prescaler */
       LL_RCC_HSE_EnablePrescaler();

      /* Set HDIV 5 */
      LL_RCC_SetAHB5Divider(LL_RCC_AHB5_DIVIDER_2); /* divided by 2 */

      scm_setup();

      /* Set VOS to range 2 */
      LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);

      break;

    case LL_RCC_SYS_CLKSOURCE_STATUS_HSE:

      /* Get AHB5 divider for HSE frequency */
      if (LL_RCC_HSE_IsEnabledPrescaler())
      {
        /* System core clock is HSE_16MHz */
        scm_system_clock_config.targeted_clock_freq = HSE_16MHZ;
      }
      else
      {
        /* System core clock is HSE_32MHz */
        scm_system_clock_config.targeted_clock_freq = HSE_32MHZ;
      }

      break;

    case LL_RCC_SYS_CLKSOURCE_STATUS_PLL1R:
        scm_system_clock_config.targeted_clock_freq = SYS_PLL;

        /* Initial PLL configuration */
        scm_system_clock_config.pll.PLLM = LL_RCC_PLL1_GetDivider();
        scm_system_clock_config.pll.PLLN = LL_RCC_PLL1_GetN();
        scm_system_clock_config.pll.PLLP = LL_RCC_PLL1_GetP();
        scm_system_clock_config.pll.PLLQ = LL_RCC_PLL1_GetQ();
        scm_system_clock_config.pll.PLLR = LL_RCC_PLL1_GetR();
        scm_system_clock_config.pll.PLLFractional = LL_RCC_PLL1_GetFRACN();
        scm_system_clock_config.pll.AHB5_PLL1_CLKDivider = LL_RCC_GetAHB5Prescaler();
        if(scm_system_clock_config.pll.PLLFractional == PLL_FRACTIONAL_MODE)
        {
          scm_system_clock_config.pll.pll_mode = PLL_FRACTIONAL_MODE;
        }
        else
        {
          scm_system_clock_config.pll.pll_mode = PLL_INTEGER_MODE;
        }

      break;
  }

  scm_system_clock_requests[SCM_USER_APP]= scm_system_clock_config.targeted_clock_freq;
}

/**
  * @brief  Setup the system clock source in usable configuration for Connectivity use cases.
  *         Called at startup or out of low power modes.
  * @param  None
  * @retval None
  */
OPTIMIZED void scm_setup(void)
{
  SYSTEM_DEBUG_SIGNAL_SET(SCM_SETUP);

  /* System clock is now on HSI 16Mhz, as it exits from stop mode */

  /* Start HSE */
  LL_RCC_HSE_Enable();
  
  /* SCM HSE BEGIN */
  if ((SCM_HSE_Get_SW_HSERDY() != 0) && (RadioState == SCM_RADIO_ACTIVE))
  /* SCM HSE END */
  {
    /**
      * The current system configuration is:
      * Range1, HDIV5 cleared, HSEPRE cleared
      */

    /* Switch System Clock on HSE32 */
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSE);

    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSE);

    scm_setwaitstates(HSE32); /* There is no limitation when in Range1 */

    /* As system switched to HSE, disable HSI */
    SCM_HSI_CLK_OFF();

    /* Check if the clock system used PLL before low power mode entry */
    if(scm_system_clock_config.targeted_clock_freq == SYS_PLL)
    {
      /* Configure system clock to use PLL */
      ConfigStartPll();
    }

    /* Ensure time base clock coherency */
    SystemCoreClockUpdate();
  }
  else
  {
    scm_setwaitstates(HSE16);

    /* Check if the system need to increase VOS range (clock frequency higher than HSE 16Mhz)*/
    if(scm_system_clock_config.targeted_clock_freq != HSE_16MHZ)
    {
      /* Set VOS to range 1 */
      LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    }

    /* SCM HSE BEGIN */
    if (SCM_HSE_Get_SW_HSERDY() != 0)
    /* SCM HSE END */
    {
      scm_hserdy_isr();
    }
    else
    {      
      /* Disable RCC IRQs */
      HAL_NVIC_DisableIRQ(RCC_IRQn);
      
      /* Enable HSERDY interrupt */      
      __HAL_RCC_ENABLE_IT(RCC_IT_HSERDY);

      if (LL_RCC_HSE_IsReady() != 0)
      {
        __HAL_RCC_CLEAR_IT(RCC_IT_HSERDY);
        
        /* SCM HSE BEGIN */
        SCM_HSE_StartStabilizationTimer();
        /* SCM HSE END */
      }
      
      /* Enable RCC IRQs */
      HAL_NVIC_EnableIRQ(RCC_IRQn);
    }
  }
  SYSTEM_DEBUG_SIGNAL_RESET(SCM_SETUP);
}

/**
  * @brief  Configure the PLL mode and parameters before PLL selection as system clock.
  * @param  p_pll_config PLL coniguration to apply
  * @retval None
  * @note   scm_pll_setconfig to be called before PLL activation (PLL set as system core clock)
  */
OPTIMIZED void scm_pll_setconfig(const scm_pll_config_t *p_pll_config)
{
  /* Initial PLL configuration */
  scm_system_clock_config.pll.PLLM = p_pll_config->PLLM;
  scm_system_clock_config.pll.PLLN = p_pll_config->PLLN;
  scm_system_clock_config.pll.PLLP = p_pll_config->PLLP;
  scm_system_clock_config.pll.PLLQ = p_pll_config->PLLQ;
  scm_system_clock_config.pll.PLLR = p_pll_config->PLLR;
  scm_system_clock_config.pll.PLLFractional = p_pll_config->PLLFractional;
  scm_system_clock_config.pll.pll_mode = p_pll_config->pll_mode;
  scm_system_clock_config.pll.AHB5_PLL1_CLKDivider = p_pll_config->AHB5_PLL1_CLKDivider;

  ConfigHwPll(&scm_system_clock_config.pll);
}

/**
  * @brief  Configure the PLL for switching fractional parameters on the fly.
  * @param  pll_frac Up to date fractional configuration.
  * @retval None
  * @note   A PLL update is requested only when the system clock is
  *         running on the PLL with a different configuration that the
  *         one required
  */
OPTIMIZED void scm_pll_fractional_update(uint32_t pll_frac)
{
  /* PLL1FRACEN set to 0 */
  LL_RCC_PLL1FRACN_Disable();

  /* Update PLL1FRACR register */
  LL_RCC_PLL1_SetFRACN(pll_frac);

  /* PLL1FRACEN set to 1 */
  LL_RCC_PLL1FRACN_Enable();

  /* Ensure time base clock coherency */
  SystemCoreClockUpdate();
}

/**
  * @brief  Set the system clock to the requested frequency.
  * @param  user_id This parameter can be one of the following:
  *         @arg SCM_USER_APP
  *         @arg SCM_USER_LL_FW
  * @param  sysclockconfig This parameter can be one of the following:
  *         @arg HSE_16MHZ
  *         @arg HSE_32MHZ
  *         @arg SYS_PLL
  * @retval None
  */
OPTIMIZED void scm_setsystemclock(scm_user_id_t user_id, scm_clockconfig_t sysclockconfig)
{
  scm_clockconfig_t max_freq_requested;

  UTILS_ENTER_LIMITED_CRITICAL_SECTION(RCC_INTR_PRIO<<4);

  /* Register the request by updating the requested frequency for this user */
  scm_system_clock_requests[user_id] = sysclockconfig;

  /* Get the higher frequency required by the clients */
  max_freq_requested = scm_getmaxfreq();

  /* Check the current system clock source (HSI or HSE) */
  if(LL_RCC_GetSysClkSource() == LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
    scm_system_clock_config.targeted_clock_freq = max_freq_requested;
    
    /* HSI is still the system clock */

    if(scm_system_clock_config.targeted_clock_freq == HSE_16MHZ)
    {
      /* The system clock target is HSE 16Mhz */

      /* Clear VOS (Range 2) */
      LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);
    }
    else
    {
      /* The system clock target is higher than HSE 16Mhz */

      /* Set VOS (Range 1) */
      LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

      if(RadioState != SCM_RADIO_NOT_ACTIVE)
      {
        /* Disable HSERDY interrupt */
        __HAL_RCC_DISABLE_IT(RCC_IT_HSERDY);

        /* Wait until VOS has changed */
        while (LL_PWR_IsActiveFlag_VOS() == 0);

        /* Wait until HSE is ready */
        /* SCM HSE BEGIN */
        SCM_HSE_WaitUntilReady();
        /* SCM HSE END */

        LL_RCC_HSE_DisablePrescaler();

        /* Switch to HSE */
        LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSE);
        while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSE);

        scm_setwaitstates(HSE32); /* There is no limitation when in Range1 */

        LL_RCC_SetAHB5Divider(LL_RCC_AHB5_DIVIDER_1);

        SCM_HSI_CLK_OFF();

        /* Check if PLL is requested */
        if(scm_system_clock_config.targeted_clock_freq == SYS_PLL)
        {
            /* Configure system clock to use PLL */
            ConfigStartPll();
        }

        /* Ensure time base clock coherency */
        SystemCoreClockUpdate();
      }
    }

    /* System clock is going to be configured in RCC HSERDY interrupt */
  }
  else
  {
    if (scm_system_clock_config.targeted_clock_freq != max_freq_requested)
    {
      scm_system_clock_config.targeted_clock_freq = max_freq_requested;
      
      /* HSE is already the system clock source */
      /* Configure the system clock */
      scm_systemclockconfig();        
    }      
  }

  UTILS_EXIT_LIMITED_CRITICAL_SECTION();
}

/**
  * @brief  Called each time the PLL is ready
  * @param  None
  * @retval None
  * @note   This function is defined as weak in SCM module.
  *         Can be overridden by user.
  */
__WEAK void scm_pllready(void)
{
  /* To be override by user */
}

/**
  * @brief  Configure the Flash and SRAMs wait cycle (when required for system clock source change)
  * @param  ws_lp_config: This parameter can be one of the following:
  *         @arg LP
  *         @arg RUN
  *         @arg HSE16
  *         @arg HSE32
  *         @arg PLL
  * @retval None
  */
OPTIMIZED void scm_setwaitstates(const scm_ws_lp_t ws_lp_config)
{
  /* Configure flash and SRAMs */
  switch (ws_lp_config) {
  case LP:
    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_3);
    while(__HAL_FLASH_GET_LATENCY() != FLASH_LATENCY_3);
    HAL_RAMCFG_ConfigWaitState(&sram1_ns, RAMCFG_WAITSTATE_1);
    HAL_RAMCFG_ConfigWaitState(&sram2_ns, RAMCFG_WAITSTATE_1);
    break;

  case RUN:
    __HAL_FLASH_SET_LATENCY(scm_system_clock_config.flash_ws_cfg);
    while(__HAL_FLASH_GET_LATENCY() != scm_system_clock_config.flash_ws_cfg);
    HAL_RAMCFG_ConfigWaitState(&sram1_ns, scm_system_clock_config.sram_ws_cfg);
    HAL_RAMCFG_ConfigWaitState(&sram2_ns, scm_system_clock_config.sram_ws_cfg);
    break;

  case HSE16:
    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_1);
    while(__HAL_FLASH_GET_LATENCY() != FLASH_LATENCY_1);
    HAL_RAMCFG_ConfigWaitState(&sram1_ns, RAMCFG_WAITSTATE_1);
    HAL_RAMCFG_ConfigWaitState(&sram2_ns, RAMCFG_WAITSTATE_1);

    scm_system_clock_config.flash_ws_cfg = FLASH_LATENCY_1;
    scm_system_clock_config.sram_ws_cfg = RAMCFG_WAITSTATE_1;

    break;

  case HSE32:
    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_0);
    while(__HAL_FLASH_GET_LATENCY() != FLASH_LATENCY_0);
    HAL_RAMCFG_ConfigWaitState(&sram1_ns, RAMCFG_WAITSTATE_0);
    HAL_RAMCFG_ConfigWaitState(&sram2_ns, RAMCFG_WAITSTATE_0);

    scm_system_clock_config.flash_ws_cfg = FLASH_LATENCY_0;
    scm_system_clock_config.sram_ws_cfg = RAMCFG_WAITSTATE_0;

    break;

  case PLL:
    /* RAM latencies are alreadey set to 0WS */
    /* Set Flash LATENCY according to PLL configuration */
    /* BELOW CONFIGURATION IS WORST CASE, SHALL BE OPTIMIZED */
    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_3);
    while(__HAL_FLASH_GET_LATENCY() != FLASH_LATENCY_3);
    scm_system_clock_config.flash_ws_cfg = FLASH_LATENCY_3;
    break;

  default:
    break;
  }
}

/**
  * @brief  SCM HSERDY interrupt handler.
  *         Switch system clock on HSE.
  * @param  None
  * @retval None
  */
OPTIMIZED void scm_hserdy_isr(void)
{
  SYSTEM_DEBUG_SIGNAL_SET(SCM_HSERDY_ISR);

  if(LL_RCC_GetSysClkSource() == LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
    /* Wait until VOS has changed */
    while (LL_PWR_IsActiveFlag_VOS() == 0);

    if(scm_system_clock_config.targeted_clock_freq == HSE_16MHZ)
    {
      /**
        * The current system configuration is:
        * Range2, HDIV5 set, Wait States compliant to HSE16
        */
      LL_RCC_HSE_EnablePrescaler();
      /* Switch to HSE */
      LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSE);
      while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSE);
    }
    else
    {
      /**
        * The current system configuration is:
        * Range1
        */

      LL_RCC_HSE_DisablePrescaler();

      /* Switch to HSE */
      LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSE);
      while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSE);

      scm_setwaitstates(HSE32); /* There is no limitation when in Range1 */

      if(scm_system_clock_config.targeted_clock_freq == SYS_PLL)
      {
        /* The system clock target is based on PLL */

        /* Configure and start PLL */
        ConfigStartPll();
      }

      /* Set HDIV 5 */
      LL_RCC_SetAHB5Divider(LL_RCC_AHB5_DIVIDER_1); /* divided by 1 */
    }

    /* As system switched to HSE, disable HSI */
    SCM_HSI_CLK_OFF();

    /* Disable HSERDY interrupt */
    __HAL_RCC_DISABLE_IT(RCC_IT_HSERDY);

    /* Ensure time base clock coherency */
    SystemCoreClockUpdate();
  }

  SYSTEM_DEBUG_SIGNAL_RESET(SCM_HSERDY_ISR);
}

/**
  * @brief  SCM PLLRDY interrupt handler.
  *         Switch system clock on PLL.
  * @param  None
  * @retval None
  */
OPTIMIZED void scm_pllrdy_isr(void)
{
  if(scm_system_clock_config.targeted_clock_freq == SYS_PLL)
  {
    /* Set PLL compatible waitstates */
    scm_setwaitstates(PLL);

    /* Switch to PLL */
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1R);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1R);

    /* Ensure time base clock coherency */
    SystemCoreClockUpdate();

    scm_pllready();
  }
  else
  {
    /**
      * The PLL was enabled but is not used anymore as system clock
      * The only case is when a request has been made and cancelled before
      * the system had time to switch the system clock on PLL
      */
    /* Disable PLL */
    LL_RCC_PLL1_Disable();

    /* Disable PLL1RDY interrupt */
    __HAL_RCC_DISABLE_IT(RCC_IT_PLL1RDY);
  }
}

/**
  * @brief  Notify the state of the Radio
  * @param  radio_state: This parameter can be one of the following:
  *         @arg SCM_RADIO_ACTIVE
  *         @arg SCM_RADIO_NOT_ACTIVE
  * @retval None
  */
OPTIMIZED void scm_notifyradiostate(const scm_radio_state_t radio_state)
{
  if(radio_state != SCM_RADIO_NOT_ACTIVE)
  {
    RadioState = SCM_RADIO_ACTIVE; /* shall be set before calling scm_setsystemclock() */
    scm_setsystemclock(SCM_USER_LL_FW, HSE_32MHZ); /* shall be set before calling scm_setsystemclock() */
  }
  else
  {
    RadioState = SCM_RADIO_NOT_ACTIVE;
    scm_setsystemclock(SCM_USER_LL_FW, HSE_16MHZ);
  }
}

/**
  * @brief  Restore system clock configuration when moving out of standby.
  * @param  None
  * @retval None
  */
OPTIMIZED void scm_standbyexit(void)
{
  if(scm_system_clock_config.pll.are_pll_params_initialized == 1)
  {
    /* Restore PLL even if not yet used in case it has been setup upfron at initialization */
    ConfigHwPll(&scm_system_clock_config.pll);
  }
  
  /* SCM HSE BEGIN */
  /* Init timer for HSE stabilization measurement */
  SCM_HSE_TimerInit ();
  /* SCM HSE END */

  scm_setup();
}

/* SCM HSE BEGIN */
scm_radio_state_t isRadioActive(void)
{
  return RadioState;
}

OPTIMIZED uint8_t SCM_HSE_Get_SW_HSERDY(void)
{
  return SW_HSERDY;
}

OPTIMIZED void SCM_HSE_Set_SW_HSERDY(void)
{
  UTILS_ENTER_CRITICAL_SECTION();
  SW_HSERDY = 1;
  UTILS_EXIT_CRITICAL_SECTION();
}

OPTIMIZED void SCM_HSE_Clear_SW_HSERDY(void)
{
  UTILS_ENTER_CRITICAL_SECTION();
  SW_HSERDY = 0;
  UTILS_EXIT_CRITICAL_SECTION();
}

OPTIMIZED void SCM_HSE_WaitUntilReady(void)
{  
  if(SCM_HSE_Get_SW_HSERDY() == 0)
  {
    /* Is timer already running ? */
    if (LL_TIM_IsEnabledCounter(TIM16) != 0)
    {  
      /* Disable update event */
      LL_TIM_DisableIT_UPDATE(TIM16);
      
      /* Blocking wait until the end of stabilization */
      while(LL_TIM_GetCounter(TIM16) != 0);
    }
    else
    {
      /* Check whether update interrupt is pending */
      if(LL_TIM_IsActiveFlag_UPDATE(TIM16) == 1)
      {
        /* Clear the update interrupt flag */
        LL_TIM_ClearFlag_UPDATE(TIM16);
      }
      else
      {
        /* Active wait on HSERDY flag */
        while (LL_RCC_HSE_IsReady() == 0);
            
        /* Clear the update flag */
        LL_TIM_ClearFlag_UPDATE(TIM16);
            
        LL_TIM_DisableIT_UPDATE(TIM16);

        LL_TIM_EnableCounter(TIM16);
        
        /* Wait until the timer is ready */
        while(LL_TIM_IsEnabledCounter(TIM16) == 0);
        
        /* Wait until the timer is over - ie: Stabilization done */
        while(LL_TIM_GetCounter(TIM16) != 0);

        LL_TIM_EnableIT_UPDATE(TIM16);
      }
    }
    
    /* Stop timer and switch clock 
       Keep same post process than if timer interrupt has been served */
    SCM_HSE_SW_HSERDY_isr();
  }
}

OPTIMIZED void SCM_HSE_StartStabilizationTimer(void)
{  
  if((SCM_HSE_Get_SW_HSERDY() == 0) && (LL_TIM_IsEnabledCounter(TIM16) == 0))
  {   
    /* Clear the update flag */
    LL_TIM_ClearFlag_UPDATE(TIM16);
    
    LL_TIM_EnableUpdateEvent(TIM16);
    
    /* Enable the update interrupt */
    LL_TIM_EnableIT_UPDATE(TIM16);
    
    /* Enable counter */
    LL_TIM_EnableCounter(TIM16);
  }
}

OPTIMIZED void SCM_HSE_StopStabilizationTimer(void)
{
  UTILS_ENTER_CRITICAL_SECTION();
  
  /* Disable counter */
  LL_TIM_DisableCounter(TIM16);
  
  LL_TIM_DisableUpdateEvent(TIM16);
  
  LL_TIM_DisableIT_UPDATE(TIM16);
  
  UTILS_EXIT_CRITICAL_SECTION();
}

OPTIMIZED void SCM_HSE_SW_HSERDY_isr(void)
{
  /* Set the SW HSERDY flag */
  SCM_HSE_Set_SW_HSERDY();
  
  /* Stop the timer */
  SCM_HSE_StopStabilizationTimer();
  
  scm_hserdy_isr();
}
/* SCM HSE END */
#else /* CFG_SCM_SUPPORTED */
__weak void scm_pllrdy_isr(void){/* Intentionally enpty */}
__weak void scm_hserdy_isr(void){/* Intentionally enpty */}
#endif /* CFG_SCM_SUPPORTED */
