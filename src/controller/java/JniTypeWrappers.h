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

#pragma once

#include <jni.h>

/// Exposes the underlying UTF string from a jni string
class JniUtfString
{
public:
    JniUtfString(JNIEnv * env, jstring string) : mEnv(env), mString(string) { mChars = env->GetStringUTFChars(string, 0); }
    ~JniUtfString() { mEnv->ReleaseStringUTFChars(mString, mChars); }

    const char * c_str() const { return mChars; }

private:
    JNIEnv * mEnv;
    jstring mString;
    const char * mChars;
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
    UtfString(JNIEnv * env, const char * data) : mEnv(env) { mData = data != nullptr ? mEnv->NewStringUTF(data) : nullptr; }
    ~UtfString() { mEnv->DeleteLocalRef(mData); }

    jstring jniValue() { return mData; }

private:
    JNIEnv * mEnv;
    jstring mData;
};
