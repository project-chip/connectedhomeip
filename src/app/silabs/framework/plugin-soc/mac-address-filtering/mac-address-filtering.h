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
 * @brief MAC Address Filtering - Public APIs
 *******************************************************************************
   ******************************************************************************/

void emberAfPluginMacAddressFilteringInitCallback(void);

void emberAfPluginMacAddressFilteringInitCallback(void);

void emberAfPluginMacAddressFilteringPrintConfigCommand(void);

void emberAfPluginMacAddressFilteringFilterNoAddressCommand(void);

void emberAfPluginMacAddressFilteringAddShortAddressCommand(void);

void emberAfPluginMacAddressFilteringSetShortAddressListType(void);

void emberAfPluginMacAddressFilteringSetLongAddressListType(void);

void emberAfPluginMacAddressFilteringAddLongAddressCommand(void);

void emberAfPluginMacAddressFilteringClearShortAddressList(void);

void emberAfPluginMacAddressFilteringClearLongAddressList(void);

void emberAfPluginMacAddressFilteringAddPanIdCommand(void);

void emberAfPluginMacAddressFilteringPanIdDeleteEntry(void);

void emberAfPluginMacAddressFilteringClearPanIdList(void);

void emberAfPluginMacAddressFilteringSetPanIdListType(void);

void emberAfPluginMacAddressFilteringReset(void);

void emberAfPluginMacAddressFilteringClearAll(void);

void emberAfPluginMacAddressFilteringShortAddressDeleteEntry(void);

void emberAfPluginMacAddressFilteringLongAddressDeleteEntry(void);

EmberStatus emberAfPluginMacAddressFilteringGetStatsForShortAddress(uint8_t index,
                                                                    uint32_t* matchCountForIndexPtr);
void emberAfPluginMacAddressFilteringGetStats(boolean shortMode,
                                              uint32_t* allowedPacketCountPtr,
                                              uint32_t* droppedPacketCountPtr,
                                              uint32_t* totalPacketCountPtr);
