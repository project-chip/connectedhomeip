/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>

#include <lib/support/DLLUtil.h>

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
     *   Our security model is built upon Exchanges and Sessions, but not SessionHolders, such that SessionHolders should be able to
     *   shift to a new session freely. If an application is holding a session which is not intended to be shifted, it can provide
     *   its shifting policy by overriding GetNewSessionHandlingPolicy in SessionDelegate. For example SessionHolders inside
     *   ExchangeContext and PairingSession are not eligible for auto-shifting.
     *
     * Note: the default implementation orders shifting to the new session, it should be fine for all users, unless the
     *       SessionHolder object is expected to be sticky to a specified session.
     *
     * Note: the implementation MUST NOT modify the session pool or the state of session holders (eg, adding new session, removing
     *       old session) from inside this callback.
     */
    virtual NewSessionHandlingPolicy GetNewSessionHandlingPolicy() { return NewSessionHandlingPolicy::kShiftToNewSession; }

    using Event = void (SessionDelegate::*)();

    /**
     * @brief
     *   Called when a session is releasing. Callees SHALL NOT make synchronous calls into SessionManager to allocate a new session.
     *   If they desire to do so, it MUST be done asynchronously.
     */
    virtual void OnSessionReleased() = 0;

    /**
     * @brief
     *   Called when a session is unresponsive for a while (detected by MRP). Callees SHALL NOT make synchronous calls into
     * SessionManager to allocate a new session. If they desire to do so, it MUST be done asynchronously.
     */
    virtual void OnSessionHang() {}
};

} // namespace chip
