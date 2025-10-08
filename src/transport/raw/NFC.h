/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *    @file
 *      This file defines the CHIP Connection object that maintains an NFC connection.
 *
 */

#pragma once

#include <lib/core/CHIPCore.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/Base.h>
#include <utility>

namespace chip {
namespace Transport {

/** Defines listening parameters for setting up a BLE transport */
class NfcListenParameters
{
public:
    explicit NfcListenParameters(void * param) {}
    NfcListenParameters(const NfcListenParameters &) = default;
    NfcListenParameters(NfcListenParameters &&)      = default;

private:
};

/** Implements a transport using NFC.
 */
class DLL_EXPORT NFCBase : public Base
{
    /**
     *  The State of the NFC connection
     *
     */
    enum class State
    {
        kNotReady    = 0, /**< State before initialization. */
        kInitialized = 1, /**< State after initialization. */
        kConnected   = 2, /**< Endpoint connected. */
    };

public:
    NFCBase(void) {}
    ~NFCBase() override;

    /**
     * Initialize a NFC transport to a given peripheral or a given device name.
     *
     * @param param        NFC configuration parameters for this transport
     */
    CHIP_ERROR Init(const NfcListenParameters & params);

    CHIP_ERROR SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf) override;

    void OnNfcTagResponse(const Transport::PeerAddress & address, System::PacketBufferHandle && buffer);
    void OnNfcTagError(const Transport::PeerAddress & address);

    bool CanSendToPeer(const Transport::PeerAddress & address) override;

private:
    void ClearState();

    State mState = State::kNotReady; ///< State of the NFC transport
};

class NFC : public NFCBase
{
public:
    NFC() : NFCBase() {}

private:
};

} // namespace Transport
} // namespace chip
