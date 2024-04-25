/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "MediaInputManager.h"
#include "TvApp-JNI.h"
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#include <string>

using namespace chip;
using namespace chip::app::Clusters::MediaInput;

/** @brief Media Input Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver: always
 *
 */
void emberAfMediaInputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Android App: MediaInput::PostClusterInit");
    TvAppJNIMgr().PostClusterInit(chip::app::Clusters::MediaInput::Id, endpoint);
}

void MediaInputManager::NewManager(jint endpoint, jobject manager)
{
    ChipLogProgress(Zcl, "TV Android App: MediaInput::SetDefaultDelegate");
    MediaInputManager * mgr = new MediaInputManager();
    mgr->InitializeWithObjects(manager);
    chip::app::Clusters::MediaInput::SetDefaultDelegate(static_cast<EndpointId>(endpoint), mgr);
}

CHIP_ERROR MediaInputManager::HandleGetInputList(chip::app::AttributeValueEncoder & aEncoder)
{
    DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NO_ENV, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received MediaInputManager::HandleGetInputList");
    VerifyOrExit(mMediaInputManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetInputListMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    return aEncoder.EncodeList([this, env](const auto & encoder) -> CHIP_ERROR {
        jobjectArray inputArray = (jobjectArray) env->CallObjectMethod(mMediaInputManagerObject.ObjectRef(), mGetInputListMethod);
        if (env->ExceptionCheck())
        {
            ChipLogError(AppServer, "Java exception in MediaInputManager::HandleGetInputList");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return CHIP_ERROR_INCORRECT_STATE;
        }

        jint size = env->GetArrayLength(inputArray);
        for (int i = 0; i < size; i++)
        {
            app::Clusters::MediaInput::Structs::InputInfoStruct::Type mediaInput;

            jobject inputObj  = env->GetObjectArrayElement(inputArray, i);
            jclass inputClass = env->GetObjectClass(inputObj);

            jfieldID indexId = env->GetFieldID(inputClass, "index", "I");
            jint index       = env->GetIntField(inputObj, indexId);
            mediaInput.index = static_cast<uint8_t>(index);

            jfieldID typeId      = env->GetFieldID(inputClass, "type", "I");
            jint type            = env->GetIntField(inputObj, typeId);
            mediaInput.inputType = static_cast<app::Clusters::MediaInput::InputTypeEnum>(type);

            jfieldID nameId = env->GetFieldID(inputClass, "name", "Ljava/lang/String;");
            jstring jname   = static_cast<jstring>(env->GetObjectField(inputObj, nameId));

            JniUtfString name(env, jname);
            if (jname != NULL)
            {
                mediaInput.name = name.charSpan();
            }

            jfieldID descriptionId = env->GetFieldID(inputClass, "description", "Ljava/lang/String;");
            jstring jdescription   = static_cast<jstring>(env->GetObjectField(inputObj, descriptionId));

            JniUtfString description(env, jdescription);
            if (jdescription != NULL)
            {
                mediaInput.description = description.charSpan();
            }

            ReturnErrorOnFailure(encoder.Encode(mediaInput));
        }

        return CHIP_NO_ERROR;
    });

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "MediaInputManager::GetInputList status error: %s", err.AsString());
    }

    return err;
}

uint8_t MediaInputManager::HandleGetCurrentInput()
{
    DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    jint index     = -1;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, 0, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received MediaInputManager::HandleGetCurrentInput");
    VerifyOrExit(mMediaInputManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetCurrentInputMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    {
        index = env->CallIntMethod(mMediaInputManagerObject.ObjectRef(), mGetCurrentInputMethod);
        if (env->ExceptionCheck())
        {
            ChipLogError(AppServer, "Java exception in MediaInputManager::HandleGetCurrentInput");
            env->ExceptionDescribe();
            env->ExceptionClear();
            err = CHIP_ERROR_INCORRECT_STATE;
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "MediaInputManager::HandleGetCurrentInput status error: %s", err.AsString());
    }

    return uint8_t(index);
}

bool MediaInputManager::HandleSelectInput(const uint8_t index)
{
    DeviceLayer::StackUnlock unlock;
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, false, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received MediaInputManager::HandleSelectInput %d", index);
    VerifyOrExit(mMediaInputManagerObject.HasValidObjectRef(), ChipLogError(Zcl, "mMediaInputManagerObject is not valid"));
    VerifyOrExit(mSelectInputMethod != nullptr, ChipLogError(Zcl, "mSelectInputMethod null"));

    env->ExceptionClear();
    ret = env->CallBooleanMethod(mMediaInputManagerObject.ObjectRef(), mSelectInputMethod, static_cast<jint>(index));
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in MediaInputManager::HandleSelectInput");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

exit:
    return static_cast<bool>(ret);
}

bool MediaInputManager::HandleShowInputStatus()
{
    DeviceLayer::StackUnlock unlock;
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, false, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received MediaInputManager::HandleShowInputStatus");
    VerifyOrExit(mMediaInputManagerObject.HasValidObjectRef(), ChipLogError(Zcl, "mMediaInputManagerObject is not valid"));
    VerifyOrExit(mShowInputStatusMethod != nullptr, ChipLogError(Zcl, "mShowInputStatusMethod null"));

    env->ExceptionClear();
    ret = env->CallBooleanMethod(mMediaInputManagerObject.ObjectRef(), mShowInputStatusMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in MediaInputManager::HandleShowInputStatus");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

exit:
    return static_cast<bool>(ret);
}

bool MediaInputManager::HandleHideInputStatus()
{
    DeviceLayer::StackUnlock unlock;
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, false, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received MediaInputManager::HandleHideInputStatus");
    VerifyOrExit(mMediaInputManagerObject.HasValidObjectRef(), ChipLogError(Zcl, "mMediaInputManagerObject is not valid"));
    VerifyOrExit(mHideInputStatusMethod != nullptr, ChipLogError(Zcl, "mHideInputStatusMethod null"));

    env->ExceptionClear();
    ret = env->CallBooleanMethod(mMediaInputManagerObject.ObjectRef(), mHideInputStatusMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in MediaInputManager::HandleHideInputStatus");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

exit:
    return static_cast<bool>(ret);
}

bool MediaInputManager::HandleRenameInput(const uint8_t index, const chip::CharSpan & name)
{
    DeviceLayer::StackUnlock unlock;
    std::string inputname(name.data(), name.size());
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, false, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received MediaInputManager::HandleRenameInput %d to %s", index, name.data());
    VerifyOrExit(mMediaInputManagerObject.HasValidObjectRef(), ChipLogError(Zcl, "mMediaInputManagerObject is not valid"));
    VerifyOrExit(mRenameInputMethod != nullptr, ChipLogError(Zcl, "mHideInputStatusMethod null"));

    env->ExceptionClear();

    {
        UtfString jniInputname(env, inputname.data());
        ret = env->CallBooleanMethod(mMediaInputManagerObject.ObjectRef(), mRenameInputMethod, static_cast<jint>(index),
                                     jniInputname.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(DeviceLayer, "Java exception in MediaInputManager::HandleRenameInput");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return false;
        }
    }

exit:
    return static_cast<bool>(ret);
}

void MediaInputManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for MediaInputManager"));

    VerifyOrReturn(mMediaInputManagerObject.Init(managerObject) == CHIP_NO_ERROR,
                   ChipLogError(Zcl, "Failed to init mMediaInputManagerObject"));

    jclass MediaInputManagerClass = env->GetObjectClass(managerObject);
    VerifyOrReturn(MediaInputManagerClass != nullptr, ChipLogError(Zcl, "Failed to get MediaInputManager Java class"));

    mGetInputListMethod =
        env->GetMethodID(MediaInputManagerClass, "getInputList", "()[Lcom/matter/tv/server/tvapp/MediaInputInfo;");
    if (mGetInputListMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaInputManager 'getInputList' method");
        env->ExceptionClear();
    }

    mGetCurrentInputMethod = env->GetMethodID(MediaInputManagerClass, "getCurrentInput", "()I");
    if (mGetCurrentInputMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaInputManager 'getCurrentInput' method");
        env->ExceptionClear();
    }

    mSelectInputMethod = env->GetMethodID(MediaInputManagerClass, "selectInput", "(I)Z");
    if (mSelectInputMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaInputManager 'selectInput' method");
        env->ExceptionClear();
    }

    mShowInputStatusMethod = env->GetMethodID(MediaInputManagerClass, "showInputStatus", "()Z");
    if (mShowInputStatusMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaInputManager 'showInputStatus' method");
        env->ExceptionClear();
    }

    mHideInputStatusMethod = env->GetMethodID(MediaInputManagerClass, "hideInputStatus", "()Z");
    if (mHideInputStatusMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaInputManager 'hideInputStatus' method");
        env->ExceptionClear();
    }

    mRenameInputMethod = env->GetMethodID(MediaInputManagerClass, "renameInput", "(ILjava/lang/String;)Z");
    if (mRenameInputMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaInputManager 'renameInput' method");
        env->ExceptionClear();
    }
}
