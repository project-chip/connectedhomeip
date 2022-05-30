/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *          Provides the implementation of the FailSafeContext object.
 */

#include <platform/FailSafeContext.h>

#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>
#include <platform/ConfigurationManager.h>

namespace chip {
namespace DeviceLayer {

namespace {
constexpr TLV::Tag kFabricIndexTag      = TLV::ContextTag(0);
constexpr TLV::Tag kAddNocCommandTag    = TLV::ContextTag(1);
constexpr TLV::Tag kUpdateNocCommandTag = TLV::ContextTag(2);
} // anonymous namespace

void FailSafeContext::HandleArmFailSafeTimer(System::Layer * layer, void * aAppState)
{
    FailSafeContext * context = reinterpret_cast<FailSafeContext *>(aAppState);
    context->FailSafeTimerExpired();
}

void FailSafeContext::HandleDisarmFailSafe(intptr_t arg)
{
    FailSafeContext * this_ = reinterpret_cast<FailSafeContext *>(arg);

    this_->mFailSafeBusy = false;

    if (ConfigurationMgr().SetFailSafeArmed(false) != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to set FailSafeArmed config to false");
    }

    if (DeleteFromStorage() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to delete FailSafeContext from configuration");
    }
}

void FailSafeContext::FailSafeTimerExpired()
{
    ScheduleFailSafeCleanup(mFabricIndex, mAddNocCommandHasBeenInvoked, mUpdateNocCommandHasBeenInvoked);
}

void FailSafeContext::ScheduleFailSafeCleanup(FabricIndex fabricIndex, bool addNocCommandInvoked, bool updateNocCommandInvoked)
{
    ResetState();

    mFailSafeBusy = true;

    ChipDeviceEvent event;
    event.Type                                                = DeviceEventType::kFailSafeTimerExpired;
    event.FailSafeTimerExpired.fabricIndex                    = fabricIndex;
    event.FailSafeTimerExpired.addNocCommandHasBeenInvoked    = addNocCommandInvoked;
    event.FailSafeTimerExpired.updateNocCommandHasBeenInvoked = updateNocCommandInvoked;
    CHIP_ERROR status                                         = PlatformMgr().PostEvent(&event);

    if (status != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to post fail-safe timer expired: %" CHIP_ERROR_FORMAT, status.Format());
    }

    PlatformMgr().ScheduleWork(HandleDisarmFailSafe, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR FailSafeContext::ArmFailSafe(FabricIndex accessingFabricIndex, System::Clock::Timeout expiryLength)
{
    mFailSafeArmed = true;
    mFabricIndex   = accessingFabricIndex;

    ReturnErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(expiryLength, HandleArmFailSafeTimer, this));
    ReturnErrorOnFailure(CommitToStorage());
    ReturnErrorOnFailure(ConfigurationMgr().SetFailSafeArmed(true));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FailSafeContext::DisarmFailSafe()
{
    ResetState();

    DeviceLayer::SystemLayer().CancelTimer(HandleArmFailSafeTimer, this);

    ReturnErrorOnFailure(ConfigurationMgr().SetFailSafeArmed(false));
    ReturnErrorOnFailure(DeleteFromStorage());

    return CHIP_NO_ERROR;
}

CHIP_ERROR FailSafeContext::SetAddNocCommandInvoked(FabricIndex nocFabricIndex)
{
    mAddNocCommandHasBeenInvoked = true;
    mFabricIndex                 = nocFabricIndex;

    ReturnErrorOnFailure(CommitToStorage());

    return CHIP_NO_ERROR;
}

CHIP_ERROR FailSafeContext::SetUpdateNocCommandInvoked()
{
    mUpdateNocCommandHasBeenInvoked = true;

    ReturnErrorOnFailure(CommitToStorage());

    return CHIP_NO_ERROR;
}

CHIP_ERROR FailSafeContext::CommitToStorage()
{
    DefaultStorageKeyAllocator keyAlloc;
    uint8_t buf[FailSafeContextTLVMaxSize()];
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(kFabricIndexTag, mFabricIndex));

    // TODO: Stop storing this, and just make the fail-safe context volatile and sweep-up stale data next boot on partial commits
    ReturnErrorOnFailure(writer.Put(kAddNocCommandTag, mAddNocCommandHasBeenInvoked));
    ReturnErrorOnFailure(writer.Put(kUpdateNocCommandTag, mUpdateNocCommandHasBeenInvoked));
    ReturnErrorOnFailure(writer.EndContainer(outerType));

    const auto failSafeContextTLVLength = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(failSafeContextTLVLength), CHIP_ERROR_BUFFER_TOO_SMALL);

    return PersistedStorage::KeyValueStoreMgr().Put(keyAlloc.FailSafeContextKey(), buf,
                                                    static_cast<uint16_t>(failSafeContextTLVLength));
}

CHIP_ERROR FailSafeContext::LoadFromStorage(FabricIndex & fabricIndex, bool & addNocCommandInvoked, bool & updateNocCommandInvoked)
{
    DefaultStorageKeyAllocator keyAlloc;
    uint8_t buf[FailSafeContextTLVMaxSize()];
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Get(keyAlloc.FailSafeContextKey(), buf, sizeof(buf)));

    TLV::ContiguousBufferTLVReader reader;
    reader.Init(buf, sizeof(buf));
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));

    ReturnErrorOnFailure(reader.Next(kFabricIndexTag));
    ReturnErrorOnFailure(reader.Get(fabricIndex));

    ReturnErrorOnFailure(reader.Next(kAddNocCommandTag));
    ReturnErrorOnFailure(reader.Get(addNocCommandInvoked));

    ReturnErrorOnFailure(reader.Next(kUpdateNocCommandTag));
    ReturnErrorOnFailure(reader.Get(updateNocCommandInvoked));

    ReturnErrorOnFailure(reader.VerifyEndOfContainer());
    ReturnErrorOnFailure(reader.ExitContainer(containerType));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FailSafeContext::DeleteFromStorage()
{
    DefaultStorageKeyAllocator keyAlloc;

    return PersistedStorage::KeyValueStoreMgr().Delete(keyAlloc.FailSafeContextKey());
}

void FailSafeContext::ForceFailSafeTimerExpiry()
{
    if (!IsFailSafeArmed())
    {
        return;
    }
    DeviceLayer::SystemLayer().CancelTimer(HandleArmFailSafeTimer, this);
    FailSafeTimerExpired();
}

} // namespace DeviceLayer
} // namespace chip
