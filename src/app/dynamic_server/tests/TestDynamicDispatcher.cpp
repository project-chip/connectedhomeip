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

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/dynamic_server/DynamicDispatcher.h>
#include <app/util/attribute-storage-detail.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <pw_unit_test/framework.h>

namespace {

constexpr chip::EndpointId kRequestorEndpoint = chip::app::dynamic_server::kWebRTCRequestorDynamicEndpointId;
constexpr chip::ClusterId kRequestorCluster   = chip::app::Clusters::WebRTCTransportRequestor::Id;

TEST(TestDynamicDispatcher, WebRTCRequestorEndpointTracksEnablement)
{
    chip::app::dynamic_server::SetWebRTCRequestorEndpointEnabled(false);
    const unsigned initialGeneration = emberAfMetadataStructureGeneration();

    EXPECT_FALSE(emberAfEndpointIndexIsEnabled(1));
    EXPECT_EQ(emberAfFindEndpointType(kRequestorEndpoint), nullptr);
    EXPECT_EQ(emberAfIndexFromEndpoint(kRequestorEndpoint), UINT16_MAX);

    chip::app::dynamic_server::SetWebRTCRequestorEndpointEnabled(true);

    EXPECT_EQ(emberAfMetadataStructureGeneration(), initialGeneration + 1);
    EXPECT_TRUE(emberAfEndpointIndexIsEnabled(1));
    EXPECT_EQ(emberAfEndpointFromIndex(1), kRequestorEndpoint);
    EXPECT_EQ(emberAfIndexFromEndpoint(kRequestorEndpoint), 1);
    EXPECT_EQ(emberAfParentEndpointFromIndex(1), 0);
    EXPECT_NE(emberAfFindEndpointType(kRequestorEndpoint), nullptr);
    EXPECT_NE(emberAfFindServerCluster(kRequestorEndpoint, kRequestorCluster), nullptr);
    const auto clusterId = emberAfGetNthClusterId(kRequestorEndpoint, 0, true);
    ASSERT_TRUE(clusterId.HasValue());
    EXPECT_EQ(clusterId.Value(), kRequestorCluster);

    chip::app::dynamic_server::SetWebRTCRequestorEndpointEnabled(false);

    EXPECT_EQ(emberAfMetadataStructureGeneration(), initialGeneration + 2);
    EXPECT_FALSE(emberAfEndpointIndexIsEnabled(1));
    EXPECT_EQ(emberAfFindEndpointType(kRequestorEndpoint), nullptr);
    EXPECT_EQ(emberAfFindServerCluster(kRequestorEndpoint, kRequestorCluster), nullptr);
}

} // namespace
