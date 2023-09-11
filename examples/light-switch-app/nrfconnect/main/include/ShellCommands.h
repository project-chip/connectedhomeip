/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef CONFIG_CHIP_LIB_SHELL
#include <lib/core/CHIPError.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>

namespace SwitchCommands {

void RegisterSwitchCommands();

} // namespace SwitchCommands

#endif
