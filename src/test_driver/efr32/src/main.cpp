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

#include <AppConfig.h>
#include <FreeRTOS.h>
#include <PigweedLogger.h>
#include <PigweedLoggerMutex.h>
#include <cstring>
#include <init_efrPlatform.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/UnitTestRegistration.h>
#include <mbedtls/platform.h>
#include <nl_test_service/nl_test.rpc.pb.h>
#include <nlunit-test.h>
#include <pigweed/RpcService.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <task.h>

extern "C" int printf(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    chip::Logging::LogV(chip::Logging::kLogModule_NotSpecified, chip::Logging::kLogCategory_Progress, format, args);
    va_end(args);
    return 0;
}

namespace chip::rpc {

class NlTest : public generated::NlTest<NlTest>
{
public:
    void Run(ServerContext &, const pw_protobuf_Empty & request, ServerWriter<chip_rpc_Event> & writer)
    {
        stream_writer = &writer;
        nlTestSetLogger(&nl_test_logger);

        RunRegisteredUnitTests();

        stream_writer = nullptr;
        writer.Finish();
    }

private:
    static ServerWriter<chip_rpc_Event> * stream_writer;
    nl_test_output_logger_t nl_test_logger = {
        chip_nl_test_log_name, chip_nl_test_log_initialize, chip_nl_test_log_terminate, chip_nl_test_log_setup,
        chip_nl_test_log_test, chip_nl_test_log_teardown,   chip_nl_test_log_statTest,  chip_nl_test_log_statAssert,
    };

    static void StreamLogEvent(const chip_rpc_Event & event)
    {
        if (!stream_writer)
        {
            return;
        }
        stream_writer->Write(event);
    }

    static void chip_nl_test_log_name(struct _nlTestSuite * inSuite)
    {
        chip_rpc_Event event = chip_rpc_Event_init_default;
        event.which_type     = chip_rpc_Event_test_suite_start_tag;
        snprintf(event.type.test_suite_start.suite_name, sizeof(event.type.test_suite_start.suite_name), "%s", inSuite->name);
        StreamLogEvent(event);
    }

    static void chip_nl_test_log_initialize(struct _nlTestSuite * inSuite, int inResult, int inWidth)
    {
        chip_rpc_Event event = chip_rpc_Event_init_default;
        event.which_type     = chip_rpc_Event_test_case_initialize_tag;
        snprintf(event.type.test_case_initialize.suite_name, sizeof(event.type.test_case_initialize.suite_name), "%s",
                 inSuite->name);
        event.type.test_case_initialize.failed = inResult == FAILURE;
        StreamLogEvent(event);
    }
    static void chip_nl_test_log_terminate(struct _nlTestSuite * inSuite, int inResult, int inWidth)
    {
        chip_rpc_Event event = chip_rpc_Event_init_default;
        event.which_type     = chip_rpc_Event_test_case_terminate_tag;
        snprintf(event.type.test_case_terminate.suite_name, sizeof(event.type.test_case_terminate.suite_name), "%s", inSuite->name);
        event.type.test_case_terminate.failed = inResult == FAILURE;
        StreamLogEvent(event);
    }

    static void chip_nl_test_log_setup(struct _nlTestSuite * inSuite, int inResult, int inWidth)
    {
        chip_rpc_Event event = chip_rpc_Event_init_default;
        event.which_type     = chip_rpc_Event_test_suite_setup_tag;
        snprintf(event.type.test_suite_setup.suite_name, sizeof(event.type.test_suite_setup.suite_name), "%s", inSuite->name);
        event.type.test_suite_setup.failed = inResult == FAILURE;
        StreamLogEvent(event);
    }

    static void chip_nl_test_log_teardown(struct _nlTestSuite * inSuite, int inResult, int inWidth)
    {
        chip_rpc_Event event = chip_rpc_Event_init_default;
        event.which_type     = chip_rpc_Event_test_suite_teardown_tag;
        snprintf(event.type.test_suite_teardown.suite_name, sizeof(event.type.test_suite_teardown.suite_name), "%s", inSuite->name);
        event.type.test_suite_teardown.failed = inResult == FAILURE;
        StreamLogEvent(event);
    }

    static void chip_nl_test_log_test(struct _nlTestSuite * inSuite, int inWidth, int inIndex)
    {
        chip_rpc_Event event = chip_rpc_Event_init_default;
        event.which_type     = chip_rpc_Event_test_case_run_tag;
        snprintf(event.type.test_case_run.suite_name, sizeof(event.type.test_case_run.suite_name), "%s", inSuite->name);
        snprintf(event.type.test_case_run.test_case_name, sizeof(event.type.test_case_run.test_case_name), "%s",
                 inSuite->tests[inIndex].name);
        event.type.test_case_run.failed = inSuite->flagError;
        StreamLogEvent(event);
    }

    static void chip_nl_test_log_statTest(struct _nlTestSuite * inSuite)
    {
        chip_rpc_Event event = chip_rpc_Event_init_default;
        event.which_type     = chip_rpc_Event_test_suite_tests_run_summary_tag;
        snprintf(event.type.test_suite_tests_run_summary.suite_name, sizeof(event.type.test_suite_tests_run_summary.suite_name),
                 "%s", inSuite->name);
        event.type.test_suite_tests_run_summary.total_count  = inSuite->runTests;
        event.type.test_suite_tests_run_summary.failed_count = inSuite->failedTests;
        StreamLogEvent(event);
    }

    static void chip_nl_test_log_statAssert(struct _nlTestSuite * inSuite)
    {
        chip_rpc_Event event = chip_rpc_Event_init_default;
        event.which_type     = chip_rpc_Event_test_suite_asserts_summary_tag;
        snprintf(event.type.test_suite_asserts_summary.suite_name, sizeof(event.type.test_suite_asserts_summary.suite_name), "%s",
                 inSuite->name);
        event.type.test_suite_asserts_summary.total_count  = inSuite->performedAssertions;
        event.type.test_suite_asserts_summary.failed_count = inSuite->failedAssertions;
        StreamLogEvent(event);
    }
};

NlTest::ServerWriter<chip_rpc_Event> * NlTest::stream_writer = 0;

} // namespace chip::rpc

namespace {

#define TEST_TASK_STACK_SIZE 16 * 1024
#define TEST_TASK_PRIORITY 1

static TaskHandle_t sTestTaskHandle;
StaticTask_t sTestTaskBuffer;
StackType_t sTestTaskStack[TEST_TASK_STACK_SIZE];

chip::rpc::NlTest nl_test_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(nl_test_service);
}

void RunRpcService(void *)
{
    Start(RegisterServices, &::chip::rpc::logger_mutex);
}

} // namespace

int main(void)
{
    init_efrPlatform();
    PigweedLogger::init();
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    chip::Platform::MemoryInit();
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();

    chip::DeviceLayer::PlatformMgr().InitChipStack();

    EFR32_LOG("***** CHIP EFR32 device tests *****\r\n");

    // Start RPC service which runs the tests.
    sTestTaskHandle = xTaskCreateStatic(RunRpcService, "RPC_TEST_TASK", ArraySize(sTestTaskStack), nullptr, TEST_TASK_PRIORITY,
                                        sTestTaskStack, &sTestTaskBuffer);
    EFR32_LOG("Starting FreeRTOS scheduler");
    vTaskStartScheduler();

    // Should never get here.
    EFR32_LOG("vTaskStartScheduler() failed");
    return -1;
}
