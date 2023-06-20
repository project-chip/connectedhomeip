/*
 *   Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *      Implementation of JNI bridge for CHIP App Server for Android TV apps
 *
 */
#include "ChipFabricProvider-JNI.h"
#include "AndroidAppServerWrapper.h"
#include <app/server/Server.h>
#include <cstdlib>
#include <iostream>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniTypeWrappers.h>
#include <platform/CHIPDeviceLayer.h>
#include <thread>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_chip_appserver_ChipFabricProvider_##METHOD_NAME

namespace {
JavaVM * sJVM;
} // namespace

CHIP_ERROR AndroidChipFabricProviderJNI_OnLoad(JavaVM * jvm, void * reserved)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;

    ChipLogProgress(DeviceLayer, "ChipFabricProvider JNI_OnLoad() called");

    chip::Platform::MemoryInit();

    // Save a reference to the JVM.  Will need this to call back into Java.
    JniReferences::GetInstance().SetJavaVm(jvm, "chip/appserver/ChipFabricProvider");
    sJVM = jvm;

    // check if the JNI environment is correct
    env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

exit:
    if (err != CHIP_NO_ERROR)
    {
        JNI_OnUnload(jvm, reserved);
    }

    return err;
}

void AndroidChipFabricProviderJNI_OnUnload(JavaVM * jvm, void * reserved)
{
    ChipLogProgress(DeviceLayer, "ChipFabricProvider JNI_OnUnload() called");
    chip::Platform::MemoryShutdown();
}

CHIP_ERROR ReadFabricList(JNIEnv * env, jobject & self)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    jclass jFabricCls = env->FindClass("chip/appserver/Fabric");
    VerifyOrExit(self != nullptr, err = CHIP_JNI_ERROR_NULL_OBJECT);
    VerifyOrExit(jFabricCls != nullptr, ChipLogError(NotSpecified, "could not find Class Fabric"));
    for (auto & fabricInfo : Server::GetInstance().GetFabricTable())
    {

        jmethodID constructor = env->GetMethodID(jFabricCls, "<init>", "()V");
        VerifyOrExit(constructor != nullptr, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);
        jobject jFabric = env->NewObject(jFabricCls, constructor);
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
        jfieldID jvendorId = env->GetFieldID(jFabricCls, "vendorId", "I");
        VerifyOrExit(jvendorId != nullptr, err = CHIP_JNI_ERROR_FIELD_NOT_FOUND);
        jfieldID jnodeId = env->GetFieldID(jFabricCls, "nodeId", "J");
        VerifyOrExit(jnodeId != nullptr, err = CHIP_JNI_ERROR_FIELD_NOT_FOUND);
        jfieldID jfabricIndex = env->GetFieldID(jFabricCls, "fabricIndex", "S");
        VerifyOrExit(jfabricIndex != nullptr, err = CHIP_JNI_ERROR_FIELD_NOT_FOUND);
        jfieldID jlabel = env->GetFieldID(jFabricCls, "label", "Ljava/lang/String;");
        VerifyOrExit(jlabel != nullptr, err = CHIP_JNI_ERROR_FIELD_NOT_FOUND);

        env->SetIntField(jFabric, jvendorId, fabricInfo.GetVendorId());
        env->SetLongField(jFabric, jnodeId, static_cast<jlong>(fabricInfo.GetNodeId()));
        env->SetShortField(jFabric, jfabricIndex, fabricInfo.GetFabricIndex());
        UtfString jLabelStr(env, fabricInfo.GetFabricLabel());
        env->SetObjectField(jFabric, jlabel, jLabelStr.jniValue());

        JniReferences::GetInstance().AddToList(self, jFabric);
    }

exit:
    return err;
}

JNI_METHOD(jint, getFabricCount)(JNIEnv * env, jobject self)
{
    // a simplified way to get fabric count,see /src/credentials/FabricTable.h#FabricCount
    return chip::Server::GetInstance().GetFabricTable().FabricCount();
}

JNI_METHOD(jobject, getFabricList)(JNIEnv * env, jobject self)
{
    jobject jFabricList;
    JniReferences::GetInstance().CreateArrayList(jFabricList);
    ReadFabricList(env, jFabricList);
    return jFabricList;
}
