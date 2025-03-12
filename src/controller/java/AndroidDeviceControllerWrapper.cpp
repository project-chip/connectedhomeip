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
#include <string>
#include <vector>

#include <string.h>

#include <app/server/Dnssd.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/java/AndroidICDClient.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/KeyValueStoreManager.h>

using namespace chip;
using namespace chip::Controller;
using namespace chip::Credentials;
using namespace TLV;

AndroidDeviceControllerWrapper::~AndroidDeviceControllerWrapper()
{
    Shutdown();
}

void AndroidDeviceControllerWrapper::SetJavaObjectRef(JavaVM * vm, jobject obj)
{
    mJavaVM = vm;
    if (mJavaObjectRef.Init(obj) != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Fail to init mJavaObjectRef");
    }
}

void AndroidDeviceControllerWrapper::CallJavaIntMethod(const char * methodName, jint argument)
{
    JniReferences::GetInstance().CallVoidInt(JniReferences::GetInstance().GetEnvForCurrentThread(), mJavaObjectRef.ObjectRef(),
                                             methodName, argument);
}

void AndroidDeviceControllerWrapper::CallJavaLongMethod(const char * methodName, jlong argument)
{
    JniReferences::GetInstance().CallVoidLong(JniReferences::GetInstance().GetEnvForCurrentThread(), mJavaObjectRef.ObjectRef(),
                                              methodName, argument);
}

AndroidDeviceControllerWrapper * AndroidDeviceControllerWrapper::AllocateNew(
    JavaVM * vm, jobject deviceControllerObj, chip::NodeId nodeId, chip::FabricId fabricId, const chip::CATValues & cats,
    chip::System::Layer * systemLayer, chip::Inet::EndPointManager<Inet::TCPEndPoint> * tcpEndPointManager,
    chip::Inet::EndPointManager<Inet::UDPEndPoint> * udpEndPointManager,
#ifdef JAVA_MATTER_CONTROLLER_TEST
    ExampleOperationalCredentialsIssuerPtr opCredsIssuerPtr,
#else
    AndroidOperationalCredentialsIssuerPtr opCredsIssuerPtr,
#endif
    jobject keypairDelegate, jbyteArray rootCertificate, jbyteArray intermediateCertificate, jbyteArray nodeOperationalCertificate,
    jbyteArray ipkEpochKey, uint16_t listenPort, uint16_t controllerVendorId, uint16_t failsafeTimerSeconds,
    bool attemptNetworkScanWiFi, bool attemptNetworkScanThread, bool skipCommissioningComplete,
    bool skipAttestationCertificateValidation, jstring countryCode, bool enableServerInteractions, CHIP_ERROR * errInfoOnFailure)
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

#ifdef JAVA_MATTER_CONTROLLER_TEST
    if (wrapper->mExampleStorage.Init() != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Init Storage failure");
        return nullptr;
    }
    chip::PersistentStorageDelegate * wrapperStorage = &wrapper->mExampleStorage;
    wrapper->SetJavaObjectRef(vm, deviceControllerObj);
    chip::Controller::ExampleOperationalCredentialsIssuer * opCredsIssuer = wrapper->mOpCredsIssuer.get();
#else
    chip::PersistentStorageDelegate * wrapperStorage = wrapper.get();

    wrapper->SetJavaObjectRef(vm, deviceControllerObj);

    chip::Controller::AndroidOperationalCredentialsIssuer * opCredsIssuer = wrapper->mOpCredsIssuer.get();
#endif

    // Initialize device attestation verifier
    if (skipAttestationCertificateValidation)
    {
        chip::Credentials::SetDeviceAttestationVerifier(wrapper->GetPartialDACVerifier());
    }
    else
    {
        const chip::Credentials::AttestationTrustStore * testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
        chip::Credentials::SetDeviceAttestationVerifier(GetDefaultDACVerifier(testingRootStore));
    }

    // Because garbage collection may delay the removal of old controller instances, two instances could temporarily exist.
    // To avoid issues with the shared ICD client storage, reset the storage before creating a new controller.
    getICDClientStorage()->Shutdown();
    *errInfoOnFailure = getICDClientStorage()->Init(wrapperStorage, &wrapper->mSessionKeystore);
    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "ICD Client Storage failure");
        return nullptr;
    }

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
    initParams.sessionKeystore                 = &wrapper->mSessionKeystore;
    initParams.dataModelProvider               = app::CodegenDataModelProviderInstance(wrapperStorage);

    wrapper->mGroupDataProvider.SetStorageDelegate(wrapperStorage);
    wrapper->mGroupDataProvider.SetSessionKeystore(initParams.sessionKeystore);

    CommissioningParameters params = wrapper->GetCommissioningParameters();
    params.SetFailsafeTimerSeconds(failsafeTimerSeconds);
    params.SetAttemptWiFiNetworkScan(attemptNetworkScanWiFi);
    params.SetAttemptThreadNetworkScan(attemptNetworkScanThread);
    params.SetSkipCommissioningComplete(skipCommissioningComplete);

    if (countryCode != nullptr)
    {
        JniUtfString countryCodeJniString(env, countryCode);
        if (countryCodeJniString.size() != kCountryCodeBufferLen)
        {
            *errInfoOnFailure = CHIP_ERROR_INVALID_ARGUMENT;
            return nullptr;
        }

        MutableCharSpan copiedCode(wrapper->mCountryCode);
        if (CopyCharSpanToMutableCharSpan(countryCodeJniString.charSpan(), copiedCode) != CHIP_NO_ERROR)
        {
            *errInfoOnFailure = CHIP_ERROR_INVALID_ARGUMENT;
            return nullptr;
        }
        params.SetCountryCode(copiedCode);
    }

    wrapper->UpdateCommissioningParameters(params);

    CHIP_ERROR err = wrapper->mGroupDataProvider.Init();
    if (err != CHIP_NO_ERROR)
    {
        *errInfoOnFailure = err;
        return nullptr;
    }
    chip::Credentials::SetGroupDataProvider(&wrapper->mGroupDataProvider);
    initParams.groupDataProvider = &wrapper->mGroupDataProvider;

    err = wrapper->mOpCertStore.Init(wrapperStorage);
    if (err != CHIP_NO_ERROR)
    {
        *errInfoOnFailure = err;
        return nullptr;
    }
    initParams.opCertStore = &wrapper->mOpCertStore;
#ifdef JAVA_MATTER_CONTROLLER_TEST
    err = opCredsIssuer->Initialize(wrapper->mExampleStorage);
#else
    // TODO: Init IPK Epoch Key in opcreds issuer, so that commissionees get the right IPK
    err = opCredsIssuer->Initialize(*wrapper.get(), &wrapper->mAutoCommissioner, wrapper.get()->mJavaObjectRef.ObjectRef());
#endif
    if (err != CHIP_NO_ERROR)
    {
        *errInfoOnFailure = err;
        return nullptr;
    }

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
        *errInfoOnFailure = nativeKeypairBridge->Initialize(Crypto::ECPKeyTarget::ECDSA);
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

        *errInfoOnFailure = ephemeralKey.Initialize(Crypto::ECPKeyTarget::ECDSA);
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

    initParams.enableServerInteractions  = enableServerInteractions;
    setupParams.enableServerInteractions = enableServerInteractions;

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

    getICDClientStorage()->UpdateFabricList(wrapper->Controller()->GetFabricIndex());

    auto engine       = chip::app::InteractionModelEngine::GetInstance();
    *errInfoOnFailure = wrapper->mCheckInDelegate.Init(getICDClientStorage(), engine);
    *errInfoOnFailure = wrapper->mCheckInHandler.Init(DeviceControllerFactory::GetInstance().GetSystemState()->ExchangeMgr(),
                                                      getICDClientStorage(), &wrapper->mCheckInDelegate, engine);

    memset(ipkBuffer.data(), 0, ipkBuffer.size());

    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        return nullptr;
    }
    wrapper->mIsInitialized = true;
    return wrapper.release();
}

void AndroidDeviceControllerWrapper::Shutdown()
{
    VerifyOrReturn(mIsInitialized);
    getICDClientStorage()->Shutdown();
    mController->Shutdown();
    DeviceControllerFactory::GetInstance().Shutdown();

    if (mKeypairBridge != nullptr)
    {
        chip::Platform::Delete(mKeypairBridge);
        mKeypairBridge = nullptr;
    }

    if (mDeviceAttestationDelegateBridge != nullptr)
    {
        delete mDeviceAttestationDelegateBridge;
        mDeviceAttestationDelegateBridge = nullptr;
    }

    if (mDeviceAttestationVerifier != nullptr)
    {
        delete mDeviceAttestationVerifier;
        mDeviceAttestationVerifier = nullptr;
    }

    if (mAttestationTrustStoreBridge != nullptr)
    {
        delete mAttestationTrustStoreBridge;
        mAttestationTrustStoreBridge = nullptr;
    }
    mIsInitialized = false;
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

        ssid                 = env->GetStringUTFChars(ssidStr, nullptr);
        password             = env->GetStringUTFChars(passwordStr, nullptr);
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

CHIP_ERROR AndroidDeviceControllerWrapper::ApplyICDRegistrationInfo(chip::Controller::CommissioningParameters & params,
                                                                    jobject icdRegistrationInfo)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    mDeviceIsICD = false;

    VerifyOrReturnError(icdRegistrationInfo != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Controller, "Failed to retrieve JNIEnv in %s.", __func__);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    jmethodID getICDStayActiveDurationMsecMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, icdRegistrationInfo, "getICDStayActiveDurationMsec",
                                                        "()Ljava/lang/Long;", &getICDStayActiveDurationMsecMethod);
    ReturnErrorOnFailure(err);
    jobject jStayActiveMsec = env->CallObjectMethod(icdRegistrationInfo, getICDStayActiveDurationMsecMethod);
    if (jStayActiveMsec != nullptr)
    {
        jlong stayActiveMsec = chip::JniReferences::GetInstance().LongToPrimitive(jStayActiveMsec);
        if (!chip::CanCastTo<uint32_t>(stayActiveMsec))
        {
            ChipLogError(Controller, "Failed to process stayActiveMsec in %s since this is not a valid 32-bit integer.", __func__);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        params.SetICDStayActiveDurationMsec(static_cast<uint32_t>(stayActiveMsec));
    }

    jmethodID getCheckInNodeIdMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, icdRegistrationInfo, "getCheckInNodeId", "()Ljava/lang/Long;",
                                                        &getCheckInNodeIdMethod);
    ReturnErrorOnFailure(err);
    jobject jCheckInNodeId = env->CallObjectMethod(icdRegistrationInfo, getCheckInNodeIdMethod);

    jmethodID getMonitoredSubjectMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, icdRegistrationInfo, "getMonitoredSubject", "()Ljava/lang/Long;",
                                                        &getMonitoredSubjectMethod);
    ReturnErrorOnFailure(err);
    jobject jMonitoredSubject = env->CallObjectMethod(icdRegistrationInfo, getMonitoredSubjectMethod);

    jmethodID getSymmetricKeyMethod;
    err =
        chip::JniReferences::GetInstance().FindMethod(env, icdRegistrationInfo, "getSymmetricKey", "()[B", &getSymmetricKeyMethod);
    ReturnErrorOnFailure(err);
    jbyteArray jSymmetricKey = static_cast<jbyteArray>(env->CallObjectMethod(icdRegistrationInfo, getSymmetricKeyMethod));

    jmethodID getClientTypeMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, icdRegistrationInfo, "getClientType", "()Ljava/lang/Integer;",
                                                        &getClientTypeMethod);
    ReturnErrorOnFailure(err);
    jobject jClientType = env->CallObjectMethod(icdRegistrationInfo, getClientTypeMethod);

    chip::NodeId checkInNodeId = chip::kUndefinedNodeId;
    if (jCheckInNodeId != nullptr)
    {
        checkInNodeId = static_cast<chip::NodeId>(chip::JniReferences::GetInstance().LongToPrimitive(jCheckInNodeId));
    }
    else
    {
        checkInNodeId = mController->GetNodeId();
    }
    params.SetICDCheckInNodeId(checkInNodeId);

    uint64_t monitoredSubject = static_cast<uint64_t>(checkInNodeId);
    if (jMonitoredSubject != nullptr)
    {
        monitoredSubject = static_cast<uint64_t>(chip::JniReferences::GetInstance().LongToPrimitive(jMonitoredSubject));
    }
    params.SetICDMonitoredSubject(monitoredSubject);

    if (jSymmetricKey != nullptr)
    {
        JniByteArray jniSymmetricKey(env, jSymmetricKey);
        VerifyOrReturnError(jniSymmetricKey.size() == sizeof(mICDSymmetricKey), CHIP_ERROR_INVALID_ARGUMENT);
        memcpy(mICDSymmetricKey, jniSymmetricKey.data(), sizeof(mICDSymmetricKey));
    }
    else
    {
        chip::Crypto::DRBG_get_bytes(mICDSymmetricKey, sizeof(mICDSymmetricKey));
    }
    params.SetICDSymmetricKey(chip::ByteSpan(mICDSymmetricKey));

    chip::app::Clusters::IcdManagement::ClientTypeEnum clientType = chip::app::Clusters::IcdManagement::ClientTypeEnum::kPermanent;
    if (jClientType != nullptr)
    {
        clientType = static_cast<chip::app::Clusters::IcdManagement::ClientTypeEnum>(
            chip::JniReferences::GetInstance().IntegerToPrimitive(jClientType));
    }
    params.SetICDClientType(clientType);

    return err;
}

CHIP_ERROR AndroidDeviceControllerWrapper::UpdateCommissioningParameters(const chip::Controller::CommissioningParameters & params)
{
    // this will wipe out any custom attestationNonce and csrNonce that was being used.
    // however, Android APIs don't allow these to be set to custom values today.
    mCommissioningParameter = params;
    return mAutoCommissioner.SetCommissioningParameters(params);
}

CHIP_ERROR AndroidDeviceControllerWrapper::UpdateDeviceAttestationDelegateBridge(jobject deviceAttestationDelegate,
                                                                                 chip::Optional<uint16_t> expiryTimeoutSecs,
                                                                                 bool shouldWaitAfterDeviceAttestation)
{
    chip::JniGlobalReference deviceAttestationDelegateRef;
    ReturnErrorOnFailure(deviceAttestationDelegateRef.Init(deviceAttestationDelegate));
    DeviceAttestationDelegateBridge * delegateBridge = new DeviceAttestationDelegateBridge(
        std::move(deviceAttestationDelegateRef), expiryTimeoutSecs, shouldWaitAfterDeviceAttestation);
    VerifyOrReturnError(delegateBridge != nullptr, CHIP_ERROR_NO_MEMORY);
    if (mDeviceAttestationDelegateBridge != nullptr)
    {
        delete mDeviceAttestationDelegateBridge;
    }

    mDeviceAttestationDelegateBridge = delegateBridge;
    return CHIP_NO_ERROR;
}

CHIP_ERROR AndroidDeviceControllerWrapper::UpdateAttestationTrustStoreBridge(jobject attestationTrustStoreDelegate,
                                                                             jobject cdTrustKeys)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceAttestationVerifier * deviceAttestationVerifier = nullptr;
    chip::JniGlobalReference attestationTrustStoreDelegateRef;
    ReturnErrorOnFailure(attestationTrustStoreDelegateRef.Init(attestationTrustStoreDelegate));
    AttestationTrustStoreBridge * attestationTrustStoreBridge =
        new AttestationTrustStoreBridge(std::move(attestationTrustStoreDelegateRef));
    VerifyOrExit(attestationTrustStoreBridge != nullptr, err = CHIP_ERROR_NO_MEMORY);

    deviceAttestationVerifier = new Credentials::DefaultDACVerifier(attestationTrustStoreBridge);
    VerifyOrExit(deviceAttestationVerifier != nullptr, err = CHIP_ERROR_NO_MEMORY);

    if (mAttestationTrustStoreBridge != nullptr)
    {
        delete mAttestationTrustStoreBridge;
    }
    mAttestationTrustStoreBridge = attestationTrustStoreBridge;
    attestationTrustStoreBridge  = nullptr;

    if (mDeviceAttestationVerifier != nullptr)
    {
        delete mDeviceAttestationVerifier;
    }
    mDeviceAttestationVerifier = deviceAttestationVerifier;
    deviceAttestationVerifier  = nullptr;

    if (cdTrustKeys != nullptr)
    {
        WellKnownKeysTrustStore * cdTrustStore = mDeviceAttestationVerifier->GetCertificationDeclarationTrustStore();
        VerifyOrExit(cdTrustStore != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

        jint size;
        err = JniReferences::GetInstance().GetListSize(cdTrustKeys, size);
        VerifyOrExit(err == CHIP_NO_ERROR, err = CHIP_ERROR_INVALID_ARGUMENT);

        for (jint i = 0; i < size; i++)
        {
            jobject jTrustKey = nullptr;
            err               = JniReferences::GetInstance().GetListItem(cdTrustKeys, i, jTrustKey);

            VerifyOrExit(err == CHIP_NO_ERROR, err = CHIP_ERROR_INVALID_ARGUMENT);

            JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
            JniByteArray jniTrustKey(env, static_cast<jbyteArray>(jTrustKey));
            err = cdTrustStore->AddTrustedKey(jniTrustKey.byteSpan());
            VerifyOrExit(err == CHIP_NO_ERROR, err = CHIP_ERROR_INVALID_ARGUMENT);
        }
    }

    mController->SetDeviceAttestationVerifier(mDeviceAttestationVerifier);

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (attestationTrustStoreBridge != nullptr)
        {
            delete attestationTrustStoreBridge;
            attestationTrustStoreBridge = nullptr;
        }
    }

    return err;
}

CHIP_ERROR AndroidDeviceControllerWrapper::StartOTAProvider(jobject otaProviderDelegate)
{
#if CHIP_DEVICE_CONFIG_DYNAMIC_SERVER
    CHIP_ERROR err = CHIP_NO_ERROR;

    OTAProviderDelegateBridge * otaProviderBridge = new OTAProviderDelegateBridge();
    auto systemState                              = DeviceControllerFactory::GetInstance().GetSystemState();

    VerifyOrExit(otaProviderBridge != nullptr, err = CHIP_ERROR_NO_MEMORY);

    err = otaProviderBridge->Init(systemState->SystemLayer(), systemState->ExchangeMgr(), otaProviderDelegate);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(Controller, "OTA Provider Initialize Error : %" CHIP_ERROR_FORMAT, err.Format()));

    mOtaProviderBridge = otaProviderBridge;
exit:
    if (err != CHIP_NO_ERROR)
    {
        if (otaProviderBridge != nullptr)
        {
            delete otaProviderBridge;
            otaProviderBridge = nullptr;
        }
    }
    return err;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR AndroidDeviceControllerWrapper::FinishOTAProvider()
{
#if CHIP_DEVICE_CONFIG_DYNAMIC_SERVER
    if (mOtaProviderBridge != nullptr)
    {
        mOtaProviderBridge->Shutdown();
        delete mOtaProviderBridge;

        mOtaProviderBridge = nullptr;
    }

    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR AndroidDeviceControllerWrapper::SetICDCheckInDelegate(jobject checkInDelegate)
{
    return mCheckInDelegate.SetDelegate(checkInDelegate);
}

void AndroidDeviceControllerWrapper::OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status)
{
    chip::DeviceLayer::StackUnlock unlock;
    CallJavaIntMethod("onStatusUpdate", static_cast<jint>(status));
}

void AndroidDeviceControllerWrapper::OnPairingComplete(CHIP_ERROR error)
{
    chip::DeviceLayer::StackUnlock unlock;
    CallJavaLongMethod("onPairingComplete", static_cast<jlong>(error.AsInteger()));
}

void AndroidDeviceControllerWrapper::OnPairingDeleted(CHIP_ERROR error)
{
    chip::DeviceLayer::StackUnlock unlock;
    CallJavaLongMethod("onPairingDeleted", static_cast<jlong>(error.AsInteger()));
}

void AndroidDeviceControllerWrapper::OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error)
{
    chip::DeviceLayer::StackUnlock unlock;

    if (error != CHIP_NO_ERROR && mDeviceIsICD)
    {
        CHIP_ERROR deleteEntryError = getICDClientStorage()->DeleteEntry(ScopedNodeId(deviceId, Controller()->GetFabricIndex()));
        if (deleteEntryError != CHIP_NO_ERROR)
        {
            ChipLogError(chipTool, "Failed to delete ICD entry: %" CHIP_ERROR_FORMAT, deleteEntryError.Format());
        }
    }

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID onCommissioningCompleteMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, mJavaObjectRef.ObjectRef(), "onCommissioningComplete", "(JJ)V",
                                                             &onCommissioningCompleteMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error finding Java method: %" CHIP_ERROR_FORMAT, err.Format()));
    env->CallVoidMethod(mJavaObjectRef.ObjectRef(), onCommissioningCompleteMethod, static_cast<jlong>(deviceId),
                        static_cast<jlong>(error.AsInteger()));

    if (ssidStr != nullptr)
    {
        env->ReleaseStringUTFChars(ssidStr, ssid);
        env->DeleteGlobalRef(ssidStr);
        ssidStr = nullptr;
    }
    if (passwordStr != nullptr)
    {
        env->ReleaseStringUTFChars(passwordStr, password);
        env->DeleteGlobalRef(passwordStr);
        passwordStr = nullptr;
    }
    if (operationalDatasetBytes != nullptr)
    {
        env->ReleaseByteArrayElements(operationalDatasetBytes, operationalDataset, 0);
        env->DeleteGlobalRef(operationalDatasetBytes);
        operationalDatasetBytes = nullptr;
    }
}

void AndroidDeviceControllerWrapper::OnCommissioningStatusUpdate(PeerId peerId, chip::Controller::CommissioningStage stageCompleted,
                                                                 CHIP_ERROR error)
{
    chip::DeviceLayer::StackUnlock unlock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);
    jmethodID onCommissioningStatusUpdateMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, mJavaObjectRef.ObjectRef(), "onCommissioningStatusUpdate",
                                                             "(JLjava/lang/String;J)V", &onCommissioningStatusUpdateMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error finding Java method: %" CHIP_ERROR_FORMAT, err.Format()));

    UtfString jStageCompleted(env, StageToString(stageCompleted));
    env->CallVoidMethod(mJavaObjectRef.ObjectRef(), onCommissioningStatusUpdateMethod, static_cast<jlong>(peerId.GetNodeId()),
                        jStageCompleted.jniValue(), static_cast<jlong>(error.AsInteger()));
}

void AndroidDeviceControllerWrapper::OnReadCommissioningInfo(const chip::Controller::ReadCommissioningInfo & info)
{
    // calls: onReadCommissioningInfo(int vendorId, int productId, int wifiEndpointId, int threadEndpointId)
    chip::DeviceLayer::StackUnlock unlock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID onReadCommissioningInfoMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, mJavaObjectRef.ObjectRef(), "onReadCommissioningInfo", "(IIII)V",
                                                             &onReadCommissioningInfoMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error finding Java method: %" CHIP_ERROR_FORMAT, err.Format()));

    // For ICD
    mUserActiveModeTriggerHint = info.icd.userActiveModeTriggerHint;
    memset(mUserActiveModeTriggerInstructionBuffer, 0x00, kUserActiveModeTriggerInstructionBufferLen);
    CopyCharSpanToMutableCharSpan(info.icd.userActiveModeTriggerInstruction, mUserActiveModeTriggerInstruction);

    ChipLogProgress(AppServer, "OnReadCommissioningInfo ICD - IdleModeDuration=%u activeModeDuration=%u activeModeThreshold=%u",
                    info.icd.idleModeDuration, info.icd.activeModeDuration, info.icd.activeModeThreshold);

    env->CallVoidMethod(mJavaObjectRef.ObjectRef(), onReadCommissioningInfoMethod, static_cast<jint>(info.basic.vendorId),
                        static_cast<jint>(info.basic.productId), static_cast<jint>(info.network.wifi.endpoint),
                        static_cast<jint>(info.network.thread.endpoint));
}

void AndroidDeviceControllerWrapper::OnScanNetworksSuccess(
    const chip::app::Clusters::NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType & dataResponse)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID javaMethod;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Error invoking Java callback: no JNIEnv"));

    err = JniReferences::GetInstance().FindMethod(
        env, mJavaObjectRef.ObjectRef(), "onScanNetworksSuccess",
        "(Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error invoking Java callback: %s", ErrorStr(err)));

    jobject NetworkingStatus;
    std::string NetworkingStatusClassName     = "java/lang/Integer";
    std::string NetworkingStatusCtorSignature = "(I)V";
    jint jniNetworkingStatus                  = static_cast<jint>(dataResponse.networkingStatus);
    chip::JniReferences::GetInstance().CreateBoxedObject<jint>(NetworkingStatusClassName, NetworkingStatusCtorSignature,
                                                               jniNetworkingStatus, NetworkingStatus);
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
            jint jniNewElementSecurity                   = static_cast<jint>(entry.security.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                newElement_securityClassName, newElement_securityCtorSignature, jniNewElementSecurity, newElement_security);
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
            jint jniChannel = static_cast<jint>(entry.channel);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>("java/lang/Integer", "(I)V", jniChannel, newElement_channel);
            jobject newElement_wiFiBand;
            jint jniWiFiBand = static_cast<jint>(entry.wiFiBand);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>("java/lang/Integer", "(I)V", jniWiFiBand,
                                                                       newElement_wiFiBand);
            jobject newElement_rssi;
            jint jniRssi = static_cast<jint>(entry.rssi);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>("java/lang/Integer", "(I)V", jniRssi, newElement_rssi);

            jclass wiFiInterfaceScanResultStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/WiFiScanResult",
                                                                      wiFiInterfaceScanResultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class WiFiScanResult");
                return;
            }
            jmethodID wiFiInterfaceScanResultStructCtor =
                env->GetMethodID(wiFiInterfaceScanResultStructClass, "<init>",
                                 "(Ljava/lang/Integer;[B[BLjava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;)V");
            if (wiFiInterfaceScanResultStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find WiFiScanResult constructor");
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
            jint jniPanId = static_cast<jint>(entry.panId);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>("java/lang/Integer", "(I)V", jniPanId, newElement_panId);
            jobject newElement_extendedPanId;
            jlong jniExtendedPanId = static_cast<jlong>(entry.extendedPanId);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>("java/lang/Long", "(J)V", jniExtendedPanId,
                                                                        newElement_extendedPanId);
            jobject newElement_networkName;
            newElement_networkName = env->NewStringUTF(std::string(entry.networkName.data(), entry.networkName.size()).c_str());
            jobject newElement_channel;
            jint jniChannel = static_cast<jint>(entry.channel);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>("java/lang/Integer", "(I)V", jniChannel, newElement_channel);
            jobject newElement_version;
            jint jniVersion = static_cast<jint>(entry.version);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>("java/lang/Integer", "(I)V", jniVersion, newElement_version);
            jobject newElement_extendedAddress;
            jbyteArray newElement_extendedAddressByteArray = env->NewByteArray(static_cast<jsize>(entry.extendedAddress.size()));
            env->SetByteArrayRegion(newElement_extendedAddressByteArray, 0, static_cast<jsize>(entry.extendedAddress.size()),
                                    reinterpret_cast<const jbyte *>(entry.extendedAddress.data()));
            newElement_extendedAddress = newElement_extendedAddressByteArray;
            jobject newElement_rssi;
            jint jniRssi = static_cast<jint>(entry.rssi);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>("java/lang/Integer", "(I)V", jniRssi, newElement_rssi);
            jobject newElement_lqi;
            jint jniLqi = static_cast<jint>(entry.lqi);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>("java/lang/Integer", "(I)V", jniLqi, newElement_lqi);

            jclass threadInterfaceScanResultStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/ThreadScanResult",
                                                                      threadInterfaceScanResultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Controller, "Could not find class ThreadScanResult");
                return;
            }
            jmethodID threadInterfaceScanResultStructCtor =
                env->GetMethodID(threadInterfaceScanResultStructClass, "<init>",
                                 "(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/String;Ljava/lang/Integer;Ljava/lang/"
                                 "Integer;[BLjava/lang/Integer;Ljava/lang/Integer;)V");
            if (threadInterfaceScanResultStructCtor == nullptr)
            {
                ChipLogError(Controller, "Could not find ThreadScanResult constructor");
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

    env->CallVoidMethod(mJavaObjectRef.ObjectRef(), javaMethod, NetworkingStatus, DebugText, WiFiScanResults, ThreadScanResults);
}

void AndroidDeviceControllerWrapper::OnScanNetworksFailure(CHIP_ERROR error)
{
    chip::DeviceLayer::StackUnlock unlock;

    CallJavaLongMethod("onScanNetworksFailure", static_cast<jlong>(error.AsInteger()));
}

void AndroidDeviceControllerWrapper::OnICDRegistrationInfoRequired()
{
    chip::DeviceLayer::StackUnlock unlock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID onICDRegistrationInfoRequiredMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, mJavaObjectRef.ObjectRef(), "onICDRegistrationInfoRequired",
                                                             "()V", &onICDRegistrationInfoRequiredMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error finding Java method: %" CHIP_ERROR_FORMAT, err.Format()));

    env->CallVoidMethod(mJavaObjectRef.ObjectRef(), onICDRegistrationInfoRequiredMethod);
}

void AndroidDeviceControllerWrapper::OnICDRegistrationComplete(chip::ScopedNodeId icdNodeId, uint32_t icdCounter)
{
    chip::DeviceLayer::StackUnlock unlock;

    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::ICDClientInfo clientInfo;
    clientInfo.peer_node         = icdNodeId;
    clientInfo.check_in_node     = chip::ScopedNodeId(mAutoCommissioner.GetCommissioningParameters().GetICDCheckInNodeId().Value(),
                                                      icdNodeId.GetFabricIndex());
    clientInfo.monitored_subject = mAutoCommissioner.GetCommissioningParameters().GetICDMonitoredSubject().Value();
    clientInfo.start_icd_counter = icdCounter;

    ByteSpan symmetricKey = mAutoCommissioner.GetCommissioningParameters().GetICDSymmetricKey().Value();

    err = getICDClientStorage()->SetKey(clientInfo, symmetricKey);
    if (err == CHIP_NO_ERROR)
    {
        err = getICDClientStorage()->StoreEntry(clientInfo);
    }

    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Saved ICD Symmetric key for " ChipLogFormatX64, ChipLogValueX64(icdNodeId.GetNodeId()));
    }
    else
    {
        getICDClientStorage()->RemoveKey(clientInfo);
        ChipLogError(Controller, "Failed to persist symmetric key for " ChipLogFormatX64 ": %s",
                     ChipLogValueX64(icdNodeId.GetNodeId()), err.AsString());
    }

    mDeviceIsICD = true;

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    JniLocalReferenceScope scope(env);
    jmethodID onICDRegistrationCompleteMethod;
    jclass icdDeviceInfoClass         = nullptr;
    jmethodID icdDeviceInfoStructCtor = nullptr;
    jobject icdDeviceInfoObj          = nullptr;
    jbyteArray jSymmetricKey          = nullptr;
    CHIP_ERROR methodErr =
        JniReferences::GetInstance().FindMethod(env, mJavaObjectRef.ObjectRef(), "onICDRegistrationComplete",
                                                "(JLchip/devicecontroller/ICDDeviceInfo;)V", &onICDRegistrationCompleteMethod);
    VerifyOrReturn(methodErr == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Error finding Java method: %" CHIP_ERROR_FORMAT, methodErr.Format()));

    methodErr = chip::JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/ICDDeviceInfo", icdDeviceInfoClass);
    VerifyOrReturn(methodErr == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find class ICDDeviceInfo"));

    icdDeviceInfoStructCtor = env->GetMethodID(icdDeviceInfoClass, "<init>", "([BILjava/lang/String;JJIJJJJJI)V");
    VerifyOrReturn(icdDeviceInfoStructCtor != nullptr, ChipLogError(Controller, "Could not find ICDDeviceInfo constructor"));

    methodErr =
        JniReferences::GetInstance().N2J_ByteArray(env, symmetricKey.data(), static_cast<jint>(symmetricKey.size()), jSymmetricKey);
    VerifyOrReturn(methodErr == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Error Parsing Symmetric Key: %" CHIP_ERROR_FORMAT, methodErr.Format()));

    jstring jUserActiveModeTriggerInstruction = env->NewStringUTF(mUserActiveModeTriggerInstruction.data());

    icdDeviceInfoObj = env->NewObject(
        icdDeviceInfoClass, icdDeviceInfoStructCtor, jSymmetricKey, static_cast<jint>(mUserActiveModeTriggerHint.Raw()),
        jUserActiveModeTriggerInstruction, static_cast<jlong>(mIdleModeDuration), static_cast<jlong>(mActiveModeDuration),
        static_cast<jint>(mActiveModeThreshold), static_cast<jlong>(icdNodeId.GetNodeId()),
        static_cast<jlong>(mAutoCommissioner.GetCommissioningParameters().GetICDCheckInNodeId().Value()),
        static_cast<jlong>(icdCounter),
        static_cast<jlong>(mAutoCommissioner.GetCommissioningParameters().GetICDMonitoredSubject().Value()),
        static_cast<jlong>(Controller()->GetFabricId()), static_cast<jint>(Controller()->GetFabricIndex()));

    env->CallVoidMethod(mJavaObjectRef.ObjectRef(), onICDRegistrationCompleteMethod, static_cast<jlong>(err.AsInteger()),
                        icdDeviceInfoObj);
}

CHIP_ERROR AndroidDeviceControllerWrapper::SyncGetKeyValue(const char * key, void * value, uint16_t & size)
{
    ChipLogProgress(Controller, "KVS: Getting key %s", StringOrNullMarker(key));

    size_t read_size = 0;

    CHIP_ERROR err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(key, value, size, &read_size);

    size = static_cast<uint16_t>(read_size);

    return err;
}

CHIP_ERROR AndroidDeviceControllerWrapper::SyncSetKeyValue(const char * key, const void * value, uint16_t size)
{
    ChipLogProgress(Controller, "KVS: Setting key %s", StringOrNullMarker(key));
    return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(key, value, size);
}

CHIP_ERROR AndroidDeviceControllerWrapper::SyncDeleteKeyValue(const char * key)
{
    ChipLogProgress(Controller, "KVS: Deleting key %s", StringOrNullMarker(key));
    return chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(key);
}

void AndroidDeviceControllerWrapper::StartDnssd()
{
    FabricTable * fabricTable = DeviceControllerFactory::GetInstance().GetSystemState()->Fabrics();
    VerifyOrReturn(fabricTable != nullptr, ChipLogError(Controller, "Fail to get fabricTable in StartDnssd"));
    chip::app::DnssdServer::Instance().SetFabricTable(fabricTable);
    chip::app::DnssdServer::Instance().StartServer();
}

void AndroidDeviceControllerWrapper::StopDnssd()
{
    chip::app::DnssdServer::Instance().StopServer();
}
