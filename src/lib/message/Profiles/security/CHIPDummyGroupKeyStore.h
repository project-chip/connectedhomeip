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
namespace Profiles {
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
    DummyGroupKeyStore(void);

    // Manage application group key material storage.
    virtual CHIP_ERROR RetrieveGroupKey(uint32_t keyId, ChipGroupKey & key);
    virtual CHIP_ERROR StoreGroupKey(const ChipGroupKey & key);
    virtual CHIP_ERROR DeleteGroupKey(uint32_t keyId);
    virtual CHIP_ERROR DeleteGroupKeysOfAType(uint32_t keyType);
    virtual CHIP_ERROR EnumerateGroupKeys(uint32_t keyType, uint32_t * keyIds, uint8_t keyIdsArraySize, uint8_t & keyCount);
    virtual CHIP_ERROR Clear(void);

private:
    // Retrieve and Store LastUsedEpochKeyId value.
    virtual CHIP_ERROR RetrieveLastUsedEpochKeyId(void);
    virtual CHIP_ERROR StoreLastUsedEpochKeyId(void);

    // Get current platform UTC time in seconds.
    virtual CHIP_ERROR GetCurrentUTCTime(uint32_t & utcTime);
};

} // namespace AppKeys
} // namespace Security
} // namespace Profiles
} // namespace chip

#endif /* CHIPDUMMYGROUPKEYSTORE_H_ */
