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

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/android/AndroidChipPlatform-JNI.h>

#include "pw_unit_test/framework.h"
#include "pw_unit_test/googletest_style_event_handler.h"
#include "pw_unit_test/logging_event_handler.h"

using namespace chip;

namespace {
JavaVM * sJVM;
} // namespace

static void ReportError(JNIEnv * env, CHIP_ERROR cbErr, const char * functName);

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = nullptr;
    ChipLogProgress(Test, "JNI_OnLoad() called");

    sJVM = jvm;
    err  = AndroidChipPlatformJNI_OnLoad(jvm, reserved);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, JNI_ERR, ReportError(env, err, __FUNCTION__), JNI_OnUnload(jvm, reserved));

    // Get a JNI environment object.
    env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, JNI_ERR, ReportError(env, err, __FUNCTION__));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Test, "JNI_OnLoad() complete");

    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM * jvm, void * reserved)
{
    ChipLogProgress(Test, "JNI_OnUnload() called");

    AndroidChipPlatformJNI_OnUnload(jvm, reserved);

    sJVM = nullptr;
}

static void ReportError(JNIEnv * env, CHIP_ERROR cbErr, const char * functName)
{
    if (cbErr == CHIP_JNI_ERROR_EXCEPTION_THROWN && env != nullptr)
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
        case CHIP_JNI_ERROR_EXCEPTION_THROWN.AsInteger():
            errStr = "CHIP Device Test Error: Java exception thrown, env is nullptr";
            break;
        default:
            errStr = ErrorStr(cbErr);
            break;
        }
        ChipLogError(Test, "Error in %s : %s", functName, errStr);
    }
}

static void onLog(const char * fmt, ...)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID method;
    jstring strObj        = nullptr;
    jclass sTestEngineCls = nullptr;
    char str[512]         = {};
    va_list args;

    ChipLogProgress(Test, "Received onLog");
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Test, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    // Get various class references need by the API.
    err = JniReferences::GetInstance().GetLocalClassRef(env, "com/tcl/chip/chiptest/TestEngine", sTestEngineCls);
    SuccessOrExit(err);

    method = env->GetStaticMethodID(sTestEngineCls, "onTestLog", "(Ljava/lang/String;)V");
    VerifyOrExit(method != nullptr, err = CHIP_JNI_ERROR_NO_ENV);

    va_start(args, fmt);
    vsnprintf(str, sizeof(str), fmt, args);
    va_end(args);
    strObj = env->NewStringUTF(str);

    ChipLogProgress(Test, "Calling Java onTestLog");

    env->ExceptionClear();
    env->CallStaticVoidMethod(sTestEngineCls, method, strObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    env->ExceptionClear();
    env->DeleteLocalRef(strObj);
    VerifyOrReturn(err != CHIP_NO_ERROR, ReportError(env, err, __FUNCTION__));
}

namespace pw::unit_test {

class AndroidLoggingEventHandler : public pw::unit_test::LoggingEventHandler
{
public:
    void RunAllTestsStart() override { onLog(PW_UNIT_TEST_GOOGLETEST_RUN_ALL_TESTS_START); }

    void RunAllTestsEnd(const RunTestsSummary & run_tests_summary) override
    {
        onLog(PW_UNIT_TEST_GOOGLETEST_RUN_ALL_TESTS_END);
        onLog(PW_UNIT_TEST_GOOGLETEST_PASSED_SUMMARY, run_tests_summary.passed_tests);
        if (run_tests_summary.skipped_tests)
        {
            onLog(PW_UNIT_TEST_GOOGLETEST_DISABLED_SUMMARY, run_tests_summary.skipped_tests);
        }
        if (run_tests_summary.failed_tests)
        {
            onLog(PW_UNIT_TEST_GOOGLETEST_FAILED_SUMMARY, run_tests_summary.failed_tests);
        }
    }

    void TestCaseStart(const TestCase & test_case) override
    {
        onLog(PW_UNIT_TEST_GOOGLETEST_CASE_START, test_case.suite_name, test_case.test_name);
    }

    void TestCaseEnd(const TestCase & test_case, TestResult result) override
    {
        // Use a switch with no default to detect changes in the test result enum.
        switch (result)
        {
        case TestResult::kSuccess:
            onLog(PW_UNIT_TEST_GOOGLETEST_CASE_OK, test_case.suite_name, test_case.test_name);
            break;
        case TestResult::kFailure:
            onLog(PW_UNIT_TEST_GOOGLETEST_CASE_FAILED, test_case.suite_name, test_case.test_name);
            break;
        case TestResult::kSkipped:
            onLog(PW_UNIT_TEST_GOOGLETEST_CASE_DISABLED, test_case.suite_name, test_case.test_name);
            break;
        }
    }
};
}; // namespace pw::unit_test

extern "C" JNIEXPORT jint Java_com_tcl_chip_chiptest_TestEngine_runTest(JNIEnv * env, jclass clazz)
{
    chip::DeviceLayer::StackLock lock;

    // Running Pigweed Tests
    testing::InitGoogleTest(nullptr, static_cast<char **>(nullptr));
    pw::unit_test::AndroidLoggingEventHandler handler;
    pw::unit_test::RegisterEventHandler(&handler);
    jint ret = RUN_ALL_TESTS();

    return ret;
}
