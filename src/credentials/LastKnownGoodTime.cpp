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

#include "LastKnownGoodTime.h"

#include <lib/support/BufferWriter.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>
#include <platform/ConfigurationManager.h>

namespace chip {

namespace {
// Tags for Last Known Good Time.
constexpr TLV::Tag kLastKnownGoodChipEpochSecondsTag         = TLV::ContextTag(0);
constexpr TLV::Tag kFailSafeLastKnownGoodChipEpochSecondsTag = TLV::ContextTag(1);
} // anonymous namespace

void LastKnownGoodTime::LogTime(const char * msg, System::Clock::Seconds32 chipEpochTime)
{
    char buf[26] = { 0 }; // strlen("00000-000-000T000:000:000") == 25
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    ChipEpochToCalendarTime(chipEpochTime.count(), year, month, day, hour, minute, second);
    snprintf(buf, sizeof(buf), "%04u-%02u-%02uT%02u:%02u:%02u", year, month, day, hour, minute, second);
    ChipLogProgress(TimeService, "%s%s", msg, buf);
}

CHIP_ERROR LastKnownGoodTime::LoadLastKnownGoodChipEpochTime(System::Clock::Seconds32 & lastKnownGoodChipEpochTime,
                                                             Optional<System::Clock::Seconds32> & failSafeBackup) const
{
    uint8_t buf[LastKnownGoodTimeTLVMaxSize()];
    uint16_t size = sizeof(buf);
    uint32_t seconds;
    DefaultStorageKeyAllocator keyAlloc;
    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(keyAlloc.LastKnownGoodTimeKey(), buf, size));
    TLV::ContiguousBufferTLVReader reader;
    reader.Init(buf, size);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(kLastKnownGoodChipEpochSecondsTag));
    ReturnErrorOnFailure(reader.Get(seconds));
    lastKnownGoodChipEpochTime = System::Clock::Seconds32(seconds);
    CHIP_ERROR err             = reader.Next();
    if (err == CHIP_END_OF_TLV)
    {
        failSafeBackup = NullOptional;
        return CHIP_NO_ERROR; // not an error; this tag is optional
    }
    VerifyOrReturnError(reader.GetTag() == kFailSafeLastKnownGoodChipEpochSecondsTag, CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
    ReturnErrorOnFailure(reader.Get(seconds));
    failSafeBackup.Emplace(seconds);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LastKnownGoodTime::LoadLastKnownGoodChipEpochTime(System::Clock::Seconds32 & lastKnownGoodChipEpochTime) const
{
    Optional<System::Clock::Seconds32> failSafeBackup;
    return LoadLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime, failSafeBackup);
}

CHIP_ERROR LastKnownGoodTime::StoreLastKnownGoodChipEpochTime(System::Clock::Seconds32 lastKnownGoodChipEpochTime,
                                                              const Optional<System::Clock::Seconds32> & failSafeBackup) const
{
    uint8_t buf[LastKnownGoodTimeTLVMaxSize()];
    TLV::TLVWriter writer;
    writer.Init(buf);
    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(kLastKnownGoodChipEpochSecondsTag, lastKnownGoodChipEpochTime.count()));
    if (failSafeBackup.HasValue())
    {
        ReturnErrorOnFailure(writer.Put(kFailSafeLastKnownGoodChipEpochSecondsTag, failSafeBackup.Value().count()));
    }
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    const auto length = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(length), CHIP_ERROR_BUFFER_TOO_SMALL);
    DefaultStorageKeyAllocator keyAlloc;
    ReturnErrorOnFailure(mStorage->SyncSetKeyValue(keyAlloc.LastKnownGoodTimeKey(), buf, static_cast<uint16_t>(length)));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LastKnownGoodTime::StoreLastKnownGoodChipEpochTime(System::Clock::Seconds32 lastKnownGoodChipEpochTime) const
{
    return StoreLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime, NullOptional);
}

CHIP_ERROR LastKnownGoodTime::Init(PersistentStorageDelegate * storage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mStorage       = storage;
    // 3.5.6.1 Last Known Good UTC Time:
    //
    // "A Node’s initial out-of-box Last Known Good UTC time SHALL be the
    // compile-time of the firmware."
    System::Clock::Seconds32 buildTime;
    SuccessOrExit(err = DeviceLayer::ConfigurationMgr().GetFirmwareBuildChipEpochTime(buildTime));
    System::Clock::Seconds32 storedLastKnownGoodChipEpochTime;
    err = LoadLastKnownGoodChipEpochTime(storedLastKnownGoodChipEpochTime);
    VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, ;);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogProgress(TimeService, "Last Known Good Time: [unknown]");
    }
    else
    {
        LogTime("Last Known Good Time: ", storedLastKnownGoodChipEpochTime);
    }
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND || buildTime > storedLastKnownGoodChipEpochTime)
    {
        // If we have no value in persistence, or the firmware build time is
        // later than the value in persistence, set last known good time to the
        // firmware build time and write back.
        LogTime("Setting Last Known Good Time to firmware build time ", buildTime);
        mLastKnownGoodChipEpochTime.SetValue(buildTime);
        SuccessOrExit(err = StoreLastKnownGoodChipEpochTime(buildTime));
    }
    else
    {
        mLastKnownGoodChipEpochTime.SetValue(storedLastKnownGoodChipEpochTime);
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(TimeService, "Failed to init Last Known Good Time: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

CHIP_ERROR LastKnownGoodTime::SetLastKnownGoodChipEpochTime(System::Clock::Seconds32 lastKnownGoodChipEpochTime,
                                                            System::Clock::Seconds32 notBefore)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mLastKnownGoodChipEpochTime.HasValue(), err = CHIP_ERROR_INCORRECT_STATE);
    LogTime("Last Known Good Time: ", mLastKnownGoodChipEpochTime.Value());
    LogTime("New proposed Last Known Good Time: ", lastKnownGoodChipEpochTime);

    // Verify that the passed value is not earlier than the firmware build time.
    System::Clock::Seconds32 buildTime;
    SuccessOrExit(err = DeviceLayer::ConfigurationMgr().GetFirmwareBuildChipEpochTime(buildTime));
    VerifyOrExit(lastKnownGoodChipEpochTime >= buildTime, err = CHIP_ERROR_INVALID_ARGUMENT);
    // Verify that the passed value is not earlier than the passed NotBefore time.
    VerifyOrExit(lastKnownGoodChipEpochTime >= notBefore, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Passed value is valid.  Capture it and write back to persistence.
    //
    // Note that we are purposefully overwriting any previous last known
    // good time that may have been stored as part of a fail-safe context.
    // This is intentional: we don't promise not to change last known good
    // time during the fail-safe timer.  For instance, if the platform has a
    // new, better time source, it is legal to capture it.  If we do, we should
    // both overwrite last known good time and discard the previous value stored
    // for fail safe recovery, as together these comprise a transaction.  By
    // overwriting both, we are fully superseding that transaction with our
    // own, which does not to have a revert feature.
    SuccessOrExit(err = StoreLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime));
    mLastKnownGoodChipEpochTime.SetValue(lastKnownGoodChipEpochTime);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(TimeService, "Failed to update Last Known Good Time: %" CHIP_ERROR_FORMAT, err.Format());
    }
    else
    {
        LogTime("Updating Last Known Good Time to ", lastKnownGoodChipEpochTime);
    }
    return err;
}

CHIP_ERROR LastKnownGoodTime::UpdateLastKnownGoodChipEpochTime(System::Clock::Seconds32 lastKnownGoodChipEpochTime)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mLastKnownGoodChipEpochTime.HasValue(), err = CHIP_ERROR_INCORRECT_STATE);
    LogTime("Last Known Good Time: ", mLastKnownGoodChipEpochTime.Value());
    LogTime("New proposed Last Known Good Time: ", lastKnownGoodChipEpochTime);
    if (lastKnownGoodChipEpochTime > mLastKnownGoodChipEpochTime.Value())
    {
        LogTime("Current Last Known Good time retained in fail-safe context, updating to ", lastKnownGoodChipEpochTime);
        // We have a later timestamp.  Advance last known good time and store
        // the fail-safe value.
        SuccessOrExit(
            err = StoreLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime, MakeOptional(mLastKnownGoodChipEpochTime.Value())));
        mLastKnownGoodChipEpochTime.SetValue(lastKnownGoodChipEpochTime);
    }
    else
    {
        ChipLogProgress(TimeService, "Retaining current Last Known Good Time");
        // Our timestamp is not later.  Retain the existing last known good time
        // and discard any fail-safe value in persistence.
        SuccessOrExit(err = StoreLastKnownGoodChipEpochTime(mLastKnownGoodChipEpochTime.Value()));
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(TimeService, "Failed to persist Last Known Good Time: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

CHIP_ERROR LastKnownGoodTime::CommitLastKnownGoodChipEpochTime()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mLastKnownGoodChipEpochTime.HasValue(), err = CHIP_ERROR_INCORRECT_STATE);
    LogTime("Committing Last Known Good Time to storage: ", mLastKnownGoodChipEpochTime.Value());
    // Writing with no fail-safe backup removes the fail-safe backup from
    // storage, thus committing the new Last Known Good Time.
    SuccessOrExit(err = StoreLastKnownGoodChipEpochTime(mLastKnownGoodChipEpochTime.Value()));
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(TimeService, "Failed to commit Last Known Good Time: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

CHIP_ERROR LastKnownGoodTime::RevertLastKnownGoodChipEpochTime()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::Clock::Seconds32 lastKnownGoodChipEpochTime;
    Optional<System::Clock::Seconds32> failSafeBackup;
    VerifyOrExit(mLastKnownGoodChipEpochTime.HasValue(), err = CHIP_ERROR_INCORRECT_STATE);
    LogTime("Last Known Good Time: ", mLastKnownGoodChipEpochTime.Value());
    SuccessOrExit(err = LoadLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime, failSafeBackup));
    if (!failSafeBackup.HasValue())
    {
        ChipLogProgress(TimeService, "No fail safe Last Known Good Time to revert to");
        return CHIP_NO_ERROR; // if there's no value to revert to, we are done
    }
    LogTime("Fail safe Last Known Good Time: ", failSafeBackup.Value());
    SuccessOrExit(err = StoreLastKnownGoodChipEpochTime(failSafeBackup.Value()));
    mLastKnownGoodChipEpochTime.SetValue(failSafeBackup.Value());
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(TimeService, "Failed to persist Last Known Good Time: %" CHIP_ERROR_FORMAT, err.Format());
    }
    else
    {
        ChipLogProgress(TimeService, "Reverted Last Known Good Time to fail safe value");
    }
    return err;
}

} // namespace chip
