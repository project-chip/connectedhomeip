/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *   This file implements the handler for echo messages.
 */

#include <app/server/EchoHandler.h>

#include <lib/support/ErrorStr.h>
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
    ChipLogProgress(AppServer, "Echo Request, len=%u ... sending response.\n", payload->DataLength());
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
