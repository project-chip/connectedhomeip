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

#include "Commands.h"

#include "Command.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

#include <lib/support/Base64.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

#include "../clusters/JsonParser.h"

namespace {

char kInteractiveModeName[]                         = "";
constexpr size_t kInteractiveModeArgumentsMaxLength = 32;
constexpr const char * kOptionalArgumentPrefix      = "--";
constexpr const char * kJsonClusterKey              = "cluster";
constexpr const char * kJsonCommandKey              = "command";
constexpr const char * kJsonCommandSpecifierKey     = "command_specifier";
constexpr const char * kJsonArgumentsKey            = "arguments";

std::vector<std::string> GetArgumentsFromJson(Command * command, Json::Value & value, bool optional)
{
    std::vector<std::string> args;
    for (size_t i = 0; i < command->GetArgumentsCount(); i++)
    {
        auto argName = command->GetArgumentName(i);
        for (auto const & memberName : value.getMemberNames())
        {
            if (strcasecmp(argName, memberName.c_str()) != 0)
            {
                continue;
            }

            if (command->GetArgumentIsOptional(i) != optional)
            {
                continue;
            }

            if (optional)
            {
                args.push_back(std::string(kOptionalArgumentPrefix) + argName);
            }

            auto argValue = value[memberName].asString();
            args.push_back(std::move(argValue));
            break;
        }
    }
    return args;
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
            ChipLogError(chipTool,
                         "Mismatched '\"' detected in argument: '%s'.  Use single quotes to delimit arguments with spaces "
                         "in them: 'x y', not \"x y\".",
                         arg);
        }
    }
}

} // namespace

void Commands::Register(const char * clusterName, commands_list commandsList)
{
    for (auto & command : commandsList)
    {
        mClusters[clusterName].push_back(std::move(command));
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
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Run command failure: %s", chip::ErrorStr(err)));

exit:
    return (err == CHIP_NO_ERROR) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int Commands::RunInteractive(const char * command)
{
    std::vector<std::string> arguments;
    VerifyOrReturnValue(DecodeArgumentsFromInteractiveMode(command, arguments), EXIT_FAILURE);

    if (arguments.size() > (kInteractiveModeArgumentsMaxLength - 1 /* for interactive mode name */))
    {
        ChipLogError(chipTool, "Too many arguments. Ignoring.");
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

    ChipLogProgress(chipTool, "Command: %s", commandStr.c_str());
    auto err = RunCommand(argc, argv, true);

    // Do not delete arg[0]
    for (auto i = 1; i < argc; i++)
    {
        delete[] argv[i];
    }

    VerifyOrReturnValue(CHIP_NO_ERROR == err, EXIT_FAILURE, ChipLogError(chipTool, "Run command failure: %s", chip::ErrorStr(err)));

    return EXIT_SUCCESS;
}

CHIP_ERROR Commands::RunCommand(int argc, char ** argv, bool interactive)
{
    std::map<std::string, CommandsVector>::iterator cluster;
    Command * command = nullptr;

    if (argc <= 1)
    {
        ChipLogError(chipTool, "Missing cluster name");
        ShowClusters(argv[0]);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    cluster = GetCluster(argv[1]);
    if (cluster == mClusters.end())
    {
        ChipLogError(chipTool, "Unknown cluster: %s", argv[1]);
        ShowClusters(argv[0]);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (argc <= 2)
    {
        ChipLogError(chipTool, "Missing command name");
        ShowCluster(argv[0], argv[1], cluster->second);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    bool isGlobalCommand = IsGlobalCommand(argv[2]);
    if (!isGlobalCommand)
    {
        command = GetCommand(cluster->second, argv[2]);
        if (command == nullptr)
        {
            ChipLogError(chipTool, "Unknown command: %s", argv[2]);
            ShowCluster(argv[0], argv[1], cluster->second);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    else if (IsEventCommand(argv[2]))
    {
        if (argc <= 3)
        {
            ChipLogError(chipTool, "Missing event name");
            ShowClusterEvents(argv[0], argv[1], argv[2], cluster->second);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        command = GetGlobalCommand(cluster->second, argv[2], argv[3]);
        if (command == nullptr)
        {
            ChipLogError(chipTool, "Unknown event: %s", argv[3]);
            ShowClusterEvents(argv[0], argv[1], argv[2], cluster->second);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    else
    {
        if (argc <= 3)
        {
            ChipLogError(chipTool, "Missing attribute name");
            ShowClusterAttributes(argv[0], argv[1], argv[2], cluster->second);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        command = GetGlobalCommand(cluster->second, argv[2], argv[3]);
        if (command == nullptr)
        {
            ChipLogError(chipTool, "Unknown attribute: %s", argv[3]);
            ShowClusterAttributes(argv[0], argv[1], argv[2], cluster->second);
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

    return interactive ? command->RunAsInteractive() : command->Run();
}

std::map<std::string, Commands::CommandsVector>::iterator Commands::GetCluster(std::string clusterName)
{
    for (auto & cluster : mClusters)
    {
        std::string key(cluster.first);
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        if (key.compare(clusterName) == 0)
        {
            return mClusters.find(cluster.first);
        }
    }

    return mClusters.end();
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

void Commands::ShowClusters(std::string executable)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s cluster_name command_name [param1 param2 ...]\n", executable.c_str());
    fprintf(stderr, "\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | Clusters:                                                                           |\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    for (auto & cluster : mClusters)
    {
        std::string clusterName(cluster.first);
        std::transform(clusterName.begin(), clusterName.end(), clusterName.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        fprintf(stderr, "  | * %-82s|\n", clusterName.c_str());
    }
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
}

void Commands::ShowCluster(std::string executable, std::string clusterName, CommandsVector & commands)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s %s command_name [param1 param2 ...]\n", executable.c_str(), clusterName.c_str());
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
    VerifyOrReturnValue(parsed, false, ChipLogError(chipTool, "Error while parsing json."));
    VerifyOrReturnValue(jsonValue.isObject(), false, ChipLogError(chipTool, "Unexpected json type."));
    VerifyOrReturnValue(jsonValue.isMember(kJsonClusterKey), false,
                        ChipLogError(chipTool, "'%s' key not found in json.", kJsonClusterKey));
    VerifyOrReturnValue(jsonValue.isMember(kJsonCommandKey), false,
                        ChipLogError(chipTool, "'%s' key not found in json.", kJsonCommandKey));
    VerifyOrReturnValue(jsonValue.isMember(kJsonArgumentsKey), false,
                        ChipLogError(chipTool, "'%s' key not found in json.", kJsonArgumentsKey));
    VerifyOrReturnValue(IsBase64String(jsonValue[kJsonArgumentsKey].asString().c_str()), false,
                        ChipLogError(chipTool, "'arguments' is not a base64 string."));

    auto clusterName = jsonValue[kJsonClusterKey].asString();
    auto commandName = jsonValue[kJsonCommandKey].asString();
    auto arguments   = jsonValue[kJsonArgumentsKey].asString();

    auto cluster = GetCluster(clusterName);
    VerifyOrReturnValue(cluster != mClusters.end(), false,
                        ChipLogError(chipTool, "Cluster '%s' is not supported.", clusterName.c_str()));

    auto command = GetCommand(cluster->second, commandName);

    if (jsonValue.isMember(kJsonCommandSpecifierKey) && IsGlobalCommand(commandName))
    {
        auto commandSpecifierName = jsonValue[kJsonCommandSpecifierKey].asString();
        command                   = GetGlobalCommand(cluster->second, commandName, commandSpecifierName);
    }
    VerifyOrReturnValue(nullptr != command, false, ChipLogError(chipTool, "Unknown command."));

    auto encodedData = arguments.c_str();
    encodedData += kBase64StringPrefixLen;

    size_t encodedDataSize        = strlen(encodedData);
    size_t expectedMaxDecodedSize = BASE64_MAX_DECODED_LEN(encodedDataSize);

    chip::Platform::ScopedMemoryBuffer<uint8_t> decodedData;
    VerifyOrReturnValue(decodedData.Calloc(expectedMaxDecodedSize + 1 /* for null */), false);

    size_t decodedDataSize = chip::Base64Decode(encodedData, static_cast<uint16_t>(encodedDataSize), decodedData.Get());
    VerifyOrReturnValue(decodedDataSize != 0, false, ChipLogError(chipTool, "Error while decoding base64 data."));

    decodedData.Get()[decodedDataSize] = '\0';

    Json::Value jsonArguments;
    bool parsedArguments = JsonParser::ParseCustomArgument(encodedData, chip::Uint8::to_char(decodedData.Get()), jsonArguments);
    VerifyOrReturnValue(parsedArguments, false, ChipLogError(chipTool, "Error while parsing json."));
    VerifyOrReturnValue(jsonArguments.isObject(), false, ChipLogError(chipTool, "Unexpected json type, expects and object."));

    auto mandatoryArguments = GetArgumentsFromJson(command, jsonArguments, false /* addOptional */);
    auto optionalArguments  = GetArgumentsFromJson(command, jsonArguments, true /* addOptional */);

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
