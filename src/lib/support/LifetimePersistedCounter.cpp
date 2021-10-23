/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "LifetimePersistedCounter.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PersistedStorage.h>

#include <stdlib.h>
#include <string.h>

namespace chip {

LifetimePersistedCounter::LifetimePersistedCounter() : mId(chip::Platform::PersistedStorage::kEmptyKey) {}

LifetimePersistedCounter::~LifetimePersistedCounter() {}

CHIP_ERROR
LifetimePersistedCounter::Init(const chip::Platform::PersistedStorage::Key aId)
{
    mId = aId;
    uint32_t startValue;

    // Read our previously-stored starting value.
    ReturnErrorOnFailure(ReadStartValue(startValue));

    // This will set the starting value, after which we're ready.
    return MonotonicallyIncreasingCounter::Init(startValue);
}

CHIP_ERROR
LifetimePersistedCounter::Advance()
{
    VerifyOrReturnError(mId != chip::Platform::PersistedStorage::kEmptyKey, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(MonotonicallyIncreasingCounter::Advance());

    return chip::Platform::PersistedStorage::Write(mId, GetValue());
}

CHIP_ERROR
LifetimePersistedCounter::ReadStartValue(uint32_t & aStartValue)
{
    aStartValue = 0;

    CHIP_ERROR err = chip::Platform::PersistedStorage::Read(mId, aStartValue);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // No previously-stored value, no worries, the counter is initialized to zero.
        // Suppress the error.
        err = CHIP_NO_ERROR;
    }
    return err;
}

} // namespace chip
