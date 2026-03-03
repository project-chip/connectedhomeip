/*
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

/**
 * @file test-hidden-manufacturer-specific-stub.cpp
 *
 * Stub implementation for the TestHiddenManufacturerSpecific cluster.
 * This cluster is intentionally NOT in controller-clusters.matter to test
 * how the test harness handles MEI clusters that the controller has no
 * knowledge of (simulating real-world vendor-specific clusters).
 *
 * Cluster ID: 0xFFF1FC21 (4294048801)
 */

#include <app/util/attribute-storage.h>
#include <lib/core/DataModelTypes.h>

using namespace chip;

// Plugin server init callback - called when the plugin is initialized
void MatterTestHiddenManufacturerSpecificPluginServerInitCallback()
{
    // No plugin initialization required for this test cluster
}

// Cluster init callback - called when cluster is initialized on an endpoint
void emberAfTestHiddenManufacturerSpecificClusterInitCallback(EndpointId endpoint)
{
    // No initialization required for this test cluster
}

// Cluster shutdown callback - called when cluster is shut down on an endpoint
void emberAfTestHiddenManufacturerSpecificClusterShutdownCallback(EndpointId endpoint)
{
    // No shutdown actions required for this test cluster
}
