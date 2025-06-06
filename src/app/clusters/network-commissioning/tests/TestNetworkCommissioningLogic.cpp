/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/network-commissioning/NetworkCommissioningLogic.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/GeneralCommissioning/Attributes.h>
#include <clusters/NetworkCommissioning/Commands.h>
#include <clusters/NetworkCommissioning/Enums.h>
#include <clusters/NetworkCommissioning/Ids.h>
#include <clusters/NetworkCommissioning/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/NetworkCommissioning.h>

#include "FakeWifiDriver.h"

namespace {

using namespace chip;
using namespace chip::app::Clusters;

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestNetworkCommissioningLogic : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestNetworkCommissioningLogic, TestFeatures)
{
    Testing::FakeWiFiDriver fakeWifiDriver;
    NetworkCommissioningLogic logic(kRootEndpointId, &fakeWifiDriver);
    ASSERT_EQ(logic.Features(), BitFlags<NetworkCommissioning::Feature>(NetworkCommissioning::Feature::kWiFiNetworkInterface));
}

} // namespace
