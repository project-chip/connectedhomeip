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

#include <app/clusters/operational-state-server/OperationalStateCluster.h>
#include <app/clusters/operational-state-server/OvenCavityOperationalStateCluster.h>
#include <app/clusters/operational-state-server/RvcOperationalStateCluster.h>
#include <app/clusters/operational-state-server/operational-state-cluster-objects.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <lib/support/CHIPMem.h>
#include <pw_unit_test/framework.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

using namespace chip::app::Clusters::OperationalState::Attributes;
using namespace chip::Protocols::InteractionModel;

// ---------------------------------------------------------------------------
// Mock delegate
// ---------------------------------------------------------------------------

class MockDelegate : public OperationalStateCluster::Delegate
{
public:
    static constexpr uint8_t kStateCount = 4;

    const GenericOperationalState mStates[kStateCount] = {
        GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kPaused)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
    };

    DataModel::Nullable<uint32_t> GetCountdownTime() override { return mCountdownTime; }

    CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & state) override
    {
        if (index >= kStateCount)
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        state = mStates[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & phase) override
    {
        if (mPhases.empty() || index >= mPhases.size())
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        (void) CopyCharSpanToMutableCharSpan(mPhases[index], phase);
        return CHIP_NO_ERROR;
    }

    void HandlePauseStateCallback(GenericOperationalError & err) override
    {
        err          = mCallbackError;
        mPauseCalled = true;
    }
    void HandleResumeStateCallback(GenericOperationalError & err) override
    {
        err           = mCallbackError;
        mResumeCalled = true;
    }
    void HandleStartStateCallback(GenericOperationalError & err) override
    {
        err          = mCallbackError;
        mStartCalled = true;
    }
    void HandleStopStateCallback(GenericOperationalError & err) override
    {
        err         = mCallbackError;
        mStopCalled = true;
    }

    DataModel::Nullable<uint32_t> mCountdownTime = DataModel::NullNullable;
    std::vector<CharSpan> mPhases;
    GenericOperationalError mCallbackError = to_underlying(ErrorStateEnum::kNoError);

    bool mPauseCalled  = false;
    bool mResumeCalled = false;
    bool mStartCalled  = false;
    bool mStopCalled   = false;

    void ResetCallFlags() { mPauseCalled = mResumeCalled = mStartCalled = mStopCalled = false; }
};

class TestableOperationalStateCluster : public OperationalStateCluster
{
public:
    using OperationalStateCluster::OperationalStateCluster;
};

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------

class OperationalStateClusterTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        mCluster = std::make_unique<TestableOperationalStateCluster>(kTestEndpoint, &mDelegate);
        mTester  = std::make_unique<Testing::ClusterTester>(*mCluster);
        ASSERT_EQ(mCluster->Startup(mTester->GetServerClusterContext()), CHIP_NO_ERROR);

        // Set default state to Stopped so commands work as expected.
        ASSERT_EQ(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kStopped)), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mCluster->Shutdown(ClusterShutdownType::kClusterShutdown);
        mTester.reset();
        mCluster.reset();
        chip::Platform::MemoryShutdown();
    }

protected:
    static constexpr EndpointId kTestEndpoint = 1;

    MockDelegate mDelegate;
    std::unique_ptr<TestableOperationalStateCluster> mCluster;
    std::unique_ptr<Testing::ClusterTester> mTester;
};

// ---------------------------------------------------------------------------
// Attributes() — mandatory list
// ---------------------------------------------------------------------------

TEST_F(OperationalStateClusterTest, AttributeList_Mandatory)
{
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> builder;
    EXPECT_EQ(mCluster->Attributes(mCluster->GetPaths()[0], builder), CHIP_NO_ERROR);
    auto list = builder.TakeBuffer();

    // ClusterRevision, FeatureMap, PhaseList, CurrentPhase, OperationalStateList, OperationalState, OperationalError
    // must all be present.
    auto HasAttr = [&](AttributeId id) {
        for (auto & entry : list)
        {
            if (entry.attributeId == id)
                return true;
        }
        return false;
    };

    EXPECT_TRUE(HasAttr(Globals::Attributes::ClusterRevision::Id));
    EXPECT_TRUE(HasAttr(Globals::Attributes::FeatureMap::Id));
    EXPECT_TRUE(HasAttr(PhaseList::Id));
    EXPECT_TRUE(HasAttr(CurrentPhase::Id));
    EXPECT_TRUE(HasAttr(OperationalStateList::Id));
    EXPECT_TRUE(HasAttr(Attributes::OperationalState::Id));
    EXPECT_TRUE(HasAttr(OperationalError::Id));
    // CountdownTime is optional and should NOT be in the list unless enabled.
    EXPECT_FALSE(HasAttr(CountdownTime::Id));
}

TEST_F(OperationalStateClusterTest, AttributeList_WithCountdownTime)
{
    OperationalStateCluster::Config cfg;
    cfg.optionalAttributes.Set<CountdownTime::Id>();
    TestableOperationalStateCluster clusterWithCountdown(kTestEndpoint, &mDelegate, cfg);
    Testing::ClusterTester tester(clusterWithCountdown);
    ASSERT_EQ(clusterWithCountdown.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(clusterWithCountdown.SetOperationalState(to_underlying(OperationalStateEnum::kStopped)), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> builder;
    EXPECT_EQ(clusterWithCountdown.Attributes(clusterWithCountdown.GetPaths()[0], builder), CHIP_NO_ERROR);
    auto list = builder.TakeBuffer();

    bool hasCountdown = false;
    for (auto & entry : list)
    {
        if (entry.attributeId == CountdownTime::Id)
        {
            hasCountdown = true;
            break;
        }
    }
    EXPECT_TRUE(hasCountdown);

    clusterWithCountdown.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------------------------------------------------------------------------
// ReadAttribute — mandatory attributes after construction
// ---------------------------------------------------------------------------

TEST_F(OperationalStateClusterTest, ReadClusterRevision)
{
    uint16_t revision = 0;
    EXPECT_TRUE(mTester->ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision).IsSuccess());
    EXPECT_EQ(revision, static_cast<uint16_t>(kRevision));
}

TEST_F(OperationalStateClusterTest, ReadFeatureMap)
{
    uint32_t featureMap = 0xFFFFFFFF;
    EXPECT_TRUE(mTester->ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap).IsSuccess());
    EXPECT_EQ(featureMap, 0u);
}

TEST_F(OperationalStateClusterTest, ReadPhaseList_NullWhenNoPhases)
{
    DataModel::Nullable<DataModel::DecodableList<CharSpan>> phaseList;
    EXPECT_TRUE(mTester->ReadAttribute(PhaseList::Id, phaseList).IsSuccess());
    EXPECT_TRUE(phaseList.IsNull());
}

TEST_F(OperationalStateClusterTest, ReadCurrentPhase_DefaultNull)
{
    DataModel::Nullable<uint8_t> phase;
    EXPECT_TRUE(mTester->ReadAttribute(CurrentPhase::Id, phase).IsSuccess());
    EXPECT_TRUE(phase.IsNull());
}

TEST_F(OperationalStateClusterTest, ReadOperationalState_InitialStopped)
{
    uint8_t state = 0xFF;
    EXPECT_TRUE(mTester->ReadAttribute(Attributes::OperationalState::Id, state).IsSuccess());
    EXPECT_EQ(state, to_underlying(OperationalStateEnum::kStopped));
}

TEST_F(OperationalStateClusterTest, ReadOperationalError_InitialNoError)
{
    Structs::ErrorStateStruct::DecodableType err;
    EXPECT_TRUE(mTester->ReadAttribute(OperationalError::Id, err).IsSuccess());
    EXPECT_EQ(err.errorStateID, to_underlying(ErrorStateEnum::kNoError));
}

// ---------------------------------------------------------------------------
// SetCurrentPhase and SetOperationalState
// ---------------------------------------------------------------------------

TEST_F(OperationalStateClusterTest, SetOperationalState_Valid)
{
    EXPECT_EQ(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kRunning)), CHIP_NO_ERROR);
    EXPECT_EQ(mCluster->GetCurrentOperationalState(), to_underlying(OperationalStateEnum::kRunning));

    auto & dirty = mTester->GetDirtyList();
    bool found   = false;
    for (auto & path : dirty)
    {
        if (path.mAttributeId == Attributes::OperationalState::Id)
        {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(OperationalStateClusterTest, SetOperationalState_ErrorState_Rejected)
{
    // kError must only be set via OnOperationalErrorDetected.
    EXPECT_EQ(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kError)), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(OperationalStateClusterTest, SetOperationalState_UnsupportedState_Rejected)
{
    EXPECT_EQ(mCluster->SetOperationalState(0x80), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(OperationalStateClusterTest, SetOperationalState_NoOpDoesNotNotify)
{
    EXPECT_EQ(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kStopped)), CHIP_NO_ERROR);
    auto & dirty = mTester->GetDirtyList();
    bool found   = false;
    for (auto & path : dirty)
    {
        if (path.mAttributeId == Attributes::OperationalState::Id)
        {
            found = true;
            break;
        }
    }
    EXPECT_FALSE(found);
}

TEST_F(OperationalStateClusterTest, SetOperationalState_PausedReportsCountdownTime)
{
    OperationalStateCluster::Config cfg;
    cfg.optionalAttributes.Set<CountdownTime::Id>();
    TestableOperationalStateCluster clusterWithCountdown(kTestEndpoint, &mDelegate, cfg);
    Testing::ClusterTester tester(clusterWithCountdown);
    ASSERT_EQ(clusterWithCountdown.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Initial state setup.
    EXPECT_EQ(clusterWithCountdown.SetOperationalState(to_underlying(OperationalStateEnum::kStopped)), CHIP_NO_ERROR);
    mDelegate.mCountdownTime = 100;
    clusterWithCountdown.UpdateCountdownTimeFromDelegate();
    tester.GetDirtyList().clear();

    // Simulate the countdown progressing by 1 second (100 -> 99).
    mDelegate.mCountdownTime = 99;

    // Change state to Paused. This should trigger UpdateCountdownTimeFromClusterLogic.
    EXPECT_EQ(clusterWithCountdown.SetOperationalState(to_underlying(OperationalStateEnum::kPaused)), CHIP_NO_ERROR);

    auto & dirty        = tester.GetDirtyList();
    bool countdownDirty = false;
    for (auto & path : dirty)
    {
        if (path.mAttributeId == CountdownTime::Id)
        {
            countdownDirty = true;
            break;
        }
    }
    // The CountdownTime must be reported when the state changes (cluster logic update),
    // even if it was a decrement that would normally be suppressed by the 10s delta rule.
    EXPECT_TRUE(countdownDirty);

    clusterWithCountdown.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------------------------------------------------------------------------
// IsSupportedPhase / IsSupportedOperationalState
// ---------------------------------------------------------------------------

TEST_F(OperationalStateClusterTest, IsSupportedOperationalState_Valid)
{
    EXPECT_TRUE(mCluster->IsSupportedOperationalState(to_underlying(OperationalStateEnum::kStopped)));
    EXPECT_TRUE(mCluster->IsSupportedOperationalState(to_underlying(OperationalStateEnum::kRunning)));
    EXPECT_FALSE(mCluster->IsSupportedOperationalState(0x80));
}

// ---------------------------------------------------------------------------
// Command: Start
// ---------------------------------------------------------------------------

TEST_F(OperationalStateClusterTest, InvokeStart_FromStopped_CallsDelegate)
{
    Commands::Start::Type req;
    auto result = mTester->Invoke(req);
    ASSERT_TRUE(result.response.has_value());
    if (result.response.has_value())
    {
        EXPECT_EQ(result.response.value().commandResponseState.errorStateID, to_underlying(ErrorStateEnum::kNoError));
    }
    EXPECT_TRUE(mDelegate.mStartCalled);
}

TEST_F(OperationalStateClusterTest, InvokeStart_WhenAlreadyRunning_NoCallback)
{
    ASSERT_EQ(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kRunning)), CHIP_NO_ERROR);
    mDelegate.ResetCallFlags();

    Commands::Start::Type req;
    (void) mTester->Invoke(req);
    EXPECT_FALSE(mDelegate.mStartCalled);
}

// ---------------------------------------------------------------------------
// Command: Stop
// ---------------------------------------------------------------------------

TEST_F(OperationalStateClusterTest, InvokeStop_FromRunning_CallsDelegate)
{
    ASSERT_EQ(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kRunning)), CHIP_NO_ERROR);
    mDelegate.ResetCallFlags();

    Commands::Stop::Type req;
    (void) mTester->Invoke(req);
    EXPECT_TRUE(mDelegate.mStopCalled);
}

TEST_F(OperationalStateClusterTest, InvokeStop_WhenAlreadyStopped_NoCallback)
{
    Commands::Stop::Type req;
    (void) mTester->Invoke(req);
    EXPECT_FALSE(mDelegate.mStopCalled);
}

// ---------------------------------------------------------------------------
// Command: Pause
// ---------------------------------------------------------------------------

TEST_F(OperationalStateClusterTest, InvokePause_FromRunning_CallsDelegate)
{
    ASSERT_EQ(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kRunning)), CHIP_NO_ERROR);
    mDelegate.ResetCallFlags();

    Commands::Pause::Type req;
    (void) mTester->Invoke(req);
    EXPECT_TRUE(mDelegate.mPauseCalled);
}

TEST_F(OperationalStateClusterTest, InvokePause_FromStopped_ErrorInvalidState)
{
    Commands::Pause::Type req;
    auto result = mTester->Invoke(req);
    EXPECT_FALSE(mDelegate.mPauseCalled);
    ASSERT_TRUE(result.response.has_value());
    if (result.response.has_value())
    {
        EXPECT_EQ(result.response.value().commandResponseState.errorStateID, to_underlying(ErrorStateEnum::kCommandInvalidInState));
    }
}

// ---------------------------------------------------------------------------
// Command: Resume
// ---------------------------------------------------------------------------

TEST_F(OperationalStateClusterTest, InvokeResume_FromPaused_CallsDelegate)
{
    ASSERT_EQ(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kRunning)), CHIP_NO_ERROR);
    ASSERT_EQ(mCluster->SetOperationalState(to_underlying(OperationalStateEnum::kPaused)), CHIP_NO_ERROR);
    mDelegate.ResetCallFlags();

    Commands::Resume::Type req;
    (void) mTester->Invoke(req);
    EXPECT_TRUE(mDelegate.mResumeCalled);
}

// ---------------------------------------------------------------------------
// OnOperationalErrorDetected
// ---------------------------------------------------------------------------

TEST_F(OperationalStateClusterTest, OnOperationalErrorDetected_SetsStateAndError)
{
    Structs::ErrorStateStruct::Type err;
    err.errorStateID = to_underlying(ErrorStateEnum::kUnableToStartOrResume);

    mCluster->OnOperationalErrorDetected(err);

    EXPECT_EQ(mCluster->GetCurrentOperationalState(), to_underlying(OperationalStateEnum::kError));

    GenericOperationalError outErr(to_underlying(ErrorStateEnum::kNoError));
    mCluster->GetCurrentOperationalError(outErr);
    EXPECT_EQ(outErr.errorStateID, to_underlying(ErrorStateEnum::kUnableToStartOrResume));
}

// ---------------------------------------------------------------------------
// AcceptedCommands
// ---------------------------------------------------------------------------

TEST_F(OperationalStateClusterTest, AcceptedCommands_ContainsBaseSet)
{
    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
    EXPECT_EQ(mCluster->AcceptedCommands(mCluster->GetPaths()[0], builder), CHIP_NO_ERROR);
    auto cmds = builder.TakeBuffer();

    auto HasCmd = [&](CommandId id) {
        for (auto & e : cmds)
        {
            if (e.commandId == id)
                return true;
        }
        return false;
    };

    EXPECT_TRUE(HasCmd(Commands::Pause::Id));
    EXPECT_TRUE(HasCmd(Commands::Stop::Id));
    EXPECT_TRUE(HasCmd(Commands::Start::Id));
    EXPECT_TRUE(HasCmd(Commands::Resume::Id));
}

// ---------------------------------------------------------------------------
// Startup / Shutdown cycle
// ---------------------------------------------------------------------------

TEST_F(OperationalStateClusterTest, StartupShutdown_Clean)
{
    // Covered by SetUp/TearDown. Verify double-startup is rejected.
    Testing::TestServerClusterContext ctx2;
    EXPECT_NE(mCluster->Startup(ctx2.Get()), CHIP_NO_ERROR);
}

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip

// ---------------------------------------------------------------------------
// RvcOperationalStateCluster tests
// ---------------------------------------------------------------------------

namespace chip {
namespace app {
namespace Clusters {
namespace RvcOperationalState {

using namespace chip::app::Clusters::OperationalState;

class MockRvcDelegate : public OperationalStateCluster::Delegate
{
public:
    static constexpr uint8_t kStateCount = 5;

    const GenericOperationalState mStates[kStateCount] = {
        GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kPaused)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
        GenericOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger)),
    };

    DataModel::Nullable<uint32_t> GetCountdownTime() override { return DataModel::NullNullable; }

    CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & state) override
    {
        if (index >= kStateCount)
            return CHIP_ERROR_NOT_FOUND;
        state = mStates[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetOperationalPhaseAtIndex(size_t, MutableCharSpan &) override { return CHIP_ERROR_NOT_FOUND; }

    void HandlePauseStateCallback(GenericOperationalError & err) override { err.Set(to_underlying(ErrorStateEnum::kNoError)); }
    void HandleResumeStateCallback(GenericOperationalError & err) override { err.Set(to_underlying(ErrorStateEnum::kNoError)); }
    void HandleStartStateCallback(GenericOperationalError & err) override
    {
        err.Set(to_underlying(chip::app::Clusters::OperationalState::ErrorStateEnum::kUnknownEnumValue));
    }
    void HandleStopStateCallback(GenericOperationalError & err) override
    {
        err.Set(to_underlying(chip::app::Clusters::OperationalState::ErrorStateEnum::kUnknownEnumValue));
    }
    void HandleGoHomeCommandCallback(GenericOperationalError & err) override
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
        mGoHomeCalled = true;
    }

    bool mGoHomeCalled = false;
};

TEST(RvcOperationalStateClusterTest, AcceptedCommands_RvcSet)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);

    MockRvcDelegate delegate;
    RvcOperationalStateCluster cluster(1, &delegate);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.SetOperationalState(to_underlying(OperationalStateEnum::kStopped)), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
    EXPECT_EQ(cluster.AcceptedCommands(cluster.GetPaths()[0], builder), CHIP_NO_ERROR);
    auto cmds = builder.TakeBuffer();

    auto HasCmd = [&](CommandId id) {
        for (auto & e : cmds)
        {
            if (e.commandId == id)
                return true;
        }
        return false;
    };

    EXPECT_TRUE(HasCmd(Commands::Pause::Id));
    EXPECT_TRUE(HasCmd(Commands::Resume::Id));
    EXPECT_TRUE(HasCmd(Commands::GoHome::Id));
    EXPECT_FALSE(HasCmd(chip::app::Clusters::OperationalState::Commands::Stop::Id));
    EXPECT_FALSE(HasCmd(chip::app::Clusters::OperationalState::Commands::Start::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    chip::Platform::MemoryShutdown();
}

TEST(RvcOperationalStateClusterTest, InvokeGoHome_CallsDelegate)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        MockRvcDelegate delegate;
        RvcOperationalStateCluster cluster(1, &delegate);
        Testing::ClusterTester tester(cluster);
        ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetOperationalState(to_underlying(OperationalStateEnum::kRunning)), CHIP_NO_ERROR);

        Commands::GoHome::Type req;
        (void) tester.Invoke(req);
        EXPECT_TRUE(delegate.mGoHomeCalled);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        // tester (and its MockCommandHandler holding the response PacketBufferHandle) is
        // destroyed here, before MemoryShutdown, to avoid a VERIFY_INITIALIZED() abort in
        // debug builds when MemoryFree is called after memoryInitialized has been zeroed.
    }
    chip::Platform::MemoryShutdown();
}

} // namespace RvcOperationalState
} // namespace Clusters
} // namespace app
} // namespace chip

// ---------------------------------------------------------------------------
// OvenCavityOperationalStateCluster tests
// ---------------------------------------------------------------------------

namespace chip {
namespace app {
namespace Clusters {
namespace OvenCavityOperationalState {

using namespace chip::app::Clusters::OperationalState;

class MockOvenDelegate : public OperationalStateCluster::Delegate
{
public:
    static constexpr uint8_t kStateCount = 3;

    const GenericOperationalState mStates[kStateCount] = {
        GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
    };

    DataModel::Nullable<uint32_t> GetCountdownTime() override { return DataModel::NullNullable; }

    CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & state) override
    {
        if (index >= kStateCount)
            return CHIP_ERROR_NOT_FOUND;
        state = mStates[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetOperationalPhaseAtIndex(size_t, MutableCharSpan &) override { return CHIP_ERROR_NOT_FOUND; }

    void HandlePauseStateCallback(GenericOperationalError & err) override { err.Set(to_underlying(ErrorStateEnum::kNoError)); }
    void HandleResumeStateCallback(GenericOperationalError & err) override { err.Set(to_underlying(ErrorStateEnum::kNoError)); }
    void HandleStartStateCallback(GenericOperationalError & err) override
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
        mStartCalled = true;
    }
    void HandleStopStateCallback(GenericOperationalError & err) override
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
        mStopCalled = true;
    }

    bool mStartCalled = false;
    bool mStopCalled  = false;
};

TEST(OvenCavityOperationalStateClusterTest, AcceptedCommands_OvenSet)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);

    MockOvenDelegate delegate;
    OvenCavityOperationalStateCluster cluster(1, &delegate);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.SetOperationalState(to_underlying(OperationalStateEnum::kStopped)), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
    EXPECT_EQ(cluster.AcceptedCommands(cluster.GetPaths()[0], builder), CHIP_NO_ERROR);
    auto cmds = builder.TakeBuffer();

    auto HasCmd = [&](CommandId id) {
        for (auto & e : cmds)
        {
            if (e.commandId == id)
                return true;
        }
        return false;
    };

    EXPECT_TRUE(HasCmd(Commands::Stop::Id));
    EXPECT_TRUE(HasCmd(Commands::Start::Id));
    EXPECT_FALSE(HasCmd(chip::app::Clusters::OperationalState::Commands::Pause::Id));
    EXPECT_FALSE(HasCmd(chip::app::Clusters::OperationalState::Commands::Resume::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    chip::Platform::MemoryShutdown();
}

TEST(OvenCavityOperationalStateClusterTest, ClusterRevision_IsTwo)
{
    ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);

    MockOvenDelegate delegate;
    OvenCavityOperationalStateCluster cluster(1, &delegate);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.SetOperationalState(to_underlying(OperationalStateEnum::kStopped)), CHIP_NO_ERROR);

    uint16_t revision = 0;
    EXPECT_TRUE(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision).IsSuccess());
    EXPECT_EQ(revision, static_cast<uint16_t>(OvenCavityOperationalState::kRevision));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    chip::Platform::MemoryShutdown();
}

} // namespace OvenCavityOperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
