/*
 *   Copyright (c) 2020-2021 Project CHIP Authors
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
#include "AndroidDeviceControllerWrapper.h"
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#include <app/chip-zcl-zpro-codec.h>
#include <atomic>
#include <ble/BleUUID.h>
#include <controller/CHIPDeviceController.h>
#include <jni.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/Protocols.h>
#include <protocols/temp_zcl/TempZCL.h>
#include <pthread.h>

#include <platform/android/AndroidChipPlatform-JNI.h>

// Choose an approximation of PTHREAD_NULL if pthread.h doesn't define one.
#ifndef PTHREAD_NULL
#define PTHREAD_NULL 0
#endif // PTHREAD_NULL

using namespace chip;
using namespace chip::Inet;
using namespace chip::Controller;

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_ChipDeviceController_##METHOD_NAME

#define CDC_JNI_CALLBACK_LOCAL_REF_COUNT 256

static void ThrowError(JNIEnv * env, CHIP_ERROR errToThrow);
static void * IOThreadMain(void * arg);
static CHIP_ERROR N2J_Error(JNIEnv * env, CHIP_ERROR inErr, jthrowable & outEx);

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
        ThrowError(env, err);
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

JNI_METHOD(jlong, newDeviceController)(JNIEnv * env, jobject self)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = NULL;
    long result                              = 0;

    ChipLogProgress(Controller, "newDeviceController() called");

    // sSystemLayer and sInetLayer are in platform/android to share with app server
    err = DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    wrapper = AndroidDeviceControllerWrapper::AllocateNew(sJVM, self, kLocalDeviceId, &DeviceLayer::SystemLayer(),
                                                          &DeviceLayer::InetLayer, &err);
    SuccessOrExit(err);

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
            ThrowError(env, err);
        }
    }

    return result;
}

JNI_METHOD(void, pairDevice)
(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jint connObj, jlong pinCode, jbyteArray csrNonce)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "pairDevice() called with device ID, connection object, and pincode");

    RendezvousParameters params = RendezvousParameters()
                                      .SetSetupPINCode(pinCode)
                                      .SetConnectionObject(reinterpret_cast<BLE_CONNECTION_OBJECT>(connObj))
                                      .SetPeerAddress(Transport::PeerAddress::BLE());
    if (csrNonce != nullptr)
    {
        JniByteArray jniCsrNonce(env, csrNonce);
        params.SetCSRNonce(jniCsrNonce.byteSpan());
    }
    err = wrapper->Controller()->PairDevice(deviceId, params);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to pair the device.");
        ThrowError(env, err);
    }
}

JNI_METHOD(void, pairDeviceWithAddress)
(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jstring address, jint port, jint discriminator, jint pinCode,
 jbyteArray csrNonce)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "pairDeviceWithAddress() called");

    Inet::IPAddress addr;
    JniUtfString addrJniString(env, address);
    VerifyOrReturn(Inet::IPAddress::FromString(addrJniString.c_str(), addr),
                   ChipLogError(Controller, "Failed to parse IP address."), ThrowError(env, CHIP_ERROR_INVALID_ARGUMENT));

    RendezvousParameters params = RendezvousParameters()
                                      .SetDiscriminator(discriminator)
                                      .SetSetupPINCode(pinCode)
                                      .SetPeerAddress(Transport::PeerAddress::UDP(addr, port));
    if (csrNonce != nullptr)
    {
        JniByteArray jniCsrNonce(env, csrNonce);
        params.SetCSRNonce(jniCsrNonce.byteSpan());
    }
    err = wrapper->Controller()->PairDevice(deviceId, params);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to pair the device.");
        ThrowError(env, err);
    }
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
        ThrowError(env, err);
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
        ThrowError(env, err);
    }
}

JNI_METHOD(void, getConnectedDevicePointer)(JNIEnv * env, jobject self, jlong handle, jlong nodeId, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    GetConnectedDeviceCallback * connectedDeviceCallback = reinterpret_cast<GetConnectedDeviceCallback *>(callbackHandle);
    VerifyOrReturn(connectedDeviceCallback != nullptr, ChipLogError(Controller, "GetConnectedDeviceCallback handle is nullptr"));
    wrapper->Controller()->GetConnectedDevice(nodeId, &connectedDeviceCallback->mOnSuccess, &connectedDeviceCallback->mOnFailure);
}

JNI_METHOD(void, pairTestDeviceWithoutSecurity)(JNIEnv * env, jobject self, jlong handle, jstring deviceAddr)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);
    chip::Inet::IPAddress deviceIPAddr;

    ChipLogProgress(Controller, "pairTestDeviceWithoutSecurity() called with IP Address");

    const char * deviceAddrStr = env->GetStringUTFChars(deviceAddr, 0);
    deviceIPAddr.FromString(deviceAddrStr, deviceIPAddr);
    env->ReleaseStringUTFChars(deviceAddr, deviceAddrStr);

    Controller::SerializedDevice mSerializedTestDevice;
    err = wrapper->Controller()->PairTestDeviceWithoutSecurity(kRemoteDeviceId, chip::Transport::PeerAddress::UDP(deviceIPAddr),
                                                               mSerializedTestDevice);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to connect to device.");
        ThrowError(env, err);
    }
}

JNI_METHOD(void, disconnectDevice)(JNIEnv * env, jobject self, jlong handle, jlong deviceId)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "disconnectDevice() called with deviceId");
    wrapper->Controller()->ReleaseDeviceById(deviceId);
}

JNI_METHOD(jboolean, isActive)(JNIEnv * env, jobject self, jlong handle)
{
    chip::DeviceLayer::StackLock lock;

    Device * chipDevice = reinterpret_cast<Device *>(handle);
    return chipDevice->IsActive();
}

JNI_METHOD(jstring, getIpAddress)(JNIEnv * env, jobject self, jlong handle, jlong deviceId)
{
    chip::DeviceLayer::StackLock lock;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    chip::Inet::IPAddress addr;
    uint16_t port;
    char addrStr[50];

    CHIP_ERROR err =
        wrapper->Controller()->GetPeerAddressAndPort(PeerId()
                                                         .SetCompressedFabricId(wrapper->Controller()->GetCompressedFabricId())
                                                         .SetNodeId(static_cast<chip::NodeId>(deviceId)),
                                                     addr, port);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to get device address.");
        ThrowError(env, err);
    }

    addr.ToString(addrStr);
    return env->NewStringUTF(addrStr);
}

JNI_METHOD(jlong, getCompressedFabricId)(JNIEnv * env, jobject self, jlong handle)
{
    chip::DeviceLayer::StackLock lock;

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    return wrapper->Controller()->GetCompressedFabricId();
}

JNI_METHOD(void, updateDevice)(JNIEnv * env, jobject self, jlong handle, jlong fabricId, jlong deviceId)
{
    chip::DeviceLayer::StackLock lock;

    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    CHIP_ERROR err = wrapper->Controller()->UpdateDevice(static_cast<chip::NodeId>(deviceId));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to update device");
        ThrowError(env, err);
    }
}

JNI_METHOD(jboolean, openPairingWindow)(JNIEnv * env, jobject self, jlong handle, jlong devicePtr, jint duration)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::SetupPayload setupPayload;

    Device * chipDevice = reinterpret_cast<Device *>(devicePtr);
    if (chipDevice == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return false;
    }

    err = chipDevice->OpenPairingWindow(duration, chip::Controller::Device::CommissioningWindowOption::kOriginalSetupCode,
                                        setupPayload);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "OpenPairingWindow failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, openPairingWindowWithPIN)
(JNIEnv * env, jobject self, jlong handle, jlong devicePtr, jint duration, jint iteration, jint discriminator, jlong setupPinCode)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::SetupPayload setupPayload;
    setupPayload.discriminator = discriminator;
    setupPayload.setUpPINCode  = setupPinCode;

    Device * chipDevice = reinterpret_cast<Device *>(devicePtr);
    if (chipDevice == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return false;
    }

    err = chipDevice->OpenPairingWindow(duration, chip::Controller::Device::CommissioningWindowOption::kTokenWithRandomPIN,
                                        setupPayload);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "OpenPairingWindow failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    return true;
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

void ThrowError(JNIEnv * env, CHIP_ERROR errToThrow)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jthrowable ex;

    err = N2J_Error(env, errToThrow, ex);
    if (err == CHIP_NO_ERROR)
    {
        env->Throw(ex);
    }
}

CHIP_ERROR N2J_Error(JNIEnv * env, CHIP_ERROR inErr, jthrowable & outEx)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    const char * errStr = NULL;
    jstring errStrObj   = NULL;
    jmethodID constructor;

    env->ExceptionClear();
    constructor = env->GetMethodID(sChipDeviceControllerExceptionCls, "<init>", "(ILjava/lang/String;)V");
    VerifyOrExit(constructor != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    switch (inErr.AsInteger())
    {
    case CHIP_JNI_ERROR_TYPE_NOT_FOUND.AsInteger():
        errStr = "CHIP Device Controller Error: JNI type not found";
        break;
    case CHIP_JNI_ERROR_METHOD_NOT_FOUND.AsInteger():
        errStr = "CHIP Device Controller Error: JNI method not found";
        break;
    case CHIP_JNI_ERROR_FIELD_NOT_FOUND.AsInteger():
        errStr = "CHIP Device Controller Error: JNI field not found";
        break;
    case CHIP_JNI_ERROR_DEVICE_NOT_FOUND.AsInteger():
        errStr = "CHIP Device Controller Error: Device not found";
        break;
    default:
        errStr = ErrorStr(inErr);
        break;
    }
    errStrObj = (errStr != NULL) ? env->NewStringUTF(errStr) : NULL;

    outEx = (jthrowable) env->NewObject(sChipDeviceControllerExceptionCls, constructor, static_cast<jint>(inErr.AsInteger()),
                                        errStrObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    env->DeleteLocalRef(errStrObj);
    return err;
}
