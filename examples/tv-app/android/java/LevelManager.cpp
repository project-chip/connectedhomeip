/**
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
#include "LevelManager.h"
#include "TvApp-JNI.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

static constexpr size_t kLevelManagerTableSize = MATTER_DM_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT;

namespace {

LevelManager * gLevelManagerTable[kLevelManagerTableSize] = { nullptr };
static_assert(kLevelManagerTableSize <= kEmberInvalidEndpointIndex, "gLevelManagerTable table size error");

} // namespace

void emberAfLevelControlClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Android App::Level::PostClusterInit");
    TvAppJNIMgr().PostClusterInit(chip::app::Clusters::LevelControl::Id, endpoint);
}

void LevelManager::NewManager(jint endpoint, jobject manager)
{
    ChipLogProgress(Zcl, "TV Android App: LevelManager::NewManager");
    uint16_t ep = emberAfGetClusterServerEndpointIndex(static_cast<chip::EndpointId>(endpoint), app::Clusters::LevelControl::Id,
                                                       MATTER_DM_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    VerifyOrReturn(ep < kLevelManagerTableSize,
                   ChipLogError(Zcl, "TV Android App::Level::NewManager: endpoint %d not found", endpoint));

    VerifyOrReturn(gLevelManagerTable[ep] == nullptr,
                   ChipLogError(Zcl, "TV Android App::Level::NewManager: endpoint %d already has a manager", endpoint));
    LevelManager * mgr = new LevelManager();
    CHIP_ERROR err     = mgr->InitializeWithObjects(manager);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "TV Android App::Level::NewManager: failed to initialize manager for endpoint %d", endpoint);
        delete mgr;
    }
    else
    {
        gLevelManagerTable[ep] = mgr;
    }
}

LevelManager * GetLevelManager(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, app::Clusters::LevelControl::Id,
                                                       MATTER_DM_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return ((ep >= kLevelManagerTableSize) ? nullptr : gLevelManagerTable[ep]);
}

void LevelManager::PostLevelChanged(chip::EndpointId endpoint, uint8_t value)
{
    ChipLogProgress(Zcl, "TV Android App: LevelManager::PostLevelChanged");
    LevelManager * mgr = GetLevelManager(endpoint);
    VerifyOrReturn(mgr != nullptr, ChipLogError(Zcl, "LevelManager null"));

    mgr->HandleLevelChanged(value);
}

jboolean LevelManager::SetLevel(jint endpoint, jint value)
{
    chip::Protocols::InteractionModel::Status status = app::Clusters::LevelControl::Attributes::CurrentLevel::Set(
        static_cast<chip::EndpointId>(endpoint), static_cast<uint8_t>(value));
    return status == chip::Protocols::InteractionModel::Status::Success;
}

CHIP_ERROR LevelManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnLogError(env != nullptr, CHIP_ERROR_INCORRECT_STATE);
    JniLocalReferenceScope scope(env);

    ReturnLogErrorOnFailure(mLevelManagerObject.Init(managerObject));

    jclass LevelManagerClass = env->GetObjectClass(managerObject);
    VerifyOrReturnLogError(LevelManagerClass != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mHandleLevelChangedMethod = env->GetMethodID(LevelManagerClass, "HandleLevelChanged", "(I)V");
    if (mHandleLevelChangedMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access LevelManager 'HandleLevelChanged' method");
        env->ExceptionClear();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

void LevelManager::HandleLevelChanged(uint8_t value)
{
    DeviceLayer::StackUnlock unlock;
    ChipLogProgress(Zcl, "LevelManager::HandleLevelChanged");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    VerifyOrReturn(mLevelManagerObject.HasValidObjectRef(), ChipLogError(Zcl, "mLevelManagerObject is not valid"));
    VerifyOrReturn(mHandleLevelChangedMethod != nullptr, ChipLogProgress(Zcl, "mHandleLevelChangedMethod null"));

    env->ExceptionClear();
    env->CallVoidMethod(mLevelManagerObject.ObjectRef(), mHandleLevelChangedMethod, static_cast<jint>(value));
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in LevelManager::HandleLevelChanged");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
