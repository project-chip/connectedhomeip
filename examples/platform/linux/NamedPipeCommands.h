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

#include <atomic>
#include <lib/core/CHIPError.h>
#include <pthread.h>
#include <string>

class NamedPipeCommandDelegate
{
public:
    virtual ~NamedPipeCommandDelegate() = default;
    /**
     * @brief Handle a single NamedPipeCommands payload.
     *
     * This method must handle dispatching to the Matter stack via `PlatformMgr().ScheduleWork(...)`
     * and must make copies internally of the input string if it needs to be passed beyond initial parsing.
     *
     * @param[in] json A null-terminated-string containing the JSON command payload to process.
     */
    virtual void OnEventCommandReceived(const char * json) = 0;
};

/**
 * This class implements a listener for named pipes (FIFOs). It uses a UNIX daemon
 * best-practice pattern of opening the FIFO with O_RDWR. This prevents the `read()`
 * call from constantly busy-looping and returning 0 (EOF) when external writers
 * (like a one-shot `echo` command) connect, write, and immediately disconnect.
 *
 * Each handler has an input `inPath` (e.g. "/tmp/matter_test_pipe") and may also
 * have an `outPath` which can be used with `WriteToOutPipe` to communicate
 * payloads to another app's input pipe.
 *
 * All payloads should be JSON to work with existing conventions (although the
 * NamedPipeCommands class does no parsing of its own or any real JSON checks.
 *
 * The handling of incoming payloads is done via NamedPipeCommandDelegate class
 * instances which expect a single JSON string payload, usually of the shape:
 *
 *   {"Name": <Some command>, "EndpointId": <Target endpoint>, ...other args}
 *
 * Example of a typical command written to inPath of `/tmp/chip_all_clusters_fifo_1146610`:
 *
 *   echo '{"Name": "SimulateSwitchIdle", "EndpointId": 3}' > /tmp/chip_all_clusters_fifo_1146610
 *
 * NOTE: Start() and Stop() must be called from the same thread (e.g., during outer
 * application initialization and shutdown sequencing) to ensure proper lifecycle
 * and synchronization.
 *
 * NOTE: Instances of NamedPipeCommands are one-time use. Once Stop() is called,
 * the instance cannot be restarted.
 *
 */
class NamedPipeCommands
{
public:
    /**
     * @brief Start listening for named pipe commands.
     *
     * @param[in] inPath Path to the input FIFO to create and listen on.
     * @param[in] delegate Delegate to handle received commands.
     */
    CHIP_ERROR Start(const std::string & inPath, NamedPipeCommandDelegate * delegate);

    /**
     * @brief Start listening for named pipe commands with an output pipe.
     *
     * @param[in] inPath Path to the input FIFO to create and listen on.
     * @param[in] outPath Path to an output FIFO for sending responses.
     * @param[in] delegate Delegate to handle received commands.
     */
    CHIP_ERROR Start(const std::string & inPath, const std::string & outPath, NamedPipeCommandDelegate * delegate);

    /**
     * @brief Stop listening for commands and shut down the listener thread.
     */
    CHIP_ERROR Stop();

    /**
     * @brief Write a JSON string to the output pipe.
     *
     * This method handles opening the pipe in a non-blocking fashion with retries
     * to avoid deadlocking the application if the test side hasn't opened the
     * pipe yet.
     *
     * @param[in] json JSON payload to write to the output pipe.
     */
    void WriteToOutPipe(const std::string & json);

    const std::string & OutPath() const { return mFifoOutPath; }

private:
    std::atomic<bool> mRunning{ false };
    std::atomic<bool> mDone{ false };
    pthread_t mChipEventCommandListener{};
    std::string mFifoInPath;
    std::string mFifoOutPath;
    NamedPipeCommandDelegate * mDelegate = nullptr;

    void Unlink();
    static void * EventCommandListenerTask(void * arg);
};
