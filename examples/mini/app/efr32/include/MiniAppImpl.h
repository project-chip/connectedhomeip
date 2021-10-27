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

#include <FreeRTOS.h>
#include <LEDWidget.h>
#include <MiniApp.h>
#include <queue.h>
#include <sl_simple_button_instances.h>
#include <string>
#include <task.h>
#include <timers.h>

#define APP_TASK_STACK_SIZE (4096)
#define APP_EVENT_QUEUE_SIZE 10

namespace example::efr32 {

struct TaskImpl : public example::Task
{
    TaskImpl(const char * name, Runner & run) : example::Task(name, run) {}
    CHIP_ERROR Start() override;
    void Finish() override;
    static void Callback(void * params);

    StackType_t mStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
    StaticTask_t mStruct;
    TaskHandle_t mHandle;
};

struct QueueImpl : public example::Queue
{
    QueueImpl(Dispatcher & dispatcher) : example::Queue(dispatcher) {}
    CHIP_ERROR Init() override;
    void Finish() override;
    CHIP_ERROR Post(const example::Event & event) override;
    void DispatchPending() override;

    uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(example::Event)];
    StaticQueue_t sAppEventQueueStruct;
    QueueHandle_t mQueue = nullptr;
};

// struct ButtonImpl : public example::Button
// {
//     Button(Id id, const char * name, Listener &listen) : example::Button(id, name, listen) {}
// };

class MiniAppImpl : public MiniApp
{
public:
    static MiniAppImpl sInstance;

    MiniAppImpl();
    CHIP_ERROR Init() override;
    // void Finish() override;
    CHIP_ERROR Start() override;
    friend void ::sl_button_on_change(const sl_button_t * handle);

protected:
    Task * CreateTask(const char * name, Task::Runner & runner) override;
    Queue * CreateQueue(Queue::Dispatcher & dispatcher) override;
    CHIP_ERROR Main() override;

private:
    LEDWidget mStatusLED;
};

} // namespace example::efr32
