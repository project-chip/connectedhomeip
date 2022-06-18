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
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConfigurationManager.h>
#include <platform/ConnectivityManager.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/internal/BLEManager.h>
#include <trace/trace.h>

#include "AndroidChipPlatform-JNI.h"
#include "BLEManagerImpl.h"
#include "CommissionableDataProviderImpl.h"
#include "DiagnosticDataProviderImpl.h"
#include "DnssdImpl.h"

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_chip_platform_AndroidChipPlatform_##METHOD_NAME
#define JNI_MDNSCALLBACK_METHOD(RETURN, METHOD_NAME)                                                                               \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_platform_ChipMdnsCallbackImpl_##METHOD_NAME

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
static bool JavaBytesToUUID(JNIEnv * env, jbyteArray value, chip::Ble::ChipBleUUID & uuid);
#endif

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

    chip::InitializeTracing();

exit:
    if (err != CHIP_NO_ERROR)
    {
        JniReferences::GetInstance().ThrowError(env, sAndroidChipPlatformExceptionCls, err);
        JNI_OnUnload(jvm, reserved);
    }

    return err;
}

void AndroidChipPlatformJNI_OnUnload(JavaVM * jvm, void * reserved)
{
    ChipLogProgress(DeviceLayer, "AndroidChipPlatform JNI_OnUnload() called");
    chip::Platform::MemoryShutdown();
}

JNI_METHOD(void, initChipStack)(JNIEnv * env, jobject self)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "Error initializing CHIP stack: %s", ErrorStr(err)));
}

// for BLEManager
JNI_METHOD(void, nativeSetBLEManager)(JNIEnv *, jobject, jobject manager)
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    chip::DeviceLayer::StackLock lock;
    chip::DeviceLayer::Internal::BLEMgrImpl().InitializeWithObject(manager);
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
}

JNI_METHOD(void, handleWriteConfirmation)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId)
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    chip::DeviceLayer::StackLock lock;
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    VerifyOrReturn(JavaBytesToUUID(env, svcId, svcUUID),
                   ChipLogError(DeviceLayer, "handleWriteConfirmation() called with invalid service ID"));
    VerifyOrReturn(JavaBytesToUUID(env, charId, charUUID),
                   ChipLogError(DeviceLayer, "handleWriteConfirmation() called with invalid characteristic ID"));

    chip::DeviceLayer::Internal::BLEMgrImpl().HandleWriteConfirmation(connObj, &svcUUID, &charUUID);
#endif
}

JNI_METHOD(void, handleIndicationReceived)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId, jbyteArray value)
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    chip::DeviceLayer::StackLock lock;
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
#endif
}

JNI_METHOD(void, handleSubscribeComplete)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId)
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    chip::DeviceLayer::StackLock lock;
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    VerifyOrReturn(JavaBytesToUUID(env, svcId, svcUUID),
                   ChipLogError(DeviceLayer, "handleSubscribeComplete() called with invalid service ID"));
    VerifyOrReturn(JavaBytesToUUID(env, charId, charUUID),
                   ChipLogError(DeviceLayer, "handleSubscribeComplete() called with invalid characteristic ID"));

    chip::DeviceLayer::Internal::BLEMgrImpl().HandleSubscribeComplete(connObj, &svcUUID, &charUUID);
#endif
}

JNI_METHOD(void, handleUnsubscribeComplete)
(JNIEnv * env, jobject self, jint conn, jbyteArray svcId, jbyteArray charId)
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    chip::DeviceLayer::StackLock lock;
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::Ble::ChipBleUUID svcUUID;
    chip::Ble::ChipBleUUID charUUID;
    VerifyOrReturn(JavaBytesToUUID(env, svcId, svcUUID),
                   ChipLogError(DeviceLayer, "handleUnsubscribeComplete() called with invalid service ID"));
    VerifyOrReturn(JavaBytesToUUID(env, charId, charUUID),
                   ChipLogError(DeviceLayer, "handleUnsubscribeComplete() called with invalid characteristic ID"));

    chip::DeviceLayer::Internal::BLEMgrImpl().HandleUnsubscribeComplete(connObj, &svcUUID, &charUUID);
#endif
}

JNI_METHOD(void, handleConnectionError)(JNIEnv * env, jobject self, jint conn)
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    chip::DeviceLayer::StackLock lock;
    BLE_CONNECTION_OBJECT const connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(conn);

    chip::DeviceLayer::Internal::BLEMgrImpl().HandleConnectionError(connObj, BLE_ERROR_APP_CLOSED_CONNECTION);
#endif
}

// for KeyValueStoreManager
JNI_METHOD(void, setKeyValueStoreManager)(JNIEnv * env, jclass self, jobject manager)
{
    chip::DeviceLayer::StackLock lock;
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().InitializeWithObject(manager);
}

// for ConfigurationManager
JNI_METHOD(void, setConfigurationManager)(JNIEnv * env, jclass self, jobject manager)
{
    chip::DeviceLayer::StackLock lock;
    chip::DeviceLayer::ConfigurationManagerImpl::GetDefaultInstance().InitializeWithObject(manager);
}

// for DiagnosticDataProviderManager
JNI_METHOD(void, setDiagnosticDataProviderManager)(JNIEnv * env, jclass self, jobject manager)
{
    chip::DeviceLayer::StackLock lock;
    chip::DeviceLayer::DiagnosticDataProviderImpl::GetDefaultInstance().InitializeWithObject(manager);
}

// for ServiceResolver and  ServiceBrowser
JNI_METHOD(void, nativeSetDnssdDelegates)(JNIEnv * env, jclass self, jobject resolver, jobject browser, jobject chipMdnsCallback)
{
    chip::DeviceLayer::StackLock lock;
    chip::Dnssd::InitializeWithObjects(resolver, browser, chipMdnsCallback);
}

JNI_MDNSCALLBACK_METHOD(void, handleServiceResolve)
(JNIEnv * env, jclass self, jstring instanceName, jstring serviceType, jstring hostName, jstring address, jint port,
 jobject attributes, jlong callbackHandle, jlong contextHandle)
{
    using ::chip::Dnssd::HandleResolve;
    HandleResolve(instanceName, serviceType, hostName, address, port, attributes, callbackHandle, contextHandle);
}

JNI_MDNSCALLBACK_METHOD(void, handleServiceBrowse)
(JNIEnv * env, jclass self, jobjectArray instanceName, jstring serviceType, jlong callbackHandle, jlong contextHandle)
{
    using ::chip::Dnssd::HandleBrowse;
    HandleBrowse(instanceName, serviceType, callbackHandle, contextHandle);
}

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
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
#endif

// for CommissionableDataProvider
JNI_METHOD(jboolean, updateCommissionableDataProviderData)
(JNIEnv * env, jclass self, jstring spake2pVerifierBase64, jstring Spake2pSaltBase64, jint spake2pIterationCount,
 jlong setupPasscode, jint discriminator)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = CommissionableDataProviderMgrImpl().Update(env, spake2pVerifierBase64, Spake2pSaltBase64,
                                                                spake2pIterationCount, setupPasscode, discriminator);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to update commissionable data provider data: %s", ErrorStr(err));
        return false;
    }

    return true;
}
