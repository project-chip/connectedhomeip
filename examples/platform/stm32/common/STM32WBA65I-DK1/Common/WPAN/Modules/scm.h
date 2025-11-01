/**
  ******************************************************************************
  * @file    scm.h
  * @author  MCD Application Team
  * @brief   Header for scm.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SCM_H
#define SCM_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#if (CFG_SCM_SUPPORTED == 1)
#include "stm32wbaxx_hal.h"
#include "stm32wbaxx_ll_pwr.h"
#include "stm32wbaxx_ll_rcc.h"
#include "stm32wbaxx_ll_tim.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Exported types ------------------------------------------------------------*/
typedef enum {
  NO_CLOCK_CONFIG = 0,
  HSE_16MHZ,
  HSE_32MHZ,
  SYS_PLL,
} scm_clockconfig_t;

typedef enum {
  LP,
  RUN,
  HSE16,
  HSE32,
  PLL,
} scm_ws_lp_t;

typedef enum {
  HSEPRE_DISABLE = 0,
  HSEPRE_ENABLE
} scm_hse_hsepre_t;

typedef enum {
  SCM_USER_APP,
  SCM_USER_LL_FW,
  SCM_USER_LL_HW_RCO_CLBR,
  TOTAL_CLIENT_NUM, /* To be at the end of the enum */
} scm_user_id_t;

typedef enum {
  NO_PLL,
  PLL_INTEGER_MODE,
  PLL_FRACTIONAL_MODE,
} scm_pll_mode_t;

typedef enum {
  SCM_RADIO_NOT_ACTIVE = 0,
  SCM_RADIO_ACTIVE,
} scm_radio_state_t;

typedef struct {
  uint8_t are_pll_params_initialized;
  scm_pll_mode_t pll_mode;
  uint32_t PLLM;
  uint32_t PLLN;
  uint32_t PLLP;
  uint32_t PLLQ;
  uint32_t PLLR;
  uint32_t PLLFractional;
  uint32_t AHB5_PLL1_CLKDivider;
} scm_pll_config_t;

typedef struct{
  scm_clockconfig_t targeted_clock_freq;
  uint32_t flash_ws_cfg;
  uint32_t sram_ws_cfg;
  scm_pll_config_t pll;
} scm_system_clock_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  System Clock Manager init code
  * @param  None
  * @retval None
  */
void scm_init(void);

/**
  * @brief  Setup the system clock source in usable configuration for Connectivity use cases.
  *         Called at startup or out of low power modes.
  * @param  None
  * @retval None
  */
void scm_setup(void);

/**
  * @brief  Configure the PLL mode and parameters before PLL selection as system clock.
  * @param  p_pll_config PLL coniguration to apply
  * @retval None
  * @note   scm_pll_setconfig to be called before PLL activation (PLL set as system core clock)
  */
void scm_pll_setconfig(const scm_pll_config_t *p_pll_config);

/**
  * @brief  Restore system clock configuration when moving out of standby.
  * @param  None
  * @retval None
  */
void scm_standbyexit(void);

/**
  * @brief  Return the state of the Radio.
  * @param  None
  * @retval radio_state
  */
scm_radio_state_t isRadioActive(void);

/**
  * @brief  Configure the PLL for switching fractional parameters on the fly.
  * @param  pll_frac Up to date fractional configuration.
  * @retval None
  * @note   A PLL update is requested only when the system clock is
  *         running on the PLL with a different configuration that the
  *         one required.
  */
void scm_pll_fractional_update(uint32_t pll_frac);

/**
  * @brief  Set the HSE clock to the requested frequency.
  * @param  user_id This parameter can be one of the following:
  *         @arg SCM_USER_APP
  *         @arg SCM_USER_LL_FW
  * @param  sysclockconfig This parameter can be one of the following:
  *         @arg HSE_16MHZ
  *         @arg HSE_32MHZ
  *         @arg SYS_PLL
  * @retval None
  */
void scm_setsystemclock (scm_user_id_t user_id, scm_clockconfig_t sysclockconfig);

/**
  * @brief  Called each time the PLL is ready
  * @param  None
  * @retval None
  * @note   This function is defined as weak in SCM module.
  *         Can be overridden by user.
  */
void scm_pllready(void);

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
void scm_setwaitstates(const scm_ws_lp_t ws_lp_config);

/**
  * @brief  Notify the state of the Radio
  * @param  radio_state: This parameter can be one of the following:
  *         @arg SCM_RADIO_ACTIVE
  *         @arg SCM_RADIO_NOT_ACTIVE
  * @retval None
  */
void scm_notifyradiostate(const scm_radio_state_t radio_state);

/**
  * @brief  SCM HSERDY interrupt handler.
  *         Switch system clock on HSE.
  * @param  None
  * @retval None
  */
void scm_hserdy_isr(void);

/**
  * @brief  SCM PLLRDY interrupt handler.
  *         Switch system clock on PLL.
  * @param  None
  * @retval None
  */
void scm_pllrdy_isr(void);

/* SCM HSE BEGIN */
/**
 * @brief Getter for SW HSERDY flag
 */
uint8_t SCM_HSE_Get_SW_HSERDY(void);

/**
 * @brief Setter for SW HSERDY flag
 */
void SCM_HSE_Set_SW_HSERDY(void);

/**
 * @brief Clean of SW HSERDY flag
 */
void SCM_HSE_Clear_SW_HSERDY(void);

/**
 * @brief Polling function to wait until HSE is ready
 */
void SCM_HSE_WaitUntilReady(void);

/**
 * @brief Start the HSE stabilization timer
 */
void SCM_HSE_StartStabilizationTimer(void);

/**
 * @brief Stop the HSE stabilization timer
 */
void SCM_HSE_StopStabilizationTimer(void);

/**
 * @brief HSE stabilization timer interrupt handler
 */
void SCM_HSE_SW_HSERDY_isr(void);
/* SCM HSE END */

/* Exported functions - To be implemented by the user ------------------------- */

/**
  * @brief  SCM HSI clock enable
  * @details A weak version is implemented in the module sources.
  * @details It can be overridden by user.
  * @param  None
  * @retval None
  */
extern void SCM_HSI_CLK_ON(void);

/**
  * @brief  SCM HSI clock may be disabled when this function is called
  * @details A weak version is implemented in the module sources.
  * @details It can be overridden by user.
  * @param  None
  * @retval None
  */
extern void SCM_HSI_CLK_OFF(void);

/* SCM HSE BEGIN */
/**
 * @brief Entry hook for HSI switch
 */
extern void SCM_HSI_SwithSystemClock_Entry(void);

/**
 * @brief Exit hook for HSI switch
 */
extern void SCM_HSI_SwithSystemClock_Exit(void);
/* SCM HSE END */
#else /* CFG_SCM_SUPPORTED */

/* Unused empty functions */
void scm_hserdy_isr(void);
void scm_pllrdy_isr(void);

#endif /* CFG_SCM_SUPPORTED */
#ifdef __cplusplus
}
#endif
#endif /* SCM_H */
