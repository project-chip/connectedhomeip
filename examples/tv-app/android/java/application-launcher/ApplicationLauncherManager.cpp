/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "ApplicationLauncherManager.h"
#include "../TvApp-JNI.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/config.h>
#include <jni.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ApplicationLauncher;
using namespace chip::Uint8;

void emberAfApplicationLauncherClusterInitCallback(chip::EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Android App: ApplicationLauncher::PostClusterInit");
    if (endpoint > kLocalVideoPlayerEndpointId)
    {
        ChipLogProgress(Zcl, "TV Android App: ignore setting the delegate for endpoints larger than 1");
        return;
    }
    TvAppJNIMgr().PostClusterInit(chip::app::Clusters::ApplicationLauncher::Id, endpoint);
}

void ApplicationLauncherManager::NewManager(jint endpoint, jobject manager)
{
    if (endpoint > kLocalVideoPlayerEndpointId)
    {
        ChipLogProgress(Zcl, "TV Android App: ignore setting the delegate for endpoints larger than 1");
        return;
    }
    ChipLogProgress(Zcl, "TV Android App: ApplicationLauncher::SetDefaultDelegate for endpoint: %d", endpoint);
    ApplicationLauncherManager * mgr = new ApplicationLauncherManager();
    mgr->InitializeWithObjects(manager);
    chip::app::Clusters::ApplicationLauncher::SetDefaultDelegate(static_cast<chip::EndpointId>(endpoint), mgr);
}

CHIP_ERROR ApplicationLauncherManager::HandleGetCatalogList(AttributeValueEncoder & aEncoder)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NO_ENV, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    chip::JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received ApplicationLauncherManager::GetCatalogList");
    VerifyOrExit(mApplicationLauncherManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetCatalogListMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    return aEncoder.EncodeList([this, env](const auto & encoder) -> CHIP_ERROR {
        jintArray jCatalogList =
            (jintArray) env->CallObjectMethod(mApplicationLauncherManagerObject.ObjectRef(), mGetCatalogListMethod);
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ApplicationLauncherManager::GetCatalogList");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return CHIP_ERROR_INCORRECT_STATE;
        }

        jint size       = env->GetArrayLength(jCatalogList);
        jint * elements = env->GetIntArrayElements(jCatalogList, 0);
        for (int i = 0; i < size; i++)
        {
            jint jCatalogVendorId = elements[i];
            ReturnErrorOnFailure(encoder.Encode(static_cast<uint16_t>(jCatalogVendorId)));
        }

        return CHIP_NO_ERROR;
    });

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ApplicationLauncherManager::GetCatalogList status error: %s", err.AsString());
    }

    return err;
}

void ApplicationLauncherManager::HandleLaunchApp(CommandResponseHelper<LauncherResponseType> & helper, const ByteSpan & data,
                                                 const ApplicationType & application)
{
    chip::DeviceLayer::StackUnlock unlock;
    LauncherResponseType response;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    chip::JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received ApplicationLauncherManager::LaunchApp");
    VerifyOrExit(mApplicationLauncherManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mLaunchAppMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    {
        // UtfString accepts const char * data
        chip::UtfString jByteData(env, reinterpret_cast<const char *>(data.data()));

        chip::UtfString jappId(env, application.applicationID);

        // Create an instance of Application
        jobject appObject = env->NewObject(mApplicationClass, mCreateApplicationMethod,
                                           static_cast<jint>(application.catalogVendorID), jappId.jniValue());
        VerifyOrReturn(appObject != nullptr, ChipLogError(Zcl, "Failed to create Application object"));

        jobject resp =
            env->CallObjectMethod(mApplicationLauncherManagerObject.ObjectRef(), mLaunchAppMethod, appObject, jByteData.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ApplicationLauncherManager::LaunchApp");
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
        chip::JniUtfString dataStr(env, jdataStr);

        response.status = static_cast<chip::app::Clusters::ApplicationLauncher::StatusEnum>(status);
        response.data   = chip::Optional<chip::ByteSpan>(dataStr.byteSpan());

        err = helper.Success(response);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ApplicationLauncherManager::LaunchApp status error: %s", err.AsString());
    }
}

void ApplicationLauncherManager::HandleStopApp(CommandResponseHelper<LauncherResponseType> & helper,
                                               const ApplicationType & application)
{
    chip::DeviceLayer::StackUnlock unlock;
    LauncherResponseType response;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    chip::JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received ApplicationLauncherManager::StopApp");
    VerifyOrExit(mApplicationLauncherManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mStopAppMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    {
        chip::UtfString jappId(env, application.applicationID);

        // Create an instance of Application
        jobject appObject = env->NewObject(mApplicationClass, mCreateApplicationMethod,
                                           static_cast<jint>(application.catalogVendorID), jappId.jniValue());
        VerifyOrReturn(appObject != nullptr, ChipLogError(Zcl, "Failed to create Application object"));

        jobject resp = env->CallObjectMethod(mApplicationLauncherManagerObject.ObjectRef(), mStopAppMethod, appObject);
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ApplicationLauncherManager::StopApp");
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
        chip::JniUtfString dataStr(env, jdataStr);

        response.status = static_cast<chip::app::Clusters::ApplicationLauncher::StatusEnum>(status);
        response.data   = chip::Optional<chip::ByteSpan>(dataStr.byteSpan());

        err = helper.Success(response);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ApplicationLauncherManager::StopApp status error: %s", err.AsString());
    }
}

void ApplicationLauncherManager::HandleHideApp(CommandResponseHelper<LauncherResponseType> & helper,
                                               const ApplicationType & application)
{
    chip::DeviceLayer::StackUnlock unlock;
    LauncherResponseType response;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    chip::JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received ApplicationLauncherManager::HideApp");
    VerifyOrExit(mApplicationLauncherManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mHideAppMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    {
        chip::UtfString jappId(env, application.applicationID);

        // Create an instance of Application
        jobject appObject = env->NewObject(mApplicationClass, mCreateApplicationMethod,
                                           static_cast<jint>(application.catalogVendorID), jappId.jniValue());
        VerifyOrReturn(appObject != nullptr, ChipLogError(Zcl, "Failed to create Application object"));

        jobject resp = env->CallObjectMethod(mApplicationLauncherManagerObject.ObjectRef(), mHideAppMethod, appObject);
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ApplicationLauncherManager::HideApp");
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
        chip::JniUtfString dataStr(env, jdataStr);

        response.status = static_cast<chip::app::Clusters::ApplicationLauncher::StatusEnum>(status);
        response.data   = chip::Optional<chip::ByteSpan>(dataStr.byteSpan());

        err = helper.Success(response);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ApplicationLauncherManager::HideApp status error: %s", err.AsString());
    }
}

void ApplicationLauncherManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for ApplicationLauncherManager"));

    VerifyOrReturn(mApplicationLauncherManagerObject.Init(managerObject) == CHIP_NO_ERROR,
                   ChipLogError(Zcl, "Failed to init mApplicationLauncherManagerObject"));

    jclass applicationLauncherClass = env->GetObjectClass(managerObject);
    VerifyOrReturn(applicationLauncherClass != nullptr, ChipLogError(Zcl, "Failed to get ApplicationLauncherManager Java class"));

    mGetCatalogListMethod = env->GetMethodID(applicationLauncherClass, "getCatalogList", "()[I");
    if (mGetCatalogListMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ApplicationLauncherManager 'getCatalogList' method");
        env->ExceptionClear();
    }

    mLaunchAppMethod = env->GetMethodID(applicationLauncherClass, "launchApp",
                                        "(Lcom/matter/tv/server/tvapp/"
                                        "Application;Ljava/lang/String;)Lcom/matter/tv/server/tvapp/LauncherResponse;");
    if (mLaunchAppMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ApplicationLauncherManager 'launchApp' method");
        env->ExceptionClear();
    }

    mStopAppMethod = env->GetMethodID(applicationLauncherClass, "stopApp",
                                      "(Lcom/matter/tv/server/tvapp/"
                                      "Application;)Lcom/matter/tv/server/tvapp/LauncherResponse;");
    if (mStopAppMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ApplicationLauncherManager 'stopApp' method");
        env->ExceptionClear();
    }

    mHideAppMethod = env->GetMethodID(applicationLauncherClass, "hideApp",
                                      "(Lcom/matter/tv/server/tvapp/"
                                      "Application;)Lcom/matter/tv/server/tvapp/LauncherResponse;");
    if (mHideAppMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ApplicationLauncherManager 'hideApp' method");
        env->ExceptionClear();
    }

    // Find the Application class
    jclass jc = env->FindClass("com/matter/tv/server/tvapp/Application");
    // convert it to a global reference, otherwise code will crash
    mApplicationClass = static_cast<jclass>(env->NewGlobalRef(jc));
    if (mApplicationClass == nullptr)
    {
        ChipLogError(Zcl, "Failed to find Application Java class");
        env->ExceptionClear();
    }

    // Get the constructor method ID
    mCreateApplicationMethod = env->GetMethodID(mApplicationClass, "<init>", "(ILjava/lang/String;)V");
    if (mCreateApplicationMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to find constructor of Application Java class");
        env->ExceptionClear();
    }
}
