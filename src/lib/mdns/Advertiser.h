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

#include <inet/InetLayer.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/core/PeerId.h>
#include <lib/mdns/TxtFields.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/SafeString.h>
#include <lib/support/Span.h>

namespace chip {
namespace Mdns {

static constexpr uint16_t kMdnsPort = 5353;
// Need 8 bytes to fit a thread mac.
static constexpr size_t kMaxMacSize = 8;

// Commissionable/commissioner node subtypes
static constexpr size_t kSubTypeShortDiscriminatorMaxLength      = 4;  // _S<dd>
static constexpr size_t kSubTypeLongDiscriminatorMaxLength       = 6;  // _L<dddd>
static constexpr size_t kSubTypeVendorMaxLength                  = 7;  // _V<ddddd>
static constexpr size_t kSubTypeDeviceTypeMaxLength              = 5;  // _T<ddd>
static constexpr size_t kSubTypeCommissioningModeMaxLength       = 3;  // _C<d>
static constexpr size_t kSubTypeAdditionalCommissioningMaxLength = 3;  // _A<d>
static constexpr size_t kSubTypeCompressedFabricIdMaxLength      = 18; //_I<16-hex-digits>
// These are the max vals for comissioning adverts
static constexpr size_t kSubTypeMaxNumber   = 6;
static constexpr size_t kSubTypeTotalLength = kSubTypeShortDiscriminatorMaxLength + kSubTypeLongDiscriminatorMaxLength +
    kSubTypeVendorMaxLength + kSubTypeDeviceTypeMaxLength + kSubTypeCommissioningModeMaxLength +
    kSubTypeAdditionalCommissioningMaxLength;

enum class CommssionAdvertiseMode : uint8_t
{
    kCommissionableNode,
    kCommissioner,
};

enum class CommissioningMode
{
    kDisabled,       // Commissioning Mode is disabled, CM=0 in DNS-SD key/value pairs
    kEnabledBasic,   // Basic Commissioning Mode, CM=1 in DNS-SD key/value pairs
    kEnabledEnhanced // Enhanced Commissioning Mode, CM=2 in DNS-SD key/value pairs
};

template <class Derived>
class BaseAdvertisingParams
{
public:
    static constexpr uint8_t kCommonTxtMaxNumber     = KeyCount(TxtKeyUse::kCommon);
    static constexpr size_t kCommonTxtMaxKeySize     = MaxKeyLen(TxtKeyUse::kCommon);
    static constexpr size_t kCommonTxtMaxValueSize   = MaxValueLen(TxtKeyUse::kCommon);
    static constexpr size_t kCommonTxtTotalKeySize   = TotalKeyLen(TxtKeyUse::kCommon);
    static constexpr size_t kCommonTxtTotalValueSize = TotalValueLen(TxtKeyUse::kCommon);

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

    // Common Flags
    Derived & SetMRPRetryIntervals(Optional<uint32_t> intervalIdle, Optional<uint32_t> intervalActive)
    {
        mMrpRetryIntervalIdle   = intervalIdle;
        mMrpRetryIntervalActive = intervalActive;
        return *reinterpret_cast<Derived *>(this);
    }
    void GetMRPRetryIntervals(Optional<uint32_t> & intervalIdle, Optional<uint32_t> & intervalActive) const
    {
        intervalIdle   = mMrpRetryIntervalIdle;
        intervalActive = mMrpRetryIntervalActive;
    }
    Derived & SetTcpSupported(Optional<bool> tcpSupported)
    {
        mTcpSupported = tcpSupported;
        return *reinterpret_cast<Derived *>(this);
    }
    Optional<bool> GetTcpSupported() const { return mTcpSupported; }

private:
    uint16_t mPort                   = CHIP_PORT;
    bool mEnableIPv4                 = true;
    uint8_t mMacStorage[kMaxMacSize] = {};
    size_t mMacLength                = 0;
    Optional<uint32_t> mMrpRetryIntervalIdle;
    Optional<uint32_t> mMrpRetryIntervalActive;
    Optional<bool> mTcpSupported;
};

/// Defines parameters required for advertising a CHIP node
/// over mDNS as an 'operationally ready' node.
class OperationalAdvertisingParameters : public BaseAdvertisingParams<OperationalAdvertisingParameters>
{
public:
    // Operational uses only common keys
    static constexpr uint8_t kTxtMaxNumber     = kCommonTxtMaxNumber;
    static constexpr uint8_t kTxtMaxKeySize    = kCommonTxtMaxKeySize;
    static constexpr uint8_t kTxtMaxValueSize  = kCommonTxtMaxValueSize;
    static constexpr size_t kTxtTotalKeySize   = kCommonTxtTotalKeySize;
    static constexpr size_t kTxtTotalValueSize = kCommonTxtTotalValueSize;

    OperationalAdvertisingParameters & SetPeerId(const PeerId & peerId)
    {
        mPeerId = peerId;
        return *this;
    }
    PeerId GetPeerId() const { return mPeerId; }

private:
    PeerId mPeerId;
};

class CommissionAdvertisingParameters : public BaseAdvertisingParams<CommissionAdvertisingParameters>
{
public:
    static constexpr uint8_t kTxtMaxNumber     = kCommonTxtMaxNumber + KeyCount(TxtKeyUse::kCommission);
    static constexpr uint8_t kTxtMaxKeySize    = std::max(kCommonTxtMaxKeySize, MaxKeyLen(TxtKeyUse::kCommission));
    static constexpr uint8_t kTxtMaxValueSize  = std::max(kCommonTxtMaxValueSize, MaxValueLen(TxtKeyUse::kCommission));
    static constexpr size_t kTxtTotalKeySize   = kCommonTxtTotalKeySize + TotalKeyLen(TxtKeyUse::kCommission);
    static constexpr size_t kTxtTotalValueSize = kCommonTxtTotalValueSize + TotalValueLen(TxtKeyUse::kCommission);

    CommissionAdvertisingParameters & SetShortDiscriminator(uint8_t discriminator)
    {
        mShortDiscriminator = discriminator;
        return *this;
    }
    uint8_t GetShortDiscriminator() const { return mShortDiscriminator; }

    CommissionAdvertisingParameters & SetLongDiscriminator(uint16_t discriminator)
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

    CommissionAdvertisingParameters & SetCommissioningMode(CommissioningMode mode)
    {
        mCommissioningMode = mode;
        return *this;
    }
    CommissioningMode GetCommissioningMode() const { return mCommissioningMode; }

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
            Platform::CopyString(mDeviceName, sizeof(mDeviceName), deviceName.Value());
            mDeviceNameHasValue = true;
        }
        else
        {
            mDeviceNameHasValue = false;
        }
        return *this;
    }
    Optional<const char *> GetDeviceName() const
    {
        return mDeviceNameHasValue ? Optional<const char *>::Value(mDeviceName) : Optional<const char *>::Missing();
    }

    CommissionAdvertisingParameters & SetRotatingId(Optional<const char *> rotatingId)
    {
        if (rotatingId.HasValue())
        {
            Platform::CopyString(mRotatingId, sizeof(mRotatingId), rotatingId.Value());
            mRotatingIdHasValue = true;
        }
        else
        {
            mRotatingIdHasValue = false;
        }
        return *this;
    }
    Optional<const char *> GetRotatingId() const
    {
        return mRotatingIdHasValue ? Optional<const char *>::Value(mRotatingId) : Optional<const char *>::Missing();
    }

    CommissionAdvertisingParameters & SetPairingInstr(Optional<const char *> pairingInstr)
    {
        if (pairingInstr.HasValue())
        {
            Platform::CopyString(mPairingInstr, sizeof(mPairingInstr), pairingInstr.Value());
            mPairingInstrHasValue = true;
        }
        else
        {
            mPairingInstrHasValue = false;
        }
        return *this;
    }
    Optional<const char *> GetPairingInstr() const
    {
        return mPairingInstrHasValue ? Optional<const char *>::Value(mPairingInstr) : Optional<const char *>::Missing();
    }

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
    uint8_t mShortDiscriminator          = 0;
    uint16_t mLongDiscriminator          = 0; // 12-bit according to spec
    CommssionAdvertiseMode mMode         = CommssionAdvertiseMode::kCommissionableNode;
    CommissioningMode mCommissioningMode = CommissioningMode::kEnabledBasic;
    chip::Optional<uint16_t> mVendorId;
    chip::Optional<uint16_t> mProductId;
    chip::Optional<uint16_t> mDeviceType;
    chip::Optional<uint16_t> mPairingHint;

    char mDeviceName[kKeyDeviceNameMaxLength + 1];
    bool mDeviceNameHasValue = false;

    char mRotatingId[kKeyRotatingIdMaxLength + 1];
    bool mRotatingIdHasValue = false;

    char mPairingInstr[kKeyPairingInstructionMaxLength + 1];
    bool mPairingInstrHasValue = false;
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

    /// Returns DNS-SD instance name formatted as hex string
    virtual CHIP_ERROR GetCommissionableInstanceName(char * instanceName, size_t maxLength) = 0;
};

} // namespace Mdns
} // namespace chip
