/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 * @file
 *
 * @brief
 *   API function declarations for reading from and writing to persistent storage.
 *   Platforms will be responsible for implementing the read/write details.
 */

#ifndef PERSISTED_STORAGE_H
#define PERSISTED_STORAGE_H

#include <Weave/Core/WeaveError.h>
#include <Weave/Core/WeaveConfig.h>

namespace nl {
namespace Weave {
namespace Platform {
namespace PersistedStorage {

typedef WEAVE_CONFIG_PERSISTED_STORAGE_KEY_TYPE Key;

/**
 *  @brief
 *    Read integer value of a key from persistent storage.
 *    Platform is responsible for validating aKey.
 *
 *  @param[in]    aKey      A key to a persistently-stored value.
 *  @param[inout] aValue    A reference to an integer value.
 *
 *  @return WEAVE_ERROR_INVALID_ARGUMENT if aKey is NULL
 *          WEAVE_ERROR_INVALID_STRING_LENGTH if aKey exceeds
 *                  WEAVE_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH
 *          WEAVE_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if aKey does not exist
 *          WEAVE_NO_ERROR otherwise
 */
WEAVE_ERROR Read(Key aKey, uint32_t &aValue);

/**
 *  @brief
 *    Write the integer value of a key to persistent storage.
 *    Platform is responsible for validating aKey.
 *    If aKey does not exist, it will be created and assigned aValue.
 *    Otherwise any existing value of aKey will be replaced with aValue.
 *
 *  @param[in] aKey      A key to a persistently-stored value.
 *  @param[in] aValue    The value.
 *
 *  @return WEAVE_ERROR_INVALID_ARGUMENT if aKey is NULL
 *          WEAVE_ERROR_INVALID_STRING_LENGTH if aKey exceeds
 *                  WEAVE_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH
 *          WEAVE_NO_ERROR otherwise
 */
WEAVE_ERROR Write(Key aKey, uint32_t aValue);

} // PersistedStorage
} // Platform
} // Weave
} // nl

#endif // PERSISTED_STORAGE_H
