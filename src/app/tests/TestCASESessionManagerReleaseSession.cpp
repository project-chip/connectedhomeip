/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

// Tests that CASESessionManager::ReleaseSession(peerId) does not tear down an
// OperationalSessionSetup that is mid-handshake, while still releasing setups
// that are fully connected or uninitialized.

#include <pw_unit_test/framework.h>

#include <app/CASEClientPool.h>
#include <app/CASESessionManager.h>
#include <app/OperationalSessionSetup.h>
#include <app/OperationalSessionSetupPool.h>
#include <app/tests/AppTestContext.h>
#include <app/tests/CASESessionManagerTestAccess.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;

namespace {

constexpr NodeId kTestNodeId       = 0x123456789abcdefULL;
constexpr FabricIndex kFabricIndex = 1;

using OSSAccess = chip::TestOperationalSessionSetupAccess;
using MgrAccess = chip::TestCASESessionManagerAccess;

// Capacity-2 pools are plenty for these single-peer tests.
using TestSetupPool  = OperationalSessionSetupPool<2>;
using TestClientPool = CASEClientPool<2>;

class TestCASESessionManagerReleaseSession : public chip::Testing::AppContext
{
protected:
    // Allocate a session setup in the pool for kTestNodeId and force it into the
    // given internal state. We force mPeerId directly so the setup is locatable
    // by ReleaseSession(peerId) without standing up a full, validated CASE
    // stack (the internal state is the only thing under test here).
    template <size_t N, size_t M>
    OperationalSessionSetup * AllocateSetup(OperationalSessionSetupPool<N> & pool, CASEClientPool<M> & clientPool,
                                            OperationalSessionReleaseDelegate & releaseDelegate, const ScopedNodeId & peerId)
    {
        // Provide a valid exchange/session manager so the setup's destructor
        // (which cancels timers via exchangeMgr->GetSessionManager()) is safe.
        // The setup's reachable state is forced below, so full validation of
        // the params is not required for this test.
        CASEClientInitParams params;
        params.sessionManager = &GetSecureSessionManager();
        params.exchangeMgr    = &GetExchangeManager();
        auto * setup          = pool.Allocate(params, &clientPool, peerId, &releaseDelegate);
        if (setup != nullptr)
        {
            OSSAccess::SetPeerId(*setup, peerId);
        }
        return setup;
    }
};

// A setup that is actively establishing a session must NOT be released by
// ReleaseSession(peerId) — destroying it mid-handshake would reset the retry
// state (attempt counter / busy backoff) and churn fresh Sigma1 attempts.
//
// Fail-without-fix: without the IsEstablishingSession() gate in
// CASESessionManager::ReleaseSession, the in-flight setup is destroyed and
// FindSessionSetup returns nullptr.
TEST_F(TestCASESessionManagerReleaseSession, EstablishingSetupIsNotReleased)
{
    TestSetupPool pool;
    TestClientPool clientPool;
    CASESessionManager manager;
    MgrAccess::SetSessionSetupPool(manager, &pool);

    ScopedNodeId peerId(kTestNodeId, kFabricIndex);
    auto * setup = AllocateSetup(pool, clientPool, manager, peerId);
    ASSERT_NE(setup, nullptr);
    OSSAccess::SetEstablishing(*setup);
    ASSERT_TRUE(OSSAccess::IsEstablishing(*setup));
    // Sanity: the setup is locatable by peer id before release.
    ASSERT_NE(pool.FindSessionSetup(peerId, false), nullptr);

    manager.ReleaseSession(peerId);

    // The in-flight setup must survive.
    EXPECT_NE(pool.FindSessionSetup(peerId, false), nullptr);

    pool.ReleaseAllSessionSetupsForFabric(kFabricIndex);
}

// A fully-connected setup is not "establishing" and must be released.
TEST_F(TestCASESessionManagerReleaseSession, SecureConnectedSetupIsReleased)
{
    TestSetupPool pool;
    TestClientPool clientPool;
    CASESessionManager manager;
    MgrAccess::SetSessionSetupPool(manager, &pool);

    ScopedNodeId peerId(kTestNodeId, kFabricIndex);
    auto * setup = AllocateSetup(pool, clientPool, manager, peerId);
    ASSERT_NE(setup, nullptr);
    OSSAccess::SetSecureConnected(*setup);
    ASSERT_FALSE(OSSAccess::IsEstablishing(*setup));
    ASSERT_NE(pool.FindSessionSetup(peerId, false), nullptr);

    manager.ReleaseSession(peerId);

    // The connected setup must have been released.
    EXPECT_EQ(pool.FindSessionSetup(peerId, false), nullptr);
}

// An uninitialized setup is not "establishing" and must be released.
TEST_F(TestCASESessionManagerReleaseSession, UninitializedSetupIsReleased)
{
    TestSetupPool pool;
    TestClientPool clientPool;
    CASESessionManager manager;
    MgrAccess::SetSessionSetupPool(manager, &pool);

    ScopedNodeId peerId(kTestNodeId, kFabricIndex);
    auto * setup = AllocateSetup(pool, clientPool, manager, peerId);
    ASSERT_NE(setup, nullptr);
    OSSAccess::SetUninitialized(*setup);
    ASSERT_FALSE(OSSAccess::IsEstablishing(*setup));
    ASSERT_NE(pool.FindSessionSetup(peerId, false), nullptr);

    manager.ReleaseSession(peerId);

    EXPECT_EQ(pool.FindSessionSetup(peerId, false), nullptr);
}

// Parameterised: each of the 5 establishing states must survive ReleaseSession.
TEST_F(TestCASESessionManagerReleaseSession, AllEstablishingStatesAreNotReleased)
{
    using ForceFn          = void (*)(OperationalSessionSetup &);
    const ForceFn states[] = {
        [](OperationalSessionSetup & s) { OSSAccess::SetNeedsAddress(s); },
        [](OperationalSessionSetup & s) { OSSAccess::SetResolvingAddress(s); },
        [](OperationalSessionSetup & s) { OSSAccess::SetHasAddress(s); },
        [](OperationalSessionSetup & s) { OSSAccess::SetEstablishing(s); }, // Connecting
        [](OperationalSessionSetup & s) { OSSAccess::SetWaitingForRetry(s); },
    };

    for (auto forceState : states)
    {
        OperationalSessionSetupPool<4> pool;
        CASEClientPool<4> clientPool;
        CASESessionManager manager;
        MgrAccess::SetSessionSetupPool(manager, &pool);

        ScopedNodeId peerId(kTestNodeId, kFabricIndex);
        CASEClientInitParams params;
        params.sessionManager = &GetSecureSessionManager();
        params.exchangeMgr    = &GetExchangeManager();
        auto * setup          = pool.Allocate(params, &clientPool, peerId, &manager);
        ASSERT_NE(setup, nullptr);
        OSSAccess::SetPeerId(*setup, peerId);

        forceState(*setup);
        ASSERT_TRUE(OSSAccess::IsEstablishing(*setup));

        OperationalSessionSetup * before = setup;
        manager.ReleaseSession(peerId);

        OperationalSessionSetup * after = pool.FindSessionSetup(peerId, false);
        ASSERT_NE(after, nullptr);
        EXPECT_EQ(after, before);

        pool.ReleaseAllSessionSetupsForFabric(kFabricIndex);
    }
}

// Repeated ReleaseSession during establishment is idempotent (no double-release
// or state corruption from N back-to-back skipped-release calls).
TEST_F(TestCASESessionManagerReleaseSession, RepeatedReleaseDuringEstablishmentIsIdempotent)
{
    OperationalSessionSetupPool<4> pool;
    CASEClientPool<4> clientPool;
    CASESessionManager manager;
    MgrAccess::SetSessionSetupPool(manager, &pool);

    ScopedNodeId peerId(kTestNodeId, kFabricIndex);
    CASEClientInitParams params;
    params.sessionManager = &GetSecureSessionManager();
    params.exchangeMgr    = &GetExchangeManager();
    auto * setup          = pool.Allocate(params, &clientPool, peerId, &manager);
    ASSERT_NE(setup, nullptr);
    OSSAccess::SetPeerId(*setup, peerId);
    OSSAccess::SetWaitingForRetry(*setup);

    OperationalSessionSetup * before = setup;
    for (int i = 0; i < 5; ++i)
    {
        manager.ReleaseSession(peerId);
    }
    OperationalSessionSetup * after = pool.FindSessionSetup(peerId, false);
    ASSERT_NE(after, nullptr);
    EXPECT_EQ(after, before);

    pool.ReleaseAllSessionSetupsForFabric(kFabricIndex);
}

// Establishing -> ReleaseSession (no-op) -> SecureConnected -> ReleaseSession
// (releases). Verifies the guard applies only during establishment and is correctly
// lifted once the handshake completes.
TEST_F(TestCASESessionManagerReleaseSession, ReleaseAfterEstablishmentCompletes)
{
    OperationalSessionSetupPool<4> pool;
    CASEClientPool<4> clientPool;
    CASESessionManager manager;
    MgrAccess::SetSessionSetupPool(manager, &pool);

    ScopedNodeId peerId(kTestNodeId, kFabricIndex);
    CASEClientInitParams params;
    params.sessionManager = &GetSecureSessionManager();
    params.exchangeMgr    = &GetExchangeManager();
    auto * setup          = pool.Allocate(params, &clientPool, peerId, &manager);
    ASSERT_NE(setup, nullptr);
    OSSAccess::SetPeerId(*setup, peerId);
    OSSAccess::SetEstablishing(*setup);

    // First call: guarded, setup survives.
    manager.ReleaseSession(peerId);
    EXPECT_NE(pool.FindSessionSetup(peerId, false), nullptr);

    // Transition to connected.
    OSSAccess::SetSecureConnected(*setup);

    // Second call: not establishing, setup is released.
    manager.ReleaseSession(peerId);
    EXPECT_EQ(pool.FindSessionSetup(peerId, false), nullptr);
}

// Integration-style: a CASE callback handler that calls
// manager.ReleaseSession(peerId) while the setup is mid-handshake must NOT
// destroy the in-flight setup. This exercises the realistic re-entry pattern
// where higher layers, on a callback (e.g. OnDeviceConnectionFailure dispatched
// by an enclosing component while another attempt is still establishing for the
// same peer), defensively call ReleaseSession on the manager.
//
// We model "called from a CASE callback" by issuing the ReleaseSession call
// from within a function-call frame that captures the manager and peerId,
// mimicking what an OnDeviceConnectionFailure / app-level handler would do.
TEST_F(TestCASESessionManagerReleaseSession, ReleaseFromCallbackDuringEstablishmentIsNoOp)
{
    OperationalSessionSetupPool<4> pool;
    CASEClientPool<4> clientPool;
    CASESessionManager manager;
    MgrAccess::SetSessionSetupPool(manager, &pool);

    ScopedNodeId peerId(kTestNodeId, kFabricIndex);
    auto * setup = AllocateSetup(pool, clientPool, manager, peerId);
    ASSERT_NE(setup, nullptr);
    OSSAccess::SetEstablishing(*setup);
    ASSERT_TRUE(OSSAccess::IsEstablishing(*setup));

    OperationalSessionSetup * before = setup;

    // Simulate a CASE callback handler invoking ReleaseSession on the manager.
    // The lambda body is exactly what a user-supplied OnDeviceConnectionFailure
    // (or similar) handler would do: ask the manager to drop the session.
    auto simulatedCaseCallback = [&]() { manager.ReleaseSession(peerId); };
    simulatedCaseCallback();

    // Setup must still be present and unchanged.
    OperationalSessionSetup * after = pool.FindSessionSetup(peerId, false);
    ASSERT_NE(after, nullptr);
    EXPECT_EQ(after, before);
    EXPECT_TRUE(OSSAccess::IsEstablishing(*after));

    // A second callback-style release while still establishing is also a no-op.
    simulatedCaseCallback();
    after = pool.FindSessionSetup(peerId, false);
    ASSERT_NE(after, nullptr);
    EXPECT_EQ(after, before);

    pool.ReleaseAllSessionSetupsForFabric(kFabricIndex);
}

// Asymmetry test: ReleaseSessionsForFabric (used on fabric removal) WILL tear
// down establishing setups, in contrast to ReleaseSession(peerId) which guards
// them. This is the documented contract in CASESessionManager::ReleaseSessionsForFabric --
// fabric removal is irreversible, so dangling references to a removed fabric are
// strictly worse than restarting an in-flight handshake.
//
// Fail-without-fix: if a future change ever extends the in-flight guard into
// the fabric-removal path, this test catches it.
TEST_F(TestCASESessionManagerReleaseSession, ReleaseSessionsForFabricTearsDownEstablishingSetups)
{
    OperationalSessionSetupPool<4> pool;
    CASEClientPool<4> clientPool;
    CASESessionManager manager;
    MgrAccess::SetSessionSetupPool(manager, &pool);

    // Two setups on the same fabric, both mid-handshake, plus a third on a
    // different fabric that must NOT be torn down.
    ScopedNodeId peerOnFabric1A(kTestNodeId, kFabricIndex);
    ScopedNodeId peerOnFabric1B(kTestNodeId + 1, kFabricIndex);
    ScopedNodeId peerOnFabric2(kTestNodeId + 2, static_cast<FabricIndex>(kFabricIndex + 1));

    auto * s1 = AllocateSetup(pool, clientPool, manager, peerOnFabric1A);
    auto * s2 = AllocateSetup(pool, clientPool, manager, peerOnFabric1B);
    auto * s3 = AllocateSetup(pool, clientPool, manager, peerOnFabric2);
    ASSERT_NE(s1, nullptr);
    ASSERT_NE(s2, nullptr);
    ASSERT_NE(s3, nullptr);

    OSSAccess::SetEstablishing(*s1);    // Connecting
    OSSAccess::SetWaitingForRetry(*s2); // also "establishing"
    OSSAccess::SetEstablishing(*s3);    // different fabric, should survive

    ASSERT_TRUE(OSSAccess::IsEstablishing(*s1));
    ASSERT_TRUE(OSSAccess::IsEstablishing(*s2));
    ASSERT_TRUE(OSSAccess::IsEstablishing(*s3));

    // Per-peer ReleaseSession is guarded -- sanity check the asymmetry first:
    // both establishing peers on fabric 1 survive a per-peer release call.
    manager.ReleaseSession(peerOnFabric1A);
    manager.ReleaseSession(peerOnFabric1B);
    EXPECT_NE(pool.FindSessionSetup(peerOnFabric1A, false), nullptr);
    EXPECT_NE(pool.FindSessionSetup(peerOnFabric1B, false), nullptr);

    // Now exercise the fabric-removal path: it must tear down both establishing
    // setups on fabric 1, while leaving fabric 2's establishing setup alone.
    manager.ReleaseSessionsForFabric(kFabricIndex);

    EXPECT_EQ(pool.FindSessionSetup(peerOnFabric1A, false), nullptr);
    EXPECT_EQ(pool.FindSessionSetup(peerOnFabric1B, false), nullptr);
    EXPECT_NE(pool.FindSessionSetup(peerOnFabric2, false), nullptr);

    pool.ReleaseAllSessionSetupsForFabric(static_cast<FabricIndex>(kFabricIndex + 1));
}

// ReleaseAllSessions (shutdown path) MUST tear down establishing setups -- the
// per-peer guard is intentionally bypassed at shutdown to avoid leaks.
TEST_F(TestCASESessionManagerReleaseSession, ShutdownReleasesEstablishingSetups)
{
    OperationalSessionSetupPool<4> pool;
    CASEClientPool<4> clientPool;
    CASESessionManager manager;
    MgrAccess::SetSessionSetupPool(manager, &pool);

    ScopedNodeId peerId1(kTestNodeId, kFabricIndex);
    ScopedNodeId peerId2(kTestNodeId + 1, kFabricIndex);

    CASEClientInitParams params;
    params.sessionManager = &GetSecureSessionManager();
    params.exchangeMgr    = &GetExchangeManager();

    auto * s1 = pool.Allocate(params, &clientPool, peerId1, &manager);
    auto * s2 = pool.Allocate(params, &clientPool, peerId2, &manager);
    ASSERT_NE(s1, nullptr);
    ASSERT_NE(s2, nullptr);
    OSSAccess::SetPeerId(*s1, peerId1);
    OSSAccess::SetPeerId(*s2, peerId2);

    OSSAccess::SetEstablishing(*s1);
    OSSAccess::SetWaitingForRetry(*s2);

    ASSERT_TRUE(OSSAccess::IsEstablishing(*s1));
    ASSERT_TRUE(OSSAccess::IsEstablishing(*s2));

    manager.ReleaseAllSessions();

    EXPECT_EQ(pool.FindSessionSetup(peerId1, false), nullptr);
    EXPECT_EQ(pool.FindSessionSetup(peerId2, false), nullptr);
}

// ReleaseSession(peerId) for a peer that has no setup in the pool must be a
// safe no-op (the FindSessionSetup lookup returns nullptr and we must not
// crash, assert, or affect any other entries in the pool).
TEST_F(TestCASESessionManagerReleaseSession, ReleaseForNonExistentPeerIsSafeNoOp)
{
    OperationalSessionSetupPool<4> pool;
    CASEClientPool<4> clientPool;
    CASESessionManager manager;
    MgrAccess::SetSessionSetupPool(manager, &pool);

    // Allocate an unrelated establishing setup that must survive the release
    // of a different, non-existent peer.
    ScopedNodeId existingPeer(kTestNodeId, kFabricIndex);
    auto * setup = AllocateSetup(pool, clientPool, manager, existingPeer);
    ASSERT_NE(setup, nullptr);
    OSSAccess::SetEstablishing(*setup);

    // Sanity-check the peer is not present.
    ScopedNodeId missingPeer(kTestNodeId + 999, kFabricIndex);
    ASSERT_EQ(pool.FindSessionSetup(missingPeer, false), nullptr);

    // Must not crash; must not disturb the unrelated establishing setup.
    manager.ReleaseSession(missingPeer);

    EXPECT_NE(pool.FindSessionSetup(existingPeer, false), nullptr);
    EXPECT_TRUE(OSSAccess::IsEstablishing(*setup));

    pool.ReleaseAllSessionSetupsForFabric(kFabricIndex);
}

// ReleaseSession(peerId) for one fabric must NOT release a setup for the
// same NodeId on a *different* fabric. ScopedNodeId equality is (fabric,
// node); a regression that collapses to a node-only lookup would break the
// fabric-scoping contract and break multi-fabric controllers.
TEST_F(TestCASESessionManagerReleaseSession, ReleaseIsScopedByFabric)
{
    OperationalSessionSetupPool<4> pool;
    CASEClientPool<4> clientPool;
    CASESessionManager manager;
    MgrAccess::SetSessionSetupPool(manager, &pool);

    ScopedNodeId peerOnFabric1(kTestNodeId, kFabricIndex);
    ScopedNodeId peerOnFabric2(kTestNodeId, static_cast<FabricIndex>(kFabricIndex + 1));

    auto * s1 = AllocateSetup(pool, clientPool, manager, peerOnFabric1);
    auto * s2 = AllocateSetup(pool, clientPool, manager, peerOnFabric2);
    ASSERT_NE(s1, nullptr);
    ASSERT_NE(s2, nullptr);

    // s1 is connected (and so would normally be released); s2 is mid-handshake
    // on a different fabric. After releasing fabric-1's peer, fabric-2's
    // setup must be untouched.
    OSSAccess::SetSecureConnected(*s1);
    OSSAccess::SetEstablishing(*s2);
    OperationalSessionSetup * s2Before = s2;

    manager.ReleaseSession(peerOnFabric1);

    EXPECT_EQ(pool.FindSessionSetup(peerOnFabric1, false), nullptr);
    OperationalSessionSetup * s2After = pool.FindSessionSetup(peerOnFabric2, false);
    ASSERT_NE(s2After, nullptr);
    EXPECT_EQ(s2After, s2Before);
    EXPECT_TRUE(OSSAccess::IsEstablishing(*s2After));

    pool.ReleaseAllSessionSetupsForFabric(static_cast<FabricIndex>(kFabricIndex + 1));
}

// Skipping the release of an establishing setup must leave the setup's
// internal state (mState, peer id) untouched. A regression that mutated
// state on the "skip" path (e.g. resetting attempt counters) would defeat
// the entire purpose of the guard.
TEST_F(TestCASESessionManagerReleaseSession, EstablishingSetupStateUnchangedAfterSkippedRelease)
{
    OperationalSessionSetupPool<4> pool;
    CASEClientPool<4> clientPool;
    CASESessionManager manager;
    MgrAccess::SetSessionSetupPool(manager, &pool);

    ScopedNodeId peerId(kTestNodeId, kFabricIndex);
    auto * setup = AllocateSetup(pool, clientPool, manager, peerId);
    ASSERT_NE(setup, nullptr);

    // Use WaitingForRetry: this is the state that holds the most retry/backoff
    // bookkeeping (mAttemptsDone, mRequestedBusyDelay) in production; any
    // regression that mutated state on the skip path would clobber it here.
    OSSAccess::SetWaitingForRetry(*setup);
    ASSERT_TRUE(OSSAccess::IsEstablishing(*setup));

    manager.ReleaseSession(peerId);

    OperationalSessionSetup * after = pool.FindSessionSetup(peerId, false);
    ASSERT_NE(after, nullptr);
    EXPECT_EQ(after, setup);
    // Still establishing, still in the same state (lookup-by-peer-id still works).
    EXPECT_TRUE(OSSAccess::IsEstablishing(*after));

    pool.ReleaseAllSessionSetupsForFabric(kFabricIndex);
}

} // namespace
