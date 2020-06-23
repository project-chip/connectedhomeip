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
/***************************************************************************//**
 * @file
 * @brief This file defines the SOC specific client behavior for the Ember
 * proprietary bootloader protocol.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/plugin/standalone-bootloader-common/bootloader-protocol.h"
#include "standalone-bootloader-client.h"

#define MIN_BOOTLOADER_VERSION_WITH_EXTENDED_TYPE_API 0x4700

void emAfStandaloneBootloaderClientGetInfo(uint16_t* bootloaderVersion,
                                           uint8_t* platformId,
                                           uint8_t* microId,
                                           uint8_t* phyId)
{
  *bootloaderVersion = halGetStandaloneBootloaderVersion();

  *platformId = PLAT;
  *microId = MICRO;
  *phyId = PHY;
}

EmberStatus emAfStandaloneBootloaderClientLaunch(void)
{
  return halLaunchStandaloneBootloader(STANDALONE_BOOTLOADER_NORMAL_MODE);
}

void emAfStandaloneBootloaderClientGetMfgInfo(uint16_t* mfgIdReturnValue,
                                              uint8_t* boardNameReturnValue)
{
  halCommonGetToken(mfgIdReturnValue, TOKEN_MFG_MANUF_ID);
  halCommonGetToken(boardNameReturnValue, TOKEN_MFG_BOARD_NAME);
}

uint32_t emAfStandaloneBootloaderClientGetRandomNumber(void)
{
  return halStackGetInt32uSymbolTick();
}

#if !defined(EMBER_TEST)

#if !defined(SERIAL_UART_BTL)
  #error Wrong Bootloader specified for configuration.  Must specify a standalone bootloader.
#endif

bool emAfPluginStandaloneBootloaderClientCheckBootloader(void)
{
  BlExtendedType blExtendedType = halBootloaderGetInstalledType();

  if (blExtendedType != BL_EXT_TYPE_SERIAL_UART_OTA) {
    // Actual bootloader on-chip is wrong.
    bootloadPrintln("Error:  Loaded bootloader type 0x%X != required type 0x%X (serial-uart-ota)",
                    blExtendedType,
                    BL_EXT_TYPE_SERIAL_UART_OTA);
    return false;
  }

  return true;
}

void emAfStandaloneBootloaderClientGetKey(uint8_t* returnData)
{
  halCommonGetToken(returnData, TOKEN_MFG_BOOTLOAD_AES_KEY);
}

#else

bool emAfPluginStandaloneBootloaderClientCheckBootloader(void)
{
  // In order to test the high level app messages in simulation, we lie
  // and say the bootloader is the "correct" one.  We can't actually bootload
  // in simulation.
  return true;
}

#endif
