/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "WebSocketServerDelegate.h"

#include <lib/core/Optional.h>
#include <lib/support/CHIPMem.h>

#include <string>

class WebSocketServer : public WebSocketServerDelegate
{
public:
    CHIP_ERROR Run(chip::Optional<uint16_t> port, WebSocketServerDelegate * delegate);
    void Send(const char * msg);

    bool OnWebSocketMessageReceived(char * msg) override;

private:
    bool mRunning;
    WebSocketServerDelegate * mDelegate = nullptr;
};
