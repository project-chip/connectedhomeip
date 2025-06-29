/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 * @brief Defines processing state managed by Commissioner for an active User Directed Commissioning (UDC) client.
 */

#pragma once

#include <lib/dnssd/Resolver.h>
#include <transport/raw/Base.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

enum class UDCClientProcessingState : uint8_t
{
    kNotInitialized,
    kDiscoveringNode,
    kNodeDiscoveryFailed,
    kPromptingUser,
    kUserDeclined,
    kObtainingOnboardingPayload,
    kWaitingForCommissionerPasscodeReady,
    kCommissioningNode,
    kCommissioningFailed,
};

using PeerAddress = ::chip::Transport::PeerAddress;

enum class TargetAppCheckState : uint8_t
{
    kNotInitialized,
    kAppNotFound,
    kAppFoundPasscodeReturned,
    kAppFoundNoPasscode,
};

/**
 * Represents information in the TargetAppList of the Identification Declaration message
 */
struct TargetAppInfo
{
    uint16_t vendorId              = 0;
    uint16_t productId             = 0;
    TargetAppCheckState checkState = TargetAppCheckState::kNotInitialized;
    uint32_t passcode              = 0;
    bool foundApp                  = false;
};

/**
 * Defines the handling state of a UDC Client.
 *
 * Information contained within the state:
 *   - PeerAddress represents how to talk to the UDC client
 *   - PeerInstanceName is the DNS-SD instance name of the UDC client
 *   - ExpirationTimeMs is a timestamp of when this object should expire.
 *   - UDCClientProcessingState contains the current state of processing this UDC Client
 *
 */
class UDCClientState
{
public:
    UDCClientState() : mPeerAddress(PeerAddress::Uninitialized()) {}

    UDCClientState(UDCClientState &&)                  = default;
    UDCClientState(const UDCClientState &)             = default;
    UDCClientState & operator=(const UDCClientState &) = default;
    UDCClientState & operator=(UDCClientState &&)      = default;

    const PeerAddress GetPeerAddress() const { return mPeerAddress; }
    void SetPeerAddress(const PeerAddress & address) { mPeerAddress = address; }

    const char * GetInstanceName() const { return mInstanceName; }
    void SetInstanceName(const char * instanceName) { Platform::CopyString(mInstanceName, instanceName); }

    const char * GetDeviceName() const { return mDeviceName; }
    void SetDeviceName(const char * deviceName) { Platform::CopyString(mDeviceName, deviceName); }

    uint16_t GetLongDiscriminator() const { return mLongDiscriminator; }
    void SetLongDiscriminator(uint16_t value) { mLongDiscriminator = value; }

    uint16_t GetVendorId() const { return mVendorId; }
    void SetVendorId(uint16_t value) { mVendorId = value; }

    uint16_t GetProductId() const { return mProductId; }
    void SetProductId(uint16_t value) { mProductId = value; }

    uint16_t GetCdPort() const { return mCdPort; }
    void SetCdPort(uint16_t port) { mCdPort = port; }

    const uint8_t * GetRotatingId() const { return mRotatingId; }
    size_t GetRotatingIdLength() const { return mRotatingIdLen; }
    void SetRotatingId(const uint8_t * rotatingId, size_t rotatingIdLen)
    {
        size_t maxSize = MATTER_ARRAY_SIZE(mRotatingId);
        mRotatingIdLen = (maxSize < rotatingIdLen) ? maxSize : rotatingIdLen;
        memcpy(mRotatingId, rotatingId, mRotatingIdLen);
    }

    const char * GetPairingInst() const { return mPairingInst; }
    void SetPairingInst(const char * pairingInst) { Platform::CopyString(mPairingInst, pairingInst); }

    uint16_t GetPairingHint() const { return mPairingHint; }
    void SetPairingHint(uint16_t pairingHint) { mPairingHint = pairingHint; }

    bool GetTargetAppInfo(uint8_t index, TargetAppInfo & info) const
    {
        if (index < mNumTargetAppInfos)
        {
            info.vendorId   = mTargetAppInfos[index].vendorId;
            info.productId  = mTargetAppInfos[index].productId;
            info.checkState = mTargetAppInfos[index].checkState;
            info.passcode   = mTargetAppInfos[index].passcode;
            info.foundApp   = mTargetAppInfos[index].foundApp;
            return true;
        }
        return false;
    }
    void SetTargetAppInfoState(uint8_t index, TargetAppCheckState checkState)
    {
        if (index < mNumTargetAppInfos)
        {
            mTargetAppInfos[index].checkState = checkState;
        }
    }
    uint8_t GetNumTargetAppInfos() const { return mNumTargetAppInfos; }

    bool AddTargetAppInfo(TargetAppInfo vid)
    {
        if (mNumTargetAppInfos >= kMaxTargetAppInfos)
        {
            // already at max
            return false;
        }
        mTargetAppInfos[mNumTargetAppInfos].vendorId   = vid.vendorId;
        mTargetAppInfos[mNumTargetAppInfos].productId  = vid.productId;
        mTargetAppInfos[mNumTargetAppInfos].checkState = TargetAppCheckState::kNotInitialized;
        mTargetAppInfos[mNumTargetAppInfos].passcode   = 0;
        mTargetAppInfos[mNumTargetAppInfos].foundApp   = false;
        mNumTargetAppInfos++;
        return true;
    }

    UDCClientProcessingState GetUDCClientProcessingState() const { return mUDCClientProcessingState; }
    void SetUDCClientProcessingState(UDCClientProcessingState state) { mUDCClientProcessingState = state; }

    System::Clock::Timestamp GetExpirationTime() const { return mExpirationTime; }
    void SetExpirationTime(System::Clock::Timestamp value) { mExpirationTime = value; }

    bool IsInitialized(System::Clock::Timestamp currentTime)
    {
        // if state is not the "not-initialized" and it has not expired
        return (mUDCClientProcessingState != UDCClientProcessingState::kNotInitialized && mExpirationTime > currentTime);
    }

    void SetNoPasscode(bool newValue) { mNoPasscode = newValue; };
    bool GetNoPasscode() const { return mNoPasscode; };

    void SetCdUponPasscodeDialog(bool newValue) { mCdUponPasscodeDialog = newValue; };
    bool GetCdUponPasscodeDialog() const { return mCdUponPasscodeDialog; };

    void SetCommissionerPasscode(bool newValue) { mCommissionerPasscode = newValue; };
    bool GetCommissionerPasscode() const { return mCommissionerPasscode; };

    void SetCommissionerPasscodeReady(bool newValue) { mCommissionerPasscodeReady = newValue; };
    bool GetCommissionerPasscodeReady() const { return mCommissionerPasscodeReady; };

    void SetCancelPasscode(bool newValue) { mCancelPasscode = newValue; };
    bool GetCancelPasscode() const { return mCancelPasscode; };

    void SetCachedCommissionerPasscode(uint32_t newValue) { mCachedCommissionerPasscode = newValue; };
    uint32_t GetCachedCommissionerPasscode() const { return mCachedCommissionerPasscode; };

    /**
     *  Reset the connection state to a completely uninitialized status.
     */
    void Reset()
    {
        mPeerAddress                = PeerAddress::Uninitialized();
        mLongDiscriminator          = 0;
        mVendorId                   = 0;
        mProductId                  = 0;
        mRotatingIdLen              = 0;
        mCdPort                     = 0;
        mDeviceName[0]              = '\0';
        mPairingInst[0]             = '\0';
        mPairingHint                = 0;
        mNoPasscode                 = false;
        mCdUponPasscodeDialog       = false;
        mCommissionerPasscode       = false;
        mCommissionerPasscodeReady  = false;
        mCancelPasscode             = false;
        mExpirationTime             = System::Clock::kZero;
        mUDCClientProcessingState   = UDCClientProcessingState::kNotInitialized;
        mCachedCommissionerPasscode = 0;
        mNumTargetAppInfos          = 0;
    }

private:
    PeerAddress mPeerAddress;
    char mInstanceName[Dnssd::Commission::kInstanceNameMaxLength + 1];
    char mDeviceName[Dnssd::kMaxDeviceNameLen + 1];
    uint16_t mLongDiscriminator = 0;
    uint16_t mVendorId          = 0;
    uint16_t mProductId         = 0;
    uint16_t mCdPort            = 0;
    uint8_t mRotatingId[chip::Dnssd::kMaxRotatingIdLen];
    size_t mRotatingIdLen                                         = 0;
    char mPairingInst[chip::Dnssd::kMaxPairingInstructionLen + 1] = {};
    uint16_t mPairingHint                                         = 0;

    constexpr static size_t kMaxTargetAppInfos = CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS;
    uint8_t mNumTargetAppInfos                 = 0; // number of vendor Ids
    TargetAppInfo mTargetAppInfos[kMaxTargetAppInfos];

    bool mNoPasscode                = false;
    bool mCdUponPasscodeDialog      = false;
    bool mCommissionerPasscode      = false;
    bool mCommissionerPasscodeReady = false;
    bool mCancelPasscode            = false;

    UDCClientProcessingState mUDCClientProcessingState;
    System::Clock::Timestamp mExpirationTime = System::Clock::kZero;

    uint32_t mCachedCommissionerPasscode = 0;
};

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
