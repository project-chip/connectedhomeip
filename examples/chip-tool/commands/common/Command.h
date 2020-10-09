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

#ifndef __CHIPTOOL_COMMAND_H__
#define __CHIPTOOL_COMMAND_H__

#include <controller/CHIPDeviceController.h>
#include <inet/InetInterface.h>
#include <support/CHIPLogging.h>

#include <memory>
#include <vector>

class Command;

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
    Number,
    String,
    Attribute,
    Address
};

struct Argument
{
    const char * name;
    ArgumentType type;
    uint32_t min;
    uint32_t max;
    void * value;
};

class Command
{
public:
    using ChipDeviceController = ::chip::DeviceController::ChipDeviceController;
    using IPAddress            = ::chip::Inet::IPAddress;
    using PacketBuffer         = ::chip::System::PacketBuffer;
    using NodeId               = ::chip::NodeId;

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
    template <class T>
    size_t AddArgument(const char * name, int64_t min, int64_t max, T * out)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out));
    }
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
    size_t AddArgument(const char * name, const void * value) { return 0; };

    virtual CHIP_ERROR Run(ChipDeviceController * dc, NodeId remoteId) = 0;

private:
    bool InitArgument(size_t argIndex, const char * argValue);
    size_t AddArgument(const char * name, int64_t min, int64_t max, void * out);

    const char * mName = nullptr;
    std::vector<Argument> mArgs;
};

#endif // __CHIPTOOL_COMMAND_H__
