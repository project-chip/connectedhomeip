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
 * @brief Tokens for the Bulb UI plugin.
 *******************************************************************************
   ******************************************************************************/

#define CREATOR_REBOOT_MONITOR  (0x000c)
#define NVM3KEY_REBOOT_MONITOR  (NVM3KEY_DOMAIN_USER | 0x000c)

#define CREATOR_SHORT_REBOOT_MONITOR (0x000e)
#define NVM3KEY_SHORT_REBOOT_MONITOR (NVM3KEY_DOMAIN_USER | 0x000e)

#ifdef DEFINETYPES
// Include or define any typedef for tokens here
#endif //DEFINETYPES
#ifdef DEFINETOKENS
// Define the actual token storage information here

DEFINE_COUNTER_TOKEN(REBOOT_MONITOR, tokTypeStackBootCounter, 0)
DEFINE_COUNTER_TOKEN(SHORT_REBOOT_MONITOR, tokTypeStackBootCounter, 0)

#endif //DEFINETOKENS
