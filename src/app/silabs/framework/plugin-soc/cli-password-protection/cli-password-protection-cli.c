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
 * @brief Commands for the CLI password protection plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "af.h"
#include "app/framework/util/af-main.h"
#include "stack/include/security.h" // emberHmacAesHash
#include "hal/micro/token.h"

#if !defined(EMBER_AF_GENERATE_CLI) || !defined(ZA_CLI_FULL)
#error The password protection plugin is not compatible with legacy CLI.
#endif

EmberKeyData nullKey = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

void emberAfPluginCliPasswordProtectionChangeCommand(void)
{
  //we just set the token so that after the next reset, the new password is used
  uint32_t newPass = (uint32_t)emberUnsignedCommandArgument(0);
  if (newPass && newPass < 0xFFFFFFFF) {
    EmberKeyData passwordProtectionKey;
    halCommonGetToken(&passwordProtectionKey, TOKEN_PLUGIN_CLI_PASSWORD_KEY);
    if (0 != MEMCOMPARE((uint8_t*)passwordProtectionKey.contents, (uint8_t*)nullKey.contents, EMBER_ENCRYPTION_KEY_SIZE)) {
      uint8_t bytes[4];
      MEMCOPY(bytes, &newPass, 4);
      uint8_t result[16];
      emberHmacAesHash(passwordProtectionKey.contents, bytes, 4, result);
      halCommonSetToken(TOKEN_PLUGIN_CLI_PASSWORD, result);
      emberAfCorePrintln("Successfully set the password to %d", newPass);
    } else {
      emberAfCorePrintln("ERROR: password can't be changed. Initial password/key is not setup. ");
    }
  } else {
    emberAfCorePrintln("ERROR: Invalid password. A new password needs to be a number between 1 and 0xFFFFFFFE");
  }
}
