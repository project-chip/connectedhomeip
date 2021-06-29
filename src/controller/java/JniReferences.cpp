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

#include "JniReferences.h"

#include "CHIPJNIError.h"

#include <support/CodeUtils.h>

namespace chip {
namespace Controller {

pthread_mutex_t * JniReferences::GetStackLock()
{
    return &mStackLock;
}

void JniReferences::SetJavaVm(JavaVM * jvm)
{
    VerifyOrReturn(mJvm == nullptr, ChipLogError(Controller, "JavaVM is already set"));
    mJvm = jvm;

    // Cache the classloader for CHIP Java classes. env->FindClass() can start in the system class loader if called from a different
    // thread, meaning it won't find our Chip classes.
    // https://developer.android.com/training/articles/perf-jni#faq_FindClass
    JNIEnv * env = GetEnvForCurrentThread();
    // Any chip.devicecontroller.* class will work here - just need something to call getClassLoader() on.
    jclass chipClass               = env->FindClass("chip/devicecontroller/ChipDeviceController");
    jclass classClass              = env->FindClass("java/lang/Class");
    jclass classLoaderClass        = env->FindClass("java/lang/ClassLoader");
    jmethodID getClassLoaderMethod = env->GetMethodID(classClass, "getClassLoader", "()Ljava/lang/ClassLoader;");

    mClassLoader     = env->NewGlobalRef(env->CallObjectMethod(chipClass, getClassLoaderMethod));
    mFindClassMethod = env->GetMethodID(classLoaderClass, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
}

JNIEnv * JniReferences::GetEnvForCurrentThread()
{
    JNIEnv * env;
    if (mJvm == nullptr)
    {
        ChipLogError(Controller, "Missing Java VM");
        return nullptr;
    }
    mJvm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (env == nullptr)
    {
        jint error;
#ifdef __ANDROID__
        error = mJvm->AttachCurrentThreadAsDaemon(&env, nullptr);
#else
        error = mJvm->AttachCurrentThreadAsDaemon((void **) &env, nullptr);
#endif
        if (error != JNI_OK)
        {
            ChipLogError(Controller, "Failed to get JNIEnv for the current thread");
            return nullptr;
        }
    }
    return env;
}

CHIP_ERROR JniReferences::GetClassRef(JNIEnv * env, const char * clsType, jclass & outCls)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jclass cls     = nullptr;

    cls = env->FindClass(clsType);
    env->ExceptionClear();

    if (cls == nullptr)
    {
        // Try the cached classloader if FindClass() didn't work.
        cls = static_cast<jclass>(env->CallObjectMethod(mClassLoader, mFindClassMethod, env->NewStringUTF(clsType)));
        VerifyOrReturnError(cls != nullptr && env->ExceptionCheck() != JNI_TRUE, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
    }

    outCls = (jclass) env->NewGlobalRef((jobject) cls);
    VerifyOrReturnError(outCls != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    return err;
}

CHIP_ERROR JniReferences::N2J_ByteArray(JNIEnv * env, const uint8_t * inArray, uint32_t inArrayLen, jbyteArray & outArray)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    outArray = env->NewByteArray((int) inArrayLen);
    VerifyOrReturnError(outArray != NULL, CHIP_ERROR_NO_MEMORY);

    env->ExceptionClear();
    env->SetByteArrayRegion(outArray, 0, inArrayLen, (jbyte *) inArray);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    return err;
}

CHIP_ERROR JniReferences::FindMethod(JNIEnv * env, jobject object, const char * methodName, const char * methodSignature,
                                     jmethodID * methodId)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    jclass javaClass = nullptr;
    VerifyOrReturnError(env != nullptr && object != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);

    javaClass = env->GetObjectClass(object);
    VerifyOrReturnError(javaClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    *methodId = env->GetMethodID(javaClass, methodName, methodSignature);
    VerifyOrReturnError(*methodId != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    return err;
}

void JniReferences::CallVoidInt(JNIEnv * env, jobject object, const char * methodName, jint argument)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID method;

    err = JniReferences::FindMethod(env, object, methodName, "(I)V", &method);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Error finding Java method: %d", err);
    }

    env->ExceptionClear();
    env->CallVoidMethod(object, method, argument);
}

} // namespace Controller
} // namespace chip
