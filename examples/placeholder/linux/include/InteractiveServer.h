/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <thread>
#include <websocket-server/WebSocketServer.h>

class InteractiveServer : public WebSocketServerDelegate
{
public:
    static InteractiveServer & GetInstance();
    void Run(const chip::Optional<uint16_t> port);

    bool Command(const chip::app::ConcreteCommandPath & path);
    bool ReadAttribute(const chip::app::ConcreteAttributePath & path);
    bool WriteAttribute(const chip::app::ConcreteAttributePath & path);
    void CommissioningComplete();

    /////////// WebSocketServerDelegate Interface /////////
    bool OnWebSocketMessageReceived(char * msg) override;

private:
    InteractiveServer(){};
    static InteractiveServer * instance;

    WebSocketServer mWebSocketServer;
    std::thread wsThread;
    bool mIsReady;
};
