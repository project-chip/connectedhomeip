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
#include "StackLock.h"

#include <app/chip-zcl-zpro-codec.h>
#include <atomic>
#include <ble/BleUUID.h>
#include <controller/CHIPDeviceController.h>
#include <jni.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/Protocols.h>
#include <protocols/temp_zcl/TempZCL.h>
#include <pthread.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/SafeInt.h>
#include <support/ThreadOperationalDataset.h>
#include <support/logging/CHIPLogging.h>

#include <gen/CHIPClientCallbacks.h>
#include <gen/CHIPClusters.h>

// Choose an approximation of PTHREAD_NULL if pthread.h doesn't define one.
#ifndef PTHREAD_NULL
#define PTHREAD_NULL 0
#endif // PTHREAD_NULL

using namespace chip;
using namespace chip::Inet;
using namespace chip::Controller;
using namespace chip::Thread;

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
static CHIP_ERROR N2J_Error(JNIEnv * env, CHIP_ERROR inErr, jthrowable & outEx);

namespace {

constexpr EndpointId kNodeEndpoint = 0;
constexpr uint64_t kBreadcrumb     = 0;
constexpr uint32_t kZclTimeoutMs   = 10000;

JavaVM * sJVM;
System::Layer sSystemLayer;
Inet::InetLayer sInetLayer;

#if CONFIG_NETWORK_LAYER_BLE
Ble::BleLayer sBleLayer;
AndroidBleApplicationDelegate sBleApplicationDelegate;
AndroidBlePlatformDelegate sBlePlatformDelegate;
AndroidBleConnectionDelegate sBleConnectionDelegate;
#endif

pthread_t sIOThread = PTHREAD_NULL;
bool sShutdown      = false;

jclass sAndroidChipStackCls              = NULL;
jclass sChipDeviceControllerExceptionCls = NULL;

} // namespace

// NOTE: Remote device ID is in sync with the echo server device id
// At some point, we may want to add an option to connect to a device without
// knowing its id, because the ID can be learned on the first response that is received.
chip::NodeId kLocalDeviceId  = chip::kTestControllerNodeId;
chip::NodeId kRemoteDeviceId = chip::kTestDeviceNodeId;

#if CONFIG_NETWORK_LAYER_BLE

chip::Ble::BleLayer * GetJNIBleLayer()
{
    return &sBleLayer;
}

#endif

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;
    int pthreadErr = 0;

    ChipLogProgress(Controller, "JNI_OnLoad() called");

    chip::Platform::MemoryInit();

    // Save a reference to the JVM.  Will need this to call back into Java.
    JniReferences::GetInstance().SetJavaVm(jvm);
    sJVM = jvm;

    // Get a JNI environment object.
    env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().InitializeMethodForward(sJVM, env);

    ChipLogProgress(Controller, "Loading Java class references.");

    // Get various class references need by the API.
    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/AndroidChipStack", sAndroidChipStackCls);
    SuccessOrExit(err);
    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/ChipDeviceControllerException",
                                                   sChipDeviceControllerExceptionCls);
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
        StackUnlockGuard unlockGuard(JniReferences::GetInstance().GetStackLock());
        JNI_OnUnload(jvm, reserved);
    }

    return (err == CHIP_NO_ERROR) ? JNI_VERSION_1_6 : JNI_ERR;
}

void JNI_OnUnload(JavaVM * jvm, void * reserved)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    ChipLogProgress(Controller, "JNI_OnUnload() called");

    // If the IO thread has been started, shut it down and wait for it to exit.
    if (sIOThread != PTHREAD_NULL)
    {
        sShutdown = true;
        sSystemLayer.WakeIOThread();

        StackUnlockGuard unlockGuard(JniReferences::GetInstance().GetStackLock());
        pthread_join(sIOThread, NULL);
    }

#if CONFIG_NETWORK_LAYER_BLE
    sBleLayer.Shutdown();
#endif

    sInetLayer.Shutdown();
    sSystemLayer.Shutdown();
    sJVM = NULL;

    chip::Platform::MemoryShutdown();
}

JNI_METHOD(jlong, newDeviceController)(JNIEnv * env, jobject self)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = NULL;
    long result                              = 0;

    ChipLogProgress(Controller, "newDeviceController() called");

    wrapper = AndroidDeviceControllerWrapper::AllocateNew(sJVM, self, JniReferences::GetInstance().GetStackLock(), kLocalDeviceId,
                                                          &sSystemLayer, &sInetLayer, &err);
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

JNI_METHOD(void, pairDevice)
(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jint connObj, jlong pinCode, jbyteArray csrNonce)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "pairDevice() called with device ID, connection object, and pincode");

    sBleLayer.mAppState         = (void *) self;
    RendezvousParameters params = RendezvousParameters()
                                      .SetSetupPINCode(pinCode)
                                      .SetConnectionObject(reinterpret_cast<BLE_CONNECTION_OBJECT>(connObj))
                                      .SetBleLayer(&sBleLayer)
                                      .SetPeerAddress(Transport::PeerAddress::BLE());
    if (csrNonce != nullptr)
    {
        JniByteArray jniCsrNonce(env, csrNonce);
        params = params.SetCSRNonce(jniCsrNonce.byteSpan());
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
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
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
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
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

JNI_METHOD(jlong, getDevicePointer)(JNIEnv * env, jobject self, jlong handle, jlong deviceId)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    Device * chipDevice = nullptr;

    ChipLogProgress(Controller, "getDevicePointer() called with device ID");

    GetCHIPDevice(env, handle, deviceId, &chipDevice);

    static_assert(sizeof(jlong) >= sizeof(void *), "Need to store a pointer in a java handle");
    return reinterpret_cast<jlong>(chipDevice);
}

JNI_METHOD(void, pairTestDeviceWithoutSecurity)(JNIEnv * env, jobject self, jlong handle, jstring deviceAddr)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
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
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    Device * chipDevice                      = nullptr;

    ChipLogProgress(Controller, "disconnectDevice() called with deviceId");

    err = wrapper->Controller()->GetDevice(deviceId, &chipDevice);

    if (err != CHIP_NO_ERROR || !chipDevice)
    {
        ChipLogError(Controller, "Failed to get paired device.");
        ThrowError(env, err);
    }

    wrapper->Controller()->ReleaseDevice(chipDevice);
}

JNI_METHOD(jboolean, isActive)(JNIEnv * env, jobject self, jlong handle)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());

    Device * chipDevice = reinterpret_cast<Device *>(handle);
    return chipDevice->IsActive();
}

void GetCHIPDevice(JNIEnv * env, long wrapperHandle, uint64_t deviceId, Device ** chipDevice)
{
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(wrapperHandle);
    CHIP_ERROR err                           = CHIP_NO_ERROR;

    err = wrapper->Controller()->GetDevice(deviceId, chipDevice);

    if (err != CHIP_NO_ERROR || !chipDevice)
    {
        ChipLogError(Controller, "Failed to get paired device.");
        ThrowError(env, err);
    }
}

JNI_METHOD(jstring, getIpAddress)(JNIEnv * env, jobject self, jlong handle, jlong deviceId)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    Device * chipDevice = nullptr;

    GetCHIPDevice(env, handle, deviceId, &chipDevice);

    chip::Inet::IPAddress addr;
    uint16_t port;
    char addrStr[50];

    if (!chipDevice->GetAddress(addr, port))
        return nullptr;

    addr.ToString(addrStr);
    return env->NewStringUTF(addrStr);
}

JNI_METHOD(void, updateAddress)(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jstring address, jint port)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    Device * chipDevice = nullptr;
    CHIP_ERROR err      = CHIP_NO_ERROR;

    GetCHIPDevice(env, handle, deviceId, &chipDevice);

    Inet::IPAddress ipAddress = {};
    JniUtfString addressAccessor(env, address);
    VerifyOrExit(Inet::IPAddress::FromString(addressAccessor.c_str(), ipAddress), err = CHIP_ERROR_INVALID_ADDRESS);
    VerifyOrExit(CanCastTo<uint16_t>(port), err = CHIP_ERROR_INVALID_ADDRESS);

    err = chipDevice->UpdateAddress(Transport::PeerAddress::UDP(ipAddress, port));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to update address");
        ThrowError(env, err);
    }
}

JNI_METHOD(void, sendMessage)(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jstring messageObj)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    CHIP_ERROR err      = CHIP_NO_ERROR;
    Device * chipDevice = nullptr;

    ChipLogProgress(Controller, "sendMessage() called with device id and message object");

    GetCHIPDevice(env, handle, deviceId, &chipDevice);

    const char * messageStr = env->GetStringUTFChars(messageObj, 0);
    size_t messageLen       = strlen(messageStr);

    System::PacketBufferHandle buffer = System::PacketBufferHandle::NewWithData(messageStr, messageLen);
    if (buffer.IsNull())
    {
        buffer = System::PacketBufferHandle::NewWithData(messageStr, messageLen);
        if (buffer.IsNull())
        {
            err = CHIP_ERROR_NO_MEMORY;
        }
        else
        {
            err = chipDevice->SendMessage(Protocols::TempZCL::MsgType::TempZCLRequest, std::move(buffer));
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
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    CHIP_ERROR err      = CHIP_NO_ERROR;
    Device * chipDevice = nullptr;

    GetCHIPDevice(env, handle, deviceId, &chipDevice);

    ChipLogProgress(Controller, "sendCommand() called");

    jclass commandCls         = env->GetObjectClass(commandObj);
    jmethodID commandMethodID = env->GetMethodID(commandCls, "getValue", "()I");
    jint commandID            = env->CallIntMethod(commandObj, commandMethodID);

    System::PacketBufferHandle buffer;

    switch (commandID)
    {
    default:
        ChipLogError(Controller, "Unknown command: %d", commandID);
        err = CHIP_ERROR_NOT_IMPLEMENTED;
        break;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to send CHIP command.");
        ThrowError(env, err);
    }
}

namespace {

void OnAddNetworkResponse(void * context, uint8_t errorCode, uint8_t * debugText);
void OnEnableNetworkResponse(void * context, uint8_t errorCode, uint8_t * debugText);
void OnNetworkCommissioningFailed(void * context, uint8_t errorCode);

// Context used for processing "AddThreadNetwork" and "EnableNetwork" commands.
struct NetworkCommissioningCtx
{
    static constexpr size_t kMaxNetworkIDLen = 32;

    NetworkCommissioningCtx(JNIEnv * env, jlong handle, uint64_t deviceID, ByteSpan networkID) :
        mEnv(env), mHandle(handle), mDeviceID(deviceID)
    {
        VerifyOrReturn(networkID.size() <= sizeof(mNetworkID), ThrowError(env, CHIP_ERROR_BUFFER_TOO_SMALL));
        memcpy(mNetworkID, networkID.data(), networkID.size());
        mNetworkIDLen = networkID.size();
    }

    JNIEnv * mEnv;
    jlong mHandle;
    uint64_t mDeviceID;
    uint8_t mNetworkID[kMaxNetworkIDLen];
    size_t mNetworkIDLen;
    Callback::Callback<NetworkCommissioningClusterAddThreadNetworkResponseCallback> mOnAddNetwork{ OnAddNetworkResponse, this };
    Callback::Callback<NetworkCommissioningClusterEnableNetworkResponseCallback> mOnEnableNetwork{ OnEnableNetworkResponse, this };
    Callback::Callback<DefaultFailureCallback> mOnCommissioningFailed{ OnNetworkCommissioningFailed, this };
};

void FinishCommissioning(NetworkCommissioningCtx * ctx, CHIP_ERROR err)
{
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(ctx->mHandle);
    wrapper->CallJavaMethod("onNetworkCommissioningComplete", static_cast<jint>(err));
    delete ctx;
}

void OnAddNetworkResponse(void * context, uint8_t errorCode, uint8_t * debugText)
{
    NetworkCommissioningCtx * ctx            = static_cast<NetworkCommissioningCtx *>(context);
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(ctx->mHandle);
    CHIP_ERROR err                           = CHIP_NO_ERROR;
    Device * chipDevice                      = nullptr;
    NetworkCommissioningCluster cluster;

    SuccessOrExit(err = wrapper->Controller()->GetDevice(ctx->mDeviceID, &chipDevice));

    cluster.Associate(chipDevice, kNodeEndpoint);
    err = cluster.EnableNetwork(ctx->mOnEnableNetwork.Cancel(), ctx->mOnCommissioningFailed.Cancel(),
                                ByteSpan(ctx->mNetworkID, ctx->mNetworkIDLen), kBreadcrumb, kZclTimeoutMs);

exit:
    if (err != CHIP_NO_ERROR)
    {
        FinishCommissioning(ctx, err);
    }
}

void OnEnableNetworkResponse(void * context, uint8_t errorCode, uint8_t * debugText)
{
    NetworkCommissioningCtx * ctx = static_cast<NetworkCommissioningCtx *>(context);
    FinishCommissioning(ctx, CHIP_NO_ERROR);
}

void OnNetworkCommissioningFailed(void * context, uint8_t errorCode)
{
    NetworkCommissioningCtx * ctx = static_cast<NetworkCommissioningCtx *>(context);
    FinishCommissioning(ctx, CHIP_ERROR_INTERNAL);
}

} // namespace

JNI_METHOD(void, enableThreadNetwork)
(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jbyteArray operationalDataset)
{
    CHIP_ERROR err             = CHIP_NO_ERROR;
    Device * chipDevice        = nullptr;
    OperationalDataset dataset = {};
    JniByteArray datasetAccessor(env, operationalDataset);
    size_t datasetLength = datasetAccessor.size();
    uint8_t datasetBytes[kSizeOperationalDataset];
    uint8_t extPanId[kSizeExtendedPanId];

    VerifyOrExit(datasetLength <= sizeof(datasetBytes), err = CHIP_ERROR_INVALID_ARGUMENT);
    memcpy(datasetBytes, datasetAccessor.data(), datasetLength);
    SuccessOrExit(err = dataset.Init(ByteSpan(datasetBytes, datasetLength)));
    SuccessOrExit(err = dataset.GetExtendedPanId(extPanId));
    GetCHIPDevice(env, handle, deviceId, &chipDevice);

    {
        auto ctx = std::make_unique<NetworkCommissioningCtx>(env, handle, deviceId, ByteSpan(extPanId, sizeof(extPanId)));
        StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
        NetworkCommissioningCluster cluster;
        cluster.Associate(chipDevice, kNodeEndpoint);
        SuccessOrExit(err = cluster.AddThreadNetwork(ctx->mOnAddNetwork.Cancel(), ctx->mOnCommissioningFailed.Cancel(),
                                                     dataset.AsByteSpan(), kBreadcrumb, kZclTimeoutMs));
        ctx.release();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to enable Thread network");
        ThrowError(env, err);
    }
}

JNI_METHOD(jboolean, openPairingWindow)(JNIEnv * env, jobject self, jlong handle, jlong deviceId, jint duration)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    CHIP_ERROR err      = CHIP_NO_ERROR;
    Device * chipDevice = nullptr;
    chip::SetupPayload setupPayload;

    GetCHIPDevice(env, handle, deviceId, &chipDevice);

    err = chipDevice->OpenPairingWindow(duration, chip::Controller::Device::PairingWindowOption::kOriginalSetupCode, setupPayload);

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
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
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

    sBleLayer.HandleIndicationReceived(connObj, &svcUUID, &charUUID, std::move(buffer));
exit:
    env->ReleaseByteArrayElements(value, valueBegin, 0);
}

JNI_ANDROID_CHIP_STACK_METHOD(void, handleWriteConfirmation)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    VerifyOrReturn(JavaBytesToUUID(env, svcId, svcUUID),
                   ChipLogError(Controller, "handleWriteConfirmation() called with invalid service ID"));
    VerifyOrReturn(JavaBytesToUUID(env, charId, charUUID),
                   ChipLogError(Controller, "handleWriteConfirmation() called with invalid characteristic ID"));

    sBleLayer.HandleWriteConfirmation(connObj, &svcUUID, &charUUID);
}

JNI_ANDROID_CHIP_STACK_METHOD(void, handleSubscribeComplete)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    VerifyOrReturn(JavaBytesToUUID(env, svcId, svcUUID),
                   ChipLogError(Controller, "handleSubscribeComplete() called with invalid service ID"));
    VerifyOrReturn(JavaBytesToUUID(env, charId, charUUID),
                   ChipLogError(Controller, "handleSubscribeComplete() called with invalid characteristic ID"));

    sBleLayer.HandleSubscribeComplete(connObj, &svcUUID, &charUUID);
}

JNI_ANDROID_CHIP_STACK_METHOD(void, handleUnsubscribeComplete)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    VerifyOrReturn(JavaBytesToUUID(env, svcId, svcUUID),
                   ChipLogError(Controller, "handleUnsubscribeComplete() called with invalid service ID"));
    VerifyOrReturn(JavaBytesToUUID(env, charId, charUUID),
                   ChipLogError(Controller, "handleUnsubscribeComplete() called with invalid characteristic ID"));

    sBleLayer.HandleUnsubscribeComplete(connObj, &svcUUID, &charUUID);
}

JNI_ANDROID_CHIP_STACK_METHOD(void, handleConnectionError)(JNIEnv * env, jobject self, jint conn)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    sBleLayer.HandleConnectionError(connObj, BLE_ERROR_APP_CLOSED_CONNECTION);
}

JNI_METHOD(void, deleteDeviceController)(JNIEnv * env, jobject self, jlong handle)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    AndroidDeviceControllerWrapper * wrapper = AndroidDeviceControllerWrapper::FromJNIHandle(handle);

    ChipLogProgress(Controller, "deleteDeviceController() called");

    if (wrapper != NULL)
    {
        delete wrapper;
    }
}

void HandleNotifyChipConnectionClosed(BLE_CONNECTION_OBJECT connObj)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID method;
    intptr_t tmpConnObj;

    ChipLogProgress(Controller, "Received NotifyChipConnectionClosed");
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    method = env->GetStaticMethodID(sAndroidChipStackCls, "onNotifyChipConnectionClosed", "(I)V");
    VerifyOrExit(method != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    ChipLogProgress(Controller, "Calling Java NotifyChipConnectionClosed");

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(connObj);
    {
        StackUnlockGuard unlockGuard(JniReferences::GetInstance().GetStackLock());
        env->CallStaticVoidMethod(sAndroidChipStackCls, method, static_cast<jint>(tmpConnObj));
    }
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    jbyteArray svcIdObj;
    jbyteArray charIdObj;
    jbyteArray characteristicDataObj;
    jmethodID method;
    intptr_t tmpConnObj;
    bool rc = false;

    ChipLogProgress(Controller, "Received SendCharacteristic");
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    err = JniReferences::GetInstance().N2J_ByteArray(env, svcId, 16, svcIdObj);
    SuccessOrExit(err);

    err = JniReferences::GetInstance().N2J_ByteArray(env, charId, 16, charIdObj);
    SuccessOrExit(err);

    err = JniReferences::GetInstance().N2J_ByteArray(env, characteristicData, characteristicDataLen, characteristicDataObj);
    SuccessOrExit(err);

    method = env->GetStaticMethodID(sAndroidChipStackCls, "onSendCharacteristic", "(I[B[B[B)Z");
    VerifyOrExit(method != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    ChipLogProgress(Controller, "Calling Java SendCharacteristic");

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(connObj);
    {
        StackUnlockGuard unlockGuard(JniReferences::GetInstance().GetStackLock());
        rc = (bool) env->CallStaticBooleanMethod(sAndroidChipStackCls, method, static_cast<jint>(tmpConnObj), svcIdObj, charIdObj,
                                                 characteristicDataObj);
    }
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    jbyteArray svcIdObj;
    jbyteArray charIdObj;
    jmethodID method;
    intptr_t tmpConnObj;
    bool rc = false;

    ChipLogProgress(Controller, "Received SubscribeCharacteristic");
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    err = JniReferences::GetInstance().N2J_ByteArray(env, svcId, 16, svcIdObj);
    SuccessOrExit(err);

    err = JniReferences::GetInstance().N2J_ByteArray(env, charId, 16, charIdObj);
    SuccessOrExit(err);

    {
        StackUnlockGuard unlockGuard(JniReferences::GetInstance().GetStackLock());
        method = env->GetStaticMethodID(sAndroidChipStackCls, "onSubscribeCharacteristic", "(I[B[B)Z");
    }
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    jbyteArray svcIdObj;
    jbyteArray charIdObj;
    jmethodID method;
    intptr_t tmpConnObj;
    bool rc = false;

    ChipLogProgress(Controller, "Received UnsubscribeCharacteristic");
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    err = JniReferences::GetInstance().N2J_ByteArray(env, svcId, 16, svcIdObj);
    SuccessOrExit(err);

    err = JniReferences::GetInstance().N2J_ByteArray(env, charId, 16, charIdObj);
    SuccessOrExit(err);

    method = env->GetStaticMethodID(sAndroidChipStackCls, "onUnsubscribeCharacteristic", "(I[B[B)Z");
    VerifyOrExit(method != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    ChipLogProgress(Controller, "Calling Java UnsubscribeCharacteristic");

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(connObj);
    {
        StackUnlockGuard unlockGuard(JniReferences::GetInstance().GetStackLock());
        rc = (bool) env->CallStaticBooleanMethod(sAndroidChipStackCls, method, static_cast<jint>(tmpConnObj), svcIdObj, charIdObj);
    }
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID method;
    intptr_t tmpConnObj;
    bool rc = false;

    ChipLogProgress(Controller, "Received CloseConnection");
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    method = env->GetStaticMethodID(sAndroidChipStackCls, "onCloseConnection", "(I)Z");
    VerifyOrExit(method != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    ChipLogProgress(Controller, "Calling Java CloseConnection");

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(connObj);
    {
        StackUnlockGuard unlockGuard(JniReferences::GetInstance().GetStackLock());
        rc = (bool) env->CallStaticBooleanMethod(sAndroidChipStackCls, method, static_cast<jint>(tmpConnObj));
    }
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID method;
    intptr_t tmpConnObj;
    uint16_t mtu = 0;

    ChipLogProgress(Controller, "Received GetMTU");
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    {
        StackUnlockGuard unlockGuard(JniReferences::GetInstance().GetStackLock());
        method = env->GetStaticMethodID(sAndroidChipStackCls, "onGetMTU", "(I)I");
    }
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID method;
    jclass deviceControllerCls;
    AndroidDeviceControllerWrapper * wrapper = reinterpret_cast<AndroidDeviceControllerWrapper *>(appState);
    jobject self                             = wrapper->JavaObjectRef();

    ChipLogProgress(Controller, "Received New Connection");
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    deviceControllerCls = env->GetObjectClass(self);
    VerifyOrExit(deviceControllerCls != NULL, err = CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    method = env->GetMethodID(deviceControllerCls, "onConnectDeviceComplete", "()V");
    VerifyOrExit(method != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    ChipLogProgress(Controller, "Calling Java onConnectDeviceComplete");

    env->ExceptionClear();
    {
        StackUnlockGuard unlockGuard(JniReferences::GetInstance().GetStackLock());
        env->CallVoidMethod(self, method);
    }
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
    pthread_mutex_lock(JniReferences::GetInstance().GetStackLock());

    System::WatchableEventManager & watchState = sSystemLayer.WatchableEvents();
    watchState.EventLoopBegins();

    // Loop until we are told to exit.
    while (!quit.load(std::memory_order_relaxed))
    {
        // TODO(#5556): add a timer for `sleepTime.tv_sec  = 10; sleepTime.tv_usec = 0;`
        watchState.PrepareEvents();

        // Unlock the stack so that Java threads can make API calls.
        pthread_mutex_unlock(JniReferences::GetInstance().GetStackLock());

        watchState.WaitForEvents();

        // Break the loop if requested to shutdown.
        // if (sShutdown)
        // break;

        // Re-lock the stack.
        pthread_mutex_lock(JniReferences::GetInstance().GetStackLock());

        watchState.HandleEvents();
    }
    watchState.EventLoopEnds();

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

CHIP_ERROR N2J_Error(JNIEnv * env, CHIP_ERROR inErr, jthrowable & outEx)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    const char * errStr = NULL;
    jstring errStrObj   = NULL;
    jmethodID constructor;

    env->ExceptionClear();
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
    case CHIP_JNI_ERROR_DEVICE_NOT_FOUND:
        errStr = "CHIP Device Controller Error: Device not found";
        break;
    default:
        errStr = ErrorStr(inErr);
        break;
    }
    errStrObj = (errStr != NULL) ? env->NewStringUTF(errStr) : NULL;

    outEx = (jthrowable) env->NewObject(sChipDeviceControllerExceptionCls, constructor, (jint) inErr, errStrObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    env->DeleteLocalRef(errStrObj);
    return err;
}
