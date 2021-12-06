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

#include <app/Command.h>
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

namespace {

class ContentLauncherAttrAccess : public app::AttributeAccessInterface
{
public:
    ContentLauncherAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), app::Clusters::ContentLauncher::Id)
    {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override
    {
        if (aPath.mAttributeId == app::Clusters::ContentLauncher::Attributes::AcceptsHeaderList::Id)
        {
            return ContentLauncherMgr().GetAcceptsHeader(aEncoder);
        }
        else if (aPath.mAttributeId == app::Clusters::ContentLauncher::Attributes::SupportedStreamingTypes::Id)
        {
            return ContentLauncherMgr().GetSupportedStreamingTypes(aEncoder);
        }

        return CHIP_NO_ERROR;
    }
};

ContentLauncherAttrAccess gContentLauncherAttrAccess;

} // anonymous namespace

/** @brief Content Launch Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfContentLauncherClusterInitCallback(EndpointId endpoint)
{
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gContentLauncherAttrAccess);
        attrAccessRegistered = true;
    }
}

ContentLaunchResponse contentLauncherClusterLaunchContent(std::list<ContentLaunchParamater> parameterList, bool autoplay,
                                                          const chip::CharSpan & data)
{
    return ContentLauncherMgr().LaunchContent(parameterList, autoplay, data);
}

ContentLaunchResponse contentLauncherClusterLaunchUrl(const chip::CharSpan & contentUrl, const chip::CharSpan & displayString,
                                                      ContentLaunchBrandingInformation & brandingInformation)
{
    return ContentLauncherMgr().LaunchUrl(contentUrl, displayString, brandingInformation);
}

void ContentLauncherManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for ContentLauncherManager"));

    mContentLauncherManagerObject = env->NewGlobalRef(managerObject);
    VerifyOrReturn(mContentLauncherManagerObject != nullptr, ChipLogError(Zcl, "Failed to NewGlobalRef ContentLauncherManager"));

    jclass ContentLauncherClass = env->GetObjectClass(managerObject);
    VerifyOrReturn(ContentLauncherClass != nullptr, ChipLogError(Zcl, "Failed to get ContentLauncherManager Java class"));

    mGetAcceptsHeaderMethod = env->GetMethodID(ContentLauncherClass, "getAcceptsHeader", "()[Ljava/lang/String;");
    if (mGetAcceptsHeaderMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaInputManager 'getInputList' method");
        env->ExceptionClear();
    }

    mGetSupportedStreamingTypesMethod = env->GetMethodID(ContentLauncherClass, "getSupportedStreamingTypes", "()[I");
    if (mGetSupportedStreamingTypesMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaInputManager 'getSupportedStreamingTypes' method");
        env->ExceptionClear();
    }

    mLaunchContentMethod = env->GetMethodID(
        ContentLauncherClass, "launchContent",
        "([Lcom/tcl/chip/tvapp/ContentLaunchSearchParameter;ZLjava/lang/String;)Lcom/tcl/chip/tvapp/ContentLaunchResponse;");
    if (mLaunchContentMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaInputManager 'launchContent' method");
        env->ExceptionClear();
    }

    mLaunchUrlMethod = env->GetMethodID(ContentLauncherClass, "launchUrl",
                                        "(Ljava/lang/String;Ljava/lang/String;Lcom/tcl/chip/tvapp/"
                                        "ContentLaunchBrandingInformation;)Lcom/tcl/chip/tvapp/ContentLaunchResponse;");
    if (mLaunchUrlMethod == nullptr)
    {
        ChipLogError(AppServer, "Failed to access 'launchUrl' method");
        env->ExceptionClear();
    }
}

CHIP_ERROR ContentLauncherManager::GetAcceptsHeader(chip::app::AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received ContentLauncherManager::GetAcceptsHeader");
    VerifyOrExit(mContentLauncherManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetAcceptsHeaderMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    return aEncoder.EncodeList([this, env](const auto & encoder) -> CHIP_ERROR {
        jobjectArray headersArray = (jobjectArray) env->CallObjectMethod(mContentLauncherManagerObject, mGetAcceptsHeaderMethod);
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ContentLauncherManager::GetAcceptsHeader");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return CHIP_ERROR_INCORRECT_STATE;
        }

        jint size = env->GetArrayLength(headersArray);
        for (int i = 0; i < size; i++)
        {
            jstring acceptsheader = (jstring) env->GetObjectArrayElement(headersArray, i);
            if (acceptsheader != nullptr)
            {
                JniUtfString header(env, acceptsheader);

                chip::ByteSpan bHeader((const uint8_t *) (header.c_str()), (size_t)(header.size()));
                ReturnErrorOnFailure(encoder.Encode(bHeader));

                // Todo: should be chanSpan?
                // ReturnErrorOnFailure(encoder.Encode(header.charSpan()));
            }
        }

        return CHIP_NO_ERROR;
    });

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ContentLauncherManager::GetAcceptsHeader status error: %s", err.AsString());
    }

    return err;
}

CHIP_ERROR ContentLauncherManager::GetSupportedStreamingTypes(chip::app::AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received ContentLauncherManager::GetSupportedStreamingTypes");
    VerifyOrExit(mContentLauncherManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetSupportedStreamingTypesMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    return aEncoder.EncodeList([this, env](const auto & encoder) -> CHIP_ERROR {
        jintArray typesArray = (jintArray) env->CallObjectMethod(mContentLauncherManagerObject, mGetSupportedStreamingTypesMethod);
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ContentLauncherManager::GetSupportedStreamingTypes");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return CHIP_ERROR_INCORRECT_STATE;
        }

        jboolean isCopy = JNI_FALSE;
        jint * ptypes   = env->GetIntArrayElements(typesArray, &isCopy);
        jint size       = env->GetArrayLength(typesArray);

        CHIP_ERROR err = CHIP_NO_ERROR;
        for (int i = 0; i < size; i++)
        {
            err = encoder.Encode(static_cast<uint8_t>(ptypes[i]));
            if (err != CHIP_NO_ERROR)
            {
                break;
            }
        }
        env->ReleaseIntArrayElements(typesArray, ptypes, 0);
        return err;
    });

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ContentLauncherManager::GetAcceptsHeader status error: %s", err.AsString());
    }

    return err;
}

ContentLaunchResponse ContentLauncherManager::LaunchContent(std::list<ContentLaunchParamater> parameterList, bool autoplay,
                                                            const chip::CharSpan & data)
{
    ContentLaunchResponse response;
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

        response.status = static_cast<chip::app::Clusters::ContentLauncher::ContentLaunchStatus>(status);
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

ContentLaunchResponse ContentLauncherManager::LaunchUrl(const chip::CharSpan & contentUrl, const chip::CharSpan & displayString,
                                                        ContentLaunchBrandingInformation & brandingInformation)
{
    ContentLaunchResponse response;
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

        response.status = static_cast<chip::app::Clusters::ContentLauncher::ContentLaunchStatus>(status);
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
