/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "KeypadInputManager.h"
#include "TvApp-JNI.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/config.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;
using namespace chip::app::Clusters::KeypadInput;

void emberAfKeypadInputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Android App: KeypadInput::PostClusterInit");
    TvAppJNIMgr().PostClusterInit(chip::app::Clusters::KeypadInput::Id, endpoint);
}

void KeypadInputManager::NewManager(jint endpoint, jobject manager)
{
    ChipLogProgress(Zcl, "TV Android App: KeypadInput::SetDefaultDelegate");
    KeypadInputManager * mgr = new KeypadInputManager();
    mgr->InitializeWithObjects(manager);
    chip::app::Clusters::KeypadInput::SetDefaultDelegate(static_cast<EndpointId>(endpoint), mgr);
}

void KeypadInputManager::HandleSendKey(CommandResponseHelper<SendKeyResponseType> & helper, const CECKeyCodeEnum & keyCode)
{
    DeviceLayer::StackUnlock unlock;
    Commands::SendKeyResponse::Type response;

    jint ret       = -1;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received keypadInputClusterSendKey: %c", to_underlying(keyCode));
    VerifyOrExit(mKeypadInputManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mSendKeyMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();
    ret = env->CallIntMethod(mKeypadInputManagerObject.ObjectRef(), mSendKeyMethod, static_cast<jint>(keyCode));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        response.status = chip::app::Clusters::KeypadInput::StatusEnum::kInvalidKeyInCurrentState;
    }
    else
    {
        response.status = static_cast<chip::app::Clusters::KeypadInput::StatusEnum>(ret);
    }
    helper.Success(response);
}

void KeypadInputManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for KeypadInputManager"));
    JniLocalReferenceScope scope(env);

    VerifyOrReturn(mKeypadInputManagerObject.Init(managerObject) == CHIP_NO_ERROR,
                   ChipLogError(Zcl, "Failed to init mKeypadInputManagerObject"));

    jclass KeypadInputManagerClass = env->GetObjectClass(managerObject);
    VerifyOrReturn(KeypadInputManagerClass != nullptr, ChipLogError(Zcl, "Failed to get KeypadInputManager Java class"));

    mSendKeyMethod = env->GetMethodID(KeypadInputManagerClass, "sendKey", "(I)I");
    if (mSendKeyMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access KeypadInputManager 'sendKey' method");
        env->ExceptionClear();
    }
}

uint32_t KeypadInputManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}
