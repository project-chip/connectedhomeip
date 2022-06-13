/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          Utilities for interacting with multiple file partitions and maps
 *          key-value config calls to the correct partition.
 */

#include <cstdint>
#include <cstring>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniTypeWrappers.h>
#include <platform/android/AndroidConfig.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

static jobject gAndroidConfigObject          = nullptr;
static jmethodID gReadConfigValueLongMethod  = nullptr;
static jmethodID gReadConfigValueStrMethod   = nullptr;
static jmethodID gReadConfigValueBinMethod   = nullptr;
static jmethodID gWriteConfigValueLongMethod = nullptr;
static jmethodID gWriteConfigValueStrMethod  = nullptr;
static jmethodID gWriteConfigValueBinMethod  = nullptr;
static jmethodID gClearConfigValueMethod     = nullptr;
static jmethodID gConfigValueExistsMethod    = nullptr;

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// NVS namespaces used to store device configuration information.
const char AndroidConfig::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char AndroidConfig::kConfigNamespace_ChipConfig[]   = "chip-config";
const char AndroidConfig::kConfigNamespace_ChipCounters[] = "chip-counters";

// Keys stored in the Chip-factory namespace
const AndroidConfig::Key AndroidConfig::kConfigKey_SerialNum             = { kConfigNamespace_ChipFactory, "serial-num" };
const AndroidConfig::Key AndroidConfig::kConfigKey_MfrDeviceId           = { kConfigNamespace_ChipFactory, "device-id" };
const AndroidConfig::Key AndroidConfig::kConfigKey_MfrDeviceCert         = { kConfigNamespace_ChipFactory, "device-cert" };
const AndroidConfig::Key AndroidConfig::kConfigKey_MfrDeviceICACerts     = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const AndroidConfig::Key AndroidConfig::kConfigKey_MfrDevicePrivateKey   = { kConfigNamespace_ChipFactory, "device-key" };
const AndroidConfig::Key AndroidConfig::kConfigKey_HardwareVersion       = { kConfigNamespace_ChipFactory, "hardware-ver" };
const AndroidConfig::Key AndroidConfig::kConfigKey_HardwareVersionString = { kConfigNamespace_ChipFactory, "hardware-ver-str" };
const AndroidConfig::Key AndroidConfig::kConfigKey_ManufacturingDate     = { kConfigNamespace_ChipFactory, "mfg-date" };
const AndroidConfig::Key AndroidConfig::kConfigKey_SetupPinCode          = { kConfigNamespace_ChipFactory, "pin-code" };
const AndroidConfig::Key AndroidConfig::kConfigKey_SetupDiscriminator    = { kConfigNamespace_ChipFactory, "discriminator" };
const AndroidConfig::Key AndroidConfig::kConfigKey_ProductId             = { kConfigNamespace_ChipFactory, "product-id" };
const AndroidConfig::Key AndroidConfig::kConfigKey_ProductName           = { kConfigNamespace_ChipFactory, "product-name" };
const AndroidConfig::Key AndroidConfig::kConfigKey_SoftwareVersion       = { kConfigNamespace_ChipFactory, "software-version" };
const AndroidConfig::Key AndroidConfig::kConfigKey_SoftwareVersionString = { kConfigNamespace_ChipFactory, "software-version-str" };
const AndroidConfig::Key AndroidConfig::kConfigKey_PartNumber            = { kConfigNamespace_ChipFactory, "part-number" };
const AndroidConfig::Key AndroidConfig::kConfigKey_ProductURL            = { kConfigNamespace_ChipFactory, "product-url" };
const AndroidConfig::Key AndroidConfig::kConfigKey_ProductLabel          = { kConfigNamespace_ChipFactory, "product-label" };
const AndroidConfig::Key AndroidConfig::kConfigKey_UniqueId              = { kConfigNamespace_ChipFactory, "uniqueId" };
const AndroidConfig::Key AndroidConfig::kConfigKey_Spake2pIterationCount = { kConfigNamespace_ChipFactory, "iteration-count" };
const AndroidConfig::Key AndroidConfig::kConfigKey_Spake2pSalt           = { kConfigNamespace_ChipFactory, "salt" };
const AndroidConfig::Key AndroidConfig::kConfigKey_Spake2pVerifier       = { kConfigNamespace_ChipFactory, "verifier" };

// Keys stored in the Chip-config namespace
const AndroidConfig::Key AndroidConfig::kConfigKey_ServiceConfig      = { kConfigNamespace_ChipConfig, "service-config" };
const AndroidConfig::Key AndroidConfig::kConfigKey_PairedAccountId    = { kConfigNamespace_ChipConfig, "account-id" };
const AndroidConfig::Key AndroidConfig::kConfigKey_ServiceId          = { kConfigNamespace_ChipConfig, "service-id" };
const AndroidConfig::Key AndroidConfig::kConfigKey_LastUsedEpochKeyId = { kConfigNamespace_ChipConfig, "last-ek-id" };
const AndroidConfig::Key AndroidConfig::kConfigKey_FailSafeArmed      = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const AndroidConfig::Key AndroidConfig::kConfigKey_WiFiStationSecType = { kConfigNamespace_ChipConfig, "sta-sec-type" };
const AndroidConfig::Key AndroidConfig::kConfigKey_RegulatoryLocation = { kConfigNamespace_ChipConfig, "regulatory-location" };
const AndroidConfig::Key AndroidConfig::kConfigKey_CountryCode        = { kConfigNamespace_ChipConfig, "country-code" };

void AndroidConfig::InitializeWithObject(jobject managerObject)
{
    JNIEnv * env              = JniReferences::GetInstance().GetEnvForCurrentThread();
    gAndroidConfigObject      = env->NewGlobalRef(managerObject);
    jclass androidConfigClass = env->GetObjectClass(gAndroidConfigObject);
    VerifyOrReturn(androidConfigClass != nullptr, ChipLogError(DeviceLayer, "Failed to get KVS Java class"));

    gReadConfigValueLongMethod =
        env->GetMethodID(androidConfigClass, "readConfigValueLong", "(Ljava/lang/String;Ljava/lang/String;)J");
    if (gReadConfigValueLongMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access AndroidConfig 'readConfigValueLong' method");
        env->ExceptionClear();
    }

    gReadConfigValueStrMethod =
        env->GetMethodID(androidConfigClass, "readConfigValueStr", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    if (gReadConfigValueStrMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access AndroidConfig 'readConfigValueStr' method");
        env->ExceptionClear();
    }

    gReadConfigValueBinMethod =
        env->GetMethodID(androidConfigClass, "readConfigValueBin", "(Ljava/lang/String;Ljava/lang/String;)[B");
    if (gReadConfigValueBinMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access AndroidConfig 'readConfigValueBin' method");
        env->ExceptionClear();
    }

    gWriteConfigValueLongMethod =
        env->GetMethodID(androidConfigClass, "writeConfigValueLong", "(Ljava/lang/String;Ljava/lang/String;J)V");
    if (gWriteConfigValueLongMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access AndroidConfig 'writeConfigValueLong' method");
        env->ExceptionClear();
    }

    gWriteConfigValueStrMethod =
        env->GetMethodID(androidConfigClass, "writeConfigValueStr", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    if (gWriteConfigValueStrMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access AndroidConfig 'writeConfigValueStr' method");
        env->ExceptionClear();
    }

    gWriteConfigValueBinMethod =
        env->GetMethodID(androidConfigClass, "writeConfigValueBin", "(Ljava/lang/String;Ljava/lang/String;[B)V");
    if (gWriteConfigValueBinMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access AndroidConfig 'writeConfigValueBin' method");
        env->ExceptionClear();
    }

    gClearConfigValueMethod = env->GetMethodID(androidConfigClass, "clearConfigValue", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (gClearConfigValueMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access AndroidConfig 'clearConfigValue' method");
        env->ExceptionClear();
    }

    gConfigValueExistsMethod = env->GetMethodID(androidConfigClass, "configValueExists", "(Ljava/lang/String;Ljava/lang/String;)Z");
    if (gConfigValueExistsMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access AndroidConfig 'configValueExists' method");
        env->ExceptionClear();
    }
}

CHIP_ERROR AndroidConfig::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    return err;
}

CHIP_ERROR AndroidConfig::ReadConfigValue(Key key, bool & val)
{
    uint64_t valuint64;
    CHIP_ERROR err = AndroidConfig::ReadConfigValue(key, valuint64);
    if (err == CHIP_NO_ERROR)
    {
        val = static_cast<bool>(valuint64);
    }

    return err;
}

CHIP_ERROR AndroidConfig::ReadConfigValue(Key key, uint32_t & val)
{
    uint64_t valuint64;
    CHIP_ERROR err = AndroidConfig::ReadConfigValue(key, valuint64);
    if (err == CHIP_NO_ERROR)
    {
        val = static_cast<uint32_t>(valuint64);
    }

    return err;
}

CHIP_ERROR AndroidConfig::ReadConfigValue(Key key, uint64_t & val)
{
    chip::DeviceLayer::StackUnlock unlock;
    ReturnErrorCodeIf(gAndroidConfigObject == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(gReadConfigValueLongMethod == nullptr, CHIP_ERROR_INCORRECT_STATE);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ReturnErrorCodeIf(env == nullptr, CHIP_ERROR_INTERNAL);

    UtfString space(env, key.Namespace);
    UtfString name(env, key.Name);

    jlong javaValue = env->CallLongMethod(gAndroidConfigObject, gReadConfigValueLongMethod, space.jniValue(), name.jniValue());
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in AndroidConfig::readConfigValueLong");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    val = static_cast<uint64_t>(javaValue);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AndroidConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    chip::DeviceLayer::StackUnlock unlock;
    ReturnErrorCodeIf(gAndroidConfigObject == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(gReadConfigValueStrMethod == nullptr, CHIP_ERROR_INCORRECT_STATE);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ReturnErrorCodeIf(env == nullptr, CHIP_ERROR_INTERNAL);

    UtfString space(env, key.Namespace);
    UtfString name(env, key.Name);

    jobject javaValue = env->CallObjectMethod(gAndroidConfigObject, gReadConfigValueStrMethod, space.jniValue(), name.jniValue());
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in AndroidConfig::ReadConfigValueStr");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    chip::JniUtfString utfValue(env, (jstring) javaValue);
    outLen = strlen(utfValue.c_str());

    strncpy(buf, utfValue.c_str(), bufSize);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AndroidConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    chip::DeviceLayer::StackUnlock unlock;
    ReturnErrorCodeIf(gAndroidConfigObject == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(gReadConfigValueBinMethod == nullptr, CHIP_ERROR_INCORRECT_STATE);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ReturnErrorCodeIf(env == nullptr, CHIP_ERROR_INTERNAL);

    UtfString space(env, key.Namespace);
    UtfString name(env, key.Name);

    jbyteArray javaValue = static_cast<jbyteArray>(
        env->CallObjectMethod(gAndroidConfigObject, gReadConfigValueBinMethod, space.jniValue(), name.jniValue()));
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in AndroidConfig::ReadConfigValueBin");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    jbyte * elements = env->GetByteArrayElements(javaValue, NULL);
    if (elements == NULL)
    {
        ChipLogError(DeviceLayer, "AndroidConfig::ReadConfigValueBin got null");
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    outLen = static_cast<size_t>(env->GetArrayLength(javaValue));
    memcpy(buf, elements, min(outLen, bufSize));

    env->ReleaseByteArrayElements(javaValue, elements, 0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AndroidConfig::WriteConfigValue(Key key, bool val)
{
    return AndroidConfig::WriteConfigValue(key, static_cast<uint64_t>(val));
}

CHIP_ERROR AndroidConfig::WriteConfigValue(Key key, uint32_t val)
{
    return AndroidConfig::WriteConfigValue(key, static_cast<uint64_t>(val));
}

CHIP_ERROR AndroidConfig::WriteConfigValue(Key key, uint64_t val)
{
    chip::DeviceLayer::StackUnlock unlock;
    ReturnErrorCodeIf(gAndroidConfigObject == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(gWriteConfigValueLongMethod == nullptr, CHIP_ERROR_INCORRECT_STATE);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ReturnErrorCodeIf(env == nullptr, CHIP_ERROR_INTERNAL);

    UtfString space(env, key.Namespace);
    UtfString name(env, key.Name);
    jlong jval = static_cast<jlong>(val);

    env->CallVoidMethod(gAndroidConfigObject, gWriteConfigValueLongMethod, space.jniValue(), name.jniValue(), jval);
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in AndroidConfig::readConfigValueLong");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AndroidConfig::WriteConfigValueStr(Key key, const char * str)
{
    chip::DeviceLayer::StackUnlock unlock;
    ReturnErrorCodeIf(gAndroidConfigObject == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(gWriteConfigValueStrMethod == nullptr, CHIP_ERROR_INCORRECT_STATE);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ReturnErrorCodeIf(env == nullptr, CHIP_ERROR_INTERNAL);

    UtfString space(env, key.Namespace);
    UtfString name(env, key.Name);
    UtfString val(env, str);

    env->CallVoidMethod(gAndroidConfigObject, gWriteConfigValueStrMethod, space.jniValue(), name.jniValue(), val.jniValue());
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in AndroidConfig::gWriteConfigValueStrMethod");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AndroidConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
#if CHIP_CONFIG_MEMORY_MGMT_MALLOC
    CHIP_ERROR err;
    char * strCopy = nullptr;

    if (str != nullptr)
    {
        strCopy = strndup(str, strLen);
        VerifyOrExit(strCopy != nullptr, err = CHIP_ERROR_NO_MEMORY);
    }

    err = AndroidConfig::WriteConfigValueStr(key, strCopy);

exit:
    if (strCopy != nullptr)
    {
        free(strCopy);
    }
    return err;
#else
#error "Unsupported CHIP_CONFIG_MEMORY_MGMT configuration"
#endif
}

CHIP_ERROR AndroidConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    chip::DeviceLayer::StackUnlock unlock;
    ReturnErrorCodeIf(gAndroidConfigObject == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(gWriteConfigValueBinMethod == nullptr, CHIP_ERROR_INCORRECT_STATE);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ReturnErrorCodeIf(env == nullptr, CHIP_ERROR_INTERNAL);

    UtfString space(env, key.Namespace);
    UtfString name(env, key.Name);
    ByteArray jval(env, reinterpret_cast<const jbyte *>(data), dataLen);

    env->CallVoidMethod(gAndroidConfigObject, gWriteConfigValueBinMethod, space.jniValue(), name.jniValue(), jval.jniValue());
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in AndroidConfig::gWriteConfigValueBinMethod");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AndroidConfig::ClearConfigValue(Key key)
{
    chip::DeviceLayer::StackUnlock unlock;
    ReturnErrorCodeIf(gAndroidConfigObject == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(gClearConfigValueMethod == nullptr, CHIP_ERROR_INCORRECT_STATE);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ReturnErrorCodeIf(env == nullptr, CHIP_ERROR_INTERNAL);

    UtfString space(env, key.Namespace);
    UtfString name(env, key.Name);

    env->CallVoidMethod(gAndroidConfigObject, gClearConfigValueMethod, space.jniValue(), name.jniValue());
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in AndroidConfig::gClearConfigValueMethod");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    return CHIP_NO_ERROR;
}

bool AndroidConfig::ConfigValueExists(Key key)
{
    chip::DeviceLayer::StackUnlock unlock;
    ReturnErrorCodeIf(gAndroidConfigObject == nullptr, false);
    ReturnErrorCodeIf(gConfigValueExistsMethod == nullptr, false);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ReturnErrorCodeIf(env == nullptr, false);

    UtfString space(env, key.Namespace);
    UtfString name(env, key.Name);

    jboolean jvalue = env->CallBooleanMethod(gAndroidConfigObject, gConfigValueExistsMethod, space.jniValue(), name.jniValue());
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in AndroidConfig::gConfigValueExistsMethod");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

    return static_cast<bool>(jvalue);
}

CHIP_ERROR AndroidConfig::EnsureNamespace(const char * ns)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR AndroidConfig::ClearNamespace(const char * ns)
{
    const AndroidConfig::Key key = { ns, nullptr };
    return AndroidConfig::ClearConfigValue(key);
}

CHIP_ERROR AndroidConfig::FactoryResetConfig()
{
    const AndroidConfig::Key key = { AndroidConfig::kConfigNamespace_ChipConfig, nullptr };
    return AndroidConfig::ClearConfigValue(key);
}

void AndroidConfig::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<AndroidConfig>();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
