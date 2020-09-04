/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *    @file
 *      This file implements interfaces for the default group key store.
 *
 */

#include <core/CHIPCore.h>
#include "CHIPDummyGroupKeyStore.h"

namespace chip {
namespace Protocols {
namespace Security {
namespace AppKeys {

DummyGroupKeyStore::DummyGroupKeyStore(void)
{
    Init();
}

CHIP_ERROR DummyGroupKeyStore::RetrieveGroupKey(uint32_t keyId, ChipGroupKey& key)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DummyGroupKeyStore::StoreGroupKey(const ChipGroupKey& key)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DummyGroupKeyStore::DeleteGroupKey(uint32_t keyId)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DummyGroupKeyStore::DeleteGroupKeysOfAType(uint32_t keyType)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DummyGroupKeyStore::EnumerateGroupKeys(uint32_t keyType, uint32_t *keyIds, uint8_t keyIdsArraySize, uint8_t & keyCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DummyGroupKeyStore::Clear(void)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DummyGroupKeyStore::RetrieveLastUsedEpochKeyId(void)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DummyGroupKeyStore::StoreLastUsedEpochKeyId(void)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DummyGroupKeyStore::GetCurrentUTCTime(uint32_t& utcTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace AppKeys
} // namespace Security
} // namespace Protocols
} // namespace chip
