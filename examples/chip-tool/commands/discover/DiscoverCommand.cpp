/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "DiscoverCommand.h"

CHIP_ERROR DiscoverCommand::RunCommand()
{
    return RunCommand(mNodeId, mFabricId);
}
