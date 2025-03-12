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
#include "FailSafeContext.h"
#include <app/icd/server/ICDServerConfig.h>
#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/server/ICDNotifier.h> // nogncheck
#endif
#include <lib/core/TLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip::DeviceLayer;

namespace chip {
namespace app {

namespace {

// Tags for AddNOCStartedMarker storage
constexpr TLV::Tag kAddNOCStartedMarkerFabricIndexTag = TLV::ContextTag(0);

constexpr size_t AddNOCStartedMarkerContextTLVMaxSize()
{
    // Add 2x uncommitted uint64_t to leave space for backwards/forwards
    // versioning for this critical feature that runs at boot.
    return TLV::EstimateStructOverhead(sizeof(FabricIndex), sizeof(uint64_t), sizeof(uint64_t));
}

} // namespace

CHIP_ERROR FailSafeContext::Init(const InitParams & initParams)
{
    VerifyOrReturnError(initParams.storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mStorage = initParams.storage;

    return CHIP_NO_ERROR;
}

void FailSafeContext::CheckAddNOCStartedMarker()
{
    AddNOCStartedMarker marker;

    CHIP_ERROR err = GetAddNOCStartedMarker(marker);

    if (err == CHIP_NO_ERROR)
    {
        // This should not be possible at this point
        if (IsFailSafeArmed())
        {
            ChipLogError(FailSafe, "Found a AddNOCStartedMarker, but Fail-Safe is armed. Something went wrong.");
            return;
        }

        // Fail-Safe may be busy due to cleanup scheduled by failed commit to FabricTable.
        // We can ignore it here, AddNOCStartedMarker will be deleted when Fail-Safe is disarmed.
        if (IsFailSafeBusy())
        {
            return;
        }

        // Found a marker! We need to trigger a cleanup.
        ChipLogError(FailSafe, "Found a Fail-Safe marker for index 0x%x, preparing cleanup!",
                     static_cast<unsigned>(marker.fabricIndex));

        // Fake arm Fail-Safe and trigger timer expiry.
        // We handle only the case when new fabric is added. FabricTable CommitMarker
        // is responsible for guarding the case of updating the existing fabric.
        SetFailSafeArmed(true);
        mFabricIndex                 = marker.fabricIndex;
        mAddNocCommandHasBeenInvoked = true;
        ForceFailSafeTimerExpiry();
    }
    else if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // Got an error, but somehow value is not missing altogether: inconsistent state but touch nothing.
        ChipLogError(FailSafe, "Error loading Fail-Safe marker: %" CHIP_ERROR_FORMAT ", hope for the best!", err.Format());
    }
}

void FailSafeContext::HandleArmFailSafeTimer(System::Layer * layer, void * aAppState)
{
    FailSafeContext * failSafeContext = reinterpret_cast<FailSafeContext *>(aAppState);
    failSafeContext->FailSafeTimerExpired();
}

void FailSafeContext::HandleMaxCumulativeFailSafeTimer(System::Layer * layer, void * aAppState)
{
    FailSafeContext * failSafeContext = reinterpret_cast<FailSafeContext *>(aAppState);
    failSafeContext->FailSafeTimerExpired();
}

void FailSafeContext::HandleDisarmFailSafe(intptr_t arg)
{
    FailSafeContext * failSafeContext = reinterpret_cast<FailSafeContext *>(arg);
    failSafeContext->DisarmFailSafe();
}

void FailSafeContext::SetFailSafeArmed(bool armed)
{
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    if (IsFailSafeArmed() != armed)
    {
        ICDNotifier::GetInstance().BroadcastActiveRequest(ICDListener::KeepActiveFlag::kFailSafeArmed, armed);
    }
#endif
    mFailSafeArmed = armed;
}

void FailSafeContext::FailSafeTimerExpired()
{
    if (!IsFailSafeArmed())
    {
        // In case this was a pending timer event in event loop, and we had
        // done CommissioningComplete or manual disarm.
        return;
    }

    ChipLogProgress(FailSafe, "Fail-safe timer expired");
    ScheduleFailSafeCleanup(mFabricIndex, mAddNocCommandHasBeenInvoked, mUpdateNocCommandHasBeenInvoked);
}

void FailSafeContext::ScheduleFailSafeCleanup(FabricIndex fabricIndex, bool addNocCommandInvoked, bool updateNocCommandInvoked)
{
    // Not armed, but busy so cannot rearm (via General Commissioning cluster) until the flushing
    // via `HandleDisarmFailSafe` path is complete.
    // TODO: This is hacky and we need to remove all this event pushing business, to keep all fail-safe logic-only.
    mFailSafeBusy = true;

    SetFailSafeArmed(false);

    ChipDeviceEvent event{ .Type                 = DeviceEventType::kFailSafeTimerExpired,
                           .FailSafeTimerExpired = {
                               .fabricIndex                            = fabricIndex,
                               .addNocCommandHasBeenInvoked            = addNocCommandInvoked,
                               .updateNocCommandHasBeenInvoked         = updateNocCommandInvoked,
                               .updateTermsAndConditionsHasBeenInvoked = mUpdateTermsAndConditionsHasBeenInvoked,
                           } };
    CHIP_ERROR status = PlatformMgr().PostEvent(&event);

    if (status != CHIP_NO_ERROR)
    {
        ChipLogError(FailSafe, "Failed to post fail-safe timer expired: %" CHIP_ERROR_FORMAT, status.Format());
    }

    PlatformMgr().ScheduleWork(HandleDisarmFailSafe, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR FailSafeContext::ArmFailSafe(FabricIndex accessingFabricIndex, System::Clock::Seconds16 expiryLengthSeconds)
{
    VerifyOrReturnError(!IsFailSafeBusy(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err           = CHIP_NO_ERROR;
    bool cancelTimersIfError = false;
    if (!IsFailSafeArmed())
    {
        System::Clock::Timeout maxCumulativeTimeout = System::Clock::Seconds32(CHIP_DEVICE_CONFIG_MAX_CUMULATIVE_FAILSAFE_SEC);
        SuccessOrExit(err = DeviceLayer::SystemLayer().StartTimer(maxCumulativeTimeout, HandleMaxCumulativeFailSafeTimer, this));
        cancelTimersIfError = true;
    }

    SuccessOrExit(
        err = DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(expiryLengthSeconds), HandleArmFailSafeTimer, this));

    SetFailSafeArmed(true);
    mFabricIndex = accessingFabricIndex;

exit:

    if (err != CHIP_NO_ERROR && cancelTimersIfError)
    {
        DeviceLayer::SystemLayer().CancelTimer(HandleArmFailSafeTimer, this);
        DeviceLayer::SystemLayer().CancelTimer(HandleMaxCumulativeFailSafeTimer, this);
    }
    return err;
}

void FailSafeContext::DisarmFailSafe()
{
    DeviceLayer::SystemLayer().CancelTimer(HandleArmFailSafeTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleMaxCumulativeFailSafeTimer, this);

    ResetState();

    ChipLogProgress(FailSafe, "Fail-safe cleanly disarmed");
}

void FailSafeContext::ForceFailSafeTimerExpiry()
{
    if (!IsFailSafeArmed())
    {
        return;
    }

    // Cancel the timer since we force its action
    DeviceLayer::SystemLayer().CancelTimer(HandleArmFailSafeTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleMaxCumulativeFailSafeTimer, this);

    FailSafeTimerExpired();
}

CHIP_ERROR FailSafeContext::GetAddNOCStartedMarker(AddNOCStartedMarker & outMarker)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint8_t tlvBuf[AddNOCStartedMarkerContextTLVMaxSize()];
    uint16_t tlvSize = sizeof(tlvBuf);

    ReturnErrorOnFailure(
        mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::FailSafeAddNOCStartedMarkerKey().KeyName(), tlvBuf, tlvSize));

    // If buffer was too small, we won't reach here.
    TLV::ContiguousBufferTLVReader reader;
    reader.Init(tlvBuf, tlvSize);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));

    ReturnErrorOnFailure(reader.Next(kAddNOCStartedMarkerFabricIndexTag));
    ReturnErrorOnFailure(reader.Get(outMarker.fabricIndex));

    // Don't try to exit container: we got all we needed. This allows us to
    // avoid erroring-out on newer versions.

    return CHIP_NO_ERROR;
}

CHIP_ERROR FailSafeContext::StoreAddNOCStartedMarker(const AddNOCStartedMarker & marker)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint8_t tlvBuf[AddNOCStartedMarkerContextTLVMaxSize()];
    TLV::TLVWriter writer;
    writer.Init(tlvBuf);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(kAddNOCStartedMarkerFabricIndexTag, marker.fabricIndex));
    ReturnErrorOnFailure(writer.EndContainer(outerType));

    const auto markerContextTLVLength = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(markerContextTLVLength), CHIP_ERROR_BUFFER_TOO_SMALL);

    return mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::FailSafeAddNOCStartedMarkerKey().KeyName(), tlvBuf,
                                     static_cast<uint16_t>(markerContextTLVLength));
}

void FailSafeContext::ClearAddNOCStartedMarker()
{
    VerifyOrDie(mStorage != nullptr);

    mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::FailSafeAddNOCStartedMarkerKey().KeyName());
}

} // namespace app
} // namespace chip
