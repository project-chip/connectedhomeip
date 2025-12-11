/*
 *   Copyright (c) 2025 Project CHIP Authors
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
#include "CustomStringPrefix.h"
#include "HexConversion.h"
#include "platform/PlatformManager.h"

#include <functional>
#include <netdb.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#include <math.h> // For INFINITY

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/StringSplitter.h>
#include <lib/support/logging/CHIPLogging.h>

constexpr char kOptionalArgumentPrefix[]       = "--";
constexpr size_t kOptionalArgumentPrefixLength = 2;

bool Command::InitArguments(int argc, char ** argv)
{
    bool isValidCommand = false;

    size_t argvExtraArgsCount = (size_t) argc;
    size_t mandatoryArgsCount = 0;
    size_t optionalArgsCount  = 0;
    for (auto & arg : mArgs)
    {
        if (arg.isOptional())
        {
            optionalArgsCount++;
        }
        else
        {
            mandatoryArgsCount++;
            argvExtraArgsCount--;
        }
    }

    VerifyOrExit((size_t) (argc) >= mandatoryArgsCount && (argvExtraArgsCount == 0 || (argvExtraArgsCount && optionalArgsCount)),
                 ChipLogError(NotSpecified, "InitArgs: Wrong arguments number: %d instead of %u", argc,
                              static_cast<unsigned int>(mandatoryArgsCount)));

    // Initialize mandatory arguments
    for (size_t i = 0; i < mandatoryArgsCount; i++)
    {
        char * arg = argv[i];
        if (!InitArgument(i, arg))
        {
            ExitNow();
        }
    }

    // Initialize optional arguments
    // Optional arguments expect a name and a value, so i is increased by 2 on every step.
    for (size_t i = mandatoryArgsCount; i < (size_t) argc; i += 2)
    {
        bool found = false;
        for (size_t j = mandatoryArgsCount; j < mandatoryArgsCount + optionalArgsCount; j++)
        {
            // optional arguments starts with kOptionalArgumentPrefix
            if (strlen(argv[i]) <= kOptionalArgumentPrefixLength &&
                strncmp(argv[i], kOptionalArgumentPrefix, kOptionalArgumentPrefixLength) != 0)
            {
                continue;
            }

            if (strcmp(argv[i] + strlen(kOptionalArgumentPrefix), mArgs[j].name) == 0)
            {
                found = true;

                VerifyOrExit((size_t) argc > (i + 1),
                             ChipLogError(NotSpecified, "InitArgs: Optional argument %s missing value.", argv[i]));
                if (!InitArgument(j, argv[i + 1]))
                {
                    ExitNow();
                }
            }
        }
        VerifyOrExit(found, ChipLogError(NotSpecified, "InitArgs: Optional argument %s does not exist.", argv[i]));
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
        ChipLogError(NotSpecified, "Invalid address: %s", addressString);
        return false;
    }

    if (result->ai_family == AF_INET6)
    {
        struct sockaddr_in6 * addr = reinterpret_cast<struct sockaddr_in6 *>(result->ai_addr);
        address->address           = ::chip::Inet::IPAddress::FromSockAddr(*addr);
        address->interfaceId       = ::chip::Inet::InterfaceId(addr->sin6_scope_id);
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (result->ai_family == AF_INET)
    {
        address->address     = ::chip::Inet::IPAddress::FromSockAddr(*reinterpret_cast<struct sockaddr_in *>(result->ai_addr));
        address->interfaceId = chip::Inet::InterfaceId::Null();
    }
#endif // INET_CONFIG_ENABLE_IPV4
    else
    {
        ChipLogError(NotSpecified, "Unsupported address: %s", addressString);
        return false;
    }

    return true;
}

// The callback should return whether the argument is valid, for the non-null
// case.  It can't directly write to isValidArgument (by closing over it)
// because in the nullable-and-null case we need to do that from this function,
// via the return value.
template <typename T>
bool HandleNullableOptional(Argument & arg, char * argValue, std::function<bool(T * value)> callback)
{
    if (arg.isOptional())
    {
        if (arg.isNullable())
        {
            arg.value = &(reinterpret_cast<chip::Optional<chip::app::DataModel::Nullable<T>> *>(arg.value)->Emplace());
        }
        else
        {
            arg.value = &(reinterpret_cast<chip::Optional<T> *>(arg.value)->Emplace());
        }
    }

    if (arg.isNullable())
    {
        auto * nullable = reinterpret_cast<chip::app::DataModel::Nullable<T> *>(arg.value);
        if (strcmp(argValue, "null") == 0)
        {
            nullable->SetNull();
            return true;
        }

        arg.value = &(nullable->SetNonNull());
    }

    return callback(reinterpret_cast<T *>(arg.value));
}

bool Command::InitArgument(size_t argIndex, char * argValue)
{
    bool isValidArgument = false;
    bool isHexNotation   = strncmp(argValue, "0x", 2) == 0 || strncmp(argValue, "0X", 2) == 0;

    Argument arg = mArgs.at(argIndex);

    // We have two places where we handle uint8_t-typed args (actual int8u and
    // bool args), so declare the handler function here so it can be reused.
    auto uint8Handler = [&](uint8_t * value) {
        // stringstream treats uint8_t as char, which is not what we want here.
        uint16_t tmpValue;
        std::stringstream ss;
        isHexNotation ? (ss << std::hex << argValue) : (ss << argValue);
        ss >> tmpValue;
        if (chip::CanCastTo<uint8_t>(tmpValue))
        {
            *value = static_cast<uint8_t>(tmpValue);

            uint64_t min = chip::CanCastTo<uint64_t>(arg.min) ? static_cast<uint64_t>(arg.min) : 0;
            uint64_t max = arg.max;
            return (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        }

        return false;
    };

    switch (arg.type)
    {
    case ArgumentType::Complex: {
        // Complex arguments may be optional, but they are not currently supported via the <chip::Optional> class.
        // Instead, they must be explicitly specified as optional using the kOptional flag,
        // and the base TypedComplexArgument<T> class is still referenced.
        auto complexArgument = static_cast<ComplexArgument *>(arg.value);
        return CHIP_NO_ERROR == complexArgument->Parse(arg.name, argValue);
    }

    case ArgumentType::Custom: {
        auto customArgument = static_cast<CustomArgument *>(arg.value);
        return CHIP_NO_ERROR == customArgument->Parse(arg.name, argValue);
    }

    case ArgumentType::VectorString: {
        std::vector<std::string> vectorArgument;

        chip::StringSplitter splitter(argValue, ',');
        chip::CharSpan value;

        while (splitter.Next(value))
        {
            vectorArgument.push_back(std::string(value.data(), value.size()));
        }

        if (arg.flags == Argument::kOptional)
        {
            auto argument = static_cast<chip::Optional<std::vector<std::string>> *>(arg.value);
            argument->SetValue(vectorArgument);
        }
        else
        {
            auto argument = static_cast<std::vector<std::string> *>(arg.value);
            *argument     = vectorArgument;
        }
        return true;
    }
    case ArgumentType::VectorBool: {
        // Currently only chip::Optional<std::vector<bool>> is supported.
        if (arg.flags != Argument::kOptional)
        {
            return false;
        }

        std::vector<bool> vectorArgument;
        std::stringstream ss(argValue);
        while (ss.good())
        {
            std::string valueAsString;
            getline(ss, valueAsString, ',');

            if (strcasecmp(valueAsString.c_str(), "true") == 0)
            {
                vectorArgument.push_back(true);
            }
            else if (strcasecmp(valueAsString.c_str(), "false") == 0)
            {
                vectorArgument.push_back(false);
            }
            else
            {
                return false;
            }
        }

        auto optionalArgument = static_cast<chip::Optional<std::vector<bool>> *>(arg.value);
        optionalArgument->SetValue(vectorArgument);
        return true;
    }

    case ArgumentType::Vector16:
    case ArgumentType::Vector32: {
        std::vector<uint64_t> values;
        uint64_t min = chip::CanCastTo<uint64_t>(arg.min) ? static_cast<uint64_t>(arg.min) : 0;
        uint64_t max = arg.max;

        std::stringstream ss(argValue);
        while (ss.good())
        {
            std::string valueAsString;
            getline(ss, valueAsString, ',');
            isHexNotation = strncmp(valueAsString.c_str(), "0x", 2) == 0 || strncmp(valueAsString.c_str(), "0X", 2) == 0;

            std::stringstream subss;
            isHexNotation ? subss << std::hex << valueAsString : subss << valueAsString;

            uint64_t value;
            subss >> value;
            VerifyOrReturnError(!subss.fail() && subss.eof() && value >= min && value <= max, false);
            values.push_back(value);
        }

        if (arg.type == ArgumentType::Vector16)
        {
            auto vectorArgument = static_cast<std::vector<uint16_t> *>(arg.value);
            for (uint64_t v : values)
            {
                vectorArgument->push_back(static_cast<uint16_t>(v));
            }
        }
        else if (arg.type == ArgumentType::Vector32 && arg.flags != Argument::kOptional)
        {
            auto vectorArgument = static_cast<std::vector<uint32_t> *>(arg.value);
            for (uint64_t v : values)
            {
                vectorArgument->push_back(static_cast<uint32_t>(v));
            }
        }
        else if (arg.type == ArgumentType::Vector32 && arg.flags == Argument::kOptional)
        {
            std::vector<uint32_t> vectorArgument;
            for (uint64_t v : values)
            {
                vectorArgument.push_back(static_cast<uint32_t>(v));
            }

            auto optionalArgument = static_cast<chip::Optional<std::vector<uint32_t>> *>(arg.value);
            optionalArgument->SetValue(vectorArgument);
        }
        else
        {
            return false;
        }

        return true;
    }

    case ArgumentType::VectorCustom: {
        auto vectorArgument = static_cast<std::vector<CustomArgument *> *>(arg.value);

        std::stringstream ss(argValue);
        while (ss.good())
        {
            std::string valueAsString;
            // By default the parameter separator is ";" in order to not collapse with the argument itself if it contains commas
            // (e.g a struct argument with multiple fields). In case one needs to use ";" it can be overriden with the following
            // environment variable.
            static constexpr char kSeparatorVariable[] = "NotSpecified,_CUSTOM_ARGUMENTS_SEPARATOR";
            char * getenvSeparatorVariableResult       = getenv(kSeparatorVariable);
            getline(ss, valueAsString, getenvSeparatorVariableResult ? getenvSeparatorVariableResult[0] : ';');

            CustomArgument * customArgument = new CustomArgument();
            vectorArgument->push_back(customArgument);
            VerifyOrReturnError(CHIP_NO_ERROR == vectorArgument->back()->Parse(arg.name, valueAsString.c_str()), false);
        }

        return true;
    }

    case ArgumentType::String: {
        isValidArgument = HandleNullableOptional<char *>(arg, argValue, [&](auto * value) {
            *value = argValue;
            return true;
        });
        break;
    }

    case ArgumentType::CharString: {
        isValidArgument = HandleNullableOptional<chip::CharSpan>(arg, argValue, [&](auto * value) {
            *value = chip::Span<const char>(argValue, strlen(argValue));
            return true;
        });
        break;
    }

    case ArgumentType::OctetString: {
        isValidArgument = HandleNullableOptional<chip::ByteSpan>(arg, argValue, [&](auto * value) {
            // We support two ways to pass an octet string argument.  If it happens
            // to be all-ASCII, you can just pass it in.  Otherwise you can pass in
            // "hex:" followed by the hex-encoded bytes.
            size_t argLen = strlen(argValue);

            if (IsHexString(argValue))
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

                size_t octetCount;
                CHIP_ERROR err = HexToBytes(
                    chip::CharSpan(argValue + kHexStringPrefixLen, argLen - kHexStringPrefixLen),
                    [&buffer](size_t allocSize) {
                        buffer.Calloc(allocSize);
                        return buffer.Get();
                    },
                    &octetCount);
                if (err != CHIP_NO_ERROR)
                {
                    return false;
                }

                memcpy(argValue, buffer.Get(), octetCount);
                *value = chip::ByteSpan(chip::Uint8::from_char(argValue), octetCount);
                return true;
            }

            // Just ASCII.  Check for the "str:" prefix.
            if (IsStrString(argValue))
            {
                // Skip the prefix
                argValue += kStrStringPrefixLen;
                argLen -= kStrStringPrefixLen;
            }
            *value = chip::ByteSpan(chip::Uint8::from_char(argValue), argLen);
            return true;
        });
        break;
    }

    case ArgumentType::Bool: {
        isValidArgument = HandleNullableOptional<bool>(arg, argValue, [&](auto * value) {
            // Start with checking for actual boolean values.
            if (strcasecmp(argValue, "true") == 0)
            {
                *value = true;
                return true;
            }

            if (strcasecmp(argValue, "false") == 0)
            {
                *value = false;
                return true;
            }

            // For backwards compat, keep accepting 0 and 1 for now as synonyms
            // for false and true.  Since we set our min to 0 and max to 1 for
            // booleans, calling uint8Handler does the right thing in terms of
            // only allowing those two values.
            uint8_t temp = 0;
            if (!uint8Handler(&temp))
            {
                return false;
            }
            *value = (temp == 1);
            return true;
        });
        break;
    }

    case ArgumentType::Number_uint8: {
        isValidArgument = HandleNullableOptional<uint8_t>(arg, argValue, uint8Handler);
        break;
    }

    case ArgumentType::Number_uint16: {
        isValidArgument = HandleNullableOptional<uint16_t>(arg, argValue, [&](auto * value) {
            std::stringstream ss;
            isHexNotation ? ss << std::hex << argValue : ss << argValue;
            ss >> *value;

            uint64_t min = chip::CanCastTo<uint64_t>(arg.min) ? static_cast<uint64_t>(arg.min) : 0;
            uint64_t max = arg.max;
            return (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        });
        break;
    }

    case ArgumentType::Number_uint32: {
        isValidArgument = HandleNullableOptional<uint32_t>(arg, argValue, [&](auto * value) {
            std::stringstream ss;
            isHexNotation ? ss << std::hex << argValue : ss << argValue;
            ss >> *value;

            uint64_t min = chip::CanCastTo<uint64_t>(arg.min) ? static_cast<uint64_t>(arg.min) : 0;
            uint64_t max = arg.max;
            return (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        });
        break;
    }

    case ArgumentType::Number_uint64: {
        isValidArgument = HandleNullableOptional<uint64_t>(arg, argValue, [&](auto * value) {
            std::stringstream ss;
            isHexNotation ? ss << std::hex << argValue : ss << argValue;
            ss >> *value;

            uint64_t min = chip::CanCastTo<uint64_t>(arg.min) ? static_cast<uint64_t>(arg.min) : 0;
            uint64_t max = arg.max;
            return (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        });
        break;
    }

    case ArgumentType::Number_int8: {
        isValidArgument = HandleNullableOptional<int8_t>(arg, argValue, [&](auto * value) {
            // stringstream treats int8_t as char, which is not what we want here.
            int16_t tmpValue;
            std::stringstream ss;
            isHexNotation ? ss << std::hex << argValue : ss << argValue;
            ss >> tmpValue;
            if (chip::CanCastTo<int8_t>(tmpValue))
            {
                *value = static_cast<int8_t>(tmpValue);

                int64_t min = arg.min;
                int64_t max = chip::CanCastTo<int64_t>(arg.max) ? static_cast<int64_t>(arg.max) : INT64_MAX;
                return (!ss.fail() && ss.eof() && *value >= min && *value <= max);
            }

            return false;
        });
        break;
    }

    case ArgumentType::Number_int16: {
        isValidArgument = HandleNullableOptional<int16_t>(arg, argValue, [&](auto * value) {
            std::stringstream ss;
            isHexNotation ? ss << std::hex << argValue : ss << argValue;
            ss >> *value;

            int64_t min = arg.min;
            int64_t max = chip::CanCastTo<int64_t>(arg.max) ? static_cast<int64_t>(arg.max) : INT64_MAX;
            return (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        });
        break;
    }

    case ArgumentType::Number_int32: {
        isValidArgument = HandleNullableOptional<int32_t>(arg, argValue, [&](auto * value) {
            std::stringstream ss;
            isHexNotation ? ss << std::hex << argValue : ss << argValue;
            ss >> *value;

            int64_t min = arg.min;
            int64_t max = chip::CanCastTo<int64_t>(arg.max) ? static_cast<int64_t>(arg.max) : INT64_MAX;
            return (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        });
        break;
    }

    case ArgumentType::Number_int64: {
        isValidArgument = HandleNullableOptional<int64_t>(arg, argValue, [&](auto * value) {
            std::stringstream ss;
            isHexNotation ? ss << std::hex << argValue : ss << argValue;
            ss >> *value;

            int64_t min = arg.min;
            int64_t max = chip::CanCastTo<int64_t>(arg.max) ? static_cast<int64_t>(arg.max) : INT64_MAX;
            return (!ss.fail() && ss.eof() && *value >= min && *value <= max);
        });
        break;
    }

    case ArgumentType::Float: {
        isValidArgument = HandleNullableOptional<float>(arg, argValue, [&](auto * value) {
            if (strcmp(argValue, "Infinity") == 0)
            {
                *value = INFINITY;
                return true;
            }

            if (strcmp(argValue, "-Infinity") == 0)
            {
                *value = -INFINITY;
                return true;
            }

            std::stringstream ss;
            ss << argValue;
            ss >> *value;
            return (!ss.fail() && ss.eof());
        });
        break;
    }

    case ArgumentType::Double: {
        isValidArgument = HandleNullableOptional<double>(arg, argValue, [&](auto * value) {
            if (strcmp(argValue, "Infinity") == 0)
            {
                *value = INFINITY;
                return true;
            }

            if (strcmp(argValue, "-Infinity") == 0)
            {
                *value = -INFINITY;
                return true;
            }

            std::stringstream ss;
            ss << argValue;
            ss >> *value;
            return (!ss.fail() && ss.eof());
        });
        break;
    }

    case ArgumentType::Address: {
        isValidArgument = HandleNullableOptional<AddressWithInterface>(
            arg, argValue, [&](auto * value) { return ParseAddressWithInterface(argValue, value); });
        break;
    }
    }

    if (!isValidArgument)
    {
        ChipLogError(NotSpecified, "InitArgs: Invalid argument %s: %s", arg.name, argValue);
    }

    return isValidArgument;
}

void Command::AddArgument(const char * name, const char * value, const char * desc)
{
    ReadOnlyGlobalCommandArgument arg;
    arg.name  = name;
    arg.value = value;
    arg.desc  = desc;

    mReadOnlyGlobalCommandArgument.SetValue(arg);
}

size_t Command::AddArgument(const char * name, char ** value, const char * desc, uint8_t flags)
{
    Argument arg;
    arg.type  = ArgumentType::String;
    arg.name  = name;
    arg.value = reinterpret_cast<void *>(value);
    arg.flags = flags;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, chip::CharSpan * value, const char * desc, uint8_t flags)
{
    Argument arg;
    arg.type  = ArgumentType::CharString;
    arg.name  = name;
    arg.value = reinterpret_cast<void *>(value);
    arg.flags = flags;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, chip::ByteSpan * value, const char * desc, uint8_t flags)
{
    Argument arg;
    arg.type  = ArgumentType::OctetString;
    arg.name  = name;
    arg.value = reinterpret_cast<void *>(value);
    arg.flags = flags;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, AddressWithInterface * out, const char * desc, uint8_t flags)
{
    Argument arg;
    arg.type  = ArgumentType::Address;
    arg.name  = name;
    arg.value = reinterpret_cast<void *>(out);
    arg.flags = flags;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, int64_t min, uint64_t max, std::vector<uint16_t> * value, const char * desc)
{
    Argument arg;
    arg.type  = ArgumentType::Vector16;
    arg.name  = name;
    arg.value = static_cast<void *>(value);
    arg.min   = min;
    arg.max   = max;
    arg.flags = 0;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, int64_t min, uint64_t max, std::vector<uint32_t> * value, const char * desc)
{
    Argument arg;
    arg.type  = ArgumentType::Vector32;
    arg.name  = name;
    arg.value = static_cast<void *>(value);
    arg.min   = min;
    arg.max   = max;
    arg.flags = 0;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, int64_t min, uint64_t max, chip::Optional<std::vector<uint32_t>> * value,
                            const char * desc)
{
    Argument arg;
    arg.type  = ArgumentType::Vector32;
    arg.name  = name;
    arg.value = static_cast<void *>(value);
    arg.min   = min;
    arg.max   = max;
    arg.flags = Argument::kOptional;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, int64_t min, uint64_t max, chip::Optional<std::vector<bool>> * value,
                            const char * desc)
{
    Argument arg;
    arg.type  = ArgumentType::VectorBool;
    arg.name  = name;
    arg.value = static_cast<void *>(value);
    arg.min   = min;
    arg.max   = max;
    arg.flags = Argument::kOptional;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, ComplexArgument * value, const char * desc, uint8_t flags)
{
    Argument arg;
    arg.type  = ArgumentType::Complex;
    arg.name  = name;
    arg.value = static_cast<void *>(value);
    arg.flags = flags;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, CustomArgument * value, const char * desc)
{
    Argument arg;
    arg.type  = ArgumentType::Custom;
    arg.name  = name;
    arg.value = const_cast<void *>(reinterpret_cast<const void *>(value));
    arg.flags = 0;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, std::vector<CustomArgument *> * value, const char * desc)
{
    Argument arg;
    arg.type  = ArgumentType::VectorCustom;
    arg.name  = name;
    arg.value = static_cast<void *>(value);
    arg.flags = 0;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, float min, float max, float * out, const char * desc, uint8_t flags)
{
    Argument arg;
    arg.type  = ArgumentType::Float;
    arg.name  = name;
    arg.value = reinterpret_cast<void *>(out);
    arg.flags = flags;
    arg.desc  = desc;
    // Ignore min/max for now; they're always +-Infinity anyway.

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, double min, double max, double * out, const char * desc, uint8_t flags)
{
    Argument arg;
    arg.type  = ArgumentType::Double;
    arg.name  = name;
    arg.value = reinterpret_cast<void *>(out);
    arg.flags = flags;
    arg.desc  = desc;
    // Ignore min/max for now; they're always +-Infinity anyway.

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, int64_t min, uint64_t max, void * out, ArgumentType type, const char * desc,
                            uint8_t flags)
{
    Argument arg;
    arg.type  = type;
    arg.name  = name;
    arg.value = out;
    arg.min   = min;
    arg.max   = max;
    arg.flags = flags;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, int64_t min, uint64_t max, void * out, const char * desc, uint8_t flags)
{
    Argument arg;
    arg.type  = ArgumentType::Number_uint8;
    arg.name  = name;
    arg.value = out;
    arg.min   = min;
    arg.max   = max;
    arg.flags = flags;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, std::vector<std::string> * value, const char * desc)
{
    Argument arg;
    arg.type  = ArgumentType::VectorString;
    arg.name  = name;
    arg.value = static_cast<void *>(value);
    arg.flags = 0;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

size_t Command::AddArgument(const char * name, chip::Optional<std::vector<std::string>> * value, const char * desc)
{
    Argument arg;
    arg.type  = ArgumentType::VectorString;
    arg.name  = name;
    arg.value = static_cast<void *>(value);
    arg.flags = Argument::kOptional;
    arg.desc  = desc;

    return AddArgumentToList(std::move(arg));
}

const char * Command::GetArgumentName(size_t index) const
{
    if (index < mArgs.size())
    {
        return mArgs.at(index).name;
    }

    return nullptr;
}

const char * Command::GetArgumentDescription(size_t index) const
{
    if (index < mArgs.size())
    {
        return mArgs.at(index).desc;
    }

    return nullptr;
}

const char * Command::GetReadOnlyGlobalCommandArgument() const
{
    if (GetAttribute())
    {
        return GetAttribute();
    }

    if (GetEvent())
    {
        return GetEvent();
    }

    return nullptr;
}

const char * Command::GetAttribute() const
{
    if (mReadOnlyGlobalCommandArgument.HasValue())
    {
        return mReadOnlyGlobalCommandArgument.Value().value;
    }

    return nullptr;
}

const char * Command::GetEvent() const
{
    if (mReadOnlyGlobalCommandArgument.HasValue())
    {
        return mReadOnlyGlobalCommandArgument.Value().value;
    }

    return nullptr;
}

size_t Command::AddArgumentToList(Argument && argument)
{
    if (argument.isOptional() || mArgs.empty() || !mArgs.back().isOptional())
    {
        // Safe to just append.
        mArgs.emplace_back(std::move(argument));
        return mArgs.size();
    }

    // We're inserting a non-optional arg but we already have something optional
    // in the list.  Insert before the first optional arg.
    for (auto cur = mArgs.cbegin(), end = mArgs.cend(); cur != end; ++cur)
    {
        if ((*cur).isOptional())
        {
            mArgs.emplace(cur, std::move(argument));
            return mArgs.size();
        }
    }

    // Never reached.
    VerifyOrDie(false);
    return 0;
}

namespace {
template <typename T>
void ResetOptionalArg(const Argument & arg)
{
    VerifyOrDie(arg.isOptional());

    if (arg.isNullable())
    {
        reinterpret_cast<chip::Optional<chip::app::DataModel::Nullable<T>> *>(arg.value)->ClearValue();
    }
    else
    {
        reinterpret_cast<chip::Optional<T> *>(arg.value)->ClearValue();
    }
}
} // anonymous namespace

void Command::ResetArguments()
{
    for (const auto & arg : mArgs)
    {
        const ArgumentType type = arg.type;
        if (arg.isOptional())
        {
            // Must always clean these up so they don't carry over to the next
            // command invocation in interactive mode.
            switch (type)
            {
            case ArgumentType::Complex: {
                // Optional Complex arguments are not currently supported via the <chip::Optional> class.
                // Instead, they must be explicitly specified as optional using the kOptional flag,
                // and the base TypedComplexArgument<T> class is referenced.
                auto argument = static_cast<ComplexArgument *>(arg.value);
                argument->Reset();
                break;
            }
            case ArgumentType::Custom: {
                // No optional custom arguments so far.
                VerifyOrDie(false);
                break;
            }
            case ArgumentType::VectorString: {
                ResetOptionalArg<std::vector<std::string>>(arg);
                break;
            }
            case ArgumentType::VectorBool: {
                ResetOptionalArg<std::vector<bool>>(arg);
                break;
            }
            case ArgumentType::Vector16: {
                // No optional Vector16 arguments so far.
                VerifyOrDie(false);
                break;
            }
            case ArgumentType::Vector32: {
                ResetOptionalArg<std::vector<uint32_t>>(arg);
                break;
            }
            case ArgumentType::VectorCustom: {
                // No optional VectorCustom arguments so far.
                VerifyOrDie(false);
                break;
            }
            case ArgumentType::String: {
                ResetOptionalArg<char *>(arg);
                break;
            }
            case ArgumentType::CharString: {
                ResetOptionalArg<chip::CharSpan>(arg);
                break;
            }
            case ArgumentType::OctetString: {
                ResetOptionalArg<chip::ByteSpan>(arg);
                break;
            }
            case ArgumentType::Bool: {
                ResetOptionalArg<bool>(arg);
                break;
            }
            case ArgumentType::Number_uint8: {
                ResetOptionalArg<uint8_t>(arg);
                break;
            }
            case ArgumentType::Number_uint16: {
                ResetOptionalArg<uint16_t>(arg);
                break;
            }
            case ArgumentType::Number_uint32: {
                ResetOptionalArg<uint32_t>(arg);
                break;
            }
            case ArgumentType::Number_uint64: {
                ResetOptionalArg<uint64_t>(arg);
                break;
            }
            case ArgumentType::Number_int8: {
                ResetOptionalArg<int8_t>(arg);
                break;
            }
            case ArgumentType::Number_int16: {
                ResetOptionalArg<int16_t>(arg);
                break;
            }
            case ArgumentType::Number_int32: {
                ResetOptionalArg<int32_t>(arg);
                break;
            }
            case ArgumentType::Number_int64: {
                ResetOptionalArg<int64_t>(arg);
                break;
            }
            case ArgumentType::Float: {
                ResetOptionalArg<float>(arg);
                break;
            }
            case ArgumentType::Double: {
                ResetOptionalArg<double>(arg);
                break;
            }
            case ArgumentType::Address: {
                ResetOptionalArg<AddressWithInterface>(arg);
                break;
            }
            }
        }
        else
        {
            // Some non-optional arguments have state that needs to be cleaned
            // up too.
            if (type == ArgumentType::Vector16)
            {
                auto vectorArgument = static_cast<std::vector<uint16_t> *>(arg.value);
                vectorArgument->clear();
            }
            else if (type == ArgumentType::Vector32)
            {
                auto vectorArgument = static_cast<std::vector<uint32_t> *>(arg.value);
                vectorArgument->clear();
            }
            else if (type == ArgumentType::VectorCustom)
            {
                auto vectorArgument = static_cast<std::vector<CustomArgument *> *>(arg.value);
                for (auto & customArgument : *vectorArgument)
                {
                    delete customArgument;
                }
                vectorArgument->clear();
            }
            else if (type == ArgumentType::Complex)
            {
                auto argument = static_cast<ComplexArgument *>(arg.value);
                argument->Reset();
            }
        }
    }
}
