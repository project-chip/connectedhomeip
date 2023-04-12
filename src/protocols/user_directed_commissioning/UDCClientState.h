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
    kCommissioningNode,
    kCommissioningFailed,
};

using PeerAddress = ::chip::Transport::PeerAddress;

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

    UDCClientState(UDCClientState &&)      = default;
    UDCClientState(const UDCClientState &) = default;
    UDCClientState & operator=(const UDCClientState &) = default;
    UDCClientState & operator=(UDCClientState &&) = default;

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

    const uint8_t * GetRotatingId() const { return mRotatingId; }
    size_t GetRotatingIdLength() const { return mRotatingIdLen; }
    void SetRotatingId(const uint8_t * rotatingId, size_t rotatingIdLen)
    {
        size_t maxSize = ArraySize(mRotatingId);
        mRotatingIdLen = (maxSize < rotatingIdLen) ? maxSize : rotatingIdLen;
        memcpy(mRotatingId, rotatingId, mRotatingIdLen);
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

    /**
     *  Reset the connection state to a completely uninitialized status.
     */
    void Reset()
    {
        mPeerAddress              = PeerAddress::Uninitialized();
        mExpirationTime           = System::Clock::kZero;
        mUDCClientProcessingState = UDCClientProcessingState::kNotInitialized;
    }

private:
    PeerAddress mPeerAddress;
    char mInstanceName[Dnssd::Commission::kInstanceNameMaxLength + 1];
    char mDeviceName[Dnssd::kMaxDeviceNameLen + 1];
    uint16_t mLongDiscriminator = 0;
    uint16_t mVendorId;
    uint16_t mProductId;
    uint8_t mRotatingId[chip::Dnssd::kMaxRotatingIdLen];
    size_t mRotatingIdLen = 0;
    UDCClientProcessingState mUDCClientProcessingState;
    System::Clock::Timestamp mExpirationTime = System::Clock::kZero;
};

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
