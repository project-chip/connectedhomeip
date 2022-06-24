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

#include "system/SystemClock.h"
#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <transport/SecureSessionTable.h>
#include <transport/SessionHolder.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>
#include <vector>

namespace chip {
namespace Transport {

class TestSecureSessionTable
{
public:
    //
    // This test specifically validates eviction of sessions in the session table
    // with various scenarios based on the existing set of sessions in the table
    // and a provided session eviction hint
    //
    static void ValidateSessionSorting(nlTestSuite * inSuite, void * inContext);

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

    nlTestSuite * mTestSuite;
    Platform::UniquePtr<SecureSessionTable> mSessionTable;
    std::vector<Platform::UniquePtr<SessionNotificationListener>> mSessionList;
};

void TestSecureSessionTable::AllocateSession(const ScopedNodeId & sessionEvictionHint,
                                             std::vector<SessionParameters> & sessionParameters, uint16_t evictedSessionIndex)
{
    auto session = mSessionTable->CreateNewSecureSession(SecureSession::Type::kCASE, sessionEvictionHint);
    NL_TEST_ASSERT(mTestSuite, session.HasValue());
    NL_TEST_ASSERT(mTestSuite, mSessionList[evictedSessionIndex].get()->mSessionReleased == true);
}

void TestSecureSessionTable::CreateSessionTable(std::vector<SessionParameters> & sessionParams)
{
    mSessionList.clear();

    mSessionTable = Platform::MakeUnique<SecureSessionTable>();
    NL_TEST_ASSERT(mTestSuite, mSessionTable.get() != nullptr);

    mSessionTable->Init();
    mSessionTable->SetMaxSessionTableSize(static_cast<uint32_t>(sessionParams.size()));

    for (unsigned int i = 0; i < sessionParams.size(); i++)
    {
        auto session = mSessionTable->CreateNewSecureSession(SecureSession::Type::kCASE, ScopedNodeId());
        NL_TEST_ASSERT(mTestSuite, session.HasValue());

        session.Value()->AsSecureSession()->Activate(
            ScopedNodeId(1, sessionParams[i].mPeer.GetFabricIndex()), sessionParams[i].mPeer, CATValues(), static_cast<uint16_t>(i),
            ReliableMessageProtocolConfig(System::Clock::Milliseconds32(0), System::Clock::Milliseconds32(0)));
        session.Value()->AsSecureSession()->mLastActivityTime = sessionParams[i].mLastActivityTime;
        session.Value()->AsSecureSession()->mState            = sessionParams[i].mState;

        mSessionList.push_back(Platform::MakeUnique<SessionNotificationListener>(session.Value()));
    }
}

void TestSecureSessionTable::ValidateSessionSorting(nlTestSuite * inSuite, void * inContext)
{
    Platform::UniquePtr<TestSecureSessionTable> & _this = *static_cast<Platform::UniquePtr<TestSecureSessionTable> *>(inContext);
    _this->mTestSuite                                   = inSuite;

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

        _this->CreateSessionTable(sessionParamList);
        _this->AllocateSession(ScopedNodeId(2, kFabric1), sessionParamList, 4);
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

        _this->CreateSessionTable(sessionParamList);
        _this->AllocateSession(ScopedNodeId(2, kFabric2), sessionParamList, 4);
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

        _this->CreateSessionTable(sessionParamList);
        _this->AllocateSession(ScopedNodeId(2, kFabric1), sessionParamList, 1);
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

        _this->CreateSessionTable(sessionParamList);
        _this->AllocateSession(ScopedNodeId(2, kFabric1), sessionParamList, 3);
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

        _this->CreateSessionTable(sessionParamList);
        _this->AllocateSession(ScopedNodeId(2, kFabric1), sessionParamList, 4);
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

        _this->CreateSessionTable(sessionParamList);
        _this->AllocateSession(ScopedNodeId(3, kFabric1), sessionParamList, 2);
    }

    //
    // This validates evicting from a table with equally loaded fabrics. In this scenario,
    // bias is given to the fabric that matches that of the eviction hint.
    //
    // There are equal sessions to Node 2 as well as Node 3 in that fabric, so the Node
    // that matches the session eviction hint will be selected, and in that, the older session.
    //
    {
        ChipLogProgress(SecureChannel,
                        "-------- Equal Fabrics Eviction (Equal # Sessions to Nodes, Hint Match On Fabric & Node) ---------");

        std::vector<SessionParameters> sessionParamList = {
            { { 1, kFabric1 }, System::Clock::Timestamp(9), SecureSession::State::kActive },
            { { 1, kFabric2 }, System::Clock::Timestamp(3), SecureSession::State::kActive },
            { { 2, kFabric1 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
            { { 3, kFabric1 }, System::Clock::Timestamp(7), SecureSession::State::kActive },
            { { 3, kFabric2 }, System::Clock::Timestamp(1), SecureSession::State::kActive },
            { { 4, kFabric2 }, System::Clock::Timestamp(2), SecureSession::State::kActive },
        };

        _this->CreateSessionTable(sessionParamList);
        _this->AllocateSession(ScopedNodeId(3, kFabric1), sessionParamList, 3);
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

        _this->CreateSessionTable(sessionParamList);
        _this->AllocateSession(ScopedNodeId(4, kFabric1), sessionParamList, 2);
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

        _this->CreateSessionTable(sessionParamList);
        _this->AllocateSession(ScopedNodeId(4, kFabric3), sessionParamList, 4);
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

        _this->CreateSessionTable(sessionParamList);
        _this->AllocateSession(ScopedNodeId(4, kFabric3), sessionParamList, 5);
    }
}

Platform::UniquePtr<TestSecureSessionTable> gTestSecureSessionTable;

} // namespace Transport
} // namespace chip

// Test Suite

namespace {

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Validate Session Sorting (Over Minima)",               chip::Transport::TestSecureSessionTable::ValidateSessionSorting),
    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * apSuite)
{
    VerifyOrReturnError(chip::Platform::MemoryInit() == CHIP_NO_ERROR, FAILURE);
    chip::Transport::gTestSecureSessionTable = chip::Platform::MakeUnique<chip::Transport::TestSecureSessionTable>();
    return SUCCESS;
}

int Finalize(void * aContext)
{
    chip::Transport::gTestSecureSessionTable.reset();
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

// clang-format off
nlTestSuite sSuite =
{
    "TestSecureSessionTable",
    &sTests[0],
    Initialize,
    Finalize
};
// clang-format on

} // namespace

/**
 *  Main
 */
int SecureSessionTableTest()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &chip::Transport::gTestSecureSessionTable);

    int r = (nlTestRunnerStats(&sSuite));
    return r;
}

CHIP_REGISTER_TEST_SUITE(SecureSessionTableTest);
