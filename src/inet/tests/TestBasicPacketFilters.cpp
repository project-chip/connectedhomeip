/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <stdio.h>
#include <string.h>

#include <inet/BasicPacketFilters.h>
#include <inet/IPPacketInfo.h>
#include <inet/InetInterface.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

#include <nlunit-test.h>

namespace {

using namespace chip;
using namespace chip::Inet;

class DropIfTooManyQueuedPacketsHarness : public DropIfTooManyQueuedPacketsFilter
{
public:
    DropIfTooManyQueuedPacketsHarness(size_t maxAllowedQueuedPackets) : DropIfTooManyQueuedPacketsFilter(maxAllowedQueuedPackets) {}

    void OnDropped(const void * endpoint, const IPPacketInfo & pktInfo,
                   const chip::System::PacketBufferHandle & pktPayload) override
    {
        ++mNumOnDroppedCalled;

        // Log a hysteretic event
        if (!mHitCeilingWantFloor)
        {
            mHitCeilingWantFloor = true;
            ChipLogError(Inet, "Hit waterwark, will log as resolved once we get back to empty");
        }
    }

    void OnLastMatchDequeued(const void * endpoint, const IPPacketInfo & pktInfo,
                             const chip::System::PacketBufferHandle & pktPayload) override
    {
        ++mNumOnLastMatchDequeuedCalled;

        // Log a hysteretic event
        if (mHitCeilingWantFloor)
        {
            mHitCeilingWantFloor = false;
            ChipLogError(Inet, "Resolved burst, got back to fully empty.");
        }
    }

    // Public bits to make testing easier
    nlTestSuite * mTestSuite          = nullptr;
    int mNumOnDroppedCalled           = 0;
    int mNumOnLastMatchDequeuedCalled = 0;
    bool mHitCeilingWantFloor;
};

class FilterDriver
{
public:
    FilterDriver(EndpointQueueFilter * filter, const void * endpoint) : mFilter(filter), mEndpoint(endpoint) {}

    EndpointQueueFilter::FilterOutcome ProcessEnqueue(const IPAddress & srcAddr, uint16_t srcPort, const IPAddress & dstAddr,
                                                      uint16_t dstPort, ByteSpan payload)
    {
        VerifyOrDie(mFilter != nullptr);

        chip::Inet::IPPacketInfo pktInfo;
        pktInfo.SrcAddress  = srcAddr;
        pktInfo.DestAddress = dstAddr;
        pktInfo.SrcPort     = srcPort;
        pktInfo.DestPort    = dstPort;

        auto pktPayload = chip::System::PacketBufferHandle::NewWithData(payload.data(), payload.size());

        return mFilter->FilterBeforeEnqueue(mEndpoint, pktInfo, pktPayload);
    }

    EndpointQueueFilter::FilterOutcome ProcessDequeue(const IPAddress & srcAddr, uint16_t srcPort, const IPAddress & dstAddr,
                                                      uint16_t dstPort, ByteSpan payload)
    {
        VerifyOrDie(mFilter != nullptr);

        chip::Inet::IPPacketInfo pktInfo;
        pktInfo.SrcAddress  = srcAddr;
        pktInfo.DestAddress = dstAddr;
        pktInfo.SrcPort     = srcPort;
        pktInfo.DestPort    = dstPort;

        auto pktPayload = chip::System::PacketBufferHandle::NewWithData(payload.data(), payload.size());

        return mFilter->FilterAfterDequeue(mEndpoint, pktInfo, pktPayload);
    }

protected:
    EndpointQueueFilter * mFilter = nullptr;
    const void * mEndpoint        = nullptr;
};

DropIfTooManyQueuedPacketsHarness gFilter(0);
int gFakeEndpointForPointer = 0;

void TestBasicPacketFilter(nlTestSuite * inSuite, void * inContext)
{
    constexpr uint16_t kMdnsPort = 5353u;

    gFilter.mTestSuite = inSuite;

    // Predicate for test is filter that destination port is 5353 (mDNS).
    // NOTE: A non-capturing lambda is used, but a plain function could have been used as well...
    auto predicate = [](void * context, const void * endpoint, const chip::Inet::IPPacketInfo & pktInfo,
                        const chip::System::PacketBufferHandle & pktPayload) -> bool {
        auto filter           = reinterpret_cast<DropIfTooManyQueuedPacketsHarness *>(context);
        auto testSuite        = filter->mTestSuite;
        auto expectedEndpoint = &gFakeEndpointForPointer;

        // Ensure we get called with context and expected endpoint pointer
        NL_TEST_ASSERT(testSuite, context == &gFilter);
        NL_TEST_ASSERT(testSuite, endpoint == expectedEndpoint);

        // Predicate filters destination port being 5353
        return (pktInfo.DestPort == kMdnsPort);
    };
    gFilter.SetPredicate(predicate, &gFilter);

    FilterDriver fakeUdpEndpoint(&gFilter, &gFakeEndpointForPointer);

    IPAddress fakeSrc;
    IPAddress fakeDest;
    IPAddress fakeMdnsDest;
    constexpr uint16_t kOtherPort    = 43210u;
    const uint8_t kFakePayloadData[] = { 1, 2, 3 };
    const ByteSpan kFakePayload{ kFakePayloadData };

    NL_TEST_ASSERT(inSuite, IPAddress::FromString("fe80::aaaa:bbbb:cccc:dddd", fakeSrc));
    NL_TEST_ASSERT(inSuite, IPAddress::FromString("fe80::0000:1111:2222:3333", fakeDest));
    NL_TEST_ASSERT(inSuite, IPAddress::FromString("ff02::fb", fakeMdnsDest));

    // Shorthands for simplifying asserts
    constexpr EndpointQueueFilter::FilterOutcome kAllowPacket = EndpointQueueFilter::FilterOutcome::kAllowPacket;
    constexpr EndpointQueueFilter::FilterOutcome kDropPacket  = EndpointQueueFilter::FilterOutcome::kDropPacket;

    constexpr int kMaxQueuedPacketsLimit = 3;
    gFilter.SetMaxQueuedPacketsLimit(kMaxQueuedPacketsLimit);

    {
        // Enqueue some packets that don't match filter, all allowed, never hit the drop
        for (int numPkt = 0; numPkt < (kMaxQueuedPacketsLimit + 1); ++numPkt)
        {
            NL_TEST_ASSERT(inSuite,
                           kAllowPacket == fakeUdpEndpoint.ProcessEnqueue(fakeSrc, kOtherPort, fakeDest, kOtherPort, kFakePayload));
        }
        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 0);

        // Dequeue all packets
        for (int numPkt = 0; numPkt < (kMaxQueuedPacketsLimit + 1); ++numPkt)
        {
            NL_TEST_ASSERT(inSuite,
                           kAllowPacket == fakeUdpEndpoint.ProcessDequeue(fakeSrc, kOtherPort, fakeDest, kOtherPort, kFakePayload));
        }
        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 0);

        // OnDroped/OnLastMatchDequeued only ever called for matching packets, never for non-matching
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 0);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 0);
    }

    {
        // Enqueue packets that match filter, up to watermark. None dropped
        for (int numPkt = 0; numPkt < kMaxQueuedPacketsLimit; ++numPkt)
        {
            NL_TEST_ASSERT(inSuite,
                           kAllowPacket ==
                               fakeUdpEndpoint.ProcessEnqueue(fakeSrc, kOtherPort, fakeMdnsDest, kMdnsPort, kFakePayload));
        }
        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 0);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 0);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 0);

        // Enqueue packets that match filter, beyond watermark: all dropped.
        for (int numPkt = 0; numPkt < 2; ++numPkt)
        {
            NL_TEST_ASSERT(
                inSuite, kDropPacket == fakeUdpEndpoint.ProcessEnqueue(fakeSrc, kOtherPort, fakeMdnsDest, kMdnsPort, kFakePayload));
        }
        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 2);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 2);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 0);

        // Dequeue 2 packets that were enqueued, matching filter
        for (int numPkt = 0; numPkt < 2; ++numPkt)
        {
            NL_TEST_ASSERT(inSuite,
                           kAllowPacket ==
                               fakeUdpEndpoint.ProcessDequeue(fakeSrc, kOtherPort, fakeMdnsDest, kMdnsPort, kFakePayload));
        }
        // Number of dropped packets didn't change
        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 2);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 2);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 0);

        // Enqueue packets that match filter, up to watermark again. None dropped.
        for (int numPkt = 0; numPkt < 2; ++numPkt)
        {
            NL_TEST_ASSERT(inSuite,
                           kAllowPacket ==
                               fakeUdpEndpoint.ProcessEnqueue(fakeSrc, kOtherPort, fakeMdnsDest, kMdnsPort, kFakePayload));
        }

        // No change from prior state
        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 2);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 2);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 0);

        // Enqueue two more packets, expect drop
        for (int numPkt = 0; numPkt < 2; ++numPkt)
        {
            NL_TEST_ASSERT(
                inSuite, kDropPacket == fakeUdpEndpoint.ProcessEnqueue(fakeSrc, kOtherPort, fakeMdnsDest, kMdnsPort, kFakePayload));
        }

        // Expect two more dropped total
        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 4);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 4);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 0);

        // Enqueue non-matching packet, expect allowed.
        for (int numPkt = 0; numPkt < kMaxQueuedPacketsLimit; ++numPkt)
        {
            NL_TEST_ASSERT(inSuite,
                           kAllowPacket == fakeUdpEndpoint.ProcessEnqueue(fakeSrc, kOtherPort, fakeDest, kOtherPort, kFakePayload));
        }

        // Expect no more dropepd
        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 4);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 4);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 0);

        // Dequeue non-matching packet, expect allowed.
        for (int numPkt = 0; numPkt < kMaxQueuedPacketsLimit; ++numPkt)
        {
            NL_TEST_ASSERT(inSuite,
                           kAllowPacket == fakeUdpEndpoint.ProcessDequeue(fakeSrc, kOtherPort, fakeDest, kOtherPort, kFakePayload));
        }

        // Expect no change
        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 4);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 4);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 0);

        // Dequeue all matching packets, expect allowed and one OnLastMatchDequeued on last one.
        for (int numPkt = 0; numPkt < (kMaxQueuedPacketsLimit - 1); ++numPkt)
        {
            NL_TEST_ASSERT(inSuite,
                           kAllowPacket ==
                               fakeUdpEndpoint.ProcessDequeue(fakeSrc, kOtherPort, fakeMdnsDest, kMdnsPort, kFakePayload));
        }

        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 4);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 4);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 0);

        NL_TEST_ASSERT(inSuite,
                       kAllowPacket == fakeUdpEndpoint.ProcessDequeue(fakeSrc, kOtherPort, fakeMdnsDest, kMdnsPort, kFakePayload));

        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 4);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 4);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 1);
    }

    // Validate that clearing drop count works
    {
        gFilter.ClearNumDroppedPackets();
        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 0);

        gFilter.mNumOnDroppedCalled           = 0;
        gFilter.mNumOnLastMatchDequeuedCalled = 0;
    }

    // Validate that all packets pass when no predicate set
    {
        gFilter.SetPredicate(nullptr, nullptr);

        // Enqueue packets up to twice the watermark. None dropped.
        for (int numPkt = 0; numPkt < (2 * kMaxQueuedPacketsLimit); ++numPkt)
        {
            NL_TEST_ASSERT(inSuite,
                           kAllowPacket ==
                               fakeUdpEndpoint.ProcessEnqueue(fakeSrc, kOtherPort, fakeMdnsDest, kMdnsPort, kFakePayload));
        }
        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 0);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 0);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 0);

        // Works even if max number of packets allowed is zero
        gFilter.SetMaxQueuedPacketsLimit(0);

        // Enqueue packets up to twice the watermark. None dropped.
        for (int numPkt = 0; numPkt < (2 * kMaxQueuedPacketsLimit); ++numPkt)
        {
            NL_TEST_ASSERT(inSuite,
                           kAllowPacket ==
                               fakeUdpEndpoint.ProcessEnqueue(fakeSrc, kOtherPort, fakeMdnsDest, kMdnsPort, kFakePayload));
        }
        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 0);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 0);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 0);
    }

    // Validate that setting max packets to zero, with a matching predicate, drops all matching packets, none of the non-matching.
    {
        gFilter.SetPredicate(predicate, &gFilter);
        gFilter.SetMaxQueuedPacketsLimit(0);

        // Enqueue packets that match filter, up to watermark. All dropped
        for (int numPkt = 0; numPkt < kMaxQueuedPacketsLimit; ++numPkt)
        {
            NL_TEST_ASSERT(
                inSuite, kDropPacket == fakeUdpEndpoint.ProcessEnqueue(fakeSrc, kOtherPort, fakeMdnsDest, kMdnsPort, kFakePayload));
        }

        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 3);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 3);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 0);

        // Enqueue non-filter-matching, none dropped
        for (int numPkt = 0; numPkt < kMaxQueuedPacketsLimit; ++numPkt)
        {
            NL_TEST_ASSERT(inSuite,
                           kAllowPacket == fakeUdpEndpoint.ProcessDequeue(fakeSrc, kOtherPort, fakeDest, kOtherPort, kFakePayload));
        }

        NL_TEST_ASSERT(inSuite, gFilter.GetNumDroppedPackets() == 3);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnDroppedCalled == 3);
        NL_TEST_ASSERT(inSuite, gFilter.mNumOnLastMatchDequeuedCalled == 0);
    }
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestBasicPacketFilter", TestBasicPacketFilter), //
    NL_TEST_SENTINEL()                                           //
};

int TestSuiteSetup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

int TestSuiteTeardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

int TestBasicPacketFilters()
{
    nlTestSuite theSuite = { "TestBasicPacketFilters", sTests, &TestSuiteSetup, &TestSuiteTeardown };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestBasicPacketFilters)
