/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
/***************************************************************************/
/**
 * @file
 * @brief APIs and defines for the Level Control plugin, which implements the
 *        Level Control cluster.
 ******************************************************************************/
 
// Rate of level control tick execution.
// To increase tick frequency (for more granular updates of device state based
// on level), redefine EMBER_AF_PLUGIN_LEVEL_CONTROL_TICKS_PER_SECOND.
#ifndef EMBER_AF_PLUGIN_LEVEL_CONTROL_TICKS_PER_SECOND
  #define EMBER_AF_PLUGIN_LEVEL_CONTROL_TICKS_PER_SECOND 32
#endif
#define EMBER_AF_PLUGIN_LEVEL_CONTROL_TICK_TIME \
  (MILLISECOND_TICKS_PER_SECOND / EMBER_AF_PLUGIN_LEVEL_CONTROL_TICKS_PER_SECOND)
