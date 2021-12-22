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

#include "ContentLauncherManager.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app/util/attribute-storage.h>

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <cstddef>
#include <cstdint>
#include <jni.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <map>

using namespace std;
using namespace chip;

ContentLauncherManager ContentLauncherManager::sInstance;

LaunchResponse ContentLauncherManager::HandleLaunchContent(chip::EndpointId endpointId, const std::list<Parameter> & parameterList,
                                                           bool autoplay, const chip::CharSpan & data)
{
    LaunchResponse response;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received ContentLauncherManager::LaunchContent");
    VerifyOrExit(mContentLauncherManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mLaunchContentMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    {
        UtfString jData(env, data);

        // Todo: make parameterList java
        jobjectArray parameterArray = nullptr;

        jobject resp =
            env->CallObjectMethod(mContentLauncherManagerObject, mLaunchContentMethod, parameterArray, autoplay, jData.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ContentLauncherManager::LaunchContent");
            env->ExceptionDescribe();
            env->ExceptionClear();
            err = CHIP_ERROR_INCORRECT_STATE;
            goto exit;
        }

        VerifyOrExit(resp != nullptr, err = CHIP_JNI_ERROR_NULL_OBJECT);
        jclass respCls     = env->GetObjectClass(resp);
        jfieldID statusFid = env->GetFieldID(respCls, "status", "I");
        VerifyOrExit(statusFid != nullptr, err = CHIP_JNI_ERROR_FIELD_NOT_FOUND);
        jint status = env->GetIntField(resp, statusFid);

        jfieldID dataFid = env->GetFieldID(respCls, "data", "Ljava/lang/String;");
        VerifyOrExit(dataFid != nullptr, err = CHIP_JNI_ERROR_FIELD_NOT_FOUND);
        jstring jdataStr = (jstring) env->GetObjectField(resp, dataFid);
        JniUtfString dataStr(env, jdataStr);

        response.status = static_cast<chip::app::Clusters::ContentLauncher::StatusEnum>(status);
        response.data   = dataStr.charSpan();
    }

exit:
    response.err = err;
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ContentLauncherManager::LaunchContent status error: %s", err.AsString());
    }

    return response;
}

LaunchResponse ContentLauncherManager::HandleLaunchUrl(const chip::CharSpan & contentUrl, const chip::CharSpan & displayString,
                                                       const std::list<BrandingInformation> & brandingInformation)
{
    LaunchResponse response;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received ContentLauncherManager::LaunchContent");
    VerifyOrExit(mContentLauncherManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mLaunchUrlMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    {
        UtfString jContentUrl(env, contentUrl);
        UtfString jDisplayString(env, displayString);

        // Todo: make brandingInformation java
        jobjectArray branding = nullptr;

        jobject resp = env->CallObjectMethod(mContentLauncherManagerObject, mLaunchUrlMethod, jContentUrl.jniValue(),
                                             jDisplayString.jniValue(), branding);
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ContentLauncherManager::LaunchUrl");
            env->ExceptionDescribe();
            env->ExceptionClear();
            err = CHIP_ERROR_INCORRECT_STATE;
            goto exit;
        }

        VerifyOrExit(resp != nullptr, err = CHIP_JNI_ERROR_NULL_OBJECT);
        jclass respCls     = env->GetObjectClass(resp);
        jfieldID statusFid = env->GetFieldID(respCls, "status", "I");
        VerifyOrExit(statusFid != nullptr, err = CHIP_JNI_ERROR_FIELD_NOT_FOUND);
        jint status = env->GetIntField(resp, statusFid);

        jfieldID dataFid = env->GetFieldID(respCls, "data", "Ljava/lang/String;");
        VerifyOrExit(dataFid != nullptr, err = CHIP_JNI_ERROR_FIELD_NOT_FOUND);
        jstring jdataStr = (jstring) env->GetObjectField(resp, dataFid);
        JniUtfString dataStr(env, jdataStr);

        response.status = static_cast<chip::app::Clusters::ContentLauncher::StatusEnum>(status);
        response.data   = dataStr.charSpan();
    }

exit:
    response.err = err;
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ContentLauncherManager::LaunchUrl status error: %s", err.AsString());
    }

    return response;
}

std::list<std::string> ContentLauncherManager::HandleGetAcceptHeaderList()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    std::list<std::string> acceptedHeadersList;

    ChipLogProgress(Zcl, "Received ContentLauncherManager::GetAcceptHeader");
    VerifyOrExit(mContentLauncherManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetAcceptHeaderMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    {
        jobjectArray acceptedHeadersArray =
            (jobjectArray) env->CallObjectMethod(mContentLauncherManagerObject, mGetAcceptHeaderMethod);
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ContentLauncherManager::GetAcceptHeader");
            env->ExceptionDescribe();
            env->ExceptionClear();
            err = CHIP_ERROR_INCORRECT_STATE;
            goto exit;
        }

        jint size = env->GetArrayLength(acceptedHeadersArray);
        for (int i = 0; i < size; i++)
        {

            jstring jAcceptedHeader     = (jstring) env->GetObjectArrayElement(acceptedHeadersArray, i);
            const char * convertedValue = (env)->GetStringUTFChars(jAcceptedHeader, JNI_FALSE);
            std::string acceptedHeader  = std::string(convertedValue, strlen(convertedValue));
            acceptedHeadersList.push_front(acceptedHeader);
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ContentLauncherManager::GetAcceptHeader status error: %s", err.AsString());
    }

    return acceptedHeadersList;
}

uint32_t ContentLauncherManager::HandleGetSupportedStreamingProtocols()
{
    CHIP_ERROR err                       = CHIP_NO_ERROR;
    JNIEnv * env                         = JniReferences::GetInstance().GetEnvForCurrentThread();
    uint32_t supportedStreamingProtocols = 0;

    ChipLogProgress(Zcl, "Received ContentLauncherManager::GetSupportedStreamingProtocols");
    VerifyOrExit(mContentLauncherManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetSupportedStreamingProtocolsMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    {
        jint jSupportedStreamingProtocols =
            env->CallIntMethod(mContentLauncherManagerObject, mGetSupportedStreamingProtocolsMethod);
        supportedStreamingProtocols = (uint32_t) jSupportedStreamingProtocols;
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ContentLauncherManager::GetAcceptHeader");
            env->ExceptionDescribe();
            env->ExceptionClear();
            err = CHIP_ERROR_INCORRECT_STATE;
            goto exit;
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ContentLauncherManager::GetSupportedStreamingProtocols status error: %s", err.AsString());
    }

    return supportedStreamingProtocols;
}

void ContentLauncherManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for ContentLauncherManager"));

    mContentLauncherManagerObject = env->NewGlobalRef(managerObject);
    VerifyOrReturn(mContentLauncherManagerObject != nullptr, ChipLogError(Zcl, "Failed to NewGlobalRef ContentLauncherManager"));

    jclass ContentLauncherClass = env->GetObjectClass(managerObject);
    VerifyOrReturn(ContentLauncherClass != nullptr, ChipLogError(Zcl, "Failed to get ContentLauncherManager Java class"));

    mGetAcceptHeaderMethod = env->GetMethodID(ContentLauncherClass, "getAcceptHeader", "()[Ljava/lang/String;");
    if (mGetAcceptHeaderMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaInputManager 'getInputList' method");
        env->ExceptionClear();
    }

    mGetSupportedStreamingProtocolsMethod = env->GetMethodID(ContentLauncherClass, "getSupportedStreamingProtocols", "()[I");
    if (mGetSupportedStreamingProtocolsMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaInputManager 'getSupportedStreamingProtocols' method");
        env->ExceptionClear();
    }

    mLaunchContentMethod = env->GetMethodID(
        ContentLauncherClass, "launchContent",
        "([Lcom/tcl/chip/tvapp/ContentLaunchSearchParameter;ZLjava/lang/String;)Lcom/tcl/chip/tvapp/LaunchResponse;");
    if (mLaunchContentMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaInputManager 'launchContent' method");
        env->ExceptionClear();
    }

    mLaunchUrlMethod = env->GetMethodID(ContentLauncherClass, "launchUrl",
                                        "(Ljava/lang/String;Ljava/lang/String;Lcom/tcl/chip/tvapp/"
                                        "ContentLaunchBrandingInformation;)Lcom/tcl/chip/tvapp/LaunchResponse;");
    if (mLaunchUrlMethod == nullptr)
    {
        ChipLogError(AppServer, "Failed to access 'launchUrl' method");
        env->ExceptionClear();
    }
}
