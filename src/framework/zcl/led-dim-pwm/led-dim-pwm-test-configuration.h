/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef LED_DIM_PWM_TEST_CONFIGURATION_H
#define LED_DIM_PWM_TEST_CONFIGURATION_H

#define CHIP_AF_API_STACK "stack/include/chip.h"
#define CHIP_AF_API_ZCL_CORE "app/thread/plugin/zcl/zcl-core/zcl-core.h"
#define CHIP_AF_API_BULB_PWM_DRIVER "plugin-common/bulb-pwm-driver/bulb-pwm-driver.h"

#define chipAfCorePrintln(...)

#include "app/thread/test-app/zoip/zoip-test/simulation/thread-zclip.h"

#endif // LED_DIM_PWM_TEST_CONFIGURATION_H
