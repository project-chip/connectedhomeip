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

#include <core/CHIPTLV.h>
#include <platform/KeyValueStoreManager.h>
#include <support/PersistentStorageMacros.h>
#include <support/SafeInt.h>
#include <support/ScopedBuffer.h>
#include <support/ThreadOperationalDataset.h>

using namespace chip;
using namespace chip::Controller;
using namespace TLV;

extern chip::Ble::BleLayer * GetJNIBleLayer();

constexpr const char kOperationalCredentialsIssuerKeypairStorage[]   = "AndroidDeviceControllerKey";
constexpr const char kOperationalCredentialsRootCertificateStorage[] = "AndroidCARootCert";
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

// TODO Refactor this API to match latest spec, so that GenerateNodeOperationalCertificate receives the full CSR Elements data
// payload.
CHIP_ERROR AndroidDeviceControllerWrapper::GenerateNOCChain(const ByteSpan & csrElements, const ByteSpan & attestationSignature,
                                                            const ByteSpan & DAC, const ByteSpan & PAI, const ByteSpan & PAA,
                                                            Callback::Callback<OnNOCChainGeneration> * onCompletion)
{
    jmethodID method;
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = JniReferences::GetInstance().FindMethod(JniReferences::GetInstance().GetEnvForCurrentThread(), mJavaObjectRef,
                                                  "onOpCSRGenerationComplete", "([B)V", &method);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Error invoking onOpCSRGenerationComplete: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    // Initializing the KeyPair.
    Initialize();

    chip::NodeId assignedId;
    if (mNodeIdRequested)
    {
        assignedId       = mNextRequestedNodeId;
        mNodeIdRequested = false;
    }
    else
    {
        assignedId = mNextAvailableNodeId++;
    }

    chip::Credentials::X509CertRequestParams request = {
        1, mIssuerId, mNow, mNow + mValidity, true, mNextFabricId, true, assignedId
    };

    TLVReader reader;
    reader.Init(csrElements.data(), static_cast<uint32_t>(csrElements.size()));

    if (reader.GetType() == kTLVType_NotSpecified)
    {
        ReturnErrorOnFailure(reader.Next());
    }

    VerifyOrReturnError(reader.GetType() == kTLVType_Structure, CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrReturnError(reader.GetTag() == AnonymousTag, CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

    TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(kTLVType_ByteString, TLV::ContextTag(1)));

    ByteSpan csr(reader.GetReadPoint(), reader.GetLength());
    reader.ExitContainer(containerType);

    chip::P256PublicKey pubkey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey));

    ChipLogProgress(chipTool, "VerifyCertificateSigningRequest");

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    ReturnErrorCodeIf(!noc.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    uint32_t nocLen = 0;

    CHIP_ERROR generateCert = NewNodeOperationalX509Cert(request, chip::Credentials::CertificateIssuerLevel::kIssuerIsRootCA,
                                                         pubkey, mIssuer, noc.Get(), kMaxCHIPDERCertLength, nocLen);

    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;
    ReturnErrorCodeIf(!rcac.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    uint16_t rootCertBufLen = kMaxCHIPDERCertLength;

    PERSISTENT_KEY_OP(mNextFabricId, kOperationalCredentialsRootCertificateStorage, key,
                      err = SyncGetKeyValue(key, rcac.Get(), rootCertBufLen));
    if (err != CHIP_NO_ERROR)
    {
        // Storage doesn't have an existing root certificate. Let's create one and add it to the storage.
        chip::Credentials::X509CertRequestParams rcac_request = { 0,    mIssuerId,     mNow,  mNow + mValidity,
                                                                  true, mNextFabricId, false, 0 };

        uint32_t outCertLen = 0;
        ReturnErrorOnFailure(NewRootX509Cert(rcac_request, mIssuer, rcac.Get(), kMaxCHIPDERCertLength, outCertLen));

        VerifyOrReturnError(CanCastTo<uint16_t>(outCertLen), CHIP_ERROR_INVALID_ARGUMENT);
        rootCertBufLen = static_cast<uint16_t>(outCertLen);
        PERSISTENT_KEY_OP(mNextFabricId, kOperationalCredentialsRootCertificateStorage, key,
                          err = SyncSetKeyValue(key, rcac.Get(), rootCertBufLen));
        ReturnErrorOnFailure(err);
    }

    onCompletion->mCall(onCompletion->mContext, generateCert, ByteSpan(noc.Get(), nocLen), ByteSpan(),
                        ByteSpan(rcac.Get(), rootCertBufLen));

    jbyteArray javaCsr;
    JniReferences::GetInstance().GetEnvForCurrentThread()->ExceptionClear();
    JniReferences::GetInstance().N2J_ByteArray(JniReferences::GetInstance().GetEnvForCurrentThread(), csrElements.data(),
                                               csrElements.size(), javaCsr);
    JniReferences::GetInstance().GetEnvForCurrentThread()->CallVoidMethod(mJavaObjectRef, method, javaCsr);
    return generateCert;
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
    CallJavaMethod("onPairingComplete", static_cast<jint>(error.AsInteger()));
}

void AndroidDeviceControllerWrapper::OnPairingDeleted(CHIP_ERROR error)
{
    StackUnlockGuard unlockGuard(mStackLock);
    CallJavaMethod("onPairingDeleted", static_cast<jint>(error.AsInteger()));
}

void AndroidDeviceControllerWrapper::OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error)
{
    StackUnlockGuard unlockGuard(mStackLock);
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID onCommissioningCompleteMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, mJavaObjectRef, "onCommissioningComplete", "(JI)V",
                                                             &onCommissioningCompleteMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error finding Java method: %" CHIP_ERROR_FORMAT, err.Format()));
    env->CallVoidMethod(mJavaObjectRef, onCommissioningCompleteMethod, static_cast<jlong>(deviceId), error.AsInteger());
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
