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
#include <string>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

#include <lib/support/CHIPArgParser.hpp>

#include <core/CHIPBuildConfig.h>

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#include "TraceHandlers.h"
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

namespace {

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
enum
{
    kOptionTraceFile = 0x1000,
    kOptionTraceLog,
};

bool HandleTraceOptions(const char * program, chip::ArgParser::OptionSet * options, int identifier, const char * name,
                        const char * value)
{
    switch (identifier)
    {
    case kOptionTraceLog:
        chip::trace::SetTraceStream(new chip::trace::TraceStreamLog());
        return true;
    case kOptionTraceFile:
        chip::trace::SetTraceStream(new chip::trace::TraceStreamFile(value));
        return true;
    default:
        chip::ArgParser::PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", program, name);
        return false;
    }
}

chip::ArgParser::OptionDef traceCmdLineOptionDefs[] = { { "trace_file", chip::ArgParser::kArgumentRequired, kOptionTraceFile },
                                                        { "trace_log", chip::ArgParser::kNoArgument, kOptionTraceLog },
                                                        {} };

const char * traceOptionHelp = "  --trace_file <file>\n"
                               "       Output trace data to the specified file.\n"
                               "  --trace_log\n"
                               "       Output trace data to the log stream.\n"
                               "\n";
chip::ArgParser::OptionSet traceCmdLineOptions = { HandleTraceOptions, traceCmdLineOptionDefs, "TRACE OPTIONS", traceOptionHelp };
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

const char * kAppName     = "chip-tool";
const char * kUsage       = "Usage: chip-tool [options] cluster command_name [param1] [param2]";
const char * kVersion     = nullptr; // Unknown
const char * kDescription = "A command line tool that uses Matter to send messages to a Matter server.";
chip::ArgParser::HelpOptions helpOptions(kAppName, kUsage, kVersion, kDescription);

chip::ArgParser::OptionSet * allOptions[] = {
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    &traceCmdLineOptions,
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    &helpOptions, nullptr
};

int gPositionalArgc       = 0;
char ** gPositionalArgv   = nullptr;
const char * gProgramName = nullptr;

bool GetPositionalArgs(const char * prog, int argc, char * argv[])
{
    gProgramName    = prog;
    gPositionalArgc = argc;
    gPositionalArgv = argv;
    return true;
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

    err = mStorage.Init();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Storage failure: %s", chip::ErrorStr(err)));

    chip::Logging::SetLogFilter(mStorage.GetLoggingLevel());

    VerifyOrExit(chip::ArgParser::ParseArgs("chip-tool", argc, argv, allOptions, GetPositionalArgs),
                 ChipLogError(chipTool, "Error parsing arguments"));

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::trace::InitTrace();
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

    err = RunCommand();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Run command failure: %s", chip::ErrorStr(err)));

exit:
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::trace::DeInitTrace();
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    return (err == CHIP_NO_ERROR) ? EXIT_SUCCESS : EXIT_FAILURE;
}

CHIP_ERROR Commands::RunCommand()
{
    std::map<std::string, CommandsVector>::iterator cluster;
    Command * command = nullptr;

    if (gPositionalArgc <= 0)
    {
        ChipLogError(chipTool, "Missing cluster name");
        ShowClusters(gProgramName);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    cluster = GetCluster(gPositionalArgv[0]);
    if (cluster == mClusters.end())
    {
        ChipLogError(chipTool, "Unknown cluster: %s", gPositionalArgv[1]);
        ShowClusters(gProgramName);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (gPositionalArgc <= 1)
    {
        ChipLogError(chipTool, "Missing command name");
        ShowCluster(gProgramName, gPositionalArgv[0], cluster->second);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!IsGlobalCommand(gPositionalArgv[1]))
    {
        command = GetCommand(cluster->second, gPositionalArgv[1]);
        if (command == nullptr)
        {
            ChipLogError(chipTool, "Unknown command: %s", gPositionalArgv[1]);
            ShowCluster(gProgramName, gPositionalArgv[0], cluster->second);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    else
    {
        if (gPositionalArgc <= 2)
        {
            ChipLogError(chipTool, "Missing attribute name");
            ShowClusterAttributes(gProgramName, gPositionalArgv[0], gPositionalArgv[1], cluster->second);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        command = GetGlobalCommand(cluster->second, gPositionalArgv[1], gPositionalArgv[2]);
        if (command == nullptr)
        {
            ChipLogError(chipTool, "Unknown attribute: %s", gPositionalArgv[2]);
            ShowClusterAttributes(gProgramName, gPositionalArgv[0], gPositionalArgv[1], cluster->second);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    if (!command->InitArguments(gPositionalArgc - 2, &gPositionalArgv[2]))
    {
        ShowCommand(gProgramName, gPositionalArgv[0], command);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return command->Run();
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

bool Commands::IsGlobalCommand(std::string commandName) const
{
    return commandName.compare("read") == 0 || commandName.compare("write") == 0 || commandName.compare("report") == 0;
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
    bool readCommand   = false;
    bool writeCommand  = false;
    bool reportCommand = false;
    for (auto & command : commands)
    {
        bool shouldPrint = true;

        if (IsGlobalCommand(command->GetName()))
        {
            if (strcmp(command->GetName(), "read") == 0 && readCommand == false)
            {
                readCommand = true;
            }
            else if (strcmp(command->GetName(), "write") == 0 && writeCommand == false)
            {
                writeCommand = true;
            }
            else if (strcmp(command->GetName(), "report") == 0 && reportCommand == false)
            {
                reportCommand = true;
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

void Commands::ShowCommand(std::string executable, std::string clusterName, Command * command)
{
    fprintf(stderr, "Usage:\n");

    std::string arguments = "";
    arguments += command->GetName();

    size_t argumentsCount = command->GetArgumentsCount();
    for (size_t i = 0; i < argumentsCount; i++)
    {
        arguments += " ";
        bool isOptional = command->GetArgumentIsOptional(i);
        if (isOptional)
        {
            arguments += "[--";
        }
        arguments += command->GetArgumentName(i);
        if (isOptional)
        {
            arguments += "]";
        }
    }
    fprintf(stderr, "  %s %s %s\n", executable.c_str(), clusterName.c_str(), arguments.c_str());
}
