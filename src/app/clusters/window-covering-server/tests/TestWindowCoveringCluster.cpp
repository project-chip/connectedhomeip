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

#include <pw_unit_test/framework.h>

#include <app/clusters/window-covering-server/WindowCoveringCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/WindowCovering/Metadata.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BitMask.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WindowCovering;
using namespace chip::Testing;

using Status = chip::Protocols::InteractionModel::Status;

namespace {

constexpr EndpointId kTestEndpointId = 1;

// ---------------------------------------------------------------------------
// Test doubles
// ---------------------------------------------------------------------------

// A mock delegate that records how it was called so the tests can assert that
// the cluster actually drives the hardware abstraction (the delegate) and with
// which parameters. The result of each call is configurable so we can simulate
// hardware that reports failure / "movement already in progress".
class MockWindowCoveringDelegate : public WindowCoveringDelegate
{
public:
    CHIP_ERROR HandleMovement(WindowCoveringType type) override
    {
        mHandleMovementCallCount++;
        mLastMovementType = type;
        return mHandleMovementResult;
    }

    CHIP_ERROR HandleStopMotion() override
    {
        mHandleStopMotionCallCount++;
        return mHandleStopMotionResult;
    }

    void Reset()
    {
        mHandleMovementCallCount   = 0;
        mHandleStopMotionCallCount = 0;
    }

    int mHandleMovementCallCount         = 0;
    WindowCoveringType mLastMovementType = WindowCoveringType::Lift;
    CHIP_ERROR mHandleMovementResult     = CHIP_NO_ERROR;

    int mHandleStopMotionCallCount     = 0;
    CHIP_ERROR mHandleStopMotionResult = CHIP_NO_ERROR;
};

// Exposes the protected `SetType` / `SetEndProductType` setters so the tests can
// exercise the RAM-backed attributes that are normally only written during
// application integration (they have no Interaction Model write path).
class TestableWindowCoveringCluster : public WindowCoveringCluster
{
public:
    using WindowCoveringCluster::SetEndProductType;
    using WindowCoveringCluster::SetType;
    using WindowCoveringCluster::WindowCoveringCluster;
};

// ---------------------------------------------------------------------------
// Test harness
// ---------------------------------------------------------------------------
//
// Bundles together the three collaborators every test needs: the mock delegate,
// the cluster under test, and a ClusterTester.
//
// The single most important detail here is Startup(): the cluster is started
// with the ClusterTester's OWN ServerClusterContext. ClusterTester embeds its
// own TestServerClusterContext (storage + attribute-change listener). If the
// cluster were started with a *different* context, then:
//   * NotifyAttributeChanged() would post dirty paths to a listener the tester
//     cannot see, so tester.GetDirtyList() would always be empty, and
//   * setter persistence would write to a different storage than the tester's.
// Wiring both to the same context is what makes the dirty-list and persistence
// assertions meaningful.
struct WindowCoveringHarness
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster;
    ClusterTester tester;

    WindowCoveringHarness(BitFlags<Feature> features, const WindowCovering::OptionalAttributeSet & optionals = {}) :
        cluster(kTestEndpointId, WindowCoveringCluster::Config(delegate).WithFeatures(features).WithOptionalAttributes(optionals)),
        tester(cluster)
    {}

    CHIP_ERROR Startup() { return cluster.Startup(tester.GetServerClusterContext()); }

    ~WindowCoveringHarness() { cluster.Shutdown(ClusterShutdownType::kClusterShutdown); }
};

struct TestWindowCoveringCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

} // namespace

// ===========================================================================
// 1. Lifecycle
// ===========================================================================

// The most basic contract: a validly configured cluster starts and stops
// cleanly. This is also a guard against the constructor's VerifyOrDie feature
// validation regressing (Lift/Tilt required, PositionAware* imply Lift/Tilt).
TEST_F(TestWindowCoveringCluster, StartupShutdown)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift });
    EXPECT_EQ(h.Startup(), CHIP_NO_ERROR);
}

// ===========================================================================
// 2. Attribute metadata verification (Attributes())
// ===========================================================================
//
// Attributes() must reflect the negotiated feature map and optional-attribute
// set exactly, because the Interaction Model uses this list to gate every read
// and write. IsAttributesListEqualTo() automatically folds in the mandatory
// global attributes (FeatureMap, ClusterRevision, ...), so the expected list
// only needs to enumerate the cluster-specific mandatory + conditional entries.

// Baseline: with a single feature and no optional attributes, only the five
// mandatory attributes (plus globals) are exposed. This pins down the floor of
// the attribute set so accidental additions are caught.
TEST_F(TestWindowCoveringCluster, AttributesList_LiftOnly)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(h.cluster,
                                        {
                                            Attributes::Type::kMetadataEntry,
                                            Attributes::ConfigStatus::kMetadataEntry,
                                            Attributes::OperationalStatus::kMetadataEntry,
                                            Attributes::EndProductType::kMetadataEntry,
                                            Attributes::Mode::kMetadataEntry,
                                        }));
}

// PositionAwareLift must pull in exactly the lift Percent100ths pair
// (Target + Current) and nothing on the tilt side.
TEST_F(TestWindowCoveringCluster, AttributesList_PositionAwareLift)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(h.cluster,
                                        {
                                            Attributes::Type::kMetadataEntry,
                                            Attributes::ConfigStatus::kMetadataEntry,
                                            Attributes::OperationalStatus::kMetadataEntry,
                                            Attributes::EndProductType::kMetadataEntry,
                                            Attributes::Mode::kMetadataEntry,
                                            Attributes::TargetPositionLiftPercent100ths::kMetadataEntry,
                                            Attributes::CurrentPositionLiftPercent100ths::kMetadataEntry,
                                        }));
}

// Symmetric case: PositionAwareTilt pulls in the tilt Percent100ths pair only.
TEST_F(TestWindowCoveringCluster, AttributesList_PositionAwareTilt)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kTilt, Feature::kPositionAwareTilt });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(h.cluster,
                                        {
                                            Attributes::Type::kMetadataEntry,
                                            Attributes::ConfigStatus::kMetadataEntry,
                                            Attributes::OperationalStatus::kMetadataEntry,
                                            Attributes::EndProductType::kMetadataEntry,
                                            Attributes::Mode::kMetadataEntry,
                                            Attributes::TargetPositionTiltPercent100ths::kMetadataEntry,
                                            Attributes::CurrentPositionTiltPercent100ths::kMetadataEntry,
                                        }));
}

// Fully-loaded case: both position-aware features plus every optional attribute.
// This verifies that feature-gated attributes and OptionalAttributeSet-gated
// attributes coexist and that the OptionalAttributeSet is built correctly with
// the checked `.Set<Id>()` API (passing an Id to the uint32_t constructor would
// silently be interpreted as a raw bitmask and drop attributes).
TEST_F(TestWindowCoveringCluster, AttributesList_AllFeaturesAndOptionals)
{
    WindowCovering::OptionalAttributeSet optionals;
    optionals.Set<Attributes::NumberOfActuationsLift::Id>()
        .Set<Attributes::NumberOfActuationsTilt::Id>()
        .Set<Attributes::CurrentPositionLiftPercentage::Id>()
        .Set<Attributes::CurrentPositionTiltPercentage::Id>()
        .Set<Attributes::SafetyStatus::Id>();

    WindowCoveringHarness h(
        BitFlags<Feature>{ Feature::kLift, Feature::kTilt, Feature::kPositionAwareLift, Feature::kPositionAwareTilt }, optionals);
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(h.cluster,
                                        {
                                            Attributes::Type::kMetadataEntry,
                                            Attributes::ConfigStatus::kMetadataEntry,
                                            Attributes::OperationalStatus::kMetadataEntry,
                                            Attributes::EndProductType::kMetadataEntry,
                                            Attributes::Mode::kMetadataEntry,
                                            Attributes::NumberOfActuationsLift::kMetadataEntry,
                                            Attributes::NumberOfActuationsTilt::kMetadataEntry,
                                            Attributes::CurrentPositionLiftPercentage::kMetadataEntry,
                                            Attributes::CurrentPositionTiltPercentage::kMetadataEntry,
                                            Attributes::SafetyStatus::kMetadataEntry,
                                            Attributes::TargetPositionLiftPercent100ths::kMetadataEntry,
                                            Attributes::CurrentPositionLiftPercent100ths::kMetadataEntry,
                                            Attributes::TargetPositionTiltPercent100ths::kMetadataEntry,
                                            Attributes::CurrentPositionTiltPercent100ths::kMetadataEntry,
                                        }));
}

// ===========================================================================
// 3. Accepted command verification (AcceptedCommands())
// ===========================================================================
//
// The three motion commands are always accepted. GoToLiftPercentage is only
// accepted when Lift is present and GoToTiltPercentage only when Tilt is
// present. Each combination is asserted independently so a wrong gating
// condition on either command is caught.

TEST_F(TestWindowCoveringCluster, AcceptedCommands_LiftOnly)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(h.cluster,
                                              {
                                                  Commands::UpOrOpen::kMetadataEntry,
                                                  Commands::DownOrClose::kMetadataEntry,
                                                  Commands::StopMotion::kMetadataEntry,
                                                  Commands::GoToLiftPercentage::kMetadataEntry,
                                              }));
}

TEST_F(TestWindowCoveringCluster, AcceptedCommands_TiltOnly)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kTilt });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(h.cluster,
                                              {
                                                  Commands::UpOrOpen::kMetadataEntry,
                                                  Commands::DownOrClose::kMetadataEntry,
                                                  Commands::StopMotion::kMetadataEntry,
                                                  Commands::GoToTiltPercentage::kMetadataEntry,
                                              }));
}

TEST_F(TestWindowCoveringCluster, AcceptedCommands_LiftAndTilt)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift, Feature::kTilt });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(h.cluster,
                                              {
                                                  Commands::UpOrOpen::kMetadataEntry,
                                                  Commands::DownOrClose::kMetadataEntry,
                                                  Commands::StopMotion::kMetadataEntry,
                                                  Commands::GoToLiftPercentage::kMetadataEntry,
                                                  Commands::GoToTiltPercentage::kMetadataEntry,
                                              }));
}

// ===========================================================================
// 4. Read attribute tests
// ===========================================================================

// Every mandatory attribute must be readable and return its documented default
// after a fresh Startup. Reading through ClusterTester also exercises the real
// IM path (it refuses to read anything absent from Attributes()), so this also
// confirms the mandatory attributes are actually advertised. FeatureMap is read
// to confirm the negotiated features round-trip through the encoder.
TEST_F(TestWindowCoveringCluster, ReadMandatoryDefaults)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift, Feature::kTilt });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    Type type{};
    ASSERT_EQ(h.tester.ReadAttribute(Attributes::Type::Id, type), CHIP_NO_ERROR);
    EXPECT_EQ(type, Type()); // default-constructed enum == kRollerShade (0)

    EndProductType productType{};
    ASSERT_EQ(h.tester.ReadAttribute(Attributes::EndProductType::Id, productType), CHIP_NO_ERROR);
    EXPECT_EQ(productType, EndProductType());

    chip::BitMask<ConfigStatus> configStatus{};
    ASSERT_EQ(h.tester.ReadAttribute(Attributes::ConfigStatus::Id, configStatus), CHIP_NO_ERROR);
    EXPECT_EQ(configStatus.Raw(), 0);

    chip::BitMask<OperationalStatus> opStatus{};
    ASSERT_EQ(h.tester.ReadAttribute(Attributes::OperationalStatus::Id, opStatus), CHIP_NO_ERROR);
    EXPECT_EQ(opStatus.Raw(), 0);

    chip::BitMask<Mode> mode{};
    ASSERT_EQ(h.tester.ReadAttribute(Attributes::Mode::Id, mode), CHIP_NO_ERROR);
    EXPECT_EQ(mode.Raw(), 0);

    BitFlags<Feature> featureMap;
    ASSERT_EQ(h.tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_TRUE(featureMap.HasAll(Feature::kLift, Feature::kTilt));
}

// Optional attributes are readable only when enabled. Here the two
// NumberOfActuations counters are enabled and must read back their default (0).
TEST_F(TestWindowCoveringCluster, ReadEnabledOptionalDefaults)
{
    WindowCovering::OptionalAttributeSet optionals;
    optionals.Set<Attributes::NumberOfActuationsLift::Id>().Set<Attributes::NumberOfActuationsTilt::Id>();

    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift, Feature::kTilt }, optionals);
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    uint16_t actuationsLift = 0xFFFF;
    ASSERT_EQ(h.tester.ReadAttribute(Attributes::NumberOfActuationsLift::Id, actuationsLift), CHIP_NO_ERROR);
    EXPECT_EQ(actuationsLift, 0u);

    uint16_t actuationsTilt = 0xFFFF;
    ASSERT_EQ(h.tester.ReadAttribute(Attributes::NumberOfActuationsTilt::Id, actuationsTilt), CHIP_NO_ERROR);
    EXPECT_EQ(actuationsTilt, 0u);
}

// A disabled optional attribute must not be readable: the IM (and therefore
// ClusterTester) rejects it as UnsupportedAttribute because it is absent from
// Attributes(). This is the exact failure mode that the previous test suite hit
// by trying to read NumberOfActuations without enabling it.
TEST_F(TestWindowCoveringCluster, ReadDisabledOptionalIsUnsupported)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    uint16_t actuationsLift = 0;
    EXPECT_EQ(h.tester.ReadAttribute(Attributes::NumberOfActuationsLift::Id, actuationsLift), Status::UnsupportedAttribute);
}

// Nullable persistent attributes must round-trip both a concrete value and null
// through the read path. CurrentPositionLiftPercent100ths starts null, accepts a
// concrete value, and can be reset back to null.
TEST_F(TestWindowCoveringCluster, ReadNullablePositionRoundTrip)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    NPercent100ths value;
    ASSERT_EQ(h.tester.ReadAttribute(Attributes::CurrentPositionLiftPercent100ths::Id, value), CHIP_NO_ERROR);
    EXPECT_TRUE(value.IsNull()); // default is null

    h.cluster.SetCurrentPositionLiftPercent100ths(NPercent100ths(4200));
    ASSERT_EQ(h.tester.ReadAttribute(Attributes::CurrentPositionLiftPercent100ths::Id, value), CHIP_NO_ERROR);
    ASSERT_FALSE(value.IsNull());
    EXPECT_EQ(value.Value(), 4200);

    h.cluster.SetCurrentPositionLiftPercent100ths(NPercent100ths()); // back to null
    ASSERT_EQ(h.tester.ReadAttribute(Attributes::CurrentPositionLiftPercent100ths::Id, value), CHIP_NO_ERROR);
    EXPECT_TRUE(value.IsNull());
}

// ===========================================================================
// 5. Write / setter boundary tests
// ===========================================================================

// Mode is the only Interaction-Model-writable attribute. The spec constrains it
// to the low nibble (bits 0..3), so raw values <= 0x0F are accepted and anything
// above must be rejected with ConstraintError. Boundary values (0x00 and 0x0F)
// are explicitly exercised.
TEST_F(TestWindowCoveringCluster, WriteMode_ValidAndConstraint)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    chip::BitMask<Mode> minValue;
    minValue.SetRaw(0x00);
    EXPECT_EQ(h.tester.WriteAttribute(Attributes::Mode::Id, minValue), CHIP_NO_ERROR);

    chip::BitMask<Mode> maxValid;
    maxValid.SetRaw(0x0F);
    EXPECT_EQ(h.tester.WriteAttribute(Attributes::Mode::Id, maxValid), CHIP_NO_ERROR);

    chip::BitMask<Mode> tooLarge;
    tooLarge.SetRaw(0x10);
    EXPECT_EQ(h.tester.WriteAttribute(Attributes::Mode::Id, tooLarge), Status::ConstraintError);
}

// Writing Mode derives ConfigStatus (operational + reversal bits). A fresh value
// marks both Mode and ConfigStatus dirty; re-writing the same value is a no-op
// that must not mark anything dirty (the SetAttributeValue no-op guard). This is
// the canonical "new value -> dirty, same value -> clean" pair for the write path.
TEST_F(TestWindowCoveringCluster, WriteMode_DirtyAndNoOp)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    auto & dirtyList = h.tester.GetDirtyList();

    chip::BitMask<Mode> mode;
    mode.Set(Mode::kLedFeedback);
    ASSERT_EQ(h.tester.WriteAttribute(Attributes::Mode::Id, mode), CHIP_NO_ERROR);

    // Mode changed 0 -> kLedFeedback and ConfigStatus changed 0 -> kOperational.
    EXPECT_TRUE(h.tester.IsAttributeDirty(Attributes::Mode::Id));
    EXPECT_TRUE(h.tester.IsAttributeDirty(Attributes::ConfigStatus::Id));
    EXPECT_EQ(dirtyList.size(), 2u);

    dirtyList.clear();

    // Re-writing the identical value is a no-op and must not re-notify.
    ASSERT_EQ(h.tester.WriteAttribute(Attributes::Mode::Id, mode), CHIP_NO_ERROR);
    EXPECT_TRUE(dirtyList.empty());
}

// The persistent counters are exposed only via C++ setters (no IM write path).
// They must follow the same dirty/no-op discipline: a changed value notifies,
// an unchanged value does not.
TEST_F(TestWindowCoveringCluster, SetterDirtyAndNoOp)
{
    WindowCovering::OptionalAttributeSet optionals;
    optionals.Set<Attributes::NumberOfActuationsLift::Id>();

    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift }, optionals);
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    auto & dirtyList = h.tester.GetDirtyList();

    h.cluster.SetNumberOfActuationsLift(5);
    EXPECT_TRUE(h.tester.IsAttributeDirty(Attributes::NumberOfActuationsLift::Id));
    EXPECT_EQ(h.cluster.GetNumberOfActuationsLift(), 5u);

    dirtyList.clear();

    h.cluster.SetNumberOfActuationsLift(5); // same value -> no-op
    EXPECT_TRUE(dirtyList.empty());
}

// ===========================================================================
// 6. Command behavior
// ===========================================================================

// UpOrOpen and DownOrClose must (a) drive the delegate for every enabled axis
// and (b) drive the position-aware target attributes to the open (0) and closed
// (10000) extremes respectively.
TEST_F(TestWindowCoveringCluster, Command_UpOrOpen_DownOrClose)
{
    WindowCoveringHarness h(
        BitFlags<Feature>{ Feature::kLift, Feature::kTilt, Feature::kPositionAwareLift, Feature::kPositionAwareTilt });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    // UpOrOpen -> fully open (minimum).
    h.delegate.Reset();
    auto up = h.tester.Invoke<Commands::UpOrOpen::Type>(Commands::UpOrOpen::Type());
    ASSERT_TRUE(up.IsSuccess());
    EXPECT_EQ(h.delegate.mHandleMovementCallCount, 2); // once for Lift, once for Tilt
    EXPECT_EQ(h.cluster.GetTargetPositionLiftPercent100ths().Value(), kWcPercent100thsMinOpen);
    EXPECT_EQ(h.cluster.GetTargetPositionTiltPercent100ths().Value(), kWcPercent100thsMinOpen);

    // DownOrClose -> fully closed (maximum).
    h.delegate.Reset();
    auto down = h.tester.Invoke<Commands::DownOrClose::Type>(Commands::DownOrClose::Type());
    ASSERT_TRUE(down.IsSuccess());
    EXPECT_EQ(h.delegate.mHandleMovementCallCount, 2);
    EXPECT_EQ(h.cluster.GetTargetPositionLiftPercent100ths().Value(), kWcPercent100thsMaxClosed);
    EXPECT_EQ(h.cluster.GetTargetPositionTiltPercent100ths().Value(), kWcPercent100thsMaxClosed);
}

// StopMotion must delegate to HandleStopMotion(). On a normal (success) stop the
// targets are snapped to the current position; the delegate call itself is the
// primary observable effect here.
TEST_F(TestWindowCoveringCluster, Command_StopMotion)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    h.delegate.Reset();
    auto result = h.tester.Invoke<Commands::StopMotion::Type>(Commands::StopMotion::Type());
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(h.delegate.mHandleStopMotionCallCount, 1);
}

// GoToLiftPercentage on a position-aware lift: a value in [0, 10000] is accepted,
// drives the delegate, and updates the lift target. The boundary values and an
// out-of-range value (ConstraintError, no delegate call) are all covered.
TEST_F(TestWindowCoveringCluster, Command_GoToLiftPercentage)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    // Mid-range value.
    {
        Commands::GoToLiftPercentage::Type cmd;
        cmd.liftPercent100thsValue = 5000;
        h.delegate.Reset();
        auto result = h.tester.Invoke<Commands::GoToLiftPercentage::Type>(cmd);
        ASSERT_TRUE(result.IsSuccess());
        EXPECT_EQ(h.delegate.mHandleMovementCallCount, 1);
        EXPECT_EQ(h.delegate.mLastMovementType, WindowCoveringType::Lift);
        EXPECT_EQ(h.cluster.GetTargetPositionLiftPercent100ths().Value(), 5000);
    }

    // Boundaries 0 and 10000 are valid.
    {
        Commands::GoToLiftPercentage::Type cmd;
        cmd.liftPercent100thsValue = kWcPercent100thsMaxClosed;
        auto result                = h.tester.Invoke<Commands::GoToLiftPercentage::Type>(cmd);
        ASSERT_TRUE(result.IsSuccess());
        EXPECT_EQ(h.cluster.GetTargetPositionLiftPercent100ths().Value(), kWcPercent100thsMaxClosed);
    }

    // Above the maximum -> ConstraintError, target unchanged, delegate untouched.
    {
        Commands::GoToLiftPercentage::Type cmd;
        cmd.liftPercent100thsValue = static_cast<Percent100ths>(kWcPercent100thsMaxClosed + 1);
        h.delegate.Reset();
        auto result = h.tester.Invoke<Commands::GoToLiftPercentage::Type>(cmd);
        EXPECT_FALSE(result.IsSuccess());
        auto statusCode = result.GetStatusCode();
        ASSERT_TRUE(statusCode.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(statusCode->GetStatus(), Status::ConstraintError);
        EXPECT_EQ(h.delegate.mHandleMovementCallCount, 0);
        EXPECT_EQ(h.cluster.GetTargetPositionLiftPercent100ths().Value(), kWcPercent100thsMaxClosed);
    }
}

// Symmetric coverage for the tilt axis.
TEST_F(TestWindowCoveringCluster, Command_GoToTiltPercentage)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kTilt, Feature::kPositionAwareTilt });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    {
        Commands::GoToTiltPercentage::Type cmd;
        cmd.tiltPercent100thsValue = 2500;
        h.delegate.Reset();
        auto result = h.tester.Invoke<Commands::GoToTiltPercentage::Type>(cmd);
        ASSERT_TRUE(result.IsSuccess());
        EXPECT_EQ(h.delegate.mHandleMovementCallCount, 1);
        EXPECT_EQ(h.delegate.mLastMovementType, WindowCoveringType::Tilt);
        EXPECT_EQ(h.cluster.GetTargetPositionTiltPercent100ths().Value(), 2500);
    }

    {
        Commands::GoToTiltPercentage::Type cmd;
        cmd.tiltPercent100thsValue = static_cast<Percent100ths>(kWcPercent100thsMaxClosed + 500);
        h.delegate.Reset();
        auto result = h.tester.Invoke<Commands::GoToTiltPercentage::Type>(cmd);
        EXPECT_FALSE(result.IsSuccess());
        auto statusCode = result.GetStatusCode();
        ASSERT_TRUE(statusCode.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(statusCode->GetStatus(), Status::ConstraintError);
        EXPECT_EQ(h.delegate.mHandleMovementCallCount, 0);
    }
}

// ===========================================================================
// 7. Motion-lock behavior (Mode -> ConfigStatus -> command gating)
// ===========================================================================
//
// Mode drives the operational bit of ConfigStatus, which in turn gates motion:
//   * neither maintenance nor calibration set  -> Success (motion allowed)
//   * maintenance mode                          -> Busy
//   * calibration mode                          -> Failure
// Each transition is asserted through an actual UpOrOpen invocation.
TEST_F(TestWindowCoveringCluster, MotionLock)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    // Default mode -> motion allowed.
    {
        auto result = h.tester.Invoke<Commands::UpOrOpen::Type>(Commands::UpOrOpen::Type());
        EXPECT_TRUE(result.IsSuccess());
    }

    // Maintenance mode -> Busy.
    {
        chip::BitMask<Mode> maintenance;
        maintenance.Set(Mode::kMaintenanceMode);
        h.cluster.SetMode(maintenance);

        auto result = h.tester.Invoke<Commands::UpOrOpen::Type>(Commands::UpOrOpen::Type());
        ASSERT_FALSE(result.IsSuccess());
        auto statusCode = result.GetStatusCode();
        ASSERT_TRUE(statusCode.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(statusCode->GetStatus(), Status::Busy);
    }

    // Calibration mode -> Failure.
    {
        chip::BitMask<Mode> calibration;
        calibration.Set(Mode::kCalibrationMode);
        h.cluster.SetMode(calibration);

        auto result = h.tester.Invoke<Commands::UpOrOpen::Type>(Commands::UpOrOpen::Type());
        ASSERT_FALSE(result.IsSuccess());
        auto statusCode = result.GetStatusCode();
        ASSERT_TRUE(statusCode.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(statusCode->GetStatus(), Status::Failure);
    }
}

// ===========================================================================
// 8. Delegate interaction edge cases
// ===========================================================================

// The cluster forwards a StopMotion to the delegate. When the delegate reports
// CHIP_ERROR_IN_PROGRESS it is signalling "I am still moving, do not snap the
// target to current"; the cluster must honor that by leaving the target value
// untouched. A prior GoTo establishes a known, non-current target so we can
// observe that it is preserved.
TEST_F(TestWindowCoveringCluster, Delegate_StopMotionInProgressKeepsTarget)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    Commands::GoToLiftPercentage::Type goTo;
    goTo.liftPercent100thsValue = 3000;
    ASSERT_TRUE(h.tester.Invoke<Commands::GoToLiftPercentage::Type>(goTo).IsSuccess());
    ASSERT_EQ(h.cluster.GetTargetPositionLiftPercent100ths().Value(), 3000);

    h.delegate.mHandleStopMotionResult = CHIP_ERROR_IN_PROGRESS;
    auto result                        = h.tester.Invoke<Commands::StopMotion::Type>(Commands::StopMotion::Type());
    ASSERT_TRUE(result.IsSuccess());

    // Because the delegate reported "in progress", the target is not snapped to
    // the (still null) current position and keeps its previous value.
    ASSERT_FALSE(h.cluster.GetTargetPositionLiftPercent100ths().IsNull());
    EXPECT_EQ(h.cluster.GetTargetPositionLiftPercent100ths().Value(), 3000);
}

// Documents actual (legacy-preserved) behavior: a failing HandleMovement is
// logged via LogErrorOnFailure but is NOT propagated to the command caller, and
// the target state is still committed. This test guards that contract so any
// future change to failure propagation is a conscious, reviewed decision rather
// than an accident.
TEST_F(TestWindowCoveringCluster, Delegate_MovementFailureIsNotPropagated)
{
    WindowCoveringHarness h(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift });
    ASSERT_EQ(h.Startup(), CHIP_NO_ERROR);

    h.delegate.mHandleMovementResult = CHIP_ERROR_INTERNAL;

    Commands::GoToLiftPercentage::Type cmd;
    cmd.liftPercent100thsValue = 6000;
    auto result                = h.tester.Invoke<Commands::GoToLiftPercentage::Type>(cmd);

    EXPECT_TRUE(result.IsSuccess());                                         // failure swallowed (logged only)
    EXPECT_EQ(h.delegate.mHandleMovementCallCount, 1);                       // delegate was still called
    EXPECT_EQ(h.cluster.GetTargetPositionLiftPercent100ths().Value(), 6000); // target still committed
}

// ===========================================================================
// 9. Persistence round-trip (reboot simulation)
// ===========================================================================
//
// Persistent attributes must survive a restart while RAM-only attributes must
// not. A single TestServerClusterContext (and therefore a single backing
// storage) outlives two successive cluster instances: the first writes values
// and shuts down, the second starts up against the same storage and must reload
// the persisted values. Type is RAM-only and must reset to its default.
TEST_F(TestWindowCoveringCluster, PersistenceRoundTrip)
{
    TestServerClusterContext sharedContext; // storage survives the inner cluster scopes
    MockWindowCoveringDelegate delegate;

    WindowCoveringCluster::Config config(delegate);
    config.WithFeatures(BitFlags<Feature>{ Feature::kLift, Feature::kTilt, Feature::kPositionAwareLift });

    // First boot: mutate persistent state and a RAM-only attribute, then shut down.
    {
        TestableWindowCoveringCluster cluster(kTestEndpointId, config);
        ASSERT_EQ(cluster.Startup(sharedContext.Get()), CHIP_NO_ERROR);

        cluster.SetNumberOfActuationsLift(7);
        cluster.SetNumberOfActuationsTilt(9);
        cluster.SetCurrentPositionLiftPercent100ths(NPercent100ths(2500));
        cluster.SetType(Type::kRollerShade2Motor); // RAM-only

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Second boot: a brand-new cluster against the same storage.
    {
        TestableWindowCoveringCluster cluster(kTestEndpointId, config);
        ASSERT_EQ(cluster.Startup(sharedContext.Get()), CHIP_NO_ERROR);

        EXPECT_EQ(cluster.GetNumberOfActuationsLift(), 7u);
        EXPECT_EQ(cluster.GetNumberOfActuationsTilt(), 9u);
        ASSERT_FALSE(cluster.GetCurrentPositionLiftPercent100ths().IsNull());
        EXPECT_EQ(cluster.GetCurrentPositionLiftPercent100ths().Value(), 2500);

        // Type is not persisted -> reset to its default on restart.
        EXPECT_EQ(cluster.GetType(), Type());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}
