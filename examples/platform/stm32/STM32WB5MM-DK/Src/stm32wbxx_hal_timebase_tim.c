/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32wbxx_hal_timebase_tim.c
  * @author  MCD Application Team
  * @brief   HAL time base based on the hardware TIM.
  *
  *          This file overrides the native HAL time base functions (defined as weak)
  *          the TIM time base:
  *           + Initializes the TIM peripheral to generate a Period elapsed Event each 1ms
  *           + HAL_IncTick is called inside HAL_TIM_PeriodElapsedCallback ie each 1ms
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) STMicroelectronics</center></h2>
(-2021) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
 @verbatim
  ==============================================================================
                        ##### How to use this driver #####
  ==============================================================================
    [..]
    This file must be copied to the application folder and modified as follows:
    (#) Rename it to 'stm32wbxx_hal_timebase_tim.c'
    (#) Add this file and the TIM HAL driver files to your project and make sure
       HAL_TIM_MODULE_ENABLED is defined in stm32wbxx_hal_conf.h

    [..]
    (@) The application needs to ensure that the time base is always set to 1 millisecond
       to have correct HAL operation.

  @endverbatim
  ******************************************************************************
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "stm32wbxx_hal.h"
#include "stm32wbxx_hal_tim.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim17;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  This function configures the TIM17 as a time base source.
 *         The time source is configured  to have 1ms time base with a dedicated
 *         Tick interrupt priority.
 * @note   This function is called  automatically at the beginning of program after
 *         reset by HAL_Init() or at any time when clock is configured, by HAL_RCC_ClockConfig().
 * @param  TickPriority: Tick interrupt priority.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    RCC_ClkInitTypeDef clkconfig;
    uint32_t uwTimclock       = 0;
    uint32_t uwPrescalerValue = 0;
    uint32_t pFLatency;
    /*Configure the TIM17 IRQ priority */
    HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM17_IRQn, TickPriority, 0);

    /* Enable the TIM17 global Interrupt */
    HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM17_IRQn);

    /* Enable TIM17 clock */
    __HAL_RCC_TIM17_CLK_ENABLE();

    /* Get clock configuration */
    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

    /* Compute TIM17 clock */
    uwTimclock = HAL_RCC_GetPCLK2Freq();
    /* Compute the prescaler value to have TIM17 counter clock equal to 1MHz */
    uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);

    /* Initialize TIM17 */
    htim17.Instance = TIM17;

    /* Initialize TIMx peripheral as follow:
    + Period = [(TIM17CLK/1000) - 1]. to have a (1/1000) s time base.
    + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
    + ClockDivision = 0
    + Counter direction = Up
    */
    htim17.Init.Period        = (1000000U / 1000U) - 1U;
    htim17.Init.Prescaler     = uwPrescalerValue;
    htim17.Init.ClockDivision = 0;
    htim17.Init.CounterMode   = TIM_COUNTERMODE_UP;

    if (HAL_TIM_Base_Init(&htim17) == HAL_OK)
    {
        /* Start the TIM time Base generation in interrupt mode */
        return HAL_TIM_Base_Start_IT(&htim17);
    }

    return HAL_ERROR;
}

/**
 * @brief  Suspend Tick increment.
 * @note   Disable the tick increment by disabling TIM17 update interrupt.
 * @param  None
 * @retval None
 */
void HAL_SuspendTick(void)
{
    /* Disable TIM17 update Interrupt */
    __HAL_TIM_DISABLE_IT(&htim17, TIM_IT_UPDATE);
}

/**
 * @brief  Resume Tick increment.
 * @note   Enable the tick increment by Enabling TIM17 update interrupt.
 * @param  None
 * @retval None
 */
void HAL_ResumeTick(void)
{
    /* Enable TIM17 Update interrupt */
    __HAL_TIM_ENABLE_IT(&htim17, TIM_IT_UPDATE);
}
