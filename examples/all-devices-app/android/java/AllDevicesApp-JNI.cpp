/*
 *   Copyright (c) 2026 Project CHIP Authors
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

#include "AppImpl.h"
#include <app/server/Server.h>
#include <app/server/java/AndroidAppServerWrapper.h>
#include <device-factory/DeviceFactory.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <platform/CHIPDeviceLayer.h>
#include <pthread.h>

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_com_google_matter_alldevices_App_##METHOD_NAME

namespace {
pthread_t sIOThread = 0;
JavaVM * sJVM       = nullptr;

void * IOThreadAppMain(void * arg)
{
    JNIEnv * env;
    JavaVMAttachArgs attachArgs;

    attachArgs.version = JNI_VERSION_1_6;
    attachArgs.name    = (char *) "AllDevicesApp IO Thread";
    attachArgs.group   = NULL;
#ifdef __ANDROID__
    sJVM->AttachCurrentThreadAsDaemon(&env, (void *) &attachArgs);
#else
    sJVM->AttachCurrentThreadAsDaemon((void **) &env, (void *) &attachArgs);
#endif

    ChipLogProgress(AppServer, "AllDevicesApp IO thread starting");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    ChipLogProgress(AppServer, "AllDevicesApp IO thread ending");

    sJVM->DetachCurrentThread();

    return NULL;
}
} // namespace

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    sJVM = jvm;
    return AndroidAppServerJNI_OnLoad(jvm, reserved);
}

void JNI_OnUnload(JavaVM * jvm, void * reserved)
{
    chip::DeviceLayer::StackLock lock;
    if (sIOThread != 0)
    {
        CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to stop event loop task: %" CHIP_ERROR_FORMAT, err.Format());
        }
        chip::DeviceLayer::StackUnlock unlock;
        pthread_join(sIOThread, NULL);
        sIOThread = 0;
    }
    AndroidAppServerJNI_OnUnload(jvm, reserved);
}

JNI_METHOD(jobjectArray, getSupportedDeviceTypes)(JNIEnv * env, jobject self)
{
    auto deviceTypes = DeviceFactory::GetInstance().SupportedDeviceTypes();

    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray array = env->NewObjectArray(static_cast<jsize>(deviceTypes.size()), stringClass, nullptr);

    for (size_t i = 0; i < deviceTypes.size(); ++i)
    {
        jstring deviceTypeStr = env->NewStringUTF(deviceTypes[i].c_str());
        env->SetObjectArrayElement(array, static_cast<jsize>(i), deviceTypeStr);
        env->DeleteLocalRef(deviceTypeStr);
    }

    // Return the array object (which is type jobjectArray)
    return array;
}

JNI_METHOD(jboolean, startApp)(JNIEnv * env, jobject self, jobjectArray selectedDevices)
{
    chip::DeviceLayer::StackLock lock;

    std::vector<std::string> devices;
    jsize len = env->GetArrayLength(selectedDevices);
    for (jsize i = 0; i < len; i++)
    {
        jstring jstr = (jstring) env->GetObjectArrayElement(selectedDevices, i);
        JniUtfString utfStr(env, jstr);
        devices.push_back(utfStr.c_str());
    }

    CHIP_ERROR err = AllDevicesAppStart(devices);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to start AllDevicesApp: %" CHIP_ERROR_FORMAT, err.Format());
        return JNI_FALSE;
    }

    if (sIOThread == 0)
    {
        pthread_create(&sIOThread, NULL, IOThreadAppMain, NULL);
    }

    return JNI_TRUE;
}

JNI_METHOD(jboolean, stopApp)(JNIEnv * env, jobject self)
{
    AllDevicesAppShutdown();
    return JNI_TRUE;
}

JNI_METHOD(void, resetApp)(JNIEnv * env, jobject self)
{
    chip::DeviceLayer::StackLock lock;
    chip::Server::GetInstance().ScheduleFactoryReset();
}

JNI_METHOD(jobjectArray, getOnboardingCodes)(JNIEnv * env, jobject self, jint discriminator)
{
    chip::SetupPayload payload;
    payload.version = 0;
    payload.rendezvousInformation.SetValue(chip::RendezvousInformationFlag::kOnNetwork);
    payload.setUpPINCode = 20202021L;
    payload.discriminator.SetLongValue(static_cast<uint16_t>(discriminator));

    chip::DeviceLayer::DeviceInstanceInfoProvider * provider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();
    if (provider != nullptr)
    {
        (void) provider->GetVendorId(payload.vendorID);
        (void) provider->GetProductId(payload.productID);
    }
    else
    {
        payload.vendorID = 65521; // fallback test VID
        payload.productID = 32768; // fallback test PID
    }

    std::string qrCode;
    LogErrorOnFailure(chip::QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(qrCode));

    std::string manualCode;
    LogErrorOnFailure(chip::ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualCode));

    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray array = env->NewObjectArray(2, stringClass, nullptr);

    jstring jManual = env->NewStringUTF(manualCode.c_str());
    env->SetObjectArrayElement(array, 0, jManual);
    env->DeleteLocalRef(jManual);

    jstring jQr = env->NewStringUTF(qrCode.c_str());
    env->SetObjectArrayElement(array, 1, jQr);
    env->DeleteLocalRef(jQr);

    return array;
}
