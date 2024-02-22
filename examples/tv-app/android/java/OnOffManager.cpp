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
#include "OnOffManager.h"
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

static constexpr size_t kOnffManagerTableSize = MATTER_DM_ON_OFF_CLUSTER_SERVER_ENDPOINT_COUNT;

namespace {

OnOffManager * gOnOffManagerTable[kOnffManagerTableSize] = { nullptr };
static_assert(kOnffManagerTableSize <= kEmberInvalidEndpointIndex, "gOnOffManagerTable table size error");

} // namespace

void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Android App::OnOff::PostClusterInit");
    TvAppJNIMgr().PostClusterInit(chip::app::Clusters::OnOff::Id, endpoint);
}

void OnOffManager::NewManager(jint endpoint, jobject manager)
{
    ChipLogProgress(Zcl, "TV Android App: OnOffManager::NewManager");
    uint16_t ep = emberAfGetClusterServerEndpointIndex(static_cast<chip::EndpointId>(endpoint), app::Clusters::OnOff::Id,
                                                       MATTER_DM_ON_OFF_CLUSTER_SERVER_ENDPOINT_COUNT);
    VerifyOrReturn(ep < kOnffManagerTableSize,
                   ChipLogError(Zcl, "TV Android App::OnOff::NewManager: endpoint %d not found", endpoint));

    VerifyOrReturn(gOnOffManagerTable[ep] == nullptr,
                   ChipLogError(Zcl, "TV Android App::OnOff::NewManager: endpoint %d already has a manager", endpoint));
    OnOffManager * mgr = new OnOffManager();
    CHIP_ERROR err     = mgr->InitializeWithObjects(manager);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "TV Android App::OnOff::NewManager: failed to initialize manager for endpoint %d", endpoint);
        delete mgr;
    }
    else
    {
        gOnOffManagerTable[ep] = mgr;
    }
}

OnOffManager * GetOnOffManager(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, app::Clusters::OnOff::Id, MATTER_DM_ON_OFF_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kOnffManagerTableSize ? nullptr : gOnOffManagerTable[ep]);
}

void OnOffManager::PostOnOffChanged(chip::EndpointId endpoint, bool value)
{
    ChipLogProgress(Zcl, "TV Android App: OnOffManager::PostOnOffChange");
    OnOffManager * mgr = GetOnOffManager(endpoint);
    VerifyOrReturn(mgr != nullptr, ChipLogError(Zcl, "OnOffManager null"));

    mgr->HandleOnOffChanged(value);
}

jboolean OnOffManager::SetOnOff(jint endpoint, bool value)
{
    chip::DeviceLayer::StackLock stack;
    chip::Protocols::InteractionModel::Status status =
        app::Clusters::OnOff::Attributes::OnOff::Set(static_cast<chip::EndpointId>(endpoint), value);
    return status == chip::Protocols::InteractionModel::Status::Success;
}

CHIP_ERROR OnOffManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NO_ENV, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ReturnLogErrorOnFailure(mOnOffManagerObject.Init(managerObject));

    jclass OnOffManagerClass = env->GetObjectClass(managerObject);
    VerifyOrReturnLogError(OnOffManagerClass != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mHandleOnOffChangedMethod = env->GetMethodID(OnOffManagerClass, "HandleOnOffChanged", "(Z)V");
    if (mHandleOnOffChangedMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access OnOffManager 'HandleOnOffChanged' method");
        env->ExceptionClear();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

void OnOffManager::HandleOnOffChanged(bool value)
{
    DeviceLayer::StackUnlock unlock;
    ChipLogProgress(Zcl, "OnOffManager::HandleOnOffChanged");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    VerifyOrReturn(mOnOffManagerObject.HasValidObjectRef(), ChipLogProgress(Zcl, "mOnOffManagerObject null"));
    VerifyOrReturn(mHandleOnOffChangedMethod != nullptr, ChipLogProgress(Zcl, "mHandleOnOffChangedMethod null"));

    env->ExceptionClear();
    env->CallVoidMethod(mOnOffManagerObject.ObjectRef(), mHandleOnOffChangedMethod, static_cast<jboolean>(value));
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in OnOffManager::HandleOnOffChanged");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
