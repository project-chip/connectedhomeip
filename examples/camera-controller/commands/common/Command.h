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

#pragma once

#include <app/data-model/Nullable.h>
#include <commands/clusters/ComplexArgument.h>
#include <commands/clusters/CustomArgument.h>
#include <inet/InetInterface.h>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
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
    Number_uint8,
    Number_uint16,
    Number_uint32,
    Number_uint64,
    Number_int8,
    Number_int16,
    Number_int32,
    Number_int64,
    Float,
    Double,
    Bool,
    String,
    CharString,
    OctetString,
    Address,
    Complex,
    Custom,
    VectorBool,
    Vector16,
    Vector32,
    VectorCustom,
    VectorString, // comma separated string items
};

struct Argument
{
    const char * name;
    ArgumentType type;
    int64_t min;
    uint64_t max;
    void * value;
    uint8_t flags;
    const char * desc;

    enum
    {
        kOptional = (1 << 0),
        kNullable = (1 << 1),
    };

    bool isOptional() const { return flags & kOptional; }
    bool isNullable() const { return flags & kNullable; }
};

struct ReadOnlyGlobalCommandArgument
{
    const char * name;
    const char * value;
    const char * desc;
};

class Command
{
public:
    struct AddressWithInterface
    {
        ::chip::Inet::IPAddress address;
        ::chip::Inet::InterfaceId interfaceId;
    };

    Command(const char * commandName, const char * helpText = nullptr) : mName(commandName), mHelpText(helpText) {}
    virtual ~Command() {}

    const char * GetName(void) const { return mName; }
    const char * GetHelpText() const { return mHelpText; }
    const char * GetReadOnlyGlobalCommandArgument(void) const;
    const char * GetAttribute(void) const;
    const char * GetEvent(void) const;
    const char * GetArgumentName(size_t index) const;
    const char * GetArgumentDescription(size_t index) const;
    bool GetArgumentIsOptional(size_t index) const { return mArgs[index].isOptional(); }
    size_t GetArgumentsCount(void) const { return mArgs.size(); }

    bool InitArguments(int argc, char ** argv);
    void AddArgument(const char * name, const char * value, const char * desc = "");
    /**
     * @brief
     *   Add a char string command argument
     *
     * @param name  The name that will be displayed in the command help
     * @param value A pointer to a `char *` where the argv value will be stored
     * @param flags
     * @param desc The description of the argument that will be displayed in the command help
     * @returns The number of arguments currently added to the command
     */
    size_t AddArgument(const char * name, char ** value, const char * desc = "", uint8_t flags = 0);

    /**
     * Add an octet string command argument
     */
    size_t AddArgument(const char * name, chip::ByteSpan * value, const char * desc = "", uint8_t flags = 0);
    size_t AddArgument(const char * name, chip::Span<const char> * value, const char * desc = "", uint8_t flags = 0);
    size_t AddArgument(const char * name, AddressWithInterface * out, const char * desc = "", uint8_t flags = 0);
    // Optional Complex arguments are not currently supported via the <chip::Optional> class.
    // Instead, they must be explicitly specified as optional using kOptional in the flags parameter,
    // and the base TypedComplexArgument<T> class is referenced.
    size_t AddArgument(const char * name, ComplexArgument * value, const char * desc = "", uint8_t flags = 0);
    size_t AddArgument(const char * name, CustomArgument * value, const char * desc = "");
    size_t AddArgument(const char * name, int64_t min, uint64_t max, bool * out, const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Bool, desc, flags);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, int8_t * out, const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_int8, desc, flags);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, int16_t * out, const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_int16, desc, flags);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, int32_t * out, const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_int32, desc, flags);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, int64_t * out, const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_int64, desc, flags);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, uint8_t * out, const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_uint8, desc, flags);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, uint16_t * out, const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_uint16, desc, flags);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, uint32_t * out, const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_uint32, desc, flags);
    }
    size_t AddArgument(const char * name, int64_t min, uint64_t max, uint64_t * out, const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, min, max, reinterpret_cast<void *>(out), Number_uint64, desc, flags);
    }

    size_t AddArgument(const char * name, float min, float max, float * out, const char * desc = "", uint8_t flags = 0);
    size_t AddArgument(const char * name, double min, double max, double * out, const char * desc = "", uint8_t flags = 0);

    size_t AddArgument(const char * name, int64_t min, uint64_t max, std::vector<uint16_t> * value, const char * desc = "");
    size_t AddArgument(const char * name, int64_t min, uint64_t max, std::vector<uint32_t> * value, const char * desc = "");
    size_t AddArgument(const char * name, std::vector<CustomArgument *> * value, const char * desc = "");
    size_t AddArgument(const char * name, int64_t min, uint64_t max, chip::Optional<std::vector<bool>> * value,
                       const char * desc = "");
    size_t AddArgument(const char * name, int64_t min, uint64_t max, chip::Optional<std::vector<uint32_t>> * value,
                       const char * desc = "");

    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    size_t AddArgument(const char * name, int64_t min, uint64_t max, T * out, const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, min, max, reinterpret_cast<std::underlying_type_t<T> *>(out), desc, flags);
    }

    template <typename T>
    size_t AddArgument(const char * name, int64_t min, uint64_t max, chip::BitFlags<T> * out, const char * desc = "",
                       uint8_t flags = 0)
    {
        // This is a terrible hack that relies on BitFlags only having the one
        // mValue member.
        return AddArgument(name, min, max, reinterpret_cast<T *>(out), desc, flags);
    }

    template <typename T>
    size_t AddArgument(const char * name, int64_t min, uint64_t max, chip::BitMask<T> * out, const char * desc = "",
                       uint8_t flags = 0)
    {
        // This is a terrible hack that relies on BitMask only having the one
        // mValue member.
        return AddArgument(name, min, max, reinterpret_cast<T *>(out), desc, flags);
    }

    template <typename T>
    size_t AddArgument(const char * name, chip::Optional<T> * value, const char * desc = "")
    {
        return AddArgument(name, reinterpret_cast<T *>(value), desc, Argument::kOptional);
    }

    template <typename T>
    size_t AddArgument(const char * name, int64_t min, uint64_t max, chip::Optional<T> * value, const char * desc = "")
    {
        return AddArgument(name, min, max, reinterpret_cast<T *>(value), desc, Argument::kOptional);
    }

    template <typename T>
    size_t AddArgument(const char * name, chip::app::DataModel::Nullable<T> * value, const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, reinterpret_cast<T *>(value), desc, flags | Argument::kNullable);
    }

    template <typename T>
    size_t AddArgument(const char * name, int64_t min, uint64_t max, chip::app::DataModel::Nullable<T> * value,
                       const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, min, max, reinterpret_cast<T *>(value), desc, flags | Argument::kNullable);
    }

    size_t AddArgument(const char * name, float min, float max, chip::app::DataModel::Nullable<float> * value,
                       const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, min, max, reinterpret_cast<float *>(value), desc, flags | Argument::kNullable);
    }

    size_t AddArgument(const char * name, double min, double max, chip::app::DataModel::Nullable<double> * value,
                       const char * desc = "", uint8_t flags = 0)
    {
        return AddArgument(name, min, max, reinterpret_cast<double *>(value), desc, flags | Argument::kNullable);
    }

    size_t AddArgument(const char * name, std::vector<std::string> * value, const char * desc);
    size_t AddArgument(const char * name, chip::Optional<std::vector<std::string>> * value, const char * desc);

    void ResetArguments();

    virtual CHIP_ERROR Run() = 0;

    bool IsInteractive() { return mIsInteractive; }

    CHIP_ERROR RunAsInteractive(const chip::Optional<char *> & interactiveStorageDirectory, bool advertiseOperational)
    {
        mStorageDirectory     = interactiveStorageDirectory;
        mIsInteractive        = true;
        mAdvertiseOperational = advertiseOperational;
        return Run();
    }

    const chip::Optional<char *> & GetStorageDirectory() const { return mStorageDirectory; }

protected:
    // mStorageDirectory lives here so we can just set it in RunAsInteractive.
    chip::Optional<char *> mStorageDirectory;

    // mAdvertiseOperational lives here so we can just set it in
    // RunAsInteractive; it's only used by CHIPCommand.
    bool mAdvertiseOperational = false;

private:
    bool InitArgument(size_t argIndex, char * argValue);
    size_t AddArgument(const char * name, int64_t min, uint64_t max, void * out, ArgumentType type, const char * desc,
                       uint8_t flags);
    size_t AddArgument(const char * name, int64_t min, uint64_t max, void * out, const char * desc, uint8_t flags);

    /**
     * Add the Argument to our list.  This preserves the property that all
     * optional arguments come at the end of the list.
     */
    size_t AddArgumentToList(Argument && argument);

    const char * mName     = nullptr;
    const char * mHelpText = nullptr;
    bool mIsInteractive    = false;

    chip::Optional<ReadOnlyGlobalCommandArgument> mReadOnlyGlobalCommandArgument;
    std::vector<Argument> mArgs;
};
