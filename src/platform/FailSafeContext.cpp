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

#include <lib/support/SafeInt.h>
#include <platform/ConfigurationManager.h>

namespace chip {
namespace DeviceLayer {

namespace {
constexpr const char kFailSafeContextKey[] = "kFailSafeContextKey";

constexpr TLV::Tag kFabricIndexTag      = TLV::ContextTag(0);
constexpr TLV::Tag kAddNocCommandTag    = TLV::ContextTag(1);
constexpr TLV::Tag kUpdateNocCommandTag = TLV::ContextTag(2);
} // anonymous namespace

void FailSafeContext::HandleArmFailSafe(System::Layer * layer, void * aAppState)
{
    FailSafeContext * context = reinterpret_cast<FailSafeContext *>(aAppState);
    context->FailSafeTimerExpired();
}

void FailSafeContext::FailSafeTimerExpired()
{
    ChipDeviceEvent event;
    event.Type                                                = DeviceEventType::kFailSafeTimerExpired;
    event.FailSafeTimerExpired.PeerFabricIndex                = mFabricIndex;
    event.FailSafeTimerExpired.AddNocCommandHasBeenInvoked    = mAddNocCommandHasBeenInvoked;
    event.FailSafeTimerExpired.UpdateNocCommandHasBeenInvoked = mUpdateNocCommandHasBeenInvoked;
    CHIP_ERROR status                                         = PlatformMgr().PostEvent(&event);

    mFailSafeArmed                  = false;
    mAddNocCommandHasBeenInvoked    = false;
    mUpdateNocCommandHasBeenInvoked = false;
    ConfigurationMgr().SetFailSafeArmed(false);

    if (status != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to post commissioning complete: %" CHIP_ERROR_FORMAT, status.Format());
    }
}

CHIP_ERROR FailSafeContext::ArmFailSafe(FabricIndex accessingFabricIndex, System::Clock::Timeout expiryLength)
{
    mFailSafeArmed = true;
    mFabricIndex   = accessingFabricIndex;
    ConfigurationMgr().SetFailSafeArmed(true);
    DeviceLayer::SystemLayer().StartTimer(expiryLength, HandleArmFailSafe, this);

    if (CommitToStorage() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to commit the fabricIndex of FailSafeContext to persistent storage");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FailSafeContext::DisarmFailSafe()
{
    mFailSafeArmed                  = false;
    mAddNocCommandHasBeenInvoked    = false;
    mUpdateNocCommandHasBeenInvoked = false;
    ConfigurationMgr().SetFailSafeArmed(false);
    DeviceLayer::SystemLayer().CancelTimer(HandleArmFailSafe, this);

    if (DeleteFromStorage() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to delete the captured FailSafeContext from persistent storage");
    }

    return CHIP_NO_ERROR;
}

void FailSafeContext::SetAddNocCommandInvoked(FabricIndex nocFabricIndex)
{
    mAddNocCommandHasBeenInvoked = true;
    mFabricIndex                 = nocFabricIndex;

    if (CommitToStorage() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to commit the AddNocCommandHasBeenInvoked of FailSafeContext to persistent storage");
    }
}

void FailSafeContext::SetUpdateNocCommandInvoked(FabricIndex nocFabricIndex)
{
    mUpdateNocCommandHasBeenInvoked = true;
    mFabricIndex                    = nocFabricIndex;

    if (CommitToStorage() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to commit the UpdateNocCommandHasBeenInvoked of FailSafeContext to persistent storage");
    }
}

CHIP_ERROR FailSafeContext::CommitToStorage()
{
    uint8_t buf[FailSafeContextTLVMaxSize()];
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(kFabricIndexTag, mFabricIndex));
    ReturnErrorOnFailure(writer.Put(kAddNocCommandTag, mAddNocCommandHasBeenInvoked));
    ReturnErrorOnFailure(writer.Put(kUpdateNocCommandTag, mUpdateNocCommandHasBeenInvoked));
    ReturnErrorOnFailure(writer.EndContainer(outerType));

    const auto failSafeContextTLVLength = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(failSafeContextTLVLength), CHIP_ERROR_BUFFER_TOO_SMALL);

    return PersistedStorage::KeyValueStoreMgr().Put(kFailSafeContextKey, buf, static_cast<uint16_t>(failSafeContextTLVLength));
}

CHIP_ERROR FailSafeContext::LoadFromStorage(FabricIndex & fabricIndex, bool & addNocCommandInvoked, bool & updateNocCommandInvoked)
{
    uint8_t buf[FailSafeContextTLVMaxSize()];
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Get(kFailSafeContextKey, buf, sizeof(buf)));

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
    return PersistedStorage::KeyValueStoreMgr().Delete(kFailSafeContextKey);
}

} // namespace DeviceLayer
} // namespace chip
