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

#include <app/ConcreteClusterPath.h>
#include <app/clusters/zone-management-server/zone-management-server.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ZoneManagement;

void InitDataModelHandler() {}

namespace {

constexpr EndpointId kTestEndpointId   = 1;
constexpr uint8_t kMaxUserDefinedZones = 5;
constexpr uint8_t kMaxZones            = 8;
constexpr uint8_t kSensitivityMax      = 4;
const TwoDCartesianVertexStruct kTwoDMaxPoint{ 640, 480 };

class MockDelegate : public Delegate
{
public:
    Protocols::InteractionModel::Status CreateTwoDCartesianZone(const TwoDCartesianZoneStorage &, uint16_t &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status UpdateTwoDCartesianZone(uint16_t, const TwoDCartesianZoneStorage &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status RemoveZone(uint16_t) override { return Protocols::InteractionModel::Status::Success; }
    Protocols::InteractionModel::Status CreateTrigger(const ZoneTriggerControlStruct &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status UpdateTrigger(const ZoneTriggerControlStruct &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }
    Protocols::InteractionModel::Status RemoveTrigger(uint16_t) override { return Protocols::InteractionModel::Status::Success; }
    void OnAttributeChanged(AttributeId) override {}
    CHIP_ERROR PersistentAttributesLoadedCallback() override { return CHIP_NO_ERROR; }
    CHIP_ERROR LoadZones(std::vector<ZoneInformationStorage> &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR LoadTriggers(std::vector<ZoneTriggerControlStruct> &) override { return CHIP_NO_ERROR; }
};

class TestZoneManagementClusterBackwardsCompatibility : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    static ConcreteAttributePath SensitivityPath()
    {
        return ConcreteAttributePath(kTestEndpointId, ZoneManagement::Id, Attributes::Sensitivity::Id);
    }

    MockDelegate mDelegate;
};

TEST_F(TestZoneManagementClusterBackwardsCompatibility, LegacySensitivitySetBeforeAndAfterInit)
{
    ZoneMgmtServer server(mDelegate, kTestEndpointId, BitFlags<Feature>(), kMaxUserDefinedZones, kMaxZones, kSensitivityMax,
                          kTwoDMaxPoint);

    ASSERT_EQ(server.SetSensitivity(3), CHIP_NO_ERROR);
    ASSERT_EQ(server.GetSensitivity(), 3);

    ASSERT_EQ(server.Init(), CHIP_NO_ERROR);
    ASSERT_TRUE(CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(kTestEndpointId, ZoneManagement::Id)) !=
                nullptr);

    Testing::TestServerClusterContext context;
    ASSERT_EQ(server.Cluster().Startup(context.Get()), CHIP_NO_ERROR);
    ASSERT_EQ(server.GetSensitivity(), 3);
    ASSERT_EQ(server.Cluster().GetSensitivity(), 3);

    ASSERT_EQ(server.SetSensitivity(4), CHIP_NO_ERROR);
    ASSERT_EQ(server.GetSensitivity(), 4);
    ASSERT_EQ(server.Cluster().GetSensitivity(), 4);

    uint8_t persistedSensitivity = 0;
    AttributePersistence persistence(context.Get().attributeStorage);
    ASSERT_TRUE(persistence.LoadNativeEndianValue(SensitivityPath(), persistedSensitivity, static_cast<uint8_t>(0)));
    ASSERT_EQ(persistedSensitivity, 4);
}

} // namespace
