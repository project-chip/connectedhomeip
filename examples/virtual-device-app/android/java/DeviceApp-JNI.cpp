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

#include "DeviceApp-JNI.h"
#include "AppImpl.h"
#include "JNIDACProvider.h"

#include "OnOffManager.h"
#include "credentials/DeviceAttestationCredsProvider.h"
#include <app/app-platform/ContentAppPlatform.h>
#include <app/server/Dnssd.h>
#include <app/server/java/AndroidAppServerWrapper.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <zap-generated/CHIPClusters.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::AppPlatform;
using namespace chip::Credentials;
using namespace chip::DeviceLayer;

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_com_matter_virtual_device_app_DeviceApp_##METHOD_NAME

#define DEVICE_VERSION_DEFAULT 1

EmberAfDeviceType gDeviceTypeIds[] = { { 0, DEVICE_VERSION_DEFAULT } };
DeviceAppJNI DeviceAppJNI::sInstance;

void DeviceAppJNI::InitializeWithObjects(jobject app)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for DeviceAppJNI"));

    mDeviceAppObject = env->NewGlobalRef(app);
    VerifyOrReturn(mDeviceAppObject != nullptr, ChipLogError(Zcl, "Failed to NewGlobalRef DeviceAppJNI"));

    jclass managerClass = env->GetObjectClass(mDeviceAppObject);
    VerifyOrReturn(managerClass != nullptr, ChipLogError(Zcl, "Failed to get DeviceAppJNI Java class"));

    mPostClusterInitMethod = env->GetMethodID(managerClass, "postClusterInit", "(II)V");
    if (mPostClusterInitMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access DeviceApp 'postClusterInit' method");
        env->ExceptionClear();
    }

    mPostEventMethod = env->GetMethodID(managerClass, "postEvent", "(I)V");
    if (mPostEventMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access DeviceApp 'postEvent' method");
        env->ExceptionClear();
    }
}

void DeviceAppJNI::PostClusterInit(int clusterId, int endpoint)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for DeviceAppJNI::PostClusterInit"));
    VerifyOrReturn(mDeviceAppObject != nullptr, ChipLogError(Zcl, "DeviceAppJNI::mDeviceAppObject null"));
    VerifyOrReturn(mPostClusterInitMethod != nullptr, ChipLogError(Zcl, "DeviceAppJNI::mPostClusterInitMethod null"));

    env->CallVoidMethod(mDeviceAppObject, mPostClusterInitMethod, static_cast<jint>(clusterId), static_cast<jint>(endpoint));
    if (env->ExceptionCheck())
    {
        ChipLogError(Zcl, "Failed to call DeviceAppJNI 'postClusterInit' method");
        env->ExceptionClear();
    }
}

void DeviceAppJNI::PostEvent(int event)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for DeviceAppJNI::PostEvent"));
    VerifyOrReturn(mDeviceAppObject != nullptr, ChipLogError(Zcl, "DeviceAppJNI::mDeviceAppObject null"));
    VerifyOrReturn(mPostEventMethod != nullptr, ChipLogError(Zcl, "DeviceAppJNI::mPostEventMethod null"));

    env->CallVoidMethod(mDeviceAppObject, mPostEventMethod, static_cast<jint>(event));
    if (env->ExceptionCheck())
    {
        ChipLogError(Zcl, "Failed to call DeviceAppJNI 'postEventMethod' method");
        env->ExceptionClear();
    }
}

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    return AndroidAppServerJNI_OnLoad(jvm, reserved);
}

void JNI_OnUnload(JavaVM * jvm, void * reserved)
{
    return AndroidAppServerJNI_OnUnload(jvm, reserved);
}

JNI_METHOD(void, nativeInit)(JNIEnv *, jobject app)
{
    DeviceAppJNIMgr().InitializeWithObjects(app);
}

JNI_METHOD(void, preServerInit)(JNIEnv *, jobject app)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(Zcl, "DeviceAppJNI::preServerInit");

    PreServerInit();
}

JNI_METHOD(void, postServerInit)(JNIEnv *, jobject app, jint deviceTypeId)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(Zcl, "DeviceAppJNI::postServerInit");

    gDeviceTypeIds[0].deviceId = static_cast<uint16_t>(deviceTypeId);
    emberAfSetDeviceTypeList(1, Span<const EmberAfDeviceType>(gDeviceTypeIds));
}

JNI_METHOD(void, setDACProvider)(JNIEnv *, jobject, jobject provider)
{
    if (!chip::Credentials::IsDeviceAttestationCredentialsProviderSet())
    {
        JNIDACProvider * p = new JNIDACProvider(provider);
        chip::Credentials::SetDeviceAttestationCredentialsProvider(p);
    }
}

/*
 * On Off Manager
 */
JNI_METHOD(void, setOnOffManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    OnOffManager::NewManager(endpoint, manager);
}

JNI_METHOD(jboolean, setOnOff)(JNIEnv *, jobject, jint endpoint, jboolean value)
{
    return DeviceLayer::SystemLayer().ScheduleLambda([endpoint, value] { OnOffManager::SetOnOff(endpoint, value); }) ==
        CHIP_NO_ERROR;
}
