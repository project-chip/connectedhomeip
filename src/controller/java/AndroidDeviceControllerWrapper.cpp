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
#include "StackLock.h"

#include <algorithm>
#include <memory>

#include "JniReferences.h"
#include <support/CodeUtils.h>

#include <platform/KeyValueStoreManager.h>
#include <support/ThreadOperationalDataset.h>

using namespace chip::Controller;

extern chip::Ble::BleLayer * GetJNIBleLayer();

constexpr const char kOperationalCredentialsIssuerKeypairStorage[] = "AndroidDeviceControllerKey";
AndroidDeviceControllerWrapper::~AndroidDeviceControllerWrapper()
{
    if ((mJavaVM != nullptr) && (mJavaObjectRef != nullptr))
    {
        JniReferences::GetInstance().GetEnvForCurrentThread()->DeleteGlobalRef(mJavaObjectRef);
    }
    mController->Shutdown();
}

void AndroidDeviceControllerWrapper::SetJavaObjectRef(JavaVM * vm, jobject obj)
{
    mJavaVM        = vm;
    mJavaObjectRef = JniReferences::GetInstance().GetEnvForCurrentThread()->NewGlobalRef(obj);
}

void AndroidDeviceControllerWrapper::CallJavaMethod(const char * methodName, jint argument)
{
    JniReferences::GetInstance().CallVoidInt(JniReferences::GetInstance().GetEnvForCurrentThread(), mJavaObjectRef, methodName,
                                             argument);
}

CHIP_ERROR AndroidDeviceControllerWrapper::GetRootCACertificate(chip::FabricId fabricId, uint8_t * certBuf, uint32_t certBufSize,
                                                                uint32_t & outCertLen)
{
    Initialize();
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    chip::X509CertRequestParams newCertParams = { 0, mIssuerId, mNow, mNow + mValidity, true, fabricId, false, 0 };
    return NewRootX509Cert(newCertParams, mIssuer, certBuf, certBufSize, outCertLen);
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

    initParams.storageDelegate                = wrapper.get();
    initParams.pairingDelegate                = wrapper.get();
    initParams.operationalCredentialsDelegate = wrapper.get();
    initParams.systemLayer                    = systemLayer;
    initParams.inetLayer                      = inetLayer;
    initParams.bleLayer                       = GetJNIBleLayer();

    *errInfoOnFailure = wrapper->OpCredsIssuer().Initialize(*initParams.storageDelegate);
    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        return nullptr;
    }

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

// TODO Refactor this API to match latest spec, so that GenerateNodeOperationalCertificate receives the full CSR Elements data
// payload.
CHIP_ERROR AndroidDeviceControllerWrapper::GenerateNodeOperationalCertificate(const chip::PeerId & peerId,
                                                                              const chip::ByteSpan & csr, int64_t serialNumber,
                                                                              uint8_t * certBuf, uint32_t certBufSize,
                                                                              uint32_t & outCertLen)
{
    jmethodID method;
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = JniReferences::GetInstance().FindMethod(JniReferences::GetInstance().GetEnvForCurrentThread(), mJavaObjectRef,
                                                  "onOpCSRGenerationComplete", "([B)V", &method);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Error invoking onOpCSRGenerationComplete: %d", err);
        return err;
    }

    // Initializing the KeyPair.
    Initialize();

    chip::X509CertRequestParams request = { serialNumber, mIssuerId,         mNow, mNow + mValidity, true, peerId.GetFabricId(),
                                            true,         peerId.GetNodeId() };

    chip::P256PublicKey pubkey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey));

    ChipLogProgress(chipTool, "VerifyCertificateSigningRequest");

    CHIP_ERROR generateCert = NewNodeOperationalX509Cert(request, chip::CertificateIssuerLevel::kIssuerIsRootCA, pubkey, mIssuer,
                                                         certBuf, certBufSize, outCertLen);
    jbyteArray argument;
    JniReferences::GetInstance().GetEnvForCurrentThread()->ExceptionClear();
    JniReferences::GetInstance().N2J_ByteArray(JniReferences::GetInstance().GetEnvForCurrentThread(), csr.data(), csr.size(),
                                               argument);
    JniReferences::GetInstance().GetEnvForCurrentThread()->CallVoidMethod(mJavaObjectRef, method, argument);
    return generateCert;
}

CHIP_ERROR AndroidDeviceControllerWrapper::Initialize()
{
    chip::Crypto::P256SerializedKeypair serializedKey;
    uint16_t keySize = static_cast<uint16_t>(sizeof(serializedKey));

    // TODO: Use Android keystore system instead of direct storage of private key and add specific errors to check if a specified
    // item is not found in the keystore.
    if (SyncGetKeyValue(kOperationalCredentialsIssuerKeypairStorage, &serializedKey, keySize) != CHIP_NO_ERROR)
    {
        // If storage doesn't have an existing keypair, create one and add it to the storage.
        ReturnErrorOnFailure(mIssuer.Initialize());
        ReturnErrorOnFailure(mIssuer.Serialize(serializedKey));
        keySize = static_cast<uint16_t>(sizeof(serializedKey));
        SyncSetKeyValue(kOperationalCredentialsIssuerKeypairStorage, &serializedKey, keySize);
    }
    else
    {
        // Use the keypair from the storage
        ReturnErrorOnFailure(mIssuer.Deserialize(serializedKey));
    }

    mInitialized = true;
    return CHIP_NO_ERROR;
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
