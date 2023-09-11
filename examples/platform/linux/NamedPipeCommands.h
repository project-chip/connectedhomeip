/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
