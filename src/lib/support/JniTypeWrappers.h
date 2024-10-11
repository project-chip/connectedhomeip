/*
 *
 *    Copyright (c) 2021-2024 Project CHIP Authors
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

#pragma once

#include <cstdint>
#include <jni.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/Span.h>
#include <string>

namespace chip {
/// Exposes the underlying UTF string from a jni string
class JniUtfString
{
public:
    JniUtfString(JNIEnv * env, jstring string) : mEnv(env), mString(string)
    {
        if (string == nullptr)
        {
            return;
        }
        mChars      = env->GetStringUTFChars(string, 0);
        mDataLength = env->GetStringUTFLength(string);
    }
    ~JniUtfString()
    {
        if (mString != nullptr)
        {
            mEnv->ReleaseStringUTFChars(mString, mChars);
        }
    }

    const char * c_str() const { return mChars; }

    chip::CharSpan charSpan() const { return chip::CharSpan(c_str(), static_cast<size_t>(size())); }

    chip::ByteSpan byteSpan() const { return chip::ByteSpan(chip::Uint8::from_const_char(c_str()), static_cast<size_t>(size())); }

    jsize size() const { return mDataLength; }

private:
    JNIEnv * mEnv       = nullptr;
    jstring mString     = nullptr;
    const char * mChars = nullptr;
    jsize mDataLength   = 0;
};

/// Exposes the underlying binary data from a jni byte array
class JniByteArray
{
public:
    JniByteArray(JNIEnv * env, jbyteArray array) :
        mEnv(env), mArray(array), mData(env->GetByteArrayElements(array, nullptr)), mDataLength(env->GetArrayLength(array))
    {}
    ~JniByteArray() { mEnv->ReleaseByteArrayElements(mArray, mData, 0); }

    const jbyte * data() const { return mData; }

    chip::ByteSpan byteSpan() const
    {
        return chip::ByteSpan(reinterpret_cast<const uint8_t *>(data()), static_cast<size_t>(size()));
    }

    jsize size() const { return mDataLength; }

private:
    JNIEnv * mEnv;
    jbyteArray mArray;
    jbyte * mData;
    jsize mDataLength;
};

/// wrap a c-string as a jni string
class UtfString
{
public:
    UtfString(JNIEnv * env, const char * data) : mEnv(env)
    {
        jstring localRef = data != nullptr ? mEnv->NewStringUTF(data) : nullptr;
        if (localRef == nullptr)
        {
            return;
        }
        mData = static_cast<jstring>(env->NewGlobalRef(localRef));
    }

    UtfString(JNIEnv * env, chip::CharSpan data) : mEnv(env)
    {
        std::string str(data.data(), data.size());
        jstring localRef = env->NewStringUTF(str.c_str());
        if (localRef == nullptr)
        {
            return;
        }
        mData = static_cast<jstring>(env->NewGlobalRef(localRef));
    }

    ~UtfString()
    {
        if (mEnv != nullptr && mData != nullptr)
        {
            mEnv->DeleteGlobalRef(mData);
            mData = nullptr;
        }
    }

    jstring jniValue() { return mData; }

private:
    JNIEnv * mEnv = nullptr;
    jstring mData = nullptr;
};

/// wrap a byte array as a JNI byte array
class ByteArray
{
public:
    ByteArray(JNIEnv * env, const jbyte * data, jsize dataLen) : mEnv(env)
    {
        jbyteArray localRef = data != nullptr ? mEnv->NewByteArray(dataLen) : nullptr;
        if (localRef == nullptr)
        {
            return;
        }
        env->SetByteArrayRegion(localRef, 0, dataLen, data);
        mArray = static_cast<jbyteArray>(env->NewGlobalRef(localRef));
    }

    ByteArray(JNIEnv * env, chip::ByteSpan data) : mEnv(env)
    {
        jbyteArray localRef = mEnv->NewByteArray(static_cast<jsize>(data.size()));
        if (localRef == nullptr)
        {
            return;
        }
        env->SetByteArrayRegion(localRef, 0, static_cast<jsize>(data.size()), reinterpret_cast<const jbyte *>(data.data()));
        mArray = static_cast<jbyteArray>(env->NewGlobalRef(localRef));
    }

    ~ByteArray()
    {
        if (mEnv != nullptr && mArray != nullptr)
        {
            mEnv->DeleteGlobalRef(mArray);
        }
    }

    jbyteArray jniValue() { return mArray; }

private:
    JNIEnv * mEnv     = nullptr;
    jbyteArray mArray = nullptr;
};

// Manages an pre-existing global reference to a jobject.
class JniGlobalRefWrapper
{
public:
    explicit JniGlobalRefWrapper(jobject mGlobalRef) : mGlobalRef(mGlobalRef) {}
    ~JniGlobalRefWrapper()
    {
        chip::JniReferences::GetInstance().GetEnvForCurrentThread()->DeleteGlobalRef(mGlobalRef);
        mGlobalRef = nullptr;
    }
    jobject classRef() { return mGlobalRef; }

private:
    jobject mGlobalRef = nullptr;
};
} // namespace chip
