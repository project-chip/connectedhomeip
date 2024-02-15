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
 *      Implementation of JNI bridge for CHIP Test for Android apps
 *
 */

#include <dlfcn.h>
#include <jni.h>

#include <core/CHIPError.h>
#include <platform/android/AndroidChipPlatform-JNI.h>
#include <support/CHIPJNIError.h>
#include <support/CodeUtils.h>
#include <support/JniReferences.h>
#include <support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;

namespace {
JavaVM * sJVM;
jclass sTestEngineCls          = NULL;
jclass sTestEngineExceptionCls = NULL;
} // namespace

static void ThrowError(JNIEnv * env, CHIP_ERROR errToThrow);
static CHIP_ERROR N2J_Error(JNIEnv * env, CHIP_ERROR inErr, jthrowable & outEx);
// static void ReportError(JNIEnv * env, CHIP_ERROR cbErr, const char * functName);

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;
    ChipLogProgress(Test, "JNI_OnLoad() called");

    // Save a reference to the JVM.  Will need this to call back into Java.
    JniReferences::GetInstance().SetJavaVm(jvm, "com/tcl/chip/chiptest/TestEngine");
    sJVM = jvm;

    // Get a JNI environment object.
    env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    ChipLogProgress(Test, "Loading Java class references.");

    // Get various class references need by the API.
    jobject testEngineCls;
    err = JniReferences::GetInstance().GetLocalClassRef(env, "com/tcl/chip/chiptest/TestEngine", sTestEngineCls);
    SuccessOrExit(err);
    err = sTestEngineCls.Init(testEngineCls);
    SuccessOrExit(err);
    jobject testEngineExceptionCls;
    err = JniReferences::GetInstance().GetLocalClassRef(env, "com/tcl/chip/chiptest/TestEngineException", sTestEngineExceptionCls);
    SuccessOrExit(err);
    err = sTestEngineExceptionCls.Init(testEngineExceptionCls);
    SuccessOrExit(err);
    ChipLogProgress(Test, "Java class references loaded.");

    err = AndroidChipPlatformJNI_OnLoad(jvm, reserved);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ThrowError(env, err);
        JNI_OnUnload(jvm, reserved);
    }

    return (err == CHIP_NO_ERROR) ? JNI_VERSION_1_6 : JNI_ERR;
}

void JNI_OnUnload(JavaVM * jvm, void * reserved)
{
    ChipLogProgress(Test, "JNI_OnUnload() called");

    AndroidChipPlatformJNI_OnUnload(jvm, reserved);

    sJVM = NULL;
}

void ReportError(JNIEnv * env, CHIP_ERROR cbErr, const char * functName)
{
    if (cbErr == CHIP_JNI_ERROR_EXCEPTION_THROWN)
    {
        ChipLogError(Test, "Java exception thrown in %s", functName);
        env->ExceptionDescribe();
    }
    else
    {
        const char * errStr;
        switch (cbErr.AsInteger())
        {
        case CHIP_JNI_ERROR_TYPE_NOT_FOUND.AsInteger():
            errStr = "JNI type not found";
            break;
        case CHIP_JNI_ERROR_METHOD_NOT_FOUND.AsInteger():
            errStr = "JNI method not found";
            break;
        case CHIP_JNI_ERROR_FIELD_NOT_FOUND.AsInteger():
            errStr = "JNI field not found";
            break;
        default:
            errStr = ErrorStr(cbErr);
            break;
        }
        ChipLogError(Test, "Error in %s : %s", functName, errStr);
    }
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
    constructor = env->GetMethodID(sTestEngineExceptionCls.ObjectRef(), "<init>", "(ILjava/lang/String;)V");
    VerifyOrExit(constructor != NULL, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    switch (inErr.AsInteger())
    {
    case CHIP_JNI_ERROR_TYPE_NOT_FOUND.AsInteger():
        errStr = "CHIP Device Test Error: JNI type not found";
        break;
    case CHIP_JNI_ERROR_METHOD_NOT_FOUND.AsInteger():
        errStr = "CHIP Device Test Error: JNI method not found";
        break;
    case CHIP_JNI_ERROR_FIELD_NOT_FOUND.AsInteger():
        errStr = "CHIP Device Test Error: JNI field not found";
        break;
    case CHIP_JNI_ERROR_DEVICE_NOT_FOUND.AsInteger():
        errStr = "CHIP Device Test Error: Device not found";
        break;
    default:
        errStr = ErrorStr(inErr);
        break;
    }
    errStrObj = (errStr != NULL) ? env->NewStringUTF(errStr) : NULL;

    outEx = (jthrowable) env->NewObject(sTestEngineExceptionCls.ObjectRef(), constructor, static_cast<jint>(inErr.AsInteger()),
                                        errStrObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    env->DeleteLocalRef(errStrObj);
    return err;
}

static void onLog(const char * fmt, ...)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID method;
    jstring strObj = NULL;
    char str[512]  = { 0 };
    va_list args;

    ChipLogProgress(Test, "Received onLog");
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    method = env->GetStaticMethodID(sTestEngineCls.ObjectRef(), "onTestLog", "(Ljava/lang/String;)V");
    VerifyOrExit(method != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    va_start(args, fmt);
    vsnprintf(str, sizeof(str), fmt, args);
    va_end(args);
    strObj = env->NewStringUTF(str);

    ChipLogProgress(Test, "Calling Java onTestLog");

    env->ExceptionClear();
    env->CallStaticVoidMethod(sTestEngineCls.ObjectRef(), method, strObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    env->ExceptionClear();
    env->DeleteLocalRef(strObj);

    if (err != CHIP_NO_ERROR)
    {
        ReportError(env, err, __FUNCTION__);
    }
}

static void jni_log_name(struct _nlTestSuite * inSuite)
{
    onLog("[ %s ]\n", inSuite->name);
}

static void jni_log_initialize(struct _nlTestSuite * inSuite, int inResult, int inWidth)
{
    onLog("[ %s : %-*s ] : %s\n", inSuite->name, inWidth, "Initialize", inResult == FAILURE ? "FAILED" : "PASSED");
}
static void jni_log_terminate(struct _nlTestSuite * inSuite, int inResult, int inWidth)
{
    onLog("[ %s : %-*s ] : %s\n", inSuite->name, inWidth, "Terminate", inResult == FAILURE ? "FAILED" : "PASSED");
}

static void jni_log_setup(struct _nlTestSuite * inSuite, int inResult, int inWidth)
{
    onLog("[ %s : %-*s ] : %s\n", inSuite->name, inWidth, "Setup", inResult == FAILURE ? "FAILED" : "PASSED");
}

static void jni_log_test(struct _nlTestSuite * inSuite, int inWidth, int inIndex)
{
    onLog("[ %s : %-*s ] : %s\n", inSuite->name, inWidth, inSuite->tests[inIndex].name, inSuite->flagError ? "FAILED" : "PASSED");
}

static void jni_log_teardown(struct _nlTestSuite * inSuite, int inResult, int inWidth)
{
    onLog("[ %s : %-*s ] : %s\n", inSuite->name, inWidth, "TearDown", inResult == FAILURE ? "FAILED" : "PASSED");
}

static void jni_log_statTest(struct _nlTestSuite * inSuite)
{
    onLog("Failed Tests:   %d / %d\n", inSuite->failedTests, inSuite->runTests);
}

static void jni_log_statAssert(struct _nlTestSuite * inSuite)
{
    onLog("Failed Asserts: %d / %d\n", inSuite->failedAssertions, inSuite->performedAssertions);
}

static nl_test_output_logger_t jni_test_logger = {
    jni_log_name, jni_log_initialize, jni_log_terminate, jni_log_setup,
    jni_log_test, jni_log_teardown,   jni_log_statTest,  jni_log_statAssert,
};

extern "C" JNIEXPORT jint Java_com_tcl_chip_chiptest_TestEngine_runTest(JNIEnv * env, jclass clazz)
{
    nlTestSetLogger(&jni_test_logger);

    jint ret = RunRegisteredUnitTests();

    return ret;
}
