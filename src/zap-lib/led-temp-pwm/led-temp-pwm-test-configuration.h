/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
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
