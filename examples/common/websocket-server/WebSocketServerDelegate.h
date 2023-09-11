/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "WebSocketServerDelegate.h"

class WebSocketServerDelegate
{
public:
    virtual ~WebSocketServerDelegate(){};
    virtual bool OnWebSocketMessageReceived(char * msg) = 0;
};
