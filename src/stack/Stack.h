/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
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

#pragma once

#include <core/CHIPCore.h>

namespace chip {

// Forward declare all CHIP components, so that all components are able to contain a pointer to the CHIP stack.
class TransportMgrBase;
class SecureSessionMgr;
namespace Transport {
class AdminPairingTable;
}
namespace Messaging {
class ExchangeManager;
}
namespace app {
class InteractionModelEngine;
}

class Stack
{
public:
    virtual ~Stack() {}

    virtual CHIP_ERROR Init()     = 0;
    virtual CHIP_ERROR Shutdown() = 0;

    virtual NodeId GetLocalNodeId()          = 0;
    virtual System::Layer & GetSystemLayer() = 0;
    virtual Inet::InetLayer & GetInetLayer() = 0;
    virtual Ble::BleLayer * GetBleLayer()    = 0;

    virtual TransportMgrBase & GetTransportManager()          = 0;
    virtual CHIP_ERROR ResetTransport()                       = 0;
    virtual SecureSessionMgr & GetSecureSessionManager()      = 0;
    virtual Transport::AdminPairingTable & GetAdmins()        = 0;
    virtual Messaging::ExchangeManager & GetExchangeManager() = 0;
#ifdef CHIP_ENABLE_INTERACTION_MODEL
    virtual app::InteractionModelEngine & GetInteractionModelEngine() = 0;
#endif
};

} // namespace chip
