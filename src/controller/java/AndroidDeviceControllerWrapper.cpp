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
#include <lib/support/JniTypeWrappers.h>

#include <controller/CHIPDeviceControllerFactory.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/android/CHIPP256KeypairBridge.h>

using namespace chip;
using namespace chip::Controller;
using namespace chip::Credentials;
using namespace TLV;

AndroidDeviceControllerWrapper::~AndroidDeviceControllerWrapper()
{
    if ((mJavaVM != nullptr) && (mJavaObjectRef != nullptr))
    {
        JniReferences::GetInstance().GetEnvForCurrentThread()->DeleteGlobalRef(mJavaObjectRef);
    }
    mController->Shutdown();

    if (mKeypairBridge != nullptr)
    {
        chip::Platform::Delete(mKeypairBridge);
        mKeypairBridge = nullptr;
    }
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

AndroidDeviceControllerWrapper * AndroidDeviceControllerWrapper::AllocateNew(
    JavaVM * vm, jobject deviceControllerObj, chip::NodeId nodeId, const chip::CATValues & cats, chip::System::Layer * systemLayer,
    chip::Inet::EndPointManager<Inet::TCPEndPoint> * tcpEndPointManager,
    chip::Inet::EndPointManager<Inet::UDPEndPoint> * udpEndPointManager, AndroidOperationalCredentialsIssuerPtr opCredsIssuerPtr,
    jobject keypairDelegate, jbyteArray rootCertificate, jbyteArray intermediateCertificate, jbyteArray nodeOperationalCertificate,
    jbyteArray ipkEpochKey, uint16_t listenPort, CHIP_ERROR * errInfoOnFailure)
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

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Controller, "Failed to retrieve JNIEnv.");
        *errInfoOnFailure = CHIP_ERROR_INCORRECT_STATE;
        return nullptr;
    }

    std::unique_ptr<DeviceCommissioner> controller(new DeviceCommissioner());

    if (!controller)
    {
        *errInfoOnFailure = CHIP_ERROR_NO_MEMORY;
        return nullptr;
    }
    std::unique_ptr<AndroidDeviceControllerWrapper> wrapper(
        new AndroidDeviceControllerWrapper(std::move(controller), std::move(opCredsIssuerPtr)));

    chip::PersistentStorageDelegate * wrapperStorage = wrapper.get();

    wrapper->SetJavaObjectRef(vm, deviceControllerObj);

    chip::Controller::AndroidOperationalCredentialsIssuer * opCredsIssuer = wrapper->mOpCredsIssuer.get();

    // Initialize device attestation verifier
    // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
    const chip::Credentials::AttestationTrustStore * testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
    SetDeviceAttestationVerifier(GetDefaultDACVerifier(testingRootStore));

    chip::Controller::FactoryInitParams initParams;
    chip::Controller::SetupParams setupParams;

    initParams.systemLayer        = systemLayer;
    initParams.tcpEndPointManager = tcpEndPointManager;
    initParams.udpEndPointManager = udpEndPointManager;

    // move bleLayer into platform/android to share with app server
#if CONFIG_NETWORK_LAYER_BLE
    initParams.bleLayer = DeviceLayer::ConnectivityMgr().GetBleLayer();
#endif
    initParams.listenPort                      = listenPort;
    setupParams.pairingDelegate                = wrapper.get();
    setupParams.operationalCredentialsDelegate = opCredsIssuer;
    initParams.fabricIndependentStorage        = wrapperStorage;

    wrapper->mGroupDataProvider.SetStorageDelegate(wrapperStorage);

    CHIP_ERROR err = wrapper->mGroupDataProvider.Init();
    if (err != CHIP_NO_ERROR)
    {
        *errInfoOnFailure = err;
        return nullptr;
    }
    initParams.groupDataProvider = &wrapper->mGroupDataProvider;

    err = wrapper->mOpCertStore.Init(wrapperStorage);
    if (err != CHIP_NO_ERROR)
    {
        *errInfoOnFailure = err;
        return nullptr;
    }
    initParams.opCertStore = &wrapper->mOpCertStore;

    // TODO: Init IPK Epoch Key in opcreds issuer, so that commissionees get the right IPK
    opCredsIssuer->Initialize(*wrapper.get(), wrapper.get()->mJavaObjectRef);

    Platform::ScopedMemoryBuffer<uint8_t> noc;
    if (!noc.Alloc(kMaxCHIPDERCertLength))
    {
        *errInfoOnFailure = CHIP_ERROR_NO_MEMORY;
        return nullptr;
    }
    MutableByteSpan nocSpan(noc.Get(), kMaxCHIPDERCertLength);

    Platform::ScopedMemoryBuffer<uint8_t> icac;
    if (!icac.Alloc(kMaxCHIPDERCertLength))
    {
        *errInfoOnFailure = CHIP_ERROR_NO_MEMORY;
        return nullptr;
    }

    MutableByteSpan icacSpan(icac.Get(), kMaxCHIPDERCertLength);

    Platform::ScopedMemoryBuffer<uint8_t> rcac;
    if (!rcac.Alloc(kMaxCHIPDERCertLength))
    {
        *errInfoOnFailure = CHIP_ERROR_NO_MEMORY;
        return nullptr;
    }
    MutableByteSpan rcacSpan(rcac.Get(), kMaxCHIPDERCertLength);

    // The lifetime of the ephemeralKey variable must be kept until SetupParams is saved.
    Crypto::P256Keypair ephemeralKey;

    if (rootCertificate != nullptr && intermediateCertificate != nullptr && nodeOperationalCertificate != nullptr &&
        keypairDelegate != nullptr)
    {
        CHIPP256KeypairBridge * nativeKeypairBridge = wrapper->GetP256KeypairBridge();
        nativeKeypairBridge->SetDelegate(keypairDelegate);
        *errInfoOnFailure = nativeKeypairBridge->Initialize();
        if (*errInfoOnFailure != CHIP_NO_ERROR)
        {
            return nullptr;
        }

        setupParams.operationalKeypair                   = nativeKeypairBridge;
        setupParams.hasExternallyOwnedOperationalKeypair = true;

        JniByteArray jniRcac(env, rootCertificate);
        JniByteArray jniIcac(env, intermediateCertificate);
        JniByteArray jniNoc(env, nodeOperationalCertificate);

        setupParams.controllerRCAC = jniRcac.byteSpan();
        setupParams.controllerICAC = jniIcac.byteSpan();
        setupParams.controllerNOC  = jniNoc.byteSpan();
    }
    else
    {
        *errInfoOnFailure = ephemeralKey.Initialize();
        if (*errInfoOnFailure != CHIP_NO_ERROR)
        {
            return nullptr;
        }
        setupParams.operationalKeypair                   = &ephemeralKey;
        setupParams.hasExternallyOwnedOperationalKeypair = false;

        *errInfoOnFailure = opCredsIssuer->GenerateNOCChainAfterValidation(nodeId,
                                                                           /* fabricId = */ 1, cats, ephemeralKey.Pubkey(),
                                                                           rcacSpan, icacSpan, nocSpan);

        if (*errInfoOnFailure != CHIP_NO_ERROR)
        {
            return nullptr;
        }

        setupParams.controllerRCAC = rcacSpan;
        setupParams.controllerICAC = icacSpan;
        setupParams.controllerNOC  = nocSpan;
    }

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

    // Setup IPK
    uint8_t compressedFabricId[sizeof(uint64_t)] = { 0 };
    chip::MutableByteSpan compressedFabricIdSpan(compressedFabricId);

    *errInfoOnFailure = wrapper->Controller()->GetCompressedFabricIdBytes(compressedFabricIdSpan);
    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        return nullptr;
    }
    ChipLogProgress(Support, "Setting up group data for Fabric Index %u with Compressed Fabric ID:",
                    static_cast<unsigned>(wrapper->Controller()->GetFabricIndex()));
    ChipLogByteSpan(Support, compressedFabricIdSpan);

    chip::ByteSpan ipkSpan;
    if (ipkEpochKey != nullptr)
    {
        JniByteArray jniIpk(env, ipkEpochKey);
        ipkSpan = jniIpk.byteSpan();
    }
    else
    {
        ipkSpan = chip::GroupTesting::DefaultIpkValue::GetDefaultIpk();
    }

    *errInfoOnFailure = chip::Credentials::SetSingleIpkEpochKey(
        &wrapper->mGroupDataProvider, wrapper->Controller()->GetFabricIndex(), ipkSpan, compressedFabricIdSpan);
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
