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

#include <platform/PersistedStorage.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <stdlib.h>
#include <string.h>

namespace chip {

LifetimePersistedCounter::LifetimePersistedCounter() : mId(chip::Platform::PersistedStorage::kEmptyKey) {}

LifetimePersistedCounter::~LifetimePersistedCounter() {}

CHIP_ERROR
LifetimePersistedCounter::Init(const chip::Platform::PersistedStorage::Key aId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mId = aId;
    uint32_t startValue;

    // Read our previously-stored starting value.
    SuccessOrExit(err = ReadStartValue(startValue));

    // This will set the starting value, after which we're ready.
    SuccessOrExit(err = MonotonicallyIncreasingCounter::Init(startValue));

exit:
    return err;
}

CHIP_ERROR
LifetimePersistedCounter::Advance()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mId != chip::Platform::PersistedStorage::kEmptyKey, err = CHIP_ERROR_INCORRECT_STATE);

    SuccessOrExit(err = MonotonicallyIncreasingCounter::Advance());

    SuccessOrExit(err = chip::Platform::PersistedStorage::Write(mId, GetValue()));
exit:
    return err;
}

CHIP_ERROR
LifetimePersistedCounter::ReadStartValue(uint32_t & aStartValue)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    aStartValue = 0;

    err = chip::Platform::PersistedStorage::Read(mId, aStartValue);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // No previously-stored value, no worries, the counter is initialized to zero.
        // Suppress the error.
        err = CHIP_NO_ERROR;
    }
    else
    {
        SuccessOrExit(err);
    }
exit:
    return err;
}

} // namespace chip
