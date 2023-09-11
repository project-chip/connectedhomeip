/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "commands/common/Commands.h"
#include "commands/payload/AdditionalDataParseCommand.h"
#include "commands/payload/SetupPayloadGenerateCommand.h"
#include "commands/payload/SetupPayloadParseCommand.h"
#include "commands/payload/SetupPayloadVerhoeff.h"

void registerCommandsPayload(Commands & commands)
{
    const char * clusterName      = "Payload";
    commands_list clusterCommands = {
        make_unique<SetupPayloadGenerateQRCodeCommand>(),     //
        make_unique<SetupPayloadGenerateManualCodeCommand>(), //
        make_unique<SetupPayloadParseCommand>(),              //
        make_unique<AdditionalDataParseCommand>(),            //
        make_unique<SetupPayloadVerhoeffVerify>(),            //
        make_unique<SetupPayloadVerhoeffGenerate>(),          //
    };

    commands.RegisterCommandSet(clusterName, clusterCommands, "Commands for parsing and generating setup payloads.");
}
