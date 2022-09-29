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
#include "AndroidDeviceControllerWrapper.h"
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#include <app/AttributePathParams.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadClient.h>
#include <app/chip-zcl-zpro-codec.h>
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
#include <lib/support/logging/CHIPLogging.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/Protocols.h>
#include <pthread.h>
#include <system/SystemClock.h>
#include <vector>

#include <platform/android/AndroidChipPlatform-JNI.h>

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
static CHIP_ERROR N2J_PaseVerifierParams(JNIEnv * env, jlong setupPincode, jbyteArray pakeVerifier, jobject & outParams);
static CHIP_ERROR N2J_NetworkLocation(JNIEnv * env, jstring ipAddress, jint port, jint interfaceIndex, jobject & outLocation);
static CHIP_ERROR GetChipPathIdValue(jobject chipPathId, uint32_t wildcardValue, uint32_t & outValue);
static CHIP_ERROR ParseAttributePathList(jobject attributePathList,
                                         std::vector<app::AttributePathParams> & outAttributePathParamsList);
static CHIP_ERROR ParseAttributePath(jobject attributePath, EndpointId & outEndpointId, ClusterId & outClusterId,
                                     AttributeId & outAttributeId);
static CHIP_ERROR ParseEventPathList(jobject eventPathList, std::vector<app::EventPathParams> & outEventPathParamsList);
static CHIP_ERROR ParseEventPath(jobject eventPath, EndpointId & outEndpointId, ClusterId & outClusterId, EventId & outEventId);
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

    err = AndroidChipPlatformJNI_OnLoad(jvm, reserved);
    SuccessOrExit(err);

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

    // If the IO thread has been started, shut it down and wait for it to exit.
    if (sIOThread != PTHREAD_NULL)
    {
        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();

        chip::DeviceLayer::StackUnlock unlock;
        pthread_join(sIOThread, NULL);
    }

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

    Optional<Crypto::AesCcm128KeySpan> ipkOptional;
    uint8_t ipkValue[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
    Crypto::AesCcm128KeySpan ipkTempSpan(ipkValue);

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

    err = wrapper->GetAndroidOperationalCredentialsIssuer()->NOCChainGenerated(CHIP_NO_ERROR, jByteArrayNoc.byteSpan(),
                                                                               jByteArrayIcac.byteSpan(), jByteArrayRcac.byteSpan(),
                                                                               ipkOptional, adminSubjectOptional);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to SetNocChain for the device: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err.AsInteger();
}

JNI_METHOD(jlong, newDeviceController)(JNIEnv * env, jobject self, jobject controllerParams)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = NULL;
    long result                              = 0;

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
        uint64_t fabricId                  = env->CallLongMethod(controllerParams, getFabricId);
        uint16_t listenPort                = env->CallIntMethod(controllerParams, getUdpListenPort);
        uint16_t controllerVendorId        = env->CallIntMethod(controllerParams, getControllerVendorId);
        jobject keypairDelegate            = env->CallObjectMethod(controllerParams, getKeypairDelegate);
        jbyteArray rootCertificate         = (jbyteArray) env->CallObjectMethod(controllerParams, getRootCertificate);
        jbyteArray intermediateCertificate = (jbyteArray) env->CallObjectMethod(controllerParams, getIntermediateCertificate);
        jbyteArray operationalCertificate  = (jbyteArray) env->CallObjectMethod(controllerParams, getOperationalCertificate);
        jbyteArray ipk                     = (jbyteArray) env->CallObjectMethod(controllerParams, getIpk);
        uint16_t failsafeTimerSeconds      = env->CallIntMethod(controllerParams, getFailsafeTimerSeconds);
        bool attemptNetworkScanWiFi        = env->CallBooleanMethod(controllerParams, getAttemptNetworkScanWiFi);
        bool attemptNetworkScanThread      = env->CallBooleanMethod(controllerParams, getAttemptNetworkScanThread);
        bool skipCommissioningComplete     = env->CallBooleanMethod(controllerParams, getSkipCommissioningComplete);
        uint64_t adminSubject              = env->CallLongMethod(controllerParams, getAdminSubject);

        std::unique_ptr<chip::Controller::AndroidOperationalCredentialsIssuer> opCredsIssuer(
            new chip::Controller::AndroidOperationalCredentialsIssuer());
        wrapper = AndroidDeviceControllerWrapper::AllocateNew(
            sJVM, self, kLocalDeviceId, fabricId, chip::kUndefinedCATs, &DeviceLayer::SystemLayer(),
            DeviceLayer::TCPEndPointManager(), DeviceLayer::UDPEndPointManager(), std::move(opCredsIssuer), keypairDelegate,
            rootCertificate, intermediateCertificate, operationalCertificate, ipk, listenPort, controllerVendorId,
            failsafeTimerSeconds, attemptNetworkScanWiFi, attemptNetworkScanThread, skipCommissioningComplete, &err);
        SuccessOrExit(err);

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

    RendezvousParameters rendezvousParams = RendezvousParameters()
                                                .SetSetupPINCode(pinCode)
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

    JniUtfString addrJniString(env, address);

    RendezvousParameters rendezvousParams =
        RendezvousParameters()
            .SetDiscriminator(discriminator)
            .SetSetupPINCode(pinCode)
            .SetPeerAddress(Transport::PeerAddress::UDP(const_cast<char *>(addrJniString.c_str()), port));

    CommissioningParameters commissioningParams = wrapper->GetCommissioningParameters();
    if (csrNonce != nullptr)
    {
        JniByteArray jniCsrNonce(env, csrNonce);
        commissioningParams.SetCSRNonce(jniCsrNonce.byteSpan());
    }
    err = wrapper->Controller()->PairDevice(deviceId, rendezvousParams, commissioningParams);

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

    RendezvousParameters rendezvousParams = RendezvousParameters()
                                                .SetSetupPINCode(pinCode)
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

    JniUtfString addrJniString(env, address);

    RendezvousParameters rendezvousParams = RendezvousParameters().SetSetupPINCode(pinCode).SetPeerAddress(
        Transport::PeerAddress::UDP(const_cast<char *>(addrJniString.c_str()), port));

    err = wrapper->Controller()->EstablishPASEConnection(deviceId, rendezvousParams);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to establish PASE connection.");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
    }
}

JNI_METHOD(void, setUseJavaCallbackForNOCRequest)
(JNIEnv * env, jobject self, jlong handle, jboolean useCallback)
{
    ChipLogProgress(Controller, "setUseJavaCallbackForNOCRequest() called");
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    wrapper->GetAndroidOperationalCredentialsIssuer()->SetUseJavaCallbackForNOCRequest(useCallback);

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
    }
    err = wrapper->UpdateCommissioningParameters(commissioningParams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "UpdateCommissioningParameters failed. Err = %" CHIP_ERROR_FORMAT, err.Format());
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, err);
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

        err = JniReferences::GetInstance().N2J_ByteArray(env, outBytes.data(), outBytes.size(), outJbytes);
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

    err = wrapper->Controller()->UnpairDevice(deviceId);

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

JNI_METHOD(void, shutdownSubscriptions)(JNIEnv * env, jobject self, jlong handle, jlong devicePtr)
{
    chip::DeviceLayer::StackLock lock;

    DeviceProxy * device = reinterpret_cast<DeviceProxy *>(devicePtr);

    //
    // We should move away from this model of shutting down subscriptions in this manner and instead,
    // have Java own the ReadClient objects directly and manage their lifetimes.
    //
    // #13163 tracks this issue.
    //
    device->ShutdownSubscriptions();
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
(JNIEnv * env, jobject self, jlong handle, jlong devicePtr, jint duration, jlong iteration, jint discriminator, jlong setupPinCode)
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

    chip::SetupPayload setupPayload;
    err = AutoCommissioningWindowOpener::OpenCommissioningWindow(
        wrapper->Controller(), chipDevice->GetDeviceId(), System::Clock::Seconds16(duration), iteration, discriminator,
        MakeOptional(static_cast<uint32_t>(setupPinCode)), NullOptional, setupPayload);

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
(JNIEnv * env, jobject self, jlong handle, jlong devicePtr, jint duration, jlong iteration, jint discriminator, jlong setupPinCode,
 jobject jcallback)
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

    chip::SetupPayload setupPayload;
    err = AndroidCommissioningWindowOpener::OpenCommissioningWindow(
        wrapper->Controller(), chipDevice->GetDeviceId(), System::Clock::Seconds16(duration), iteration, discriminator,
        MakeOptional(static_cast<uint32_t>(setupPinCode)), NullOptional, jcallback, setupPayload);

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
    }

    err = chipDevice->GetAttestationChallenge(attestationChallenge);
    SuccessOrExit(err);
    VerifyOrExit(attestationChallenge.size() == 16, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = JniReferences::GetInstance().N2J_ByteArray(env, attestationChallenge.data(), attestationChallenge.size(),
                                                     attestationChallengeJbytes);
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
    err = wrapper->Controller()->ComputePASEVerifier(iterations, setupPincode, jniSalt.byteSpan(), verifier);
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
 jint minInterval, jint maxInterval, jboolean keepSubscriptions, jboolean isFabricFiltered)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceProxy * device = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (device == nullptr)
    {
        ChipLogError(Controller, "No device found");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, CHIP_ERROR_INCORRECT_STATE);
    }

    std::vector<app::AttributePathParams> attributePathParamsList;
    err = ParseAttributePathList(attributePathList, attributePathParamsList);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error parsing Java attribute paths: %s", ErrorStr(err)));

    std::vector<app::EventPathParams> eventPathParamsList;
    err = ParseEventPathList(eventPathList, eventPathParamsList);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error parsing Java event paths: %s", ErrorStr(err)));

    app::ReadPrepareParams params(device->GetSecureSession().Value());
    params.mMinIntervalFloorSeconds     = minInterval;
    params.mMaxIntervalCeilingSeconds   = maxInterval;
    params.mpAttributePathParamsList    = attributePathParamsList.data();
    params.mAttributePathParamsListSize = attributePathParamsList.size();
    params.mpEventPathParamsList        = eventPathParamsList.data();
    params.mEventPathParamsListSize     = eventPathParamsList.size();
    params.mKeepSubscriptions           = (keepSubscriptions != JNI_FALSE);
    params.mIsFabricFiltered            = (isFabricFiltered != JNI_FALSE);

    auto callback = reinterpret_cast<ReportCallback *>(callbackHandle);

    app::ReadClient * readClient =
        Platform::New<app::ReadClient>(app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                       callback->mBufferedReadAdapter, app::ReadClient::InteractionType::Subscribe);

    err = readClient->SendRequest(params);
    if (err != CHIP_NO_ERROR)
    {
        chip::AndroidClusterExceptions::GetInstance().ReturnIllegalStateException(env, callback->mReportCallbackRef, ErrorStr(err),
                                                                                  err);
        delete readClient;
        delete callback;
        return;
    }

    callback->mReadClient = readClient;
}

JNI_METHOD(void, read)
(JNIEnv * env, jobject self, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList, jobject eventPathList,
 jboolean isFabricFiltered)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceProxy * device = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (device == nullptr)
    {
        ChipLogError(Controller, "No device found");
        JniReferences::GetInstance().ThrowError(env, sChipDeviceControllerExceptionCls, CHIP_ERROR_INCORRECT_STATE);
    }

    std::vector<app::AttributePathParams> attributePathParamsList;
    err = ParseAttributePathList(attributePathList, attributePathParamsList);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error parsing Java attribute paths: %s", ErrorStr(err)));

    std::vector<app::EventPathParams> eventPathParamsList;
    err = ParseEventPathList(eventPathList, eventPathParamsList);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Error parsing Java event paths: %s", ErrorStr(err)));

    app::ReadPrepareParams params(device->GetSecureSession().Value());
    params.mpAttributePathParamsList    = attributePathParamsList.data();
    params.mAttributePathParamsListSize = attributePathParamsList.size();
    params.mpEventPathParamsList        = eventPathParamsList.data();
    params.mEventPathParamsListSize     = eventPathParamsList.size();

    params.mIsFabricFiltered = (isFabricFiltered != JNI_FALSE);

    auto callback = reinterpret_cast<ReportCallback *>(callbackHandle);

    app::ReadClient * readClient =
        Platform::New<app::ReadClient>(app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                       callback->mBufferedReadAdapter, app::ReadClient::InteractionType::Read);

    err = readClient->SendRequest(params);
    if (err != CHIP_NO_ERROR)
    {
        chip::AndroidClusterExceptions::GetInstance().ReturnIllegalStateException(env, callback->mReportCallbackRef, ErrorStr(err),
                                                                                  err);
        delete readClient;
        delete callback;
        return;
    }

    callback->mReadClient = readClient;
}

/**
 * Takes objects in attributePathList, converts them to app:AttributePathParams, and appends them to outAttributePathParamsList.
 */
CHIP_ERROR ParseAttributePathList(jobject attributePathList, std::vector<app::AttributePathParams> & outAttributePathParamsList)
{
    jint listSize;
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
    VerifyOrReturnError(endpointIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    jobject attributeIdObj = env->CallObjectMethod(attributePath, getAttributeIdMethod);
    VerifyOrReturnError(endpointIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);

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
    ReturnErrorOnFailure(JniReferences::GetInstance().GetListSize(eventPathList, listSize));

    for (uint8_t i = 0; i < listSize; i++)
    {
        jobject eventPathItem = nullptr;
        ReturnErrorOnFailure(JniReferences::GetInstance().GetListItem(eventPathList, i, eventPathItem));

        EndpointId endpointId;
        ClusterId clusterId;
        EventId eventId;
        ReturnErrorOnFailure(ParseEventPath(eventPathItem, endpointId, clusterId, eventId));
        outEventPathParamsList.push_back(app::EventPathParams(endpointId, clusterId, eventId));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ParseEventPath(jobject eventPath, EndpointId & outEndpointId, ClusterId & outClusterId, EventId & outEventId)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID getEndpointIdMethod = nullptr;
    jmethodID getClusterIdMethod  = nullptr;
    jmethodID getEventIdMethod    = nullptr;

    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, eventPath, "getEndpointId", "()Lchip/devicecontroller/model/ChipPathId;", &getEndpointIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, eventPath, "getClusterId", "()Lchip/devicecontroller/model/ChipPathId;", &getClusterIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, eventPath, "getEventId",
                                                                 "()Lchip/devicecontroller/model/ChipPathId;", &getEventIdMethod));

    jobject endpointIdObj = env->CallObjectMethod(eventPath, getEndpointIdMethod);
    VerifyOrReturnError(endpointIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    jobject clusterIdObj = env->CallObjectMethod(eventPath, getClusterIdMethod);
    VerifyOrReturnError(clusterIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    jobject eventIdObj = env->CallObjectMethod(eventPath, getEventIdMethod);
    VerifyOrReturnError(eventIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint32_t endpointId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(endpointIdObj, kInvalidEndpointId, endpointId));
    uint32_t clusterId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(clusterIdObj, kInvalidClusterId, clusterId));
    uint32_t eventId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(eventIdObj, kInvalidEventId, eventId));

    outEndpointId = static_cast<EndpointId>(endpointId);
    outClusterId  = static_cast<ClusterId>(clusterId);
    outEventId    = static_cast<EventId>(eventId);

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
    outValue = env->CallLongMethod(chipPathId, getIdMethod);
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
    VerifyOrReturnError(idType != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);

    jmethodID nameMethod = nullptr;
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, idType, "name", "()Ljava/lang/String;", &nameMethod));

    jstring typeNameString = static_cast<jstring>(env->CallObjectMethod(idType, nameMethod));
    VerifyOrReturnError(idType != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);
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
