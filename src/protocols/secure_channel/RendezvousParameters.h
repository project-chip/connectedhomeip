/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <transport/raw/Base.h>
#include <transport/raw/PeerAddress.h>
#if CONFIG_NETWORK_LAYER_BLE
#include <ble/Ble.h>
#endif // CONFIG_NETWORK_LAYER_BLE

#include <lib/support/logging/CHIPLogging.h>
#include <protocols/secure_channel/PASESession.h>

namespace chip {

// The largest supported value for Rendezvous discriminators
const uint16_t kMaxRendezvousDiscriminatorValue = 0xFFF;

class RendezvousParameters
{
public:
    RendezvousParameters() = default;

    bool IsController() const { return HasDiscriminator() || HasConnectionObject(); }

    bool HasSetupPINCode() const { return mSetupPINCode != 0; }
    uint32_t GetSetupPINCode() const { return mSetupPINCode; }
    RendezvousParameters & SetSetupPINCode(uint32_t setupPINCode)
    {
        mSetupPINCode = setupPINCode;
        return *this;
    }

    bool HasPeerAddress() const { return mPeerAddress.IsInitialized(); }
    Transport::PeerAddress GetPeerAddress() const { return mPeerAddress; }
    RendezvousParameters & SetPeerAddress(const Transport::PeerAddress & peerAddress)
    {
        mPeerAddress = peerAddress;
        return *this;
    }

    // Discriminators in RendezvousParameters are always long (12-bit)
    // discriminators.
    bool HasDiscriminator() const { return mDiscriminator <= kMaxRendezvousDiscriminatorValue; }
    uint16_t GetDiscriminator() const { return mDiscriminator; }
    RendezvousParameters & SetDiscriminator(uint16_t discriminator)
    {
        mDiscriminator = discriminator;
        return *this;
    }

    bool HasPASEVerifier() const { return mHasPASEVerifier; }
    const Spake2pVerifier & GetPASEVerifier() const { return mPASEVerifier; }
    RendezvousParameters & SetPASEVerifier(Spake2pVerifier & verifier)
    {
        memmove(&mPASEVerifier, &verifier, sizeof(verifier));
        mHasPASEVerifier = true;
        return *this;
    }

#if CONFIG_NETWORK_LAYER_BLE
    bool HasBleLayer() const { return mBleLayer != nullptr; }
    Ble::BleLayer * GetBleLayer() const { return mBleLayer; }
    RendezvousParameters & SetBleLayer(Ble::BleLayer * value)
    {
        mBleLayer = value;
        return *this;
    }

    bool HasConnectionObject() const { return mConnectionObject != BLE_CONNECTION_UNINITIALIZED; }
    BLE_CONNECTION_OBJECT GetConnectionObject() const { return mConnectionObject; }
    RendezvousParameters & SetConnectionObject(BLE_CONNECTION_OBJECT connObj)
    {
        mConnectionObject = connObj;
        return *this;
    }
#else
    bool HasConnectionObject() const { return false; }
#endif // CONFIG_NETWORK_LAYER_BLE

private:
    Transport::PeerAddress mPeerAddress;  ///< the peer node address
    uint32_t mSetupPINCode  = 0;          ///< the target peripheral setup PIN Code
    uint16_t mDiscriminator = UINT16_MAX; ///< the target peripheral discriminator

    Spake2pVerifier mPASEVerifier;
    bool mHasPASEVerifier = false;

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer               = nullptr;
    BLE_CONNECTION_OBJECT mConnectionObject = BLE_CONNECTION_UNINITIALIZED;
#endif // CONFIG_NETWORK_LAYER_BLE
};

} // namespace chip
