/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 * @file DeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/
#include "DeviceCallbacks.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/CommandHandler.h>
#include <app/server/Dnssd.h>
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <app/util/util.h>
#include <jni.h>
#include <lib/dnssd/Advertiser.h>
#include <lib/support/JniReferences.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging;
namespace {
void OnPlatformEventWrapper(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    DeviceCallbacks * commissionMgr = reinterpret_cast<DeviceCallbacks *>(arg);
    commissionMgr->OnPlatformEvent(event);
}
} // namespace
void DeviceCallbacks::NewManager(jobject manager)
{
    ChipLogProgress(AppServer, "TV Android App: set ChipDeviceEvent delegate");
    DeviceCallbacks * mgr = new DeviceCallbacks();
    PlatformMgr().AddEventHandler(OnPlatformEventWrapper, reinterpret_cast<intptr_t>(mgr));
    mgr->InitializeWithObjects(manager);
}

void DeviceCallbacks::InitializeWithObjects(jobject provider)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(AppServer, "Failed to GetEnvForCurrentThread for DeviceEventProvider"));

    mProvider = env->NewGlobalRef(provider);
    VerifyOrReturn(mProvider != nullptr, ChipLogError(AppServer, "Failed to NewGlobalRef DeviceEventProvider"));
    jclass deviceEventProviderCls = env->GetObjectClass(mProvider);
    VerifyOrReturn(deviceEventProviderCls != nullptr, ChipLogError(AppServer, "Failed to get KeypadInputManager Java class"));

    mCommissioningCompleteMethod = env->GetMethodID(deviceEventProviderCls, "onCommissioningComplete", "()V");
    if (mCommissioningCompleteMethod == nullptr)
    {
        ChipLogError(AppServer, "Failed to access DeviceEventProvider 'onCommissioningComplete' method");
        env->ExceptionClear();
    }
}

void DeviceCallbacks::OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCommissioningComplete:
        OnCommissioningComplete(event);
        break;
    }
}

void DeviceCallbacks::OnCommissioningComplete(const ChipDeviceEvent * event)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for DeviceEventProvider"));
    env->CallVoidMethod(mProvider, mCommissioningCompleteMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in DeviceEventProvider::onCommissioningComplete");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
