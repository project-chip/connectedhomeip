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
#include <support/ScopedBuffer.h>
#include <support/ThreadOperationalDataset.h>

using namespace chip;
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

CHIP_ERROR AndroidDeviceControllerWrapper::GetRootCACertificate(FabricId fabricId, MutableByteSpan & outCert)
{
    Initialize();
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    chip::Credentials::X509CertRequestParams newCertParams = { 0, mIssuerId, mNow, mNow + mValidity, true, fabricId, false, 0 };

    size_t outCertSize  = (outCert.size() > UINT32_MAX) ? UINT32_MAX : outCert.size();
    uint32_t outCertLen = 0;
    ReturnErrorOnFailure(NewRootX509Cert(newCertParams, mIssuer, outCert.data(), static_cast<uint32_t>(outCertSize), outCertLen));
    outCert.reduce_size(outCertLen);

    return CHIP_NO_ERROR;
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
CHIP_ERROR
AndroidDeviceControllerWrapper::GenerateNodeOperationalCertificate(const Optional<NodeId> & nodeId, FabricId fabricId,
                                                                   const ByteSpan & csr, const ByteSpan & DAC,
                                                                   Callback::Callback<NOCGenerated> * onNOCGenerated)
{
    jmethodID method;
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = JniReferences::GetInstance().FindMethod(JniReferences::GetInstance().GetEnvForCurrentThread(), mJavaObjectRef,
                                                  "onOpCSRGenerationComplete", "([B)V", &method);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Error invoking onOpCSRGenerationComplete: %" CHIP_ERROR_FORMAT, ChipError::FormatError(err));
        return err;
    }

    // Initializing the KeyPair.
    Initialize();

    chip::NodeId assignedId;
    if (nodeId.HasValue())
    {
        assignedId = nodeId.Value();
    }
    else
    {
        assignedId = mNextAvailableNodeId++;
    }

    chip::Credentials::X509CertRequestParams request = { 1, mIssuerId, mNow, mNow + mValidity, true, fabricId, true, assignedId };

    chip::P256PublicKey pubkey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey));

    ChipLogProgress(chipTool, "VerifyCertificateSigningRequest");

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    ReturnErrorCodeIf(!noc.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    uint32_t nocLen = 0;

    CHIP_ERROR generateCert = NewNodeOperationalX509Cert(request, chip::Credentials::CertificateIssuerLevel::kIssuerIsRootCA,
                                                         pubkey, mIssuer, noc.Get(), kMaxCHIPDERCertLength, nocLen);

    onNOCGenerated->mCall(onNOCGenerated->mContext, ByteSpan(noc.Get(), nocLen));

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
