/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "DoorLockManager.h"
#include "DeviceApp-JNI.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/reporting/reporting.h>
#include <app/util/config.h>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/ZclString.h>
#include <platform/PlatformManager.h>
#include <vector>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DoorLock;
using namespace chip::DeviceLayer;

static constexpr size_t kDoorLockManagerTableSize =
    MATTER_DM_DOOR_LOCK_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

namespace {

DoorLockManager * gDoorLockManagerTable[kDoorLockManagerTableSize] = { nullptr };

}

void emberAfDoorLockClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Device App::DoorLock::PostClusterInit");
    DeviceAppJNIMgr().PostClusterInit(chip::app::Clusters::DoorLock::Id, endpoint);
    DoorLockServer::Instance().InitServer(endpoint);
    Protocols::InteractionModel::Status status = DoorLock::Attributes::FeatureMap::Set(endpoint, 0);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogProgress(Zcl, "Device App::DoorLock::emberAfDoorLockClusterInitCallback()::Updating feature map %x",
                        to_underlying(status));
    }
}

bool emberAfPluginDoorLockOnDoorLockCommand(EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                            const Nullable<chip::NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                            OperationErrorEnum & err)
{
    ChipLogProgress(Zcl, "Device App::DoorLock::emberAfPluginDoorLockOnDoorLockCommand");
    return DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kLocked);
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                              const Nullable<chip::NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                              OperationErrorEnum & err)
{
    ChipLogProgress(Zcl, "Device App::DoorLock::emberAfPluginDoorLockOnDoorUnlockCommand");
    return DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kUnlocked);
}

void DoorLockManager::NewManager(jint endpoint, jobject manager)
{
    ChipLogProgress(Zcl, "Device App: DoorLockManager::NewManager");
    uint16_t ep = emberAfGetClusterServerEndpointIndex(static_cast<chip::EndpointId>(endpoint), app::Clusters::DoorLock::Id,
                                                       MATTER_DM_DOOR_LOCK_CLUSTER_SERVER_ENDPOINT_COUNT);
    VerifyOrReturn(ep < kDoorLockManagerTableSize,
                   ChipLogError(Zcl, "Device App::DoorLock::NewManager: endpoint %d not found", endpoint));

    VerifyOrReturn(gDoorLockManagerTable[ep] == nullptr,
                   ChipLogError(Zcl, "Device App::DoorLock::NewManager: endpoint %d already has a manager", endpoint));
    DoorLockManager * mgr = new DoorLockManager();
    CHIP_ERROR err        = mgr->InitializeWithObjects(manager);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Device App::DoorLock::NewManager: failed to initialize manager for endpoint %d", endpoint);
        delete mgr;
    }
    else
    {
        gDoorLockManagerTable[ep] = mgr;
    }
}

DoorLockManager * GetDoorLockManager(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, app::Clusters::DoorLock::Id,
                                                       MATTER_DM_DOOR_LOCK_CLUSTER_SERVER_ENDPOINT_COUNT);
    return ((ep >= kDoorLockManagerTableSize) ? nullptr : gDoorLockManagerTable[ep]);
}

jboolean DoorLockManager::SetLockType(jint endpoint, jint value)
{
    Protocols::InteractionModel::Status status = app::Clusters::DoorLock::Attributes::LockType::Set(
        static_cast<chip::EndpointId>(endpoint), static_cast<app::Clusters::DoorLock::DlLockType>(value));
    return status == Protocols::InteractionModel::Status::Success;
}

jboolean DoorLockManager::SetLockState(jint endpoint, jint value)
{
    return DoorLockServer::Instance().SetLockState(static_cast<chip::EndpointId>(endpoint),
                                                   static_cast<app::Clusters::DoorLock::DlLockState>(value));
}

jboolean DoorLockManager::SetActuatorEnabled(jint endpoint, jboolean value)
{
    return DoorLockServer::Instance().SetActuatorEnabled(static_cast<chip::EndpointId>(endpoint), value);
}

jboolean DoorLockManager::SetAutoRelockTime(jint endpoint, jint value)
{
    return DoorLockServer::Instance().SetAutoRelockTime(static_cast<chip::EndpointId>(endpoint), static_cast<uint32_t>(value));
}

jboolean DoorLockManager::SetOperatingMode(jint endpoint, jint value)
{
    Protocols::InteractionModel::Status status = app::Clusters::DoorLock::Attributes::OperatingMode::Set(
        static_cast<chip::EndpointId>(endpoint), static_cast<app::Clusters::DoorLock::OperatingModeEnum>(value));
    return status == Protocols::InteractionModel::Status::Success;
}

jboolean DoorLockManager::SetSupportedOperatingModes(jint endpoint, jint value)
{
    Protocols::InteractionModel::Status status = app::Clusters::DoorLock::Attributes::SupportedOperatingModes::Set(
        static_cast<chip::EndpointId>(endpoint), static_cast<app::Clusters::DoorLock::DlSupportedOperatingModes>(value));
    return status == Protocols::InteractionModel::Status::Success;
}

jboolean DoorLockManager::SendLockAlarmEvent(jint endpoint)
{
    return DoorLockServer::Instance().SendLockAlarmEvent(static_cast<chip::EndpointId>(endpoint), AlarmCodeEnum::kDoorForcedOpen);
}

void DoorLockManager::PostLockStateChanged(chip::EndpointId endpoint, int value)
{
    ChipLogProgress(Zcl, "Device App: DoorLockManager::PostLockStateChanged:%d", value);
    DoorLockManager * mgr = GetDoorLockManager(endpoint);
    VerifyOrReturn(mgr != nullptr, ChipLogError(Zcl, "DoorLockManager null"));

    mgr->HandleLockStateChanged(static_cast<int>(endpoint), value);
}

CHIP_ERROR DoorLockManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnLogError(env != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnLogErrorOnFailure(mDoorLockManagerObject.Init(managerObject));

    jclass DoorLockManagerClass = env->GetObjectClass(managerObject);
    VerifyOrReturnLogError(DoorLockManagerClass != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mHandleLockStateChangedMethod = env->GetMethodID(DoorLockManagerClass, "handleLockStateChanged", "(I)V");
    if (mHandleLockStateChangedMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access DoorLockManager 'handleLockStateChanged' method");
        env->ExceptionClear();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

void DoorLockManager::HandleLockStateChanged(jint endpoint, jint value)
{
    ChipLogProgress(Zcl, "DoorLockManager::HandleLockStateChanged:%d", value);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != NULL, ChipLogProgress(Zcl, "env null"));
    VerifyOrReturn(mDoorLockManagerObject.HasValidObjectRef(), ChipLogProgress(Zcl, "mDoorLockManagerObject null"));
    VerifyOrReturn(mHandleLockStateChangedMethod != nullptr, ChipLogProgress(Zcl, "mHandleLockStateChangedMethod null"));

    env->ExceptionClear();
    env->CallVoidMethod(mDoorLockManagerObject.ObjectRef(), mHandleLockStateChangedMethod, value);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in DoorLockManager::HandleLockStateChanged");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
