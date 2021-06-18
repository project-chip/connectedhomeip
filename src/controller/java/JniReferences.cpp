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

pthread_mutex_t JniReferences::sStackLock  = PTHREAD_MUTEX_INITIALIZER;
JavaVM * JniReferences::sJvm               = nullptr;
jclass JniReferences::sClusterExceptionCls = nullptr;

void JniReferences::SetJavaVm(JavaVM * jvm)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    sJvm           = jvm;
    JNIEnv * env   = GetEnvForCurrentThread();

    VerifyOrReturn(env != NULL, ChipLogError(Controller, "Could not get JNI env for current thread"));
    err = GetClassRef(env, "chip/devicecontroller/ChipClusterException", sClusterExceptionCls);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find ChipClusterException class"));
}

JNIEnv * JniReferences::GetEnvForCurrentThread()
{
    JNIEnv * env;
    if (sJvm == NULL)
    {
        ChipLogError(Controller, "Missing Java VM");
        return nullptr;
    }
    sJvm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (env == NULL)
    {
        jint error;
#ifdef __ANDROID__
        error = sJvm->AttachCurrentThreadAsDaemon(&env, NULL);
#else
        error = sJvm->AttachCurrentThreadAsDaemon((void **) &env, NULL);
#endif
        if (error != JNI_OK)
        {
            ChipLogError(Controller, "Failed to get JNIEnv for the current thread");
            return nullptr;
        }
    }
    return env;
}

CHIP_ERROR GetClassRef(JNIEnv * env, const char * clsType, jclass & outCls)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jclass cls     = NULL;

    cls = env->FindClass(clsType);
    VerifyOrExit(cls != NULL, err = CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    outCls = (jclass) env->NewGlobalRef((jobject) cls);
    VerifyOrExit(outCls != NULL, err = CHIP_JNI_ERROR_TYPE_NOT_FOUND);

exit:
    env->DeleteLocalRef(cls);
    return err;
}

CHIP_ERROR FindMethod(JNIEnv * env, jobject object, const char * methodName, const char * methodSignature, jmethodID * methodId)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    jclass javaClass = NULL;
    VerifyOrExit(env != nullptr && object != nullptr, err = CHIP_JNI_ERROR_NULL_OBJECT);

    javaClass = env->GetObjectClass(object);
    VerifyOrExit(javaClass != NULL, err = CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    *methodId = env->GetMethodID(javaClass, methodName, methodSignature);
    VerifyOrExit(*methodId != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

exit:
    return err;
}

void CallVoidInt(JNIEnv * env, jobject object, const char * methodName, jint argument)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID method;

    err = FindMethod(env, object, methodName, "(I)V", &method);
    SuccessOrExit(err);

    env->ExceptionClear();
    env->CallVoidMethod(object, method, argument);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Error calling Java method: %d", err);
    }
}
