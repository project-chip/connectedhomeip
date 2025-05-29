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

#include "ColorControlManager.h"
#include "DoorLockManager.h"
#include "OnOffManager.h"
#include "PowerSourceManager.h"
#include "credentials/DeviceAttestationCredsProvider.h"
#include <app/app-platform/ContentAppPlatform.h>
#include <app/server/Dnssd.h>
#include <app/server/java/AndroidAppServerWrapper.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

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

    VerifyOrReturn(mDeviceAppObject.Init(app) == CHIP_NO_ERROR, ChipLogError(Zcl, "Failed to init mDeviceAppObject"));

    jclass managerClass = env->GetObjectClass(app);
    VerifyOrReturn(managerClass != nullptr, ChipLogError(Zcl, "Failed to get DeviceAppJNI Java class"));

    mPostClusterInitMethod = env->GetMethodID(managerClass, "postClusterInit", "(JI)V");
    if (mPostClusterInitMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access DeviceApp 'postClusterInit' method");
        env->ExceptionClear();
    }

    mPostEventMethod = env->GetMethodID(managerClass, "postEvent", "(J)V");
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
    VerifyOrReturn(mDeviceAppObject.HasValidObjectRef(), ChipLogError(Zcl, "DeviceAppJNI::mDeviceAppObject null"));
    VerifyOrReturn(mPostClusterInitMethod != nullptr, ChipLogError(Zcl, "DeviceAppJNI::mPostClusterInitMethod null"));

    env->CallVoidMethod(mDeviceAppObject.ObjectRef(), mPostClusterInitMethod, static_cast<jlong>(clusterId),
                        static_cast<jint>(endpoint));
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
    VerifyOrReturn(mDeviceAppObject.HasValidObjectRef(), ChipLogError(Zcl, "DeviceAppJNI::mDeviceAppObject null"));
    VerifyOrReturn(mPostEventMethod != nullptr, ChipLogError(Zcl, "DeviceAppJNI::mPostEventMethod null"));

    env->CallVoidMethod(mDeviceAppObject.ObjectRef(), mPostEventMethod, static_cast<jlong>(event));
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

    gDeviceTypeIds[0].deviceTypeId = static_cast<uint16_t>(deviceTypeId);
    emberAfSetDeviceTypeList(1, Span<const EmberAfDeviceType>(gDeviceTypeIds));
}

JNI_METHOD(void, setDACProvider)(JNIEnv *, jobject, jobject provider)
{
    if (!chip::Credentials::IsDeviceAttestationCredentialsProviderSet())
    {
        chip::Credentials::SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());
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

/*
 * Color Control Manager
 */
JNI_METHOD(void, setColorControlManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    ColorControlManager::NewManager(endpoint, manager);
}

/*
 * Door Lock Manager
 */
JNI_METHOD(void, setDoorLockManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    DoorLockManager::NewManager(endpoint, manager);
}

JNI_METHOD(jboolean, setLockType)(JNIEnv *, jobject, jint endpoint, jint value)
{
    return DeviceLayer::SystemLayer().ScheduleLambda([endpoint, value] { DoorLockManager::SetLockType(endpoint, value); }) ==
        CHIP_NO_ERROR;
}

JNI_METHOD(jboolean, setLockState)(JNIEnv *, jobject, jint endpoint, jint value)
{
    return DeviceLayer::SystemLayer().ScheduleLambda([endpoint, value] { DoorLockManager::SetLockState(endpoint, value); }) ==
        CHIP_NO_ERROR;
}

JNI_METHOD(jboolean, setActuatorEnabled)(JNIEnv *, jobject, jint endpoint, jboolean value)
{
    return DeviceLayer::SystemLayer().ScheduleLambda([endpoint, value] { DoorLockManager::SetActuatorEnabled(endpoint, value); }) ==
        CHIP_NO_ERROR;
}

JNI_METHOD(jboolean, setAutoRelockTime)(JNIEnv *, jobject, jint endpoint, jint value)
{
    return DeviceLayer::SystemLayer().ScheduleLambda([endpoint, value] { DoorLockManager::SetAutoRelockTime(endpoint, value); }) ==
        CHIP_NO_ERROR;
}

JNI_METHOD(jboolean, setOperatingMode)(JNIEnv *, jobject, jint endpoint, jint value)
{
    return DeviceLayer::SystemLayer().ScheduleLambda([endpoint, value] { DoorLockManager::SetOperatingMode(endpoint, value); }) ==
        CHIP_NO_ERROR;
}

JNI_METHOD(jboolean, setSupportedOperatingModes)(JNIEnv *, jobject, jint endpoint, jint value)
{
    return DeviceLayer::SystemLayer().ScheduleLambda(
               [endpoint, value] { DoorLockManager::SetSupportedOperatingModes(endpoint, value); }) == CHIP_NO_ERROR;
}

JNI_METHOD(jboolean, sendLockAlarmEvent)(JNIEnv *, jobject, jint endpoint)
{
    return DeviceLayer::SystemLayer().ScheduleLambda([endpoint] { DoorLockManager::SendLockAlarmEvent(endpoint); }) ==
        CHIP_NO_ERROR;
}

/*
 * Power Source Manager
 */
JNI_METHOD(void, setPowerSourceManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    PowerSourceManager::NewManager(endpoint, manager);
}

JNI_METHOD(jboolean, setBatPercentRemaining)(JNIEnv *, jobject, jint endpoint, jint value)
{
    return DeviceLayer::SystemLayer().ScheduleLambda(
               [endpoint, value] { PowerSourceManager::SetBatPercentRemaining(endpoint, value); }) == CHIP_NO_ERROR;
}
