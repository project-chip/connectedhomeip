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
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#include <app/AttributePathParams.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadClient.h>
#include <app/WriteClient.h>
#include <app/util/error-mapping.h>
#include <atomic>
#include <ble/BleUUID.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningWindowOpener.h>
#include <controller/java/AndroidClusterExceptions.h>
#include <credentials/CHIPCert.h>
#include <jni.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/jsontlv/JsonToTlv.h>
#include <lib/support/jsontlv/TlvToJson.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/Protocols.h>
#include <pthread.h>
#include <system/SystemClock.h>
#include <vector>

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
static CHIP_ERROR GetChipPathIdValue(jobject chipPathId, uint32_t wildcardValue, uint32_t & outValue);
static CHIP_ERROR ParseAttributePathList(jobject attributePathList,
                                         std::vector<app::AttributePathParams> & outAttributePathParamsList);
CHIP_ERROR ParseAttributePath(jobject attributePath, EndpointId & outEndpointId, ClusterId & outClusterId,
                              AttributeId & outAttributeId);
static CHIP_ERROR ParseEventPathList(jobject eventPathList, std::vector<app::EventPathParams> & outEventPathParamsList);
CHIP_ERROR ParseEventPath(jobject eventPath, EndpointId & outEndpointId, ClusterId & outClusterId, EventId & outEventId,
                          bool & outIsUrgent);
static CHIP_ERROR IsWildcardChipPathId(jobject chipPathId, bool & isWildcard);

namespace {

JavaVM * sJVM;

pthread_t sIOThread = PTHREAD_NULL;

jclass sChipDeviceControllerExceptionCls = NULL;

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
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    ChipLogProgress(Controller, "Loading Java class references.");

    // Get various class references need by the API.
    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/ChipDeviceControllerException",
                                                   sChipDeviceControllerExceptionCls);
    SuccessOrExit(err);
    ChipLogProgress(Controller, "Java class references loaded.");

#ifndef JAVA_MATTER_CONTROLLER_TEST
    err = AndroidChipPlatformJNI_OnLoad(jvm, reserved);
    SuccessOrExit(err);
#endif // JAVA_MATTER_CONTROLLER_TEST

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

    sJVM = NULL;

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
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getRootCertificate", "()[B", &getRootCertificate);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, err.AsInteger());

    jmethodID getIntermediateCertificate;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getIntermediateCertificate", "()[B",
                                                        &getIntermediateCertificate);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, err.AsInteger());

    jmethodID getOperationalCertificate;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getOperationalCertificate", "()[B",
                                                        &getOperationalCertificate);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, err.AsInteger());

    jmethodID getIpk;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getIpk", "()[B", &getIpk);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, err.AsInteger());

    jmethodID getAdminSubject;
    err = chip::JniReferences::GetInstance().FindMethod(env, controllerParams, "getAdminSubject", "()J", &getAdminSubject);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, err.AsInteger());

    jbyteArray rootCertificate = (jbyteArray) env->CallObjectMethod(controllerParams, getRootCertificate);
    VerifyOrReturnValue(rootCertificate != nullptr, CHIP_ERROR_BAD_REQUEST.AsInteger());

    jbyteArray intermediateCertificate = (jbyteArray) env->CallObjectMethod(controllerParams, getIntermediateCertificate);
    VerifyOrReturnValue(intermediateCertificate != nullptr, CHIP_ERROR_BAD_REQUEST.AsInteger());

    jbyteArray operationalCertificate = (jbyteArray) env->CallObjectMethod(controllerParams, getOperationalCertificate);
    VerifyOrReturnValue(operationalCertificate != nullptr, CHIP_ERROR_BAD_REQUEST.AsInteger());

    // use ipk and adminSubject from CommissioningParameters if not set in ControllerParams
    CommissioningParameters commissioningParams = wrapper->GetCommissioningParameters();

    Optional<Crypto::IdentityProtectionKeySpan> ipkOptional;
    uint8_t ipkValue[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
    Crypto::IdentityProtectionKeySpan ipkTempSpan(ipkValue);

    jbyteArray ipk = (jbyteArray) env->CallObjectMethod(controllerParams, getIpk);
    if (ipk != nullptr)
    {
        JniByteArray jByteArrayIpk(env, ipk);

        VerifyOrReturnValue(jByteArrayIpk.byteSpan().size() == sizeof(ipkValue), CHIP_ERROR_INTERNAL.AsInteger());
        memcpy(&ipkValue[0], jByteArrayIpk.byteSpan().data(), jByteArrayIpk.byteSpan().size());

        ipkOptional.SetValue(ipkTempSpan);
    }
    else if (commissioningParams.GetIpk().HasValue())
    {
        // if no value pass in ControllerParams, use value from CommissioningParameters
        ipkOptional.SetValue(commissioningParams.GetIpk().Value());
    }

    Optional<NodeId> adminSubjectOptional;
    uint64_t adminSubject = env->CallLongMethod(controllerParams, getAdminSubject);
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

    JniByteArray jByteArrayRcac(env, rootCertificate);
    JniByteArray jByteArrayIcac(env, intermediateCertificate);
    JniByteArray jByteArrayNoc(env, operationalCertificate);

#ifndef JAVA_MATTER_CONTROLLER_TEST
    err = wrapper->GetAndroidOperationalCredentialsIssuer()->NOCChainGenerated(CHIP_NO_ERROR, jByteArrayNoc.byteSpan(),
                                                                               jByteArrayIcac.byteSpan(), jByteArrayRcac.byteSpan(),
                                                                               ipkOptional, adminSubjectOptional);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to SetNocChain for the device: %" CHIP_ERROR_FORMAT, err.Format());
    }
#endif // JAVA_MATTER_CONTROLLER_TEST
    return err.AsInteger();
}

JNI_METHOD(jlong, newDeviceController)(JNIEnv * env, jobject self, jobject controllerParams)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = NULL;
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
        bool attemptNetworkScanWiFi        = env->CallBooleanMethod(controllerParams, getAttemptNetworkScanWiFi);
        bool attemptNetworkScanThread      = env->CallBooleanMethod(controllerParams, getAttemptNetworkScanThread);
        bool skipCommissioningComplete     = env->CallBooleanMethod(controllerParams, getSkipCommissioningComplete);
        uint64_t adminSubject              = static_cast<uint64_t>(env->CallLongMethod(controllerParams, getAdminSubject));
        jobject countryCodeOptional        = env->CallObjectMethod(controllerParams, getCountryCode);
        jobject regulatoryLocationOptional = env->CallObjectMethod(controllerParams, getRegulatoryLocation);

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
            failsafeTimerSeconds, attemptNetworkScanWiFi, attemptNetworkScanThread, skipCommissioningComplete, &err);
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

        jobject countryCode;
        err = chip::JniReferences::GetInstance().GetOptionalValue(countryCodeOptional, countryCode);
        SuccessOrExit(err);

        if (countryCode != nullptr)
        {
            jstring countryCodeStr = static_cast<jstring>(countryCode);
            JniUtfString countryCodeJniString(env, countryCodeStr);

            VerifyOrExit(countryCodeJniString.size() == 2, err = CHIP_ERROR_INVALID_ARGUMENT);

            chip::Controller::CommissioningParameters commissioningParams = wrapper->GetCommissioningParameters();
            commissioningParams.SetCountryCode(countryCodeJniString.charSpan());

            // The wrapper internally has reserved storage for the country code and will copy the value.
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
        int pthreadErr = pthread_create(&sIOThread, NULL, IOThreadMain, NULL);
        VerifyOrExit(pthreadErr == 0, err = CHIP_ERROR_POSIX(pthreadErr));
    }

    result = wrapper->ToJNIHandle();

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (wrapper != NULL)
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
        jobject deviceAttestationDelegateRef  = env->NewGlobalRef(deviceAttestationDelegate);

        VerifyOrExit(deviceAttestationDelegateRef != nullptr, err = CHIP_JNI_ERROR_NULL_OBJECT);
        JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/DeviceAttestationDelegate",
                                                 deviceAttestationDelegateCls);
        VerifyOrExit(deviceAttestationDelegateCls != nullptr, err = CHIP_JNI_ERROR_TYPE_NOT_FOUND);

        if (env->IsInstanceOf(deviceAttestationDelegate, deviceAttestationDelegateCls))
        {
            shouldWaitAfterDeviceAttestation = true;
        }

        err = wrapper->UpdateDeviceAttestationDelegateBridge(deviceAttestationDelegateRef, timeoutSecs,
                                                             shouldWaitAfterDeviceAttestation);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to set device attestation delegate.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, setAttestationTrustStoreDelegate)
(JNIEnv * env, jobject self, jlong handle, jobject attestationTrustStoreDelegate)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "setAttestationTrustStoreDelegate() called");

    if (attestationTrustStoreDelegate != nullptr)
    {
        jobject attestationTrustStoreDelegateRef = env->NewGlobalRef(attestationTrustStoreDelegate);
        err                                      = wrapper->UpdateAttestationTrustStoreBridge(attestationTrustStoreDelegateRef);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to set device attestation delegate.");
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
    err = wrapper->Controller()->Commission(deviceId, commissioningParams);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to commission the device.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, pairDevice)
(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jint connObj, jlong pinCode, jbyteArray csrNonce,
 jobject networkCredentials)
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
    if (wrapper->GetDeviceAttestationDelegateBridge() != nullptr)
    {
        commissioningParams.SetDeviceAttestationDelegate(wrapper->GetDeviceAttestationDelegateBridge());
    }
    err = wrapper->Controller()->PairDevice(deviceId, rendezvousParams, commissioningParams);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to pair the device.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, pairDeviceWithAddress)
(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jstring address, jint port, jint discriminator, jlong pinCode,
 jbyteArray csrNonce)
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
    if (wrapper->GetDeviceAttestationDelegateBridge() != nullptr)
    {
        commissioningParams.SetDeviceAttestationDelegate(wrapper->GetDeviceAttestationDelegateBridge());
    }
    err = wrapper->Controller()->PairDevice(deviceId, rendezvousParams, commissioningParams);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to pair the device.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, pairDeviceWithCode)
(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jstring setUpCode, jboolean discoverOnce,
 jboolean useOnlyOnNetworkDiscovery, jbyteArray csrNonce, jobject networkCredentials)
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

    if (wrapper->GetDeviceAttestationDelegateBridge() != nullptr)
    {
        commissioningParams.SetDeviceAttestationDelegate(wrapper->GetDeviceAttestationDelegateBridge());
    }
    err = wrapper->Controller()->PairDevice(deviceId, setUpCodeJniString.c_str(), commissioningParams, discoveryType);

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

    err = wrapper->Controller()->EstablishPASEConnection(deviceId, rendezvousParams);

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

    err = wrapper->Controller()->EstablishPASEConnection(deviceId, rendezvousParams);

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

    CommissioningParameters commissioningParams = wrapper->GetCommissioningParameters();
    CHIP_ERROR err                              = wrapper->ApplyNetworkCredentials(commissioningParams, networkCredentials);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "ApplyNetworkCredentials failed. Err = %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
        return;
    }
    err = wrapper->UpdateCommissioningParameters(commissioningParams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "UpdateCommissioningParameters failed. Err = %" CHIP_ERROR_FORMAT, err.Format());
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
        fabric = MakeOptional(static_cast<FabricId>(jfabricId));
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
        fabric = MakeOptional(static_cast<FabricId>(jfabricId));
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

    err = JniReferences::GetInstance().N2J_ByteArray(env, publicKey.Bytes(), static_cast<uint32_t>(publicKey.Length()), outJbytes);
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

        err = JniReferences::GetInstance().N2J_ByteArray(env, outBytes.data(), static_cast<uint32_t>(outBytes.size()), outJbytes);
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

JNI_METHOD(jbyteArray, extractSkidFromPaaCert)
(JNIEnv * env, jobject self, jbyteArray paaCert)
{
    uint32_t allocatedCertLength = chip::Credentials::kMaxCHIPCertLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> outBuf;
    jbyteArray outJbytes = nullptr;
    JniByteArray paaCertBytes(env, paaCert);

    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(outBuf.Alloc(allocatedCertLength), err = CHIP_ERROR_NO_MEMORY);
    {
        MutableByteSpan outBytes(outBuf.Get(), allocatedCertLength);

        err = chip::Crypto::ExtractSKIDFromX509Cert(paaCertBytes.byteSpan(), outBytes);
        SuccessOrExit(err);

        VerifyOrExit(chip::CanCastTo<uint32_t>(outBytes.size()), err = CHIP_ERROR_INTERNAL);

        err = JniReferences::GetInstance().N2J_ByteArray(env, outBytes.data(), static_cast<uint32_t>(outBytes.size()), outJbytes);
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to extract skid frome X509 cert. Err = %" CHIP_ERROR_FORMAT, err.Format());
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

    err = wrapper->Controller()->UnpairDevice(deviceId);

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

    err = wrapper->Controller()->StopPairing(deviceId);

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
    VerifyOrReturn(connectedDeviceCallback != nullptr, ChipLogError(Controller, "GetConnectedDeviceCallback handle is nullptr"));
    err = wrapper->Controller()->GetConnectedDevice(nodeId, &connectedDeviceCallback->mOnSuccess,
                                                    &connectedDeviceCallback->mOnFailure);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error invoking GetConnectedDevice"));
}

JNI_METHOD(void, releaseOperationalDevicePointer)(JNIEnv * env, jobject self, jlong devicePtr)
{
    chip::DeviceLayer::StackLock lock;
    OperationalDeviceProxy * device = reinterpret_cast<OperationalDeviceProxy *>(devicePtr);
    if (device != NULL)
    {
        delete device;
    }
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

    CHIP_ERROR err = wrapper->Controller()->GetPeerAddressAndPort(deviceId, addr, port);

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

    return wrapper->Controller()->GetCompressedFabricId();
}

JNI_METHOD(jlong, getControllerNodeId)(JNIEnv * env, jobject self, jlong handle)
{
    chip::DeviceLayer::StackLock lock;

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    return wrapper->Controller()->GetNodeId();
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
        return nullptr;
    }

    jclass discoveredDeviceCls = env->FindClass("chip/devicecontroller/DiscoveredDevice");
    jmethodID constructor      = env->GetMethodID(discoveredDeviceCls, "<init>", "()V");

    jfieldID discrminatorID = env->GetFieldID(discoveredDeviceCls, "discriminator", "J");
    jfieldID ipAddressID    = env->GetFieldID(discoveredDeviceCls, "ipAddress", "Ljava/lang/String;");

    jobject discoveredObj = env->NewObject(discoveredDeviceCls, constructor);

    env->SetLongField(discoveredObj, discrminatorID, data->commissionData.longDiscriminator);

    char ipAddress[100];
    data->resolutionData.ipAddress[0].ToString(ipAddress, 100);
    jstring jniipAdress = env->NewStringUTF(ipAddress);
    env->SetObjectField(discoveredObj, ipAddressID, jniipAdress);

    if (data == nullptr)
    {
        ChipLogError(Controller, "GetDiscoveredDevice - not found");
    }
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
    wrapper->Controller()->Shutdown();
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

    err = JniReferences::GetInstance().N2J_ByteArray(
        env, attestationChallenge.data(), static_cast<uint32_t>(attestationChallenge.size()), attestationChallengeJbytes);
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

    if (wrapper != NULL)
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

JNI_METHOD(void, subscribe)
(JNIEnv * env, jobject self, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList, jobject eventPathList,
 jint minInterval, jint maxInterval, jboolean keepSubscriptions, jboolean isFabricFiltered, jint imTimeoutMs, jobject eventMin)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err               = CHIP_NO_ERROR;
    app::ReadClient * readClient = nullptr;
    jint numAttributePaths       = 0;
    jint numEventPaths           = 0;
    auto callback                = reinterpret_cast<ReportCallback *>(callbackHandle);
    DeviceProxy * device         = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (device == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, CHIP_ERROR_INCORRECT_STATE);
        return;
    }
    app::ReadPrepareParams params(device->GetSecureSession().Value());

    uint16_t aImTimeoutMs             = static_cast<uint16_t>(imTimeoutMs);
    params.mMinIntervalFloorSeconds   = static_cast<uint16_t>(minInterval);
    params.mMaxIntervalCeilingSeconds = static_cast<uint16_t>(maxInterval);
    params.mKeepSubscriptions         = (keepSubscriptions != JNI_FALSE);
    params.mIsFabricFiltered          = (isFabricFiltered != JNI_FALSE);
    params.mTimeout                   = aImTimeoutMs != 0 ? System::Clock::Milliseconds32(aImTimeoutMs) : System::Clock::kZero;

    if (attributePathList != nullptr)
    {
        SuccessOrExit(err = JniReferences::GetInstance().GetListSize(attributePathList, numAttributePaths));
    }

    if (numAttributePaths > 0)
    {
        std::unique_ptr<chip::app::AttributePathParams[]> attributePaths(new chip::app::AttributePathParams[numAttributePaths]);
        for (uint8_t i = 0; i < numAttributePaths; i++)
        {
            jobject attributePathItem = nullptr;
            SuccessOrExit(err = JniReferences::GetInstance().GetListItem(attributePathList, i, attributePathItem));

            EndpointId endpointId;
            ClusterId clusterId;
            AttributeId attributeId;
            SuccessOrExit(err = ParseAttributePath(attributePathItem, endpointId, clusterId, attributeId));
            attributePaths[i] = chip::app::AttributePathParams(endpointId, clusterId, attributeId);
        }
        params.mpAttributePathParamsList    = attributePaths.get();
        params.mAttributePathParamsListSize = numAttributePaths;
        attributePaths.release();
    }

    if (eventMin != nullptr)
    {
        params.mEventNumber.SetValue(static_cast<chip::EventNumber>(JniReferences::GetInstance().LongToPrimitive(eventMin)));
    }

    if (eventPathList != nullptr)
    {
        SuccessOrExit(err = JniReferences::GetInstance().GetListSize(eventPathList, numEventPaths));
    }

    if (numEventPaths > 0)
    {
        std::unique_ptr<chip::app::EventPathParams[]> eventPaths(new chip::app::EventPathParams[numEventPaths]);
        for (uint8_t i = 0; i < numEventPaths; i++)
        {
            jobject eventPathItem = nullptr;
            SuccessOrExit(err = JniReferences::GetInstance().GetListItem(eventPathList, i, eventPathItem));

            EndpointId endpointId;
            ClusterId clusterId;
            EventId eventId;
            bool isUrgent;
            SuccessOrExit(err = ParseEventPath(eventPathItem, endpointId, clusterId, eventId, isUrgent));
            eventPaths[i] = chip::app::EventPathParams(endpointId, clusterId, eventId, isUrgent);
        }

        params.mpEventPathParamsList    = eventPaths.get();
        params.mEventPathParamsListSize = numEventPaths;
        eventPaths.release();
    }

    readClient = Platform::New<app::ReadClient>(app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                callback->mClusterCacheAdapter.GetBufferedCallback(),
                                                app::ReadClient::InteractionType::Subscribe);

    SuccessOrExit(err = readClient->SendAutoResubscribeRequest(std::move(params)));
    callback->mReadClient = readClient;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Subscribe Error: %s", err.AsString());
        if (err == CHIP_JNI_ERROR_EXCEPTION_THROWN)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        callback->OnError(err);
        if (readClient != nullptr)
        {
            Platform::Delete(readClient);
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
        }
    }
}

JNI_METHOD(void, read)
(JNIEnv * env, jobject self, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList, jobject eventPathList,
 jboolean isFabricFiltered, jint imTimeoutMs, jobject eventMin)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    auto callback = reinterpret_cast<ReportCallback *>(callbackHandle);
    std::vector<app::AttributePathParams> attributePathParamsList;
    std::vector<app::EventPathParams> eventPathParamsList;
    app::ReadClient * readClient = nullptr;
    DeviceProxy * device         = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (device == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, CHIP_ERROR_INCORRECT_STATE);
        return;
    }
    app::ReadPrepareParams params(device->GetSecureSession().Value());

    SuccessOrExit(err = ParseAttributePathList(attributePathList, attributePathParamsList));
    SuccessOrExit(err = ParseEventPathList(eventPathList, eventPathParamsList));
    VerifyOrExit(attributePathParamsList.size() != 0 || eventPathParamsList.size() != 0, err = CHIP_ERROR_INVALID_ARGUMENT);
    params.mpAttributePathParamsList    = attributePathParamsList.data();
    params.mAttributePathParamsListSize = attributePathParamsList.size();
    params.mpEventPathParamsList        = eventPathParamsList.data();
    params.mEventPathParamsListSize     = eventPathParamsList.size();

    params.mIsFabricFiltered = (isFabricFiltered != JNI_FALSE);
    params.mTimeout          = imTimeoutMs != 0 ? System::Clock::Milliseconds32(imTimeoutMs) : System::Clock::kZero;

    if (eventMin != nullptr)
    {
        params.mEventNumber.SetValue(static_cast<chip::EventNumber>(JniReferences::GetInstance().LongToPrimitive(eventMin)));
    }

    readClient = Platform::New<app::ReadClient>(app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                callback->mClusterCacheAdapter.GetBufferedCallback(),
                                                app::ReadClient::InteractionType::Read);

    SuccessOrExit(err = readClient->SendRequest(params));
    callback->mReadClient = readClient;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Read Error: %s", err.AsString());
        if (err == CHIP_JNI_ERROR_EXCEPTION_THROWN)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        callback->OnError(err);
        if (readClient != nullptr)
        {
            Platform::Delete(readClient);
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
        }
    }
}

// Convert Json to Tlv, and remove the outer structure
CHIP_ERROR ConvertJsonToTlvWithoutStruct(const std::string & json, MutableByteSpan & data)
{
    Platform::ScopedMemoryBufferWithSize<uint8_t> buf;
    VerifyOrReturnError(buf.Calloc(data.size()), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan dataWithStruct(buf.Get(), buf.AllocatedSize());
    ReturnErrorOnFailure(JsonToTlv(json, dataWithStruct));
    TLV::TLVReader tlvReader;
    TLV::TLVType outerContainer = TLV::kTLVType_Structure;
    tlvReader.Init(dataWithStruct);
    ReturnErrorOnFailure(tlvReader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    ReturnErrorOnFailure(tlvReader.EnterContainer(outerContainer));
    ReturnErrorOnFailure(tlvReader.Next());

    TLV::TLVWriter tlvWrite;
    tlvWrite.Init(data);
    ReturnErrorOnFailure(tlvWrite.CopyElement(TLV::AnonymousTag(), tlvReader));
    ReturnErrorOnFailure(tlvWrite.Finalize());
    data.reduce_size(tlvWrite.GetLengthWritten());
    return CHIP_NO_ERROR;
}

CHIP_ERROR PutPreencodedWriteAttribute(app::WriteClient & writeClient, app::ConcreteDataAttributePath & path, const ByteSpan & data)
{
    TLV::TLVReader reader;
    reader.Init(data);
    ReturnErrorOnFailure(reader.Next());
    return writeClient.PutPreencodedAttribute(path, reader);
}

JNI_METHOD(void, write)
(JNIEnv * env, jobject self, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributeList, jint timedRequestTimeoutMs,
 jint imTimeoutMs)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    jint listSize                           = 0;
    auto callback                           = reinterpret_cast<WriteAttributesCallback *>(callbackHandle);
    app::WriteClient * writeClient          = nullptr;
    uint16_t convertedTimedRequestTimeoutMs = static_cast<uint16_t>(timedRequestTimeoutMs);

    ChipLogDetail(Controller, "IM write() called");

    DeviceProxy * device = reinterpret_cast<DeviceProxy *>(devicePtr);
    VerifyOrExit(device != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(device->GetSecureSession().HasValue(), err = CHIP_ERROR_MISSING_SECURE_SESSION);
    VerifyOrExit(attributeList != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    SuccessOrExit(err = JniReferences::GetInstance().GetListSize(attributeList, listSize));

    writeClient = Platform::New<app::WriteClient>(
        device->GetExchangeManager(), callback->GetChunkedWriteCallback(),
        convertedTimedRequestTimeoutMs != 0 ? Optional<uint16_t>(convertedTimedRequestTimeoutMs) : Optional<uint16_t>::Missing());

    for (uint8_t i = 0; i < listSize; i++)
    {
        jobject attributeItem             = nullptr;
        uint32_t endpointId               = 0;
        uint32_t clusterId                = 0;
        uint32_t attributeId              = 0;
        jmethodID getEndpointIdMethod     = nullptr;
        jmethodID getClusterIdMethod      = nullptr;
        jmethodID getAttributeIdMethod    = nullptr;
        jmethodID hasDataVersionMethod    = nullptr;
        jmethodID getDataVersionMethod    = nullptr;
        jmethodID getTlvByteArrayMethod   = nullptr;
        jmethodID getJsonStringMethod     = nullptr;
        jobject endpointIdObj             = nullptr;
        jobject clusterIdObj              = nullptr;
        jobject attributeIdObj            = nullptr;
        jbyteArray tlvBytesObj            = nullptr;
        bool hasDataVersion               = false;
        Optional<DataVersion> dataVersion = Optional<DataVersion>();

        SuccessOrExit(err = JniReferences::GetInstance().GetListItem(attributeList, i, attributeItem));
        SuccessOrExit(err = JniReferences::GetInstance().FindMethod(
                          env, attributeItem, "getEndpointId", "()Lchip/devicecontroller/model/ChipPathId;", &getEndpointIdMethod));
        SuccessOrExit(err = JniReferences::GetInstance().FindMethod(
                          env, attributeItem, "getClusterId", "()Lchip/devicecontroller/model/ChipPathId;", &getClusterIdMethod));
        SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, attributeItem, "getAttributeId",
                                                                    "()Lchip/devicecontroller/model/ChipPathId;",
                                                                    &getAttributeIdMethod));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, attributeItem, "hasDataVersion", "()Z", &hasDataVersionMethod));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, attributeItem, "getDataVersion", "()I", &getDataVersionMethod));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, attributeItem, "getTlvByteArray", "()[B", &getTlvByteArrayMethod));

        endpointIdObj = env->CallObjectMethod(attributeItem, getEndpointIdMethod);
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
        VerifyOrExit(endpointIdObj != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        clusterIdObj = env->CallObjectMethod(attributeItem, getClusterIdMethod);
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
        VerifyOrExit(clusterIdObj != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        attributeIdObj = env->CallObjectMethod(attributeItem, getAttributeIdMethod);
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
        VerifyOrExit(attributeIdObj != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        SuccessOrExit(err = GetChipPathIdValue(endpointIdObj, kInvalidEndpointId, endpointId));
        SuccessOrExit(err = GetChipPathIdValue(clusterIdObj, kInvalidClusterId, clusterId));
        SuccessOrExit(err = GetChipPathIdValue(attributeIdObj, kInvalidAttributeId, attributeId));

        hasDataVersion = static_cast<bool>(env->CallBooleanMethod(attributeItem, hasDataVersionMethod));
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
        if (hasDataVersion)
        {
            DataVersion dataVersionVal = static_cast<DataVersion>(env->CallIntMethod(attributeItem, getDataVersionMethod));
            VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
            dataVersion.SetValue(dataVersionVal);
        }

        tlvBytesObj = static_cast<jbyteArray>(env->CallObjectMethod(attributeItem, getTlvByteArrayMethod));
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
        app::ConcreteDataAttributePath path(static_cast<EndpointId>(endpointId), static_cast<ClusterId>(clusterId),
                                            static_cast<AttributeId>(attributeId), dataVersion);
        if (tlvBytesObj != nullptr)
        {
            JniByteArray tlvByteArray(env, tlvBytesObj);
            SuccessOrExit(err = PutPreencodedWriteAttribute(*writeClient, path, tlvByteArray.byteSpan()));
        }
        else
        {
            SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, attributeItem, "getJsonString", "()Ljava/lang/String;",
                                                                        &getJsonStringMethod));
            jstring jsonJniString = static_cast<jstring>(env->CallObjectMethod(attributeItem, getJsonStringMethod));
            VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
            VerifyOrExit(jsonJniString != nullptr, err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
            JniUtfString jsonUtfJniString(env, jsonJniString);
            std::string jsonString = std::string(jsonUtfJniString.c_str(), jsonUtfJniString.size());

            // Context: Chunk write is supported in sdk, oversized list could be chunked in multiple message. When transforming
            // JSON to TLV, we need know the actual size for tlv blob when handling JsonToTlv
            // TODO: Implement memory auto-grow to get the actual size needed for tlv blob when transforming tlv to json.
            // Workaround: Allocate memory using json string's size, which is large enough to hold the corresponding tlv blob
            Platform::ScopedMemoryBufferWithSize<uint8_t> tlvBytes;
            size_t length = jsonUtfJniString.size();
            VerifyOrExit(tlvBytes.Calloc(length), err = CHIP_ERROR_NO_MEMORY);
            MutableByteSpan data(tlvBytes.Get(), tlvBytes.AllocatedSize());
            SuccessOrExit(err = ConvertJsonToTlvWithoutStruct(jsonString, data));
            SuccessOrExit(err = PutPreencodedWriteAttribute(*writeClient, path, data));
        }
    }

    err = writeClient->SendWriteRequest(device->GetSecureSession().Value(),
                                        imTimeoutMs != 0 ? System::Clock::Milliseconds32(imTimeoutMs) : System::Clock::kZero);
    SuccessOrExit(err);
    callback->mWriteClient = writeClient;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Write Error: %s", err.AsString());
        if (err == CHIP_JNI_ERROR_EXCEPTION_THROWN)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        callback->OnError(writeClient, err);
        if (writeClient != nullptr)
        {
            Platform::Delete(writeClient);
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
        }
    }
}

CHIP_ERROR PutPreencodedInvokeRequest(app::CommandSender & commandSender, app::CommandPathParams & path, const ByteSpan & data)
{
    // PrepareCommand does nott create the struct container with kFields and copycontainer below sets the
    // kFields container already
    ReturnErrorOnFailure(commandSender.PrepareCommand(path, false /* aStartDataStruct */));
    TLV::TLVWriter * writer = commandSender.GetCommandDataIBTLVWriter();
    VerifyOrReturnError(writer != nullptr, CHIP_ERROR_INCORRECT_STATE);
    TLV::TLVReader reader;
    reader.Init(data);
    ReturnErrorOnFailure(reader.Next());
    return writer->CopyContainer(TLV::ContextTag(app::CommandDataIB::Tag::kFields), reader);
}

JNI_METHOD(void, invoke)
(JNIEnv * env, jobject self, jlong handle, jlong callbackHandle, jlong devicePtr, jobject invokeElement, jint timedRequestTimeoutMs,
 jint imTimeoutMs)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    auto callback                           = reinterpret_cast<InvokeCallback *>(callbackHandle);
    app::CommandSender * commandSender      = nullptr;
    uint32_t endpointId                     = 0;
    uint32_t clusterId                      = 0;
    uint32_t commandId                      = 0;
    jmethodID getEndpointIdMethod           = nullptr;
    jmethodID getClusterIdMethod            = nullptr;
    jmethodID getCommandIdMethod            = nullptr;
    jmethodID getTlvByteArrayMethod         = nullptr;
    jmethodID getJsonStringMethod           = nullptr;
    jobject endpointIdObj                   = nullptr;
    jobject clusterIdObj                    = nullptr;
    jobject commandIdObj                    = nullptr;
    jbyteArray tlvBytesObj                  = nullptr;
    uint16_t convertedTimedRequestTimeoutMs = static_cast<uint16_t>(timedRequestTimeoutMs);
    ChipLogDetail(Controller, "IM invoke() called");

    DeviceProxy * device = reinterpret_cast<DeviceProxy *>(devicePtr);
    VerifyOrExit(device != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(device->GetSecureSession().HasValue(), err = CHIP_ERROR_MISSING_SECURE_SESSION);

    commandSender = Platform::New<app::CommandSender>(callback, device->GetExchangeManager(), timedRequestTimeoutMs != 0);

    SuccessOrExit(err = JniReferences::GetInstance().FindMethod(
                      env, invokeElement, "getEndpointId", "()Lchip/devicecontroller/model/ChipPathId;", &getEndpointIdMethod));
    SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getClusterId",
                                                                "()Lchip/devicecontroller/model/ChipPathId;", &getClusterIdMethod));
    SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getCommandId",
                                                                "()Lchip/devicecontroller/model/ChipPathId;", &getCommandIdMethod));
    SuccessOrExit(
        err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getTlvByteArray", "()[B", &getTlvByteArrayMethod));

    endpointIdObj = env->CallObjectMethod(invokeElement, getEndpointIdMethod);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
    VerifyOrExit(endpointIdObj != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    clusterIdObj = env->CallObjectMethod(invokeElement, getClusterIdMethod);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
    VerifyOrExit(clusterIdObj != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    commandIdObj = env->CallObjectMethod(invokeElement, getCommandIdMethod);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
    VerifyOrExit(commandIdObj != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    SuccessOrExit(err = GetChipPathIdValue(endpointIdObj, kInvalidEndpointId, endpointId));
    SuccessOrExit(err = GetChipPathIdValue(clusterIdObj, kInvalidClusterId, clusterId));
    SuccessOrExit(err = GetChipPathIdValue(commandIdObj, kInvalidCommandId, commandId));

    tlvBytesObj = static_cast<jbyteArray>(env->CallObjectMethod(invokeElement, getTlvByteArrayMethod));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
    {
        app::CommandPathParams path(static_cast<EndpointId>(endpointId), /* group id */ 0, static_cast<ClusterId>(clusterId),
                                    static_cast<CommandId>(commandId), app::CommandPathFlags::kEndpointIdValid);
        if (tlvBytesObj != nullptr)
        {
            JniByteArray tlvBytesObjBytes(env, tlvBytesObj);
            SuccessOrExit(err = PutPreencodedInvokeRequest(*commandSender, path, tlvBytesObjBytes.byteSpan()));
        }
        else
        {
            SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getJsonString", "()Ljava/lang/String;",
                                                                        &getJsonStringMethod));
            jstring jsonJniString = static_cast<jstring>(env->CallObjectMethod(invokeElement, getJsonStringMethod));
            VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
            VerifyOrExit(jsonJniString != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
            JniUtfString jsonUtfJniString(env, jsonJniString);
            // The invoke does not support chunk, kMaxSecureSduLengthBytes should be enough for command json blob
            uint8_t tlvBytes[chip::app::kMaxSecureSduLengthBytes] = { 0 };
            MutableByteSpan tlvEncodingLocal{ tlvBytes };
            SuccessOrExit(err = JsonToTlv(std::string(jsonUtfJniString.c_str(), jsonUtfJniString.size()), tlvEncodingLocal));
            SuccessOrExit(err = PutPreencodedInvokeRequest(*commandSender, path, tlvEncodingLocal));
        }
    }
    SuccessOrExit(err = commandSender->FinishCommand(convertedTimedRequestTimeoutMs != 0
                                                         ? Optional<uint16_t>(convertedTimedRequestTimeoutMs)
                                                         : Optional<uint16_t>::Missing()));
    SuccessOrExit(err =
                      commandSender->SendCommandRequest(device->GetSecureSession().Value(),
                                                        imTimeoutMs != 0 ? MakeOptional(System::Clock::Milliseconds32(imTimeoutMs))
                                                                         : Optional<System::Clock::Timeout>::Missing()));

    callback->mCommandSender = commandSender;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Invoke Error: %s", err.AsString());
        if (err == CHIP_JNI_ERROR_EXCEPTION_THROWN)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        callback->OnError(nullptr, err);
        if (commandSender != nullptr)
        {
            Platform::Delete(commandSender);
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
        }
    }
}

/**
 * Takes objects in attributePathList, converts them to app:AttributePathParams, and appends them to outAttributePathParamsList.
 */
CHIP_ERROR ParseAttributePathList(jobject attributePathList, std::vector<app::AttributePathParams> & outAttributePathParamsList)
{
    jint listSize;

    if (attributePathList == nullptr)
    {
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(JniReferences::GetInstance().GetListSize(attributePathList, listSize));

    for (uint8_t i = 0; i < listSize; i++)
    {
        jobject attributePathItem = nullptr;
        ReturnErrorOnFailure(JniReferences::GetInstance().GetListItem(attributePathList, i, attributePathItem));

        EndpointId endpointId;
        ClusterId clusterId;
        AttributeId attributeId;
        ReturnErrorOnFailure(ParseAttributePath(attributePathItem, endpointId, clusterId, attributeId));
        outAttributePathParamsList.push_back(app::AttributePathParams(endpointId, clusterId, attributeId));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ParseAttributePath(jobject attributePath, EndpointId & outEndpointId, ClusterId & outClusterId,
                              AttributeId & outAttributeId)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID getEndpointIdMethod  = nullptr;
    jmethodID getClusterIdMethod   = nullptr;
    jmethodID getAttributeIdMethod = nullptr;
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, attributePath, "getEndpointId", "()Lchip/devicecontroller/model/ChipPathId;", &getEndpointIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, attributePath, "getClusterId", "()Lchip/devicecontroller/model/ChipPathId;", &getClusterIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, attributePath, "getAttributeId", "()Lchip/devicecontroller/model/ChipPathId;", &getAttributeIdMethod));

    jobject endpointIdObj = env->CallObjectMethod(attributePath, getEndpointIdMethod);
    VerifyOrReturnError(endpointIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    jobject clusterIdObj = env->CallObjectMethod(attributePath, getClusterIdMethod);
    VerifyOrReturnError(clusterIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    jobject attributeIdObj = env->CallObjectMethod(attributePath, getAttributeIdMethod);
    VerifyOrReturnError(attributeIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint32_t endpointId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(endpointIdObj, kInvalidEndpointId, endpointId));
    uint32_t clusterId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(clusterIdObj, kInvalidClusterId, clusterId));
    uint32_t attributeId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(attributeIdObj, kInvalidAttributeId, attributeId));

    outEndpointId  = static_cast<EndpointId>(endpointId);
    outClusterId   = static_cast<ClusterId>(clusterId);
    outAttributeId = static_cast<AttributeId>(attributeId);

    return CHIP_NO_ERROR;
}

/**
 * Takes objects in eventPathList, converts them to app:EventPathParams, and appends them to outEventPathParamsList.
 */
CHIP_ERROR ParseEventPathList(jobject eventPathList, std::vector<app::EventPathParams> & outEventPathParamsList)
{
    jint listSize;

    if (eventPathList == nullptr)
    {
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(JniReferences::GetInstance().GetListSize(eventPathList, listSize));

    for (uint8_t i = 0; i < listSize; i++)
    {
        jobject eventPathItem = nullptr;
        ReturnErrorOnFailure(JniReferences::GetInstance().GetListItem(eventPathList, i, eventPathItem));

        EndpointId endpointId;
        ClusterId clusterId;
        EventId eventId;
        bool isUrgent;
        ReturnErrorOnFailure(ParseEventPath(eventPathItem, endpointId, clusterId, eventId, isUrgent));
        outEventPathParamsList.push_back(app::EventPathParams(endpointId, clusterId, eventId, isUrgent));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ParseEventPath(jobject eventPath, EndpointId & outEndpointId, ClusterId & outClusterId, EventId & outEventId,
                          bool & outIsUrgent)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID getEndpointIdMethod = nullptr;
    jmethodID getClusterIdMethod  = nullptr;
    jmethodID getEventIdMethod    = nullptr;
    jmethodID isUrgentMethod      = nullptr;

    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, eventPath, "getEndpointId", "()Lchip/devicecontroller/model/ChipPathId;", &getEndpointIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, eventPath, "getClusterId", "()Lchip/devicecontroller/model/ChipPathId;", &getClusterIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, eventPath, "getEventId",
                                                                 "()Lchip/devicecontroller/model/ChipPathId;", &getEventIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, eventPath, "isUrgent", "()Z", &isUrgentMethod));

    jobject endpointIdObj = env->CallObjectMethod(eventPath, getEndpointIdMethod);
    VerifyOrReturnError(endpointIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    jobject clusterIdObj = env->CallObjectMethod(eventPath, getClusterIdMethod);
    VerifyOrReturnError(clusterIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    jobject eventIdObj = env->CallObjectMethod(eventPath, getEventIdMethod);
    VerifyOrReturnError(eventIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    jboolean isUrgent = env->CallBooleanMethod(eventPath, isUrgentMethod);

    uint32_t endpointId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(endpointIdObj, kInvalidEndpointId, endpointId));
    uint32_t clusterId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(clusterIdObj, kInvalidClusterId, clusterId));
    uint32_t eventId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(eventIdObj, kInvalidEventId, eventId));

    outEndpointId = static_cast<EndpointId>(endpointId);
    outClusterId  = static_cast<ClusterId>(clusterId);
    outEventId    = static_cast<EventId>(eventId);
    outIsUrgent   = (isUrgent == JNI_TRUE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR GetChipPathIdValue(jobject chipPathId, uint32_t wildcardValue, uint32_t & outValue)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    bool idIsWildcard = false;
    ReturnErrorOnFailure(IsWildcardChipPathId(chipPathId, idIsWildcard));

    if (idIsWildcard)
    {
        outValue = wildcardValue;
        return CHIP_NO_ERROR;
    }

    jmethodID getIdMethod = nullptr;
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, chipPathId, "getId", "()J", &getIdMethod));
    outValue = static_cast<uint32_t>(env->CallLongMethod(chipPathId, getIdMethod));
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

    return CHIP_NO_ERROR;
}

CHIP_ERROR IsWildcardChipPathId(jobject chipPathId, bool & isWildcard)
{
    JNIEnv * env            = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID getTypeMethod = nullptr;
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, chipPathId, "getType", "()Lchip/devicecontroller/model/ChipPathId$IdType;", &getTypeMethod));

    jobject idType = env->CallObjectMethod(chipPathId, getTypeMethod);
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);
    VerifyOrReturnError(idType != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);

    jmethodID nameMethod = nullptr;
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, idType, "name", "()Ljava/lang/String;", &nameMethod));

    jstring typeNameString = static_cast<jstring>(env->CallObjectMethod(idType, nameMethod));
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);
    VerifyOrReturnError(typeNameString != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);

    JniUtfString typeNameJniString(env, typeNameString);

    isWildcard = strncmp(typeNameJniString.c_str(), "WILDCARD", 8) == 0;

    return CHIP_NO_ERROR;
}

void * IOThreadMain(void * arg)
{
    JNIEnv * env;
    JavaVMAttachArgs attachArgs;

    // Attach the IO thread to the JVM as a daemon thread.
    // This allows the JVM to shutdown without waiting for this thread to exit.
    attachArgs.version = JNI_VERSION_1_6;
    attachArgs.name    = (char *) "CHIP Device Controller IO Thread";
    attachArgs.group   = NULL;
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

    return NULL;
}

// NOTE: This function SHALL be called with the stack lock held.
CHIP_ERROR StopIOThread()
{
    if (sIOThread != PTHREAD_NULL)
    {
        ChipLogProgress(Controller, "IO thread stopping");
        chip::DeviceLayer::StackUnlock unlock;

        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();

        pthread_join(sIOThread, NULL);
        sIOThread = PTHREAD_NULL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR N2J_PaseVerifierParams(JNIEnv * env, jlong setupPincode, jbyteArray paseVerifier, jobject & outParams)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID constructor;
    jclass paramsClass;

    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/PaseVerifierParams", paramsClass);
    JniClass paseVerifierParamsClass(paramsClass);
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

    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/NetworkLocation", locationClass);
    JniClass networkLocationClass(locationClass);
    SuccessOrExit(err);

    env->ExceptionClear();
    constructor = env->GetMethodID(locationClass, "<init>", "(Ljava/lang/String;II)V");
    VerifyOrExit(constructor != nullptr, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    outLocation = (jobject) env->NewObject(locationClass, constructor, ipAddress, port, interfaceIndex);

    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
exit:
    return err;
}
