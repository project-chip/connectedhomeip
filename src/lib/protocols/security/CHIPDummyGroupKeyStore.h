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
 *      This file defines class, which is used as a default dummy (empty)
 *      implementation of the group key store.
 *
 */

#ifndef CHIPDUMMYGROUPKEYSTORE_H_
#define CHIPDUMMYGROUPKEYSTORE_H_

#include "CHIPApplicationKeys.h"

namespace chip {
namespace Protocols {
namespace Security {
namespace AppKeys {

/**
 *  @class DummyKeyStore
 *
 *  @brief
 *    The definition of the dummy group key store.
 *
 */
class DummyGroupKeyStore : public GroupKeyStoreBase
{
public:
    DummyGroupKeyStore();

    // Manage application group key material storage.
    CHIP_ERROR RetrieveGroupKey(uint32_t keyId, ChipGroupKey & key) override;
    CHIP_ERROR StoreGroupKey(const ChipGroupKey & key) override;
    CHIP_ERROR DeleteGroupKey(uint32_t keyId) override;
    CHIP_ERROR DeleteGroupKeysOfAType(uint32_t keyType) override;
    CHIP_ERROR EnumerateGroupKeys(uint32_t keyType, uint32_t * keyIds, uint8_t keyIdsArraySize, uint8_t & keyCount) override;
    CHIP_ERROR Clear() override;

private:
    // Retrieve and Store LastUsedEpochKeyId value.
    CHIP_ERROR RetrieveLastUsedEpochKeyId() override;
    CHIP_ERROR StoreLastUsedEpochKeyId() override;

    // Get current platform UTC time in seconds.
    CHIP_ERROR GetCurrentUTCTime(uint32_t & utcTime) override;
};

} // namespace AppKeys
} // namespace Security
} // namespace Protocols
} // namespace chip

#endif /* CHIPDUMMYGROUPKEYSTORE_H_ */
