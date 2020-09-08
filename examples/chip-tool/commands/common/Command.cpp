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

    SuccessOrExit(strcmp(name, argv[0]) != 0);
    VerifyOrExit(argsCount == argc - 1,
                 ChipLogProgress(chipTool, "InitArgs: Wrong arguments number: %zu instead of %zu", argc - 1, argsCount));

    for (int i = 0; i < argsCount; i++)
    {
        SuccessOrExit(!InitArgument(i, argv[1 + i]));
    }

    isValidCommand = true;

exit:
    return isValidCommand;
}

bool Command::InitArgument(size_t argIndex, const char * argValue)
{
    bool isValidArgument = false;

    Argument * arg = args[argIndex];
    switch (arg->type)
    {
    case ArgumentType::Attribute: {
        char * value    = reinterpret_cast<char *>(arg->value);
        isValidArgument = (strcmp(argValue, value) == 0);
        break;
    }

    case ArgumentType::Number: {
        uint32_t * value = reinterpret_cast<uint32_t *>(arg->value);
        // stringstream treats uint8_t as char, which is not what we want here.
        std::stringstream ss(argValue);
        ss >> *value;
        isValidArgument = (!ss.fail() && ss.eof() && *value >= arg->min && *value <= arg->max);
        break;
    }

    case ArgumentType::Address: {
        Inet::IPAddress * value = reinterpret_cast<Inet::IPAddress *>(arg->value);
        isValidArgument         = IPAddress::FromString(argValue, *value);
        break;
    }
    }

    if (!isValidArgument)
    {
        ChipLogError(chipTool, "InitArgs: Invalid argument %s: %s", arg->name, argValue);
    }

    return isValidArgument;
}

size_t Command::AddArgument(const char * name, const char * value)
{
    Argument * arg = new Argument();
    arg->type      = ArgumentType::Attribute;
    arg->name      = name;
    arg->value     = const_cast<void *>(reinterpret_cast<const void *>(value));

    args[argsCount] = arg;
    return argsCount++;
}

size_t Command::AddArgument(const char * name, Inet::IPAddress * out)
{
    Argument * arg = new Argument();
    arg->type      = ArgumentType::Address;
    arg->name      = name;
    arg->value     = reinterpret_cast<void *>(out);

    args[argsCount] = arg;
    return argsCount++;
}

size_t Command::AddArgument(const char * name, uint32_t min, uint32_t max, uint32_t * out)
{
    Argument * arg = new Argument();
    arg->type      = ArgumentType::Number;
    arg->name      = name;
    arg->value     = reinterpret_cast<void *>(out);
    arg->min       = min;
    arg->max       = max;

    args[argsCount] = arg;
    return argsCount++;
}

const char * Command::GetArgumentName(size_t index) const
{
    if (index < argsCount)
    {
        return args[index]->name;
    }

    return nullptr;
}

const char * Command::GetAttribute(void) const
{
    for (size_t i = 0; i < argsCount; i++)
    {
        if (args[i]->type == ArgumentType::Attribute)
        {
            return reinterpret_cast<const char *>(args[i]->value);
        }
    }

    return nullptr;
}
