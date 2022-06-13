/*
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

#pragma once

#include <transport/SessionHandle.h>

namespace chip {

class DLL_EXPORT SessionDelegate
{
public:
    virtual ~SessionDelegate() {}

    enum class NewSessionHandlingPolicy : uint8_t
    {
        kShiftToNewSession,
        kStayAtOldSession,
    };

    /**
     * @brief
     *   Called when a new secure session to the same peer is established, over the delegate of SessionHolderWithDelegate object. It
     *   is suggested to shift to the newly created session.
     *
     * Note: the default implementation orders shifting to the new session, it should be fine for all users, unless the
     * SessionHolder object is expected to be sticky to a specified session.
     */
    virtual NewSessionHandlingPolicy GetNewSessionHandlingPolicy() { return NewSessionHandlingPolicy::kShiftToNewSession; }

    using Event = void (SessionDelegate::*)();

    /**
     * @brief
     *   Called when a session is releasing
     */
    virtual void OnSessionReleased() = 0;

    /**
     * @brief
     *   Called when the first message delivery in an exchange fails, so actions aiming to recover connection can be performed.
     *
     *   Note: the implementation must not do anything that will destroy the session or change the SessionHolder.
     */
    virtual void OnFirstMessageDeliveryFailed() {}

    /**
     * @brief
     *   Called when a session is unresponsive for a while (detected by MRP)
     *
     *   Note: the implementation must not do anything that will destroy the session or change the SessionHolder.
     */
    virtual void OnSessionHang() {}
};

} // namespace chip
