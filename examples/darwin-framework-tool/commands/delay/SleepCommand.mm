/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "SleepCommand.h"
#include <chrono>
#include <thread>

CHIP_ERROR SleepCommand::RunCommand()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(mDurationInMs));
    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}
