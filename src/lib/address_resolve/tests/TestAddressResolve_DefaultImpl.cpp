/*
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <lib/address_resolve/AddressResolve_DefaultImpl.h>

#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::AddressResolve;

namespace {

using chip::Dnssd::IPAddressSorter::IpScore;
using chip::Dnssd::IPAddressSorter::ScoreIpAddress;

constexpr uint8_t kNumberOfAvailableSlots = CHIP_CONFIG_MDNS_RESOLVE_LOOKUP_RESULTS;

Transport::PeerAddress GetAddressWithLowScore(uint16_t port = CHIP_PORT, Inet::InterfaceId interfaceId = Inet::InterfaceId::Null())
{
    // Unique Local - expect score "3"
    Inet::IPAddress ipAddress;
    if (!Inet::IPAddress::FromString("fdff:aabb:ccdd:1::4", ipAddress))
    {
        ChipLogError(NotSpecified, "!!!!!!!! IP Parse failure");
    }
    return Transport::PeerAddress::UDP(ipAddress, port, interfaceId);
}

Transport::PeerAddress GetAddressWithMediumScore(uint16_t port                 = CHIP_PORT,
                                                 Inet::InterfaceId interfaceId = Inet::InterfaceId::Null())
{

    // Global Unicast - expect score '4'
    Inet::IPAddress ipAddress;
    if (!Inet::IPAddress::FromString("2001::aabb:ccdd:2233:4455", ipAddress))
    {
        ChipLogError(NotSpecified, "!!!!!!!! IP Parse failure");
    }
    return Transport::PeerAddress::UDP(ipAddress, port, interfaceId);
}

Transport::PeerAddress GetAddressWithHighScore(uint16_t port = CHIP_PORT, Inet::InterfaceId interfaceId = Inet::InterfaceId::Null())
{
    // LinkLocal - expect score '7'
    // Likely that the interfaceId is wrong (link local needs it),
    // however we do not expect sorter to care
    Inet::IPAddress ipAddress;
    if (!Inet::IPAddress::FromString("fe80::aabb:ccdd:2233:4455", ipAddress))
    {
        ChipLogError(NotSpecified, "!!!!!!!! IP Parse failure");
    }
    return Transport::PeerAddress::UDP(ipAddress, port, interfaceId);
}

void TestLookupResult(nlTestSuite * inSuite, void * inContext)
{
    ResolveResult lowResult;
    lowResult.address = GetAddressWithLowScore();

    ResolveResult mediumResult;
    mediumResult.address = GetAddressWithMediumScore();

    ResolveResult highResult;
    highResult.address = GetAddressWithHighScore();

    // Ensure test expectations regarding ordering is matched

    IpScore lowScore    = ScoreIpAddress(lowResult.address.GetIPAddress(), Inet::InterfaceId::Null());
    IpScore mediumScore = ScoreIpAddress(mediumResult.address.GetIPAddress(), Inet::InterfaceId::Null());
    IpScore highScore   = ScoreIpAddress(highResult.address.GetIPAddress(), Inet::InterfaceId::Null());

    NL_TEST_ASSERT(inSuite, to_underlying(lowScore) < to_underlying(mediumScore));
    NL_TEST_ASSERT(inSuite, to_underlying(mediumScore) < to_underlying(highScore));

    ResolveResult outResult;

    AddressResolve::NodeLookupHandle handle;

    auto now     = System::SystemClock().GetMonotonicTimestamp();
    auto request = NodeLookupRequest(chip::PeerId(1, 2));
    handle.ResetForLookup(now, request);

    // Check that no result exists.
    NL_TEST_ASSERT(inSuite, !handle.HasLookupResult());

    // Fill a single slot.
    handle.LookupResult(lowResult);

    // Check that a result exists.
    NL_TEST_ASSERT(inSuite, handle.HasLookupResult());

    // Check that the result match what has been inserted.
    outResult = handle.TakeLookupResult();
    NL_TEST_ASSERT(inSuite, lowResult.address == outResult.address);

    // Check that the result has been consumed properly
    NL_TEST_ASSERT(inSuite, !handle.HasLookupResult());

    handle.ResetForLookup(now, request);

    // Fill all the possible slots.
    for (auto i = 0; i < kNumberOfAvailableSlots; i++)
    {
        handle.LookupResult(lowResult);
    }

    // Read back all results and validate that they match the input.
    for (auto i = 0; i < kNumberOfAvailableSlots; i++)
    {
        NL_TEST_ASSERT(inSuite, handle.HasLookupResult());
        outResult = handle.TakeLookupResult();
        NL_TEST_ASSERT(inSuite, lowResult.address == outResult.address);
    }

    // Check that the results has been consumed properly.
    NL_TEST_ASSERT(inSuite, !handle.HasLookupResult());

    handle.ResetForLookup(now, request);

    // Fill all the possible slots by giving it 2 times more results than the available slots.
    for (auto i = 0; i < kNumberOfAvailableSlots * 2; i++)
    {
        handle.LookupResult(lowResult);
    }

    // Read back all results and validate that they match the input.
    for (auto i = 0; i < kNumberOfAvailableSlots; i++)
    {
        NL_TEST_ASSERT(inSuite, handle.HasLookupResult());
        outResult = handle.TakeLookupResult();
        NL_TEST_ASSERT(inSuite, lowResult.address == outResult.address);
    }

    // Check that the results has been consumed properly.
    NL_TEST_ASSERT(inSuite, !handle.HasLookupResult());

    handle.ResetForLookup(now, request);

    // Fill all the possible slots.
    for (auto i = 0; i < kNumberOfAvailableSlots; i++)
    {
        handle.LookupResult(lowResult);
    }

    // Add a result with a medium score and ensure it sits at the top.
    handle.LookupResult(mediumResult);
    NL_TEST_ASSERT(inSuite, handle.HasLookupResult());
    outResult = handle.TakeLookupResult();
    NL_TEST_ASSERT(inSuite, mediumResult.address == outResult.address);

    handle.ResetForLookup(now, request);

    // Fill all the possible slots.
    for (auto i = 0; i < kNumberOfAvailableSlots; i++)
    {
        handle.LookupResult(lowResult);
    }

    // Add a result with a medium score and a result with a high score and ensure the result with the high score comes first.
    handle.LookupResult(mediumResult);
    handle.LookupResult(highResult);
    NL_TEST_ASSERT(inSuite, handle.HasLookupResult());
    outResult = handle.TakeLookupResult();
    NL_TEST_ASSERT(inSuite, highResult.address == outResult.address);

    if (kNumberOfAvailableSlots > 1)
    {
        // Ensure the second result is the medium result.
        NL_TEST_ASSERT(inSuite, handle.HasLookupResult());
        outResult = handle.TakeLookupResult();
        NL_TEST_ASSERT(inSuite, mediumResult.address == outResult.address);
    }

    if (kNumberOfAvailableSlots > 2)
    {
        // Ensure that all the other results are low results.
        for (auto i = 2; i < kNumberOfAvailableSlots; i++)
        {
            NL_TEST_ASSERT(inSuite, handle.HasLookupResult());
            outResult = handle.TakeLookupResult();
            NL_TEST_ASSERT(inSuite, lowResult.address == outResult.address);
        }
    }

    // Check that the results has been consumed properly.
    NL_TEST_ASSERT(inSuite, !handle.HasLookupResult());
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestLookupResult", TestLookupResult), //
    NL_TEST_SENTINEL()                                 //
};

} // namespace

int TestAddressResolve_DefaultImpl()
{
    nlTestSuite theSuite = { "AddressResolve_DefaultImpl", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestAddressResolve_DefaultImpl)
