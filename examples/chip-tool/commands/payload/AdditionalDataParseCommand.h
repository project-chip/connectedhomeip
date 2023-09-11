/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "../common/Command.h"

class AdditionalDataParseCommand : public Command
{
public:
    AdditionalDataParseCommand() : Command("parse-additional-data-payload") { AddArgument("payload", &mPayload); }
    CHIP_ERROR Run() override;

private:
    char * mPayload;
};
