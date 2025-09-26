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
#include <system/SystemLayerImpl.h>
#include <transport/raw/PeerAddress.h>

#include <functional>
#include <optional>

using namespace chip;
using namespace chip::AddressResolve;
using namespace chip::System::Clock::Literals;
using chip::Dnssd::DiscoveryContext;
using chip::Dnssd::DiscoveryFilter;
using chip::Dnssd::DiscoveryType;
using chip::Dnssd::OperationalResolveDelegate;

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

TEST(TestAddressResolveDefaultImpl, TestNextActionKeepSearchingByReturingRemainingTimeouts)
{
    AddressResolve::NodeLookupHandle handle;

    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    /// now = 0
    auto now     = System::SystemClock().GetMonotonicTimestamp();
    auto request = NodeLookupRequest(chip::PeerId(1, 2));

    request.SetMinLookupTime(100_ms32);
    request.SetMaxLookupTime(200_ms32);

    handle.ResetForLookup(now, request);

    // let's wait 50ms to ensure that the min lookup time is not yet reached
    clock.AdvanceMonotonic(50_ms64);

    // now = 50ms
    now = clock.GetMonotonicTimestamp();

    // timeout should be 50ms, as we are still waiting for the min lookup time
    EXPECT_EQ(handle.NextEventTimeout(now), request.GetMinLookupTime() - 50_ms64);

    // advancing time by another 60ms we reach 110ms after request start
    clock.AdvanceMonotonic(60_ms64);

    // still no results, then timeout should be 90ms, since 110ms have gone by and max lookup time is 200ms
    now = clock.GetMonotonicTimestamp();

    EXPECT_EQ(handle.NextEventTimeout(now), request.GetMaxLookupTime() - 110_ms64);

    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

TEST(TestAddressResolveDefaultImpl, TestNextActionConsumesTimeoutWhenResultsAreFound)
{
    AddressResolve::NodeLookupHandle handle;

    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    ResolveResult lowResult;
    lowResult.address = GetAddressWithLowScore(static_cast<uint16_t>(1));

    /// now = 0
    auto now     = System::SystemClock().GetMonotonicTimestamp();
    auto request = NodeLookupRequest(chip::PeerId(1, 2));

    request.SetMinLookupTime(100_ms32);
    request.SetMaxLookupTime(200_ms32);

    handle.ResetForLookup(now, request);

    // fill in single result
    handle.LookupResult(lowResult);

    // let's move time to the middle of the range [minlookuptime, maxlookuptime]
    clock.AdvanceMonotonic(150_ms64);

    // now = 150ms
    now = clock.GetMonotonicTimestamp();

    // timeout should be consumed now, as we have a result
    EXPECT_EQ(handle.NextEventTimeout(now), 0_ms64);

    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

TEST(TestAddressResolveDefaultImpl, TestTimeoutGetsClearedWhenResultsAreNotFoundAndMaxLookupTimeIsSurpassed)
{
    AddressResolve::NodeLookupHandle handle;

    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    /// now = 0
    auto now     = System::SystemClock().GetMonotonicTimestamp();
    auto request = NodeLookupRequest(chip::PeerId(1, 2));

    request.SetMinLookupTime(100_ms32);
    request.SetMaxLookupTime(200_ms32);

    handle.ResetForLookup(now, request);

    // let's move time passed the end of the range [minlookuptime, maxlookuptime]
    clock.AdvanceMonotonic(250_ms64);

    // now = 250ms
    now = clock.GetMonotonicTimestamp();

    // timeout should be cleared
    EXPECT_EQ(handle.NextEventTimeout(now), 0_ms64);

    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

TEST(TestAddressResolveDefaultImpl, TestKeepsSearchingWhenConsumedTImeIsLessThanMinLookupTime)
{
    AddressResolve::NodeLookupHandle handle;

    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    /// now = 0
    auto now     = System::SystemClock().GetMonotonicTimestamp();
    auto request = NodeLookupRequest(chip::PeerId(1, 2));

    request.SetMinLookupTime(100_ms32);
    request.SetMaxLookupTime(200_ms32);

    handle.ResetForLookup(now, request);

    // let's move time to a point before the min lookup time
    clock.AdvanceMonotonic(50_ms64);

    // now = 50ms
    now = clock.GetMonotonicTimestamp();

    // should keep searching
    EXPECT_EQ(handle.NextAction(now).Type(), chip::AddressResolve::Impl::NodeLookupResult::kKeepSearching);

    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

TEST(TestAddressResolveDefaultImpl, TestReturnsFoundResultAfterMinLookupTimeIsReached)
{
    AddressResolve::NodeLookupHandle handle;

    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    ResolveResult lowResult;
    lowResult.address = GetAddressWithLowScore(static_cast<uint16_t>(1));

    /// now = 0
    auto now     = System::SystemClock().GetMonotonicTimestamp();
    auto request = NodeLookupRequest(chip::PeerId(1, 2));

    request.SetMinLookupTime(100_ms32);
    request.SetMaxLookupTime(200_ms32);

    handle.ResetForLookup(now, request);

    handle.LookupResult(lowResult);

    // let's move time to a point after the min lookup time
    clock.AdvanceMonotonic(150_ms64);

    // now = 150ms
    now = clock.GetMonotonicTimestamp();

    auto action = handle.NextAction(now);
    // should inform success in searching
    EXPECT_EQ(action.Type(), chip::AddressResolve::Impl::NodeLookupResult::kLookupSuccess);
    // and return the result
    EXPECT_EQ(action.ResolveResult().address, lowResult.address);

    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

TEST(TestAddressResolveDefaultImpl, TestGivesUpAfterMaxLookupTimeIsReachedWithoutResults)
{
    AddressResolve::NodeLookupHandle handle;

    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    /// now = 0
    auto now     = System::SystemClock().GetMonotonicTimestamp();
    auto request = NodeLookupRequest(chip::PeerId(1, 2));

    request.SetMinLookupTime(100_ms32);
    request.SetMaxLookupTime(200_ms32);

    handle.ResetForLookup(now, request);

    // let's move time to a point after the max lookup time
    clock.AdvanceMonotonic(250_ms64);

    // now = 250ms
    now = clock.GetMonotonicTimestamp();

    auto action = handle.NextAction(now);
    // should inform error in searching
    EXPECT_EQ(action.Type(), chip::AddressResolve::Impl::NodeLookupResult::kLookupError);
    // and report timeout error
    EXPECT_EQ(action.ErrorResult(), CHIP_ERROR_TIMEOUT);

    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

class MockResolver : public chip::Dnssd::Resolver
{
public:
    CHIP_ERROR Init(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager) override { return InitStatus; }
    bool IsInitialized() override { return true; }
    void Shutdown() override {}
    void SetOperationalDelegate(OperationalResolveDelegate * delegate) override {}
    CHIP_ERROR ResolveNodeId(const PeerId & peerId) override { return ResolveNodeIdStatus; }
    void NodeIdResolutionNoLongerNeeded(const PeerId & peerId) override {}
    CHIP_ERROR StartDiscovery(DiscoveryType type, DiscoveryFilter filter, DiscoveryContext &) override
    {
        if (DiscoveryType::kCommissionerNode == type)
            return DiscoverCommissionersStatus;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR StopDiscovery(DiscoveryContext &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR ReconfirmRecord(const char * hostname, Inet::IPAddress address, Inet::InterfaceId interfaceId) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR InitStatus                  = CHIP_NO_ERROR;
    CHIP_ERROR ResolveNodeIdStatus         = CHIP_NO_ERROR;
    CHIP_ERROR DiscoverCommissionersStatus = CHIP_NO_ERROR;
};

class TestAddressResolveDefaultImplWithSystemLayer : public ::testing::Test
{
public:
    void SetUp() { mSystemLayer.Init(); }

    void TearDown() { mSystemLayer.Shutdown(); }

    class MockSystemLayer : public chip::System::LayerImpl
    {
    public:
        CHIP_ERROR StartTimer(System::Clock::Timeout aDelay, System::TimerCompleteCallback aComplete, void * aAppState) override
        {
            return mStartTimerCallback ? mStartTimerCallback.value()(aDelay, aComplete, aAppState) : CHIP_NO_ERROR;
        }
        CHIP_ERROR ScheduleWork(System::TimerCompleteCallback aComplete, void * aAppState) override
        {
            return mScheduleWorkCallback ? mScheduleWorkCallback.value()(aComplete, aAppState) : CHIP_NO_ERROR;
        }

        std::optional<std::function<CHIP_ERROR(System::Clock::Timeout, System::TimerCompleteCallback, void *)>> mStartTimerCallback;
        std::optional<std::function<CHIP_ERROR(System::TimerCompleteCallback, void *)>> mScheduleWorkCallback;
    };

    MockSystemLayer mSystemLayer;
    MockResolver mockResolver;
};

class TestAddressResolveDefaultImplWithSystemLayerAndNodeListener : public TestAddressResolveDefaultImplWithSystemLayer
{
public:
    void SetUp() override { TestAddressResolveDefaultImplWithSystemLayer::SetUp(); }

    void TearDown() override { TestAddressResolveDefaultImplWithSystemLayer::TearDown(); }
    /// @brief TestNodeListener is an inner class for fixture and also a mock implementation of the NodeListener interface
    /// that allows for testing the address resolution process by providing
    /// customizable callbacks for when a node address is resolved or when resolution fails.
    class TestNodeListener : public chip::AddressResolve::NodeListener
    {
    public:
        using OnNodeAddressResolvedCallback =
            std::function<void(const chip::PeerId &, const chip::AddressResolve::ResolveResult &)>;
        using OnNodeAddressResolutionFailedCallback = std::function<void(const chip::PeerId &, CHIP_ERROR)>;

        /// @brief  Sets the callback to be called when a node address is resolved. Acts as a customization point for tests.
        /// @param callback OnNodeAddressResolvedCallback
        void SetOnNodeAddressResolved(OnNodeAddressResolvedCallback callback) { mOnNodeAddressResolved = std::move(callback); }

        /// @brief Sets the callback to be called when a node address resolution fails. Acts as a customization point for tests.
        /// @param callback OnNodeAddressResolutionFailedCallback
        void SetOnNodeAddressResolutionFailed(OnNodeAddressResolutionFailedCallback callback)
        {
            mOnNodeAddressResolutionFailed = std::move(callback);
        }

        /// @brief  Called when a node address is resolved. Dispatches handling to the injected callback if set.
        void OnNodeAddressResolved(const PeerId & peerId, const ResolveResult & result) override
        {
            if (mOnNodeAddressResolved)
            {
                mOnNodeAddressResolved.value()(peerId, result);
            }
        };

        /// @brief Called when a node address resolution fails. Dispatches handling to the injected callback if set.
        void OnNodeAddressResolutionFailed(const PeerId & peerId, CHIP_ERROR reason) override
        {
            if (mOnNodeAddressResolutionFailed)
            {
                mOnNodeAddressResolutionFailed.value()(peerId, reason);
            }
        };

    private:
        std::optional<OnNodeAddressResolvedCallback> mOnNodeAddressResolved{ std::nullopt };
        std::optional<OnNodeAddressResolutionFailedCallback> mOnNodeAddressResolutionFailed{ std::nullopt };
    };

    TestNodeListener mNodeListener;
    MockResolver mMockResolver;
};

TEST_F(TestAddressResolveDefaultImplWithSystemLayerAndNodeListener, TriesNextResultAndCallsResolvedAddressListener)
{
    chip::AddressResolve::Impl::Resolver resolver;
    auto r = resolver.Init(&mSystemLayer);

    ASSERT_EQ(r, CHIP_NO_ERROR);

    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    AddressResolve::NodeLookupHandle handle;
    auto request = NodeLookupRequest(chip::PeerId(1, 2));

    ResolveResult lowResult;
    lowResult.address = GetAddressWithLowScore(static_cast<uint16_t>(1));

    /// now = 0
    auto now = System::SystemClock().GetMonotonicTimestamp();

    request.SetMinLookupTime(100_ms32);
    request.SetMaxLookupTime(200_ms32);

    handle.ResetForLookup(now, request);

    // fill in single result
    handle.LookupResult(lowResult);

    // move time to a point after the min lookup time
    clock.AdvanceMonotonic(150_ms64);

    chip::PeerId expectedPeerId;
    chip::AddressResolve::ResolveResult expectedResult;

    // set up a success lookup listener
    mNodeListener.SetOnNodeAddressResolved(
        [&expectedPeerId, &expectedResult](const chip::PeerId & peerId, const chip::AddressResolve::ResolveResult & result) {
            expectedPeerId = peerId;
            expectedResult = result;
        });

    handle.SetListener(&mNodeListener);

    r = resolver.TryNextResult(handle);

    ASSERT_EQ(r, CHIP_NO_ERROR);
    ASSERT_EQ(expectedPeerId.GetNodeId(), NodeId{ 2 });
    ASSERT_EQ(expectedResult.address, GetAddressWithLowScore(static_cast<uint16_t>(1)));

    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

TEST_F(TestAddressResolveDefaultImplWithSystemLayerAndNodeListener, CancellingLookupCallsOnNodeAddressResolutionFailed)
{
    chip::Dnssd::Resolver::SetInstance(mockResolver);

    chip::AddressResolve::Impl::Resolver resolver;
    auto r = resolver.Init(&mSystemLayer);

    ASSERT_EQ(r, CHIP_NO_ERROR);

    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    AddressResolve::NodeLookupHandle handle;
    auto request = NodeLookupRequest(chip::PeerId(1, 2));

    ResolveResult lowResult;
    lowResult.address = GetAddressWithLowScore(static_cast<uint16_t>(1));

    /// now = 0
    auto now = System::SystemClock().GetMonotonicTimestamp();

    request.SetMinLookupTime(100_ms32);
    request.SetMaxLookupTime(200_ms32);

    handle.ResetForLookup(now, request);

    for (auto i = 0; i < kNumberOfAvailableSlots; i++)
    {
        // Set up UNIQUE addresses
        lowResult.address = GetAddressWithLowScore(static_cast<uint16_t>(i + 10));
        handle.LookupResult(lowResult);
    }

    // push some NodeLookup handle into Resolver's internal list
    resolver.LookupNode(request, handle);

    // set up a failure listener
    CHIP_ERROR expectedError = CHIP_NO_ERROR;
    mNodeListener.SetOnNodeAddressResolutionFailed(
        [&expectedError](const chip::PeerId & peerId, CHIP_ERROR reason) { expectedError = reason; });

    handle.SetListener(&mNodeListener);

    // cancel the lookup
    r = resolver.CancelLookup(handle, Resolver::FailureCallback::Call);

    EXPECT_EQ(r, CHIP_NO_ERROR);
    EXPECT_EQ(expectedError, CHIP_ERROR_CANCELLED);

    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

TEST_F(TestAddressResolveDefaultImplWithSystemLayerAndNodeListener, LooksUpFailsAndCallsFailureListenerWhenSystemStartTimerFails)
{
    chip::Dnssd::Resolver::SetInstance(mockResolver);

    chip::AddressResolve::Impl::Resolver resolver;
    auto r = resolver.Init(&mSystemLayer);

    ASSERT_EQ(r, CHIP_NO_ERROR);

    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    AddressResolve::NodeLookupHandle handle;
    auto request = NodeLookupRequest(chip::PeerId(1, 2));

    CHIP_ERROR expectedError = CHIP_NO_ERROR;
    mNodeListener.SetOnNodeAddressResolutionFailed(
        [&expectedError](const chip::PeerId & peerId, CHIP_ERROR reason) { expectedError = reason; });

    handle.SetListener(&mNodeListener);

    CHIP_ERROR expectedTimerError    = CHIP_NO_ERROR;
    mSystemLayer.mStartTimerCallback = [&expectedTimerError](auto, auto, auto *) {
        // Simulate failure in starting the timer
        expectedTimerError = CHIP_ERROR_CANCELLED;
        return expectedTimerError;
    };

    r = resolver.LookupNode(request, handle);

    EXPECT_EQ(r, CHIP_NO_ERROR);
    EXPECT_EQ(expectedError, CHIP_ERROR_CANCELLED);
    EXPECT_EQ(expectedTimerError, CHIP_ERROR_CANCELLED);

    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

TEST_F(TestAddressResolveDefaultImplWithSystemLayerAndNodeListener,
       CallsSuccessAndFailureListenersWhenAppropriateLookupResultsAreFound)
{
    chip::Dnssd::Resolver::SetInstance(mockResolver);

    chip::AddressResolve::Impl::Resolver resolver;
    auto r = resolver.Init(&mSystemLayer);

    ASSERT_EQ(r, CHIP_NO_ERROR);

    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    AddressResolve::NodeLookupHandle successfulHandle;
    auto successfulRequest = NodeLookupRequest(chip::PeerId(1, 2));

    successfulRequest.SetMinLookupTime(100_ms32);
    successfulRequest.SetMaxLookupTime(200_ms32);

    AddressResolve::NodeLookupHandle failedHandle;
    auto failedRequest = NodeLookupRequest(chip::PeerId(1, 3));

    failedRequest.SetMinLookupTime(50_ms32);
    failedRequest.SetMaxLookupTime(100_ms32);

    r = resolver.LookupNode(successfulRequest, successfulHandle);
    EXPECT_EQ(r, CHIP_NO_ERROR);

    r = resolver.LookupNode(failedRequest, failedHandle);
    EXPECT_EQ(r, CHIP_NO_ERROR);

    clock.AdvanceMonotonic(150_ms64);

    chip::PeerId expectedPeerId;
    chip::AddressResolve::ResolveResult expectedResult;
    // set up a success lookup listener
    mNodeListener.SetOnNodeAddressResolved(
        [&expectedPeerId, &expectedResult](const chip::PeerId & peerId, const chip::AddressResolve::ResolveResult & result) {
            expectedPeerId = peerId;
            expectedResult = result;
        });

    CHIP_ERROR expectedError = CHIP_NO_ERROR;
    chip::PeerId expectedFailedPeerId;
    // set up a failed lookup listener
    mNodeListener.SetOnNodeAddressResolutionFailed(
        [&expectedFailedPeerId, &expectedError](const chip::PeerId & peerId, CHIP_ERROR reason) {
            expectedFailedPeerId = peerId;
            expectedError        = reason;
        });

    successfulHandle.SetListener(&mNodeListener);
    failedHandle.SetListener(&mNodeListener);

    ResolveResult lowResult;
    lowResult.address = GetAddressWithLowScore();

    Dnssd::ResolvedNodeData resolvedData;
    resolvedData.resolutionData.numIPs       = 1;
    resolvedData.resolutionData.ipAddress[0] = lowResult.address.GetIPAddress();
    resolvedData.resolutionData.interfaceId  = lowResult.address.GetInterface();
    resolvedData.resolutionData.port         = lowResult.address.GetPort();
    resolvedData.operationalData.peerId      = successfulRequest.GetPeerId();

    resolver.OnOperationalNodeResolved(resolvedData);                                          // successful resolution
    resolver.OnOperationalNodeResolutionFailed(failedRequest.GetPeerId(), CHIP_ERROR_TIMEOUT); // failed resolution

    EXPECT_EQ(expectedPeerId.GetNodeId(), NodeId{ 2 });
    EXPECT_EQ(expectedResult.address, GetAddressWithLowScore());
    EXPECT_EQ(expectedFailedPeerId.GetNodeId(), NodeId{ 3 });
    EXPECT_EQ(expectedError, CHIP_ERROR_TIMEOUT);

    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

TEST_F(TestAddressResolveDefaultImplWithSystemLayerAndNodeListener, ResolverShutsDownAndClearsAllPendingLookups)
{
    chip::AddressResolve::Impl::Resolver resolver;
    auto r = resolver.Init(&mSystemLayer);

    ASSERT_EQ(r, CHIP_NO_ERROR);

    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    AddressResolve::NodeLookupHandle handle;
    auto request = NodeLookupRequest(chip::PeerId(1, 2));

    request.SetMinLookupTime(100_ms32);
    request.SetMaxLookupTime(200_ms32);

    r = resolver.LookupNode(request, handle);
    EXPECT_EQ(r, CHIP_NO_ERROR);

    CHIP_ERROR expectedError = CHIP_NO_ERROR;
    mNodeListener.SetOnNodeAddressResolutionFailed(
        [&expectedError](const chip::PeerId & peerId, CHIP_ERROR reason) { expectedError = reason; });

    handle.SetListener(&mNodeListener);

    // Shutdown the resolver
    resolver.Shutdown();

    EXPECT_EQ(expectedError, CHIP_ERROR_SHUT_DOWN);

    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

} // namespace
