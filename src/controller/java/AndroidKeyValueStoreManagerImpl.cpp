/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <platform/KeyValueStoreManager.h>

#include <algorithm>
#include <memory>
#include <string.h>

#include "AndroidKeyValueStoreManagerImpl.h"
#include "CHIPJNIError.h"
#include "JniReferences.h"
#include "JniTypeWrappers.h"

#include <lib/support/Base64.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

using namespace chip::Controller;
namespace {

constexpr size_t kMaxKvsValueBytes        = 4096;
constexpr size_t kMaxKvsValueEncodedChars = BASE64_ENCODED_LEN(kMaxKvsValueBytes) + 1;

} // namespace

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset)
{
    ReturnErrorCodeIf(mKeyValueStoreManagerObject == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(mGetMethod == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(offset != 0, CHIP_ERROR_INVALID_ARGUMENT);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    ReturnErrorCodeIf(env == nullptr, CHIP_ERROR_INTERNAL);

    UtfString javaKey(env, key);

    jobject javaValue = env->CallObjectMethod(mKeyValueStoreManagerObject, mGetMethod, javaKey.jniValue());
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in KVS::Get");
        env->ExceptionDescribe();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    if (javaValue == nullptr)
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    JniUtfString utfValue(env, (jstring) javaValue);
    const size_t utfValueLen = strlen(utfValue.c_str());

    if (utfValueLen > kMaxKvsValueEncodedChars)
    {
        ChipLogError(DeviceLayer, "Unexpected large value received from KVS");
        return CHIP_ERROR_NO_MEMORY;
    }

    std::unique_ptr<uint8_t[]> buffer(new uint8_t[BASE64_MAX_DECODED_LEN(utfValueLen)]);
    uint16_t decodedLength = chip::Base64Decode(utfValue.c_str(), utfValueLen, buffer.get());
    if (decodedLength == UINT16_MAX)
    {
        ChipLogError(DeviceLayer, "KVS base64 decoding failed");
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    }

    if (read_bytes_size != nullptr)
    {
        *read_bytes_size = decodedLength;
    }

    if (value != nullptr)
    {
        memcpy(value, buffer.get(), std::min<size_t>(value_size, decodedLength));
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
        ChipLogError(DeviceLayer, "Java exception in KVS::Delete");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    return CHIP_NO_ERROR;
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
        ChipLogError(DeviceLayer, "Java exception in KVS::Delete");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    return CHIP_NO_ERROR;
}

void KeyValueStoreManagerImpl::InitializeWithObject(jobject managerObject)
{
    JNIEnv * env                     = JniReferences::GetInstance().GetEnvForCurrentThread();
    mKeyValueStoreManagerObject      = env->NewGlobalRef(managerObject);
    jclass keyValueStoreManagerClass = env->GetObjectClass(mKeyValueStoreManagerObject);

    VerifyOrReturn(keyValueStoreManagerClass != nullptr, ChipLogError(DeviceLayer, "Failed to get KVS Java class"));

    mGetMethod = env->GetMethodID(keyValueStoreManagerClass, "get", "(Ljava/lang/String;)Ljava/lang/String;");
    if (mGetMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access KVS 'get' method");
        env->ExceptionClear();
    }

    mSetMethod = env->GetMethodID(keyValueStoreManagerClass, "set", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (mSetMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access KVS 'set' method");
        env->ExceptionClear();
    }

    mDeleteMethod = env->GetMethodID(keyValueStoreManagerClass, "delete", "(Ljava/lang/String;)V");
    if (mDeleteMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access KVS 'delete' method");
        env->ExceptionClear();
    }
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
