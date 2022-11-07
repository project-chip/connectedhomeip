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

#pragma once

#include <app/util/config.h>

#ifdef EMBER_AF_PLUGIN_SCENES_USE_TOKENS

#define CREATOR_SCENES_NUM_ENTRIES (0x8723)
#define NVM3KEY_SCENES_NUM_ENTRIES (NVM3KEY_DOMAIN_ZIGBEE | 0x8723)

#define CREATOR_SCENES_TABLE (0x8724)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_SCENES_TABLE (NVM3KEY_DOMAIN_ZIGBEE | 0x4080)

#ifdef DEFINETYPES
// Include or define any typedef for tokens here
#endif // DEFINETYPES
#ifdef DEFINETOKENS
// Define the actual token storage information here

DEFINE_BASIC_TOKEN(SCENES_NUM_ENTRIES, uint8_t, 0x00)
DEFINE_INDEXED_TOKEN(SCENES_TABLE, EmberAfSceneTableEntry, MATTER_SCENES_TABLE_SIZE, { EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID })
#endif // DEFINETOKENS

#endif // EMBER_AF_PLUGIN_SCENES_USE_TOKENS
