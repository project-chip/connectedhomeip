/*******************************************************************************
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inin. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inin. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <provision/ProvisionManager.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/CHIPMem.h>
#include <mbedtls/platform.h>
#include <FreeRTOS.h>
#include <task.h>

using namespace chip::DeviceLayer::Silabs;

#define MAIN_TASK_STACK_SIZE    (1024)
#define MAIN_TASK_PRIORITY      (configMAX_PRIORITIES - 1)

namespace {

TaskHandle_t main_Task;

void taskMain(void * pvParameter)
{
    // Run manager
    Provision::Manager &man = Provision::Manager::GetInstance();
    while (man.Step());
    // Reset
    vTaskDelay(pdMS_TO_TICKS(500));
    NVIC_SystemReset();
}

} // namespace

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/

void app_init(void)
{
#if !defined(MBEDTLS_PLATFORM_CALLOC_MACRO) ||  !defined(MBEDTLS_PLATFORM_FREE_MACRO)
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);
    ReturnOnFailure(chip::Platform::MemoryInit());
#endif
    xTaskCreate(taskMain, "Provision Task", MAIN_TASK_STACK_SIZE, nullptr, MAIN_TASK_PRIORITY, &main_Task);
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/

void app_process_action(void)
{
}
