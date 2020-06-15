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
 * @brief Tokens for the Illuminance Measurement Server plugin.
 *******************************************************************************
   ******************************************************************************/

/**
 * Custom Application Tokens
 */
#define CREATOR_SI1141_MULTIPLIER  (0x000F)
#define NVM3KEY_SI1141_MULTIPLIER  (NVM3KEY_DOMAIN_USER | 0x000F)

#ifdef DEFINETOKENS
DEFINE_BASIC_TOKEN(SI1141_MULTIPLIER, uint8_t, 0)
#endif //DEFINETOKENS
