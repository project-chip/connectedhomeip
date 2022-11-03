/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

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
