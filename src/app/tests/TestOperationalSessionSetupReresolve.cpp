/*
 *
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

// Regression coverage for stale-operational-address re-resolve on CASE timeout:
//
// A controller commissioning a Thread Sleepy End Device locks CASE onto the
// device's STALE pre-operational address and, on CASE Sigma1 timeout, kept
// retransmitting to that dead address (walking only the cached resolve
// snapshot) instead of re-resolving. The device's real operational SRP record
// (a NEW address) was published ~12s later but never picked up, so commissioning
// hung until the watchdog fired.
//
// The fix: OperationalSessionSetup::OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT)
// now starts a FRESH DNS-SD lookup (LookupPeerAddress -> Resolver::LookupNode ->
// Dnssd::Resolver::ResolveNodeId) before exhausting the stale cached snapshot,
// so a newer operational advertisement can be obtained.
//
// This test drives OnSessionEstablishmentError(TIMEOUT) directly on a setup that
// has a stale cached result and asserts that a fresh DNS-SD query is issued.
//
// Fail-without-fix: the pre-fix code calls Resolver::TryNextResult() first, which
// re-delivers the stale cached address synchronously (no fresh ResolveNodeId),
// so mResolveNodeIdCount stays 0 and the assertion fails.

#include <pw_unit_test/framework.h>

#include <app/CASEClientPool.h>
#include <app/OperationalSessionSetup.h>
#include <app/OperationalSessionSetupPool.h>
#include <app/tests/AppTestContext.h>
#include <app/tests/OperationalSessionSetupTestAccess.h>
#include <lib/address_resolve/AddressResolve.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/CodeUtils.h>
#include <transport/raw/PeerAddress.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Inet;
using namespace chip::Transport;

namespace {

using OSSAccess = chip::OperationalSessionSetupTestAccess;

using TestSetupPool  = OperationalSessionSetupPool<2>;
using TestClientPool = CASEClientPool<2>;

constexpr NodeId kTestNodeId = 0x123456789abcdefULL;

// A no-op release delegate. The re-resolve success path of
// OnSessionEstablishmentError() never releases the setup (it starts a fresh
// lookup and returns), so this is only the delegate the pool requires to
// allocate a setup; the test releases the pool directly at teardown.
class NoopReleaseDelegate : public OperationalSessionReleaseDelegate
{
public:
    void ReleaseSession(OperationalSessionSetup *) override {}
};

// A Dnssd::Resolver mock that counts ResolveNodeId calls. A fresh operational
// DNS-SD lookup is what surfaces a newer SRP/operational advertisement, so a
// bump in this counter is the observable signal that we re-resolved rather than
// reusing the stale cached snapshot.
class CountingDnssdResolver : public chip::Dnssd::Resolver
{
public:
    CHIP_ERROR Init(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> *) override { return CHIP_NO_ERROR; }
    bool IsInitialized() override { return true; }
    void Shutdown() override {}
    void SetOperationalDelegate(chip::Dnssd::OperationalResolveDelegate * delegate) override { mOperationalDelegate = delegate; }
    CHIP_ERROR ResolveNodeId(const PeerId & peerId) override
    {
        mResolveNodeIdCount++;
        mLastResolvedPeer = peerId;
        return CHIP_NO_ERROR;
    }
    void NodeIdResolutionNoLongerNeeded(const PeerId &) override {}
    CHIP_ERROR StartDiscovery(chip::Dnssd::DiscoveryType, chip::Dnssd::DiscoveryFilter, chip::Dnssd::DiscoveryContext &) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR StopDiscovery(chip::Dnssd::DiscoveryContext &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR ReconfirmRecord(const char *, Inet::IPAddress, Inet::InterfaceId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    int mResolveNodeIdCount                                        = 0;
    PeerId mLastResolvedPeer                                       = PeerId();
    chip::Dnssd::OperationalResolveDelegate * mOperationalDelegate = nullptr;
};

class TestOperationalSessionSetupReresolve : public chip::Testing::AppContext
{
public:
    void SetUp() override
    {
        AppContext::SetUp();
        // A fabric is required for LookupPeerAddress() (it resolves the
        // compressed fabric id). Alice's fabric is provided by the harness.
        ASSERT_EQ(CreateAliceFabric(), CHIP_NO_ERROR);

        // Route the address resolver's underlying DNS-SD queries to our counting
        // mock and (re)initialize the global address resolver against the test
        // system layer so LookupNode is functional.
        mPreviousDnssdResolver = &chip::Dnssd::Resolver::Instance();
        chip::Dnssd::Resolver::SetInstance(mDnssdResolver);
        ASSERT_EQ(AddressResolve::Resolver::Instance().Init(&GetSystemLayer()), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        AddressResolve::Resolver::Instance().Shutdown();
        if (mPreviousDnssdResolver != nullptr)
        {
            chip::Dnssd::Resolver::SetInstance(*mPreviousDnssdResolver);
        }
        AppContext::TearDown();
    }

protected:
    CountingDnssdResolver mDnssdResolver;
    chip::Dnssd::Resolver * mPreviousDnssdResolver = nullptr;

    AddressResolve::ResolveResult StaleResult()
    {
        AddressResolve::ResolveResult result;
        IPAddress addr;
        // The stale, pre-operational address (matches the scenario's
        // fd72:...:7ecd:... record that CASE locked onto).
        EXPECT_TRUE(IPAddress::FromString("fd72:71b4:b243:1:7ecd:2485:9aeb:f3b0", addr));
        result.address         = PeerAddress::UDP(addr, CHIP_PORT);
        result.mrpRemoteConfig = GetDefaultMRPConfig();
        return result;
    }
};

// On CASE timeout, with resolve attempts remaining and a stale cached result,
// OnSessionEstablishmentError must trigger a FRESH DNS-SD lookup so a newer
// operational advertisement can be obtained -- instead of only re-trying the
// stale cached address.
TEST_F(TestOperationalSessionSetupReresolve, TimeoutWithResolveAttemptsTriggersFreshLookup)
{
    TestSetupPool pool;
    TestClientPool clientPool;
    NoopReleaseDelegate releaseDelegate;

    ScopedNodeId peerId(kTestNodeId, GetAliceFabricIndex());

    CASEClientInitParams params;
    params.sessionManager = &GetSecureSessionManager();
    params.exchangeMgr    = &GetExchangeManager();
    params.fabricTable    = &GetFabricTable();

    auto * setup = pool.Allocate(params, &clientPool, peerId, &releaseDelegate);
    ASSERT_NE(setup, nullptr);
    OSSAccess::SetPeerId(*setup, peerId);

    // Arrange: a stale cached resolve result (as if CASE had locked onto the
    // pre-operational address), at least one fresh resolve attempt allowed, and
    // the setup mid-handshake (Connecting) with an allocated CASE client.
    OSSAccess::SeedCachedLookupResult(*setup, StaleResult());
    ASSERT_TRUE(OSSAccess::HasCachedLookupResult(*setup));
    OSSAccess::SetResolveAttemptsAllowed(*setup, 2);
    ASSERT_EQ(OSSAccess::EnterConnectingWithCaseClient(*setup, params, clientPool, releaseDelegate), CHIP_NO_ERROR);

    const int resolvesBefore = mDnssdResolver.mResolveNodeIdCount;

    // Act: CASE Sigma1 timed out.
    OSSAccess::InvokeOnSessionEstablishmentError(*setup, CHIP_ERROR_TIMEOUT);

    // Assert: a fresh DNS-SD query was issued (re-resolve), rather than only
    // re-walking the stale cached snapshot. This is the crux of the fix.
    EXPECT_GT(mDnssdResolver.mResolveNodeIdCount, resolvesBefore);
    EXPECT_EQ(mDnssdResolver.mLastResolvedPeer.GetNodeId(), kTestNodeId);

    // The fresh lookup is now in flight on the global resolver; cancel it before
    // the setup and resolver are torn down so no callback fires into freed state.
    if (OSSAccess::GetAddressLookupHandle(*setup).IsActive())
    {
        LogErrorOnFailure(AddressResolve::Resolver::Instance().CancelLookup(OSSAccess::GetAddressLookupHandle(*setup),
                                                                            AddressResolve::Resolver::FailureCallback::Skip));
    }

    pool.ReleaseAllSessionSetupsForFabric(GetAliceFabricIndex());
}

} // namespace
