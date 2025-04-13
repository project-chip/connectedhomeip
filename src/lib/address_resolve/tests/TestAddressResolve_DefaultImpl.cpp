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

#include <pw_unit_test/framework.h>

#include <lib/address_resolve/AddressResolve_DefaultImpl.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/IPAddressSorter.h>
#include <lib/support/StringBuilder.h>
#include <transport/raw/PeerAddress.h>

using namespace chip;
using namespace chip::AddressResolve;

namespace pw {

template <>
StatusWithSize ToString<Transport::PeerAddress>(const Transport::PeerAddress & addr, pw::span<char> buffer)
{
    char buff[Transport::PeerAddress::kMaxToStringSize];
    addr.ToString(buff);
    return pw::string::Format(buffer, "IP<%s>", buff);
}

} // namespace pw

namespace {

using chip::Dnssd::IPAddressSorter::IpScore;
using chip::Dnssd::IPAddressSorter::ScoreIpAddress;

constexpr uint8_t kNumberOfAvailableSlots = CHIP_CONFIG_MDNS_RESOLVE_LOOKUP_RESULTS;

/// Get an address that should have `kUniqueLocal` (one of the lowest) priority.
///
/// Since for various tests we check filling the cache with values, we allow
/// unique address generation by varying the `idx` parameter
///
/// @param idx - a value to generate a unique IP address (in case we do not want dedups to happen)
/// @param port - port in case some tests would like to vary it. Required for PeerAddress
/// @param interfaceId - interface required for PeerAddress
Transport::PeerAddress GetAddressWithLowScore(uint16_t idx = 4, uint16_t port = CHIP_PORT,
                                              Inet::InterfaceId interfaceId = Inet::InterfaceId::Null())
{
    // Unique Local - expect score "3"
    Inet::IPAddress ipAddress;

    auto high = static_cast<uint8_t>(idx >> 8);
    auto low  = static_cast<uint8_t>(idx & 0xFF);

    StringBuilder<64> address;
    address.Add("fdff:aabb:ccdd:1::");
    if (high != 0)
    {
        address.AddFormat("%x:", high);
    }
    address.AddFormat("%x", low);

    if (!Inet::IPAddress::FromString(address.c_str(), ipAddress))
    {
        ChipLogError(NotSpecified, "!!!!!!!! IP Parse failure for %s", address.c_str());
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

#if CHIP_CONFIG_MDNS_RESOLVE_LOOKUP_RESULTS >= 3

// test requires at least 3 slots: for high, medium and low
TEST(TestAddressResolveDefaultImpl, UpdateResultsDoesNotAddDuplicatesWhenFull)
{
    Impl::NodeLookupResults results;
    ASSERT_EQ(results.count, 0);

    for (auto i = 0; i < kNumberOfAvailableSlots; i++)
    {
        ResolveResult result;
        result.address = GetAddressWithLowScore(static_cast<uint16_t>(i + 10));
        ASSERT_TRUE(results.UpdateResults(result, Dnssd::IPAddressSorter::IpScore::kUniqueLocal));
    }
    ASSERT_EQ(results.count, kNumberOfAvailableSlots);

    // Adding another one should fail as there is no more room
    ResolveResult result;
    result.address = GetAddressWithLowScore(static_cast<uint16_t>(5));
    ASSERT_FALSE(results.UpdateResults(result, Dnssd::IPAddressSorter::IpScore::kUniqueLocal));
    ASSERT_EQ(results.count, kNumberOfAvailableSlots);

    // however one with higher priority should work
    result.address = GetAddressWithHighScore();
    ASSERT_TRUE(results.UpdateResults(result, Dnssd::IPAddressSorter::IpScore::kGlobalUnicast));
    ASSERT_EQ(results.count, kNumberOfAvailableSlots);

    // however not duplicate
    ASSERT_FALSE(results.UpdateResults(result, Dnssd::IPAddressSorter::IpScore::kGlobalUnicast));
    ASSERT_EQ(results.count, kNumberOfAvailableSlots);

    // another higher priority one
    result.address = GetAddressWithMediumScore();
    ASSERT_TRUE(results.UpdateResults(result, Dnssd::IPAddressSorter::IpScore::kLinkLocal));
    ASSERT_EQ(results.count, kNumberOfAvailableSlots);

    // however not duplicate
    ASSERT_FALSE(results.UpdateResults(result, Dnssd::IPAddressSorter::IpScore::kLinkLocal));
    ASSERT_EQ(results.count, kNumberOfAvailableSlots);
}

// test requires at least 3 slots: for high, medium and low
TEST(TestAddressResolveDefaultImpl, UpdateResultsDoesNotAddDuplicates)
{
    static_assert(Impl::kNodeLookupResultsLen >= 3, "Test uses 3 address slots");

    Impl::NodeLookupResults results;
    ASSERT_EQ(results.count, 0);

    // The order below is VERY explicit to test both before and after inserts
    //   - low first
    //   - high (to be before low)
    //   - medium (to be after high, even though before low)

    ResolveResult lowResult;
    lowResult.address = GetAddressWithLowScore();

    ResolveResult mediumResult;
    mediumResult.address = GetAddressWithMediumScore();

    ResolveResult highResult;
    highResult.address = GetAddressWithHighScore();

    results.UpdateResults(lowResult, Dnssd::IPAddressSorter::IpScore::kUniqueLocal);
    ASSERT_EQ(results.count, 1);

    // same address again. we should not actually insert it!
    results.UpdateResults(lowResult, Dnssd::IPAddressSorter::IpScore::kUniqueLocal);
    ASSERT_EQ(results.count, 1);

    // we CAN insert a different one
    results.UpdateResults(highResult, Dnssd::IPAddressSorter::IpScore::kGlobalUnicast);
    ASSERT_EQ(results.count, 2);

    // extra insertions of the same address should NOT make a difference
    results.UpdateResults(lowResult, Dnssd::IPAddressSorter::IpScore::kUniqueLocal);
    ASSERT_EQ(results.count, 2);
    results.UpdateResults(highResult, Dnssd::IPAddressSorter::IpScore::kGlobalUnicast);
    ASSERT_EQ(results.count, 2);

    // we CAN insert a different one
    results.UpdateResults(mediumResult, Dnssd::IPAddressSorter::IpScore::kLinkLocal);
    ASSERT_EQ(results.count, 3);

    // re-insertin any of these should not make a difference
    results.UpdateResults(lowResult, Dnssd::IPAddressSorter::IpScore::kUniqueLocal);
    ASSERT_EQ(results.count, 3);
    results.UpdateResults(highResult, Dnssd::IPAddressSorter::IpScore::kGlobalUnicast);
    ASSERT_EQ(results.count, 3);
    results.UpdateResults(mediumResult, Dnssd::IPAddressSorter::IpScore::kLinkLocal);
    ASSERT_EQ(results.count, 3);
}

#endif

TEST(TestAddressResolveDefaultImpl, TestLookupResult)
{
    ResolveResult lowResult;
    lowResult.address = GetAddressWithLowScore(static_cast<uint16_t>(1));

    ResolveResult mediumResult;
    mediumResult.address = GetAddressWithMediumScore();

    ResolveResult highResult;
    highResult.address = GetAddressWithHighScore();

    // Ensure test expectations regarding ordering is matched

    IpScore lowScore    = ScoreIpAddress(lowResult.address.GetIPAddress(), Inet::InterfaceId::Null());
    IpScore mediumScore = ScoreIpAddress(mediumResult.address.GetIPAddress(), Inet::InterfaceId::Null());
    IpScore highScore   = ScoreIpAddress(highResult.address.GetIPAddress(), Inet::InterfaceId::Null());

    EXPECT_LT(to_underlying(lowScore), to_underlying(mediumScore));
    EXPECT_LT(to_underlying(mediumScore), to_underlying(highScore));

    ResolveResult outResult;

    AddressResolve::NodeLookupHandle handle;

    auto now     = System::SystemClock().GetMonotonicTimestamp();
    auto request = NodeLookupRequest(chip::PeerId(1, 2));
    handle.ResetForLookup(now, request);

    // Check that no result exists.
    EXPECT_FALSE(handle.HasLookupResult());

    // Fill a single slot.
    handle.LookupResult(lowResult);

    // Check that a result exists.
    EXPECT_TRUE(handle.HasLookupResult());

    // Check that the result match what has been inserted.
    outResult = handle.TakeLookupResult();
    EXPECT_EQ(lowResult.address, outResult.address);

    // Check that the result has been consumed properly
    EXPECT_FALSE(handle.HasLookupResult());

    handle.ResetForLookup(now, request);

    // Fill all the possible slots.
    for (auto i = 0; i < kNumberOfAvailableSlots; i++)
    {
        // Set up UNIQUE addresses to not apply dedup here
        lowResult.address = GetAddressWithLowScore(static_cast<uint16_t>(i + 10));
        handle.LookupResult(lowResult);
    }

    // Read back all results and validate that they match the input.
    for (auto i = 0; i < kNumberOfAvailableSlots; i++)
    {
        EXPECT_TRUE(handle.HasLookupResult());
        outResult = handle.TakeLookupResult();
        EXPECT_EQ(GetAddressWithLowScore(static_cast<uint16_t>(i + 10)), outResult.address);
    }

    // Check that the results has been consumed properly.
    EXPECT_FALSE(handle.HasLookupResult());

    handle.ResetForLookup(now, request);

    // Fill all the possible slots by giving it 2 times more results than the available slots.
    for (auto i = 0; i < kNumberOfAvailableSlots * 2; i++)
    {
        lowResult.address = GetAddressWithLowScore(static_cast<uint16_t>(i + 1000));
        handle.LookupResult(lowResult);
    }

    // Read back all results and validate that they match the input.
    for (auto i = 0; i < kNumberOfAvailableSlots; i++)
    {
        EXPECT_TRUE(handle.HasLookupResult());
        outResult = handle.TakeLookupResult();
        EXPECT_EQ(GetAddressWithLowScore(static_cast<uint16_t>(i + 1000)), outResult.address);
    }

    // Check that the results has been consumed properly.
    EXPECT_FALSE(handle.HasLookupResult());

    handle.ResetForLookup(now, request);

    // Fill all the possible slots.
    for (auto i = 0; i < kNumberOfAvailableSlots; i++)
    {
        handle.LookupResult(lowResult);
    }

    // Add a result with a medium score and ensure it sits at the top.
    handle.LookupResult(mediumResult);
    EXPECT_TRUE(handle.HasLookupResult());
    outResult = handle.TakeLookupResult();
    EXPECT_EQ(mediumResult.address, outResult.address);

    handle.ResetForLookup(now, request);

    // Fill all the possible slots.
    for (auto i = 0; i < kNumberOfAvailableSlots; i++)
    {
        lowResult.address = GetAddressWithLowScore(static_cast<uint16_t>(i + 10));
        handle.LookupResult(lowResult);
    }

    // Add a result with a medium score and a result with a high score and ensure the result with the high score comes first.
    handle.LookupResult(mediumResult);
    handle.LookupResult(highResult);
    EXPECT_TRUE(handle.HasLookupResult());
    outResult = handle.TakeLookupResult();
    EXPECT_EQ(highResult.address, outResult.address);

    if (kNumberOfAvailableSlots > 1)
    {
        // Ensure the second result is the medium result.
        EXPECT_TRUE(handle.HasLookupResult());
        outResult = handle.TakeLookupResult();
        EXPECT_EQ(mediumResult.address, outResult.address);
    }

    if (kNumberOfAvailableSlots > 2)
    {
        // Ensure that all the other results are low results.
        for (auto i = 2; i < kNumberOfAvailableSlots; i++)
        {
            EXPECT_TRUE(handle.HasLookupResult());
            outResult = handle.TakeLookupResult();
            // - 2 because we start from 2 at the top for the high and medium slots
            EXPECT_EQ(GetAddressWithLowScore(static_cast<uint16_t>(i + 10 - 2)), outResult.address);
        }
    }

    // Check that the results has been consumed properly.
    EXPECT_FALSE(handle.HasLookupResult());
}
} // namespace
