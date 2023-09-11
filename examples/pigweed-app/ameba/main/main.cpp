/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "pw_rpc/echo_service_nanopb.h"
#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_ameba/init.h"

#include "PigweedLoggerMutex.h"
#include "pigweed/RpcService.h"
#include <FreeRTOS.h>
#include <task.h>

namespace {
using std::byte;

#define RPC_TASK_STACK_SIZE 4096
#define RPC_TASK_PRIORITY 2
static TaskHandle_t sRpcTaskHandle;

pw::rpc::EchoService echo_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(echo_service);
}

void RunRpcService(void *)
{
    Start(RegisterServices, &::chip::rpc::logger_mutex);
}

} // namespace

extern "C" void ChipTest(void)
{
    PigweedLogger::init();

    printf("***** CHIP ameba pigweed example *****\r\n");

    // pw_sys_io_Init();

    xTaskCreate(RunRpcService, "RPC_Task", RPC_TASK_STACK_SIZE / sizeof(StackType_t), nullptr, RPC_TASK_PRIORITY, &sRpcTaskHandle);
}
