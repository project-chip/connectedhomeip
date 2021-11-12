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
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <pthread.h>
#include <string>

namespace chip {
class JniReferences
{
public:
    // No copy, move or assignment.
    JniReferences(const JniReferences &)  = delete;
    JniReferences(const JniReferences &&) = delete;
    JniReferences & operator=(const JniReferences &) = delete;

    static JniReferences & GetInstance()
    {
        static JniReferences jniReferences;
        return jniReferences;
    }

    /**
     * Set the JavaVM.
     *
     * we need clsType in context to get ClassLoader
     *
     * This must be called before GetEnvForCurrentThread() or GetClassRef().
     */
    void SetJavaVm(JavaVM * jvm, const char * clsType);

    /**
     * Returns a JNIEnv for the current thread.
     *
     * This must be called after SetJavaVm(). If the current thread is not attached to the JVM, this method will attach the thread
     * first, then retrieve the JNIEnv.
     */
    JNIEnv * GetEnvForCurrentThread();

    /**
     * @brief
     *   Creates a jclass reference to the given class type.
     *
     *   This must be called after SetJavaVm().
     *
     * @param[in] env The JNIEnv for finding a Java class and creating a new Java reference.
     * @param[in] clsType The fully-qualified Java class name to find, e.g. java/lang/IllegalStateException.
     * @param[out] outCls A Java reference to the class matching clsType.
     */
    CHIP_ERROR GetClassRef(JNIEnv * env, const char * clsType, jclass & outCls);
    CHIP_ERROR FindMethod(JNIEnv * env, jobject object, const char * methodName, const char * methodSignature,
                          jmethodID * methodId);
    void CallVoidInt(JNIEnv * env, jobject object, const char * methodName, jint argument);

    CHIP_ERROR N2J_ByteArray(JNIEnv * env, const uint8_t * inArray, jsize inArrayLen, jbyteArray & outArray);

    void ReportError(JNIEnv * env, CHIP_ERROR cbErr, const char * functName);

    void ThrowError(JNIEnv * env, jclass exceptionCls, CHIP_ERROR errToThrow);

    /**
     * Creates a java.util.Optional wrapping the specified jobject. If the wrapped jobject is null, an empty
     * Optional will be returned.
     */
    CHIP_ERROR CreateOptional(jobject objectToWrap, jobject & outOptional);

private:
    JniReferences() {}

    JavaVM * mJvm              = nullptr;
    jobject mClassLoader       = nullptr;
    jmethodID mFindClassMethod = nullptr;
};
} // namespace chip
