/*
 *    Copyright (c) 2026 Project CHIP Authors
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

// FuzzTest harness for WiFiPAFEndPoint::Receive. The injected layer delegate returns send/receive
// errors and withholds resources on a fuzz-controlled schedule (modeling a congested/closing
// transport), so the endpoint's error / close / back-pressure paths are exercised alongside the
// data-phase reassembly. Fragments are seeded with real PAFTP shapes.

#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>
#include <wifipaf/WiFiPAFEndPoint.h>
#include <wifipaf/WiFiPAFLayer.h>
#include <wifipaf/WiFiPAFLayerDelegate.h>

namespace chip {
namespace WiFiPAF {

// Reuses the friended TestWiFiPAFLayer name to bring up an endpoint without the full handshake.
class TestWiFiPAFLayer : public WiFiPAFLayer, private WiFiPAFLayerDelegate
{
public:
    CHIP_ERROR FuzzSetup(uint32_t sendErrMask, uint32_t recvErrMask, bool resourceAvail)
    {
        mSendErrMask   = sendErrMask;
        mRecvErrMask   = recvErrMask;
        mResourceAvail = resourceAvail;
        ReturnErrorOnFailure(Init(&DeviceLayer::SystemLayer()));
        mWiFiPAFTransport = this;
        return CHIP_NO_ERROR;
    }

    void FuzzTeardown()
    {
        Shutdown();
        mWiFiPAFTransport = nullptr;
    }

    CHIP_ERROR WiFiPAFMessageReceived(WiFiPAFSession &, System::PacketBufferHandle &&) override { return NextError(mRecvErrMask); }
    CHIP_ERROR WiFiPAFMessageSend(WiFiPAFSession &, System::PacketBufferHandle &&) override { return NextError(mSendErrMask); }
    CHIP_ERROR WiFiPAFCloseSession(WiFiPAFSession &) override { return CHIP_NO_ERROR; }
    bool WiFiPAFResourceAvailable() override { return mResourceAvail; }

private:
    // A set bit returns a transport error (as a real backend does when its queue is full).
    CHIP_ERROR NextError(uint32_t & mask)
    {
        const bool fail = (mask & 1u) != 0u;
        mask >>= 1;
        return fail ? CHIP_ERROR_NOT_CONNECTED : CHIP_NO_ERROR;
    }

    uint32_t mSendErrMask = 0;
    uint32_t mRecvErrMask = 0;
    bool mResourceAvail   = true;
};

} // namespace WiFiPAF
} // namespace chip

namespace {

using chip::System::PacketBufferHandle;
using chip::WiFiPAF::kWiFiPafRole_Publisher;
using chip::WiFiPAF::kWiFiPafRole_Subscriber;
using chip::WiFiPAF::PafInfoAccess;
using chip::WiFiPAF::State;
using chip::WiFiPAF::TestWiFiPAFLayer;
using chip::WiFiPAF::WiFiPAFEndPoint;
using chip::WiFiPAF::WiFiPafRole;
using chip::WiFiPAF::WiFiPAFSession;

using namespace fuzztest;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(chip::Platform::MemoryInit() == CHIP_NO_ERROR);
        VerifyOrDie(chip::DeviceLayer::SystemLayer().Init() == CHIP_NO_ERROR);
        std::atexit([] { chip::DeviceLayer::SystemLayer().Shutdown(); });
        return true;
    }();
    (void) sInitialized;
}

auto AnyRole()
{
    return ElementOf<WiFiPafRole>({ kWiFiPafRole_Publisher, kWiFiPafRole_Subscriber });
}

auto AnyStartState()
{
    return ElementOf<uint8_t>({ static_cast<uint8_t>(WiFiPAFEndPoint::kState_Ready),
                                static_cast<uint8_t>(WiFiPAFEndPoint::kState_Connecting),
                                static_cast<uint8_t>(WiFiPAFEndPoint::kState_Connected) });
}

std::vector<std::vector<uint8_t>> CapabilitySeeds()
{
    return {
        { 0x65, 0x6c, 0x04, 0x00, 0x00, 0x00, 0x5e, 0x01, 0x06 },
        { 0x65, 0x6c, 0x04, 0x5b, 0x01, 0x06 },
    };
}

std::vector<std::vector<uint8_t>> PaftpSeeds()
{
    return {
        { 0x05, 0x01, 0x01, 0x00, 0x00 },
        { 0x05, 0x01, 0x03, 0x00, 0x00 },
        { 0x01, 0x00, 0x01, 0x00, 0x00 },
        { 0x04, 0x01, 0x02, 0x00 },
    };
}

void EndpointReceiveDoesNotCrash(WiFiPafRole role, uint8_t startState, uint32_t sessionId, uint16_t discriminator,
                                 uint32_t sendErrMask, uint32_t recvErrMask, bool resourceAvail, const std::vector<uint8_t> & frag0,
                                 const std::vector<uint8_t> & frag1, const std::vector<uint8_t> & frag2,
                                 const std::vector<uint8_t> & frag3)
{
    EnsureInitialized();

    TestWiFiPAFLayer harness;
    ASSERT_EQ(harness.FuzzSetup(sendErrMask, recvErrMask, resourceAvail), CHIP_NO_ERROR);

    WiFiPAFSession session = {};
    session.role           = role;
    // 0 and kUndefinedWiFiPafSessionId are sentinels Shutdown() skips, which would leak the endpoint across iterations.
    session.id            = (sessionId == 0 || sessionId == chip::WiFiPAF::kUndefinedWiFiPafSessionId) ? 1 : sessionId;
    session.peer_id       = 1;
    session.nodeId        = 1;
    session.discriminator = discriminator;

    WiFiPAFEndPoint * ep = nullptr;
    if (harness.NewEndPoint(&ep, session, role) != CHIP_NO_ERROR || ep == nullptr)
    {
        harness.FuzzTeardown();
        return;
    }
    // Best-effort registration so Shutdown() can find the endpoint; a duplicate/full table is fine here.
    RETURN_SAFELY_IGNORED harness.AddPafSession(PafInfoAccess::kAccSessionId, session);
    ep->mState = static_cast<decltype(ep->mState)>(startState);
    harness.SetWiFiPAFState(startState == WiFiPAFEndPoint::kState_Connected ? State::kConnected : State::kInitialized);

    for (const auto * frag : { &frag0, &frag1, &frag2, &frag3 })
    {
        auto buf = PacketBufferHandle::NewWithData(frag->data(), frag->size());
        if (buf.IsNull())
        {
            continue;
        }
        // Fuzzer asserts only the no-crash property; Receive's status is irrelevant here.
        RETURN_SAFELY_IGNORED ep->Receive(std::move(buf));
        if (ep->mState == WiFiPAFEndPoint::kState_Closed || ep->mState == WiFiPAFEndPoint::kState_Aborting)
        {
            break;
        }
    }

    // FuzzTeardown()->Shutdown() closes the registered endpoint through the production DoClose path
    // (cancels timers, frees buffers, returns the static pool slot via ClearAll), so the slot is reusable
    // next iteration without poking mWiFiPafLayer directly. The session-id clamp above keeps it findable.
    harness.FuzzTeardown();
}

FUZZ_TEST(FuzzWiFiPAFEndPointPW, EndpointReceiveDoesNotCrash)
    .WithDomains(AnyRole(), AnyStartState(), Arbitrary<uint32_t>(), Arbitrary<uint16_t>(),
                 Arbitrary<uint32_t>(), // send-failure schedule
                 Arbitrary<uint32_t>(), // receive-failure schedule
                 Arbitrary<bool>(),     // resource availability / back-pressure
                 Arbitrary<std::vector<uint8_t>>().WithSeeds(CapabilitySeeds()),
                 Arbitrary<std::vector<uint8_t>>().WithSeeds(PaftpSeeds()),
                 Arbitrary<std::vector<uint8_t>>().WithSeeds(PaftpSeeds()),
                 Arbitrary<std::vector<uint8_t>>().WithSeeds(PaftpSeeds()));

} // namespace
