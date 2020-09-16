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

#include "Command.h"
#include <sstream>
#include <support/CHIPLogging.h>

bool Command::InitArguments(int argc, char * argv[])
{
    bool isValidCommand = false;
    size_t argsCount    = mArgs.size();

    VerifyOrExit(argsCount == (size_t)(argc),
                 ChipLogProgress(chipTool, "InitArgs: Wrong arguments number: %zu instead of %zu", argc, argsCount));

    for (size_t i = 0; i < argsCount; i++)
    {
        if (!InitArgument(i, argv[i]))
        {
            ExitNow();
        }
    }

    isValidCommand = true;

exit:
    return isValidCommand;
}

bool Command::InitArgument(size_t argIndex, const char * argValue)
{
    bool isValidArgument = false;

    Argument arg = mArgs.at(argIndex);
    switch (arg.type)
    {
    case ArgumentType::Attribute: {
        char * value    = reinterpret_cast<char *>(arg.value);
        isValidArgument = (strcmp(argValue, value) == 0);
        break;
    }

    case ArgumentType::Number: {
        uint32_t * value = reinterpret_cast<uint32_t *>(arg.value);
        // stringstream treats uint8_t as char, which is not what we want here.
        std::stringstream ss(argValue);
        ss >> *value;
        isValidArgument = (!ss.fail() && ss.eof() && *value >= arg.min && *value <= arg.max);
        break;
    }

    case ArgumentType::Address: {
        IPAddress * value = reinterpret_cast<IPAddress *>(arg.value);
        isValidArgument   = IPAddress::FromString(argValue, *value);
        break;
    }
    }

    if (!isValidArgument)
    {
        ChipLogError(chipTool, "InitArgs: Invalid argument %s: %s", arg.name, argValue);
    }

    return isValidArgument;
}

size_t Command::AddArgument(const char * name, const char * value)
{
    Argument arg;
    arg.type  = ArgumentType::Attribute;
    arg.name  = name;
    arg.value = const_cast<void *>(reinterpret_cast<const void *>(value));

    mArgs.push_back(arg);
    return mArgs.size();
}

size_t Command::AddArgument(const char * name, IPAddress * out)
{
    Argument arg;
    arg.type  = ArgumentType::Address;
    arg.name  = name;
    arg.value = reinterpret_cast<void *>(out);

    mArgs.push_back(arg);
    return mArgs.size();
}

size_t Command::AddArgument(const char * name, uint32_t min, uint32_t max, uint32_t * out)
{
    Argument arg;
    arg.type  = ArgumentType::Number;
    arg.name  = name;
    arg.value = reinterpret_cast<void *>(out);
    arg.min   = min;
    arg.max   = max;

    mArgs.push_back(arg);
    return mArgs.size();
}

const char * Command::GetArgumentName(size_t index) const
{
    if (index < mArgs.size())
    {
        return mArgs.at(index).name;
    }

    return nullptr;
}

const char * Command::GetAttribute(void) const
{
    size_t argsCount = mArgs.size();
    for (size_t i = 0; i < argsCount; i++)
    {
        Argument arg = mArgs.at(i);
        if (arg.type == ArgumentType::Attribute)
        {
            return reinterpret_cast<const char *>(arg.value);
        }
    }

    return nullptr;
}
