/*
 *   Copyright (c) 2020 Project CHIP Authors
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
#include "AndroidDeviceControllerWrapper.h"
#include "CHIPJNIError.h"

#include <algorithm>
#include <memory>

#include <platform/KeyValueStoreManager.h>
#include <support/ThreadOperationalDataset.h>

using chip::Controller::DeviceCommissioner;

extern chip::Ble::BleLayer * GetJNIBleLayer();

AndroidDeviceControllerWrapper::~AndroidDeviceControllerWrapper()
{
    if ((mJavaVM != nullptr) && (mJavaObjectRef != nullptr))
    {
        GetEnvForCurrentThread()->DeleteGlobalRef(mJavaObjectRef);
    }
    mController->Shutdown();
}

void AndroidDeviceControllerWrapper::SetJavaObjectRef(JavaVM * vm, jobject obj)
{
    mJavaVM        = vm;
    mJavaObjectRef = GetEnvForCurrentThread()->NewGlobalRef(obj);
}

void AndroidDeviceControllerWrapper::CallJavaMethod(const char * methodName, jint argument)
{
    CallVoidInt(GetEnvForCurrentThread(), mJavaObjectRef, methodName, argument);
}

AndroidDeviceControllerWrapper * AndroidDeviceControllerWrapper::AllocateNew(JavaVM * vm, jobject deviceControllerObj,
                                                                             pthread_mutex_t * stackLock, chip::NodeId nodeId,
                                                                             chip::System::Layer * systemLayer,
                                                                             chip::Inet::InetLayer * inetLayer,
                                                                             CHIP_ERROR * errInfoOnFailure)
{
    if (errInfoOnFailure == nullptr)
    {
        ChipLogError(Controller, "Missing error info");
        return nullptr;
    }
    if (systemLayer == nullptr)
    {
        ChipLogError(Controller, "Missing system layer");
        *errInfoOnFailure = CHIP_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }
    if (inetLayer == nullptr)
    {
        ChipLogError(Controller, "Missing inet layer");
        *errInfoOnFailure = CHIP_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    *errInfoOnFailure = CHIP_NO_ERROR;

    std::unique_ptr<DeviceCommissioner> controller(new DeviceCommissioner());

    if (!controller)
    {
        *errInfoOnFailure = CHIP_ERROR_NO_MEMORY;
        return nullptr;
    }
    std::unique_ptr<AndroidDeviceControllerWrapper> wrapper(new AndroidDeviceControllerWrapper(std::move(controller), stackLock));

    wrapper->SetJavaObjectRef(vm, deviceControllerObj);
    wrapper->Controller()->SetUdpListenPort(CHIP_PORT + 1);

    chip::Controller::CommissionerInitParams initParams;

    initParams.storageDelegate = wrapper.get();
    initParams.pairingDelegate = wrapper.get();
    initParams.systemLayer     = systemLayer;
    initParams.inetLayer       = inetLayer;
    initParams.bleLayer        = GetJNIBleLayer();

    *errInfoOnFailure = wrapper->OpCredsIssuer().Initialize(*initParams.storageDelegate);
    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    initParams.operationalCredentialsDelegate = &wrapper->OpCredsIssuer();

    *errInfoOnFailure = wrapper->Controller()->Init(nodeId, initParams);

    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    *errInfoOnFailure = wrapper->Controller()->ServiceEvents();

    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    return wrapper.release();
}

void AndroidDeviceControllerWrapper::OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status)
{
    StackUnlockGuard unlockGuard(mStackLock);
    CallJavaMethod("onStatusUpdate", static_cast<jint>(status));
}

void AndroidDeviceControllerWrapper::OnPairingComplete(CHIP_ERROR error)
{
    StackUnlockGuard unlockGuard(mStackLock);
    CallJavaMethod("onPairingComplete", static_cast<jint>(error));
}

void AndroidDeviceControllerWrapper::OnPairingDeleted(CHIP_ERROR error)
{
    StackUnlockGuard unlockGuard(mStackLock);
    CallJavaMethod("onPairingDeleted", static_cast<jint>(error));
}

void AndroidDeviceControllerWrapper::OnMessage(chip::System::PacketBufferHandle && msg) {}

void AndroidDeviceControllerWrapper::OnStatusChange(void) {}

CHIP_ERROR AndroidDeviceControllerWrapper::SyncGetKeyValue(const char * key, void * value, uint16_t & size)
{
    ChipLogProgress(chipTool, "KVS: Getting key %s", key);

    size_t read_size = 0;

    CHIP_ERROR err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(key, value, size, &read_size);

    size = static_cast<uint16_t>(read_size);

    return err;
}

CHIP_ERROR AndroidDeviceControllerWrapper::SyncSetKeyValue(const char * key, const void * value, uint16_t size)
{
    ChipLogProgress(chipTool, "KVS: Setting key %s", key);
    return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(key, value, size);
}

CHIP_ERROR AndroidDeviceControllerWrapper::SyncDeleteKeyValue(const char * key)
{
    ChipLogProgress(chipTool, "KVS: Deleting key %s", key);
    return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(key);
}
