/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          Platform-specific implementation of KVS for android.
 */

#include <platform/KeyValueStoreManager.h>

#include <algorithm>
#include <string.h>

#include <lib/support/Base64.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

namespace {
constexpr size_t kMaxKvsValueBytes        = 4096;
constexpr size_t kMaxKvsValueEncodedChars = BASE64_ENCODED_LEN(kMaxKvsValueBytes) + 1;
} // namespace

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

void KeyValueStoreManagerImpl::InitializeWithObject(jobject manager)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(DeviceLayer, "Failed to GetEnvForCurrentThread for KeyValueStoreManagerImpl"));

    mKeyValueStoreManagerObject = env->NewGlobalRef(manager);
    VerifyOrReturn(mKeyValueStoreManagerObject != nullptr,
                   ChipLogError(DeviceLayer, "Failed to NewGlobalRef KeyValueStoreManager"));

    jclass KeyValueStoreManagerClass = env->GetObjectClass(manager);
    VerifyOrReturn(KeyValueStoreManagerClass != nullptr,
                   ChipLogError(DeviceLayer, "Failed to get KeyValueStoreManager Java class"));

    mGetMethod = env->GetMethodID(KeyValueStoreManagerClass, "get", "(Ljava/lang/String;)Ljava/lang/String;");
    if (mGetMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access KeyValueStoreManager 'get' method");
        env->ExceptionClear();
    }

    mSetMethod = env->GetMethodID(KeyValueStoreManagerClass, "set", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (mSetMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access KeyValueStoreManager 'set' method");
        env->ExceptionClear();
    }

    mDeleteMethod = env->GetMethodID(KeyValueStoreManagerClass, "delete", "(Ljava/lang/String;)V");
    if (mDeleteMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access KeyValueStoreManager 'delete' method");
        env->ExceptionClear();
    }
}

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReturnErrorCodeIf(mKeyValueStoreManagerObject == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(mGetMethod == nullptr, CHIP_ERROR_INCORRECT_STATE);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ReturnErrorCodeIf(env == nullptr, CHIP_JNI_ERROR_NO_ENV);

    chip::UtfString javaKey(env, key);

    jobject javaValue = env->CallObjectMethod(mKeyValueStoreManagerObject, mGetMethod, javaKey.jniValue());
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in KeyValueStoreManager::Get");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    if (javaValue == nullptr)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    chip::JniUtfString utfValue(env, (jstring) javaValue);
    const size_t utfValueLen = strlen(utfValue.c_str());

    if (utfValueLen > kMaxKvsValueEncodedChars)
    {
        ChipLogError(DeviceLayer, "Unexpected large value received from KeyValueStoreManager");
        return CHIP_ERROR_NO_MEMORY;
    }

    std::unique_ptr<uint8_t[]> buffer(new uint8_t[BASE64_MAX_DECODED_LEN(utfValueLen)]);
    uint16_t decodedLength = chip::Base64Decode(utfValue.c_str(), utfValueLen, buffer.get());
    if (decodedLength == UINT16_MAX)
    {
        ChipLogError(DeviceLayer, "KeyValueStoreManager base64 decoding failed");
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    }
    ReturnErrorCodeIf(offset_bytes != 0 && offset_bytes >= decodedLength, CHIP_ERROR_INVALID_ARGUMENT);
    size_t read_size = std::min<size_t>(value_size, decodedLength - offset_bytes);
    if (value_size + offset_bytes < decodedLength)
    {
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    if (read_bytes_size != nullptr)
    {
        *read_bytes_size = read_size;
    }

    if (value != nullptr)
    {
        memcpy(value, buffer.get() + offset_bytes, read_size);
    }

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    ReturnErrorCodeIf(mKeyValueStoreManagerObject == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(mSetMethod == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(value_size > kMaxKvsValueBytes, CHIP_ERROR_INVALID_ARGUMENT);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ReturnErrorCodeIf(env == nullptr, CHIP_ERROR_INTERNAL);

    std::unique_ptr<char[]> buffer(new char[BASE64_ENCODED_LEN(value_size) + 1]);

    size_t length        = chip::Base64Encode(static_cast<const uint8_t *>(value), value_size, buffer.get());
    buffer.get()[length] = 0;

    UtfString utfKey(env, key);
    UtfString utfBase64Value(env, buffer.get());

    env->CallVoidMethod(mKeyValueStoreManagerObject, mSetMethod, utfKey.jniValue(), utfBase64Value.jniValue());

    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in KeyValueStoreManager::Put");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    ReturnErrorCodeIf(mKeyValueStoreManagerObject == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(mDeleteMethod == nullptr, CHIP_ERROR_INCORRECT_STATE);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ReturnErrorCodeIf(env == nullptr, CHIP_ERROR_INTERNAL);

    UtfString javaKey(env, key);

    env->CallVoidMethod(mKeyValueStoreManagerObject, mDeleteMethod, javaKey.jniValue());

    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in KeyValueStoreManager::Delete");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    return CHIP_NO_ERROR;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
