/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../../config/PersistentStorage.h"
#include "../common/Command.h"

class CommissionedListCommand : public Command
{
public:
    CommissionedListCommand() : Command("list") {}
    CHIP_ERROR Run() override;

private:
    CHIP_ERROR PrintInformation();
    CHIP_ERROR PrintDeviceInformation(chip::NodeId deviceId);

    PersistentStorage mStorage;
};
