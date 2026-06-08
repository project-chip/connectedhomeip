/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include <ICDShellCommands.h>
#include <app/icd/server/ICDNotifier.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

namespace {

Engine sShellICDSubCommands;

#if defined(CHIP_CONFIG_ENABLE_ICD_DSLS) && CHIP_CONFIG_ENABLE_ICD_DSLS
Engine sShellDynamicSitLitSubCommands;
#endif // defined(CHIP_CONFIG_ENABLE_ICD_DSLS) && CHIP_CONFIG_ENABLE_ICD_DSLS

CHIP_ERROR HelpHandler(int argc, char ** argv)
{
    sShellICDSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return HelpHandler(argc, argv);
    }

    return sShellICDSubCommands.ExecCommand(argc, argv);
}

#if defined(CHIP_CONFIG_ENABLE_ICD_DSLS) && CHIP_CONFIG_ENABLE_ICD_DSLS
namespace DynamicSitLit {

CHIP_ERROR HelpHandler(int argc, char ** argv)
{
    sShellDynamicSitLitSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return DynamicSitLit::HelpHandler(argc, argv);
    }

    return sShellDynamicSitLitSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR SetSitModeReq(int argc, char ** argv)
{
    return chip::DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t arg) { chip::app::ICDNotifier::GetInstance().NotifySITModeRequestNotification(); }, 0);
}

CHIP_ERROR RemoveSitModeReq(int argc, char ** argv)
{
    return chip::DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t arg) { chip::app::ICDNotifier::GetInstance().NotifySITModeRequestWithdrawal(); }, 0);
}

} // namespace DynamicSitLit
#endif // defined(CHIP_CONFIG_ENABLE_ICD_DSLS) && CHIP_CONFIG_ENABLE_ICD_DSLS

} // namespace

namespace ICDCommands {

/**
 * @brief configures ICD matter shell
 */
void RegisterCommands()
{
    static const shell_command_t sLitICDSubCommands[] = {
#if defined(CHIP_CONFIG_ENABLE_ICD_DSLS) && CHIP_CONFIG_ENABLE_ICD_DSLS
        { DynamicSitLit::CommandHandler, "dsls", "Dynamic Sit/Lit commands. Usage: dsls <subcommand>" },
#endif // defined(CHIP_CONFIG_ENABLE_ICD_DSLS) && CHIP_CONFIG_ENABLE_ICD_DSLS
        { &HelpHandler, "help", "Usage: icd <subcommand>" }
    };
    sShellICDSubCommands.RegisterCommands(sLitICDSubCommands, MATTER_ARRAY_SIZE(sLitICDSubCommands));

#if defined(CHIP_CONFIG_ENABLE_ICD_DSLS) && CHIP_CONFIG_ENABLE_ICD_DSLS
    static const shell_command_t sDynamicSitLitSubCommands[] = {
        { &DynamicSitLit::SetSitModeReq, "add", "Add SIT mode requirement." },
        { &DynamicSitLit::RemoveSitModeReq, "remove", "Removes SIT mode requirement." },
        { &DynamicSitLit::HelpHandler, "help", "Usage : icd dsls <subcommand>." }
    };
    sShellDynamicSitLitSubCommands.RegisterCommands(sDynamicSitLitSubCommands, MATTER_ARRAY_SIZE(sDynamicSitLitSubCommands));
#endif // defined(CHIP_CONFIG_ENABLE_ICD_DSLS) && CHIP_CONFIG_ENABLE_ICD_DSLS

    static const shell_command_t sICDCommand = { &CommandHandler, "icd", "ICD commands. Usage: icd <subcommand>" };
    Engine::Root().RegisterCommands(&sICDCommand, 1);
}

} // namespace ICDCommands
