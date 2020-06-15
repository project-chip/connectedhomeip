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
 * @brief This file is used as an example of custom token.
 *******************************************************************************
   ******************************************************************************/

// Identifier tags for tokens
// Creator for attribute: CUSTOM
#define CREATOR_CUSTOM_TOKEN 0xDEED
// NVM3 key for attribute: CUSTOM (in zigbee key domain)
#define NVM3KEY_CUSTOM_TOKEN (NVM3KEY_DOMAIN_ZIGBEE | 0xDEED)

// Types for the tokens
#ifdef DEFINETYPES
typedef uint8_t  tokType_custom_token;
#endif // DEFINETYPES

// Actual token definitions
#ifdef DEFINETOKENS
DEFINE_BASIC_TOKEN(CUSTOM_TOKEN, tokType_custom_token, 0x42)
#endif // DEFINETOKENS
