/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief Include file for em4 unit tests
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_EM4_TEST_H
#define SILABS_EM4_TEST_H

#define EMBER_ENABLE_EM4

#define EMBER_AF_PLUGIN_IDLE_SLEEP_MINIMUM_SLEEP_DURATION_MS 0
#define EMBER_AF_PLUGIN_IDLE_SLEEP_BACKOFF_SLEEP_MS 0

#undef halPowerDown
#define halPowerDown(...)

#undef halPowerUp
#define halPowerUp(...)

#undef halSleepForMilliseconds
#define halSleepForMilliseconds(...)

#undef halBeforeEM4
#define halBeforeEM4(data1, data2)

#endif //SILABS_EM4_TEST_H
