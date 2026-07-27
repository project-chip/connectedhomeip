/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      This file defines the interface for upcalls from NfcLayer
 *      to a client application.
 */

#pragma once

#include <nfc/NfcConfig.h>

#include <lib/support/DLLUtil.h>

#include <transport/raw/NFC.h>

namespace chip {
namespace Nfc {

// Platform-agnostic NFC interface
class DLL_EXPORT NfcApplicationDelegate
{
public:
    virtual ~NfcApplicationDelegate() {}

    virtual bool CanSendToPeer(const Transport::PeerAddress & address) = 0;

    virtual CHIP_ERROR SendToNfcTag(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf) = 0;

    virtual void SetNFCBase(Transport::NFCBase * nfcBase) = 0;
};

} /* namespace Nfc */
} /* namespace chip */
