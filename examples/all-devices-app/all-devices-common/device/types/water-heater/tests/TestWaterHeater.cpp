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

#include <device/types/water-heater/WaterHeater.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <credentials/FabricTable.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/Types.h>
#include <lib/support/TimerDelegateMock.h>
#include <platform/CHIPDeviceLayer.h>

#include <algorithm>
#include <vector>

using namespace chip;
using namespace chip::app;

namespace {

class TestWaterHeaterDelegates : public Clusters::WaterHeaterManagement::Delegate,
                                 public Clusters::Thermostat::Delegate,
                                 public Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate,
                                 public Clusters::ModeBase::AppDelegate
{
public:
    Protocols::InteractionModel::Status HandleBoost(uint32_t, Optional<bool>, Optional<bool>, Optional<int16_t>, Optional<Percent>,
                                                    Optional<Percent>) override
    {
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status HandleCancelBoost() override { return Protocols::InteractionModel::Status::Success; }
    BitMask<Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap> GetHeaterTypes() override { return {}; }
    BitMask<Clusters::WaterHeaterManagement::WaterHeaterHeatSourceBitmap> GetHeatDemand() override { return {}; }
    uint16_t GetTankVolume() override { return 0; }
    Energy_mWh GetEstimatedHeatRequired() override { return 0; }
    Percent GetTankPercentage() override { return 0; }
    Clusters::WaterHeaterManagement::BoostStateEnum GetBoostState() override
    {
        return Clusters::WaterHeaterManagement::BoostStateEnum::kInactive;
    }

    FabricTable & GetFabricTable() const override { return mFabricTable; }
    DataModel::Nullable<Clusters::Thermostat::temperature> GetLocalTemperature() const override { return 2000; }
    Protocols::InteractionModel::Status SetLocalTemperature(DataModel::Nullable<Clusters::Thermostat::temperature>,
                                                            bool & changed) override
    {
        changed = false;
        return Protocols::InteractionModel::Status::Success;
    }
    Clusters::Thermostat::SystemModeEnum GetSystemMode() const override { return Clusters::Thermostat::SystemModeEnum::kOff; }
    Protocols::InteractionModel::Status SetSystemMode(Clusters::Thermostat::SystemModeEnum, bool & changed) override
    {
        changed = false;
        return Protocols::InteractionModel::Status::Success;
    }
    Clusters::Thermostat::ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const override
    {
        return Clusters::Thermostat::ControlSequenceOfOperationEnum::kHeatingOnly;
    }
    Protocols::InteractionModel::Status SetControlSequenceOfOperation(Clusters::Thermostat::ControlSequenceOfOperationEnum,
                                                                      bool & changed) override
    {
        changed = false;
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status GetRunningMode(Clusters::Thermostat::ThermostatRunningModeEnum & runningMode) const override
    {
        runningMode = Clusters::Thermostat::ThermostatRunningModeEnum::kOff;
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status SetRunningMode(Clusters::Thermostat::ThermostatRunningModeEnum, bool & changed) override
    {
        changed = false;
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status
    GetRunningState(BitMask<Clusters::Thermostat::RelayStateBitmap> & runningState) const override
    {
        runningState = Clusters::Thermostat::RelayStateBitmap::kHeat;
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status SetRunningState(BitMask<Clusters::Thermostat::RelayStateBitmap>, bool & changed) override
    {
        changed = false;
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status SetRemoteSensing(BitMask<Clusters::Thermostat::RemoteSensingBitmap>,
                                                         bool & changed) override
    {
        changed = false;
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status GetOccupiedHeatingSetpoint(Clusters::Thermostat::temperature & setpoint) const override
    {
        setpoint = 3000;
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status SetOccupiedHeatingSetpoint(Clusters::Thermostat::temperature, bool & changed) override
    {
        changed = false;
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status GetAbsMinHeatSetpointLimit(Clusters::Thermostat::temperature & limit) const override
    {
        limit = 0;
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status GetAbsMaxHeatSetpointLimit(Clusters::Thermostat::temperature & limit) const override
    {
        limit = 10000;
        return Protocols::InteractionModel::Status::Success;
    }

    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override
    {
        VerifyOrReturnError(modeIndex == 0, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        return CopyCharSpanToMutableCharSpan("Off"_span, label);
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

private:
    mutable FabricTable mFabricTable;
};

class WaterHeaterTestFixture : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    TimerDelegateMock mTimerDelegate;
    TestWaterHeaterDelegates mDelegates;

    WaterHeater MakeDevice()
    {
        return WaterHeater(WaterHeater::Config{
            .timerDelegate                 = mTimerDelegate,
            .diagnosticDataProvider        = DeviceLayer::GetDiagnosticDataProvider(),
            .whmFeatures                   = {},
            .waterHeaterManagementDelegate = mDelegates,
            .thermostatOptionalAttributes  = {},
            .thermostatDelegate            = mDelegates,
            .heatingDelegate               = mDelegates,
            .waterHeaterModeDelegate       = mDelegates,
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

class FailingOptionalClusterWaterHeater : public WaterHeater
{
public:
    using WaterHeater::WaterHeater;

    CHIP_ERROR registrationError             = CHIP_ERROR_INTERNAL;
    unsigned unregisterOptionalClustersCalls = 0;

protected:
    CHIP_ERROR RegisterOptionalClusters(EndpointId, CodeDrivenDataModelProvider &) override { return registrationError; }
    void UnregisterOptionalClusters(CodeDrivenDataModelProvider &) override { unregisterOptionalClustersCalls++; }
};

TEST_F(WaterHeaterTestFixture, RegistersExpectedEndpointAndClusters)
{
    auto device                      = MakeDevice();
    constexpr EndpointId kEndpointId = 1;

    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    ASSERT_EQ(device.Register(kEndpointId, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(device.GetEndpointId(), kEndpointId);

    const std::vector<ClusterId> expectedClusterIds = { Clusters::Descriptor::Id, Clusters::WaterHeaterMode::Id,
                                                        Clusters::Thermostat::Id, Clusters::WaterHeaterManagement::Id };
    auto sortedExpectedClusterIds                   = expectedClusterIds;
    std::sort(sortedExpectedClusterIds.begin(), sortedExpectedClusterIds.end());
    EXPECT_EQ(GetServerClusterIds(kEndpointId), sortedExpectedClusterIds);

    EXPECT_EQ(device.WaterHeaterManagementCluster().GetPaths()[0].mEndpointId, kEndpointId);
    EXPECT_EQ(device.ThermostatCluster().GetPaths()[0].mEndpointId, kEndpointId);
    EXPECT_EQ(device.WaterHeaterModeCluster().GetPaths()[0].mEndpointId, kEndpointId);

    ReadOnlyBufferBuilder<DataModel::EndpointEntry> endpointsBuilder;
    ASSERT_EQ(mProvider.Endpoints(endpointsBuilder), CHIP_NO_ERROR);
    auto endpoints = endpointsBuilder.TakeBuffer();
    ASSERT_EQ(endpoints.size(), 1u);
    EXPECT_EQ(endpoints[0].id, kEndpointId);

    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> deviceTypesBuilder;
    ASSERT_EQ(mProvider.DeviceTypes(kEndpointId, deviceTypesBuilder), CHIP_NO_ERROR);
    auto deviceTypes = deviceTypesBuilder.TakeBuffer();
    ASSERT_EQ(deviceTypes.size(), 1u);
    EXPECT_EQ(deviceTypes[0].deviceTypeId, Device::Type::kWaterHeater.deviceTypeId);
    EXPECT_EQ(deviceTypes[0].deviceTypeRevision, Device::Type::kWaterHeater.deviceTypeRevision);

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
}

TEST_F(WaterHeaterTestFixture, RejectsRepeatedRegistrationAndAllowsReuse)
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

TEST_F(WaterHeaterTestFixture, RollsBackRequiredClustersWhenOptionalRegistrationFails)
{
    FailingOptionalClusterWaterHeater device(WaterHeater::Config{
        .timerDelegate                 = mTimerDelegate,
        .diagnosticDataProvider        = DeviceLayer::GetDiagnosticDataProvider(),
        .whmFeatures                   = {},
        .waterHeaterManagementDelegate = mDelegates,
        .thermostatOptionalAttributes  = {},
        .thermostatDelegate            = mDelegates,
        .heatingDelegate               = mDelegates,
        .waterHeaterModeDelegate       = mDelegates,
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

TEST_F(WaterHeaterTestFixture, UnregistersCleanlyAfterProviderStartup)
{
    auto device = MakeDevice();
    ASSERT_EQ(device.Register(1, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

} // namespace
