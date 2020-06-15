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
 * @brief Tokens for the Manufacturing Library CLI plugin.
 *******************************************************************************
   ******************************************************************************/

/**
 * Custom Application Tokens
 */
#define CREATOR_MFG_LIB_ENABLED  (0x000d)
#define NVM3KEY_MFG_LIB_ENABLED  (NVM3KEY_DOMAIN_USER | 0x000d)

#ifdef DEFINETYPES
// Include or define any typedef for tokens here
#endif //DEFINETYPES
#ifdef DEFINETOKENS
// Define the actual token storage information here

DEFINE_BASIC_TOKEN(MFG_LIB_ENABLED, uint8_t, 0)

#endif //DEFINETOKENS
