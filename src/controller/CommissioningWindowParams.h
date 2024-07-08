/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#pragma once

#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>
#include <setup_payload/SetupPayload.h>
#include <system/SystemClock.h>

namespace chip {
namespace Controller {

template <typename Derived>
class CommissioningWindowCommonParams
{
public:
    CommissioningWindowCommonParams() = default;

    NodeId GetNodeId() const { return mNodeId; }
    // The node identifier of device
    Derived & SetNodeId(NodeId nodeId)
    {
        mNodeId = nodeId;
        return static_cast<Derived &>(*this);
    }

    System::Clock::Seconds16 GetTimeout() const { return mTimeout; }
    // The duration for which the commissioning window should remain open.
    Derived & SetTimeout(System::Clock::Seconds16 timeout)
    {
        mTimeout = timeout;
        return static_cast<Derived &>(*this);
    }
    Derived & SetTimeout(uint16_t timeoutSeconds) { return SetTimeout(System::Clock::Seconds16(timeoutSeconds)); }

    // The PAKE iteration count associated with the PAKE Passcode ID and
    // ephemeral PAKE passcode verifier to be used for this commissioning.
    uint32_t GetIteration() const { return mIteration; }
    Derived & SetIteration(uint32_t iteration)
    {
        mIteration = iteration;
        return static_cast<Derived &>(*this);
    }

    // The long discriminator for the DNS-SD advertisement.
    uint16_t GetDiscriminator() const { return mDiscriminator; }
    Derived & SetDiscriminator(uint16_t discriminator)
    {
        mDiscriminator = discriminator;
        return static_cast<Derived &>(*this);
    }

private:
    NodeId mNodeId                    = 0;
    System::Clock::Seconds16 mTimeout = System::Clock::Seconds16(300);
    uint32_t mIteration               = 1000;
    uint16_t mDiscriminator           = 0;
};

class CommissioningWindowPasscodeParams : public CommissioningWindowCommonParams<CommissioningWindowPasscodeParams>
{
public:
    CommissioningWindowPasscodeParams() = default;

    bool HasSetupPIN() const { return mSetupPIN.HasValue(); }
    // Get the value of salt if present.
    // Returns 0 if absent, make sure to check HasSetupPIN() if a valid value is required.
    uint32_t GetSetupPIN() const { return mSetupPIN.ValueOr(0); }
    // The setup PIN (Passcode) to use. A random one will be generated if not provided.
    CommissioningWindowPasscodeParams & SetSetupPIN(uint32_t setupPIN) { return SetSetupPIN(MakeOptional(setupPIN)); }
    // The setup PIN (Passcode) to use. A random one will be generated if NullOptional is used.
    CommissioningWindowPasscodeParams & SetSetupPIN(Optional<uint32_t> setupPIN)
    {
        mSetupPIN = setupPIN;
        return *this;
    }

    bool HasSalt() const { return mSalt.HasValue(); }
    // Get the value of salt if present.
    // Dies if absent! Make sure to check HasSalt()
    ByteSpan GetSalt() const { return mSalt.Value(); }
    // The salt to use. A random one will be generated if not provided.
    // If provided, must be at least kSpake2p_Min_PBKDF_Salt_Length bytes
    // and at most kSpake2p_Max_PBKDF_Salt_Length bytes in length.
    CommissioningWindowPasscodeParams & SetSalt(ByteSpan salt) { return SetSalt(MakeOptional(salt)); }
    // The salt to use. A random one will be generated if NullOptional is used.
    // If provided, must be at least kSpake2p_Min_PBKDF_Salt_Length bytes
    // and at most kSpake2p_Max_PBKDF_Salt_Length bytes in length.
    // Note that this an overloaded optional arg function to support existing APIs.
    CommissioningWindowPasscodeParams & SetSalt(Optional<ByteSpan> salt)
    {
        mSalt = salt;
        return *this;
    }

    bool GetReadVIDPIDAttributes() const { return mReadVIDPIDAttributes; }
    // Should the API internally read VID and PID from the device while opening the
    // commissioning window.  If this argument is `true`, the API will read VID and PID
    // from the device and include them in the setup payload passed to the callback.
    CommissioningWindowPasscodeParams & SetReadVIDPIDAttributes(bool readVIDPIDAttributes)
    {
        mReadVIDPIDAttributes = readVIDPIDAttributes;
        return *this;
    }

private:
    Optional<uint32_t> mSetupPIN = NullOptional;
    Optional<ByteSpan> mSalt     = NullOptional;
    bool mReadVIDPIDAttributes   = false;
};

class CommissioningWindowVerifierParams : public CommissioningWindowCommonParams<CommissioningWindowVerifierParams>
{
public:
    CommissioningWindowVerifierParams() = default;

    ByteSpan GetVerifier() const { return mVerifier; }
    // The PAKE passcode verifier generated with enclosed iterations, salt and not-enclosed passcode.
    CommissioningWindowVerifierParams & SetVerifier(ByteSpan verifier)
    {
        mVerifier = verifier;
        return *this;
    }

    ByteSpan GetSalt() const { return mSalt; }
    // The salt that was used to generate the verifier.
    // It must be at least kSpake2p_Min_PBKDF_Salt_Length bytes.
    // Note: This is REQUIRED when verifier is used
    CommissioningWindowVerifierParams & SetSalt(ByteSpan salt)
    {
        mSalt = salt;
        return *this;
    }

private:
    ByteSpan mVerifier;
    ByteSpan mSalt;
};

} // namespace Controller
} // namespace chip
