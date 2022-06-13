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
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

static constexpr size_t kOnffManagerTableSize = EMBER_AF_ON_OFF_CLUSTER_SERVER_ENDPOINT_COUNT;

namespace {

OnOffManager * gOnOffManagerTable[kOnffManagerTableSize] = { nullptr };

}

void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Android App::OnOff::PostClusterInit");
    TvAppJNIMgr().PostClusterInit(chip::app::Clusters::OnOff::Id, endpoint);
}

void OnOffManager::NewManager(jint endpoint, jobject manager)
{
    ChipLogProgress(Zcl, "TV Android App: OnOffManager::NewManager");
    uint16_t ep = emberAfFindClusterServerEndpointIndex(static_cast<chip::EndpointId>(endpoint), app::Clusters::OnOff::Id);
    VerifyOrReturn(ep != kEmberInvalidEndpointIndex && ep < EMBER_AF_ON_OFF_CLUSTER_SERVER_ENDPOINT_COUNT,
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
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, app::Clusters::OnOff::Id);
    return ((ep == kEmberInvalidEndpointIndex || ep >= EMBER_AF_MEDIA_PLAYBACK_CLUSTER_SERVER_ENDPOINT_COUNT)
                ? nullptr
                : gOnOffManagerTable[ep]);
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
    EmberAfStatus status = app::Clusters::OnOff::Attributes::OnOff::Set(static_cast<chip::EndpointId>(endpoint), value);
    return status == EMBER_ZCL_STATUS_SUCCESS;
}

CHIP_ERROR OnOffManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnLogError(env != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mOnOffManagerObject = env->NewGlobalRef(managerObject);
    VerifyOrReturnLogError(mOnOffManagerObject != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

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
    ChipLogProgress(Zcl, "OnOffManager::HandleOnOffChanged");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != NULL, ChipLogProgress(Zcl, "env null"));
    VerifyOrReturn(mOnOffManagerObject != nullptr, ChipLogProgress(Zcl, "mOnOffManagerObject null"));
    VerifyOrReturn(mHandleOnOffChangedMethod != nullptr, ChipLogProgress(Zcl, "mHandleOnOffChangedMethod null"));

    env->ExceptionClear();
    env->CallVoidMethod(mOnOffManagerObject, mHandleOnOffChangedMethod, static_cast<jboolean>(value));
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in OnOffManager::HandleOnOffChanged");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
