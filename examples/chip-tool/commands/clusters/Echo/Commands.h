/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef __CHIPTOOL_ECHO_COMMANDS_H__
#define __CHIPTOOL_ECHO_COMMANDS_H__

#include "../../common/ModelCommand.h"

static const char * PAYLOAD = "Message from Standalone CHIP echo client!";

class Echo : public ModelCommand
{
public:
    Echo(const uint16_t clusterId) : ModelCommand("echo", clusterId) {}
    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeEchoCommand(buffer->Start(), bufferSize, endPointId, PAYLOAD);
    }

    bool HandleClusterResponse(uint8_t * message, uint16_t messageLen) const override
    {
        char * msg = (char *) malloc(strlen(PAYLOAD) + 1);
        uint8_t msgLen;

        CHECK_MESSAGE_LENGTH(messageLen == strlen(PAYLOAD) + 1);

        msgLen = chip::Encoding::Read8(message);
        VerifyOrExit(msgLen == strlen(PAYLOAD),
                     ChipLogError(chipTool, "Echo error: message length is `%d', should be: `%d'", msgLen, strlen(PAYLOAD)));

        memcpy(msg, message, msgLen);
        VerifyOrExit(strcmp(msg, PAYLOAD) == 0, ChipLogError(chipTool, "Echo error: echo does not match: %s", msg));

        ChipLogProgress(chipTool, "Echo response %s (%d)", msg, msgLen);

    exit:
        free(msg);
        return true;
    }
};

void registerClusterEcho(Commands & commands)
{
    const uint16_t clusterId = 0x1234;

    commands.Register(make_unique<Echo>(clusterId));
}

#endif // __CHIPTOOL_ECHO_COMMANDS_H__
