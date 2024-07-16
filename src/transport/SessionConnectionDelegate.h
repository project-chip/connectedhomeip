/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <transport/Session.h>
#include <transport/raw/MessageHeader.h>

namespace chip {

/**
 * @brief
 *   This class defines a delegate that will be called by the SessionManager on
 *   specific connection-related (e.g. for TCP) events. If the user of SessionManager
 *   is interested in receiving these callbacks, they can specialize this class and
 *   handle each trigger in their implementation of this class.
 */
class DLL_EXPORT SessionConnectionDelegate
{
public:
    virtual ~SessionConnectionDelegate() {}

    /**
     * @brief
     *   Called when the underlying connection for the session is closed.
     *
     * @param session       The handle to the secure session
     * @param conErr        The connection error code
     */
    virtual void OnTCPConnectionClosed(const SessionHandle & session, CHIP_ERROR conErr) = 0;
};

} // namespace chip
