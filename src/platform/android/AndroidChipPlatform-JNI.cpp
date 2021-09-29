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
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/StackLock.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConfigurationManager.h>
#include <platform/ConnectivityManager.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/internal/BLEManager.h>

#include "AndroidChipPlatform-JNI.h"
#include "BLEManagerImpl.h"
#include "MdnsImpl.h"

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_chip_platform_AndroidChipPlatform_##METHOD_NAME
#define JNI_MDNSCALLBACK_METHOD(RETURN, METHOD_NAME)                                                                               \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_platform_ChipMdnsCallbackImpl_##METHOD_NAME

static void ThrowError(JNIEnv * env, CHIP_ERROR errToThrow);
static CHIP_ERROR N2J_Error(JNIEnv * env, CHIP_ERROR inErr, jthrowable & outEx);
static bool JavaBytesToUUID(JNIEnv * env, jbyteArray value, chip::Ble::ChipBleUUID & uuid);

namespace {
JavaVM * sJVM;
jclass sAndroidChipPlatformCls          = NULL;
jclass sAndroidChipPlatformExceptionCls = NULL;
} // namespace

CHIP_ERROR AndroidChipPlatformJNI_OnLoad(JavaVM * jvm, void * reserved)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;

    ChipLogProgress(DeviceLayer, "AndroidChipPlatform JNI_OnLoad() called");

    chip::Platform::MemoryInit();

    // Save a reference to the JVM.  Will need this to call back into Java.
    JniReferences::GetInstance().SetJavaVm(jvm, "chip/platform/AndroidChipPlatform");
    sJVM = jvm;

    // Get a JNI environment object.
    env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    ChipLogProgress(DeviceLayer, "Loading Java class references.");

    // Get various class references need by the API.
    err = JniReferences::GetInstance().GetClassRef(env, "chip/platform/AndroidChipPlatform", sAndroidChipPlatformCls);
    SuccessOrExit(err);
    err = JniReferences::GetInstance().GetClassRef(env, "chip/platform/AndroidChipPlatformException",
                                                   sAndroidChipPlatformExceptionCls);
    SuccessOrExit(err);
    ChipLogProgress(DeviceLayer, "Java class references loaded.");

exit:
    if (err != CHIP_NO_ERROR)
    {
        ThrowError(env, err);
        JNI_OnUnload(jvm, reserved);
    }

    return err;
}

void AndroidChipPlatformJNI_OnUnload(JavaVM * jvm, void * reserved)
{
    ChipLogProgress(DeviceLayer, "AndroidChipPlatform JNI_OnUnload() called");
    chip::Platform::MemoryShutdown();
}

// for BLEManager
JNI_METHOD(void, nativeSetBLEManager)(JNIEnv *, jobject, jobject manager)
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    chip::DeviceLayer::Internal::BLEMgrImpl().InitializeWithObject(manager);
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
}

JNI_METHOD(void, handleWriteConfirmation)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId)
{
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    VerifyOrReturn(JavaBytesToUUID(env, svcId, svcUUID),
                   ChipLogError(DeviceLayer, "handleWriteConfirmation() called with invalid service ID"));
    VerifyOrReturn(JavaBytesToUUID(env, charId, charUUID),
                   ChipLogError(DeviceLayer, "handleWriteConfirmation() called with invalid characteristic ID"));

    chip::DeviceLayer::Internal::BLEMgrImpl().HandleWriteConfirmation(connObj, &svcUUID, &charUUID);
}

JNI_METHOD(void, handleIndicationReceived)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId, jbyteArray value)
{
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);
    const auto valueBegin               = env->GetByteArrayElements(value, nullptr);
    const auto valueLength              = env->GetArrayLength(value);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    chip::System::PacketBufferHandle buffer;

    VerifyOrExit(JavaBytesToUUID(env, svcId, svcUUID),
                 ChipLogError(DeviceLayer, "handleIndicationReceived() called with invalid service ID"));
    VerifyOrExit(JavaBytesToUUID(env, charId, charUUID),
                 ChipLogError(DeviceLayer, "handleIndicationReceived() called with invalid characteristic ID"));

    buffer = System::PacketBufferHandle::NewWithData(valueBegin, valueLength);
    VerifyOrExit(!buffer.IsNull(), ChipLogError(DeviceLayer, "Failed to allocate packet buffer"));

    chip::DeviceLayer::Internal::BLEMgrImpl().HandleIndicationReceived(connObj, &svcUUID, &charUUID, std::move(buffer));
exit:
    env->ReleaseByteArrayElements(value, valueBegin, 0);
}

JNI_METHOD(void, handleSubscribeComplete)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId)
{
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    VerifyOrReturn(JavaBytesToUUID(env, svcId, svcUUID),
                   ChipLogError(DeviceLayer, "handleSubscribeComplete() called with invalid service ID"));
    VerifyOrReturn(JavaBytesToUUID(env, charId, charUUID),
                   ChipLogError(DeviceLayer, "handleSubscribeComplete() called with invalid characteristic ID"));

    chip::DeviceLayer::Internal::BLEMgrImpl().HandleSubscribeComplete(connObj, &svcUUID, &charUUID);
}

JNI_METHOD(void, handleUnsubscribeComplete)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId)
{
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    VerifyOrReturn(JavaBytesToUUID(env, svcId, svcUUID),
                   ChipLogError(DeviceLayer, "handleUnsubscribeComplete() called with invalid service ID"));
    VerifyOrReturn(JavaBytesToUUID(env, charId, charUUID),
                   ChipLogError(DeviceLayer, "handleUnsubscribeComplete() called with invalid characteristic ID"));

    chip::DeviceLayer::Internal::BLEMgrImpl().HandleUnsubscribeComplete(connObj, &svcUUID, &charUUID);
}

JNI_METHOD(void, handleConnectionError)(JNIEnv * env, jobject self, jint conn)
{
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::DeviceLayer::Internal::BLEMgrImpl().HandleConnectionError(connObj, BLE_ERROR_APP_CLOSED_CONNECTION);
}

// for KeyValueStoreManager
JNI_METHOD(void, setKeyValueStoreManager)(JNIEnv * env, jclass self, jobject manager)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().InitializeWithObject(manager);
}

// for ConfigurationManager
JNI_METHOD(void, setConfigurationManager)(JNIEnv * env, jclass self, jobject manager)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    chip::DeviceLayer::ConfigurationMgrImpl().InitializeWithObject(manager);
}

// for ServiceResolver
JNI_METHOD(void, nativeSetServiceResolver)(JNIEnv * env, jclass self, jobject resolver, jobject chipMdnsCallback)
{
    StackLockGuard lock(JniReferences::GetInstance().GetStackLock());
    chip::Mdns::InitializeWithObjects(resolver, chipMdnsCallback);
}

JNI_MDNSCALLBACK_METHOD(void, handleServiceResolve)
(JNIEnv * env, jclass self, jstring instanceName, jstring serviceType, jstring address, jint port, jlong callbackHandle,
 jlong contextHandle)
{
    using ::chip::Mdns::HandleResolve;
    HandleResolve(instanceName, serviceType, address, port, callbackHandle, contextHandle);
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
    constructor = env->GetMethodID(sAndroidChipPlatformExceptionCls, "<init>", "(ILjava/lang/String;)V");
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

    outEx =
        (jthrowable) env->NewObject(sAndroidChipPlatformExceptionCls, constructor, static_cast<jint>(inErr.AsInteger()), errStrObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    env->DeleteLocalRef(errStrObj);
    return err;
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
