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

#include <algorithm>
#include <cstdint>

#include <core/CHIPError.h>
#include <core/Optional.h>
#include <core/PeerId.h>
#include <inet/InetLayer.h>
#include <lib/support/Span.h>

namespace chip {
namespace Mdns {

static constexpr uint16_t kMdnsPort = 5353;
// Need 8 bytes to fit a thread mac.
static constexpr size_t kMaxMacSize = 8;

static constexpr size_t kKeyDiscriminatorMaxLength      = 5;
static constexpr size_t kKeyVendorProductMaxLength      = 11;
static constexpr size_t kKeyAdditionalPairingMaxLength  = 1;
static constexpr size_t kKeyCommissioningModeMaxLength  = 1;
static constexpr size_t kKeyDeviceTypeMaxLength         = 5;
static constexpr size_t kKeyDeviceNameMaxLength         = 32;
static constexpr size_t kKeyRotatingIdMaxLength         = 100;
static constexpr size_t kKeyPairingInstructionMaxLength = 128;
static constexpr size_t kKeyPairingHintMaxLength        = 10;

static constexpr size_t kSubTypeShortDiscriminatorMaxLength = 3;
static constexpr size_t kSubTypeLongDiscriminatorMaxLength  = 4;
static constexpr size_t kSubTypeVendorMaxLength             = 5;
static constexpr size_t kSubTypeDeviceTypeMaxLength         = 5;
static constexpr size_t kSubTypeCommissioningModeMaxLength  = 1;
static constexpr size_t kSubTypeAdditionalPairingMaxLength  = 1;

enum class CommssionAdvertiseMode : uint8_t
{
    kCommissionableNode,
    kCommissioner,
};

template <class Derived>
class BaseAdvertisingParams
{
public:
    Derived & SetPort(uint16_t port)
    {
        mPort = port;
        return *reinterpret_cast<Derived *>(this);
    }
    uint64_t GetPort() const { return mPort; }

    Derived & EnableIpV4(bool enable)
    {
        mEnableIPv4 = enable;
        return *reinterpret_cast<Derived *>(this);
    }
    bool IsIPv4Enabled() const { return mEnableIPv4; }
    Derived & SetMac(chip::ByteSpan mac)
    {
        mMacLength = std::min(mac.size(), kMaxMacSize);
        memcpy(mMacStorage, mac.data(), mMacLength);
        return *reinterpret_cast<Derived *>(this);
    }
    const chip::ByteSpan GetMac() const { return chip::ByteSpan(mMacStorage, mMacLength); }

private:
    uint16_t mPort                   = CHIP_PORT;
    bool mEnableIPv4                 = true;
    uint8_t mMacStorage[kMaxMacSize] = {};
    size_t mMacLength                = 0;
}; // namespace Mdns

/// Defines parameters required for advertising a CHIP node
/// over mDNS as an 'operationally ready' node.
class OperationalAdvertisingParameters : public BaseAdvertisingParams<OperationalAdvertisingParameters>
{
public:
    OperationalAdvertisingParameters & SetPeerId(const PeerId & peerId)
    {
        mPeerId = peerId;
        return *this;
    }
    PeerId GetPeerId() const { return mPeerId; }

    OperationalAdvertisingParameters & SetCRMPRetryIntervals(uint32_t intervalIdle, uint32_t intervalActive)
    {
        mCrmpRetryIntervalIdle   = intervalIdle;
        mCrmpRetryIntervalActive = intervalActive;
        return *this;
    }
    void GetCRMPRetryIntervals(uint32_t & intervalIdle, uint32_t & intervalActive) const
    {
        intervalIdle   = mCrmpRetryIntervalIdle;
        intervalActive = mCrmpRetryIntervalActive;
    }

private:
    PeerId mPeerId;
    uint32_t mCrmpRetryIntervalIdle   = 0;
    uint32_t mCrmpRetryIntervalActive = 0;
};

class CommissionAdvertisingParameters : public BaseAdvertisingParams<CommissionAdvertisingParameters>
{
public:
    CommissionAdvertisingParameters & SetShortDiscriminator(uint8_t discriminator)
    {
        mShortDiscriminator = discriminator;
        return *this;
    }
    uint8_t GetShortDiscriminator() const { return mShortDiscriminator; }

    CommissionAdvertisingParameters & SetLongDiscrimininator(uint16_t discriminator)
    {
        mLongDiscriminator = discriminator;
        return *this;
    }
    uint16_t GetLongDiscriminator() const { return mLongDiscriminator; }

    CommissionAdvertisingParameters & SetVendorId(Optional<uint16_t> vendorId)
    {
        mVendorId = vendorId;
        return *this;
    }
    Optional<uint16_t> GetVendorId() const { return mVendorId; }

    CommissionAdvertisingParameters & SetProductId(Optional<uint16_t> productId)
    {
        mProductId = productId;
        return *this;
    }
    Optional<uint16_t> GetProductId() const { return mProductId; }

    CommissionAdvertisingParameters & SetCommissioningMode(bool modeEnabled, bool openWindow)
    {
        mCommissioningModeEnabled    = modeEnabled;
        mOpenWindowCommissioningMode = openWindow;
        return *this;
    }
    bool GetCommissioningMode() const { return mCommissioningModeEnabled; }
    bool GetOpenWindowCommissioningMode() const { return mOpenWindowCommissioningMode; }

    CommissionAdvertisingParameters & SetDeviceType(Optional<uint16_t> deviceType)
    {
        mDeviceType = deviceType;
        return *this;
    }
    Optional<uint16_t> GetDeviceType() const { return mDeviceType; }

    CommissionAdvertisingParameters & SetDeviceName(Optional<const char *> deviceName)
    {
        if (deviceName.HasValue())
        {
            strncpy(sDeviceName, deviceName.Value(), min(strlen(deviceName.Value()) + 1, sizeof(sDeviceName)));
            mDeviceName = Optional<const char *>::Value(static_cast<const char *>(sDeviceName));
        }
        return *this;
    }
    Optional<const char *> GetDeviceName() const { return mDeviceName; }

    CommissionAdvertisingParameters & SetRotatingId(Optional<const char *> rotatingId)
    {
        if (rotatingId.HasValue())
        {
            strncpy(sRotatingId, rotatingId.Value(), min(strlen(rotatingId.Value()) + 1, sizeof(sRotatingId)));
            mRotatingId = Optional<const char *>::Value(static_cast<const char *>(sRotatingId));
        }
        return *this;
    }
    Optional<const char *> GetRotatingId() const { return mRotatingId; }

    CommissionAdvertisingParameters & SetPairingInstr(Optional<const char *> pairingInstr)
    {
        if (pairingInstr.HasValue())
        {
            strncpy(sPairingInstr, pairingInstr.Value(), min(strlen(pairingInstr.Value()) + 1, sizeof(sPairingInstr)));
            mPairingInstr = Optional<const char *>::Value(static_cast<const char *>(sPairingInstr));
        }
        return *this;
    }
    Optional<const char *> GetPairingInstr() const { return mPairingInstr; }

    CommissionAdvertisingParameters & SetPairingHint(Optional<uint16_t> pairingHint)
    {
        mPairingHint = pairingHint;
        return *this;
    }
    Optional<uint16_t> GetPairingHint() const { return mPairingHint; }

    CommissionAdvertisingParameters & SetCommissionAdvertiseMode(CommssionAdvertiseMode mode)
    {
        mMode = mode;
        return *this;
    }
    CommssionAdvertiseMode GetCommissionAdvertiseMode() const { return mMode; }

private:
    uint8_t mShortDiscriminator       = 0;
    uint16_t mLongDiscriminator       = 0; // 12-bit according to spec
    CommssionAdvertiseMode mMode      = CommssionAdvertiseMode::kCommissionableNode;
    bool mCommissioningModeEnabled    = false;
    bool mOpenWindowCommissioningMode = false;
    chip::Optional<uint16_t> mVendorId;
    chip::Optional<uint16_t> mProductId;
    chip::Optional<uint16_t> mDeviceType;
    chip::Optional<uint16_t> mPairingHint;

    char sDeviceName[kKeyDeviceNameMaxLength + 1];
    chip::Optional<const char *> mDeviceName;

    char sRotatingId[kKeyRotatingIdMaxLength + 1];
    chip::Optional<const char *> mRotatingId;

    char sPairingInstr[kKeyPairingInstructionMaxLength + 1];
    chip::Optional<const char *> mPairingInstr;
};

/// Handles advertising of CHIP nodes
class ServiceAdvertiser
{
public:
    virtual ~ServiceAdvertiser() {}

    /// Starts the advertiser. Items 'Advertised' will become visible.
    /// Must be called before Advertise() calls.
    virtual CHIP_ERROR Start(chip::Inet::InetLayer * inetLayer, uint16_t port) = 0;

    /// Advertises the CHIP node as an operational node
    virtual CHIP_ERROR Advertise(const OperationalAdvertisingParameters & params) = 0;

    /// Advertises the CHIP node as a commissioner/commissionable node
    virtual CHIP_ERROR Advertise(const CommissionAdvertisingParameters & params) = 0;

    /// Stops the advertiser.
    virtual CHIP_ERROR StopPublishDevice() = 0;

    /// Provides the system-wide implementation of the service advertiser
    static ServiceAdvertiser & Instance();
};

} // namespace Mdns
} // namespace chip
