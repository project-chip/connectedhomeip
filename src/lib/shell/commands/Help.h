/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Header that defines default shell commands for CHIP examples
 */

#pragma once

#include <lib/shell/Engine.h>

namespace chip {
namespace Shell {

CHIP_ERROR PrintCommandHelp(shell_command_t * command, void * arg);

} // namespace Shell
} // namespace chip
