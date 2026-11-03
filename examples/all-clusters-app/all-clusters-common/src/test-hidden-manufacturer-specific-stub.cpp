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
 * Stub for the TestHiddenManufacturerSpecific cluster (0xFFF1FC21). The
 * cluster carries no real behavior; it exists so all-clusters-app
 * advertises a manufacturer-specific cluster the Python controller has
 * no codegen for, exercising the wildcard-read regression first
 * observed during SVE on TC_IDM_2_2.
 */

#include <app/util/attribute-storage.h>
#include <lib/core/DataModelTypes.h>

using namespace chip;

void MatterTestHiddenManufacturerSpecificPluginServerInitCallback() {}

void emberAfTestHiddenManufacturerSpecificClusterInitCallback(EndpointId endpoint) {}

void emberAfTestHiddenManufacturerSpecificClusterShutdownCallback(EndpointId endpoint) {}
