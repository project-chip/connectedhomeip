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

/**
 *    @file
 *      Implementation of JNI bridge for CHIP Device Controller for Android apps
 *
 */
#include "AndroidCallbacks.h"
#include "AndroidCommissioningWindowOpener.h"
#include "AndroidCurrentFabricRemover.h"
#include "AndroidDeviceControllerWrapper.h"
#include "AndroidInteractionClient.h"
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#include <app/AttributePathParams.h>
#include <app/DataVersionFilter.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadClient.h>
#include <app/WriteClient.h>
#include <atomic>
#include <ble/BleUUID.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningWindowOpener.h>
#include <controller/java/GroupDeviceProxy.h>
#include <credentials/CHIPCert.h>
#include <jni.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/jsontlv/JsonToTlv.h>
#include <lib/support/jsontlv/TlvToJson.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/SessionEstablishmentDelegate.h>
#include <pthread.h>
#include <system/SystemClock.h>
#include <vector>

#ifdef CHIP_DEVICE_CONFIG_DYNAMIC_SERVER
#include <app/dynamic_server/AccessControl.h>
#endif // CHIP_DEVICE_CONFIG_DYNAMIC_SERVER

#ifdef JAVA_MATTER_CONTROLLER_TEST
#include <controller/ExampleOperationalCredentialsIssuer.h>
#else
#include <platform/android/AndroidChipPlatform-JNI.h>
#endif

// Choose an approximation of PTHREAD_NULL if pthread.h doesn't define one.
#ifndef PTHREAD_NULL
#define PTHREAD_NULL 0
#endif // PTHREAD_NULL

using namespace chip;
using namespace chip::Inet;
using namespace chip::Controller;
using namespace chip::Credentials;

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_ChipDeviceController_##METHOD_NAME

#define CDC_JNI_CALLBACK_LOCAL_REF_COUNT 256

static void * IOThreadMain(void * arg);
static CHIP_ERROR StopIOThread();
static CHIP_ERROR N2J_PaseVerifierParams(JNIEnv * env, jlong setupPincode, jbyteArray pakeVerifier, jobject & outParams);
static CHIP_ERROR N2J_NetworkLocation(JNIEnv * env, jstring ipAddress, jint port, jint interfaceIndex, jobject & outLocation);

namespace {
JavaVM * sJVM       = nullptr;
pthread_t sIOThread = PTHREAD_NULL;
chip::JniGlobalReference sChipDeviceControllerExceptionCls;
} // namespace

// NOTE: Remote device ID is in sync with the echo server device id
// At some point, we may want to add an option to connect to a device without
// knowing its id, because the ID can be learned on the first response that is received.
chip::NodeId kLocalDeviceId  = chip::kTestControllerNodeId;
chip::NodeId kRemoteDeviceId = chip::kTestDeviceNodeId;

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;

    ChipLogProgress(Controller, "JNI_OnLoad() called");

    chip::Platform::MemoryInit();

    // Save a reference to the JVM.  Will need this to call back into Java.
    JniReferences::GetInstance().SetJavaVm(jvm, "chip/devicecontroller/ChipDeviceController");
    sJVM = jvm;

    // Get a JNI environment object.
    env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrExit(env != nullptr, err = CHIP_JNI_ERROR_NO_ENV);

    ChipLogProgress(Controller, "Loading Java class references.");

    // Get various class references need by the API.
    jclass controllerExceptionCls;
    err = JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/ChipDeviceControllerException",
                                                        controllerExceptionCls);
    SuccessOrExit(err = sChipDeviceControllerExceptionCls.Init(controllerExceptionCls));

    ChipLogProgress(Controller, "Java class references loaded.");

#ifndef JAVA_MATTER_CONTROLLER_TEST
    err = AndroidChipPlatformJNI_OnLoad(jvm, reserved);
    SuccessOrExit(err);
#endif // JAVA_MATTER_CONTROLLER_TEST

#ifdef CHIP_DEVICE_CONFIG_DYNAMIC_SERVER
    chip::app::dynamic_server::InitAccessControl();
#endif // CHIP_DEVICE_CONFIG_DYNAMIC_SERVER

exit:
    if (err != CHIP_NO_ERROR)
    {
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
        chip::DeviceLayer::StackUnlock unlock;
        JNI_OnUnload(jvm, reserved);
    }

    return (err == CHIP_NO_ERROR) ? JNI_VERSION_1_6 : JNI_ERR;
}

void JNI_OnUnload(JavaVM * jvm, void * reserved)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(Controller, "JNI_OnUnload() called");

    // If the IO thread has not been stopped yet, shut it down now.
    // TODO(arkq): Maybe we should just assert here, as the IO thread
    //             should be stopped before the library is unloaded.
    StopIOThread();

    sJVM = nullptr;

    chip::Platform::MemoryShutdown();
}

JNI_METHOD(jint, onNOCChainGeneration)
(JNIEnv * env, jobject self, jlong handle, jobject controllerParams)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "setNOCChain() called");

    jmethodID getRootCertificate;
    jmethodID getIntermediateCertificate;
    jmethodID getOperationalCertificate;
    jmethodID getIpk;
    jmethodID getAdminSubject;

    jbyteArray rootCertificate         = nullptr;
    jbyteArray intermediateCertificate = nullptr;
    jbyteArray operationalCertificate  = nullptr;
    jbyteArray ipk                     = nullptr;

    Optional<NodeId> adminSubjectOptional;
    uint64_t adminSubject = chip::kUndefinedNodeId;

    CommissioningParameters commissioningParams = wrapper->GetCommissioningParameters();

    Optional<Crypto::IdentityProtectionKeySpan> ipkOptional;
    uint8_t ipkValue[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
    Crypto::IdentityProtectionKeySpan ipkTempSpan(ipkValue);

    VerifyOrExit(controllerParams != nullptr, ChipLogError(Controller, "controllerParams is null!"));

    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getRootCertificate", "()[B", &getRootCertificate);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Find getRootCertificate method fail!"));

    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getIntermediateCertificate", "()[B",
                                                        &getIntermediateCertificate);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Find getIntermediateCertificate method fail!"));

    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getOperationalCertificate", "()[B",
                                                        &getOperationalCertificate);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Find getOperationalCertificate method fail!"));

    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getIpk", "()[B", &getIpk);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Find getIpk method fail!"));

    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getAdminSubject", "()J", &getAdminSubject);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Find getAdminSubject method fail!"));

    rootCertificate = static_cast<jbyteArray>(env->CallObjectMethod(controllerParams, getRootCertificate));
    VerifyOrExit(rootCertificate != nullptr, err = CHIP_ERROR_BAD_REQUEST);

    intermediateCertificate = static_cast<jbyteArray>(env->CallObjectMethod(controllerParams, getIntermediateCertificate));
    VerifyOrExit(intermediateCertificate != nullptr, err = CHIP_ERROR_BAD_REQUEST);

    operationalCertificate = static_cast<jbyteArray>(env->CallObjectMethod(controllerParams, getOperationalCertificate));
    VerifyOrExit(operationalCertificate != nullptr, err = CHIP_ERROR_BAD_REQUEST);

    ipk = static_cast<jbyteArray>(env->CallObjectMethod(controllerParams, getIpk));
    if (ipk != nullptr)
    {
        JniByteArray jByteArrayIpk(env, ipk);

        if (jByteArrayIpk.byteSpan().size() != sizeof(ipkValue))
        {
            ChipLogError(Controller, "Invalid IPK size %u and expect %u", static_cast<unsigned>(jByteArrayIpk.byteSpan().size()),
                         static_cast<unsigned>(sizeof(ipkValue)));
            ExitNow(err = CHIP_ERROR_INVALID_IPK);
        }
        memcpy(&ipkValue[0], jByteArrayIpk.byteSpan().data(), jByteArrayIpk.byteSpan().size());

        ipkOptional.SetValue(ipkTempSpan);
    }
    else if (commissioningParams.GetIpk().HasValue())
    {
        // if no value pass in ControllerParams, use value from CommissioningParameters
        ipkOptional.SetValue(commissioningParams.GetIpk().Value());
    }

    adminSubject = static_cast<uint64_t>(env->CallLongMethod(controllerParams, getAdminSubject));
    if (adminSubject == kUndefinedNodeId)
    {
        // if no value pass in ControllerParams, use value from CommissioningParameters
        adminSubject = commissioningParams.GetAdminSubject().ValueOr(kUndefinedNodeId);
    }
    if (adminSubject != kUndefinedNodeId)
    {
        adminSubjectOptional.SetValue(adminSubject);
    }
    // NOTE: we are allowing adminSubject to not be set since the OnNOCChainGeneration callback makes this field
    // optional and includes logic to handle the case where it is not set. It would also make sense to return
    // an error here since that use case may not be realistic.
    {
        JniByteArray jByteArrayRcac(env, rootCertificate);
        JniByteArray jByteArrayIcac(env, intermediateCertificate);
        JniByteArray jByteArrayNoc(env, operationalCertificate);

#ifndef JAVA_MATTER_CONTROLLER_TEST
        err = wrapper->GetAndroidOperationalCredentialsIssuer()->NOCChainGenerated(
            CHIP_NO_ERROR, jByteArrayNoc.byteSpan(), jByteArrayIcac.byteSpan(), jByteArrayRcac.byteSpan(), ipkOptional,
            adminSubjectOptional);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Failed to SetNocChain for the device: %" CHIP_ERROR_FORMAT, err.Format());
        }
        return static_cast<jint>(err.AsInteger());
#endif // JAVA_MATTER_CONTROLLER_TEST
    }
exit:
#ifndef JAVA_MATTER_CONTROLLER_TEST
    err = wrapper->GetAndroidOperationalCredentialsIssuer()->NOCChainGenerated(err, ByteSpan(), ByteSpan(), ByteSpan(), ipkOptional,
                                                                               adminSubjectOptional);
#endif // JAVA_MATTER_CONTROLLER_TEST
    return static_cast<jint>(err.AsInteger());
}

JNI_METHOD(jlong, newDeviceController)(JNIEnv * env, jobject self, jobject controllerParams)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = nullptr;
    jlong result                             = 0;

    ChipLogProgress(Controller, "newDeviceController() called");

    // Retrieve initialization params.
    jmethodID getFabricId;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getFabricId", "()J", &getFabricId);
    SuccessOrExit(err);

    jmethodID getUdpListenPort;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getUdpListenPort", "()I", &getUdpListenPort);
    SuccessOrExit(err);

    jmethodID getControllerVendorId;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getControllerVendorId", "()I",
                                                        &getControllerVendorId);

    jmethodID getFailsafeTimerSeconds;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getFailsafeTimerSeconds", "()I",
                                                        &getFailsafeTimerSeconds);
    SuccessOrExit(err);

    jmethodID getCASEFailsafeTimerSeconds;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getCASEFailsafeTimerSeconds", "()I",
                                                        &getCASEFailsafeTimerSeconds);
    SuccessOrExit(err);

    jmethodID getAttemptNetworkScanWiFi;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getAttemptNetworkScanWiFi", "()Z",
                                                        &getAttemptNetworkScanWiFi);
    SuccessOrExit(err);

    jmethodID getAttemptNetworkScanThread;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getAttemptNetworkScanThread", "()Z",
                                                        &getAttemptNetworkScanThread);
    SuccessOrExit(err);

    jmethodID getSkipCommissioningComplete;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getSkipCommissioningComplete", "()Z",
                                                        &getSkipCommissioningComplete);
    SuccessOrExit(err);

    jmethodID getSkipAttestationCertificateValidation;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getSkipAttestationCertificateValidation", "()Z",
                                                        &getSkipAttestationCertificateValidation);
    SuccessOrExit(err);

    jmethodID getCountryCode;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getCountryCode", "()Ljava/util/Optional;",
                                                        &getCountryCode);
    SuccessOrExit(err);

    jmethodID getRegulatoryLocation;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getRegulatoryLocation", "()Ljava/util/Optional;",
                                                        &getRegulatoryLocation);
    SuccessOrExit(err);

    jmethodID getKeypairDelegate;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getKeypairDelegate",
                                                        "()Lchip/devicecontroller/KeypairDelegate;", &getKeypairDelegate);
    SuccessOrExit(err);

    jmethodID getRootCertificate;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getRootCertificate", "()[B", &getRootCertificate);
    SuccessOrExit(err);

    jmethodID getIntermediateCertificate;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getIntermediateCertificate", "()[B",
                                                        &getIntermediateCertificate);
    SuccessOrExit(err);

    jmethodID getOperationalCertificate;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getOperationalCertificate", "()[B",
                                                        &getOperationalCertificate);
    SuccessOrExit(err);

    jmethodID getIpk;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getIpk", "()[B", &getIpk);
    SuccessOrExit(err);

    jmethodID getAdminSubject;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getAdminSubject", "()J", &getAdminSubject);
    SuccessOrExit(err);

    jmethodID getEnableServerInteractions;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getEnableServerInteractions", "()Z",
                                                        &getEnableServerInteractions);
    SuccessOrExit(err);

    {
        uint64_t fabricId                  = static_cast<uint64_t>(env->CallLongMethod(controllerParams, getFabricId));
        uint16_t listenPort                = static_cast<uint16_t>(env->CallIntMethod(controllerParams, getUdpListenPort));
        uint16_t controllerVendorId        = static_cast<uint16_t>(env->CallIntMethod(controllerParams, getControllerVendorId));
        jobject keypairDelegate            = env->CallObjectMethod(controllerParams, getKeypairDelegate);
        jbyteArray rootCertificate         = (jbyteArray) env->CallObjectMethod(controllerParams, getRootCertificate);
        jbyteArray intermediateCertificate = (jbyteArray) env->CallObjectMethod(controllerParams, getIntermediateCertificate);
        jbyteArray operationalCertificate  = (jbyteArray) env->CallObjectMethod(controllerParams, getOperationalCertificate);
        jbyteArray ipk                     = (jbyteArray) env->CallObjectMethod(controllerParams, getIpk);
        uint16_t failsafeTimerSeconds      = static_cast<uint16_t>(env->CallIntMethod(controllerParams, getFailsafeTimerSeconds));
        uint16_t caseFailsafeTimerSeconds =
            static_cast<uint16_t>(env->CallIntMethod(controllerParams, getCASEFailsafeTimerSeconds));
        bool attemptNetworkScanWiFi    = env->CallBooleanMethod(controllerParams, getAttemptNetworkScanWiFi);
        bool attemptNetworkScanThread  = env->CallBooleanMethod(controllerParams, getAttemptNetworkScanThread);
        bool skipCommissioningComplete = env->CallBooleanMethod(controllerParams, getSkipCommissioningComplete);
        bool skipAttestationCertificateValidation =
            env->CallBooleanMethod(controllerParams, getSkipAttestationCertificateValidation);
        uint64_t adminSubject              = static_cast<uint64_t>(env->CallLongMethod(controllerParams, getAdminSubject));
        jobject countryCodeOptional        = env->CallObjectMethod(controllerParams, getCountryCode);
        jobject regulatoryLocationOptional = env->CallObjectMethod(controllerParams, getRegulatoryLocation);
        bool enableServerInteractions      = env->CallBooleanMethod(controllerParams, getEnableServerInteractions) == JNI_TRUE;

        jobject countryCode;
        err = chip::JniReferences::GetInstance().GetOptionalValue(countryCodeOptional, countryCode);
        SuccessOrExit(err);

#ifdef JAVA_MATTER_CONTROLLER_TEST
        std::unique_ptr<chip::Controller::ExampleOperationalCredentialsIssuer> opCredsIssuer(
            new chip::Controller::ExampleOperationalCredentialsIssuer());
#else
        std::unique_ptr<chip::Controller::AndroidOperationalCredentialsIssuer> opCredsIssuer(
            new chip::Controller::AndroidOperationalCredentialsIssuer());
#endif
        wrapper = AndroidDeviceControllerWrapper::AllocateNew(
            sJVM, self, kLocalDeviceId, fabricId, chip::kUndefinedCATs, &DeviceLayer::SystemLayer(),
            DeviceLayer::TCPEndPointManager(), DeviceLayer::UDPEndPointManager(), std::move(opCredsIssuer), keypairDelegate,
            rootCertificate, intermediateCertificate, operationalCertificate, ipk, listenPort, controllerVendorId,
            failsafeTimerSeconds, attemptNetworkScanWiFi, attemptNetworkScanThread, skipCommissioningComplete,
            skipAttestationCertificateValidation, static_cast<jstring>(countryCode), enableServerInteractions, &err);
        SuccessOrExit(err);

        if (caseFailsafeTimerSeconds > 0)
        {
            CommissioningParameters commissioningParams = wrapper->GetCommissioningParameters();
            commissioningParams.SetCASEFailsafeTimerSeconds(caseFailsafeTimerSeconds);
            err = wrapper->UpdateCommissioningParameters(commissioningParams);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Controller, "UpdateCommissioningParameters failed. Err = %" CHIP_ERROR_FORMAT, err.Format());
                SuccessOrExit(err);
            }
        }

        if (adminSubject != kUndefinedNodeId)
        {
            // if there is a valid adminSubject in the ControllerParams, then remember it
            CommissioningParameters commissioningParams = wrapper->GetCommissioningParameters();
            commissioningParams.SetAdminSubject(adminSubject);
            err = wrapper->UpdateCommissioningParameters(commissioningParams);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Controller, "UpdateCommissioningParameters failed. Err = %" CHIP_ERROR_FORMAT, err.Format());
                SuccessOrExit(err);
            }
        }

        jobject regulatoryLocation;
        err = chip::JniReferences::GetInstance().GetOptionalValue(regulatoryLocationOptional, regulatoryLocation);
        SuccessOrExit(err);

        if (regulatoryLocation != nullptr)
        {
            using namespace app::Clusters::GeneralCommissioning;

            jint regulatoryLocationJint = chip::JniReferences::GetInstance().IntegerToPrimitive(regulatoryLocation);
            VerifyOrExit(chip::CanCastTo<RegulatoryLocationTypeEnum>(regulatoryLocationJint), err = CHIP_ERROR_INVALID_ARGUMENT);

            auto regulatoryLocationType = static_cast<RegulatoryLocationTypeEnum>(regulatoryLocationJint);
            VerifyOrExit(regulatoryLocationType >= RegulatoryLocationTypeEnum::kIndoor, err = CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrExit(regulatoryLocationType <= RegulatoryLocationTypeEnum::kIndoorOutdoor, err = CHIP_ERROR_INVALID_ARGUMENT);

            chip::Controller::CommissioningParameters commissioningParams = wrapper->GetCommissioningParameters();
            commissioningParams.SetDeviceRegulatoryLocation(regulatoryLocationType);
            err = wrapper->UpdateCommissioningParameters(commissioningParams);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Controller, "UpdateCommissioningParameters failed. Err = %" CHIP_ERROR_FORMAT, err.Format());
                SuccessOrExit(err);
            }
        }
    }

    // Create and start the IO thread. Must be called after Controller()->Init
    if (sIOThread == PTHREAD_NULL)
    {
        int pthreadErr = pthread_create(&sIOThread, nullptr, IOThreadMain, nullptr);
        VerifyOrExit(pthreadErr == 0, err = CHIP_ERROR_POSIX(pthreadErr));
    }

    result = wrapper->ToJNIHandle();

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (wrapper != nullptr)
        {
            delete wrapper;
        }

        if (err != CHIP_JNI_ERROR_EXCEPTION_THROWN)
        {
            JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
        }
    }

    return result;
}

JNI_METHOD(void, setDeviceAttestationDelegate)
(JNIEnv * env, jobject self, jlong handle, jint failSafeExpiryTimeoutSecs, jobject deviceAttestationDelegate)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "setDeviceAttestationDelegate() called");
    if (deviceAttestationDelegate != nullptr)
    {
        chip::Optional<uint16_t> timeoutSecs  = chip::MakeOptional(static_cast<uint16_t>(failSafeExpiryTimeoutSecs));
        bool shouldWaitAfterDeviceAttestation = false;
        jclass deviceAttestationDelegateCls   = nullptr;
        JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/DeviceAttestationDelegate",
                                                      deviceAttestationDelegateCls);
        VerifyOrExit(deviceAttestationDelegateCls != nullptr, err = CHIP_JNI_ERROR_TYPE_NOT_FOUND);

        if (env->IsInstanceOf(deviceAttestationDelegate, deviceAttestationDelegateCls))
        {
            shouldWaitAfterDeviceAttestation = true;
        }

        err = wrapper->UpdateDeviceAttestationDelegateBridge(deviceAttestationDelegate, timeoutSecs,
                                                             shouldWaitAfterDeviceAttestation);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to set device attestation delegate.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, setAttestationTrustStoreDelegate)
(JNIEnv * env, jobject self, jlong handle, jobject attestationTrustStoreDelegate, jobject cdTrustKeys)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "setAttestationTrustStoreDelegate() called");

    if (attestationTrustStoreDelegate != nullptr)
    {
        err = wrapper->UpdateAttestationTrustStoreBridge(attestationTrustStoreDelegate, cdTrustKeys);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to set device attestation delegate.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, startOTAProvider)(JNIEnv * env, jobject self, jlong handle, jobject otaProviderDelegate)
{
#if CHIP_DEVICE_CONFIG_DYNAMIC_SERVER
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrExit(wrapper != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Controller, "startOTAProvider() called");
    err = wrapper->StartOTAProvider(otaProviderDelegate);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to start OTA Provider. : %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
#endif
}

JNI_METHOD(void, finishOTAProvider)(JNIEnv * env, jobject self, jlong handle)
{
#if CHIP_DEVICE_CONFIG_DYNAMIC_SERVER
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrExit(wrapper != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Controller, "finishOTAProvider() called");

    err = wrapper->FinishOTAProvider();
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to finish OTA Provider. : %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
#endif
}

JNI_METHOD(void, setICDCheckInDelegate)(JNIEnv * env, jobject self, jlong handle, jobject checkInDelegate)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrExit(wrapper != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Controller, "setICDCheckInDelegate() called");

    err = wrapper->SetICDCheckInDelegate(checkInDelegate);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to set ICD Check-In Deleagate. : %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, commissionDevice)
(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jbyteArray csrNonce, jobject networkCredentials)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "commissionDevice() called");

    CommissioningParameters commissioningParams = wrapper->GetCommissioningParameters();
    if (networkCredentials != nullptr)
    {
        err = wrapper->ApplyNetworkCredentials(commissioningParams, networkCredentials);
        VerifyOrExit(err == CHIP_NO_ERROR, err = CHIP_ERROR_INVALID_ARGUMENT);
    }
    if (wrapper->GetDeviceAttestationDelegateBridge() != nullptr)
    {
        commissioningParams.SetDeviceAttestationDelegate(wrapper->GetDeviceAttestationDelegateBridge());
    }
    if (csrNonce != nullptr)
    {
        JniByteArray jniCsrNonce(env, csrNonce);
        commissioningParams.SetCSRNonce(jniCsrNonce.byteSpan());
    }
    err = wrapper->Controller()->Commission(static_cast<chip::NodeId>(deviceId), commissioningParams);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to commission the device.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, pairDevice)
(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jint connObj, jlong pinCode, jbyteArray csrNonce,
 jobject networkCredentials, jobject icdRegistrationInfo)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "pairDevice() called with device ID, connection object, and pincode");

    if (!chip::CanCastTo<uint32_t>(pinCode))
    {
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, CHIP_ERROR_INVALID_ARGUMENT);
        return;
    }

    RendezvousParameters rendezvousParams = RendezvousParameters()
                                                .SetSetupPINCode(static_cast<uint32_t>(pinCode))
#if CONFIG_NETWORK_LAYER_BLE
                                                .SetConnectionObject(reinterpret_cast<BLE_CONNECTION_OBJECT>(connObj))
#endif
                                                .SetPeerAddress(Transport::PeerAddress::BLE());

    CommissioningParameters commissioningParams = wrapper->GetCommissioningParameters();
    wrapper->ApplyNetworkCredentials(commissioningParams, networkCredentials);

    if (csrNonce != nullptr)
    {
        JniByteArray jniCsrNonce(env, csrNonce);
        commissioningParams.SetCSRNonce(jniCsrNonce.byteSpan());
    }

    commissioningParams.SetICDRegistrationStrategy(ICDRegistrationStrategy::kBeforeComplete);
    wrapper->ApplyICDRegistrationInfo(commissioningParams, icdRegistrationInfo);

    if (wrapper->GetDeviceAttestationDelegateBridge() != nullptr)
    {
        commissioningParams.SetDeviceAttestationDelegate(wrapper->GetDeviceAttestationDelegateBridge());
    }
    err = wrapper->Controller()->PairDevice(static_cast<chip::NodeId>(deviceId), rendezvousParams, commissioningParams);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to pair the device.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, pairDeviceWithAddress)
(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jstring address, jint port, jint discriminator, jlong pinCode,
 jbyteArray csrNonce, jobject icdRegistrationInfo)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "pairDeviceWithAddress() called");

    if (!chip::CanCastTo<uint32_t>(pinCode))
    {
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, CHIP_ERROR_INVALID_ARGUMENT);
        return;
    }

    JniUtfString addrJniString(env, address);

    RendezvousParameters rendezvousParams =
        RendezvousParameters()
            .SetDiscriminator(static_cast<uint16_t>(discriminator))
            .SetSetupPINCode(static_cast<uint32_t>(pinCode))
            .SetPeerAddress(Transport::PeerAddress::UDP(const_cast<char *>(addrJniString.c_str()), static_cast<uint16_t>(port)));

    CommissioningParameters commissioningParams = wrapper->GetCommissioningParameters();
    if (csrNonce != nullptr)
    {
        JniByteArray jniCsrNonce(env, csrNonce);
        commissioningParams.SetCSRNonce(jniCsrNonce.byteSpan());
    }

    commissioningParams.SetICDRegistrationStrategy(ICDRegistrationStrategy::kBeforeComplete);
    wrapper->ApplyICDRegistrationInfo(commissioningParams, icdRegistrationInfo);

    if (wrapper->GetDeviceAttestationDelegateBridge() != nullptr)
    {
        commissioningParams.SetDeviceAttestationDelegate(wrapper->GetDeviceAttestationDelegateBridge());
    }
    err = wrapper->Controller()->PairDevice(static_cast<chip::NodeId>(deviceId), rendezvousParams, commissioningParams);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to pair the device.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, pairDeviceWithCode)
(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jstring setUpCode, jboolean discoverOnce,
 jboolean useOnlyOnNetworkDiscovery, jbyteArray csrNonce, jobject networkCredentials, jobject icdRegistrationInfo)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "pairDeviceWithCode() called");

    JniUtfString setUpCodeJniString(env, setUpCode);

    CommissioningParameters commissioningParams = wrapper->GetCommissioningParameters();

    auto discoveryType = DiscoveryType::kAll;
    if (useOnlyOnNetworkDiscovery)
    {
        discoveryType = DiscoveryType::kDiscoveryNetworkOnly;
    }

    if (discoverOnce)
    {
        discoveryType = DiscoveryType::kDiscoveryNetworkOnlyWithoutPASEAutoRetry;
    }

    if (csrNonce != nullptr)
    {
        JniByteArray jniCsrNonce(env, csrNonce);
        commissioningParams.SetCSRNonce(jniCsrNonce.byteSpan());
    }

    if (networkCredentials != nullptr)
    {
        wrapper->ApplyNetworkCredentials(commissioningParams, networkCredentials);
    }

    commissioningParams.SetICDRegistrationStrategy(ICDRegistrationStrategy::kBeforeComplete);
    wrapper->ApplyICDRegistrationInfo(commissioningParams, icdRegistrationInfo);

    if (wrapper->GetDeviceAttestationDelegateBridge() != nullptr)
    {
        commissioningParams.SetDeviceAttestationDelegate(wrapper->GetDeviceAttestationDelegateBridge());
    }
    err = wrapper->Controller()->PairDevice(static_cast<chip::NodeId>(deviceId), setUpCodeJniString.c_str(), commissioningParams,
                                            discoveryType);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to pair the device.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, establishPaseConnection)(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jint connObj, jlong pinCode)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    if (!chip::CanCastTo<uint32_t>(pinCode))
    {
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, CHIP_ERROR_INVALID_ARGUMENT);
        return;
    }

    RendezvousParameters rendezvousParams = RendezvousParameters()
                                                .SetSetupPINCode(static_cast<uint32_t>(pinCode))
#if CONFIG_NETWORK_LAYER_BLE
                                                .SetConnectionObject(reinterpret_cast<BLE_CONNECTION_OBJECT>(connObj))
#endif
                                                .SetPeerAddress(Transport::PeerAddress::BLE());

    err = wrapper->Controller()->EstablishPASEConnection(static_cast<chip::NodeId>(deviceId), rendezvousParams);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to establish PASE connection.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, establishPaseConnectionByAddress)
(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jstring address, jint port, jlong pinCode)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    if (!chip::CanCastTo<uint32_t>(pinCode))
    {
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, CHIP_ERROR_INVALID_ARGUMENT);
        return;
    }

    JniUtfString addrJniString(env, address);

    RendezvousParameters rendezvousParams =
        RendezvousParameters()
            .SetSetupPINCode(static_cast<uint32_t>(pinCode))
            .SetPeerAddress(Transport::PeerAddress::UDP(const_cast<char *>(addrJniString.c_str()), static_cast<uint16_t>(port)));

    err = wrapper->Controller()->EstablishPASEConnection(static_cast<chip::NodeId>(deviceId), rendezvousParams);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to establish PASE connection.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, continueCommissioning)
(JNIEnv * env, jobject self, jlong handle, jlong devicePtr, jboolean ignoreAttestationFailure)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(Controller, "continueCommissioning() called.");
    CHIP_ERROR err                                                    = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper                          = AndroidDeviceControllerWrapper::FromJNIHandle(handle);
    DeviceAttestationDelegateBridge * deviceAttestationDelegateBridge = wrapper->GetDeviceAttestationDelegateBridge();
    auto lastAttestationResult = deviceAttestationDelegateBridge ? deviceAttestationDelegateBridge->attestationVerificationResult()
                                                                 : chip::Credentials::AttestationVerificationResult::kSuccess;
    chip::DeviceProxy * deviceProxy = reinterpret_cast<chip::DeviceProxy *>(devicePtr);
    err                             = wrapper->Controller()->ContinueCommissioningAfterDeviceAttestation(
        deviceProxy, ignoreAttestationFailure ? chip::Credentials::AttestationVerificationResult::kSuccess : lastAttestationResult);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to continue commissioning.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, setUseJavaCallbackForNOCRequest)
(JNIEnv * env, jobject self, jlong handle, jboolean useCallback)
{
    ChipLogProgress(Controller, "setUseJavaCallbackForNOCRequest() called");
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

#ifndef JAVA_MATTER_CONTROLLER_TEST
    wrapper->GetAndroidOperationalCredentialsIssuer()->SetUseJavaCallbackForNOCRequest(useCallback);
#endif

    if (useCallback)
    {
        // if we are assigning a callback, then make the device commissioner delegate verification to the
        // PartialDACVerifier so that DAC chain and CD validation can be performed by custom code
        // triggered by ChipDeviceController.NOCChainIssuer.onNOCChainGenerationNeeded().
        wrapper->Controller()->SetDeviceAttestationVerifier(wrapper->GetPartialDACVerifier());
    }
    else
    {
        // if we are setting callback to null, then make the device commissioner use the default verifier
        wrapper->Controller()->SetDeviceAttestationVerifier(GetDeviceAttestationVerifier());
    }
}

JNI_METHOD(void, updateCommissioningNetworkCredentials)
(JNIEnv * env, jobject self, jlong handle, jobject networkCredentials)
{
    ChipLogProgress(Controller, "updateCommissioningNetworkCredentials() called");
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    CommissioningParameters commissioningParams = wrapper->GetAutoCommissioner()->GetCommissioningParameters();
    CHIP_ERROR err                              = wrapper->ApplyNetworkCredentials(commissioningParams, networkCredentials);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "ApplyNetworkCredentials failed. Err = %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
        return;
    }
    err = wrapper->GetAutoCommissioner()->SetCommissioningParameters(commissioningParams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "SetCommissioningParameters failed. Err = %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
        return;
    }

    // Only invoke NetworkCredentialsReady when called in response to NetworkScan result
    if (wrapper->Controller()->GetCommissioningStage() == CommissioningStage::kNeedsNetworkCreds)
    {
        err = wrapper->Controller()->NetworkCredentialsReady();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "NetworkCredentialsReady failed. Err = %" CHIP_ERROR_FORMAT, err.Format());
            JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
        }
    }
}

JNI_METHOD(void, updateCommissioningICDRegistrationInfo)
(JNIEnv * env, jobject self, jlong handle, jobject icdRegistrationInfo)
{
    ChipLogProgress(Controller, "updateCommissioningICDRegistrationInfo() called");
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    CommissioningParameters commissioningParams = wrapper->GetAutoCommissioner()->GetCommissioningParameters();
    CHIP_ERROR err                              = wrapper->ApplyICDRegistrationInfo(commissioningParams, icdRegistrationInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "ApplyICDRegistrationInfo failed. Err = %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
        return;
    }
    err = wrapper->GetAutoCommissioner()->SetCommissioningParameters(commissioningParams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "SetCommissioningParameters failed. Err = %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
        return;
    }

    // Only invoke ICDRegistrationInfoReady when called in ICDRegistartionInfo stage.
    if (wrapper->Controller()->GetCommissioningStage() == CommissioningStage::kICDGetRegistrationInfo)
    {
        err = wrapper->Controller()->ICDRegistrationInfoReady();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "ICDRegistrationInfoReady failed. Err = %" CHIP_ERROR_FORMAT, err.Format());
            JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
        }
    }
}

jint GetCalendarFieldID(JNIEnv * env, const char * method)
{
    jclass calendarCls = env->FindClass("java/util/Calendar");
    jfieldID fieldID   = env->GetStaticFieldID(calendarCls, method, "I");
    return env->GetStaticIntField(calendarCls, fieldID);
}

CHIP_ERROR GetEpochTime(JNIEnv * env, jobject calendar, uint32_t & epochTime)
{
    using namespace ASN1;
    ASN1UniversalTime universalTime;

    jmethodID getMethod = nullptr;

    jint yearID   = GetCalendarFieldID(env, "YEAR");
    jint monthID  = GetCalendarFieldID(env, "MONTH");
    jint dayID    = GetCalendarFieldID(env, "DAY_OF_MONTH");
    jint hourID   = GetCalendarFieldID(env, "HOUR_OF_DAY");
    jint minuteID = GetCalendarFieldID(env, "MINUTE");
    jint secondID = GetCalendarFieldID(env, "SECOND");

    if (calendar == nullptr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, calendar, "get", "(I)I", &getMethod));

    universalTime.Year = static_cast<uint16_t>(env->CallIntMethod(calendar, getMethod, yearID));
    // The first month of the year in the Gregorian and Julian calendars is JANUARY which is 0. See detailed in
    // https://docs.oracle.com/javase/8/docs/api/java/util/Calendar.html#MONTH
    universalTime.Month  = static_cast<uint8_t>(static_cast<uint8_t>(env->CallIntMethod(calendar, getMethod, monthID)) + 1u);
    universalTime.Day    = static_cast<uint8_t>(env->CallIntMethod(calendar, getMethod, dayID));
    universalTime.Hour   = static_cast<uint8_t>(env->CallIntMethod(calendar, getMethod, hourID));
    universalTime.Minute = static_cast<uint8_t>(env->CallIntMethod(calendar, getMethod, minuteID));
    universalTime.Second = static_cast<uint8_t>(env->CallIntMethod(calendar, getMethod, secondID));

    ReturnErrorOnFailure(ASN1ToChipEpochTime(universalTime, epochTime));

    return CHIP_NO_ERROR;
}

JNI_METHOD(jbyteArray, createRootCertificate)
(JNIEnv * env, jclass clazz, jobject jKeypair, jlong issuerId, jobject fabricId, jobject validityStart, jobject validityEnd)
{
#ifdef JAVA_MATTER_CONTROLLER_TEST
    return nullptr;
#else
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint32_t allocatedCertLength = chip::Credentials::kMaxDERCertLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> outBuf;
    jbyteArray outRcac = nullptr;
    CHIPP256KeypairBridge keypair;
    Optional<FabricId> fabric = Optional<FabricId>();

    VerifyOrExit(outBuf.Alloc(allocatedCertLength), err = CHIP_ERROR_NO_MEMORY);

    keypair.SetDelegate(jKeypair);
    err = keypair.Initialize(Crypto::ECPKeyTarget::ECDSA);
    SuccessOrExit(err);

    if (fabricId != nullptr)
    {
        jlong jfabricId = chip::JniReferences::GetInstance().LongToPrimitive(fabricId);
        fabric          = MakeOptional(static_cast<FabricId>(jfabricId));
    }

    {
        MutableByteSpan rcac(outBuf.Get(), allocatedCertLength);

        uint32_t start;
        uint32_t end;

        err = GetEpochTime(env, validityStart, start);
        SuccessOrExit(err);

        err = GetEpochTime(env, validityEnd, end);
        SuccessOrExit(err);

        err = AndroidOperationalCredentialsIssuer::GenerateRootCertificate(keypair, static_cast<uint64_t>(issuerId), fabric, start,
                                                                           end, rcac);
        SuccessOrExit(err);

        err = JniReferences::GetInstance().N2J_ByteArray(env, rcac.data(), static_cast<uint32_t>(rcac.size()), outRcac);
        SuccessOrExit(err);
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to create Root Certificate. Err = %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }

    return outRcac;
#endif
}

JNI_METHOD(jbyteArray, createIntermediateCertificate)
(JNIEnv * env, jclass clazz, jobject rootKeypair, jbyteArray rootCertificate, jbyteArray intermediatePublicKey, jlong issuerId,
 jobject fabricId, jobject validityStart, jobject validityEnd)
{
#ifdef JAVA_MATTER_CONTROLLER_TEST
    return nullptr;
#else
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint32_t allocatedCertLength = chip::Credentials::kMaxDERCertLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> outBuf;
    jbyteArray outIcac = nullptr;
    CHIPP256KeypairBridge keypair;
    Optional<FabricId> fabric = Optional<FabricId>();

    chip::JniByteArray jniRcac(env, rootCertificate);
    chip::JniByteArray jnipublicKey(env, intermediatePublicKey);

    Credentials::P256PublicKeySpan publicKeySpan(reinterpret_cast<const uint8_t *>(jnipublicKey.data()));
    Crypto::P256PublicKey publicKey{ publicKeySpan };

    VerifyOrExit(outBuf.Alloc(allocatedCertLength), err = CHIP_ERROR_NO_MEMORY);

    keypair.SetDelegate(rootKeypair);
    err = keypair.Initialize(Crypto::ECPKeyTarget::ECDSA);
    SuccessOrExit(err);

    if (fabricId != nullptr)
    {
        jlong jfabricId = chip::JniReferences::GetInstance().LongToPrimitive(fabricId);
        fabric          = MakeOptional(static_cast<FabricId>(jfabricId));
    }

    {
        MutableByteSpan icac(outBuf.Get(), allocatedCertLength);

        uint32_t start;
        uint32_t end;

        err = GetEpochTime(env, validityStart, start);
        SuccessOrExit(err);

        err = GetEpochTime(env, validityEnd, end);
        SuccessOrExit(err);

        err = AndroidOperationalCredentialsIssuer::GenerateIntermediateCertificate(
            keypair, jniRcac.byteSpan(), publicKey, static_cast<uint64_t>(issuerId), fabric, start, end, icac);
        SuccessOrExit(err);

        ChipLogByteSpan(Controller, icac);

        err = JniReferences::GetInstance().N2J_ByteArray(env, icac.data(), static_cast<uint32_t>(icac.size()), outIcac);
        SuccessOrExit(err);
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to create Intermediate Certificate. Err = %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }

    return outIcac;
#endif
}

JNI_METHOD(jbyteArray, createOperationalCertificate)
(JNIEnv * env, jclass clazz, jobject signingKeypair, jbyteArray signingCertificate, jbyteArray operationalPublicKey, jlong fabricId,
 jlong nodeId, jobject caseAuthenticatedTags, jobject validityStart, jobject validityEnd)
{
#ifdef JAVA_MATTER_CONTROLLER_TEST
    return nullptr;
#else
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint32_t allocatedCertLength = chip::Credentials::kMaxDERCertLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> outBuf;
    jbyteArray outNoc = nullptr;
    CHIPP256KeypairBridge keypair;

    chip::JniByteArray jniCert(env, signingCertificate);
    chip::JniByteArray jnipublicKey(env, operationalPublicKey);

    Credentials::P256PublicKeySpan publicKeySpan(reinterpret_cast<const uint8_t *>(jnipublicKey.data()));
    Crypto::P256PublicKey publicKey{ publicKeySpan };

    chip::CATValues cats = chip::kUndefinedCATs;
    if (caseAuthenticatedTags != nullptr)
    {
        jint size;
        JniReferences::GetInstance().GetListSize(caseAuthenticatedTags, size);
        VerifyOrExit(static_cast<size_t>(size) <= chip::kMaxSubjectCATAttributeCount, err = CHIP_ERROR_INVALID_ARGUMENT);

        for (jint i = 0; i < size; i++)
        {
            jobject cat = nullptr;
            JniReferences::GetInstance().GetListItem(caseAuthenticatedTags, i, cat);
            VerifyOrExit(cat != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
            cats.values[i] = static_cast<uint32_t>(JniReferences::GetInstance().IntegerToPrimitive(cat));
        }
    }

    VerifyOrExit(outBuf.Alloc(allocatedCertLength), err = CHIP_ERROR_NO_MEMORY);

    keypair.SetDelegate(signingKeypair);
    err = keypair.Initialize(Crypto::ECPKeyTarget::ECDSA);
    SuccessOrExit(err);
    {
        MutableByteSpan noc(outBuf.Get(), allocatedCertLength);

        uint32_t start;
        uint32_t end;

        err = GetEpochTime(env, validityStart, start);
        SuccessOrExit(err);

        err = GetEpochTime(env, validityEnd, end);
        SuccessOrExit(err);

        err = AndroidOperationalCredentialsIssuer::GenerateOperationalCertificate(
            keypair, jniCert.byteSpan(), publicKey, static_cast<uint64_t>(fabricId), static_cast<uint64_t>(nodeId), cats, start,
            end, noc);
        SuccessOrExit(err);

        ChipLogByteSpan(Controller, noc);

        err = JniReferences::GetInstance().N2J_ByteArray(env, noc.data(), static_cast<uint32_t>(noc.size()), outNoc);
        SuccessOrExit(err);
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to create Intermediate Certificate. Err = %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }

    return outNoc;
#endif
}

JNI_METHOD(jbyteArray, publicKeyFromCSR)
(JNIEnv * env, jclass clazz, jbyteArray certificateSigningRequest)
{
    jbyteArray outJbytes = nullptr;

    chip::JniByteArray jniCsr(env, certificateSigningRequest);
    P256PublicKey publicKey;
    CHIP_ERROR err = VerifyCertificateSigningRequest(jniCsr.byteSpan().data(), jniCsr.byteSpan().size(), publicKey);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "publicKeyFromCSR: %" CHIP_ERROR_FORMAT, err.Format());
        return nullptr;
    }

    err = JniReferences::GetInstance().N2J_ByteArray(env, publicKey.Bytes(), static_cast<jsize>(publicKey.Length()), outJbytes);
    SuccessOrExit(err);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to publicKeyFromCSR. Err = %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }

    return outJbytes;
}

JNI_METHOD(jbyteArray, convertX509CertToMatterCert)
(JNIEnv * env, jobject self, jbyteArray x509Cert)
{
    chip::DeviceLayer::StackLock lock;

    uint32_t allocatedCertLength = chip::Credentials::kMaxCHIPCertLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> outBuf;
    jbyteArray outJbytes = nullptr;
    JniByteArray x509CertBytes(env, x509Cert);

    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(outBuf.Alloc(allocatedCertLength), err = CHIP_ERROR_NO_MEMORY);

    {
        MutableByteSpan outBytes(outBuf.Get(), allocatedCertLength);

        err = chip::Credentials::ConvertX509CertToChipCert(x509CertBytes.byteSpan(), outBytes);
        SuccessOrExit(err);

        VerifyOrExit(chip::CanCastTo<uint32_t>(outBytes.size()), err = CHIP_ERROR_INTERNAL);

        err = JniReferences::GetInstance().N2J_ByteArray(env, outBytes.data(), static_cast<jsize>(outBytes.size()), outJbytes);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to convert X509 cert to CHIP cert. Err = %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }

    return outJbytes;
}

JNI_METHOD(void, unpairDevice)(JNIEnv * env, jobject self, jlong handle, jlong deviceId)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "unpairDevice() called with device ID");

    err = wrapper->Controller()->UnpairDevice(static_cast<chip::NodeId>(deviceId));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to unpair the device.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, unpairDeviceCallback)(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jobject callback)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "unpairDeviceCallback() called with device ID and callback object");

    err = AndroidCurrentFabricRemover::RemoveCurrentFabric(wrapper->Controller(), static_cast<NodeId>(deviceId), callback);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to unpair the device.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, stopDevicePairing)(JNIEnv * env, jobject self, jlong handle, jlong deviceId)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "stopDevicePairing() called with device ID");

    err = wrapper->Controller()->StopPairing(static_cast<chip::NodeId>(deviceId));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to unpair the device.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(jlong, getDeviceBeingCommissionedPointer)(JNIEnv * env, jobject self, jlong handle, jlong nodeId)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    CommissioneeDeviceProxy * commissioneeDevice = nullptr;
    err = wrapper->Controller()->GetDeviceBeingCommissioned(static_cast<NodeId>(nodeId), &commissioneeDevice);

    if (commissioneeDevice == nullptr)
    {
        ChipLogError(Controller, "Commissionee device was nullptr");
        err = CHIP_ERROR_INCORRECT_STATE;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to get commissionee device: %s", ErrorStr(err));
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
        return 0;
    }

    return reinterpret_cast<jlong>(commissioneeDevice);
}

JNI_METHOD(void, getConnectedDevicePointer)(JNIEnv * env, jobject self, jlong handle, jlong nodeId, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    GetConnectedDeviceCallback * connectedDeviceCallback = reinterpret_cast<GetConnectedDeviceCallback *>(callbackHandle);
    VerifyOrExit(connectedDeviceCallback != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    err = wrapper->Controller()->GetConnectedDevice(static_cast<chip::NodeId>(nodeId), &connectedDeviceCallback->mOnSuccess,
                                                    &connectedDeviceCallback->mOnFailure);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "GetConnectedDevice failed: %" CHIP_ERROR_FORMAT, err.Format());
        OperationalSessionSetup::ConnnectionFailureInfo failureInfo(
            chip::ScopedNodeId(static_cast<chip::NodeId>(nodeId), wrapper->Controller()->GetFabricIndex()), err,
            SessionEstablishmentStage::kUnknown);
        connectedDeviceCallback->mOnFailure.mCall(&connectedDeviceCallback->mOnFailure.mContext, failureInfo);
    }
}

JNI_METHOD(void, releaseOperationalDevicePointer)(JNIEnv * env, jobject self, jlong devicePtr)
{
    chip::DeviceLayer::StackLock lock;
    OperationalDeviceProxy * device = reinterpret_cast<OperationalDeviceProxy *>(devicePtr);
    if (device != nullptr)
    {
        delete device;
    }
}

JNI_METHOD(jlong, getGroupDevicePointer)(JNIEnv * env, jobject self, jlong handle, jint groupId)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrReturnValue(wrapper != nullptr, 0, ChipLogError(Controller, "wrapper is null"));

    GroupDeviceProxy * device = new GroupDeviceProxy(static_cast<GroupId>(groupId), wrapper->Controller()->GetFabricIndex(),
                                                     wrapper->Controller()->ExchangeMgr());

    if (device == nullptr)
    {
        CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;
        ChipLogError(Controller, "GroupDeviceProxy handle is nullptr: %s", ErrorStr(err));
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
        return 0;
    }

    return reinterpret_cast<jlong>(device);
}

JNI_METHOD(void, releaseGroupDevicePointer)(JNIEnv * env, jobject self, jlong devicePtr)
{
    chip::DeviceLayer::StackLock lock;
    GroupDeviceProxy * device = reinterpret_cast<GroupDeviceProxy *>(devicePtr);
    if (device != nullptr)
    {
        delete device;
    }
}

JNI_METHOD(jobject, getAvailableGroupIds)(JNIEnv * env, jobject self, jlong handle)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrReturnValue(wrapper != nullptr, nullptr, ChipLogError(Controller, "wrapper is null"));

    CHIP_ERROR err                                           = CHIP_NO_ERROR;
    chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
    auto it = groupDataProvider->IterateGroupInfo(wrapper->Controller()->GetFabricIndex());

    jobject groupIds;
    err = chip::JniReferences::GetInstance().CreateArrayList(groupIds);

    chip::Credentials::GroupDataProvider::GroupInfo group;

    if (it)
    {
        while (it->Next(group))
        {
            jobject jGroupId;
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>("java/lang/Integer", "(I)V",
                                                                       static_cast<jint>(group.group_id), jGroupId);
            chip::JniReferences::GetInstance().AddToList(groupIds, jGroupId);
        }
    }

    return groupIds;
}

JNI_METHOD(jstring, getGroupName)(JNIEnv * env, jobject self, jlong handle, jint jGroupId)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrReturnValue(wrapper != nullptr, nullptr, ChipLogError(Controller, "wrapper is null"));

    chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
    auto it = groupDataProvider->IterateGroupInfo(wrapper->Controller()->GetFabricIndex());

    GroupId groupId = static_cast<GroupId>(jGroupId);
    chip::Credentials::GroupDataProvider::GroupInfo group;

    if (it)
    {
        while (it->Next(group))
        {
            if (group.group_id == groupId)
            {
                return env->NewStringUTF(group.name);
            }
        }
    }

    return nullptr;
}

JNI_METHOD(jobject, findKeySetId)(JNIEnv * env, jobject self, jlong handle, jint jGroupId)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrReturnValue(wrapper != nullptr, nullptr, ChipLogError(Controller, "wrapper is null"));

    chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
    auto iter = groupDataProvider->IterateGroupKeys(wrapper->Controller()->GetFabricIndex());
    chip::Credentials::GroupDataProvider::GroupKey groupKey;
    GroupId groupId = static_cast<GroupId>(jGroupId);
    jobject wrapperKeyId;

    if (iter)
    {
        while (iter->Next(groupKey))
        {
            if (groupKey.group_id == groupId)
            {
                jobject jKeyId;
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>("java/lang/Integer", "(I)V",
                                                                           static_cast<jint>(groupKey.keyset_id), jKeyId);
                chip::JniReferences::GetInstance().CreateOptional(jKeyId, wrapperKeyId);
                iter->Release();
                return wrapperKeyId;
            }
        }
        iter->Release();
    }
    chip::JniReferences::GetInstance().CreateOptional(nullptr, wrapperKeyId);
    return wrapperKeyId;
}

JNI_METHOD(jboolean, addGroup)(JNIEnv * env, jobject self, jlong handle, jint jGroupId, jstring groupName)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrReturnValue(wrapper != nullptr, JNI_FALSE, ChipLogError(Controller, "wrapper is null"));

    chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
    chip::Credentials::GroupDataProvider::GroupInfo group;

    chip::JniUtfString jniGroupName(env, groupName);
    group.SetName(jniGroupName.charSpan());
    group.group_id = static_cast<GroupId>(jGroupId);

    CHIP_ERROR err = groupDataProvider->SetGroupInfo(wrapper->Controller()->GetFabricIndex(), group);

    return err == CHIP_NO_ERROR ? JNI_TRUE : JNI_FALSE;
}

JNI_METHOD(jboolean, removeGroup)(JNIEnv * env, jobject self, jlong handle, jint jGroupId)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrReturnValue(wrapper != nullptr, JNI_FALSE, ChipLogError(Controller, "wrapper is null"));

    chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
    CHIP_ERROR err = groupDataProvider->RemoveGroupInfo(wrapper->Controller()->GetFabricIndex(), static_cast<GroupId>(jGroupId));

    return err == CHIP_NO_ERROR ? JNI_TRUE : JNI_FALSE;
}

JNI_METHOD(jobject, getKeySetIds)(JNIEnv * env, jobject self, jlong handle)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrReturnValue(wrapper != nullptr, nullptr, ChipLogError(Controller, "wrapper is null"));

    CHIP_ERROR err                                           = CHIP_NO_ERROR;
    chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
    auto it = groupDataProvider->IterateKeySets(wrapper->Controller()->GetFabricIndex());

    jobject keySetIds;
    err = chip::JniReferences::GetInstance().CreateArrayList(keySetIds);

    chip::Credentials::GroupDataProvider::KeySet keySet;

    if (it)
    {
        while (it->Next(keySet))
        {
            jobject jKeySetId;
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>("java/lang/Integer", "(I)V",
                                                                       static_cast<jint>(keySet.keyset_id), jKeySetId);
            chip::JniReferences::GetInstance().AddToList(keySetIds, jKeySetId);
        }
        it->Release();
    }

    return keySetIds;
}

JNI_METHOD(jobject, getKeySecurityPolicy)(JNIEnv * env, jobject self, jlong handle, int jKeySetId)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrReturnValue(wrapper != nullptr, nullptr, ChipLogError(Controller, "wrapper is null"));

    chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
    auto it = groupDataProvider->IterateKeySets(wrapper->Controller()->GetFabricIndex());

    chip::Credentials::GroupDataProvider::KeySet keySet;

    uint16_t keySetId = static_cast<uint16_t>(jKeySetId);
    jobject wrapperKeyPolicy;

    if (it)
    {
        while (it->Next(keySet))
        {
            if (keySet.keyset_id == keySetId)
            {
                jobject jKeyPolicy;
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>("java/lang/Integer", "(I)V",
                                                                           static_cast<jint>(keySet.policy), jKeyPolicy);
                chip::JniReferences::GetInstance().CreateOptional(jKeyPolicy, wrapperKeyPolicy);
                it->Release();
                return wrapperKeyPolicy;
            }
        }
        it->Release();
    }
    chip::JniReferences::GetInstance().CreateOptional(nullptr, wrapperKeyPolicy);
    return wrapperKeyPolicy;
}

JNI_METHOD(jboolean, bindKeySet)(JNIEnv * env, jobject self, jlong handle, jint jGroupId, jint jKeySetId)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrReturnValue(wrapper != nullptr, JNI_FALSE, ChipLogError(Controller, "wrapper is null"));

    chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
    auto iter            = groupDataProvider->IterateGroupKeys(wrapper->Controller()->GetFabricIndex());
    size_t current_count = iter->Count();

    iter->Release();
    CHIP_ERROR err = groupDataProvider->SetGroupKeyAt(
        wrapper->Controller()->GetFabricIndex(), current_count,
        chip::Credentials::GroupDataProvider::GroupKey(static_cast<uint16_t>(jGroupId), static_cast<uint16_t>(jKeySetId)));
    return err == CHIP_NO_ERROR ? JNI_TRUE : JNI_FALSE;
}

JNI_METHOD(jboolean, unbindKeySet)(JNIEnv * env, jobject self, jlong handle, jint jGroupId, jint jKeySetId)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrReturnValue(wrapper != nullptr, JNI_FALSE, ChipLogError(Controller, "wrapper is null"));

    size_t index                                             = 0;
    chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
    auto iter       = groupDataProvider->IterateGroupKeys(wrapper->Controller()->GetFabricIndex());
    size_t maxCount = iter->Count();
    chip::Credentials::GroupDataProvider::GroupKey groupKey;

    GroupId groupId   = static_cast<GroupId>(jGroupId);
    uint16_t keysetId = static_cast<uint16_t>(jKeySetId);

    while (iter->Next(groupKey))
    {
        if (groupKey.group_id == groupId && groupKey.keyset_id == keysetId)
        {
            break;
        }
        index++;
    }
    iter->Release();
    if (index >= maxCount)
    {
        return JNI_FALSE;
    }

    CHIP_ERROR err = groupDataProvider->RemoveGroupKeyAt(wrapper->Controller()->GetFabricIndex(), index);

    return err == CHIP_NO_ERROR ? JNI_TRUE : JNI_FALSE;
}

JNI_METHOD(jboolean, addKeySet)
(JNIEnv * env, jobject self, jlong handle, jint jKeySetId, jint jKeyPolicy, jlong validityTime, jbyteArray epochKey)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrReturnValue(wrapper != nullptr, JNI_FALSE, ChipLogError(Controller, "wrapper is null"));

    chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
    uint8_t compressed_fabric_id[sizeof(chip::FabricId)];
    chip::MutableByteSpan compressed_fabric_id_span(compressed_fabric_id);
    VerifyOrReturnValue(wrapper->Controller()->GetCompressedFabricIdBytes(compressed_fabric_id_span) == CHIP_NO_ERROR, JNI_FALSE);

    chip::Credentials::GroupDataProvider::SecurityPolicy keyPolicy =
        static_cast<chip::Credentials::GroupDataProvider::SecurityPolicy>(jKeyPolicy);
    chip::JniByteArray jniEpochKey(env, epochKey);
    size_t epochKeySize = static_cast<size_t>(jniEpochKey.size());
    if ((keyPolicy != chip::Credentials::GroupDataProvider::SecurityPolicy::kCacheAndSync &&
         keyPolicy != chip::Credentials::GroupDataProvider::SecurityPolicy::kTrustFirst) ||
        epochKeySize != chip::Credentials::GroupDataProvider::EpochKey::kLengthBytes)
    {
        return JNI_FALSE;
    }

    chip::Credentials::GroupDataProvider::KeySet keySet(static_cast<uint16_t>(jKeySetId), keyPolicy, 1);
    chip::Credentials::GroupDataProvider::EpochKey epoch_key;
    epoch_key.start_time = static_cast<uint64_t>(validityTime);
    memcpy(epoch_key.key, jniEpochKey.byteSpan().data(), chip::Credentials::GroupDataProvider::EpochKey::kLengthBytes);
    memcpy(keySet.epoch_keys, &epoch_key, sizeof(chip::Credentials::GroupDataProvider::EpochKey));

    VerifyOrReturnValue(groupDataProvider->SetKeySet(wrapper->Controller()->GetFabricIndex(), compressed_fabric_id_span, keySet) ==
                            CHIP_NO_ERROR,
                        JNI_FALSE);

    return JNI_TRUE;
}

JNI_METHOD(jboolean, removeKeySet)(JNIEnv * env, jobject self, jlong handle, jint jKeySetId)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrReturnValue(wrapper != nullptr, JNI_FALSE, ChipLogError(Controller, "wrapper is null"));

    CHIP_ERROR err                                           = CHIP_NO_ERROR;
    chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();

    size_t index      = 0;
    auto iter         = groupDataProvider->IterateGroupKeys(wrapper->Controller()->GetFabricIndex());
    uint16_t keysetId = static_cast<uint16_t>(jKeySetId);
    chip::Credentials::GroupDataProvider::GroupKey groupKey;
    if (iter)
    {
        while (iter->Next(groupKey))
        {
            if (groupKey.keyset_id == keysetId)
            {
                err = groupDataProvider->RemoveGroupKeyAt(wrapper->Controller()->GetFabricIndex(), index);
                if (err != CHIP_NO_ERROR)
                {
                    break;
                }
            }
            index++;
        }
        iter->Release();
        if (err == CHIP_NO_ERROR)
        {
            err = groupDataProvider->RemoveKeySet(wrapper->Controller()->GetFabricIndex(), keysetId);
        }
        return err == CHIP_NO_ERROR ? JNI_TRUE : JNI_FALSE;
    }

    return JNI_FALSE;
}

JNI_METHOD(jint, getFabricIndex)(JNIEnv * env, jobject self, jlong handle)
{
    chip::DeviceLayer::StackLock lock;

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    return wrapper->Controller()->GetFabricIndex();
}

JNI_METHOD(void, shutdownSubscriptions)
(JNIEnv * env, jobject self, jobject handle, jobject fabricIndex, jobject peerNodeId, jobject subscriptionId)
{
    chip::DeviceLayer::StackLock lock;
    if (fabricIndex == nullptr && peerNodeId == nullptr && subscriptionId == nullptr)
    {
        app::InteractionModelEngine::GetInstance()->ShutdownAllSubscriptions();
        return;
    }

    if (fabricIndex != nullptr && peerNodeId != nullptr && subscriptionId == nullptr)
    {
        jint jFabricIndex = chip::JniReferences::GetInstance().IntegerToPrimitive(fabricIndex);
        jlong jPeerNodeId = chip::JniReferences::GetInstance().LongToPrimitive(peerNodeId);
        app::InteractionModelEngine::GetInstance()->ShutdownSubscriptions(static_cast<chip::FabricIndex>(jFabricIndex),
                                                                          static_cast<chip::NodeId>(jPeerNodeId));
        return;
    }

    if (fabricIndex != nullptr && peerNodeId == nullptr && subscriptionId == nullptr)
    {
        jint jFabricIndex = chip::JniReferences::GetInstance().IntegerToPrimitive(fabricIndex);
        app::InteractionModelEngine::GetInstance()->ShutdownSubscriptions(static_cast<chip::FabricIndex>(jFabricIndex));
        return;
    }

    if (fabricIndex != nullptr && peerNodeId != nullptr && subscriptionId != nullptr)
    {
        jint jFabricIndex     = chip::JniReferences::GetInstance().IntegerToPrimitive(fabricIndex);
        jlong jPeerNodeId     = chip::JniReferences::GetInstance().LongToPrimitive(peerNodeId);
        jlong jSubscriptionId = chip::JniReferences::GetInstance().LongToPrimitive(subscriptionId);
        app::InteractionModelEngine::GetInstance()->ShutdownSubscription(
            chip::ScopedNodeId(static_cast<chip::NodeId>(jPeerNodeId), static_cast<chip::FabricIndex>(jFabricIndex)),
            static_cast<chip::SubscriptionId>(jSubscriptionId));
        return;
    }
    ChipLogError(Controller, "Failed to shutdown subscriptions with correct input paramemeter");
}

JNI_METHOD(jstring, getIpAddress)(JNIEnv * env, jobject self, jlong handle, jlong deviceId)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    chip::Inet::IPAddress addr;
    uint16_t port;
    char addrStr[50];

    CHIP_ERROR err = wrapper->Controller()->GetPeerAddressAndPort(static_cast<chip::NodeId>(deviceId), addr, port);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to get device address.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
        return nullptr;
    }

    addr.ToString(addrStr);
    return env->NewStringUTF(addrStr);
}

JNI_METHOD(jlong, generateCompressedFabricId)
(JNIEnv * env, jobject self, jbyteArray rcac, jbyteArray noc)
{
    chip::DeviceLayer::StackLock lock;
    CompressedFabricId compressedFabricId;
    FabricId fabricId;
    NodeId nodeId;
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::JniByteArray jniRcac(env, rcac);
    chip::JniByteArray jniNoc(env, noc);
    err = ExtractNodeIdFabricIdCompressedFabricIdFromOpCerts(jniRcac.byteSpan(), jniNoc.byteSpan(), compressedFabricId, fabricId,
                                                             nodeId);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to extract compressed fabric ID.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }

    return static_cast<jlong>(compressedFabricId);
}

JNI_METHOD(jobject, getNetworkLocation)(JNIEnv * env, jobject self, jlong handle, jlong deviceId)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    Transport::PeerAddress addr;
    jobject networkLocation;
    char addrStr[50];

    CHIP_ERROR err = wrapper->Controller()->GetPeerAddress(static_cast<chip::NodeId>(deviceId), addr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to get device address.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
        return nullptr;
    }

    addr.GetIPAddress().ToString(addrStr);

    err = N2J_NetworkLocation(env, env->NewStringUTF(addrStr), static_cast<jint>(addr.GetPort()),
                              static_cast<jint>(addr.GetInterface().GetPlatformInterface()), networkLocation);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to create NetworkLocation");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }

    return networkLocation;
}

JNI_METHOD(jlong, getCompressedFabricId)(JNIEnv * env, jobject self, jlong handle)
{
    chip::DeviceLayer::StackLock lock;

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    return static_cast<jlong>(wrapper->Controller()->GetCompressedFabricId());
}

JNI_METHOD(jlong, getControllerNodeId)(JNIEnv * env, jobject self, jlong handle)
{
    chip::DeviceLayer::StackLock lock;

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    return static_cast<jlong>(wrapper->Controller()->GetNodeId());
}

JNI_METHOD(void, discoverCommissionableNodes)(JNIEnv * env, jobject self, jlong handle)
{
    chip::DeviceLayer::StackLock lock;

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);
    chip::Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kNone, (uint64_t) 0);

    CHIP_ERROR err = wrapper->Controller()->DiscoverCommissionableNodes(filter);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to discoverCommissionableNodes");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(jobject, getDiscoveredDevice)(JNIEnv * env, jobject self, jlong handle, jint idx)
{
    chip::DeviceLayer::StackLock lock;

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);
    const Dnssd::DiscoveredNodeData * data   = wrapper->Controller()->GetDiscoveredDevice(idx);

    if (data == nullptr)
    {
        ChipLogError(Controller, "GetDiscoveredDevice - not found");
        return nullptr;
    }

    jclass discoveredDeviceCls = env->FindClass("chip/devicecontroller/DiscoveredDevice");
    jmethodID constructor      = env->GetMethodID(discoveredDeviceCls, "<init>", "()V");

    jfieldID discrminatorID       = env->GetFieldID(discoveredDeviceCls, "discriminator", "J");
    jfieldID ipAddressID          = env->GetFieldID(discoveredDeviceCls, "ipAddress", "Ljava/lang/String;");
    jfieldID portID               = env->GetFieldID(discoveredDeviceCls, "port", "I");
    jfieldID deviceTypeID         = env->GetFieldID(discoveredDeviceCls, "deviceType", "J");
    jfieldID vendorIdID           = env->GetFieldID(discoveredDeviceCls, "vendorId", "I");
    jfieldID productIdID          = env->GetFieldID(discoveredDeviceCls, "productId", "I");
    jfieldID rotatingIdID         = env->GetFieldID(discoveredDeviceCls, "rotatingId", "[B");
    jfieldID instanceNameID       = env->GetFieldID(discoveredDeviceCls, "instanceName", "Ljava/lang/String;");
    jfieldID deviceNameID         = env->GetFieldID(discoveredDeviceCls, "deviceName", "Ljava/lang/String;");
    jfieldID pairingInstructionID = env->GetFieldID(discoveredDeviceCls, "pairingInstruction", "Ljava/lang/String;");

    jmethodID setCommissioningModeID = env->GetMethodID(discoveredDeviceCls, "setCommissioningMode", "(I)V");
    jmethodID setPairingHintID       = env->GetMethodID(discoveredDeviceCls, "setPairingHint", "(I)V");

    jobject discoveredObj = env->NewObject(discoveredDeviceCls, constructor);

    env->SetLongField(discoveredObj, discrminatorID, data->commissionData.longDiscriminator);

    char ipAddress[100];
    data->resolutionData.ipAddress[0].ToString(ipAddress, 100);
    jstring jniipAdress = env->NewStringUTF(ipAddress);

    env->SetObjectField(discoveredObj, ipAddressID, jniipAdress);
    env->SetIntField(discoveredObj, portID, static_cast<jint>(data->resolutionData.port));
    env->SetLongField(discoveredObj, deviceTypeID, static_cast<jlong>(data->commissionData.deviceType));
    env->SetIntField(discoveredObj, vendorIdID, static_cast<jint>(data->commissionData.vendorId));
    env->SetIntField(discoveredObj, productIdID, static_cast<jint>(data->commissionData.productId));

    jbyteArray jRotatingId;
    CHIP_ERROR err = JniReferences::GetInstance().N2J_ByteArray(
        env, data->commissionData.rotatingId, static_cast<jsize>(data->commissionData.rotatingIdLen), jRotatingId);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "jRotatingId N2J_ByteArray error : %" CHIP_ERROR_FORMAT, err.Format());
        return nullptr;
    }
    env->SetObjectField(discoveredObj, rotatingIdID, static_cast<jobject>(jRotatingId));
    env->SetObjectField(discoveredObj, instanceNameID, env->NewStringUTF(data->commissionData.instanceName));
    env->SetObjectField(discoveredObj, deviceNameID, env->NewStringUTF(data->commissionData.deviceName));
    env->SetObjectField(discoveredObj, pairingInstructionID, env->NewStringUTF(data->commissionData.pairingInstruction));

    env->CallVoidMethod(discoveredObj, setCommissioningModeID, static_cast<jint>(data->commissionData.commissioningMode));
    env->CallVoidMethod(discoveredObj, setPairingHintID, static_cast<jint>(data->commissionData.pairingHint));

    return discoveredObj;
}

JNI_METHOD(jboolean, openPairingWindow)(JNIEnv * env, jobject self, jlong handle, jlong devicePtr, jint duration)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceProxy * chipDevice = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (chipDevice == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return false;
    }

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    err = AutoCommissioningWindowOpener::OpenBasicCommissioningWindow(wrapper->Controller(), chipDevice->GetDeviceId(),
                                                                      System::Clock::Seconds16(duration));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "OpenPairingWindow failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, openPairingWindowWithPIN)
(JNIEnv * env, jobject self, jlong handle, jlong devicePtr, jint duration, jlong iteration, jint discriminator,
 jobject setupPinCode)
{
    VerifyOrReturnValue(chip::CanCastTo<uint32_t>(iteration), false);

    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceProxy * chipDevice = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (chipDevice == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return false;
    }

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    Optional<uint32_t> pinCode = Optional<uint32_t>();
    if (setupPinCode != nullptr)
    {
        jlong jsetupPinCode = chip::JniReferences::GetInstance().LongToPrimitive(setupPinCode);
        pinCode             = MakeOptional(static_cast<uint32_t>(jsetupPinCode));
    }

    chip::SetupPayload setupPayload;
    err = AutoCommissioningWindowOpener::OpenCommissioningWindow(
        wrapper->Controller(), chipDevice->GetDeviceId(), System::Clock::Seconds16(duration), static_cast<uint32_t>(iteration),
        static_cast<uint16_t>(discriminator), pinCode, NullOptional, setupPayload);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "OpenPairingWindow failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, openPairingWindowCallback)
(JNIEnv * env, jobject self, jlong handle, jlong devicePtr, jint duration, jobject jcallback)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceProxy * chipDevice = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (chipDevice == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return false;
    }

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    err = AndroidCommissioningWindowOpener::OpenBasicCommissioningWindow(wrapper->Controller(), chipDevice->GetDeviceId(),
                                                                         System::Clock::Seconds16(duration), jcallback);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "OpenPairingWindow failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, openPairingWindowWithPINCallback)
(JNIEnv * env, jobject self, jlong handle, jlong devicePtr, jint duration, jlong iteration, jint discriminator,
 jobject setupPinCode, jobject jcallback)
{
    VerifyOrReturnValue(chip::CanCastTo<uint32_t>(iteration), false);

    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceProxy * chipDevice = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (chipDevice == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return false;
    }

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    Optional<uint32_t> pinCode = Optional<uint32_t>();
    if (setupPinCode != nullptr)
    {
        jlong jsetupPinCode = chip::JniReferences::GetInstance().LongToPrimitive(setupPinCode);
        pinCode             = MakeOptional(static_cast<uint32_t>(jsetupPinCode));
    }

    chip::SetupPayload setupPayload;
    err = AndroidCommissioningWindowOpener::OpenCommissioningWindow(
        wrapper->Controller(), chipDevice->GetDeviceId(), System::Clock::Seconds16(duration), static_cast<uint32_t>(iteration),
        static_cast<uint16_t>(discriminator), pinCode, NullOptional, jcallback, setupPayload);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "OpenPairingWindow failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    return true;
}

JNI_METHOD(void, shutdownCommissioning)
(JNIEnv * env, jobject self, jlong handle)
{
    chip::DeviceLayer::StackLock lock;

    // Stop the IO thread, so that the controller can be safely shut down.
    StopIOThread();

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);
    wrapper->Shutdown();
}

JNI_METHOD(jbyteArray, getAttestationChallenge)
(JNIEnv * env, jobject self, jlong handle, jlong devicePtr)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    ByteSpan attestationChallenge;
    jbyteArray attestationChallengeJbytes = nullptr;

    DeviceProxy * chipDevice = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (chipDevice == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, CHIP_ERROR_INCORRECT_STATE);
        return nullptr;
    }

    err = chipDevice->GetAttestationChallenge(attestationChallenge);
    SuccessOrExit(err);
    VerifyOrExit(attestationChallenge.size() == 16, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = JniReferences::GetInstance().N2J_ByteArray(env, attestationChallenge.data(),
                                                     static_cast<jsize>(attestationChallenge.size()), attestationChallengeJbytes);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
    return attestationChallengeJbytes;
}

JNI_METHOD(void, deleteDeviceController)(JNIEnv * env, jobject self, jlong handle)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "deleteDeviceController() called");

    if (wrapper != nullptr)
    {
        delete wrapper;
    }
}

JNI_METHOD(jobject, computePaseVerifier)
(JNIEnv * env, jobject self, jlong handle, jlong devicePtr, jlong setupPincode, jlong iterations, jbyteArray salt)
{
    chip::DeviceLayer::StackLock lock;

    CHIP_ERROR err = CHIP_NO_ERROR;
    jobject params;
    jbyteArray verifierBytes;
    Spake2pVerifier verifier;
    Spake2pVerifierSerialized serializedVerifier;
    MutableByteSpan serializedVerifierSpan(serializedVerifier);
    JniByteArray jniSalt(env, salt);

    ChipLogProgress(Controller, "computePaseVerifier() called");

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    VerifyOrExit(chip::CanCastTo<uint32_t>(iterations), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(chip::CanCastTo<uint32_t>(setupPincode), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = wrapper->Controller()->ComputePASEVerifier(static_cast<uint32_t>(iterations), static_cast<uint32_t>(setupPincode),
                                                     jniSalt.byteSpan(), verifier);
    SuccessOrExit(err);

    err = verifier.Serialize(serializedVerifierSpan);
    SuccessOrExit(err);

    err = JniReferences::GetInstance().N2J_ByteArray(env, serializedVerifier, kSpake2p_VerifierSerialized_Length, verifierBytes);
    SuccessOrExit(err);

    err = N2J_PaseVerifierParams(env, setupPincode, verifierBytes, params);
    SuccessOrExit(err);
    return params;
exit:
    if (err != CHIP_NO_ERROR)
    {
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
    return nullptr;
}

JNI_METHOD(jbyteArray, validateAndExtractCSR)(JNIEnv * env, jclass clazz, jbyteArray jCsrElements, jbyteArray jCsrNonce)
{

    chip::JniByteArray csrElements(env, jCsrElements);
    chip::JniByteArray csrNonce(env, jCsrNonce);

    chip::ByteSpan csrSpan;
    chip::ByteSpan csrNonceSpan;
    chip::ByteSpan vendor_reserved1, vendor_reserved2, vendor_reserved3;
    CHIP_ERROR err = chip::Credentials::DeconstructNOCSRElements(csrElements.byteSpan(), csrSpan, csrNonceSpan, vendor_reserved1,
                                                                 vendor_reserved2, vendor_reserved3);

    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                        ChipLogError(Controller, "CsrElement decoding error: %" CHIP_ERROR_FORMAT, err.Format()));
    VerifyOrReturnValue(csrNonceSpan.size() == Controller::kCSRNonceLength, nullptr,
                        ChipLogError(Controller, "csrNonce size is invalid"));

    // Verify that Nonce matches with what we sent
    VerifyOrReturnValue(csrNonceSpan.data_equal(csrNonce.byteSpan()), nullptr,
                        ChipLogError(Controller, "csrNonce is not matched!"));

    jbyteArray javaCsr;
    chip::JniReferences::GetInstance().N2J_ByteArray(chip::JniReferences::GetInstance().GetEnvForCurrentThread(), csrSpan.data(),
                                                     static_cast<jsize>(csrSpan.size()), javaCsr);
    return javaCsr;
}

JNI_METHOD(jobject, getICDClientInfo)(JNIEnv * env, jobject self, jlong handle, jint jFabricIndex)
{
    chip::DeviceLayer::StackLock lock;

    CHIP_ERROR err = CHIP_NO_ERROR;

    jobject jInfo = nullptr;
    chip::app::ICDClientInfo info;
    chip::FabricIndex fabricIndex = static_cast<chip::FabricIndex>(jFabricIndex);

    ChipLogProgress(Controller, "getICDClientInfo(%u) called", fabricIndex);
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);
    VerifyOrReturnValue(wrapper != nullptr, nullptr, ChipLogError(Controller, "wrapper is null"));

    err = JniReferences::GetInstance().CreateArrayList(jInfo);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                        ChipLogError(Controller, "CreateArrayList failed!: %" CHIP_ERROR_FORMAT, err.Format()));

    auto iter = wrapper->getICDClientStorage()->IterateICDClientInfo();
    VerifyOrReturnValue(iter != nullptr, nullptr, ChipLogError(Controller, "IterateICDClientInfo failed!"));
    app::DefaultICDClientStorage::ICDClientInfoIteratorWrapper clientInfoIteratorWrapper(iter);

    jmethodID constructor;
    jclass infoClass;
    JniLocalReferenceScope scope(env);

    err = JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/ICDClientInfo", infoClass);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                        ChipLogError(Controller, "Find ICDClientInfo class: %" CHIP_ERROR_FORMAT, err.Format()));

    env->ExceptionClear();
    constructor = env->GetMethodID(infoClass, "<init>", "(JJJJ[B[B)V");
    VerifyOrReturnValue(constructor != nullptr, nullptr, ChipLogError(Controller, "Find GetMethodID error!"));

    while (iter->Next(info))
    {
        jbyteArray jIcdAesKey  = nullptr;
        jbyteArray jIcdHmacKey = nullptr;
        jobject jICDClientInfo = nullptr;

        if (info.peer_node.GetFabricIndex() != fabricIndex)
        {
            continue;
        }

        err = chip::JniReferences::GetInstance().N2J_ByteArray(env, info.aes_key_handle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                                                               chip::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, jIcdAesKey);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "ICD AES KEY N2J_ByteArray error!: %" CHIP_ERROR_FORMAT, err.Format()));

        err = chip::JniReferences::GetInstance().N2J_ByteArray(env, info.hmac_key_handle.As<Crypto::Symmetric128BitsKeyByteArray>(),
                                                               chip::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, jIcdHmacKey);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "ICD HMAC KEY N2J_ByteArray error!: %" CHIP_ERROR_FORMAT, err.Format()));

        jICDClientInfo = (jobject) env->NewObject(infoClass, constructor, static_cast<jlong>(info.peer_node.GetNodeId()),
                                                  static_cast<jlong>(info.start_icd_counter), static_cast<jlong>(info.offset),
                                                  static_cast<jlong>(info.monitored_subject), jIcdAesKey, jIcdHmacKey);

        err = JniReferences::GetInstance().AddToList(jInfo, jICDClientInfo);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "AddToList error!: %" CHIP_ERROR_FORMAT, err.Format()));
    }

    return jInfo;
}

JNI_METHOD(void, subscribe)
(JNIEnv * env, jclass clz, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList, jobject eventPathList,
 jobject dataVersionFilterList, jint minInterval, jint maxInterval, jboolean keepSubscriptions, jboolean isFabricFiltered,
 jint imTimeoutMs, jobject eventMin)
{
    CHIP_ERROR err = subscribe(env, handle, callbackHandle, devicePtr, attributePathList, eventPathList, dataVersionFilterList,
                               minInterval, maxInterval, keepSubscriptions, isFabricFiltered, imTimeoutMs, eventMin);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Subscribe Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

JNI_METHOD(void, read)
(JNIEnv * env, jclass clz, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList, jobject eventPathList,
 jobject dataVersionFilterList, jboolean isFabricFiltered, jint imTimeoutMs, jobject eventMin)
{
    CHIP_ERROR err = read(env, handle, callbackHandle, devicePtr, attributePathList, eventPathList, dataVersionFilterList,
                          isFabricFiltered, imTimeoutMs, eventMin);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Read Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

JNI_METHOD(void, write)
(JNIEnv * env, jclass clz, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributeList, jint timedRequestTimeoutMs,
 jint imTimeoutMs)
{
    CHIP_ERROR err = write(env, handle, callbackHandle, devicePtr, attributeList, timedRequestTimeoutMs, imTimeoutMs);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Write Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

JNI_METHOD(void, invoke)
(JNIEnv * env, jclass clz, jlong handle, jlong callbackHandle, jlong devicePtr, jobject invokeElement, jint timedRequestTimeoutMs,
 jint imTimeoutMs)
{
    CHIP_ERROR err = invoke(env, handle, callbackHandle, devicePtr, invokeElement, timedRequestTimeoutMs, imTimeoutMs);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Invoke Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

JNI_METHOD(void, extendableInvoke)
(JNIEnv * env, jclass clz, jlong handle, jlong callbackHandle, jlong devicePtr, jobject invokeElementList,
 jint timedRequestTimeoutMs, jint imTimeoutMs)
{
    CHIP_ERROR err =
        extendableInvoke(env, handle, callbackHandle, devicePtr, invokeElementList, timedRequestTimeoutMs, imTimeoutMs);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Batch Invoke Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void * IOThreadMain(void * arg)
{
    JNIEnv * env;
    JavaVMAttachArgs attachArgs;

    // Attach the IO thread to the JVM as a daemon thread.
    // This allows the JVM to shutdown without waiting for this thread to exit.
    attachArgs.version = JNI_VERSION_1_6;
    attachArgs.name    = (char *) "CHIP Device Controller IO Thread";
    attachArgs.group   = nullptr;
#ifdef __ANDROID__
    sJVM->AttachCurrentThreadAsDaemon(&env, (void *) &attachArgs);
#else
    sJVM->AttachCurrentThreadAsDaemon((void **) &env, (void *) &attachArgs);
#endif

    ChipLogProgress(Controller, "IO thread starting");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    ChipLogProgress(Controller, "IO thread ending");

    // Detach the thread from the JVM.
    sJVM->DetachCurrentThread();

    return nullptr;
}

// NOTE: This function SHALL be called with the stack lock held.
CHIP_ERROR StopIOThread()
{
    if (sIOThread != PTHREAD_NULL)
    {
        ChipLogProgress(Controller, "IO thread stopping");
        chip::DeviceLayer::StackUnlock unlock;

        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();

        pthread_join(sIOThread, nullptr);
        sIOThread = PTHREAD_NULL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR N2J_PaseVerifierParams(JNIEnv * env, jlong setupPincode, jbyteArray paseVerifier, jobject & outParams)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID constructor;
    jclass paramsClass;

    err = JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/PaseVerifierParams", paramsClass);
    SuccessOrExit(err);

    env->ExceptionClear();
    constructor = env->GetMethodID(paramsClass, "<init>", "(J[B)V");
    VerifyOrExit(constructor != nullptr, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    outParams = (jobject) env->NewObject(paramsClass, constructor, setupPincode, paseVerifier);

    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
exit:
    return err;
}

CHIP_ERROR N2J_NetworkLocation(JNIEnv * env, jstring ipAddress, jint port, jint interfaceIndex, jobject & outLocation)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID constructor;
    jclass locationClass;

    err = JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/NetworkLocation", locationClass);
    SuccessOrExit(err);

    env->ExceptionClear();
    constructor = env->GetMethodID(locationClass, "<init>", "(Ljava/lang/String;II)V");
    VerifyOrExit(constructor != nullptr, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    outLocation = (jobject) env->NewObject(locationClass, constructor, ipAddress, port, interfaceIndex);

    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
exit:
    return err;
}
