/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Contains the functions for compatibility with ember ZCL inner state
 *          when calling ember callbacks.
 */

#pragma once

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-types.h>
#include <lib/core/CHIPCore.h>

namespace chip {
namespace app {
namespace Compatibility {

void SetupEmberAfCommandHandler(CommandHandler * command, const ConcreteCommandPath & commandPath);
bool IMEmberAfSendDefaultResponseWithCallback(EmberAfStatus status);
void ResetEmberAfObjects();

} // namespace Compatibility
} // namespace app
} // namespace chip
