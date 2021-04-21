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

#include "AndroidBleApplicationDelegate.h"
#include "AndroidBleConnectionDelegate.h"
#include "AndroidBlePlatformDelegate.h"
#include "AndroidDeviceControllerWrapper.h"
#include "CHIPJNIError.h"
#include "JniReferences.h"
#include "JniTypeWrappers.h"

#include <app/chip-zcl-zpro-codec.h>
#include <atomic>
#include <ble/BleUUID.h>
#include <controller/CHIPDeviceController.h>
#include <jni.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/Protocols.h>
#include <pthread.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/SafeInt.h>
#include <support/ThreadOperationalDataset.h>
#include <support/logging/CHIPLogging.h>

// Choose an approximation of PTHREAD_NULL if pthread.h doesn't define one.
#ifndef PTHREAD_NULL
#define PTHREAD_NULL 0
#endif // PTHREAD_NULL

using namespace chip;
using namespace chip::Inet;
using namespace chip::Controller;

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_ChipDeviceController_##METHOD_NAME

#define JNI_ANDROID_CHIP_STACK_METHOD(RETURN, METHOD_NAME)                                                                         \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_AndroidChipStack_##METHOD_NAME

#define CDC_JNI_CALLBACK_LOCAL_REF_COUNT 256

static void GetCHIPDevice(JNIEnv * env, long wrapperHandle, uint64_t deviceId, Device ** device);
static void HandleNotifyChipConnectionClosed(BLE_CONNECTION_OBJECT connObj);
static bool HandleSendCharacteristic(BLE_CONNECTION_OBJECT connObj, const uint8_t * svcId, const uint8_t * charId,
                                     const uint8_t * characteristicData, uint32_t characteristicDataLen);
static bool HandleSubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const uint8_t * svcId, const uint8_t * charId);
static bool HandleUnsubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const uint8_t * svcId, const uint8_t * charId);
static bool HandleCloseConnection(BLE_CONNECTION_OBJECT connObj);
static uint16_t HandleGetMTU(BLE_CONNECTION_OBJECT connObj);
static void HandleNewConnection(void * appState, const uint16_t discriminator);
static void ThrowError(JNIEnv * env, CHIP_ERROR errToThrow);
static void ReportError(JNIEnv * env, CHIP_ERROR cbErr, const char * cbName);
static void * IOThreadMain(void * arg);
static CHIP_ERROR N2J_ByteArray(JNIEnv * env, const uint8_t * inArray, uint32_t inArrayLen, jbyteArray & outArray);
static CHIP_ERROR N2J_Error(JNIEnv * env, CHIP_ERROR inErr, jthrowable & outEx);

namespace {

JavaVM * sJVM;
System::Layer sSystemLayer;
Inet::InetLayer sInetLayer;

#if CONFIG_NETWORK_LAYER_BLE
Ble::BleLayer sBleLayer;
AndroidBleApplicationDelegate sBleApplicationDelegate;
AndroidBlePlatformDelegate sBlePlatformDelegate;
AndroidBleConnectionDelegate sBleConnectionDelegate;
#endif

pthread_mutex_t sStackLock = PTHREAD_MUTEX_INITIALIZER;
pthread_t sIOThread        = PTHREAD_NULL;
bool sShutdown             = false;

jclass sAndroidChipStackCls              = NULL;
jclass sChipDeviceControllerExceptionCls = NULL;

/** A scoped lock/unlock around a mutex. */
class ScopedPthreadLock
{
public:
    ScopedPthreadLock(pthread_mutex_t * mutex) : mMutex(mutex) { pthread_mutex_lock(mMutex); }
    ~ScopedPthreadLock() { pthread_mutex_unlock(mMutex); }

private:
    pthread_mutex_t * mMutex;
};

// Use StackUnlockGuard to temporarily unlock the CHIP BLE stack, e.g. when calling application
// or Android BLE code as a result of a BLE event.
struct StackUnlockGuard
{
    StackUnlockGuard() { pthread_mutex_unlock(&sStackLock); }
    ~StackUnlockGuard() { pthread_mutex_lock(&sStackLock); }
};

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
    int pthreadErr = 0;

    ChipLogProgress(Controller, "JNI_OnLoad() called");

    chip::Platform::MemoryInit();

    // Save a reference to the JVM.  Will need this to call back into Java.
    sJVM = jvm;

    // Get a JNI environment object.
    sJVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().InitializeMethodForward(env);

    ChipLogProgress(Controller, "Loading Java class references.");

    // Get various class references need by the API.
    err = GetClassRef(env, "chip/devicecontroller/AndroidChipStack", sAndroidChipStackCls);
    SuccessOrExit(err);
    err = GetClassRef(env, "chip/devicecontroller/ChipDeviceControllerException", sChipDeviceControllerExceptionCls);
    SuccessOrExit(err);
    ChipLogProgress(Controller, "Java class references loaded.");

    // Initialize the CHIP System Layer.
    err = sSystemLayer.Init(NULL);
    SuccessOrExit(err);

    // Initialize the CHIP Inet layer.
    err = sInetLayer.Init(sSystemLayer, NULL);
    SuccessOrExit(err);
    ChipLogProgress(Controller, "Inet layer initialized.");

#if CONFIG_NETWORK_LAYER_BLE
    ChipLogProgress(Controller, "BLE Layer being configured.");

    // Initialize the BleApplicationDelegate
    sBleApplicationDelegate.SetNotifyChipConnectionClosedCallback(HandleNotifyChipConnectionClosed);
    // Initialize the BlePlatformDelegate
    sBlePlatformDelegate.SetSendWriteRequestCallback(HandleSendCharacteristic);
    sBlePlatformDelegate.SetSubscribeCharacteristicCallback(HandleSubscribeCharacteristic);
    sBlePlatformDelegate.SetUnsubscribeCharacteristicCallback(HandleUnsubscribeCharacteristic);
    sBlePlatformDelegate.SetCloseConnectionCallback(HandleCloseConnection);
    sBlePlatformDelegate.SetGetMTUCallback(HandleGetMTU);
    // Initialize the BleConnectionDelegate
    sBleConnectionDelegate.SetNewConnectionCallback(HandleNewConnection);

    ChipLogProgress(Controller, "Asking for BLE Layer initialization.");
    // Initialize the BleLayer object.
    err = sBleLayer.Init(&sBlePlatformDelegate, &sBleConnectionDelegate, &sBleApplicationDelegate, &sSystemLayer);
    SuccessOrExit(err);

    ChipLogProgress(Controller, "BLE was initialized.");
#endif

    // Create and start the IO thread.
    sShutdown  = false;
    pthreadErr = pthread_create(&sIOThread, NULL, IOThreadMain, NULL);
    VerifyOrExit(pthreadErr == 0, err = System::MapErrorPOSIX(pthreadErr));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ThrowError(env, err);
        JNI_OnUnload(jvm, reserved);
    }

    return (err == CHIP_NO_ERROR) ? JNI_VERSION_1_6 : JNI_ERR;
}

void JNI_OnUnload(JavaVM * jvm, void * reserved)
{
    ChipLogProgress(Controller, "JNI_OnUnload() called");

    // If the IO thread has been started, shut it down and wait for it to exit.
    if (sIOThread != PTHREAD_NULL)
    {
        sShutdown = true;
        sSystemLayer.WakeSelect();
        pthread_join(sIOThread, NULL);
    }

#if CONFIG_NETWORK_LAYER_BLE
    sBleLayer.Shutdown();
#endif

    sSystemLayer.Shutdown();
    sInetLayer.Shutdown();
    sJVM = NULL;

    chip::Platform::MemoryShutdown();
}

JNI_METHOD(jlong, newDeviceController)(JNIEnv * env, jobject self)
{
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = NULL;
    long result                              = 0;

    ChipLogProgress(Controller, "newDeviceController() called");

    wrapper = AndroidDeviceControllerWrapper::AllocateNew(sJVM, self, kLocalDeviceId, &sSystemLayer, &sInetLayer, &err);
    SuccessOrExit(err);

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

JNI_METHOD(void, pairDevice)(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jint connObj, jlong pinCode)
{
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "pairDevice() called with device ID, connection object, and pincode");

    {
        ScopedPthreadLock lock(&sStackLock);
        sBleLayer.mAppState         = (void *) self;
        RendezvousParameters params = RendezvousParameters()
                                          .SetSetupPINCode(pinCode)
                                          .SetConnectionObject(reinterpret_cast<BLE_CONNECTION_OBJECT>(connObj))
                                          .SetBleLayer(&sBleLayer)
                                          .SetPeerAddress(Transport::PeerAddress::BLE());
        err = wrapper->Controller()->PairDevice(deviceId, params);
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to pair the device.");
        ThrowError(env, err);
    }
}

JNI_METHOD(void, unpairDevice)(JNIEnv * env, jobject self, jlong handle, jlong deviceId)
{
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "unpairDevice() called with device ID");

    {
        ScopedPthreadLock lock(&sStackLock);
        err = wrapper->Controller()->UnpairDevice(deviceId);
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to unpair the device.");
        ThrowError(env, err);
    }
}

JNI_METHOD(void, stopDevicePairing)(JNIEnv * env, jobject self, jlong handle, jlong deviceId)
{
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "stopDevicePairing() called with device ID");

    {
        ScopedPthreadLock lock(&sStackLock);
        err = wrapper->Controller()->StopPairing(deviceId);
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to unpair the device.");
        ThrowError(env, err);
    }
}

JNI_METHOD(void, sendWiFiCredentials)(JNIEnv * env, jobject self, jlong handle, jstring ssid, jstring password)
{
    JniUtfString ssidStr(env, ssid);
    JniUtfString passwordStr(env, password);

    ChipLogProgress(Controller, "Sending Wi-Fi credentials for: %s", ssidStr.c_str());
    {
        ScopedPthreadLock lock(&sStackLock);
        AndroidDeviceControllerWrapper::FromJNIHandle(handle)->SendNetworkCredentials(ssidStr.c_str(), passwordStr.c_str());
    }
}

JNI_METHOD(void, sendThreadCredentials)
(JNIEnv * env, jobject self, jlong handle, jint channel, jint panId, jbyteArray xpanId, jbyteArray masterKey)
{
    using namespace chip::DeviceLayer::Internal;

    JniByteArray xpanIdBytes(env, xpanId);
    JniByteArray masterKeyBytes(env, masterKey);

    VerifyOrReturn(CanCastTo<uint8_t>(channel), ChipLogError(Controller, "sendThreadCredentials() called with invalid Channel"));
    VerifyOrReturn(CanCastTo<uint16_t>(panId), ChipLogError(Controller, "sendThreadCredentials() called with invalid PAN ID"));
    VerifyOrReturn(xpanIdBytes.size() == static_cast<jsize>(Thread::kSizeExtendedPanId),
                   ChipLogError(Controller, "sendThreadCredentials() called with invalid XPAN ID"));
    VerifyOrReturn(masterKeyBytes.size() == static_cast<jsize>(Thread::kSizeMasterKey),
                   ChipLogError(Controller, "sendThreadCredentials() called with invalid Master Key"));

    Thread::OperationalDataset dataset{};

    dataset.SetChannel(channel);
    dataset.SetPanId(panId);

    // TODO network name is required, need to add UI
    {
        char networkName[Thread::kSizeNetworkName + 1];

        snprintf(networkName, sizeof(networkName), "CHIP-%04X", panId);
        dataset.SetNetworkName(networkName);
    }

    {
        uint8_t value[Thread::kSizeMasterKey];

        memcpy(value, masterKeyBytes.data(), sizeof(value));

        dataset.SetMasterKey(value);
    }
    {
        uint8_t value[Thread::kSizeExtendedPanId];

        memcpy(value, xpanIdBytes.data(), sizeof(value));

        dataset.SetExtendedPanId(value);
    }

    ScopedPthreadLock lock(&sStackLock);
    AndroidDeviceControllerWrapper::FromJNIHandle(handle)->SendThreadCredentials(dataset.AsByteSpan());
}

JNI_METHOD(void, pairTestDeviceWithoutSecurity)(JNIEnv * env, jobject self, jlong handle, jstring deviceAddr)
{
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);
    chip::Inet::IPAddress deviceIPAddr;

    ChipLogProgress(Controller, "pairTestDeviceWithoutSecurity() called with IP Address");

    const char * deviceAddrStr = env->GetStringUTFChars(deviceAddr, 0);
    deviceIPAddr.FromString(deviceAddrStr, deviceIPAddr);
    env->ReleaseStringUTFChars(deviceAddr, deviceAddrStr);

    {
        ScopedPthreadLock lock(&sStackLock);
        Controller::SerializedDevice mSerializedTestDevice;
        err = wrapper->Controller()->PairTestDeviceWithoutSecurity(kRemoteDeviceId, chip::Transport::PeerAddress::UDP(deviceIPAddr),
                                                                   mSerializedTestDevice);
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to connect to device.");
        ThrowError(env, err);
    }
}

JNI_METHOD(void, disconnectDevice)(JNIEnv * env, jobject self, jlong handle, jlong deviceId)
{
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    Device * chipDevice                      = nullptr;

    ChipLogProgress(Controller, "disconnectDevice() called with deviceId");

    {
        ScopedPthreadLock lock(&sStackLock);
        err = wrapper->Controller()->GetDevice(deviceId, &chipDevice);
    }

    if (err != CHIP_NO_ERROR || !chipDevice)
    {
        ChipLogError(Controller, "Failed to get paired device.");
        ThrowError(env, err);
    }

    wrapper->Controller()->ReleaseDevice(chipDevice);
}

JNI_METHOD(jboolean, isActive)(JNIEnv * env, jobject self, jlong handle)
{
    Device * chipDevice = reinterpret_cast<Device *>(handle);

    {
        ScopedPthreadLock lock(&sStackLock);
        return chipDevice->IsActive();
    }
}

void GetCHIPDevice(JNIEnv * env, long wrapperHandle, uint64_t deviceId, Device ** chipDevice)
{
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(wrapperHandle);
    CHIP_ERROR err                           = CHIP_NO_ERROR;

    {
        ScopedPthreadLock lock(&sStackLock);
        err = wrapper->Controller()->GetDevice(deviceId, chipDevice);
    }

    if (err != CHIP_NO_ERROR || !chipDevice)
    {
        ChipLogError(Controller, "Failed to get paired device.");
        ThrowError(env, err);
    }
}

JNI_METHOD(jstring, getIpAddress)(JNIEnv * env, jobject self, jlong handle, jlong deviceId)
{
    Device * chipDevice = nullptr;

    GetCHIPDevice(env, handle, deviceId, &chipDevice);

    chip::Inet::IPAddress addr;
    uint16_t port;
    char addrStr[50];

    {
        ScopedPthreadLock lock(&sStackLock);
        if (!chipDevice->GetAddress(addr, port))
            return nullptr;
    }

    addr.ToString(addrStr);
    return env->NewStringUTF(addrStr);
}

JNI_METHOD(void, sendMessage)(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jstring messageObj)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    Device * chipDevice = nullptr;

    ChipLogProgress(Controller, "sendMessage() called with device id and message object");

    GetCHIPDevice(env, handle, deviceId, &chipDevice);

    const char * messageStr = env->GetStringUTFChars(messageObj, 0);
    size_t messageLen       = strlen(messageStr);

    {
        ScopedPthreadLock lock(&sStackLock);

        System::PacketBufferHandle buffer = System::PacketBufferHandle::NewWithData(messageStr, messageLen);
        if (buffer.IsNull())
        {
            err = CHIP_ERROR_NO_MEMORY;
        }
        else
        {
            err = chipDevice->SendMessage(Protocols::TempZCL::Id, 0, std::move(buffer));
        }
    }

    env->ReleaseStringUTFChars(messageObj, messageStr);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to send message.");
        ThrowError(env, err);
    }
}

JNI_METHOD(void, sendCommand)(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jobject commandObj, jint aValue)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    Device * chipDevice = nullptr;

    GetCHIPDevice(env, handle, deviceId, &chipDevice);

    ChipLogProgress(Controller, "sendCommand() called");

    jclass commandCls         = env->GetObjectClass(commandObj);
    jmethodID commandMethodID = env->GetMethodID(commandCls, "getValue", "()I");
    jint commandID            = env->CallIntMethod(commandObj, commandMethodID);

    {
        ScopedPthreadLock lock(&sStackLock);
        System::PacketBufferHandle buffer;

        // Hardcode endpoint to 1 for now
        uint8_t endpoint = 1;

        switch (commandID)
        {
        case 0:
            buffer = encodeOnOffClusterOffCommand(0, endpoint);
            break;
        case 1:
            buffer = encodeOnOffClusterOnCommand(0, endpoint);
            break;
        case 2:
            buffer = encodeOnOffClusterToggleCommand(0, endpoint);
            break;
        case 3:
            buffer = encodeLevelControlClusterMoveToLevelCommand(0, endpoint, (uint8_t)(aValue & 0xff), 0xFFFF, 0, 0);
            break;
        default:
            ChipLogError(Controller, "Unknown command: %d", commandID);
            return;
        }

        if (buffer.IsNull())
        {
            err = CHIP_ERROR_NO_MEMORY;
        }
        else
        {
            err = chipDevice->SendMessage(Protocols::TempZCL::Id, 0, std::move(buffer));
        }
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to send CHIP command.");
        ThrowError(env, err);
    }
}

JNI_METHOD(jboolean, openPairingWindow)(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jint duration)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    Device * chipDevice = nullptr;
    chip::SetupPayload setupPayload;

    GetCHIPDevice(env, handle, deviceId, &chipDevice);

    {
        ScopedPthreadLock lock(&sStackLock);
        err = chipDevice->OpenPairingWindow(duration, chip::Controller::Device::PairingWindowOption::kOriginalSetupCode,
                                            setupPayload);
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "OpenPairingWindow failed: %d", err);
        return false;
    }

    return true;
}

static bool JavaBytesToUUID(JNIEnv * env, jbyteArray value, chip::Ble::ChipBleUUID & uuid)
{
    const auto valueBegin  = env->GetByteArrayElements(value, nullptr);
    const auto valueLength = env->GetArrayLength(value);
    bool result            = true;

    VerifyOrExit(valueBegin && valueLength == sizeof(uuid.bytes), result = false);
    memcpy(uuid.bytes, valueBegin, valueLength);

exit:
    env->ReleaseByteArrayElements(value, valueBegin, 0);
    return result;
}

JNI_ANDROID_CHIP_STACK_METHOD(void, handleIndicationReceived)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId, jbyteArray value)
{
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);
    const auto valueBegin               = env->GetByteArrayElements(value, nullptr);
    const auto valueLength              = env->GetArrayLength(value);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    chip::System::PacketBufferHandle buffer;

    VerifyOrExit(JavaBytesToUUID(env, svcId, svcUUID),
                 ChipLogError(Controller, "handleIndicationReceived() called with invalid service ID"));
    VerifyOrExit(JavaBytesToUUID(env, charId, charUUID),
                 ChipLogError(Controller, "handleIndicationReceived() called with invalid characteristic ID"));

    buffer = System::PacketBufferHandle::NewWithData(valueBegin, valueLength);
    VerifyOrExit(!buffer.IsNull(), ChipLogError(Controller, "Failed to allocate packet buffer"));

    pthread_mutex_lock(&sStackLock);
    sBleLayer.HandleIndicationReceived(connObj, &svcUUID, &charUUID, std::move(buffer));
    pthread_mutex_unlock(&sStackLock);
exit:
    env->ReleaseByteArrayElements(value, valueBegin, 0);
}

JNI_ANDROID_CHIP_STACK_METHOD(void, handleWriteConfirmation)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId)
{
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    VerifyOrExit(JavaBytesToUUID(env, svcId, svcUUID),
                 ChipLogError(Controller, "handleWriteConfirmation() called with invalid service ID"));
    VerifyOrExit(JavaBytesToUUID(env, charId, charUUID),
                 ChipLogError(Controller, "handleWriteConfirmation() called with invalid characteristic ID"));

    pthread_mutex_lock(&sStackLock);
    sBleLayer.HandleWriteConfirmation(connObj, &svcUUID, &charUUID);
    pthread_mutex_unlock(&sStackLock);
exit:
    return;
}

JNI_ANDROID_CHIP_STACK_METHOD(void, handleSubscribeComplete)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId)
{
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    VerifyOrExit(JavaBytesToUUID(env, svcId, svcUUID),
                 ChipLogError(Controller, "handleSubscribeComplete() called with invalid service ID"));
    VerifyOrExit(JavaBytesToUUID(env, charId, charUUID),
                 ChipLogError(Controller, "handleSubscribeComplete() called with invalid characteristic ID"));

    pthread_mutex_lock(&sStackLock);
    sBleLayer.HandleSubscribeComplete(connObj, &svcUUID, &charUUID);
    pthread_mutex_unlock(&sStackLock);
exit:
    return;
}

JNI_ANDROID_CHIP_STACK_METHOD(void, handleUnsubscribeComplete)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId)
{
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    VerifyOrExit(JavaBytesToUUID(env, svcId, svcUUID),
                 ChipLogError(Controller, "handleUnsubscribeComplete() called with invalid service ID"));
    VerifyOrExit(JavaBytesToUUID(env, charId, charUUID),
                 ChipLogError(Controller, "handleUnsubscribeComplete() called with invalid characteristic ID"));

    pthread_mutex_lock(&sStackLock);
    sBleLayer.HandleUnsubscribeComplete(connObj, &svcUUID, &charUUID);
    pthread_mutex_unlock(&sStackLock);
exit:
    return;
}

JNI_ANDROID_CHIP_STACK_METHOD(void, handleConnectionError)(JNIEnv * env, jobject self, jint conn)
{
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    pthread_mutex_lock(&sStackLock);
    sBleLayer.HandleConnectionError(connObj, BLE_ERROR_APP_CLOSED_CONNECTION);
    pthread_mutex_unlock(&sStackLock);
}

JNI_METHOD(void, deleteDeviceController)(JNIEnv * env, jobject self, jlong handle)
{
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "deleteDeviceController() called");

    if (wrapper != NULL)
    {
        delete wrapper;
    }
}

void HandleNotifyChipConnectionClosed(BLE_CONNECTION_OBJECT connObj)
{
    StackUnlockGuard unlockGuard;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;
    jmethodID method;
    intptr_t tmpConnObj;

    ChipLogProgress(Controller, "Received NotifyChipConnectionClosed");

    sJVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    method = env->GetStaticMethodID(sAndroidChipStackCls, "onNotifyChipConnectionClosed", "(I)V");
    VerifyOrExit(method != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    ChipLogProgress(Controller, "Calling Java NotifyChipConnectionClosed");

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(connObj);
    env->CallStaticVoidMethod(sAndroidChipStackCls, method, static_cast<jint>(tmpConnObj));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ReportError(env, err, __FUNCTION__);
    }
    env->ExceptionClear();
}

bool HandleSendCharacteristic(BLE_CONNECTION_OBJECT connObj, const uint8_t * svcId, const uint8_t * charId,
                              const uint8_t * characteristicData, uint32_t characteristicDataLen)
{
    StackUnlockGuard unlockGuard;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;
    jbyteArray svcIdObj;
    jbyteArray charIdObj;
    jbyteArray characteristicDataObj;
    jmethodID method;
    intptr_t tmpConnObj;
    bool rc = false;

    ChipLogProgress(Controller, "Received SendCharacteristic");

    sJVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    err = N2J_ByteArray(env, svcId, 16, svcIdObj);
    SuccessOrExit(err);

    err = N2J_ByteArray(env, charId, 16, charIdObj);
    SuccessOrExit(err);

    err = N2J_ByteArray(env, characteristicData, characteristicDataLen, characteristicDataObj);
    SuccessOrExit(err);

    method = env->GetStaticMethodID(sAndroidChipStackCls, "onSendCharacteristic", "(I[B[B[B)Z");
    VerifyOrExit(method != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    ChipLogProgress(Controller, "Calling Java SendCharacteristic");

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(connObj);
    rc = (bool) env->CallStaticBooleanMethod(sAndroidChipStackCls, method, static_cast<jint>(tmpConnObj), svcIdObj, charIdObj,
                                             characteristicDataObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ReportError(env, err, __FUNCTION__);
        rc = false;
    }
    env->ExceptionClear();

    return rc;
}

bool HandleSubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const uint8_t * svcId, const uint8_t * charId)
{
    StackUnlockGuard unlockGuard;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;
    jbyteArray svcIdObj;
    jbyteArray charIdObj;
    jmethodID method;
    intptr_t tmpConnObj;
    bool rc = false;

    ChipLogProgress(Controller, "Received SubscribeCharacteristic");

    sJVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    err = N2J_ByteArray(env, svcId, 16, svcIdObj);
    SuccessOrExit(err);

    err = N2J_ByteArray(env, charId, 16, charIdObj);
    SuccessOrExit(err);

    method = env->GetStaticMethodID(sAndroidChipStackCls, "onSubscribeCharacteristic", "(I[B[B)Z");
    VerifyOrExit(method != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    ChipLogProgress(Controller, "Calling Java SubscribeCharacteristic");

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(connObj);
    rc = (bool) env->CallStaticBooleanMethod(sAndroidChipStackCls, method, static_cast<jint>(tmpConnObj), svcIdObj, charIdObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ReportError(env, err, __FUNCTION__);
        rc = false;
    }
    env->ExceptionClear();

    return rc;
}

bool HandleUnsubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const uint8_t * svcId, const uint8_t * charId)
{
    StackUnlockGuard unlockGuard;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;
    jbyteArray svcIdObj;
    jbyteArray charIdObj;
    jmethodID method;
    intptr_t tmpConnObj;
    bool rc = false;

    ChipLogProgress(Controller, "Received UnsubscribeCharacteristic");

    sJVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    err = N2J_ByteArray(env, svcId, 16, svcIdObj);
    SuccessOrExit(err);

    err = N2J_ByteArray(env, charId, 16, charIdObj);
    SuccessOrExit(err);

    method = env->GetStaticMethodID(sAndroidChipStackCls, "onUnsubscribeCharacteristic", "(I[B[B)Z");
    VerifyOrExit(method != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    ChipLogProgress(Controller, "Calling Java UnsubscribeCharacteristic");

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(connObj);
    rc = (bool) env->CallStaticBooleanMethod(sAndroidChipStackCls, method, static_cast<jint>(tmpConnObj), svcIdObj, charIdObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ReportError(env, err, __FUNCTION__);
        rc = false;
    }
    env->ExceptionClear();

    return rc;
}

bool HandleCloseConnection(BLE_CONNECTION_OBJECT connObj)
{
    StackUnlockGuard unlockGuard;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;
    jmethodID method;
    intptr_t tmpConnObj;
    bool rc = false;

    ChipLogProgress(Controller, "Received CloseConnection");

    sJVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    method = env->GetStaticMethodID(sAndroidChipStackCls, "onCloseConnection", "(I)Z");
    VerifyOrExit(method != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    ChipLogProgress(Controller, "Calling Java CloseConnection");

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(connObj);
    rc         = (bool) env->CallStaticBooleanMethod(sAndroidChipStackCls, method, static_cast<jint>(tmpConnObj));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ReportError(env, err, __FUNCTION__);
        rc = false;
    }
    env->ExceptionClear();
    return rc;
}

uint16_t HandleGetMTU(BLE_CONNECTION_OBJECT connObj)
{
    StackUnlockGuard unlockGuard;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;
    jmethodID method;
    intptr_t tmpConnObj;
    uint16_t mtu = 0;

    ChipLogProgress(Controller, "Received GetMTU");

    sJVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    method = env->GetStaticMethodID(sAndroidChipStackCls, "onGetMTU", "(I)I");
    VerifyOrExit(method != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    ChipLogProgress(Controller, "Calling Java onGetMTU");

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(connObj);
    mtu        = (int16_t) env->CallStaticIntMethod(sAndroidChipStackCls, method, static_cast<jint>(tmpConnObj));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ReportError(env, err, __FUNCTION__);
        mtu = 0;
    }
    env->ExceptionClear();

    return mtu;
}

void HandleNewConnection(void * appState, const uint16_t discriminator)
{
    StackUnlockGuard unlockGuard;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;
    jmethodID method;
    jclass deviceControllerCls;
    AndroidDeviceControllerWrapper * wrapper = reinterpret_cast<AndroidDeviceControllerWrapper *>(appState);
    jobject self                             = wrapper->JavaObjectRef();

    ChipLogProgress(Controller, "Received New Connection");

    sJVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    deviceControllerCls = env->GetObjectClass(self);
    VerifyOrExit(deviceControllerCls != NULL, err = CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    method = env->GetMethodID(deviceControllerCls, "onConnectDeviceComplete", "()V");
    VerifyOrExit(method != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    ChipLogProgress(Controller, "Calling Java onConnectDeviceComplete");

    env->ExceptionClear();
    env->CallVoidMethod(self, method);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ReportError(env, err, __FUNCTION__);
    }
    env->ExceptionClear();
}

void * IOThreadMain(void * arg)
{
    JNIEnv * env;
    JavaVMAttachArgs attachArgs;
    struct timeval sleepTime;
    fd_set readFDs, writeFDs, exceptFDs;
    int numFDs = 0;

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

    // Set to true to quit the loop. This is currently unused.
    std::atomic<bool> quit;

    ChipLogProgress(Controller, "IO thread starting");

    // Lock the stack to prevent collisions with Java threads.
    pthread_mutex_lock(&sStackLock);

    // Loop until we are told to exit.
    while (!quit.load(std::memory_order_relaxed))
    {
        numFDs = 0;
        FD_ZERO(&readFDs);
        FD_ZERO(&writeFDs);
        FD_ZERO(&exceptFDs);

        sleepTime.tv_sec  = 10;
        sleepTime.tv_usec = 0;

        // Collect the currently active file descriptors.
        sSystemLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, sleepTime);
        sInetLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, sleepTime);

        // Unlock the stack so that Java threads can make API calls.
        pthread_mutex_unlock(&sStackLock);

        // Wait for for I/O or for the next timer to expire.
        int selectRes = select(numFDs, &readFDs, &writeFDs, &exceptFDs, &sleepTime);

        // Break the loop if requested to shutdown.
        // if (sShutdown)
        // break;

        // Re-lock the stack.
        pthread_mutex_lock(&sStackLock);

        // Perform I/O and/or dispatch timers.
        sSystemLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
        sInetLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }

    // Detach the thread from the JVM.
    sJVM->DetachCurrentThread();

    return NULL;
}

void ReportError(JNIEnv * env, CHIP_ERROR cbErr, const char * functName)
{
    if (cbErr == CHIP_JNI_ERROR_EXCEPTION_THROWN)
    {
        ChipLogError(Controller, "Java exception thrown in %s", functName);
        env->ExceptionDescribe();
    }
    else
    {
        const char * errStr;
        switch (cbErr)
        {
        case CHIP_JNI_ERROR_TYPE_NOT_FOUND:
            errStr = "JNI type not found";
            break;
        case CHIP_JNI_ERROR_METHOD_NOT_FOUND:
            errStr = "JNI method not found";
            break;
        case CHIP_JNI_ERROR_FIELD_NOT_FOUND:
            errStr = "JNI field not found";
            break;
        default:
            errStr = ErrorStr(cbErr);
            break;
        }
        ChipLogError(Controller, "Error in %s : %s", functName, errStr);
    }
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

CHIP_ERROR N2J_ByteArray(JNIEnv * env, const uint8_t * inArray, uint32_t inArrayLen, jbyteArray & outArray)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    outArray = env->NewByteArray((int) inArrayLen);
    VerifyOrExit(outArray != NULL, err = CHIP_ERROR_NO_MEMORY);

    env->ExceptionClear();
    env->SetByteArrayRegion(outArray, 0, inArrayLen, (jbyte *) inArray);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    return err;
}

CHIP_ERROR N2J_Error(JNIEnv * env, CHIP_ERROR inErr, jthrowable & outEx)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    const char * errStr = NULL;
    jstring errStrObj   = NULL;
    jmethodID constructor;

    constructor = env->GetMethodID(sChipDeviceControllerExceptionCls, "<init>", "(ILjava/lang/String;)V");
    VerifyOrExit(constructor != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    switch (inErr)
    {
    case CHIP_JNI_ERROR_TYPE_NOT_FOUND:
        errStr = "CHIP Device Controller Error: JNI type not found";
        break;
    case CHIP_JNI_ERROR_METHOD_NOT_FOUND:
        errStr = "CHIP Device Controller Error: JNI method not found";
        break;
    case CHIP_JNI_ERROR_FIELD_NOT_FOUND:
        errStr = "CHIP Device Controller Error: JNI field not found";
        break;
    default:
        errStr = ErrorStr(inErr);
        break;
    }
    errStrObj = (errStr != NULL) ? env->NewStringUTF(errStr) : NULL;

    env->ExceptionClear();
    outEx = (jthrowable) env->NewObject(sChipDeviceControllerExceptionCls, constructor, (jint) inErr, errStrObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    env->DeleteLocalRef(errStrObj);
    return err;
}
