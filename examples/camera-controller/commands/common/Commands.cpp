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

#include "Commands.h"

#include "Command.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

#include <lib/support/Base64.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/KeyValueStoreManager.h>

#include "../clusters/JsonParser.h"

namespace {

char kInteractiveModeName[]                         = "";
constexpr size_t kInteractiveModeArgumentsMaxLength = 32;
constexpr char kOptionalArgumentPrefix[]            = "--";
constexpr char kJsonClusterKey[]                    = "cluster";
constexpr char kJsonCommandKey[]                    = "command";
constexpr char kJsonCommandSpecifierKey[]           = "command_specifier";
constexpr char kJsonArgumentsKey[]                  = "arguments";

#if !CHIP_DISABLE_PLATFORM_KVS
template <typename T>
struct HasInitWithString
{
    template <typename U>
    static constexpr auto check(U *) -> typename std::is_same<decltype(std::declval<U>().Init("")), CHIP_ERROR>::type;

    template <typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<std::remove_reference_t<T>>(nullptr)) type;

public:
    static constexpr bool value = type::value;
};

// Template so we can do conditional enabling
template <typename T, std::enable_if_t<HasInitWithString<T>::value, int> = 0>
static void UseStorageDirectory(T & storageManagerImpl, const char * storageDirectory)
{
    std::string platformKVS = std::string(storageDirectory) + "/chip_tool_kvs";
    storageManagerImpl.Init(platformKVS.c_str());
}

template <typename T, std::enable_if_t<!HasInitWithString<T>::value, int> = 0>
static void UseStorageDirectory(T & storageManagerImpl, const char * storageDirectory)
{}
#endif // !CHIP_DISABLE_PLATFORM_KVS

bool GetArgumentsFromJson(Command * command, Json::Value & value, bool optional, std::vector<std::string> & outArgs)
{
    auto memberNames = value.getMemberNames();

    std::vector<std::string> args;
    for (size_t i = 0; i < command->GetArgumentsCount(); i++)
    {
        auto argName             = command->GetArgumentName(i);
        auto memberNamesIterator = memberNames.begin();
        while (memberNamesIterator != memberNames.end())
        {
            auto memberName = *memberNamesIterator;
            if (strcasecmp(argName, memberName.c_str()) != 0)
            {
                memberNamesIterator++;
                continue;
            }

            if (command->GetArgumentIsOptional(i) != optional)
            {
                memberNamesIterator = memberNames.erase(memberNamesIterator);
                continue;
            }

            if (optional)
            {
                args.push_back(std::string(kOptionalArgumentPrefix) + argName);
            }

            auto argValue = value[memberName].asString();
            args.push_back(std::move(argValue));
            memberNamesIterator = memberNames.erase(memberNamesIterator);
            break;
        }
    }

    if (memberNames.size())
    {
        auto memberName = memberNames.front();
        ChipLogError(NotSpecified, "The argument \"\%s\" is not supported.", memberName.c_str());
        return false;
    }

    outArgs = args;
    return true;
};

// Check for arguments with a starting '"' but no ending '"': those
// would indicate that people are using double-quoting, not single
// quoting, on arguments with spaces.
static void DetectAndLogMismatchedDoubleQuotes(int argc, char ** argv)
{
    for (int curArg = 0; curArg < argc; ++curArg)
    {
        char * arg = argv[curArg];
        if (!arg)
        {
            continue;
        }

        auto len = strlen(arg);
        if (len == 0)
        {
            continue;
        }

        if (arg[0] == '"' && arg[len - 1] != '"')
        {
            ChipLogError(NotSpecified,
                         "Mismatched '\"' detected in argument: '%s'.  Use single quotes to delimit arguments with spaces "
                         "in them: 'x y', not \"x y\".",
                         arg);
        }
    }
}

} // namespace

// Define the static member
Commands Commands::sInstance;

void Commands::Register(const char * commandSetName, commands_list commandsList, const char * helpText, bool isCluster)
{
    VerifyOrDieWithMsg(isCluster || helpText != nullptr, NotSpecified, "Non-cluster command sets must have help text");
    mCommandSets[commandSetName].isCluster = isCluster;
    mCommandSets[commandSetName].helpText  = helpText;
    for (auto & command : commandsList)
    {
        mCommandSets[commandSetName].commands.push_back(std::move(command));
    }
}

int Commands::Run(int argc, char ** argv)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Memory failure: %s", chip::ErrorStr(err)));

#ifdef CONFIG_USE_LOCAL_STORAGE
    err = mStorage.Init();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Storage failure: %s", chip::ErrorStr(err)));

    chip::Logging::SetLogFilter(mStorage.GetLoggingLevel());
#endif // CONFIG_USE_LOCAL_STORAGE

    err = RunCommand(argc, argv);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(NotSpecified, "Run command failure: %s", chip::ErrorStr(err)));

exit:
    return (err == CHIP_NO_ERROR) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int Commands::RunInteractive(const char * command, const chip::Optional<char *> & storageDirectory, bool advertiseOperational)
{
    std::vector<std::string> arguments;
    VerifyOrReturnValue(DecodeArgumentsFromInteractiveMode(command, arguments), EXIT_FAILURE);

    if (arguments.size() > (kInteractiveModeArgumentsMaxLength - 1 /* for interactive mode name */))
    {
        ChipLogError(NotSpecified, "Too many arguments. Ignoring.");
        arguments.resize(kInteractiveModeArgumentsMaxLength - 1);
    }

    int argc                                        = 0;
    char * argv[kInteractiveModeArgumentsMaxLength] = {};
    argv[argc++]                                    = kInteractiveModeName;

    std::string commandStr;
    for (auto & arg : arguments)
    {
        argv[argc] = new char[arg.size() + 1];
        strcpy(argv[argc++], arg.c_str());
        commandStr += arg;
        commandStr += " ";
    }

    ChipLogProgress(NotSpecified, "Command: %s", commandStr.c_str());
    auto err = RunCommand(argc, argv, true, storageDirectory, advertiseOperational);

    // Do not delete arg[0]
    for (auto i = 1; i < argc; i++)
    {
        delete[] argv[i];
    }

    return (err == CHIP_NO_ERROR) ? EXIT_SUCCESS : EXIT_FAILURE;
}

CHIP_ERROR Commands::RunCommand(int argc, char ** argv, bool interactive,
                                const chip::Optional<char *> & interactiveStorageDirectory, bool interactiveAdvertiseOperational)
{
    Command * command = nullptr;

    if (argc <= 1)
    {
        ChipLogError(NotSpecified, "Missing cluster or command set name");
        ShowCommandSets(argv[0]);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    auto commandSetIter = GetCommandSet(argv[1]);
    if (commandSetIter == mCommandSets.end())
    {
        ChipLogError(NotSpecified, "Unknown cluster or command set: %s", argv[1]);
        ShowCommandSets(argv[0]);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    auto & commandList = commandSetIter->second.commands;
    auto * helpText    = commandSetIter->second.helpText;

    if (argc <= 2)
    {
        ChipLogError(NotSpecified, "Missing command name");
        ShowCommandSet(argv[0], argv[1], commandList, helpText);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    bool isGlobalCommand = IsGlobalCommand(argv[2]);
    if (!isGlobalCommand)
    {
        command = GetCommand(commandList, argv[2]);
        if (command == nullptr)
        {
            ChipLogError(NotSpecified, "Unknown command: %s", argv[2]);
            ShowCommandSet(argv[0], argv[1], commandList, helpText);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    else if (IsEventCommand(argv[2]))
    {
        if (argc <= 3)
        {
            ChipLogError(NotSpecified, "Missing event name");
            ShowClusterEvents(argv[0], argv[1], argv[2], commandList);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        command = GetGlobalCommand(commandList, argv[2], argv[3]);
        if (command == nullptr)
        {
            ChipLogError(NotSpecified, "Unknown event: %s", argv[3]);
            ShowClusterEvents(argv[0], argv[1], argv[2], commandList);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    else
    {
        if (argc <= 3)
        {
            ChipLogError(NotSpecified, "Missing attribute name");
            ShowClusterAttributes(argv[0], argv[1], argv[2], commandList);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        command = GetGlobalCommand(commandList, argv[2], argv[3]);
        if (command == nullptr)
        {
            ChipLogError(NotSpecified, "Unknown attribute: %s", argv[3]);
            ShowClusterAttributes(argv[0], argv[1], argv[2], commandList);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    int argumentsPosition = isGlobalCommand ? 4 : 3;
    if (!command->InitArguments(argc - argumentsPosition, &argv[argumentsPosition]))
    {
        if (interactive)
        {
            DetectAndLogMismatchedDoubleQuotes(argc - argumentsPosition, &argv[argumentsPosition]);
        }
        ShowCommand(argv[0], argv[1], command);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (interactive)
    {
        return command->RunAsInteractive(interactiveStorageDirectory, interactiveAdvertiseOperational);
    }

    // Now that the command is initialized, get our storage from it as needed
    // and set up our loging level.
#ifdef CONFIG_USE_LOCAL_STORAGE
    CHIP_ERROR err = mStorage.Init(nullptr, command->GetStorageDirectory().ValueOr(nullptr));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Init Storage failure: %s", chip::ErrorStr(err));
        return err;
    }

    chip::Logging::SetLogFilter(mStorage.GetLoggingLevel());

#if !CHIP_DISABLE_PLATFORM_KVS
    UseStorageDirectory(chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl(), mStorage.GetDirectory());
#endif // !CHIP_DISABLE_PLATFORM_KVS

#endif // CONFIG_USE_LOCAL_STORAGE

    return command->Run();
}

Commands::CommandSetMap::iterator Commands::GetCommandSet(std::string commandSetName)
{
    for (auto & commandSet : mCommandSets)
    {
        std::string key(commandSet.first);
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);

        if (key.compare(commandSetName) == 0)
        {
            return mCommandSets.find(commandSet.first);
        }
    }

    return mCommandSets.end();
}

Command * Commands::GetCommandByName(std::string commandSetName, std::string commandName)
{
    auto commandSetIter = GetCommandSet(commandSetName);
    if (commandSetIter != mCommandSets.end())
    {
        auto & commandList = commandSetIter->second.commands;
        for (auto & command : commandList)
        {
            if (command->GetName() == commandName)
            {
                return command.get();
            }
        }
    }
    return nullptr;
}

Command * Commands::GetCommand(CommandsVector & commands, std::string commandName)
{
    for (auto & command : commands)
    {
        if (commandName.compare(command->GetName()) == 0)
        {
            return command.get();
        }
    }

    return nullptr;
}

Command * Commands::GetGlobalCommand(CommandsVector & commands, std::string commandName, std::string attributeName)
{
    for (auto & command : commands)
    {
        if (commandName.compare(command->GetName()) == 0 && attributeName.compare(command->GetAttribute()) == 0)
        {
            return command.get();
        }
    }

    return nullptr;
}

bool Commands::IsAttributeCommand(std::string commandName) const
{
    return commandName.compare("read") == 0 || commandName.compare("write") == 0 || commandName.compare("force-write") == 0 ||
        commandName.compare("subscribe") == 0;
}

bool Commands::IsEventCommand(std::string commandName) const
{
    return commandName.compare("read-event") == 0 || commandName.compare("subscribe-event") == 0;
}

bool Commands::IsGlobalCommand(std::string commandName) const
{
    return IsAttributeCommand(commandName) || IsEventCommand(commandName);
}

void Commands::ShowCommandSetOverview(std::string commandSetName, const CommandSet & commandSet)
{
    std::transform(commandSetName.begin(), commandSetName.end(), commandSetName.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    fprintf(stderr, "  | * %-82s|\n", commandSetName.c_str());
    ShowHelpText(commandSet.helpText);
}

void Commands::ShowCommandSets(std::string executable)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s cluster_name command_name [param1 param2 ...]\n", executable.c_str());
    fprintf(stderr, "or:\n");
    fprintf(stderr, "  %s command_set_name command_name [param1 param2 ...]\n", executable.c_str());
    fprintf(stderr, "\n");
    // Table of clusters
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | Clusters:                                                                           |\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    for (auto & commandSet : mCommandSets)
    {
        if (commandSet.second.isCluster)
        {
            ShowCommandSetOverview(commandSet.first, commandSet.second);
        }
    }
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "\n");

    // Table of command sets
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | Command sets:                                                                       |\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    for (auto & commandSet : mCommandSets)
    {
        if (!commandSet.second.isCluster)
        {
            ShowCommandSetOverview(commandSet.first, commandSet.second);
        }
    }
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
}

void Commands::ShowCommandSet(std::string executable, std::string commandSetName, CommandsVector & commands, const char * helpText)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s %s command_name [param1 param2 ...]\n", executable.c_str(), commandSetName.c_str());

    if (helpText)
    {
        fprintf(stderr, "\n%s\n", helpText);
    }

    fprintf(stderr, "\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | Commands:                                                                           |\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    bool readCommand           = false;
    bool writeCommand          = false;
    bool writeOverrideCommand  = false;
    bool subscribeCommand      = false;
    bool readEventCommand      = false;
    bool subscribeEventCommand = false;
    for (auto & command : commands)
    {
        bool shouldPrint = true;

        if (IsGlobalCommand(command->GetName()))
        {
            if (strcmp(command->GetName(), "read") == 0 && !readCommand)
            {
                readCommand = true;
            }
            else if (strcmp(command->GetName(), "write") == 0 && !writeCommand)
            {
                writeCommand = true;
            }
            else if (strcmp(command->GetName(), "force-write") == 0 && !writeOverrideCommand)
            {
                writeOverrideCommand = true;
            }
            else if (strcmp(command->GetName(), "subscribe") == 0 && !subscribeCommand)
            {
                subscribeCommand = true;
            }
            else if (strcmp(command->GetName(), "read-event") == 0 && !readEventCommand)
            {
                readEventCommand = true;
            }
            else if (strcmp(command->GetName(), "subscribe-event") == 0 && !subscribeEventCommand)
            {
                subscribeEventCommand = true;
            }
            else
            {
                shouldPrint = false;
            }
        }

        if (shouldPrint)
        {
            fprintf(stderr, "  | * %-82s|\n", command->GetName());
            ShowHelpText(command->GetHelpText());
        }
    }
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
}

void Commands::ShowClusterAttributes(std::string executable, std::string clusterName, std::string commandName,
                                     CommandsVector & commands)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s %s %s attribute-name [param1 param2 ...]\n", executable.c_str(), clusterName.c_str(),
            commandName.c_str());
    fprintf(stderr, "\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | Attributes:                                                                         |\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    for (auto & command : commands)
    {
        if (commandName.compare(command->GetName()) == 0)
        {
            fprintf(stderr, "  | * %-82s|\n", command->GetAttribute());
        }
    }
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
}

void Commands::ShowClusterEvents(std::string executable, std::string clusterName, std::string commandName,
                                 CommandsVector & commands)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s %s %s event-name [param1 param2 ...]\n", executable.c_str(), clusterName.c_str(), commandName.c_str());
    fprintf(stderr, "\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | Events:                                                                             |\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    for (auto & command : commands)
    {
        if (commandName.compare(command->GetName()) == 0)
        {
            fprintf(stderr, "  | * %-82s|\n", command->GetEvent());
        }
    }
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
}

void Commands::ShowCommand(std::string executable, std::string clusterName, Command * command)
{
    fprintf(stderr, "Usage:\n");

    std::string arguments;
    std::string description;
    arguments += command->GetName();

    if (command->GetReadOnlyGlobalCommandArgument())
    {
        arguments += ' ';
        arguments += command->GetReadOnlyGlobalCommandArgument();
    }

    size_t argumentsCount = command->GetArgumentsCount();
    for (size_t i = 0; i < argumentsCount; i++)
    {
        std::string arg;
        bool isOptional = command->GetArgumentIsOptional(i);
        if (isOptional)
        {
            arg += "[--";
        }
        arg += command->GetArgumentName(i);
        if (isOptional)
        {
            arg += "]";
        }
        arguments += " ";
        arguments += arg;

        const char * argDescription = command->GetArgumentDescription(i);
        if ((argDescription != nullptr) && (strlen(argDescription) > 0))
        {
            description += "\n";
            description += arg;
            description += ":\n  ";
            description += argDescription;
            description += "\n";
        }
    }
    fprintf(stderr, "  %s %s %s\n", executable.c_str(), clusterName.c_str(), arguments.c_str());

    if (command->GetHelpText())
    {
        fprintf(stderr, "\n%s\n", command->GetHelpText());
    }

    if (description.size() > 0)
    {
        fprintf(stderr, "%s\n", description.c_str());
    }
}

bool Commands::DecodeArgumentsFromInteractiveMode(const char * command, std::vector<std::string> & args)
{
    // Remote clients may not know the ordering of arguments, so instead of a strict ordering arguments can
    // be passed in as a json payload encoded in base64 and are reordered on the fly.
    return IsJsonString(command) ? DecodeArgumentsFromBase64EncodedJson(command, args)
                                 : DecodeArgumentsFromStringStream(command, args);
}

bool Commands::DecodeArgumentsFromBase64EncodedJson(const char * json, std::vector<std::string> & args)
{
    Json::Value jsonValue;
    bool parsed = JsonParser::ParseCustomArgument(json, json + kJsonStringPrefixLen, jsonValue);
    VerifyOrReturnValue(parsed, false, ChipLogError(NotSpecified, "Error while parsing json."));
    VerifyOrReturnValue(jsonValue.isObject(), false, ChipLogError(NotSpecified, "Unexpected json type."));
    VerifyOrReturnValue(jsonValue.isMember(kJsonClusterKey), false,
                        ChipLogError(NotSpecified, "'%s' key not found in json.", kJsonClusterKey));
    VerifyOrReturnValue(jsonValue.isMember(kJsonCommandKey), false,
                        ChipLogError(NotSpecified, "'%s' key not found in json.", kJsonCommandKey));
    VerifyOrReturnValue(jsonValue.isMember(kJsonArgumentsKey), false,
                        ChipLogError(NotSpecified, "'%s' key not found in json.", kJsonArgumentsKey));
    VerifyOrReturnValue(IsBase64String(jsonValue[kJsonArgumentsKey].asString().c_str()), false,
                        ChipLogError(NotSpecified, "'arguments' is not a base64 string."));

    auto clusterName = jsonValue[kJsonClusterKey].asString();
    auto commandName = jsonValue[kJsonCommandKey].asString();
    auto arguments   = jsonValue[kJsonArgumentsKey].asString();

    auto clusterIter = GetCommandSet(clusterName);
    VerifyOrReturnValue(clusterIter != mCommandSets.end(), false,
                        ChipLogError(NotSpecified, "Cluster '%s' is not supported.", clusterName.c_str()));

    auto & commandList = clusterIter->second.commands;

    auto command = GetCommand(commandList, commandName);

    if (jsonValue.isMember(kJsonCommandSpecifierKey) && IsGlobalCommand(commandName))
    {
        auto commandSpecifierName = jsonValue[kJsonCommandSpecifierKey].asString();
        command                   = GetGlobalCommand(commandList, commandName, commandSpecifierName);
    }
    VerifyOrReturnValue(nullptr != command, false, ChipLogError(NotSpecified, "Unknown command."));

    auto encodedData = arguments.c_str();
    encodedData += kBase64StringPrefixLen;

    size_t encodedDataSize        = strlen(encodedData);
    size_t expectedMaxDecodedSize = BASE64_MAX_DECODED_LEN(encodedDataSize);

    chip::Platform::ScopedMemoryBuffer<uint8_t> decodedData;
    VerifyOrReturnValue(decodedData.Calloc(expectedMaxDecodedSize + 1 /* for null */), false);

    size_t decodedDataSize = chip::Base64Decode(encodedData, static_cast<uint16_t>(encodedDataSize), decodedData.Get());
    VerifyOrReturnValue(decodedDataSize != 0, false, ChipLogError(NotSpecified, "Error while decoding base64 data."));

    decodedData.Get()[decodedDataSize] = '\0';

    Json::Value jsonArguments;
    bool parsedArguments = JsonParser::ParseCustomArgument(encodedData, chip::Uint8::to_char(decodedData.Get()), jsonArguments);
    VerifyOrReturnValue(parsedArguments, false, ChipLogError(NotSpecified, "Error while parsing json."));
    VerifyOrReturnValue(jsonArguments.isObject(), false, ChipLogError(NotSpecified, "Unexpected json type, expects and object."));

    std::vector<std::string> mandatoryArguments;
    std::vector<std::string> optionalArguments;
    VerifyOrReturnValue(GetArgumentsFromJson(command, jsonArguments, false /* addOptional */, mandatoryArguments), false);
    VerifyOrReturnValue(GetArgumentsFromJson(command, jsonArguments, true /* addOptional */, optionalArguments), false);

    args.push_back(std::move(clusterName));
    args.push_back(std::move(commandName));
    if (jsonValue.isMember(kJsonCommandSpecifierKey))
    {
        auto commandSpecifierName = jsonValue[kJsonCommandSpecifierKey].asString();
        args.push_back(std::move(commandSpecifierName));
    }
    args.insert(args.end(), mandatoryArguments.begin(), mandatoryArguments.end());
    args.insert(args.end(), optionalArguments.begin(), optionalArguments.end());

    return true;
}

bool Commands::DecodeArgumentsFromStringStream(const char * command, std::vector<std::string> & args)
{
    std::string arg;
    std::stringstream ss(command);
    while (ss >> std::quoted(arg, '\''))
    {
        args.push_back(std::move(arg));
    }

    return true;
}

void Commands::ShowHelpText(const char * helpText)
{
    if (helpText == nullptr)
    {
        return;
    }

    // We leave 82 chars for command/cluster names.  The help text starts
    // two chars further to the right, so there are 80 chars left
    // for it.
    if (strlen(helpText) > 80)
    {
        // Add "..." at the end to indicate truncation, and only
        // show the first 77 chars, since that's what will fit.
        fprintf(stderr, "  |   - %.77s...|\n", helpText);
    }
    else
    {
        fprintf(stderr, "  |   - %-80s|\n", helpText);
    }
}
