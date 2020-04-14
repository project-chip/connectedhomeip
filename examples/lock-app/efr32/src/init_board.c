/***************************************************************************//**
 * @file
 * @brief init_board.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#include "hal-config.h"
#else
#include "bspconfig.h"
#endif

#include "board_features.h"
#include "em_cmu.h"


#include "em_cmu.h"

#include "em_usart.h"
#include "mx25flash_spi.h"

#include "bsp.h"

void initBoard(void)
{

  // Enable clock for CRYOTIMER
  CMU_ClockEnable(cmuClock_CRYOTIMER, true);
#if ((HAL_VCOM_ENABLE == 1) \
    || (HAL_USART3_ENABLE == 1) \
    || (HAL_USART1_ENABLE == 1) \
    || (HAL_USART0_ENABLE == 1))
#if defined(FEATURE_EXP_HEADER_USART3)
  // Enable clock for USART3
  CMU_ClockEnable(cmuClock_USART3, true);
#elif defined(FEATURE_EXP_HEADER_USART1)
  // Enable clock for USART1
  CMU_ClockEnable(cmuClock_USART1, true);
#else
  // Enable clock for USART0
  CMU_ClockEnable(cmuClock_USART0, true);
#endif
#endif //(HAL_VCOM_ENABLE == 1)
#if ((HAL_I2CSENSOR_ENABLE == 1) \
    || (HAL_VCOM_ENABLE == 1) \
    || (HAL_SPIDISPLAY_ENABLE == 1) \
    || (HAL_USART3_ENABLE == 1) \
    || (HAL_USART1_ENABLE == 1) \
    || (HAL_USART0_ENABLE == 1))
  // Enable clock for PRS
  CMU_ClockEnable(cmuClock_PRS, true);
  // Enable GPIO clock source
  CMU_ClockEnable(cmuClock_GPIO, true);
#endif /* ((HAL_I2CSENSOR_ENABLE == 1) \
           || (HAL_VCOM_ENABLE == 1) \
           || (HAL_SPIDISPLAY_ENABLE == 1) \
           || (HAL_USART3_ENABLE == 1) \
           || (HAL_USART1_ENABLE == 1) \
           || (HAL_USART0_ENABLE == 1)) */

  // Put the SPI flash into Deep Power Down mode for those radio boards where it is available
  MX25_init();
  MX25_DP();
  // We must disable SPI communication
  MX25_deinit();

}

void initVcomEnable(void)
{
#if defined(HAL_VCOM_ENABLE)
  // Enable VCOM if requested
  GPIO_PinModeSet(BSP_VCOM_ENABLE_PORT, BSP_VCOM_ENABLE_PIN, gpioModePushPull, HAL_VCOM_ENABLE);
#endif // HAL_VCOM_ENABLE
}