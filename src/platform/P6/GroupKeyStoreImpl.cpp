/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides an implementation of the Chip GroupKeyStore interface
 *          for the PSoC6 platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/P6/GroupKeyStoreImpl.h>

using namespace ::chip;
using namespace ::chip::Profiles::Security::AppKeys;

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR GroupKeyStoreImpl::RetrieveGroupKey(uint32_t keyId, ChipGroupKey & key)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::StoreGroupKey(const ChipGroupKey & key)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::DeleteGroupKey(uint32_t keyId)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::DeleteGroupKeysOfAType(uint32_t keyType)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::EnumerateGroupKeys(uint32_t keyType, uint32_t * keyIds, uint8_t keyIdsArraySize, uint8_t & keyCount)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::Clear(void)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::RetrieveLastUsedEpochKeyId(void)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::StoreLastUsedEpochKeyId(void)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::EncodeGroupKey(const ChipGroupKey & key, uint8_t * buf, size_t bufSize, size_t & encodedKeyLen)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::DecodeGroupKeyId(const uint8_t * encodedKey, size_t encodedKeyLen, uint32_t & keyId)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::DecodeGroupKey(const uint8_t * encodedKey, size_t encodedKeyLen, ChipGroupKey & key)
{
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
