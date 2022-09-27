/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#include <lib/core/CHIPError.h>
#include <pthread.h>
#include <string>

class NamedPipeCommandDelegate
{
public:
    virtual ~NamedPipeCommandDelegate()                    = default;
    virtual void OnEventCommandReceived(const char * json) = 0;
};

class NamedPipeCommands
{
public:
    CHIP_ERROR Start(std::string & path, NamedPipeCommandDelegate * delegate);
    CHIP_ERROR Stop();

private:
    bool mStarted = false;
    pthread_t mChipEventCommandListener;
    std::string mChipEventFifoPath;
    NamedPipeCommandDelegate * mDelegate = nullptr;

    static void * EventCommandListenerTask(void * arg);
};
