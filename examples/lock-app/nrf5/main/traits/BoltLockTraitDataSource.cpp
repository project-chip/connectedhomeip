/*
 *
 *    Copyright (c) 2019 Google LLC.
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
 *      A trait data source implementation for the Weave Security BoltLockTrait.
 *
 */

#include <traits/include/BoltLockTraitDataSource.h>
#include <schema/include/BoltLockTrait.h>
#include "nrf_log.h"
#include <WDMFeature.h>
#include <BoltLockManager.h>
#include <AppTask.h>

#include <Weave/DeviceLayer/WeaveDeviceLayer.h>
#include <Weave/Support/TraitEventUtils.h>

using namespace nl::Weave;
using namespace nl::Weave::TLV;
using namespace nl::Weave::Profiles::DataManagement;
using namespace Schema::Weave::Trait::Security;
using namespace Schema::Weave::Trait::Security::BoltLockTrait;

BoltLockTraitDataSource::BoltLockTraitDataSource() : TraitDataSource(&BoltLockTrait::TraitSchema)
{
    mLockedState   = BOLT_LOCKED_STATE_LOCKED;
    mLockActor     = BOLT_LOCK_ACTOR_METHOD_PHYSICAL;
    mActuatorState = BOLT_ACTUATOR_STATE_OK;
    mState         = BOLT_STATE_EXTENDED;
}

bool BoltLockTraitDataSource::IsLocked()
{
    bool lock_state = false;
    if (mLockedState == BOLT_LOCKED_STATE_LOCKED)
    {
        lock_state = true;
    }

    return lock_state;
}

void BoltLockTraitDataSource::InitiateLock(int32_t aLockActor)
{
    Lock();

    mLockActor     = aLockActor;
    mActuatorState = BOLT_ACTUATOR_STATE_LOCKING;
    mState         = BOLT_STATE_EXTENDED;

    SetDirty(BoltLockTrait::kPropertyHandle_State);
    SetDirty(BoltLockTrait::kPropertyHandle_BoltLockActor_Method);
    SetDirty(BoltLockTrait::kPropertyHandle_ActuatorState);

    Unlock();

    BoltActuatorStateChangeEvent ev;
    EventOptions options(true);
    ev.state = BOLT_STATE_EXTENDED;
    ev.actuatorState = BOLT_ACTUATOR_STATE_LOCKING;
    ev.lockedState = BOLT_LOCKED_STATE_UNLOCKED;
    ev.boltLockActor.method = aLockActor;
    ev.boltLockActor.SetOriginatorNull();
    ev.boltLockActor.SetAgentNull();
    nl::LogEvent(&ev, options);

    WdmFeature().ProcessTraitChanges();

}

void BoltLockTraitDataSource::InitiateUnlock(int32_t aLockActor)
{
    Lock();

    mLockActor     = aLockActor;
    mActuatorState = BOLT_ACTUATOR_STATE_UNLOCKING;
    mLockedState   = BOLT_LOCKED_STATE_UNLOCKED;

    SetDirty(BoltLockTrait::kPropertyHandle_BoltLockActor_Method);
    SetDirty(BoltLockTrait::kPropertyHandle_ActuatorState);
    SetDirty(BoltLockTrait::kPropertyHandle_LockedState);
    SetDirty(BoltLockTrait::kPropertyHandle_LockedStateLastChangedAt);

    Unlock();

    BoltActuatorStateChangeEvent ev;
    EventOptions options(true);
    ev.state = BOLT_STATE_EXTENDED;
    ev.actuatorState = BOLT_ACTUATOR_STATE_UNLOCKING;
    ev.lockedState = BOLT_LOCKED_STATE_UNLOCKED;
    ev.boltLockActor.method = aLockActor;
    ev.boltLockActor.SetOriginatorNull();
    ev.boltLockActor.SetAgentNull();
    nl::LogEvent(&ev, options);

    WdmFeature().ProcessTraitChanges();
}

void BoltLockTraitDataSource::LockingSuccessful(void)
{
    Lock();

    mActuatorState = BOLT_ACTUATOR_STATE_OK;
    mLockedState   = BOLT_LOCKED_STATE_LOCKED;

    SetDirty(BoltLockTrait::kPropertyHandle_ActuatorState);
    SetDirty(BoltLockTrait::kPropertyHandle_LockedState);
    SetDirty(BoltLockTrait::kPropertyHandle_LockedStateLastChangedAt);

    Unlock();

    BoltActuatorStateChangeEvent ev;
    EventOptions options(true);
    ev.state = BOLT_STATE_EXTENDED;
    ev.actuatorState = BOLT_ACTUATOR_STATE_OK;
    ev.lockedState = BOLT_LOCKED_STATE_LOCKED;
    ev.boltLockActor.method = mLockActor;
    ev.boltLockActor.SetOriginatorNull();
    ev.boltLockActor.SetAgentNull();
    nl::LogEvent(&ev, options);

    WdmFeature().ProcessTraitChanges();
}

void BoltLockTraitDataSource::UnlockingSuccessful(void)
{
    Lock();

    mState         = BOLT_STATE_RETRACTED;
    mActuatorState = BOLT_ACTUATOR_STATE_OK;

    SetDirty(BoltLockTrait::kPropertyHandle_State);
    SetDirty(BoltLockTrait::kPropertyHandle_ActuatorState);

    Unlock();

    BoltActuatorStateChangeEvent ev;
    EventOptions options(true);
    ev.state = BOLT_STATE_RETRACTED;
    ev.actuatorState = BOLT_ACTUATOR_STATE_OK;
    ev.lockedState = BOLT_LOCKED_STATE_UNLOCKED;
    ev.boltLockActor.method = mLockActor;
    ev.boltLockActor.SetOriginatorNull();
    ev.boltLockActor.SetAgentNull();
    nl::LogEvent(&ev, options);

    WdmFeature().ProcessTraitChanges();
}

WEAVE_ERROR BoltLockTraitDataSource::GetLeafData(PropertyPathHandle aLeafHandle, uint64_t aTagToWrite, TLVWriter & aWriter)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    switch (aLeafHandle)
    {
        case BoltLockTrait::kPropertyHandle_State:
            err = aWriter.Put(aTagToWrite, mState);
            SuccessOrExit(err);
            break;

        case BoltLockTrait::kPropertyHandle_ActuatorState:
            err = aWriter.Put(aTagToWrite, mActuatorState);
            SuccessOrExit(err);
            break;

        case BoltLockTrait::kPropertyHandle_LockedState:
            err = aWriter.Put(aTagToWrite, mLockedState);
            SuccessOrExit(err);
            break;

        case BoltLockTrait::kPropertyHandle_BoltLockActor_Method:
            err = aWriter.Put(aTagToWrite, mLockActor);
            SuccessOrExit(err);
            break;

        case BoltLockTrait::kPropertyHandle_LockedStateLastChangedAt:
        {
            uint64_t currentTime = 0;
            System::Platform::Layer::GetClock_RealTimeMS(currentTime);
            err = aWriter.Put(aTagToWrite, static_cast<int64_t>(currentTime));
            SuccessOrExit(err);
            break;
        }

        case BoltLockTrait::kPropertyHandle_BoltLockActor_Originator:
        case BoltLockTrait::kPropertyHandle_BoltLockActor_Agent:
            err = aWriter.PutNull(aTagToWrite);
            SuccessOrExit(err);
            break;

        default:
            NRF_LOG_INFO("Unexpected Leaf");
            break;
    }

exit:
    return err;
}

void BoltLockTraitDataSource::OnCustomCommand(nl::Weave::Profiles::DataManagement::Command * aCommand,
                                              const nl::Weave::WeaveMessageInfo * aMsgInfo, nl::Weave::PacketBuffer * aPayload,
                                              const uint64_t & aCommandType, const bool aIsExpiryTimeValid,
                                              const int64_t & aExpiryTimeMicroSecond, const bool aIsMustBeVersionValid,
                                              const uint64_t & aMustBeVersion, nl::Weave::TLV::TLVReader & aArgumentReader)
{
    WEAVE_ERROR err           = WEAVE_NO_ERROR;
    uint32_t reportProfileId  = nl::Weave::Profiles::kWeaveProfile_Common;
    uint16_t reportStatusCode = nl::Weave::Profiles::Common::kStatus_BadRequest;

    if (aIsMustBeVersionValid)
    {
        if (aMustBeVersion != GetVersion())
        {
            NRF_LOG_INFO("Actual version is 0x%X, while must-be version is: 0x%" PRIx64, GetVersion(), aMustBeVersion);
            reportProfileId  = nl::Weave::Profiles::kWeaveProfile_WDM;
            reportStatusCode = kStatus_VersionMismatch;
            goto exit;
        }
    }

    if (aIsExpiryTimeValid)
    {
        reportProfileId = nl::Weave::Profiles::kWeaveProfile_WDM;
#if WEAVE_DEVICE_CONFIG_ENABLE_WEAVE_TIME_SERVICE_TIME_SYNC
        uint64_t currentTime;
        err = System::Platform::Layer::GetClock_RealTimeMS(currentTime);
        if (err == WEAVE_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED)
        {
            NRF_LOG_INFO("BoltLockChangeRequest Command failed!");
            reportStatusCode = kStatus_NotTimeSyncedYet;
            goto exit;
        }

        // If we have already passed the commands expiration time,
        // error out
        if (aExpiryTimeMicroSecond < static_cast<int64_t>(currentTime))
        {
            NRF_LOG_INFO("BoltLockChangeRequest Command Expired!");
            reportStatusCode = kStatus_RequestExpiredInTime;
            goto exit;
        }

#else
        reportStatusCode = kStatus_ExpiryTimeNotSupported;
        goto exit;
#endif
    }

    VerifyOrExit(aCommandType == BoltLockTrait::kBoltLockChangeRequestId, err = WEAVE_ERROR_NOT_IMPLEMENTED);

    NRF_LOG_INFO("BoltLockChangeRequest Command Valid!");

    {
        int32_t changeRequestParam_State;
        int32_t changeRequestParam_Actor;
        nl::Weave::TLV::TLVType OuterContainerType;
        err = aArgumentReader.EnterContainer(OuterContainerType);
        SuccessOrExit(err);

        while (WEAVE_NO_ERROR == (err = aArgumentReader.Next()))
        {
            VerifyOrExit(nl::Weave::TLV::IsContextTag(aArgumentReader.GetTag()), err = WEAVE_ERROR_INVALID_TLV_TAG);
            switch (nl::Weave::TLV::TagNumFromTag(aArgumentReader.GetTag()))
            {
                case kBoltLockChangeRequestParameter_State:
                    err = aArgumentReader.Get(changeRequestParam_State);
                    SuccessOrExit(err);
                    break;

                case kBoltLockChangeRequestParameter_BoltLockActor:
                {
                    WEAVE_ERROR err;
                    nl::Weave::TLV::TLVType InnerContainerType;
                    err = aArgumentReader.EnterContainer(InnerContainerType);
                    SuccessOrExit(err);

                    err = aArgumentReader.Next();
                    SuccessOrExit(err);

                    err = aArgumentReader.Get(changeRequestParam_Actor);
                    SuccessOrExit(err);

                    err = aArgumentReader.ExitContainer(InnerContainerType);
                    SuccessOrExit(err);
                    break;
                }

                default:
                    // Unrecognized arguments are not allowed.
                    NRF_LOG_INFO("Unexpected Tag in CustomCommand");
                    ExitNow(err = WEAVE_ERROR_INVALID_TLV_TAG);
            }
        }

        if (WEAVE_END_OF_TLV == err)
        {
            err = WEAVE_NO_ERROR;
        }
        SuccessOrExit(err);

        if (changeRequestParam_State == BOLT_STATE_RETRACTED)
        {
            GetAppTask().PostLockActionRequest(changeRequestParam_Actor, BoltLockManager::UNLOCK_ACTION);
        }
        else if (changeRequestParam_State == BOLT_STATE_EXTENDED)
        {
            GetAppTask().PostLockActionRequest(changeRequestParam_Actor, BoltLockManager::LOCK_ACTION);
        }
        else
        {
            // Command changeRequestParam_State value is invalid.
            err = WEAVE_ERROR_STATUS_REPORT_RECEIVED;
        }
    }

    PacketBuffer::Free(aPayload);
    aPayload = NULL;

    // Generate a success response right here.
    if (err == WEAVE_NO_ERROR)
    {
        NRF_LOG_INFO("BoltLockChangeRequest Command Parsed!");

        PacketBuffer * msgBuf = PacketBuffer::New();
        if (NULL == msgBuf)
        {
            reportProfileId  = nl::Weave::Profiles::kWeaveProfile_Common;
            reportStatusCode = nl::Weave::Profiles::Common::kStatus_OutOfMemory;
            ExitNow(err = WEAVE_ERROR_NO_MEMORY);
        }

        NRF_LOG_INFO("Sending Success Response to BoltLockChangeRequest Command");
        aCommand->SendResponse(GetVersion(), msgBuf);
        aCommand = NULL;
        msgBuf   = NULL;
    }
    else
    {
        NRF_LOG_INFO("BoltLockChangeRequest Command Error : %d", err);
    }

exit:
    if (NULL != aCommand)
    {
        aCommand->SendError(reportProfileId, reportStatusCode, err);
        aCommand = NULL;
    }

    if (aPayload)
    {
        PacketBuffer::Free(aPayload);
        aPayload = NULL;
    }
}
