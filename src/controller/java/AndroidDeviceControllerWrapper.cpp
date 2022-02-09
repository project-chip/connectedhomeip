/*
 *   Copyright (c) 2020-2022 Project CHIP Authors
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
#include <lib/support/CHIPJNIError.h>

#include <algorithm>
#include <memory>

#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>

#include <controller/CHIPDeviceControllerFactory.h>
#include <credentials/DeviceAttestationVerifier.h>
#include <credentials/examples/DefaultDeviceAttestationVerifier.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/KeyValueStoreManager.h>

using namespace chip;
using namespace chip::Controller;
using namespace chip::Credentials;
using namespace TLV;

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

CHIP_ERROR AndroidDeviceControllerWrapper::GenerateNOCChainAfterValidation(NodeId nodeId, FabricId fabricId,
                                                                           const Crypto::P256PublicKey & pubkey,
                                                                           MutableByteSpan & rcac, MutableByteSpan & icac,
                                                                           MutableByteSpan & noc)
{
    ChipDN noc_dn;
    noc_dn.AddAttribute(chip::ASN1::kOID_AttributeType_ChipFabricId, fabricId);
    noc_dn.AddAttribute(chip::ASN1::kOID_AttributeType_ChipNodeId, nodeId);
    ChipDN rcac_dn;
    rcac_dn.AddAttribute(chip::ASN1::kOID_AttributeType_ChipRootId, mIssuerId);
    rcac_dn.AddAttribute(chip::ASN1::kOID_AttributeType_ChipFabricId, fabricId);

    ChipLogProgress(Controller, "Generating NOC");
    chip::Credentials::X509CertRequestParams noc_request = { 1, mNow, mNow + mValidity, noc_dn, rcac_dn };
    ReturnErrorOnFailure(NewNodeOperationalX509Cert(noc_request, pubkey, mIssuer, noc));
    icac.reduce_size(0);

    uint16_t rcacBufLen = static_cast<uint16_t>(std::min(rcac.size(), static_cast<size_t>(UINT16_MAX)));
    CHIP_ERROR err      = CHIP_NO_ERROR;
    PERSISTENT_KEY_OP(fabricId, kOperationalCredentialsRootCertificateStorage, key,
                      err = SyncGetKeyValue(key, rcac.data(), rcacBufLen));
    if (err == CHIP_NO_ERROR)
    {
        // Found root certificate in the storage.
        rcac.reduce_size(rcacBufLen);
        return CHIP_NO_ERROR;
    }

    ChipLogProgress(Controller, "Generating RCAC");
    chip::Credentials::X509CertRequestParams rcac_request = { 0, mNow, mNow + mValidity, rcac_dn, rcac_dn };
    ReturnErrorOnFailure(NewRootX509Cert(rcac_request, mIssuer, rcac));

    VerifyOrReturnError(CanCastTo<uint16_t>(rcac.size()), CHIP_ERROR_INTERNAL);
    PERSISTENT_KEY_OP(fabricId, kOperationalCredentialsRootCertificateStorage, key,
                      err = SyncSetKeyValue(key, rcac.data(), static_cast<uint16_t>(rcac.size())));

    return err;
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

    NodeId assignedId;
    if (mNodeIdRequested)
    {
        assignedId       = mNextRequestedNodeId;
        mNodeIdRequested = false;
    }
    else
    {
        assignedId = mNextAvailableNodeId++;
    }

    TLVReader reader;
    reader.Init(csrElements);

    if (reader.GetType() == kTLVType_NotSpecified)
    {
        ReturnErrorOnFailure(reader.Next());
    }

    VerifyOrReturnError(reader.GetType() == kTLVType_Structure, CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrReturnError(reader.GetTag() == AnonymousTag(), CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

    TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(kTLVType_ByteString, TLV::ContextTag(1)));

    ByteSpan csr(reader.GetReadPoint(), reader.GetLength());
    reader.ExitContainer(containerType);

    P256PublicKey pubkey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey));

    ChipLogProgress(chipTool, "VerifyCertificateSigningRequest");

    Platform::ScopedMemoryBuffer<uint8_t> noc;
    ReturnErrorCodeIf(!noc.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan nocSpan(noc.Get(), kMaxCHIPDERCertLength);

    Platform::ScopedMemoryBuffer<uint8_t> rcac;
    ReturnErrorCodeIf(!rcac.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan rcacSpan(rcac.Get(), kMaxCHIPDERCertLength);

    MutableByteSpan icacSpan;

    ReturnErrorOnFailure(GenerateNOCChainAfterValidation(assignedId, mNextFabricId, pubkey, rcacSpan, icacSpan, nocSpan));

    onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, nocSpan, ByteSpan(), rcacSpan, Optional<AesCcm128KeySpan>(),
                        Optional<NodeId>());

    jbyteArray javaCsr;
    JniReferences::GetInstance().GetEnvForCurrentThread()->ExceptionClear();
    JniReferences::GetInstance().N2J_ByteArray(JniReferences::GetInstance().GetEnvForCurrentThread(), csrElements.data(),
                                               csrElements.size(), javaCsr);
    JniReferences::GetInstance().GetEnvForCurrentThread()->CallVoidMethod(mJavaObjectRef, method, javaCsr);
    return CHIP_NO_ERROR;
}

AndroidDeviceControllerWrapper * AndroidDeviceControllerWrapper::AllocateNew(
    JavaVM * vm, jobject deviceControllerObj, chip::NodeId nodeId, chip::System::Layer * systemLayer,
    chip::Inet::EndPointManager<Inet::TCPEndPoint> * tcpEndPointManager,
    chip::Inet::EndPointManager<Inet::UDPEndPoint> * udpEndPointManager, CHIP_ERROR * errInfoOnFailure)
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
    if (tcpEndPointManager == nullptr)
    {
        ChipLogError(Controller, "Missing TCP layer");
        *errInfoOnFailure = CHIP_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }
    if (udpEndPointManager == nullptr)
    {
        ChipLogError(Controller, "Missing UDP layer");
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
    std::unique_ptr<AndroidDeviceControllerWrapper> wrapper(new AndroidDeviceControllerWrapper(std::move(controller)));

    wrapper->SetJavaObjectRef(vm, deviceControllerObj);

    // Initialize device attestation verifier
    // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
    const chip::Credentials::AttestationTrustStore * testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
    SetDeviceAttestationVerifier(GetDefaultDACVerifier(testingRootStore));

    chip::Controller::FactoryInitParams initParams;
    chip::Controller::SetupParams setupParams;

    initParams.systemLayer        = systemLayer;
    initParams.tcpEndPointManager = tcpEndPointManager;
    initParams.udpEndPointManager = udpEndPointManager;
    initParams.fabricStorage      = wrapper.get();
    // move bleLayer into platform/android to share with app server
#if CONFIG_NETWORK_LAYER_BLE
    initParams.bleLayer = DeviceLayer::ConnectivityMgr().GetBleLayer();
#endif
    initParams.listenPort                      = CHIP_PORT + 1;
    setupParams.storageDelegate                = wrapper.get();
    setupParams.pairingDelegate                = wrapper.get();
    setupParams.operationalCredentialsDelegate = wrapper.get();

    wrapper->InitializeOperationalCredentialsIssuer();

    Platform::ScopedMemoryBuffer<uint8_t> noc;
    if (!noc.Alloc(kMaxCHIPDERCertLength))
    {
        *errInfoOnFailure = CHIP_ERROR_NO_MEMORY;
        return nullptr;
    }
    MutableByteSpan nocSpan(noc.Get(), kMaxCHIPDERCertLength);

    MutableByteSpan icacSpan;

    Platform::ScopedMemoryBuffer<uint8_t> rcac;
    if (!rcac.Alloc(kMaxCHIPDERCertLength))
    {
        *errInfoOnFailure = CHIP_ERROR_NO_MEMORY;
        return nullptr;
    }
    MutableByteSpan rcacSpan(rcac.Get(), kMaxCHIPDERCertLength);

    Crypto::P256Keypair ephemeralKey;
    *errInfoOnFailure = ephemeralKey.Initialize();
    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    *errInfoOnFailure = wrapper->GenerateNOCChainAfterValidation(nodeId, 0, ephemeralKey.Pubkey(), rcacSpan, icacSpan, nocSpan);
    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    setupParams.operationalKeypair = &ephemeralKey;
    setupParams.controllerRCAC     = rcacSpan;
    setupParams.controllerICAC     = icacSpan;
    setupParams.controllerNOC      = nocSpan;

    *errInfoOnFailure = DeviceControllerFactory::GetInstance().Init(initParams);
    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        return nullptr;
    }
    *errInfoOnFailure = DeviceControllerFactory::GetInstance().SetupCommissioner(setupParams, *wrapper->Controller());
    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    return wrapper.release();
}

CHIP_ERROR AndroidDeviceControllerWrapper::ApplyNetworkCredentials(chip::Controller::CommissioningParameters & params,
                                                                   jobject networkCredentials)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Retrieve WiFi or Thread credentials from the NetworkCredentials Java object, and set them in the commissioning params.
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID getWiFiCredentials;
    err = chip::JniReferences::GetInstance().FindMethod(env, networkCredentials, "getWiFiCredentials",
                                                        "()Lchip/devicecontroller/NetworkCredentials$WiFiCredentials;",
                                                        &getWiFiCredentials);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);
    jobject wifiCredentialsJava = env->CallObjectMethod(networkCredentials, getWiFiCredentials);

    jmethodID getThreadCredentials;
    err = chip::JniReferences::GetInstance().FindMethod(env, networkCredentials, "getThreadCredentials",
                                                        "()Lchip/devicecontroller/NetworkCredentials$ThreadCredentials;",
                                                        &getThreadCredentials);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);
    jobject threadCredentialsJava = env->CallObjectMethod(networkCredentials, getThreadCredentials);

    if (wifiCredentialsJava != nullptr)
    {
        jmethodID getSsid;
        jmethodID getPassword;
        err = chip::JniReferences::GetInstance().FindMethod(env, wifiCredentialsJava, "getSsid", "()Ljava/lang/String;", &getSsid);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);
        err = chip::JniReferences::GetInstance().FindMethod(env, wifiCredentialsJava, "getPassword", "()Ljava/lang/String;",
                                                            &getPassword);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);
        ssidStr = static_cast<jstring>(env->NewGlobalRef(env->CallObjectMethod(wifiCredentialsJava, getSsid)));
        VerifyOrReturnError(ssidStr != nullptr && !env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);
        passwordStr = static_cast<jstring>(env->NewGlobalRef(env->CallObjectMethod(wifiCredentialsJava, getPassword)));
        VerifyOrReturnError(ssidStr != nullptr && !env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        ssid                 = env->GetStringUTFChars(ssidStr, 0);
        password             = env->GetStringUTFChars(passwordStr, 0);
        jsize ssidLength     = env->GetStringUTFLength(ssidStr);
        jsize passwordLength = env->GetStringUTFLength(passwordStr);

        params.SetWiFiCredentials(
            WiFiCredentials(chip::ByteSpan(reinterpret_cast<const uint8_t *>(ssid), static_cast<size_t>(ssidLength)),
                            chip::ByteSpan(reinterpret_cast<const uint8_t *>(password), static_cast<size_t>(passwordLength))));
    }
    else if (threadCredentialsJava != nullptr)
    {
        jmethodID getOperationalDataset;
        err = chip::JniReferences::GetInstance().FindMethod(env, threadCredentialsJava, "getOperationalDataset", "()[B",
                                                            &getOperationalDataset);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);
        operationalDatasetBytes =
            static_cast<jbyteArray>(env->NewGlobalRef(env->CallObjectMethod(threadCredentialsJava, getOperationalDataset)));
        VerifyOrReturnError(operationalDatasetBytes != nullptr && !env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        operationalDataset = env->GetByteArrayElements(operationalDatasetBytes, nullptr);
        jsize length       = env->GetArrayLength(operationalDatasetBytes);

        params.SetThreadOperationalDataset(
            chip::ByteSpan(reinterpret_cast<const uint8_t *>(operationalDataset), static_cast<size_t>(length)));
    }
    else
    {
        ChipLogError(Controller, "Both WiFi and Thread credentials were null in NetworkCredentials");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return err;
}

void AndroidDeviceControllerWrapper::OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status)
{
    chip::DeviceLayer::StackUnlock unlock;
    CallJavaMethod("onStatusUpdate", static_cast<jint>(status));
}

void AndroidDeviceControllerWrapper::OnPairingComplete(CHIP_ERROR error)
{
    chip::DeviceLayer::StackUnlock unlock;
    CallJavaMethod("onPairingComplete", static_cast<jint>(error.AsInteger()));
}

void AndroidDeviceControllerWrapper::OnPairingDeleted(CHIP_ERROR error)
{
    chip::DeviceLayer::StackUnlock unlock;
    CallJavaMethod("onPairingDeleted", static_cast<jint>(error.AsInteger()));
}

void AndroidDeviceControllerWrapper::OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error)
{
    chip::DeviceLayer::StackUnlock unlock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID onCommissioningCompleteMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, mJavaObjectRef, "onCommissioningComplete", "(JI)V",
                                                             &onCommissioningCompleteMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error finding Java method: %" CHIP_ERROR_FORMAT, err.Format()));
    env->CallVoidMethod(mJavaObjectRef, onCommissioningCompleteMethod, static_cast<jlong>(deviceId), error.AsInteger());

    if (ssidStr != nullptr)
    {
        env->ReleaseStringUTFChars(ssidStr, ssid);
        env->DeleteGlobalRef(ssidStr);
    }
    if (passwordStr != nullptr)
    {
        env->ReleaseStringUTFChars(passwordStr, password);
        env->DeleteGlobalRef(passwordStr);
    }
    if (operationalDatasetBytes != nullptr)
    {
        env->ReleaseByteArrayElements(operationalDatasetBytes, operationalDataset, 0);
        env->DeleteGlobalRef(operationalDatasetBytes);
    }
}

CHIP_ERROR AndroidDeviceControllerWrapper::InitializeOperationalCredentialsIssuer()
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

CHIP_ERROR AndroidDeviceControllerWrapper::SyncStore(chip::FabricIndex fabricIndex, const char * key, const void * buffer,
                                                     uint16_t size)
{
    return SyncSetKeyValue(key, buffer, size);
};

CHIP_ERROR AndroidDeviceControllerWrapper::SyncLoad(chip::FabricIndex fabricIndex, const char * key, void * buffer, uint16_t & size)
{
    return SyncGetKeyValue(key, buffer, size);
};

CHIP_ERROR AndroidDeviceControllerWrapper::SyncDelete(chip::FabricIndex fabricIndex, const char * key)
{
    return SyncDeleteKeyValue(key);
};
