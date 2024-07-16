/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

/**
 *    @file
 *      This file implements unit tests for the SessionManager implementation.
 */

#include <errno.h>
#include <vector>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemClock.h>
#include <transport/SecureSessionTable.h>
#include <transport/SessionHolder.h>

namespace chip {
namespace Transport {

class TestSecureSessionTable : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void ValidateSessionSorting();

private:
    struct SessionParameters
    {
        ScopedNodeId mPeer;
        System::Clock::Timestamp mLastActivityTime;
        SecureSession::State mState;
    };

    //
    // This listener lets us track which sessions get evicted by
    // using a SessionHolderWithDelegate to get notified on session release.
    //
    class SessionNotificationListener : public SessionDelegate
    {
    public:
        SessionNotificationListener(const SessionHandle & session) : mSessionHolder(*this) { mSessionHolder.Grab(session); }

        void OnSessionReleased() { mSessionReleased = true; }

        NewSessionHandlingPolicy GetNewSessionHandlingPolicy() { return NewSessionHandlingPolicy::kStayAtOldSession; }

        SessionHolderWithDelegate mSessionHolder;
        bool mSessionReleased = false;
    };

    static constexpr FabricIndex kFabric1 = 1;
    static constexpr FabricIndex kFabric2 = 2;
    static constexpr FabricIndex kFabric3 = 3;

    //
    // Allocates a new secure session given an eviction hint. The session that was evicted is compared against the provided
    // evictedSessionIndex (which indexes into the provided SessionParameter table) to validate that it matches.
    //
    void AllocateSession(const ScopedNodeId & sessionEvictionHint, std::vector<SessionParameters> & sessionParameters,
                         uint16_t evictedSessionIndex);

    //
    // Reset our internal SecureSessionTable list and create a new one given the provided parameters.
    //
    void CreateSessionTable(std::vector<SessionParameters> & sessionParams);

    Platform::UniquePtr<SecureSessionTable> mSessionTable;
    std::vector<Platform::UniquePtr<SessionNotificationListener>> mSessionList;
};

void TestSecureSessionTable::AllocateSession(const ScopedNodeId & sessionEvictionHint,
                                             std::vector<SessionParameters> & sessionParameters, uint16_t evictedSessionIndex)
{
    auto session = mSessionTable->CreateNewSecureSession(SecureSession::Type::kCASE, sessionEvictionHint);
    EXPECT_TRUE(session.HasValue());
    EXPECT_TRUE(mSessionList[evictedSessionIndex].get()->mSessionReleased);
}

void TestSecureSessionTable::CreateSessionTable(std::vector<SessionParameters> & sessionParams)
{
    mSessionList.clear();

    mSessionTable = Platform::MakeUnique<SecureSessionTable>();
    EXPECT_NE(mSessionTable.get(), nullptr);

    mSessionTable->Init();
    mSessionTable->SetMaxSessionTableSize(static_cast<uint32_t>(sessionParams.size()));

    for (unsigned int i = 0; i < sessionParams.size(); i++)
    {
        auto session = mSessionTable->CreateNewSecureSession(SecureSession::Type::kCASE, ScopedNodeId());
        EXPECT_TRUE(session.HasValue());

        session.Value()->AsSecureSession()->Activate(
            ScopedNodeId(1, sessionParams[i].mPeer.GetFabricIndex()), sessionParams[i].mPeer, CATValues(), static_cast<uint16_t>(i),
            ReliableMessageProtocolConfig(System::Clock::Milliseconds32(0), System::Clock::Milliseconds32(0),
                                          System::Clock::Milliseconds16(0)));

        // Make sure we set up our holder _before_ the session goes into a state
        // other than active, because holders refuse to hold non-active
        // sessions.
        mSessionList.push_back(Platform::MakeUnique<SessionNotificationListener>(session.Value()));

        session.Value()->AsSecureSession()->mLastActivityTime = sessionParams[i].mLastActivityTime;
        session.Value()->AsSecureSession()->mState            = sessionParams[i].mState;
    }
}

void TestSecureSessionTable::ValidateSessionSorting()
{
    //
    // This validates basic eviction. The table is full of sessions from Fabric1 from the same
    // Node (2). Eviction should select the oldest session in the table (with timestamp 1) and evict that
    //
    {
        ChipLogProgress(SecureChannel, "-------- Validating Basic Eviction (Matching Hint's Fabric)  --------");

        std::vector<SessionParameters> sessionParamList = {
            { { 2, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(7), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(1), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
        };

        CreateSessionTable(sessionParamList);
        AllocateSession(ScopedNodeId(2, kFabric1), sessionParamList, 4);
    }

    //
    // This validates basic eviction, with the sessionHint indicating a request from a different fabric than
    // those in the table. Nothing changes from the example above since the sessions in the table are over minima,
    // so it will just reap the oldest session in the table (with timestamp 1 again).
    //
    //
    {
        ChipLogProgress(SecureChannel, "-------- Validating Basic Eviction (No Match for Hint's Fabric) --------");

        std::vector<SessionParameters> sessionParamList = {
            { { 2, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(7), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(1), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
        };

        CreateSessionTable(sessionParamList);
        AllocateSession(ScopedNodeId(2, kFabric2), sessionParamList, 4);
    }

    //
    // This validates evicting an over-minima fabric from the session table where there
    // are sessions from two fabrics, Fabric1 and Fabric2.
    //
    // Fabric1 has 2 sessions, and Fabric2 has 4 sessions. Fabric2 will be selected since
    // it has more sessions than Fabric2.
    //
    // Within that set, there are more sessions to Node 2 than others, so the oldest one
    // in that set (timestamp 3) will be selected.
    //
    {
        ChipLogProgress(SecureChannel, "-------- Over-minima Fabric Eviction ---------");

        std::vector<SessionParameters> sessionParamList = {
            { { 2, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kActive },
            { { 2, kFabric2 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 1, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 2, kFabric2 }, System::Clock::Timestamp(7), SecureSession::State::kActive },
            { { 3, kFabric2 }, System::Clock::Timestamp(1), SecureSession::State::kActive },
            { { 4, kFabric2 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
        };

        CreateSessionTable(sessionParamList);
        AllocateSession(ScopedNodeId(2, kFabric1), sessionParamList, 1);
    }

    //
    // This validates evicting an over-minima fabric from the session table where there
    // are sessions from two fabrics, Fabric1 and Fabric2.
    //
    // Fabric1 has 2 sessions, and Fabric2 has 3 sessions. Fabric2 will be selected since
    // it has more sessions than Fabric2.
    //
    // Within that set, there are more sessions to Node 2 than others, except one session
    // is in the pairing state. So the active one will be selected instead.
    //
    {
        ChipLogProgress(SecureChannel, "-------- Over-minima Fabric Eviction (State) ---------");

        std::vector<SessionParameters> sessionParamList = {
            { { 2, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kActive },
            { { 2, kFabric2 }, System::Clock::Timestamp(3), SecureSession::State::kEstablishing },
            { { 1, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 2, kFabric2 }, System::Clock::Timestamp(7), SecureSession::State::kActive },
            { { 3, kFabric2 }, System::Clock::Timestamp(1), SecureSession::State::kActive },
            { { 4, kFabric2 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
        };

        CreateSessionTable(sessionParamList);
        AllocateSession(ScopedNodeId(2, kFabric1), sessionParamList, 3);
    }

    //
    // This validates evicting an over-minima fabric from the session table where there
    // are sessions from two fabrics, Fabric1 and Fabric2.
    //
    // Fabric1 has 2 sessions, and Fabric2 has 4 sessions. Fabric2 will be selected since
    // it has more sessions than Fabric1.
    //
    // Within that set, there are equal sessions to each node, so the session with the
    // older timestamp will be selected.
    //
    {
        ChipLogProgress(SecureChannel, "-------- Over-minima Fabric Eviction ---------");

        std::vector<SessionParameters> sessionParamList = {
            { { 2, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kActive },
            { { 1, kFabric2 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 1, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 2, kFabric2 }, System::Clock::Timestamp(7), SecureSession::State::kActive },
            { { 3, kFabric2 }, System::Clock::Timestamp(1), SecureSession::State::kActive },
            { { 4, kFabric2 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
        };

        CreateSessionTable(sessionParamList);
        AllocateSession(ScopedNodeId(2, kFabric1), sessionParamList, 4);
    }

    //
    // This validates evicting from a table with equally loaded fabrics. In this scenario,
    // bias is given to the fabric that matches that of the eviction hint.
    //
    // There are more sessions to Node 2 in that fabric, so despite there be a match to
    // Node 3 in the table, the older session to Node 2 will be evicted.
    //
    {
        ChipLogProgress(SecureChannel, "-------- Equal Fabrics Eviction (Un-equal # Sessions / Node) ---------");

        std::vector<SessionParameters> sessionParamList = {
            { { 2, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kActive },
            { { 1, kFabric2 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 3, kFabric1 }, System::Clock::Timestamp(7), SecureSession::State::kActive },
            { { 3, kFabric2 }, System::Clock::Timestamp(1), SecureSession::State::kActive },
            { { 4, kFabric2 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
        };

        CreateSessionTable(sessionParamList);
        AllocateSession(ScopedNodeId(3, kFabric1), sessionParamList, 2);
    }

    //
    // This validates evicting from a table with equally loaded fabrics. In this scenario,
    // bias is given to the fabric that matches that of the eviction hint.
    //
    // There is an equal number sessions to nodes 1, 2, and 3 in that fabric, so the Node
    // that matches the session eviction hint will be selected.
    //
    // All the sessions in the table are defunct, because for unique active
    // sessions eviction hints are ignored.
    //
    {
        ChipLogProgress(
            SecureChannel,
            "-------- Equal Fabrics Eviction (Single equal # Sessions to Nodes, Hint Match On Fabric & Node) ---------");

        std::vector<SessionParameters> sessionParamList = {
            { { 1, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kDefunct },
            { { 1, kFabric2 }, System::Clock::Timestamp(3), SecureSession::State::kDefunct },
            { { 2, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kDefunct },
            { { 3, kFabric1 }, System::Clock::Timestamp(7), SecureSession::State::kDefunct },
            { { 3, kFabric2 }, System::Clock::Timestamp(1), SecureSession::State::kDefunct },
            { { 4, kFabric2 }, System::Clock::Timestamp(2), SecureSession::State::kDefunct },
        };

        CreateSessionTable(sessionParamList);
        AllocateSession(ScopedNodeId(3, kFabric1), sessionParamList, 3);
    }

    //
    // This validates evicting from a table with equally loaded fabrics. In this scenario,
    // bias is given to the fabric that matches that of the eviction hint.
    //
    // There is an equal number sessions to nodes 1, 2, and 3 in that fabric, so the Node
    // that matches the session eviction hint will be selected.
    //
    // All the peers in this table have two sessions to them, so that we pay
    // attention to the eviction hint.  The older of the two should be selected.
    //
    {
        ChipLogProgress(
            SecureChannel,
            "-------- Equal Fabrics Eviction (Multiple equal # Sessions to Nodes, Hint Match On Fabric & Node) ---------");

        std::vector<SessionParameters> sessionParamList = {
            { { 1, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kActive },
            { { 1, kFabric2 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 3, kFabric1 }, System::Clock::Timestamp(7), SecureSession::State::kActive },
            { { 3, kFabric2 }, System::Clock::Timestamp(1), SecureSession::State::kActive },
            { { 4, kFabric2 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 1, kFabric1 }, System::Clock::Timestamp(10), SecureSession::State::kActive },
            { { 1, kFabric2 }, System::Clock::Timestamp(4), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 3, kFabric1 }, System::Clock::Timestamp(8), SecureSession::State::kActive },
            { { 3, kFabric2 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 4, kFabric2 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
        };

        CreateSessionTable(sessionParamList);
        AllocateSession(ScopedNodeId(3, kFabric1), sessionParamList, 3);
    }

    //
    // This validates evicting from a table with equally loaded fabrics. In this scenario,
    // bias is given to the fabric that matches that of the eviction hint.
    //
    // There is an equal sessions to nodes 1, 2, and 3 in that fabric, and only
    // one per node.  Since all the sessions are active, the eviction hint's
    // node id will be ignored and the oldest session on the fabric will be selected.
    //
    {
        ChipLogProgress(SecureChannel,
                        "-------- Equal Fabrics Eviction (Equal # Sessions to Nodes, Hint Match On Fabric & Node, hint node "
                        "ignored) ---------");

        std::vector<SessionParameters> sessionParamList = {
            { { 1, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kActive },
            { { 1, kFabric2 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 3, kFabric1 }, System::Clock::Timestamp(7), SecureSession::State::kActive },
            { { 3, kFabric2 }, System::Clock::Timestamp(1), SecureSession::State::kActive },
            { { 4, kFabric2 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
        };

        CreateSessionTable(sessionParamList);
        AllocateSession(ScopedNodeId(3, kFabric1), sessionParamList, 2);
    }

    //
    // This validates evicting from a table with equally loaded fabrics. In this scenario,
    // bias is given to the fabric that matches that of the eviction hint.
    //
    // There is an equal sessions to nodes 1, 2, and 3 in that fabric, and only
    // one per node.  Since the hinted session is active, the eviction hint's
    // node id will be ignored and the defunct session will be selected, even
    // though it's the newest one.
    //
    {
        ChipLogProgress(SecureChannel,
                        "-------- Equal Fabrics Eviction (Equal # Sessions to Nodes, Hint Match On Fabric & Node, hint node "
                        "ignored and state wins) ---------");

        std::vector<SessionParameters> sessionParamList = {
            { { 1, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kDefunct },
            { { 1, kFabric2 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 3, kFabric1 }, System::Clock::Timestamp(7), SecureSession::State::kActive },
            { { 3, kFabric2 }, System::Clock::Timestamp(1), SecureSession::State::kActive },
            { { 4, kFabric2 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
        };

        CreateSessionTable(sessionParamList);
        AllocateSession(ScopedNodeId(3, kFabric1), sessionParamList, 0);
    }

    //
    // Similar to above, except that the eviction hint matches a given fabric (kFabric1) in the
    // session table, but not any nodes. In this case, the oldest session in that fabric is selected
    // for eviction from the table.
    //
    {
        ChipLogProgress(SecureChannel,
                        "-------- Equal Fabrics Eviction (Equal # of Sessions to Nodes, Hint Match on Fabric ONLY) ---------");

        std::vector<SessionParameters> sessionParamList = {
            { { 1, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kActive },
            { { 1, kFabric2 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 3, kFabric1 }, System::Clock::Timestamp(7), SecureSession::State::kActive },
            { { 3, kFabric2 }, System::Clock::Timestamp(1), SecureSession::State::kActive },
            { { 4, kFabric2 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
        };

        CreateSessionTable(sessionParamList);
        AllocateSession(ScopedNodeId(4, kFabric1), sessionParamList, 2);
    }

    //
    // Similar to above, except the eviction hint does not match any fabric in the session table.
    // Given all fabrics are within minimas, the oldest session is then selected.
    //
    {
        ChipLogProgress(SecureChannel, "-------- Equal Fabrics Eviction (Equal # of Sessions to Nodes, No Hint Match) ---------");

        std::vector<SessionParameters> sessionParamList = {
            { { 1, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kActive },
            { { 1, kFabric2 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 3, kFabric1 }, System::Clock::Timestamp(7), SecureSession::State::kActive },
            { { 3, kFabric2 }, System::Clock::Timestamp(1), SecureSession::State::kActive },
            { { 4, kFabric2 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
        };

        CreateSessionTable(sessionParamList);
        AllocateSession(ScopedNodeId(4, kFabric3), sessionParamList, 4);
    }

    //
    // Similar to above, except the oldest session happens to not be an active one. Instead,
    // select the next oldest active session.
    //
    {
        ChipLogProgress(
            SecureChannel,
            "-------- Equal Fabrics Eviction (Equal # of Sessions to Nodes, No Hint Match, In-active Session) ---------");

        std::vector<SessionParameters> sessionParamList = {
            { { 1, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kActive },
            { { 1, kFabric2 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 3, kFabric1 }, System::Clock::Timestamp(7), SecureSession::State::kActive },
            { { 3, kFabric2 }, System::Clock::Timestamp(1), SecureSession::State::kEstablishing },
            { { 4, kFabric2 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
        };

        CreateSessionTable(sessionParamList);
        AllocateSession(ScopedNodeId(4, kFabric3), sessionParamList, 5);
    }
}

TEST_F(TestSecureSessionTable, ValidateSessionSorting)
{
    // This calls TestSecureSessionTable::ValidateSessionSorting instead of just doing the
    // tests directly in here, since the tests reference `SecureSession::State`, which is
    // private.  Defining the function inside TestSecureSessionTable allows State to be
    // accessible since SecureSession contains `friend class TestSecureSessionTable`.
    ValidateSessionSorting();
}

} // namespace Transport
} // namespace chip
