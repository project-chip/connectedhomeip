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
 *    @file
 *      This file defines the classes corresponding to CHIP Exchange Manager Delegate.
 *
 */

#pragma once

#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>

namespace chip {
namespace Messaging {

class ExchangeManager;

class DLL_EXPORT ExchangeMgrDelegate
{
public:
    virtual ~ExchangeMgrDelegate() {}

    /**
     * @brief
     *   Called when a new pairing is being established
     *
     * @param session   The handle to the secure session
     * @param mgr       A pointer to the ExchangeManager
     */
    virtual void OnNewConnection(SecureSessionHandle session, ExchangeManager * mgr) {}

    /**
     * @brief
     *   Called when a connection is closing
     *
     * @param session   The handle to the secure session
     * @param mgr       A pointer to the ExchangeManager
     */
    virtual void OnConnectionExpired(SecureSessionHandle session, ExchangeManager * mgr) {}
};

} // namespace Messaging
} // namespace chip
