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
#include <optional>

#include <lib/core/CHIPError.h>
#include <lib/core/PeerId.h>
#include <lib/dnssd/TxtFields.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/SafeString.h>
#include <lib/support/Span.h>

namespace chip {
namespace Dnssd {

static constexpr uint16_t kMdnsPort = 5353;
// Need 8 bytes to fit a thread mac.
static constexpr size_t kMaxMacSize = 8;

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

enum class ICDModeAdvertise : uint8_t
{
    kNone, // The device does not support the LIT feature-set. No ICD= key is advertised in DNS-SD.
    kSIT,  // The ICD supports the LIT feature-set, but is currently operating as a SIT. ICD=0 in DNS-SD key/value pairs.
    kLIT,  // The ICD is currently operating as a LIT. ICD=1 in DNS-SD key/value pairs.
};

enum class TCPModeAdvertise : uint16_t
{
    kNone            = 0,                         // The device does not support TCP.
    kTCPClient       = 1 << 1,                    // The device supports the TCP client.
    kTCPServer       = 1 << 2,                    // The device supports the TCP server.
    kTCPClientServer = (kTCPClient | kTCPServer), // The device supports both the TCP client and server.
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
    uint16_t GetPort() const { return mPort; }

    Derived & SetInterfaceId(Inet::InterfaceId interfaceId)
    {
        mInterfaceId = interfaceId;
        return *reinterpret_cast<Derived *>(this);
    }

    Inet::InterfaceId GetInterfaceId() const { return mInterfaceId; }

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
    Derived & SetLocalMRPConfig(const std::optional<ReliableMessageProtocolConfig> & config)
    {
        mLocalMRPConfig = config;
        return *reinterpret_cast<Derived *>(this);
    }
    const std::optional<ReliableMessageProtocolConfig> & GetLocalMRPConfig() const { return mLocalMRPConfig; }

    Derived & SetTCPSupportModes(TCPModeAdvertise tcpSupportModes)
    {
        mTcpSupportModes = tcpSupportModes;
        return *reinterpret_cast<Derived *>(this);
    }
    TCPModeAdvertise GetTCPSupportModes() const { return mTcpSupportModes; }

    Derived & SetICDModeToAdvertise(ICDModeAdvertise operatingMode)
    {
        mICDModeAdvertise = operatingMode;
        return *reinterpret_cast<Derived *>(this);
    }
    ICDModeAdvertise GetICDModeToAdvertise() const { return mICDModeAdvertise; }

private:
    uint16_t mPort                   = CHIP_PORT;
    Inet::InterfaceId mInterfaceId   = Inet::InterfaceId::Null();
    bool mEnableIPv4                 = true;
    uint8_t mMacStorage[kMaxMacSize] = {};
    size_t mMacLength                = 0;
    std::optional<ReliableMessageProtocolConfig> mLocalMRPConfig;
    TCPModeAdvertise mTcpSupportModes  = TCPModeAdvertise::kNone;
    ICDModeAdvertise mICDModeAdvertise = ICDModeAdvertise::kNone;
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

    CompressedFabricId GetCompressedFabricId() const { return mPeerId.GetCompressedFabricId(); }

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

    CommissionAdvertisingParameters & SetVendorId(std::optional<uint16_t> vendorId)
    {
        mVendorId = vendorId;
        return *this;
    }
    std::optional<uint16_t> GetVendorId() const { return mVendorId; }

    CommissionAdvertisingParameters & SetProductId(std::optional<uint16_t> productId)
    {
        mProductId = productId;
        return *this;
    }
    std::optional<uint16_t> GetProductId() const { return mProductId; }

    CommissionAdvertisingParameters & SetCommissioningMode(CommissioningMode mode)
    {
        mCommissioningMode = mode;
        return *this;
    }
    CommissioningMode GetCommissioningMode() const { return mCommissioningMode; }

    CommissionAdvertisingParameters & SetDeviceType(std::optional<uint32_t> deviceType)
    {
        mDeviceType = deviceType;
        return *this;
    }
    std::optional<uint32_t> GetDeviceType() const { return mDeviceType; }

    CommissionAdvertisingParameters & SetDeviceName(std::optional<const char *> deviceName)
    {
        if (deviceName.has_value())
        {
            Platform::CopyString(mDeviceName, sizeof(mDeviceName), *deviceName);
            mDeviceNameHasValue = true;
        }
        else
        {
            mDeviceNameHasValue = false;
        }
        return *this;
    }
    std::optional<const char *> GetDeviceName() const
    {
        return mDeviceNameHasValue ? std::make_optional<const char *>(mDeviceName) : std::nullopt;
    }

    CommissionAdvertisingParameters & SetRotatingDeviceId(std::optional<const char *> rotatingId)
    {
        if (rotatingId.has_value())
        {
            Platform::CopyString(mRotatingId, sizeof(mRotatingId), *rotatingId);
            mRotatingIdHasValue = true;
        }
        else
        {
            mRotatingIdHasValue = false;
        }
        return *this;
    }
    std::optional<const char *> GetRotatingDeviceId() const
    {
        return mRotatingIdHasValue ? std::make_optional<const char *>(mRotatingId) : std::nullopt;
    }

    CommissionAdvertisingParameters & SetPairingInstruction(std::optional<const char *> pairingInstr)
    {
        if (pairingInstr.has_value())
        {
            Platform::CopyString(mPairingInstr, sizeof(mPairingInstr), *pairingInstr);
            mPairingInstrHasValue = true;
        }
        else
        {
            mPairingInstrHasValue = false;
        }
        return *this;
    }
    std::optional<const char *> GetPairingInstruction() const
    {
        return mPairingInstrHasValue ? std::make_optional<const char *>(mPairingInstr) : std::nullopt;
    }

    CommissionAdvertisingParameters & SetPairingHint(std::optional<uint16_t> pairingHint)
    {
        mPairingHint = pairingHint;
        return *this;
    }
    std::optional<uint16_t> GetPairingHint() const { return mPairingHint; }

    CommissionAdvertisingParameters & SetCommissionAdvertiseMode(CommssionAdvertiseMode mode)
    {
        mMode = mode;
        return *this;
    }
    CommssionAdvertiseMode GetCommissionAdvertiseMode() const { return mMode; }

    CommissionAdvertisingParameters & SetCommissionerPasscodeSupported(std::optional<bool> commissionerPasscodeSupported)
    {
        mCommissionerPasscodeSupported = commissionerPasscodeSupported;
        return *this;
    }
    std::optional<bool> GetCommissionerPasscodeSupported() const { return mCommissionerPasscodeSupported; }

private:
    uint8_t mShortDiscriminator          = 0;
    uint16_t mLongDiscriminator          = 0; // 12-bit according to spec
    CommssionAdvertiseMode mMode         = CommssionAdvertiseMode::kCommissionableNode;
    CommissioningMode mCommissioningMode = CommissioningMode::kEnabledBasic;
    std::optional<uint16_t> mVendorId;
    std::optional<uint16_t> mProductId;
    std::optional<uint32_t> mDeviceType;
    std::optional<uint16_t> mPairingHint;

    char mDeviceName[kKeyDeviceNameMaxLength + 1];
    bool mDeviceNameHasValue = false;

    char mRotatingId[kKeyRotatingDeviceIdMaxLength + 1];
    bool mRotatingIdHasValue = false;

    char mPairingInstr[kKeyPairingInstructionMaxLength + 1];
    bool mPairingInstrHasValue = false;

    std::optional<bool> mCommissionerPasscodeSupported;
};

/**
 * Interface for advertising CHIP DNS-SD services.
 *
 * A user of this interface must first initialize the advertiser using the `Init` method.
 *
 * Then, whenever advertised services need to be refreshed, the following sequence of events must
 * occur:
 * 1. Call the `RemoveServices` method.
 * 2. Call one of the `Advertise` methods for each service to be added or updated.
 * 3. Call the `FinalizeServiceUpdate` method to finalize the update and apply all pending changes.
 */
class ServiceAdvertiser
{
public:
    virtual ~ServiceAdvertiser() {}

    /**
     * Initializes the advertiser.
     *
     * The method must be called before other methods of this class.
     * If the advertiser has already been initialized, the method exits immediately with no error.
     */
    virtual CHIP_ERROR Init(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager) = 0;

    /**
     * Returns whether the advertiser has completed the initialization.
     *
     * Returns true if the advertiser is ready to advertise services.
     */
    virtual bool IsInitialized() = 0;

    /**
     * Shuts down the advertiser.
     */
    virtual void Shutdown() = 0;

    /**
     * Removes or marks all services being advertised for removal.
     *
     * Depending on the implementation, the method may either stop advertising existing services
     * immediately, or mark them for removal upon the subsequent `FinalizeServiceUpdate` method call.
     */
    virtual CHIP_ERROR RemoveServices() = 0;

    /**
     * Advertises the given operational node service.
     */
    virtual CHIP_ERROR Advertise(const OperationalAdvertisingParameters & params) = 0;

    /**
     * Advertises the given commissionable/commissioner node service.
     */
    virtual CHIP_ERROR Advertise(const CommissionAdvertisingParameters & params) = 0;

    /**
     * Finalizes updating advertised services.
     *
     * This method can be used by some implementations to apply changes made with the `RemoveServices`
     * and `Advertise` methods in case they could not be applied immediately.
     */
    virtual CHIP_ERROR FinalizeServiceUpdate() = 0;

    /**
     * Returns the commissionable node service instance name formatted as hex string.
     */
    virtual CHIP_ERROR GetCommissionableInstanceName(char * instanceName, size_t maxLength) const = 0;

    /**
     * Generates an updated commissionable instance name.  This happens
     * automatically when Init() is called, but may be needed at other times as
     * well.
     */
    virtual CHIP_ERROR UpdateCommissionableInstanceName() = 0;

    /**
     * Returns the system-wide implementation of the service advertiser.
     *
     * The method returns a reference to the advertiser object configured by
     * a user using the \c ServiceAdvertiser::SetInstance() method, or the
     * default advertiser returned by the \c GetDefaultAdvertiser() function.
     */
    static ServiceAdvertiser & Instance();

    /**
     * Sets the system-wide implementation of the service advertiser.
     */
    static void SetInstance(ServiceAdvertiser & advertiser);

private:
    static ServiceAdvertiser * sInstance;
};

/**
 * Returns the default implementation of the service advertiser.
 */
extern ServiceAdvertiser & GetDefaultAdvertiser();

} // namespace Dnssd
} // namespace chip
