/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <lib/support/CodeUtils.h>

#include <app-common/zap-generated/tests/simulated-cluster-objects.h>

class LogCommands
{
public:
    LogCommands(){};
    virtual ~LogCommands(){};

    virtual CHIP_ERROR ContinueOnChipMainThread(CHIP_ERROR err) = 0;

    CHIP_ERROR Log(const char * identity, const chip::app::Clusters::LogCommands::Commands::Log::Type & value);
    CHIP_ERROR UserPrompt(const char * identity, const chip::app::Clusters::LogCommands::Commands::UserPrompt::Type & value);
};
