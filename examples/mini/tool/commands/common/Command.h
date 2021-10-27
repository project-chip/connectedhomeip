/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#pragma once

#include "controller/ExampleOperationalCredentialsIssuer.h"
#include <controller/CHIPDeviceController.h>
#include <inet/InetInterface.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

class Command;
class PersistentStorage;

template <typename T, typename... Args>
std::unique_ptr<Command> make_unique(Args &&... args)
{
    return std::unique_ptr<Command>(new T(std::forward<Args>(args)...));
}

struct movable_initializer_list
{
    movable_initializer_list(std::unique_ptr<Command> && in) : item(std::move(in)) {}
    operator std::unique_ptr<Command>() const && { return std::move(item); }
    mutable std::unique_ptr<Command> item;
};

typedef std::initializer_list<movable_initializer_list> commands_list;

enum ArgumentType
{
    Number_uint8,
    Number_uint16,
    Number_uint32,
    Number_uint64,
    Number_int8,
    Number_int16,
    Number_int32,
    Number_int64,
    CharString,
    Boolean,
    OctetString,
    Attribute,
    Address
};

struct Argument
{
    const char * name;
    ArgumentType type;
    int64_t min;
    uint64_t max;
    void * value;
};

class Command
{
public:
    using ChipDeviceCommissioner = ::chip::Controller::DeviceCommissioner;
    using ChipDeviceController   = ::chip::Controller::DeviceController;
    using ChipSerializedDevice   = ::chip::Controller::SerializedDevice;
    using ChipDevice             = ::chip::Controller::Device;
    using PeerAddress            = ::chip::Transport::PeerAddress;
    using IPAddress              = ::chip::Inet::IPAddress;
    using PacketBufferHandle     = ::chip::System::PacketBufferHandle;
    using NodeId                 = ::chip::NodeId;

    struct AddressWithInterface
    {
        ::chip::Inet::IPAddress address;
        ::chip::Inet::InterfaceId interfaceId;
    };

    /**
     * @brief
     *   Encapsulates key objects in the CHIP stack that need continued
     *   access, so wrapping it in here makes it nice and compactly encapsulated.
     */
    struct ExecutionContext
    {
        ChipDeviceCommissioner * commissioner;
        chip::Controller::ExampleOperationalCredentialsIssuer * opCredsIssuer;
        PersistentStorage * storage;
        chip::NodeId localId;
        chip::NodeId remoteId;
    };

    Command(const char * commandName) : mName(commandName) {}
    virtual ~Command() {}

    void SetExecutionContext(ExecutionContext & execContext) { mExecContext = execContext; }

    const char * GetName(void) const { return mName; }
    const char * GetAttribute(void) const;
    const char * GetArgumentName(size_t index) const;
    size_t GetArgumentsCount(void) const { return mArgs.size(); }

    bool InitArguments(int argc, char ** argv);
    size_t AddArgument(const char * name, const char * value);
    /**
     * @brief
     *   Add a char string command argument
     *
     * @param name  The name that will be displayed in the command help
     * @param value A pointer to a `char *` where the argv value will be stored
     * @returns The number of arguments currently added to the command
     */
    size_t AddArgument(const char * name, char ** value);
    /**
     * Add an octet string command argument
     */
    size_t AddArgument(const char * name, chip::ByteSpan * value);
    size_t AddArgument(const char * name, AddressWithInterface * out);
    size_t AddArgument(const char * name, int64_t min, uint64_t max, bool * out)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Boolean);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, int8_t * out)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_int8);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, int16_t * out)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_int16);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, int32_t * out)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_int32);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, int64_t * out)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_int64);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, uint8_t * out)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_uint8);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, uint16_t * out)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_uint16);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, uint32_t * out)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_uint32);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, uint64_t * out)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_uint64);
    }

    // Will be called in a setting in which it's safe to touch the CHIP
    // stack. The rules for Run() are as follows:
    //
    // 1) If error is returned, Run() must not call SetCommandExitStatus.
    // 2) If success is returned Run() must either have called
    //    SetCommandExitStatus() or scheduled async work that will do that.
    virtual CHIP_ERROR Run() = 0;

    // Get the wait duration, in seconds, before the command times out.
    virtual uint16_t GetWaitDurationInSeconds() const = 0;

    // Shut down the command, in case any work needs to be done after the event
    // loop has been stopped.
    virtual void Shutdown() {}

    CHIP_ERROR GetCommandExitStatus() const { return mCommandExitStatus; }
    void SetCommandExitStatus(CHIP_ERROR status)
    {
        mCommandExitStatus = status;
        UpdateWaitForResponse(false);
    }

    void UpdateWaitForResponse(bool value);

    // There is a certain symmetry between the single-event-loop and
    // separate-event-loop approaches.  With a separate event loop, we schedule
    // our work on that event loop and synchronously wait (block) waiting for a
    // response. When using a single event loop, we ask for an async response
    // notification and then block processing work on the event loop
    // synchronously until that notification happens.
#if CONFIG_USE_SEPARATE_EVENTLOOP
    void WaitForResponse(uint16_t seconds);
#else  // CONFIG_USE_SEPARATE_EVENTLOOP
    CHIP_ERROR ScheduleWaitForResponse(uint16_t seconds);
#endif // CONFIG_USE_SEPARATE_EVENTLOOP

protected:
    ExecutionContext * GetExecContext() { return &mExecContext; }

private:
    bool InitArgument(size_t argIndex, char * argValue);
    size_t AddArgument(const char * name, int64_t min, uint64_t max, void * out, ArgumentType type);
    size_t AddArgument(const char * name, int64_t min, uint64_t max, void * out);

    CHIP_ERROR mCommandExitStatus = CHIP_ERROR_INTERNAL;
    const char * mName            = nullptr;
    std::vector<Argument> mArgs;

    ExecutionContext mExecContext;

#if CONFIG_USE_SEPARATE_EVENTLOOP
    std::condition_variable cvWaitingForResponse;
    std::mutex cvWaitingForResponseMutex;
    bool mWaitingForResponse{ false };
#endif // CONFIG_USE_SEPARATE_EVENTLOOP
};
