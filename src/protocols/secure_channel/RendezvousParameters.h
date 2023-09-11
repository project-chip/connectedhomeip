/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <transport/raw/Base.h>
#include <transport/raw/PeerAddress.h>
#if CONFIG_NETWORK_LAYER_BLE
#include <ble/Ble.h>
#endif // CONFIG_NETWORK_LAYER_BLE

#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <protocols/secure_channel/PASESession.h>

namespace chip {

// The largest supported value for Rendezvous discriminators
const uint16_t kMaxRendezvousDiscriminatorValue = 0xFFF;

// The largest supported value for sleepy idle interval and sleepy active interval
constexpr uint32_t kMaxSleepyInterval = 3600000;

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
    Transport::PeerAddress mPeerAddress;  ///< the peer node address
    uint32_t mSetupPINCode  = 0;          ///< the target peripheral setup PIN Code
    uint16_t mDiscriminator = UINT16_MAX; ///< the target peripheral discriminator

    Spake2pVerifier mPASEVerifier;
    bool mHasPASEVerifier = false;

    Optional<ReliableMessageProtocolConfig> mMRPConfig;

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer               = nullptr;
    BLE_CONNECTION_OBJECT mConnectionObject = BLE_CONNECTION_UNINITIALIZED;
    BLE_CONNECTION_OBJECT mDiscoveredObject = BLE_CONNECTION_UNINITIALIZED;
#endif // CONFIG_NETWORK_LAYER_BLE
};

} // namespace chip
