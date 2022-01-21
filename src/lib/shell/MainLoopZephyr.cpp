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
#include <map>
#include <shell/shell.h>
#include <stdio.h>
#include <string>
#include <vector>

#include <lib/core/CHIPError.h>
#include <lib/shell/Engine.h>
#include <lib/shell/streamer_zephyr.h>
#include <lib/support/CodeUtils.h>

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

    std::string sub_prefix_str = prefix;

    // concatenate prefix and syntax to construct the prefix string for command suggestion.

    if (strcmp(prefix, "") != 0)
    {
        sub_prefix_str += " ";
    }

    sub_prefix_str += syntax;

    const char * sub_prefix = sub_prefix_str.c_str();

    std::vector<shell_command_t *> subcmds = Engine::GetCommandSuggestions(sub_prefix);

    const shell_static_entry * static_entry;

    if (subcmds.size() > 0)
    {
        // Non-leaf node, recursively build tree for each child node
        const struct shell_static_entry * sub_static_entry = new shell_static_entry[subcmds.size() + 1];
        for (size_t i = 0; i < subcmds.size(); i++)
        {
            const struct shell_static_entry * sub_entry =
                build_command_tree(sub_prefix, subcmds[i]->cmd_name, subcmds[i]->cmd_help);
            memcpy((shell_static_entry *) &sub_static_entry[i], sub_entry, sizeof(struct shell_static_entry));
        }

        // Zephyr's shell_cmd_entry.u.entry needs an additional NULL element to terminate the array.
        memset((shell_static_entry *) &sub_static_entry[subcmds.size()], 0, sizeof(struct shell_static_entry));

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
        // the leaf nodes should have 0 required arguments but accepts a few (set 10 here) optional arguments.
        static_entry =
            new const shell_static_entry{ .syntax = syntax, .help = help, .subcmd = NULL, .handler = NULL, .args = { 0, 10 } };
    }

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
