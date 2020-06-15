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
 * @brief Definitions for the WWAH Client Silabs plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef _WWAH_CLIENT_SILABS_H_
#define _WWAH_CLIENT_SILABS_H_

void emberAfSlWwahClusterClientInitCallback(uint8_t endpoint);
bool emberAfSlWwahClusterDebugReportQueryResponseCallback(uint8_t debugReportId,
                                                          uint8_t* debugReportData);

#endif  // #ifndef _WWAH_CLIENT_SILABS_H_
