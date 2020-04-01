/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef LED_TEMP_PWM_TEST_CONFIGURATION_H
#define LED_TEMP_PWM_TEST_CONFIGURATION_H

#define EMBER_AF_API_STACK "stack/include/ember.h"
#define EMBER_AF_API_ZCL_CORE "app/thread/plugin/zcl/zcl-core/zcl-core.h"
#define EMBER_AF_API_BULB_PWM_DRIVER "plugin-common/bulb-pwm-driver/bulb-pwm-driver.h"

#define emberAfCorePrintln(...)

#include "app/thread/test-app/zoip/zoip-test/simulation/thread-zclip.h"

#endif // LED_TEMP_PWM_TEST_CONFIGURATION_H
