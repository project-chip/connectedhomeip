/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

/**
 *  @file
 *    This file contains definitions for DeviceProxy base class. The objects of this
 *    class will be used by applications to interact with peer CHIP devices.
 *    The class provides mechanism to construct, send and receive messages to and
 *    from the corresponding CHIP devices.
 */

#pragma once

#include <app/CommandSender.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemClock.h>

namespace chip {

class DLL_EXPORT DeviceProxy
{
public:
    virtual ~DeviceProxy() {}
    DeviceProxy() {}

    /**
     *  Mark any open session with the device as expired.
     */
    virtual void Disconnect() = 0;

    virtual NodeId GetDeviceId() const = 0;

    virtual CHIP_ERROR SendCommands(app::CommandSender * commandObj, chip::Optional<System::Clock::Timeout> timeout = NullOptional);

    virtual Messaging::ExchangeManager * GetExchangeManager() const = 0;

    virtual chip::Optional<SessionHandle> GetSecureSession() const = 0;

    virtual CHIP_ERROR SetPeerId(ByteSpan rcac, ByteSpan noc) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * Facilities for keeping track of the latest point we can expect the
     * fail-safe to last through.  These timestamp values use the monotonic clock.
     */
    void SetFailSafeExpirationTimestamp(System::Clock::Timestamp timestamp) { mFailSafeExpirationTimestamp = timestamp; }
    System::Clock::Timestamp GetFailSafeExpirationTimestamp() const { return mFailSafeExpirationTimestamp; }

    /**
     * @brief
     *   This function returns the attestation challenge for the secure session.
     *
     * @param[out] attestationChallenge The output for the attestationChallenge
     *
     * @return CHIP_ERROR               CHIP_NO_ERROR on success, or CHIP_ERROR_INVALID_ARGUMENT if no secure session is active
     */
    virtual CHIP_ERROR GetAttestationChallenge(ByteSpan & attestationChallenge);

protected:
    virtual bool IsSecureConnected() const = 0;

    System::Clock::Timestamp mFailSafeExpirationTimestamp = System::Clock::kZero;
};

} // namespace chip
