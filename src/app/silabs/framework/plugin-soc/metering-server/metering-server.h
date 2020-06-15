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
 * @brief Definitions for the Metering Server plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef METERING_SERVER_H_
#define METERING_SERVER_H_

/** @brief Sets the fast polling mode.
 *
 * Enables and disables fast polling mode.
 *
 * @param enableFastPolling False: disable, True: enable.
 */
void emberAfPluginMeteringServerEnableFastPolling(bool enableFastPolling);

#endif /* METERING_SERVER_H_ */
