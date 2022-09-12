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
#include <vector>

#include <string.h>

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
    JavaVM * vm, jobject deviceControllerObj, chip::NodeId nodeId, chip::FabricId fabricId, const chip::CATValues & cats,
    chip::System::Layer * systemLayer, chip::Inet::EndPointManager<Inet::TCPEndPoint> * tcpEndPointManager,
    chip::Inet::EndPointManager<Inet::UDPEndPoint> * udpEndPointManager, AndroidOperationalCredentialsIssuerPtr opCredsIssuerPtr,
    jobject keypairDelegate, jbyteArray rootCertificate, jbyteArray intermediateCertificate, jbyteArray nodeOperationalCertificate,
    jbyteArray ipkEpochKey, uint16_t listenPort, uint16_t controllerVendorId, uint16_t failsafeTimerSeconds,
    bool attemptNetworkScanWiFi, bool attemptNetworkScanThread, bool skipCommissioningComplete, CHIP_ERROR * errInfoOnFailure)
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
    setupParams.controllerVendorId             = static_cast<chip::VendorId>(controllerVendorId);
    setupParams.pairingDelegate                = wrapper.get();
    setupParams.operationalCredentialsDelegate = opCredsIssuer;
    setupParams.defaultCommissioner            = &wrapper->mAutoCommissioner;
    initParams.fabricIndependentStorage        = wrapperStorage;

    wrapper->mGroupDataProvider.SetStorageDelegate(wrapperStorage);

    CommissioningParameters params = wrapper->mAutoCommissioner.GetCommissioningParameters();
    params.SetFailsafeTimerSeconds(failsafeTimerSeconds);
    params.SetAttemptWiFiNetworkScan(attemptNetworkScanWiFi);
    params.SetAttemptThreadNetworkScan(attemptNetworkScanThread);
    params.SetSkipCommissioningComplete(skipCommissioningComplete);
    wrapper->UpdateCommissioningParameters(params);

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
    opCredsIssuer->Initialize(*wrapper.get(), &wrapper->mAutoCommissioner, wrapper.get()->mJavaObjectRef);

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

    if (rootCertificate != nullptr && nodeOperationalCertificate != nullptr && keypairDelegate != nullptr)
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
        JniByteArray jniNoc(env, nodeOperationalCertificate);

        // Make copies of the cert that outlive the scope so that future factor init does not
        // cause loss of scope from the JNI refs going away. Also, this keeps the certs
        // handy for debugging commissioner init.
        wrapper->mRcacCertificate = std::vector<uint8_t>(jniRcac.byteSpan().begin(), jniRcac.byteSpan().end());

        // Intermediate cert could be missing. Let's only copy it if present
        wrapper->mIcacCertificate.clear();
        if (intermediateCertificate != nullptr)
        {
            JniByteArray jniIcac(env, intermediateCertificate);
            wrapper->mIcacCertificate = std::vector<uint8_t>(jniIcac.byteSpan().begin(), jniIcac.byteSpan().end());
        }

        wrapper->mNocCertificate = std::vector<uint8_t>(jniNoc.byteSpan().begin(), jniNoc.byteSpan().end());

        setupParams.controllerRCAC = chip::ByteSpan(wrapper->mRcacCertificate.data(), wrapper->mRcacCertificate.size());
        setupParams.controllerICAC = chip::ByteSpan(wrapper->mIcacCertificate.data(), wrapper->mIcacCertificate.size());
        setupParams.controllerNOC  = chip::ByteSpan(wrapper->mNocCertificate.data(), wrapper->mNocCertificate.size());
    }
    else
    {
        ChipLogProgress(Controller,
                        "No existing credentials provided: generating ephemeral local NOC chain with OperationalCredentialsIssuer");

        *errInfoOnFailure = ephemeralKey.Initialize();
        if (*errInfoOnFailure != CHIP_NO_ERROR)
        {
            return nullptr;
        }
        setupParams.operationalKeypair                   = &ephemeralKey;
        setupParams.hasExternallyOwnedOperationalKeypair = false;

        *errInfoOnFailure = opCredsIssuer->GenerateNOCChainAfterValidation(nodeId, fabricId, cats, ephemeralKey.Pubkey(), rcacSpan,
                                                                           icacSpan, nocSpan);

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
    ChipLogProgress(Controller, "Setting up group data for Fabric Index %u with Compressed Fabric ID:",
                    static_cast<unsigned>(wrapper->Controller()->GetFabricIndex()));
    ChipLogByteSpan(Support, compressedFabricIdSpan);

    chip::ByteSpan ipkSpan;
    std::vector<uint8_t> ipkBuffer;
    if (ipkEpochKey != nullptr)
    {
        JniByteArray jniIpk(env, ipkEpochKey);
        ipkBuffer = std::vector<uint8_t>(jniIpk.byteSpan().begin(), jniIpk.byteSpan().end());
        ipkSpan   = chip::ByteSpan(ipkBuffer.data(), ipkBuffer.size());
    }
    else
    {
        ipkSpan = chip::GroupTesting::DefaultIpkValue::GetDefaultIpk();
    }

    *errInfoOnFailure = chip::Credentials::SetSingleIpkEpochKey(
        &wrapper->mGroupDataProvider, wrapper->Controller()->GetFabricIndex(), ipkSpan, compressedFabricIdSpan);

    memset(ipkBuffer.data(), 0, ipkBuffer.size());

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

CHIP_ERROR AndroidDeviceControllerWrapper::UpdateCommissioningParameters(const chip::Controller::CommissioningParameters & params)
{
    // this will wipe out any custom attestationNonce and csrNonce that was being used.
    // however, Android APIs don't allow these to be set to custom values today.
    return mAutoCommissioner.SetCommissioningParameters(params);
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

void AndroidDeviceControllerWrapper::OnCommissioningStatusUpdate(PeerId peerId, chip::Controller::CommissioningStage stageCompleted,
                                                                 CHIP_ERROR error)
{
    chip::DeviceLayer::StackUnlock unlock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID onCommissioningStatusUpdateMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, mJavaObjectRef, "onCommissioningStatusUpdate",
                                                             "(JLjava/lang/String;I)V", &onCommissioningStatusUpdateMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error finding Java method: %" CHIP_ERROR_FORMAT, err.Format()));

    UtfString jStageCompleted(env, StageToString(stageCompleted));
    env->CallVoidMethod(mJavaObjectRef, onCommissioningStatusUpdateMethod, static_cast<jlong>(peerId.GetNodeId()),
                        jStageCompleted.jniValue(), error.AsInteger());
}

void AndroidDeviceControllerWrapper::OnReadCommissioningInfo(const chip::Controller::ReadCommissioningInfo & info)
{
    // calls: onReadCommissioningInfo(int vendorId, int productId, int wifiEndpointId, int threadEndpointId)
    chip::DeviceLayer::StackUnlock unlock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID onReadCommissioningInfoMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, mJavaObjectRef, "onReadCommissioningInfo", "(IIII)V",
                                                             &onReadCommissioningInfoMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error finding Java method: %" CHIP_ERROR_FORMAT, err.Format()));

    env->CallVoidMethod(mJavaObjectRef, onReadCommissioningInfoMethod, static_cast<jint>(info.basic.vendorId),
                        static_cast<jint>(info.basic.productId), static_cast<jint>(info.network.wifi.endpoint),
                        static_cast<jint>(info.network.thread.endpoint));
}

void AndroidDeviceControllerWrapper::OnScanNetworksSuccess(
    const chip::app::Clusters::NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType & dataResponse)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID javaMethod;

    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Error invoking Java callback: no JNIEnv"));

    err = JniReferences::GetInstance().FindMethod(
        env, mJavaObjectRef, "onScanNetworksSuccess",
        "(Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error invoking Java callback: %s", ErrorStr(err)));

    jobject NetworkingStatus;
    std::string NetworkingStatusClassName     = "java/lang/Integer";
    std::string NetworkingStatusCtorSignature = "(I)V";
    chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
        NetworkingStatusClassName.c_str(), NetworkingStatusCtorSignature.c_str(),
        static_cast<uint8_t>(dataResponse.networkingStatus), NetworkingStatus);
    jobject DebugText;
    if (!dataResponse.debugText.HasValue())
    {
        chip::JniReferences::GetInstance().CreateOptional(nullptr, DebugText);
    }
    else
    {
        jobject DebugTextInsideOptional;
        DebugTextInsideOptional =
            env->NewStringUTF(std::string(dataResponse.debugText.Value().data(), dataResponse.debugText.Value().size()).c_str());
        chip::JniReferences::GetInstance().CreateOptional(DebugTextInsideOptional, DebugText);
    }
    jobject WiFiScanResults;
    if (!dataResponse.wiFiScanResults.HasValue())
    {
        chip::JniReferences::GetInstance().CreateOptional(nullptr, WiFiScanResults);
    }
    else
    {
        // TODO: use this
        jobject WiFiScanResultsInsideOptional;
        chip::JniReferences::GetInstance().CreateArrayList(WiFiScanResultsInsideOptional);

        auto iter_WiFiScanResultsInsideOptional = dataResponse.wiFiScanResults.Value().begin();
        while (iter_WiFiScanResultsInsideOptional.Next())
        {
            auto & entry = iter_WiFiScanResultsInsideOptional.GetValue();
            jobject newElement_security;
            std::string newElement_securityClassName     = "java/lang/Integer";
            std::string newElement_securityCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_securityClassName.c_str(),
                                                                          newElement_securityCtorSignature.c_str(),
                                                                          entry.security.Raw(), newElement_security);
            jobject newElement_ssid;
            jbyteArray newElement_ssidByteArray = env->NewByteArray(static_cast<jsize>(entry.ssid.size()));
            env->SetByteArrayRegion(newElement_ssidByteArray, 0, static_cast<jsize>(entry.ssid.size()),
                                    reinterpret_cast<const jbyte *>(entry.ssid.data()));
            newElement_ssid = newElement_ssidByteArray;
            jobject newElement_bssid;
            jbyteArray newElement_bssidByteArray = env->NewByteArray(static_cast<jsize>(entry.bssid.size()));
            env->SetByteArrayRegion(newElement_bssidByteArray, 0, static_cast<jsize>(entry.bssid.size()),
                                    reinterpret_cast<const jbyte *>(entry.bssid.data()));
            newElement_bssid = newElement_bssidByteArray;
            jobject newElement_channel;
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>("java/lang/Integer", "(I)V", entry.channel,
                                                                           newElement_channel);
            jobject newElement_wiFiBand;
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                "java/lang/Integer", "(I)V", static_cast<uint8_t>(entry.wiFiBand), newElement_wiFiBand);
            jobject newElement_rssi;
            chip::JniReferences::GetInstance().CreateBoxedObject<int8_t>("java/lang/Integer", "(I)V", entry.rssi, newElement_rssi);

            jclass wiFiInterfaceScanResultStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipStructs$NetworkCommissioningClusterWiFiInterfaceScanResult",
                wiFiInterfaceScanResultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipStructs$NetworkCommissioningClusterWiFiInterfaceScanResult");
                return;
            }
            jmethodID wiFiInterfaceScanResultStructCtor =
                env->GetMethodID(wiFiInterfaceScanResultStructClass, "<init>",
                                 "(Ljava/lang/Integer;[B[BLjava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;)V");
            if (wiFiInterfaceScanResultStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipStructs$NetworkCommissioningClusterWiFiInterfaceScanResult constructor");
                return;
            }

            jobject newElement =
                env->NewObject(wiFiInterfaceScanResultStructClass, wiFiInterfaceScanResultStructCtor, newElement_security,
                               newElement_ssid, newElement_bssid, newElement_channel, newElement_wiFiBand, newElement_rssi);
            chip::JniReferences::GetInstance().AddToList(WiFiScanResultsInsideOptional, newElement);
        }
        chip::JniReferences::GetInstance().CreateOptional(WiFiScanResultsInsideOptional, WiFiScanResults);
    }
    jobject ThreadScanResults;
    if (!dataResponse.threadScanResults.HasValue())
    {
        chip::JniReferences::GetInstance().CreateOptional(nullptr, ThreadScanResults);
    }
    else
    {
        jobject ThreadScanResultsInsideOptional;
        chip::JniReferences::GetInstance().CreateArrayList(ThreadScanResultsInsideOptional);

        auto iter_ThreadScanResultsInsideOptional = dataResponse.threadScanResults.Value().begin();
        while (iter_ThreadScanResultsInsideOptional.Next())
        {
            auto & entry = iter_ThreadScanResultsInsideOptional.GetValue();
            jobject newElement_panId;
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>("java/lang/Integer", "(I)V", entry.panId,
                                                                           newElement_panId);
            jobject newElement_extendedPanId;
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>("java/lang/Long", "(J)V", entry.extendedPanId,
                                                                           newElement_extendedPanId);
            jobject newElement_networkName;
            newElement_networkName = env->NewStringUTF(std::string(entry.networkName.data(), entry.networkName.size()).c_str());
            jobject newElement_channel;
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>("java/lang/Integer", "(I)V", entry.channel,
                                                                           newElement_channel);
            jobject newElement_version;
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>("java/lang/Integer", "(I)V", entry.version,
                                                                          newElement_version);
            jobject newElement_extendedAddress;
            jbyteArray newElement_extendedAddressByteArray = env->NewByteArray(static_cast<jsize>(entry.extendedAddress.size()));
            env->SetByteArrayRegion(newElement_extendedAddressByteArray, 0, static_cast<jsize>(entry.extendedAddress.size()),
                                    reinterpret_cast<const jbyte *>(entry.extendedAddress.data()));
            newElement_extendedAddress = newElement_extendedAddressByteArray;
            jobject newElement_rssi;
            chip::JniReferences::GetInstance().CreateBoxedObject<int8_t>("java/lang/Integer", "(I)V", entry.rssi, newElement_rssi);
            jobject newElement_lqi;
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>("java/lang/Integer", "(I)V", entry.lqi, newElement_lqi);

            jclass threadInterfaceScanResultStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipStructs$NetworkCommissioningClusterThreadInterfaceScanResult",
                threadInterfaceScanResultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipStructs$NetworkCommissioningClusterThreadInterfaceScanResult");
                return;
            }
            jmethodID threadInterfaceScanResultStructCtor =
                env->GetMethodID(threadInterfaceScanResultStructClass, "<init>",
                                 "(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/String;Ljava/lang/Integer;Ljava/lang/"
                                 "Integer;[BLjava/lang/Integer;Ljava/lang/Integer;)V");
            if (threadInterfaceScanResultStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipStructs$NetworkCommissioningClusterThreadInterfaceScanResult constructor");
                return;
            }

            jobject newElement =
                env->NewObject(threadInterfaceScanResultStructClass, threadInterfaceScanResultStructCtor, newElement_panId,
                               newElement_extendedPanId, newElement_networkName, newElement_channel, newElement_version,
                               newElement_extendedAddress, newElement_rssi, newElement_lqi);
            chip::JniReferences::GetInstance().AddToList(ThreadScanResultsInsideOptional, newElement);
        }
        chip::JniReferences::GetInstance().CreateOptional(ThreadScanResultsInsideOptional, ThreadScanResults);
    }

    env->CallVoidMethod(mJavaObjectRef, javaMethod, NetworkingStatus, DebugText, WiFiScanResults, ThreadScanResults);
}

void AndroidDeviceControllerWrapper::OnScanNetworksFailure(CHIP_ERROR error)
{
    chip::DeviceLayer::StackUnlock unlock;

    CallJavaMethod("onScanNetworksFailure", static_cast<jint>(error.AsInteger()));
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
