/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 * @brief Defines a table of fabrics that have provisioned the device.
 */

#pragma once

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/Optional.h>
#include <lib/core/TLV.h>
#include <system/SystemClock.h>

namespace chip {

class LastKnownGoodTime
{
public:
    LastKnownGoodTime() {}

    /**
     * Initialize Last Known Good Time to the later of firmware build time or
     * last known good time persisted in storage.  Persist the selected value
     * if this differs from the value in storage or no value is yet persisted.
     *
     * @param storage storage delegate to persist Last Known Good Time
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP_ERROR
     */
    CHIP_ERROR Init(PersistentStorageDelegate * storage);

    /**
     * Get the current Last Known Good Time.
     *
     * @param lastKnownGoodChipEpochTime (out) the current last known good time, if any is known
     * &return CHIP_NO_ERROR on success, else an appropriate CHIP_ERROR
     */
    CHIP_ERROR GetLastKnownGoodChipEpochTime(System::Clock::Seconds32 & lastKnownGoodChipEpochTime) const
    {
        VerifyOrReturnError(mLastKnownGoodChipEpochTime.HasValue(), CHIP_ERROR_INCORRECT_STATE);
        lastKnownGoodChipEpochTime = mLastKnownGoodChipEpochTime.Value();
        return CHIP_NO_ERROR;
    }

    /**
     * Validate that the passed Last Known Good Time is within bounds and then
     * store this and write back to storage.  Legal values are those which are
     * not earlier than firmware build time or any of our stored certificates'
     * NotBefore times:
     *
     *    3.5.6.1. Last Known Good UTC Time
     *
     *    A Node MAY adjust the Last Known Good UTC Time backwards if it
     *    believes the current Last Known Good UTC Time is incorrect and it has
     *    a good time value from a trusted source. The Node SHOULD NOT adjust
     *    the Last Known Good UTC to a time before the later of:
     *      • The build timestamp of its currently running software image
     *      • The not-before timestamp of any of its operational certificates
     *
     * @param lastKnownGoodChipEpochTime Last Known Good Time in seconds since CHIP epoch
     * @param notBefore the latest NotBefore time of all installed certificates
     * @return CHIP_NO_ERROR on success, else an appopriate CHIP_ERROR
     */
    CHIP_ERROR SetLastKnownGoodChipEpochTime(System::Clock::Seconds32 lastKnownGoodChipEpochTime,
                                             System::Clock::Seconds32 notBefore);

    /**
     * Update the Last Known Good Time to the later of the current value and
     * the passed value and store in RAM.  This does not persist the value.
     * Persist only happens if CommitPendingLastKnownGoodChipEpochTime is
     * called.
     *
     * @param lastKnownGoodChipEpochTime Last Known Good Time in seconds since CHIP epoch
     * @return CHIP_NO_ERROR on success, else an appopriate CHIP_ERROR
     */
    CHIP_ERROR UpdatePendingLastKnownGoodChipEpochTime(System::Clock::Seconds32 lastKnownGoodChipEpochTime);

    /*
     * Commit the pending Last Known Good Time in RAM to storage.
     *
     * @return CHIP_NO_ERROR on success, else an appopriate CHIP_ERROR
     */
    CHIP_ERROR CommitPendingLastKnownGoodChipEpochTime();

    /*
     * Revert the Last Known Good Time to the fail-safe backup value in
     * persistence if any exists.
     *
     * @return CHIP_NO_ERROR on success, else an appopriate CHIP_ERROR
     */
    CHIP_ERROR RevertPendingLastKnownGoodChipEpochTime();

private:
    static constexpr size_t LastKnownGoodTimeTLVMaxSize()
    {
        // We have Last Known Good Time and, optionally, a previous Last known
        // good time for fail safe cleanup.
        return TLV::EstimateStructOverhead(sizeof(uint32_t), sizeof(uint32_t));
    }

    /**
     * Log the message, appending the passed CHIP epoch time in ISO8601 format.
     *
     * @param msg message to log with ISO8601 time appended
     * @param chipEpochTime time in seconds from the CHIP epoch
     */
    void LogTime(const char * msg, System::Clock::Seconds32 chipEpochTime);

    /**
     * Load the Last Known Good Time from storage.
     *
     * @param lastKnownGoodChipEpochTime (out) Last Known Good Time as seconds from CHIP epoch
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP_ERROR
     */
    CHIP_ERROR LoadLastKnownGoodChipEpochTime(System::Clock::Seconds32 & lastKnownGoodChipEpochTime) const;

    /**
     * Store the Last Known Good Time to storage.
     *
     * @param lastKnownGoodChipEpochTime Last Known Good Time as seconds from CHIP epoch
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP_ERROR
     */
    CHIP_ERROR StoreLastKnownGoodChipEpochTime(System::Clock::Seconds32 lastKnownGoodChipEpochTime) const;

    PersistentStorageDelegate * mStorage = nullptr;
    Optional<System::Clock::Seconds32> mLastKnownGoodChipEpochTime;
};

} // namespace chip
