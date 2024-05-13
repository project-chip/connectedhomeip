/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file
 *   This file implements the handler for echo messages.
 */

#include <app/server/EchoHandler.h>

#include <lib/core/ErrorStr.h>
#include <protocols/echo/Echo.h>

namespace {

// The EchoServer object.
chip::Protocols::Echo::EchoServer gEchoServer;

/**
 * Callback handler when a CHIP EchoRequest is received.
 *
 * @param [in] ec      The exchange context holding the incoming message.
 * @param [in] payload The buffer holding the message.  This function guarantees
 *                     that it will free the buffer before returning.
 *
 */
void HandleEchoRequestReceived(chip::Messaging::ExchangeContext * ec, chip::System::PacketBufferHandle && payload)
{
    ChipLogProgress(AppServer, "Echo Request, len=%" PRIu32 "... sending response.\n",
                    static_cast<uint32_t>(payload->DataLength()));
}

} // namespace

CHIP_ERROR InitEchoHandler(chip::Messaging::ExchangeManager * exchangeMgr)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = gEchoServer.Init(exchangeMgr);
    SuccessOrExit(err);

    // Arrange to get a callback whenever an Echo Request is received.
    gEchoServer.SetEchoRequestReceived(HandleEchoRequestReceived);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "EchoServer failed, err:%s\n", chip::ErrorStr(err));
    }

    return err;
}
