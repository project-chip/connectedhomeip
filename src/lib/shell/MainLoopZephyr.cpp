/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <cstring>
#include <init.h>
#include <shell/shell.h>
#include <stdio.h>

#include <lib/core/CHIPError.h>
#include <lib/shell/Engine.h>
#include <lib/shell/streamer_zephyr.h>
#include <lib/support/CodeUtils.h>

using chip::Shell::cmd_completion_context;
using chip::Shell::Engine;
using chip::Shell::shell_command_t;
using chip::Shell::shell_map_t;

static int cmd_matter(const struct shell * shell, size_t argc, char ** argv)
{
    chip::Shell::streamer_set_shell(shell);
    return (Engine::Root().ExecCommand(argc - 1, argv + 1) == CHIP_NO_ERROR) ? 0 : -ENOEXEC;
}

/**
 * This recursive function obtains all sub-commands (possbile completions) for a command node
 * and build it into a tree of Zephyr's shell_static_entry structure.
 *
 * @param prefix                  The path/prefix until the root command node of the tree, without tailing space.
 *                                e.g. "dns browse". Set to emptry string "" if building tree for a root-level command.
 * @param syntax                  The single-word command name/syntax for the root node of the tree. e.g. "resolve".
 *                                Set to emptry string "" if calling to build from the root (to include all commands).
 * @param help                    The help text for the command at roote node of the tree. Set to NULL if none.
 *
 * @return                        The shell_static_entry structure tree that chains all the sub-commands from the
 *                                requested command node.
 *
 */
const struct shell_static_entry * build_command_tree(const char * prefix, const char * syntax, const char * help)
{

    char * sub_prefix = new char[strlen(prefix) + strlen(syntax) + 2];
    size_t pos        = 0;
    if (strcmp(prefix, "") != 0)
    {
        strncpy(&sub_prefix[pos], prefix, strlen(prefix));
        pos += strlen(prefix);
        strncpy(&sub_prefix[pos], " ", 1);
        pos += 1;
    }
    strncpy(&sub_prefix[pos], syntax, strlen(syntax) + 1);

    cmd_completion_context context = cmd_completion_context(sub_prefix);
    CHIP_ERROR ret                 = Engine::GetCommandCompletions(&context);

    const struct shell_static_entry * static_entry;
    if (ret == CHIP_NO_ERROR && context.cmdc != 0)
    {
        const struct shell_static_entry * sub_static_entry = new shell_static_entry[context.cmdc + 1];
        for (size_t i = 0; i < context.cmdc; i++)
        {
            shell_command_t * subcmd                    = context.cmdv[i];
            const struct shell_static_entry * sub_entry = build_command_tree(sub_prefix, subcmd->cmd_name, subcmd->cmd_help);
            if (sub_entry == nullptr)
            {
                // leaf node, the command should require no argument but can accept optional arguments
                const struct shell_static_entry leaf_entry = {
                    .syntax = subcmd->cmd_name, .help = subcmd->cmd_help, .subcmd = NULL, .handler = NULL, .args = { 0, 10 }
                };
                memcpy((shell_static_entry *) &sub_static_entry[i], &leaf_entry, sizeof(struct shell_static_entry));
                continue;
            }
            memcpy((shell_static_entry *) &sub_static_entry[i], sub_entry, sizeof(struct shell_static_entry));
        }

        // Zephyr's shell_cmd_entry.u.entry needs an additional NULL element to terminate the array.
        memset((shell_static_entry *) &sub_static_entry[context.cmdc], 0, sizeof(struct shell_static_entry));

        const struct shell_cmd_entry * sub_cmd_entry =
            new const shell_cmd_entry{ .is_dynamic = false,
                                       .u          = { .entry = (const struct shell_static_entry *) sub_static_entry } };

        // The non-leaf nodes should require at least 1 required argument (for its child)
        // Set the root command to use syntax "matter"
        // Only the root "matter" node should have a handler function in Zephry-shell, so that all commands are sent
        // to be dispatched by their registered Engine instances.
        static_entry = new const shell_static_entry{ .syntax  = (strcmp(syntax, "") == 0) ? "matter" : syntax,
                                                     .help    = help,
                                                     .subcmd  = (const struct shell_cmd_entry *) sub_cmd_entry,
                                                     .handler = (strcmp(syntax, "") == 0) ? cmd_matter : NULL,
                                                     .args    = { 1, 10 } };
    }
    else
    {
        static_entry = nullptr;
    }
    delete[] sub_prefix;
    return static_entry;
}

static void register_matter_command_to_zephyr()
{
    static const struct shell_static_entry _shell_matter = *build_command_tree("", "", "Matter commands");
    static const struct shell_cmd_entry shell_cmd_matter __attribute__((section("." STRINGIFY(shell_root_cmd_matter))))
    __attribute__((used)) = { .is_dynamic = false, .u = { .entry = &_shell_matter } };
}

static int RegisterCommands(const struct device * dev)
{
    Engine::Root().RegisterDefaultCommands();
    register_matter_command_to_zephyr();
    return 0;
}
SYS_INIT(RegisterCommands, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

namespace chip {
namespace Shell {

void Engine::RunMainLoop()
{
    // Intentionally empty as Zephyr has own thread handling shell
}

} // namespace Shell
} // namespace chip
