/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines the Delegate class that contains callbacks to
 *      establish a secure session and update status of the session establishment process.
 *
 */

#pragma once

#include <system/SystemPacketBuffer.h>
#include <transport/Session.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

class DLL_EXPORT SessionEstablishmentDelegate
{
public:
    /**
     *   Called when session establishment fails with an error.  This will be
     *   called at most once per session establishment and will not be called if
     *   OnSessionEstablished is called.
     */
    virtual void OnSessionEstablishmentError(CHIP_ERROR error) {}

    /**
     *   Called on start of session establishment process
     */
    virtual void OnSessionEstablishmentStarted() {}

    /**
     *   Called when the new secure session has been established.  This is
     *   mututally exclusive with OnSessionEstablishmentError for a give session
     *   establishment.
     */
    virtual void OnSessionEstablished(const SessionHandle & session) {}

    virtual ~SessionEstablishmentDelegate() {}
};

} // namespace chip
