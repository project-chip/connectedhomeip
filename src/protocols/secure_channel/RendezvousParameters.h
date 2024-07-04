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

#include <optional>

#include <transport/raw/Base.h>
#include <transport/raw/PeerAddress.h>
#if CONFIG_NETWORK_LAYER_BLE
#include <ble/Ble.h>
#endif // CONFIG_NETWORK_LAYER_BLE

#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/SetupDiscriminator.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <protocols/secure_channel/PASESession.h>

namespace chip {

// The largest supported value for Rendezvous discriminators
const uint16_t kMaxRendezvousDiscriminatorValue = 0xFFF;

// The largest supported value for session idle interval and session active interval
inline constexpr uint32_t kMaxSessionIdleInterval = 3600000;

class RendezvousParameters
{
public:
    RendezvousParameters() = default;

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
    bool HasDiscriminator() const { return mSetupDiscriminator.has_value(); }

    // Obtains the long version of the discriminator, or 0 if short.
    // WARNING: This is lossy and a bad idea to use. The correct method to use
    //          is GetSetupDiscriminator(). This method exists for public
    //          API backwards compatibility.
    uint16_t GetDiscriminator() const
    {
        if (!mSetupDiscriminator.has_value())
        {
            ChipLogError(Discovery,
                         "Get RendezvousParameters::GetDiscriminator() called without discriminator in params (inconsistent). "
                         "Using value 0 to avoid crash! Ensure discriminator is set!");
            return 0;
        }

        if (mSetupDiscriminator.value().IsShortDiscriminator())
        {
            ChipLogError(Discovery,
                         "Get RendezvousParameters::GetDiscriminator() called with SHORT discriminator (inconsistent). Using value "
                         "0 to avoid crash! Call GetSetupDiscriminator() to avoid loss.");
            return 0;
        }

        return mSetupDiscriminator.value().GetLongValue();
    }

    std::optional<SetupDiscriminator> GetSetupDiscriminator() const
    {
        if (!mSetupDiscriminator.has_value())
        {
            ChipLogError(
                Discovery,
                "Get RendezvousParameters::GetSetupDiscriminator() called without discriminator in params (inconsistent).");
        }
        return mSetupDiscriminator;
    }

    RendezvousParameters & SetSetupDiscriminator(SetupDiscriminator discriminator)
    {
        mSetupDiscriminator = discriminator;
        return *this;
    }

    RendezvousParameters & SetDiscriminator(uint16_t discriminator)
    {
        SetupDiscriminator tempDiscriminator;
        tempDiscriminator.SetLongValue(discriminator);
        mSetupDiscriminator = tempDiscriminator;
        return *this;
    }

    bool HasPASEVerifier() const { return mHasPASEVerifier; }
    const Crypto::Spake2pVerifier & GetPASEVerifier() const { return mPASEVerifier; }
    RendezvousParameters & SetPASEVerifier(Crypto::Spake2pVerifier & verifier)
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

    bool HasDiscoveredObject() const { return mDiscoveredObject != BLE_CONNECTION_UNINITIALIZED; }
    BLE_CONNECTION_OBJECT GetDiscoveredObject() const { return mDiscoveredObject; }
    RendezvousParameters & SetDiscoveredObject(BLE_CONNECTION_OBJECT connObj)
    {
        mDiscoveredObject = connObj;
        return *this;
    }
#else
    bool HasConnectionObject() const { return false; }
    bool HasDiscoveredObject() const { return false; }
#endif // CONFIG_NETWORK_LAYER_BLE

    bool HasMRPConfig() const { return mMRPConfig.HasValue(); }
    ReliableMessageProtocolConfig GetMRPConfig() const { return mMRPConfig.ValueOr(GetDefaultMRPConfig()); }
    RendezvousParameters & SetIdleInterval(System::Clock::Milliseconds32 interval)
    {
        if (!mMRPConfig.HasValue())
        {
            mMRPConfig.Emplace(GetDefaultMRPConfig());
        }
        mMRPConfig.Value().mIdleRetransTimeout = interval;
        return *this;
    }

    RendezvousParameters & SetActiveInterval(System::Clock::Milliseconds32 interval)
    {
        if (!mMRPConfig.HasValue())
        {
            mMRPConfig.Emplace(GetDefaultMRPConfig());
        }
        mMRPConfig.Value().mActiveRetransTimeout = interval;
        return *this;
    }

private:
    Transport::PeerAddress mPeerAddress; ///< the peer node address
    uint32_t mSetupPINCode = 0;          ///< the target peripheral setup PIN Code
    std::optional<SetupDiscriminator> mSetupDiscriminator;

    Crypto::Spake2pVerifier mPASEVerifier;
    bool mHasPASEVerifier = false;

    Optional<ReliableMessageProtocolConfig> mMRPConfig;

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer               = nullptr;
    BLE_CONNECTION_OBJECT mConnectionObject = BLE_CONNECTION_UNINITIALIZED;
    BLE_CONNECTION_OBJECT mDiscoveredObject = BLE_CONNECTION_UNINITIALIZED;
#endif // CONFIG_NETWORK_LAYER_BLE
};

} // namespace chip
