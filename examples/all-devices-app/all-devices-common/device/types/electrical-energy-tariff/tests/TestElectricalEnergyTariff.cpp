/*
 *
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

#include <device/types/electrical-energy-tariff/ElectricalEnergyTariff.h>
#include <device/types/electrical-energy-tariff/impl/SimulatedElectricalEnergyTariff.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/CommodityPrice/Attributes.h>
#include <clusters/CommodityPrice/ClusterId.h>
#include <clusters/Descriptor/ClusterId.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/Types.h>

#include <algorithm>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::CommodityPrice::Attributes;
using chip::Testing::ClusterTester;

namespace {

constexpr EndpointId kEndpointId = 1;

class ElectricalEnergyTariffTestFixture : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };

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

    void ExpectElectricalEnergyTariffDeviceType(EndpointId endpoint)
    {
        ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> builder;
        ASSERT_EQ(mProvider.DeviceTypes(endpoint, builder), CHIP_NO_ERROR);
        auto deviceTypes = builder.TakeBuffer();
        ASSERT_EQ(deviceTypes.size(), 1u);
        EXPECT_EQ(deviceTypes[0].deviceTypeId, Device::Type::kElectricalEnergyTariff.deviceTypeId);
        EXPECT_EQ(deviceTypes[0].deviceTypeRevision, Device::Type::kElectricalEnergyTariff.deviceTypeRevision);
    }

    bool HasNoEndpoints()
    {
        ReadOnlyBufferBuilder<DataModel::EndpointEntry> builder;
        EXPECT_EQ(mProvider.Endpoints(builder), CHIP_NO_ERROR);
        return builder.TakeBuffer().empty();
    }
};

class FailingOptionalClusterElectricalEnergyTariff : public ElectricalEnergyTariff
{
public:
    CHIP_ERROR registrationError             = CHIP_ERROR_INTERNAL;
    unsigned unregisterOptionalClustersCalls = 0;

protected:
    CHIP_ERROR RegisterOptionalClusters(EndpointId, CodeDrivenDataModelProvider &) override { return registrationError; }
    void UnregisterOptionalClusters(CodeDrivenDataModelProvider &) override { unregisterOptionalClustersCalls++; }
};

TEST_F(ElectricalEnergyTariffTestFixture, GenericTypeRegistersOnlyDescriptor)
{
    ElectricalEnergyTariff device;

    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    ASSERT_EQ(device.Register(kEndpointId, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(device.GetEndpointId(), kEndpointId);

    EXPECT_EQ(GetServerClusterIds(kEndpointId), std::vector<ClusterId>{ Clusters::Descriptor::Id });
    ExpectElectricalEnergyTariffDeviceType(kEndpointId);

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_TRUE(HasNoEndpoints());
}

TEST_F(ElectricalEnergyTariffTestFixture, RejectsRepeatedRegistrationAndAllowsReuse)
{
    ElectricalEnergyTariff device;
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

TEST_F(ElectricalEnergyTariffTestFixture, RollsBackWhenOptionalRegistrationFails)
{
    FailingOptionalClusterElectricalEnergyTariff device;

    EXPECT_EQ(device.Register(kEndpointId, mProvider), CHIP_ERROR_INTERNAL);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.unregisterOptionalClustersCalls, 1u);
    EXPECT_TRUE(HasNoEndpoints());

    device.registrationError = CHIP_NO_ERROR;
    ASSERT_EQ(device.Register(kEndpointId, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(device.GetEndpointId(), kEndpointId);
    device.Unregister(mProvider);
    EXPECT_EQ(device.unregisterOptionalClustersCalls, 2u);
}

TEST_F(ElectricalEnergyTariffTestFixture, SimulatedRegistersCommodityPrice)
{
    SimulatedElectricalEnergyTariff device;
    ASSERT_EQ(device.Register(kEndpointId, mProvider), CHIP_NO_ERROR);

    std::vector<ClusterId> expectedClusterIds = { Clusters::Descriptor::Id, Clusters::CommodityPrice::Id };
    std::sort(expectedClusterIds.begin(), expectedClusterIds.end());
    EXPECT_EQ(GetServerClusterIds(kEndpointId), expectedClusterIds);
    ExpectElectricalEnergyTariffDeviceType(kEndpointId);
    EXPECT_EQ(device.CommodityPriceCluster().GetPaths()[0].mEndpointId, kEndpointId);

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_TRUE(HasNoEndpoints());
}

TEST_F(ElectricalEnergyTariffTestFixture, SimulatedSetsInitialPrice)
{
    SimulatedElectricalEnergyTariff device;
    ASSERT_EQ(device.Register(kEndpointId, mProvider), CHIP_NO_ERROR);

    ClusterTester tester{ device.CommodityPriceCluster() };

    TariffUnit::TypeInfo::DecodableType tariffUnit = Clusters::Globals::TariffUnitEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(TariffUnit::Id, tariffUnit), CHIP_NO_ERROR);
    EXPECT_EQ(tariffUnit, Clusters::Globals::TariffUnitEnum::kKWh);

    Currency::TypeInfo::DecodableType currency;
    ASSERT_EQ(tester.ReadAttribute(Currency::Id, currency), CHIP_NO_ERROR);
    ASSERT_FALSE(currency.IsNull());
    EXPECT_EQ(currency.Value().currency, SimulatedElectricalEnergyTariff::kCurrencyEur);
    EXPECT_EQ(currency.Value().decimalPoints, SimulatedElectricalEnergyTariff::kCurrencyDecimalPoints);

    CurrentPrice::TypeInfo::DecodableType currentPrice;
    ASSERT_EQ(tester.ReadAttribute(CurrentPrice::Id, currentPrice), CHIP_NO_ERROR);
    ASSERT_FALSE(currentPrice.IsNull());
    EXPECT_EQ(currentPrice.Value().periodStart, 0u);
    EXPECT_TRUE(currentPrice.Value().periodEnd.IsNull());
    ASSERT_TRUE(currentPrice.Value().price.HasValue());
    EXPECT_EQ(currentPrice.Value().price.Value(), SimulatedElectricalEnergyTariff::kPricePerKWh);

    device.Unregister(mProvider);
}

TEST_F(ElectricalEnergyTariffTestFixture, SimulatedUnregistersCleanlyAfterProviderStartup)
{
    SimulatedElectricalEnergyTariff device;
    ASSERT_EQ(device.Register(kEndpointId, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

} // namespace
