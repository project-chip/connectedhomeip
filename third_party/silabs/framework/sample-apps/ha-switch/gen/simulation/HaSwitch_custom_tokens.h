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
 * @brief
 *******************************************************************************
   ******************************************************************************/

// Copyright 2007 - 2011 by Ember Corporation. All rights reserved.
//
// This file contains customized application tokens

// Identifier tags for tokens
#define CREATOR_HALIGHT_NODE      0x1111
#define CREATOR_HALIGHT_ENDPOINT  0x1112

#define NVM3KEY_HALIGHT_NODE      (NVM3KEY_DOMAIN_USER | 0x1111)
#define NVM3KEY_HALIGHT_ENDPOINT  (NVM3KEY_DOMAIN_USER | 0x1112)

// Types for the tokens
#ifdef DEFINETYPES
#endif // DEFINETYPES

// Actual token definitions
#ifdef DEFINETOKENS
DEFINE_BASIC_TOKEN(HALIGHT_NODE, uint16_t, 0xFFFF)
DEFINE_BASIC_TOKEN(HALIGHT_ENDPOINT, uint8_t, 0xFF)
#endif // DEFINETOKENS
