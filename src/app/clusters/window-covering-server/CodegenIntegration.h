/**
 *
 *    Copyright (c) 2023-2025 Project CHIP Authors
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

#pragma once

#include <app/clusters/window-covering-server/WindowCoveringCluster.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

// Declare Position Limit Status
enum class LimitStatus : uint8_t
{
    Intermediate      = 0x00,
    IsUpOrOpen        = 0x01,
    IsDownOrClose     = 0x02,
    Inverted          = 0x03,
    IsPastUpOrOpen    = 0x04,
    IsPastDownOrClose = 0x05,
};
static_assert(sizeof(LimitStatus) == sizeof(uint8_t), "LimitStatus Size is not correct");

struct AbsoluteLimits
{
    uint16_t open;
    uint16_t closed;
};

/**
 * @brief Returns the WindowCoveringCluster instance registered on the given endpoint.
 *
 * @param[in] endpointId The endpoint to look up.
 * @return Pointer to the cluster instance, or nullptr if no WindowCovering cluster is registered on this endpoint.
 */
WindowCoveringCluster * FindClusterOnEndpoint(EndpointId endpointId);

/**
 * @brief Sets the application-provided delegate for the WindowCovering cluster on the given endpoint.
 *
 * The cluster instance must already be initialized on the endpoint (i.e. after MatterWindowCoveringClusterInitCallback
 * has run). If no cluster is found, a log error is emitted and the call is ignored.
 *
 * @param[in] endpointId The endpoint whose delegate should be set.
 * @param[in] delegate   The application delegate to use for hardware interaction.
 */
void SetDefaultDelegate(EndpointId endpointId, WindowCoveringDelegate * delegate);

// Helper functions used by example apps and CodegenIntegration.
chip::BitMask<Mode> ModeGet(chip::EndpointId endpoint);

OperationalState ComputeOperationalState(uint16_t target, uint16_t current);
OperationalState ComputeOperationalState(NPercent100ths target, NPercent100ths current);

LimitStatus CheckLimitState(uint16_t position, AbsoluteLimits limits);
uint16_t ConvertValue(uint16_t inputLowValue, uint16_t inputHighValue, uint16_t outputLowValue, uint16_t outputHighValue,
                      uint16_t value);
uint16_t Percent100thsToValue(AbsoluteLimits limits, Percent100ths relative);

Percent100ths ComputePercent100thsStep(OperationalState direction, Percent100ths previous, Percent100ths delta);

// These functions are only kept for backwards compatibility with example apps and should not be used by new code.
void TypeSet(chip::EndpointId endpoint, Type type);
Type TypeGet(chip::EndpointId endpoint);

void ConfigStatusPrint(const chip::BitMask<ConfigStatus> & configStatus);
void ConfigStatusSet(chip::EndpointId endpoint, const chip::BitMask<ConfigStatus> & status);
chip::BitMask<ConfigStatus> ConfigStatusGet(chip::EndpointId endpoint);
void ConfigStatusUpdateFeatures(chip::EndpointId endpoint);

void OperationalStatusPrint(const chip::BitMask<OperationalStatus> & opStatus);
void OperationalStatusSet(chip::EndpointId endpoint, chip::BitMask<OperationalStatus> newStatus);
chip::BitMask<OperationalStatus> OperationalStatusGet(chip::EndpointId endpoint);
void OperationalStateSet(chip::EndpointId endpoint, const chip::BitMask<OperationalStatus> field, OperationalState state);
OperationalState OperationalStateGet(chip::EndpointId endpoint, const chip::BitMask<OperationalStatus> field);

void EndProductTypeSet(chip::EndpointId endpoint, EndProductType type);
EndProductType EndProductTypeGet(chip::EndpointId endpoint);

void ModePrint(const chip::BitMask<Mode> & mode);
void ModeSet(chip::EndpointId endpoint, chip::BitMask<Mode> & mode);

void LiftPositionSet(chip::EndpointId endpoint, NPercent100ths position);
void TiltPositionSet(chip::EndpointId endpoint, NPercent100ths position);

/**
 * @brief PostAttributeChange is called when an Attribute is modified.
 *
 * The method is called by MatterWindowCoveringClusterServerAttributeChangedCallback
 * to update cluster attributes values. If the application overrides MatterWindowCoveringClusterServerAttributeChangedCallback,
 * it should call the PostAttributeChange on its own.
 *
 * @param[in] endpoint
 * @param[in] attributeId
 */
void PostAttributeChange(chip::EndpointId endpoint, chip::AttributeId attributeId);

} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
