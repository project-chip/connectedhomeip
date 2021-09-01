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
#include "platform/PlatformManager.h"

#include <netdb.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>

bool Command::InitArguments(int argc, char ** argv)
{
    bool isValidCommand = false;
    size_t argsCount    = mArgs.size();

    VerifyOrExit(argsCount == (size_t)(argc),
                 ChipLogError(chipTool, "InitArgs: Wrong arguments number: %d instead of %zu", argc, argsCount));

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

static bool ParseAddressWithInterface(const char * addressString, Command::AddressWithInterface * address)
{
    struct addrinfo hints;
    struct addrinfo * result;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    ret               = getaddrinfo(addressString, nullptr, &hints, &result);
    if (ret < 0)
    {
        ChipLogError(chipTool, "Invalid address: %s", addressString);
        return false;
    }

    address->address = ::chip::Inet::IPAddress::FromSockAddr(*result->ai_addr);
    if (result->ai_family == AF_INET6)
    {
        struct sockaddr_in6 * addr = reinterpret_cast<struct sockaddr_in6 *>(result->ai_addr);
        address->interfaceId       = addr->sin6_scope_id;
    }
    else
    {
        address->interfaceId = INET_NULL_INTERFACEID;
    }

    return true;
}

bool Command::InitArgument(size_t argIndex, char * argValue)
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

    case ArgumentType::CharString: {
        const char ** value = reinterpret_cast<const char **>(arg.value);
        *value              = argValue;
        isValidArgument     = true;
        break;
    }

    case ArgumentType::OctetString: {
        auto * value = static_cast<chip::ByteSpan *>(arg.value);
        // We support two ways to pass an octet string argument.  If it happens
        // to be all-ASCII, you can just pass it in.  Otherwise you can pass in
        // 0x followed by the hex-encoded bytes.
        size_t argLen                     = strlen(argValue);
        static constexpr char hexPrefix[] = "hex:";
        constexpr size_t prefixLen        = ArraySize(hexPrefix) - 1; // Don't count the null
        if (strncmp(argValue, hexPrefix, prefixLen) == 0)
        {
            // Hex-encoded.  Decode it into a temporary buffer first, so if we
            // run into errors we can do correct "argument is not valid" logging
            // that actually shows the value that was passed in.  After we
            // determine it's valid, modify the passed-in value to hold the
            // right bytes, so we don't need to worry about allocating storage
            // for this somewhere else.  This works because the hex
            // representation is always longer than the octet string it encodes,
            // so we have enough space in argValue for the decoded version.
            chip::Platform::ScopedMemoryBuffer<uint8_t> buffer;
            if (!buffer.Calloc(argLen)) // Bigger than needed, but it's fine.
            {
                isValidArgument = false;
                break;
            }

            size_t octetCount = chip::Encoding::HexToBytes(argValue + prefixLen, argLen - prefixLen, buffer.Get(), argLen);
            if (octetCount == 0)
            {
                isValidArgument = false;
                break;
            }

            memcpy(argValue, buffer.Get(), octetCount);
            *value          = chip::ByteSpan(chip::Uint8::from_char(argValue), octetCount);
            isValidArgument = true;
        }
        else
        {
            // Just ASCII.  Check for the "str:" prefix.
            static constexpr char strPrefix[] = "str:";
            constexpr size_t strPrefixLen     = ArraySize(strPrefix) - 1; // Don't count the null
            if (strncmp(argValue, strPrefix, strPrefixLen) == 0)
            {
                // Skip the prefix
                argValue += strPrefixLen;
                argLen -= strPrefixLen;
            }
            *value          = chip::ByteSpan(chip::Uint8::from_char(argValue), argLen);
            isValidArgument = true;
        }
        break;
    }

    case ArgumentType::Boolean:
    case ArgumentType::Number_uint8: {
        uint8_t * value = reinterpret_cast<uint8_t *>(arg.value);

        // stringstream treats uint8_t as char, which is not what we want here.
        uint16_t tmpValue;
        std::stringstream ss(argValue);
        ss >> tmpValue;
        if (chip::CanCastTo<uint8_t>(tmpValue))
        {
            *value = static_cast<uint8_t>(tmpValue);

            uint64_t min    = chip::CanCastTo<uint64_t>(arg.min) ? static_cast<uint64_t>(arg.min) : 0;
            uint64_t max    = arg.max;
            isValidArgument = (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        }
        else
        {
            isValidArgument = false;
        }
        break;
    }

    case ArgumentType::Number_uint16: {
        uint16_t * value = reinterpret_cast<uint16_t *>(arg.value);
        std::stringstream ss(argValue);
        ss >> *value;

        uint64_t min    = chip::CanCastTo<uint64_t>(arg.min) ? static_cast<uint64_t>(arg.min) : 0;
        uint64_t max    = arg.max;
        isValidArgument = (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        break;
    }

    case ArgumentType::Number_uint32: {
        uint32_t * value = reinterpret_cast<uint32_t *>(arg.value);
        std::stringstream ss(argValue);
        ss >> *value;

        uint64_t min    = chip::CanCastTo<uint64_t>(arg.min) ? static_cast<uint64_t>(arg.min) : 0;
        uint64_t max    = arg.max;
        isValidArgument = (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        break;
    }

    case ArgumentType::Number_uint64: {
        uint64_t * value = reinterpret_cast<uint64_t *>(arg.value);
        std::stringstream ss(argValue);
        ss >> *value;

        uint64_t min    = chip::CanCastTo<uint64_t>(arg.min) ? static_cast<uint64_t>(arg.min) : 0;
        uint64_t max    = arg.max;
        isValidArgument = (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        break;
    }

    case ArgumentType::Number_int8: {
        int8_t * value = reinterpret_cast<int8_t *>(arg.value);

        // stringstream treats int8_t as char, which is not what we want here.
        int16_t tmpValue;
        std::stringstream ss(argValue);
        ss >> tmpValue;
        if (chip::CanCastTo<int8_t>(tmpValue))
        {
            *value = static_cast<int8_t>(tmpValue);

            int64_t min     = arg.min;
            int64_t max     = chip::CanCastTo<int64_t>(arg.max) ? static_cast<int64_t>(arg.max) : INT64_MAX;
            isValidArgument = (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        }
        else
        {
            isValidArgument = false;
        }
        break;
    }

    case ArgumentType::Number_int16: {
        int16_t * value = reinterpret_cast<int16_t *>(arg.value);
        std::stringstream ss(argValue);
        ss >> *value;

        int64_t min     = arg.min;
        int64_t max     = chip::CanCastTo<int64_t>(arg.max) ? static_cast<int64_t>(arg.max) : INT64_MAX;
        isValidArgument = (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        break;
    }

    case ArgumentType::Number_int32: {
        int32_t * value = reinterpret_cast<int32_t *>(arg.value);
        std::stringstream ss(argValue);
        ss >> *value;

        int64_t min     = arg.min;
        int64_t max     = chip::CanCastTo<int64_t>(arg.max) ? static_cast<int64_t>(arg.max) : INT64_MAX;
        isValidArgument = (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        break;
    }

    case ArgumentType::Number_int64: {
        int64_t * value = reinterpret_cast<int64_t *>(arg.value);
        std::stringstream ss(argValue);
        ss >> *value;

        int64_t min     = arg.min;
        int64_t max     = chip::CanCastTo<int64_t>(arg.max) ? static_cast<int64_t>(arg.max) : INT64_MAX;
        isValidArgument = (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        break;
    }

    case ArgumentType::Address: {
        AddressWithInterface * value = reinterpret_cast<AddressWithInterface *>(arg.value);
        isValidArgument              = ParseAddressWithInterface(argValue, value);
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

    mArgs.emplace_back(arg);
    return mArgs.size();
}

size_t Command::AddArgument(const char * name, char ** value)
{
    Argument arg;
    arg.type  = ArgumentType::CharString;
    arg.name  = name;
    arg.value = reinterpret_cast<void *>(value);

    mArgs.emplace_back(arg);
    return mArgs.size();
}

size_t Command::AddArgument(const char * name, chip::ByteSpan * value)
{
    Argument arg;
    arg.type  = ArgumentType::OctetString;
    arg.name  = name;
    arg.value = reinterpret_cast<void *>(value);

    mArgs.emplace_back(arg);
    return mArgs.size();
}

size_t Command::AddArgument(const char * name, AddressWithInterface * out)
{
    Argument arg;
    arg.type  = ArgumentType::Address;
    arg.name  = name;
    arg.value = reinterpret_cast<void *>(out);

    mArgs.emplace_back(arg);
    return mArgs.size();
}

size_t Command::AddArgument(const char * name, int64_t min, uint64_t max, void * out, ArgumentType type)
{
    Argument arg;
    arg.type  = type;
    arg.name  = name;
    arg.value = out;
    arg.min   = min;
    arg.max   = max;

    mArgs.emplace_back(arg);
    return mArgs.size();
}

size_t Command::AddArgument(const char * name, int64_t min, uint64_t max, void * out)
{
    Argument arg;
    arg.type  = ArgumentType::Number_uint8;
    arg.name  = name;
    arg.value = out;
    arg.min   = min;
    arg.max   = max;

    mArgs.emplace_back(arg);
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

void Command::UpdateWaitForResponse(bool value)
{
#if CONFIG_USE_SEPARATE_EVENTLOOP
    {
        std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
        mWaitingForResponse = value;
    }
    cvWaitingForResponse.notify_all();
#else  // CONFIG_USE_SEPARATE_EVENTLOOP
    if (value == false)
    {
        if (mCommandExitStatus != CHIP_NO_ERROR)
        {
            ChipLogError(chipTool, "Run command failure: %s", chip::ErrorStr(mCommandExitStatus));
        }

        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    }
#endif // CONFIG_USE_SEPARATE_EVENTLOOP
}

#if CONFIG_USE_SEPARATE_EVENTLOOP

void Command::WaitForResponse(uint16_t seconds)
{
    std::chrono::seconds waitingForResponseTimeout(seconds);
    std::unique_lock<std::mutex> lk(cvWaitingForResponseMutex);
    auto waitingUntil = std::chrono::system_clock::now() + waitingForResponseTimeout;
    if (!cvWaitingForResponse.wait_until(lk, waitingUntil, [this]() { return !this->mWaitingForResponse; }))
    {
        ChipLogError(chipTool, "No response from device");
    }
}

#else // CONFIG_USE_SEPARATE_EVENTLOOP

static void OnResponseTimeout(chip::System::Layer *, void *)
{
    ChipLogError(chipTool, "No response from device");

    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
}

CHIP_ERROR Command::ScheduleWaitForResponse(uint16_t seconds)
{
    CHIP_ERROR err = chip::DeviceLayer::SystemLayer.StartTimer(seconds * 1000, OnResponseTimeout, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(chipTool, "Failed to allocate timer %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

#endif // CONFIG_USE_SEPARATE_EVENTLOOP
