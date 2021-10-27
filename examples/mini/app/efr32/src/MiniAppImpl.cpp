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
#include <MiniAppImpl.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/server/OnboardingCodesUtil.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <qrcodegen.h>
#include <sl_simple_button_instances.h>
#include <sl_simple_led_instances.h>
#include <sl_system_kernel.h>

#include <init_efrPlatform.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>

#include <mbedtls/platform.h>

#define APP_STATE_LED &sl_led_led0
#define APP_ACTION_LED &sl_led_led1
#define APP_TASK_PRIORITY 2

example::MiniApp & example::MiniApp::Instance()
{
    return example::efr32::MiniAppImpl::sInstance;
}

namespace example::efr32 {

MiniAppImpl MiniAppImpl::sInstance;

//------------------------------------------------------------------------------
// Task
//------------------------------------------------------------------------------

CHIP_ERROR TaskImpl::Start()
{
    mHandle = xTaskCreateStatic(TaskImpl::Callback, mName, ArraySize(mStack), this, 1, mStack, &mStruct);
    VerifyOrReturnError(mHandle, CHIP_ERROR_NO_MEMORY);
    return CHIP_NO_ERROR;
}

void TaskImpl::Finish()
{
    vTaskDelete(mHandle);
}

void TaskImpl::Callback(void * params)
{
    TaskImpl * task = static_cast<TaskImpl *>(params);
    task->Run();
}

//------------------------------------------------------------------------------
// Queue
//------------------------------------------------------------------------------

CHIP_ERROR QueueImpl::Init()
{
    mQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(example::Event), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (NULL == mQueue)
    {
        EFR32_LOG("Failed to allocate app event queue");
        return CHIP_ERROR_NO_MEMORY;
    }
    return CHIP_NO_ERROR;
}

void QueueImpl::Finish()
{
    vQueueDelete(mQueue);
}

CHIP_ERROR QueueImpl::Post(const Event & event)
{
    if (mQueue)
    {
        BaseType_t status;
        if (xPortIsInsideInterrupt())
        {
            BaseType_t higherPrioTaskWoken = pdFALSE;
            status                         = xQueueSendFromISR(mQueue, &event, &higherPrioTaskWoken);

#ifdef portYIELD_FROM_ISR
            portYIELD_FROM_ISR(higherPrioTaskWoken);
#elif portEND_SWITCHING_ISR // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
            portEND_SWITCHING_ISR(higherPrioTaskWoken);
#else                       // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
#error "Must have portYIELD_FROM_ISR or portEND_SWITCHING_ISR"
#endif // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
        }
        else
        {
            status = xQueueSend(mQueue, &event, 1);
        }
        if (status)
        {
            return CHIP_NO_ERROR;
        }
    }
    EFR32_LOG("Failed to post event to app task event queue");
    return CHIP_ERROR_NO_MEMORY;
}

void QueueImpl::DispatchPending()
{
    example::Event event = example::Event::Id::None;

    BaseType_t received = xQueueReceive(mQueue, &event, pdMS_TO_TICKS(10));
    while (pdTRUE == received)
    {
        Dispatch(event);
        received = xQueueReceive(mQueue, &event, 0);
    }
}

//------------------------------------------------------------------------------
// App
//------------------------------------------------------------------------------

MiniAppImpl::MiniAppImpl() {}

CHIP_ERROR MiniAppImpl::Init()
{
    init_efrPlatform();
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

#if PW_RPC_ENABLED
    chip::rpc::Init();
#endif

#ifdef HEAP_MONITORING
    MemMonitoring::startHeapMonitoring();
#endif

    mStatusLED.Init(APP_STATE_LED);
    mStatusLED.Set(false);
    mStatusLED.Blink(1000);

    EFR32_LOG("==================================================");
    EFR32_LOG("chip-efr32-minimal-example starting");
    EFR32_LOG("==================================================");

    return MiniApp::Init();
}

// void MiniAppImpl::Finish()
// {
//     MiniApp::Finish();
// }

CHIP_ERROR MiniAppImpl::Start()
{
    CHIP_ERROR err = MiniApp::Start();
    VerifyOrReturnError(CHIP_NO_ERROR == err, err);

    EFR32_LOG("Starting FreeRTOS scheduler");
    sl_system_kernel_start();

    return CHIP_NO_ERROR;
}

example::Task * MiniAppImpl::CreateTask(const char * name, example::Task::Runner & runner)
{
    return new TaskImpl(name, runner);
}

example::Queue * MiniAppImpl::CreateQueue(example::Queue::Dispatcher & dispatcher)
{
    return new QueueImpl(dispatcher);
}

uint64_t sCounter = 0;

CHIP_ERROR MiniAppImpl::Main()
{
    mStatusLED.Animate();
    return CHIP_NO_ERROR;
}

} // namespace example::efr32

//------------------------------------------------------------------------------
// Buttons
//------------------------------------------------------------------------------

// Silabs button callback from button event ISR
void sl_button_on_change(const sl_button_t * handle)
{
    bool pressed = sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED;
    
    if (handle == &sl_button_btn0)
    {
        // Button Up
        example::MiniApp::Instance().PostEvent(pressed ? example::Event::Id::Button0Pressed : example::Event::Id::Button0Released);
    }
    else
    {
        // Button Down
        example::MiniApp::Instance().PostEvent(pressed ? example::Event::Id::Button1Pressed : example::Event::Id::Button1Released);
    }
}
