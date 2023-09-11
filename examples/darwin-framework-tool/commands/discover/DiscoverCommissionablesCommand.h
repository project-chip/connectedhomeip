/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "../common/CHIPCommandBridge.h"

class DiscoverCommissionablesStartCommand : public CHIPCommandBridge
{
public:
    DiscoverCommissionablesStartCommand() : CHIPCommandBridge("start") {}

protected:
    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(30); }
};

class DiscoverCommissionablesStopCommand : public CHIPCommandBridge
{
public:
    DiscoverCommissionablesStopCommand() : CHIPCommandBridge("stop") {}

protected:
    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }
};

class DiscoverCommissionablesListCommand : public CHIPCommandBridge
{
public:
    DiscoverCommissionablesListCommand() : CHIPCommandBridge("list") {}

protected:
    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }
};
