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
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/attribute-storage.h>
#include <cstdlib>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <string>

using namespace chip;

MediaInputManager MediaInputManager::sInstance;

class MediaInputAttrAccess : public app::AttributeAccessInterface
{
public:
    MediaInputAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), app::Clusters::MediaInput::Id) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override
    {
        if (aPath.mAttributeId == app::Clusters::MediaInput::Attributes::MediaInputList::Id)
        {
            return MediaInputMgr().GetInputList(aEncoder);
        }
        else if (aPath.mAttributeId == app::Clusters::MediaInput::Attributes::CurrentMediaInput::Id)
        {
            return MediaInputMgr().GetCurrentInput(aEncoder);
        }

        return CHIP_NO_ERROR;
    }
};

MediaInputAttrAccess gMediaInputAttrAccess;

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
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gMediaInputAttrAccess);
        attrAccessRegistered = true;
    }
}

bool mediaInputClusterSelectInput(uint8_t input)
{
    return MediaInputMgr().SelectInput(input);
}

bool mediaInputClusterShowInputStatus()
{
    return MediaInputMgr().ShowInputStatus();
}

bool mediaInputClusterHideInputStatus()
{
    return MediaInputMgr().HideInputStatus();
}

bool mediaInputClusterRenameInput(uint8_t input, std::string name)
{
    return MediaInputMgr().RenameInput(input, name);
}

CHIP_ERROR MediaInputManager::GetInputList(app::AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received MediaInputManager::GetInputList");
    VerifyOrExit(mMediaInputManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetInputListMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    return aEncoder.EncodeList([this, env](const auto & encoder) -> CHIP_ERROR {
        jobjectArray inputArray = (jobjectArray) env->CallObjectMethod(mMediaInputManagerObject, mGetInputListMethod);
        if (env->ExceptionCheck())
        {
            ChipLogError(AppServer, "Java exception in MediaInputManager::GetInputList");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return CHIP_ERROR_INCORRECT_STATE;
        }

        jint size = env->GetArrayLength(inputArray);
        for (int i = 0; i < size; i++)
        {
            app::Clusters::MediaInput::Structs::MediaInputInfo::Type mediaInput;

            jobject inputObj  = env->GetObjectArrayElement(inputArray, i);
            jclass inputClass = env->GetObjectClass(inputObj);

            jfieldID indexId = env->GetFieldID(inputClass, "index", "I");
            jint index       = env->GetIntField(inputObj, indexId);
            mediaInput.index = static_cast<uint8_t>(index);

            jfieldID typeId      = env->GetFieldID(inputClass, "type", "I");
            jint type            = env->GetIntField(inputObj, typeId);
            mediaInput.inputType = static_cast<app::Clusters::MediaInput::MediaInputType>(type);

            jfieldID nameId = env->GetFieldID(inputClass, "name", "Ljava/lang/String;");
            jstring jname   = static_cast<jstring>(env->GetObjectField(inputObj, nameId));

            if (jname != NULL)
            {
                JniUtfString name(env, jname);
                mediaInput.name = name.charSpan();
            }

            jfieldID descriptionId = env->GetFieldID(inputClass, "description", "Ljava/lang/String;");
            jstring jdescription   = static_cast<jstring>(env->GetObjectField(inputObj, descriptionId));

            if (jdescription != NULL)
            {
                JniUtfString description(env, jdescription);
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

CHIP_ERROR MediaInputManager::GetCurrentInput(chip::app::AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jint index     = -1;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received MediaInputManager::GetInputList");
    VerifyOrExit(mMediaInputManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetCurrentInputMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    index = env->CallIntMethod(mMediaInputManagerObject, mGetCurrentInputMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in MediaInputManager::GetCurrentInput");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(Zcl, "GetCurrentInput = %d", index);
    if (index >= 0)
    {
        err = aEncoder.Encode(uint8_t(index));
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "MediaInputManager::GetCurrentInput status error: %s", err.AsString());
    }

    return err;
}

bool MediaInputManager::SelectInput(uint8_t index)
{
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received MediaInputManager::SelectInput %d", index);
    VerifyOrExit(mMediaInputManagerObject != nullptr, ChipLogError(Zcl, "mMediaInputManagerObject null"));
    VerifyOrExit(mSelectInputMethod != nullptr, ChipLogError(Zcl, "mSelectInputMethod null"));
    VerifyOrExit(env != NULL, ChipLogError(Zcl, "env null"));

    env->ExceptionClear();
    ret = env->CallBooleanMethod(mMediaInputManagerObject, mSelectInputMethod, static_cast<jint>(index));
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in MediaInputManager::selectInput");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

exit:
    return static_cast<bool>(ret);
}

bool MediaInputManager::ShowInputStatus()
{
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received MediaInputManager::ShowInputStatus");
    VerifyOrExit(mMediaInputManagerObject != nullptr, ChipLogError(Zcl, "mMediaInputManagerObject null"));
    VerifyOrExit(mShowInputStatusMethod != nullptr, ChipLogError(Zcl, "mShowInputStatusMethod null"));
    VerifyOrExit(env != NULL, ChipLogError(Zcl, "env null"));

    env->ExceptionClear();
    ret = env->CallBooleanMethod(mMediaInputManagerObject, mShowInputStatusMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in MediaInputManager::showInputStatus");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

exit:
    return static_cast<bool>(ret);
}

bool MediaInputManager::HideInputStatus()
{
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received MediaInputManager::HideInputStatus");
    VerifyOrExit(mMediaInputManagerObject != nullptr, ChipLogError(Zcl, "mMediaInputManagerObject null"));
    VerifyOrExit(mHideInputStatusMethod != nullptr, ChipLogError(Zcl, "mHideInputStatusMethod null"));
    VerifyOrExit(env != NULL, ChipLogError(Zcl, "env null"));

    env->ExceptionClear();
    ret = env->CallBooleanMethod(mMediaInputManagerObject, mHideInputStatusMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in MediaInputManager::HideInputStatus");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

exit:
    return static_cast<bool>(ret);
}

bool MediaInputManager::RenameInput(uint8_t index, std::string name)
{
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received MediaInputManager::RenameInput %d to %s", index, name.c_str());
    VerifyOrExit(mMediaInputManagerObject != nullptr, ChipLogError(Zcl, "mMediaInputManagerObject null"));
    VerifyOrExit(mRenameInputMethod != nullptr, ChipLogError(Zcl, "mHideInputStatusMethod null"));
    VerifyOrExit(env != NULL, ChipLogError(Zcl, "env null"));

    {
        UtfString jniInputname(env, name.c_str());
        env->ExceptionClear();
        ret =
            env->CallBooleanMethod(mMediaInputManagerObject, mRenameInputMethod, static_cast<jint>(index), jniInputname.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(DeviceLayer, "Java exception in MediaInputManager::RenameInput");
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

    mMediaInputManagerObject = env->NewGlobalRef(managerObject);
    VerifyOrReturn(mMediaInputManagerObject != nullptr, ChipLogError(Zcl, "Failed to NewGlobalRef MediaInputManager"));

    jclass MediaInputManagerClass = env->GetObjectClass(managerObject);
    VerifyOrReturn(MediaInputManagerClass != nullptr, ChipLogError(Zcl, "Failed to get MediaInputManager Java class"));

    mGetInputListMethod = env->GetMethodID(MediaInputManagerClass, "getInputList", "()[Lcom/tcl/chip/tvapp/MediaInputInfo;");
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
