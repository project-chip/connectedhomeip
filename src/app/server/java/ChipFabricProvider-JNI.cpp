/*
 *   Copyright (c) 2021 Project CHIP Authors
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
#include <trace/trace.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_chip_appserver_ChipFabricProvider_##METHOD_NAME

namespace {
JavaVM * sJVM;
// jclass sFabricProviderCls = NULL;
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

    // Get a JNI environment object.
    env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    ChipLogProgress(DeviceLayer, "Loading Java class references.");

    chip::InitializeTracing();

exit:
    if (err != CHIP_NO_ERROR)
    {
        // JniReferences::GetInstance().ThrowError(env, sFabricProviderCls, err);
        JNI_OnUnload(jvm, reserved);
    }

    return err;
}

void AndroidChipFabricProviderJNI_OnUnload(JavaVM * jvm, void * reserved)
{
    ChipLogProgress(DeviceLayer, "ChipFabricProvider JNI_OnUnload() called");
    chip::Platform::MemoryShutdown();
}

int GetFabricCount()
{
    return chip::Server::GetInstance().GetFabricTable().FabricCount();
}

jobject ReadFabricList(JNIEnv * env, jobject self)
{
    jobject jFabricList;
    JniReferences::GetInstance().CreateArrayList(jFabricList);
    jclass jFabricCls = env->FindClass("chip/appserver/Fabric");
    VerifyOrExit(jFabricCls != nullptr, ChipLogError(NotSpecified, "unfind Class Fabric"));
    {
        for (auto & fabricInfo : Server::GetInstance().GetFabricTable())
        {
            // see src/app/clusters/operational-credentials-server/operational-credentials-server.cpp#ReadFabricsList
            if (!fabricInfo.IsInitialized())
                continue;

            jmethodID jFabricMethod = env->GetMethodID(jFabricCls, "<init>", "()V");
            jobject jFabric         = env->NewObject(jFabricCls, jFabricMethod);
            jfieldID jvendorId      = env->GetFieldID(jFabricCls, "vendorId", "I");
            jfieldID jnodeId        = env->GetFieldID(jFabricCls, "nodeId", "J");
            jfieldID jfabricIndex   = env->GetFieldID(jFabricCls, "fabricIndex", "S");
            jfieldID jlabel         = env->GetFieldID(jFabricCls, "label", "Ljava/lang/String;");
            env->SetIntField(jFabric, jvendorId, fabricInfo.GetVendorId());
            env->SetLongField(jFabric, jnodeId, fabricInfo.GetNodeId());
            env->SetShortField(jFabric, jfabricIndex, fabricInfo.GetFabricIndex());
            UtfString jLabelStr(env, fabricInfo.GetFabricLabel());
            env->SetObjectField(jFabric, jlabel, jLabelStr.jniValue());

            JniReferences::GetInstance().AddToList(jFabricList, jFabric);
        }
        return jFabricList;
    }

exit:
    return jFabricList;
}

JNI_METHOD(jint, getFabricCount)(JNIEnv * env, jobject self)
{
    return GetFabricCount();
}

JNI_METHOD(jobject, getFabricList)(JNIEnv * env, jobject self)
{
    return ReadFabricList(env, self);
}