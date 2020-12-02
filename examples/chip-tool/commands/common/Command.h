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

#include <controller/CHIPDeviceController.h>
#include <inet/InetInterface.h>
#include <support/logging/CHIPLogging.h>

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
    String,
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
    using ChipSerializedDevice   = ::chip::Controller::SerializedDevice;
    using ChipDevice             = ::chip::Controller::Device;
    using PeerAddress            = ::chip::Transport::PeerAddress;
    using IPAddress              = ::chip::Inet::IPAddress;
    using PacketBuffer           = ::chip::System::PacketBuffer;
    using PacketBufferHandle     = ::chip::System::PacketBufferHandle;
    using NodeId                 = ::chip::NodeId;

    struct AddressWithInterface
    {
        ::chip::Inet::IPAddress address;
        ::chip::Inet::InterfaceId interfaceId;
    };

    Command(const char * commandName) : mName(commandName) {}
    virtual ~Command() {}

    const char * GetName(void) const { return mName; }
    const char * GetAttribute(void) const;
    const char * GetArgumentName(size_t index) const;
    size_t GetArgumentsCount(void) const { return mArgs.size(); }

    bool InitArguments(int argc, char ** argv);
    size_t AddArgument(const char * name, const char * value);
    /**
     * @brief
     *   Add a string command argument
     *
     * @param name  The name that will be displayed in the command help
     * @param value A pointer to a `char *` where the argv value will be stored
     * @returns The number of arguments currently added to the command
     */
    size_t AddArgument(const char * name, char ** value);
    size_t AddArgument(const char * name, AddressWithInterface * out);
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

    virtual CHIP_ERROR Run(PersistentStorage & storage, NodeId localId, NodeId remoteId) = 0;

    bool GetCommandExitStatus() const { return mCommandExitStatus; }
    void SetCommandExitStatus(bool status) { mCommandExitStatus = status; }

    void UpdateWaitForResponse(bool value);
    void WaitForResponse(uint16_t duration);

private:
    bool InitArgument(size_t argIndex, const char * argValue);
    size_t AddArgument(const char * name, int64_t min, uint64_t max, void * out, ArgumentType type);
    size_t AddArgument(const char * name, int64_t min, uint64_t max, void * out);

    bool mCommandExitStatus = false;
    const char * mName      = nullptr;
    std::vector<Argument> mArgs;

    std::condition_variable cvWaitingForResponse;
    std::mutex cvWaitingForResponseMutex;
    bool mWaitingForResponse{ false };
};
