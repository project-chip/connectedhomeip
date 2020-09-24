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

#include <vector>

#include <controller/CHIPDeviceController.h>
#include <core/CHIPError.h>
#include <inet/InetInterface.h>
#include <support/CHIPLogging.h>

enum ArgumentType
{
    Number,
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

    const char * GetName(void) const { return mName; }
    const char * GetAttribute(void) const;
    const char * GetArgumentName(size_t index) const;
    size_t GetArgumentsCount(void) const { return mArgs.size(); }

    bool InitArguments(int argc, char * argv[]);
    size_t AddArgument(const char * name, const char * value);
    size_t AddArgument(const char * name, uint32_t min, uint32_t max, uint32_t * out);
    size_t AddArgument(const char * name, AddressWithInterface * out);

    virtual CHIP_ERROR Run(ChipDeviceController * dc, NodeId remoteId) = 0;

private:
    bool InitArgument(size_t argIndex, const char * argValue);

    const char * mName = nullptr;
    std::vector<Argument> mArgs;
};

#endif // __CHIPTOOL_COMMAND_H__
