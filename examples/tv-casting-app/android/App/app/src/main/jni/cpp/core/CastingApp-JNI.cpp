/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "CastingApp-JNI.h"

#include "../JNIDACProvider.h"
#include "../support/Converters-JNI.h"
#include "../support/RotatingDeviceIdUniqueIdProvider-JNI.h"

// from tv-casting-common
#include "core/CastingApp.h"
#include "support/ChipDeviceEventHandler.h"

#include <app/clusters/bindings/BindingManager.h>
#include <app/server/Server.h>
#include <jni.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_com_matter_casting_core_CastingApp_##METHOD_NAME

namespace matter {
namespace casting {
namespace core {

CastingAppJNI CastingAppJNI::sInstance;

jobject extractJAppParameter(jobject jAppParameters, const char * methodName, const char * methodSig);

JNI_METHOD(jobject, finishInitialization)(JNIEnv *, jobject, jobject jAppParameters)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "JNI_METHOD CastingApp-JNI::finishInitialization() called");
    VerifyOrReturnValue(jAppParameters != nullptr, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INVALID_ARGUMENT));
    CHIP_ERROR err = CHIP_NO_ERROR;

    jobject jUniqueIdProvider =
        extractJAppParameter(jAppParameters, "getRotatingDeviceIdUniqueIdProvider", "()Lcom/matter/casting/support/DataProvider;");
    VerifyOrReturnValue(jUniqueIdProvider != nullptr, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INCORRECT_STATE));
    support::RotatingDeviceIdUniqueIdProviderJNI * uniqueIdProvider = new support::RotatingDeviceIdUniqueIdProviderJNI();
    err                                                             = uniqueIdProvider->Initialize(jUniqueIdProvider);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INVALID_ARGUMENT));

    // set the RotatingDeviceIdUniqueId
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    chip::MutableByteSpan * uniqueId = uniqueIdProvider->Get();
    if (uniqueId != nullptr)
    {
        chip::DeviceLayer::ConfigurationMgr().SetRotatingDeviceIdUniqueId(*uniqueId);
    }
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID

    // get the DACProvider
    jobject jDACProvider = extractJAppParameter(jAppParameters, "getDacProvider", "()Lcom/matter/casting/support/DACProvider;");
    VerifyOrReturnValue(jDACProvider != nullptr, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INCORRECT_STATE));

    // set the DACProvider
    JNIDACProvider * dacProvider = new JNIDACProvider(jDACProvider);
    chip::Credentials::SetDeviceAttestationCredentialsProvider(dacProvider);

    return support::convertMatterErrorFromCppToJava(CHIP_NO_ERROR);
}

JNI_METHOD(jobject, finishStartup)(JNIEnv *, jobject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "JNI_METHOD CastingAppJNI::finishStartup() called");

    CHIP_ERROR err = CHIP_NO_ERROR;
    auto & server  = chip::Server::GetInstance();

    // TODO: Set AppDelegate
    // &server.GetCommissioningWindowManager().SetAppDelegate(??);

    // Initialize binding handlers
    err = chip::BindingManager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });
    VerifyOrReturnValue(err == CHIP_NO_ERROR, support::convertMatterErrorFromCppToJava(err),
                        ChipLogError(AppServer, "Failed to init BindingManager %" CHIP_ERROR_FORMAT, err.Format()));

    // TODO: Set FabricDelegate
    // chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&mPersistenceManager);

    err = chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(support::ChipDeviceEventHandler::Handle, 0);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, support::convertMatterErrorFromCppToJava(err),
                        ChipLogError(AppServer, "Failed to register ChipDeviceEventHandler %" CHIP_ERROR_FORMAT, err.Format()));

    return support::convertMatterErrorFromCppToJava(CHIP_NO_ERROR);
}

JNI_METHOD(jobject, shutdownAllSubscriptions)(JNIEnv * env, jobject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "JNI_METHOD CastingApp-JNI::shutdownAllSubscriptions called");

    CHIP_ERROR err = matter::casting::core::CastingApp::GetInstance()->ShutdownAllSubscriptions();
    return support::convertMatterErrorFromCppToJava(err);
}

JNI_METHOD(jobject, clearCache)(JNIEnv * env, jobject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "JNI_METHOD CastingApp-JNI::clearCache called");

    CHIP_ERROR err = matter::casting::core::CastingApp::GetInstance()->ClearCache();
    return support::convertMatterErrorFromCppToJava(err);
}

jobject extractJAppParameter(jobject jAppParameters, const char * methodName, const char * methodSig)
{
    ChipLogProgress(AppServer, "JNI_METHOD CastingApp-JNI::extractJAppParameter() called");
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();

    jclass jAppParametersClass;
    CHIP_ERROR err =
        chip::JniReferences::GetInstance().GetLocalClassRef(env, "com/matter/casting/support/AppParameters", jAppParametersClass);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr);

    // get the RotatingDeviceIdUniqueIdProvider
    jmethodID getMethod = env->GetMethodID(jAppParametersClass, methodName, methodSig);
    if (env->ExceptionCheck())
    {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }

    jobject jParameter = (jobject) env->CallObjectMethod(jAppParameters, getMethod);
    if (env->ExceptionCheck())
    {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }

    return jParameter;
}

}; // namespace core
}; // namespace casting
}; // namespace matter
