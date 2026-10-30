/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/types/energy-evse/EnergyEvse.h>
#include <device/types/energy-evse/impl/SimulatedEnergyEvse.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Descriptor/ClusterId.h>
#include <clusters/EnergyEvse/ClusterId.h>
#include <clusters/EnergyEvseMode/ClusterId.h>
#include <clusters/Identify/ClusterId.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/Types.h>
#include <lib/support/TimerDelegateMock.h>
#include <platform/CHIPDeviceLayer.h>

#include <algorithm>
#include <vector>

using namespace chip;
using namespace chip::app;

namespace {

class TestEvseDelegates : public Clusters::EnergyEvse::Delegate, public Clusters::ModeBase::AppDelegate
{
public:
    // Clusters::EnergyEvse::Delegate
    Protocols::InteractionModel::Status Disable() override { return Protocols::InteractionModel::Status::Success; }
    Protocols::InteractionModel::Status EnableCharging(const DataModel::Nullable<uint32_t> &, const int64_t &,
                                                       const int64_t &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status EnableDischarging(const DataModel::Nullable<uint32_t> &, const int64_t &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status StartDiagnostics() override { return Protocols::InteractionModel::Status::Success; }
    Protocols::InteractionModel::Status
    SetTargets(const DataModel::DecodableList<Clusters::EnergyEvse::Structs::ChargingTargetScheduleStruct::DecodableType> &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status LoadTargets() override { return Protocols::InteractionModel::Status::Success; }
    Protocols::InteractionModel::Status
    GetTargets(DataModel::List<const Clusters::EnergyEvse::Structs::ChargingTargetScheduleStruct::Type> &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status ClearTargets() override { return Protocols::InteractionModel::Status::Success; }

    void OnStateChanged(Clusters::EnergyEvse::StateEnum) override {}
    void OnSupplyStateChanged(Clusters::EnergyEvse::SupplyStateEnum) override {}
    void OnFaultStateChanged(Clusters::EnergyEvse::FaultStateEnum) override {}
    void OnChargingEnabledUntilChanged(DataModel::Nullable<uint32_t>) override {}
    void OnDischargingEnabledUntilChanged(DataModel::Nullable<uint32_t>) override {}
    void OnCircuitCapacityChanged(int64_t) override {}
    void OnMinimumChargeCurrentChanged(int64_t) override {}
    void OnMaximumChargeCurrentChanged(int64_t) override {}
    void OnMaximumDischargeCurrentChanged(int64_t) override {}
    void OnUserMaximumChargeCurrentChanged(int64_t) override {}
    void OnRandomizationDelayWindowChanged(uint32_t) override {}
    void OnNextChargeStartTimeChanged(DataModel::Nullable<uint32_t>) override {}
    void OnNextChargeTargetTimeChanged(DataModel::Nullable<uint32_t>) override {}
    void OnNextChargeRequiredEnergyChanged(DataModel::Nullable<int64_t>) override {}
    void OnNextChargeTargetSoCChanged(DataModel::Nullable<Percent>) override {}
    void OnApproximateEVEfficiencyChanged(DataModel::Nullable<uint16_t>) override {}
    void OnStateOfChargeChanged(DataModel::Nullable<Percent>) override {}
    void OnBatteryCapacityChanged(DataModel::Nullable<int64_t>) override {}
    void OnVehicleIDChanged(DataModel::Nullable<CharSpan>) override {}
    void OnSessionIDChanged(DataModel::Nullable<uint32_t>) override {}
    void OnSessionDurationChanged(DataModel::Nullable<uint32_t>) override {}
    void OnSessionEnergyChargedChanged(DataModel::Nullable<int64_t>) override {}
    void OnSessionEnergyDischargedChanged(DataModel::Nullable<int64_t>) override {}

    // Clusters::ModeBase::AppDelegate
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override
    {
        VerifyOrReturnError(modeIndex == 0, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        return CopyCharSpanToMutableCharSpan("Manual"_span, label);
    }
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override
    {
        VerifyOrReturnError(modeIndex == 0, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        value = 0;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex,
                                  DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) override
    {
        VerifyOrReturnError(modeIndex == 0, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        VerifyOrReturnError(modeTags.size() > 0, CHIP_ERROR_INVALID_ARGUMENT);
        modeTags[0].value = 0;
        modeTags.reduce_size(1);
        return CHIP_NO_ERROR;
    }
    void HandleChangeToMode(uint8_t, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override
    {
        response.status = to_underlying(Clusters::ModeBase::StatusCode::kSuccess);
    }
};

class EnergyEvseTestFixture : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    TimerDelegateMock mTimerDelegate;
    TestEvseDelegates mDelegates;

    EnergyEvse MakeDevice()
    {
        return EnergyEvse(EnergyEvse::Config{
            .timerDelegate          = mTimerDelegate,
            .diagnosticDataProvider = DeviceLayer::GetDiagnosticDataProvider(),
            .evseDelegate           = mDelegates,
            .evseModeDelegate       = mDelegates,
            .evseFeatures           = BitMask<Clusters::EnergyEvse::Feature>(Clusters::EnergyEvse::Feature::kChargingPreferences),
            .evseOptionalCmds       = {},
            .evseOptionalAttrs      = {},
        });
    }

    std::vector<ClusterId> GetServerClusterIds(EndpointId endpoint)
    {
        ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> builder;
        EXPECT_EQ(mProvider.ServerClusters(endpoint, builder), CHIP_NO_ERROR);
        auto clusters = builder.TakeBuffer();
        std::vector<ClusterId> ids;
        for (const auto & entry : clusters)
        {
            ids.push_back(entry.clusterId);
        }
        std::sort(ids.begin(), ids.end());
        return ids;
    }
};

class FailingOptionalClusterEnergyEvse : public EnergyEvse
{
public:
    using EnergyEvse::EnergyEvse;

    CHIP_ERROR registrationError             = CHIP_ERROR_INTERNAL;
    unsigned unregisterOptionalClustersCalls = 0;

protected:
    CHIP_ERROR RegisterOptionalClusters(EndpointId, CodeDrivenDataModelProvider &) override { return registrationError; }
    void UnregisterOptionalClusters(CodeDrivenDataModelProvider &) override { unregisterOptionalClustersCalls++; }
};

TEST_F(EnergyEvseTestFixture, RegistersExpectedEndpointAndClusters)
{
    auto device                      = MakeDevice();
    constexpr EndpointId kEndpointId = 1;

    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    ASSERT_EQ(device.Register(kEndpointId, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(device.GetEndpointId(), kEndpointId);

    const std::vector<ClusterId> expectedClusterIds = { Clusters::Descriptor::Id, Clusters::EnergyEvse::Id,
                                                        Clusters::EnergyEvseMode::Id };
    auto sortedExpectedClusterIds                   = expectedClusterIds;
    std::sort(sortedExpectedClusterIds.begin(), sortedExpectedClusterIds.end());
    EXPECT_EQ(GetServerClusterIds(kEndpointId), sortedExpectedClusterIds);

    EXPECT_EQ(device.EnergyEvseCluster().GetPaths()[0].mEndpointId, kEndpointId);
    EXPECT_EQ(device.EnergyEvseModeCluster().GetPaths()[0].mEndpointId, kEndpointId);

    ReadOnlyBufferBuilder<DataModel::EndpointEntry> endpointsBuilder;
    ASSERT_EQ(mProvider.Endpoints(endpointsBuilder), CHIP_NO_ERROR);
    auto endpoints = endpointsBuilder.TakeBuffer();
    ASSERT_EQ(endpoints.size(), 1u);
    EXPECT_EQ(endpoints[0].id, kEndpointId);

    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> deviceTypesBuilder;
    ASSERT_EQ(mProvider.DeviceTypes(kEndpointId, deviceTypesBuilder), CHIP_NO_ERROR);
    auto deviceTypes = deviceTypesBuilder.TakeBuffer();
    ASSERT_EQ(deviceTypes.size(), 1u);
    EXPECT_EQ(deviceTypes[0].deviceTypeId, Device::Type::kEvse.deviceTypeId);
    EXPECT_EQ(deviceTypes[0].deviceTypeRevision, Device::Type::kEvse.deviceTypeRevision);

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
}

TEST_F(EnergyEvseTestFixture, RejectsRepeatedRegistrationAndAllowsReuse)
{
    auto device = MakeDevice();
    EXPECT_EQ(device.Register(kInvalidEndpointId, mProvider), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);

    ASSERT_EQ(device.Register(1, mProvider), CHIP_NO_ERROR);

    EXPECT_EQ(device.Register(2, mProvider), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(device.GetEndpointId(), 1);
    device.Unregister(mProvider);

    ASSERT_EQ(device.Register(2, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(device.GetEndpointId(), 2);
    device.Unregister(mProvider);
}

TEST_F(EnergyEvseTestFixture, RollsBackRequiredClustersWhenOptionalRegistrationFails)
{
    FailingOptionalClusterEnergyEvse device(EnergyEvse::Config{
        .timerDelegate          = mTimerDelegate,
        .diagnosticDataProvider = DeviceLayer::GetDiagnosticDataProvider(),
        .evseDelegate           = mDelegates,
        .evseModeDelegate       = mDelegates,
        .evseFeatures           = BitMask<Clusters::EnergyEvse::Feature>(Clusters::EnergyEvse::Feature::kChargingPreferences),
        .evseOptionalCmds       = {},
        .evseOptionalAttrs      = {},
    });

    EXPECT_EQ(device.Register(1, mProvider), CHIP_ERROR_INTERNAL);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.unregisterOptionalClustersCalls, 1u);

    ReadOnlyBufferBuilder<DataModel::EndpointEntry> endpointsBuilder;
    ASSERT_EQ(mProvider.Endpoints(endpointsBuilder), CHIP_NO_ERROR);
    EXPECT_TRUE(endpointsBuilder.TakeBuffer().empty());

    device.registrationError = CHIP_NO_ERROR;
    ASSERT_EQ(device.Register(1, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(device.GetEndpointId(), 1);
    device.Unregister(mProvider);
    EXPECT_EQ(device.unregisterOptionalClustersCalls, 2u);
}

TEST_F(EnergyEvseTestFixture, UnregistersCleanlyAfterProviderStartup)
{
    auto device = MakeDevice();
    ASSERT_EQ(device.Register(1, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(EnergyEvseTestFixture, SimulatedEnergyEvseExecutesCommands)
{
    SimulatedEnergyEvse simulatedDevice({ .timerDelegate = mTimerDelegate });
    constexpr EndpointId kEndpointId = 1;

    ASSERT_EQ(simulatedDevice.Register(kEndpointId, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    // Initial state check
    EXPECT_EQ(simulatedDevice.EnergyEvseCluster().GetSupplyState(), Clusters::EnergyEvse::SupplyStateEnum::kDisabled);
    EXPECT_EQ(simulatedDevice.EnergyEvseCluster().GetFaultState(), Clusters::EnergyEvse::FaultStateEnum::kNoError);

    // Test EnableCharging command execution via simulated delegate
    constexpr int64_t kMinCurrent = 6000;
    constexpr int64_t kMaxCurrent = 32000;
    DataModel::Nullable<uint32_t> chargeUntil(123456);

    auto & evseCluster = simulatedDevice.EnergyEvseCluster();
    EXPECT_EQ(evseCluster.SetFaultState(Clusters::EnergyEvse::FaultStateEnum::kNoError), CHIP_NO_ERROR);

    // Enable charging through cluster/delegate
    EXPECT_EQ(simulatedDevice.EnergyEvseCluster().SetSupplyState(Clusters::EnergyEvse::SupplyStateEnum::kChargingEnabled),
              CHIP_NO_ERROR);
    EXPECT_EQ(simulatedDevice.EnergyEvseCluster().SetChargingEnabledUntil(chargeUntil), CHIP_NO_ERROR);
    EXPECT_EQ(simulatedDevice.EnergyEvseCluster().SetMinimumChargeCurrent(kMinCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(simulatedDevice.EnergyEvseCluster().SetMaximumChargeCurrent(kMaxCurrent), CHIP_NO_ERROR);

    EXPECT_EQ(evseCluster.GetSupplyState(), Clusters::EnergyEvse::SupplyStateEnum::kChargingEnabled);
    EXPECT_EQ(evseCluster.GetChargingEnabledUntil(), chargeUntil);
    EXPECT_EQ(evseCluster.GetMinimumChargeCurrent(), kMinCurrent);
    EXPECT_EQ(evseCluster.GetMaximumChargeCurrent(), kMaxCurrent);

    // Check optional Identify cluster is registered on simulated device
    const std::vector<ClusterId> expectedClusterIds = { Clusters::Descriptor::Id, Clusters::EnergyEvse::Id,
                                                        Clusters::EnergyEvseMode::Id, Clusters::Identify::Id };
    auto sortedExpectedClusterIds                   = expectedClusterIds;
    std::sort(sortedExpectedClusterIds.begin(), sortedExpectedClusterIds.end());
    EXPECT_EQ(GetServerClusterIds(kEndpointId), sortedExpectedClusterIds);

    simulatedDevice.Unregister(mProvider);
    EXPECT_EQ(simulatedDevice.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

} // namespace

