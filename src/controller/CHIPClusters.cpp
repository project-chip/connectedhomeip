/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "CHIPClusters.h"

#include <app/chip-zcl-zpro-codec-api.h>

namespace chip {
namespace Controller {

// BarrierControl Cluster Commands
CHIP_ERROR BarrierControlCluster::BarrierControlGoToPercent(Callback::Callback<> * onCompletion, uint8_t percentOpen)
{
    System::PacketBufferHandle payload = encodeBarrierControlClusterBarrierControlGoToPercentCommand(mEndpoint, percentOpen);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR BarrierControlCluster::BarrierControlStop(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBarrierControlClusterBarrierControlStopCommand(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// BarrierControl Cluster Attributes
CHIP_ERROR BarrierControlCluster::DiscoverAttributes(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBarrierControlClusterDiscoverAttributes(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}
CHIP_ERROR BarrierControlCluster::ReadAttributeBarrierMovingState(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBarrierControlClusterReadBarrierMovingStateAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR BarrierControlCluster::ReadAttributeBarrierSafetyStatus(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBarrierControlClusterReadBarrierSafetyStatusAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR BarrierControlCluster::ReadAttributeBarrierCapabilities(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBarrierControlClusterReadBarrierCapabilitiesAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR BarrierControlCluster::ReadAttributeBarrierPosition(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBarrierControlClusterReadBarrierPositionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR BarrierControlCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBarrierControlClusterReadClusterRevisionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// Basic Cluster Commands
CHIP_ERROR BasicCluster::MfgSpecificPing(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBasicClusterMfgSpecificPingCommand(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR BasicCluster::ResetToFactoryDefaults(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBasicClusterResetToFactoryDefaultsCommand(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// Basic Cluster Attributes
CHIP_ERROR BasicCluster::DiscoverAttributes(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBasicClusterDiscoverAttributes(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}
CHIP_ERROR BasicCluster::ReadAttributeZclVersion(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBasicClusterReadZclVersionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR BasicCluster::ReadAttributePowerSource(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBasicClusterReadPowerSourceAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR BasicCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBasicClusterReadClusterRevisionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// Binding Cluster Commands
CHIP_ERROR BindingCluster::Bind(Callback::Callback<> * onCompletion, chip::NodeId nodeId, chip::GroupId groupId,
                                chip::EndpointId endpointId, chip::ClusterId clusterId)
{
    System::PacketBufferHandle payload = encodeBindingClusterBindCommand(mEndpoint, nodeId, groupId, endpointId, clusterId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR BindingCluster::Unbind(Callback::Callback<> * onCompletion, chip::NodeId nodeId, chip::GroupId groupId,
                                  chip::EndpointId endpointId, chip::ClusterId clusterId)
{
    System::PacketBufferHandle payload = encodeBindingClusterUnbindCommand(mEndpoint, nodeId, groupId, endpointId, clusterId);
    return SendCommand(std::move(payload), onCompletion);
}

// Binding Cluster Attributes
CHIP_ERROR BindingCluster::DiscoverAttributes(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBindingClusterDiscoverAttributes(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}
CHIP_ERROR BindingCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeBindingClusterReadClusterRevisionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// ColorControl Cluster Commands
CHIP_ERROR ColorControlCluster::MoveColor(Callback::Callback<> * onCompletion, int16_t rateX, int16_t rateY, uint8_t optionsMask,
                                          uint8_t optionsOverride)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterMoveColorCommand(mEndpoint, rateX, rateY, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::MoveColorTemperature(Callback::Callback<> * onCompletion, uint8_t moveMode, uint16_t rate,
                                                     uint16_t colorTemperatureMinimum, uint16_t colorTemperatureMaximum,
                                                     uint8_t optionsMask, uint8_t optionsOverride)
{
    System::PacketBufferHandle payload = encodeColorControlClusterMoveColorTemperatureCommand(
        mEndpoint, moveMode, rate, colorTemperatureMinimum, colorTemperatureMaximum, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::MoveHue(Callback::Callback<> * onCompletion, uint8_t moveMode, uint8_t rate, uint8_t optionsMask,
                                        uint8_t optionsOverride)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterMoveHueCommand(mEndpoint, moveMode, rate, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::MoveSaturation(Callback::Callback<> * onCompletion, uint8_t moveMode, uint8_t rate,
                                               uint8_t optionsMask, uint8_t optionsOverride)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterMoveSaturationCommand(mEndpoint, moveMode, rate, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::MoveToColor(Callback::Callback<> * onCompletion, uint16_t colorX, uint16_t colorY,
                                            uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterMoveToColorCommand(mEndpoint, colorX, colorY, transitionTime, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::MoveToColorTemperature(Callback::Callback<> * onCompletion, uint16_t colorTemperature,
                                                       uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    System::PacketBufferHandle payload = encodeColorControlClusterMoveToColorTemperatureCommand(
        mEndpoint, colorTemperature, transitionTime, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::MoveToHue(Callback::Callback<> * onCompletion, uint8_t hue, uint8_t direction,
                                          uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterMoveToHueCommand(mEndpoint, hue, direction, transitionTime, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::MoveToHueAndSaturation(Callback::Callback<> * onCompletion, uint8_t hue, uint8_t saturation,
                                                       uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    System::PacketBufferHandle payload = encodeColorControlClusterMoveToHueAndSaturationCommand(
        mEndpoint, hue, saturation, transitionTime, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::MoveToSaturation(Callback::Callback<> * onCompletion, uint8_t saturation, uint16_t transitionTime,
                                                 uint8_t optionsMask, uint8_t optionsOverride)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterMoveToSaturationCommand(mEndpoint, saturation, transitionTime, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::StepColor(Callback::Callback<> * onCompletion, int16_t stepX, int16_t stepY,
                                          uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterStepColorCommand(mEndpoint, stepX, stepY, transitionTime, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::StepColorTemperature(Callback::Callback<> * onCompletion, uint8_t stepMode, uint16_t stepSize,
                                                     uint16_t transitionTime, uint16_t colorTemperatureMinimum,
                                                     uint16_t colorTemperatureMaximum, uint8_t optionsMask, uint8_t optionsOverride)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterStepColorTemperatureCommand(mEndpoint, stepMode, stepSize, transitionTime, colorTemperatureMinimum,
                                                             colorTemperatureMaximum, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::StepHue(Callback::Callback<> * onCompletion, uint8_t stepMode, uint8_t stepSize,
                                        uint8_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterStepHueCommand(mEndpoint, stepMode, stepSize, transitionTime, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::StepSaturation(Callback::Callback<> * onCompletion, uint8_t stepMode, uint8_t stepSize,
                                               uint8_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterStepSaturationCommand(mEndpoint, stepMode, stepSize, transitionTime, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::StopMoveStep(Callback::Callback<> * onCompletion, uint8_t optionsMask, uint8_t optionsOverride)
{
    System::PacketBufferHandle payload = encodeColorControlClusterStopMoveStepCommand(mEndpoint, optionsMask, optionsOverride);
    return SendCommand(std::move(payload), onCompletion);
}

// ColorControl Cluster Attributes
CHIP_ERROR ColorControlCluster::DiscoverAttributes(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterDiscoverAttributes(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}
CHIP_ERROR ColorControlCluster::ReadAttributeCurrentHue(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadCurrentHueAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReportAttributeCurrentHue(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange,
                                                          uint16_t minInterval, uint16_t maxInterval, uint8_t change)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterReportCurrentHueAttribute(mEndpoint, minInterval, maxInterval, change);
    return RequestAttributeReporting(std::move(payload), onCompletion, onChange);
}

CHIP_ERROR ColorControlCluster::ReadAttributeCurrentSaturation(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadCurrentSaturationAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReportAttributeCurrentSaturation(Callback::Callback<> * onCompletion,
                                                                 Callback::Callback<> * onChange, uint16_t minInterval,
                                                                 uint16_t maxInterval, uint8_t change)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterReportCurrentSaturationAttribute(mEndpoint, minInterval, maxInterval, change);
    return RequestAttributeReporting(std::move(payload), onCompletion, onChange);
}

CHIP_ERROR ColorControlCluster::ReadAttributeRemainingTime(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadRemainingTimeAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeCurrentX(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadCurrentXAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReportAttributeCurrentX(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange,
                                                        uint16_t minInterval, uint16_t maxInterval, uint16_t change)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterReportCurrentXAttribute(mEndpoint, minInterval, maxInterval, change);
    return RequestAttributeReporting(std::move(payload), onCompletion, onChange);
}

CHIP_ERROR ColorControlCluster::ReadAttributeCurrentY(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadCurrentYAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReportAttributeCurrentY(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange,
                                                        uint16_t minInterval, uint16_t maxInterval, uint16_t change)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterReportCurrentYAttribute(mEndpoint, minInterval, maxInterval, change);
    return RequestAttributeReporting(std::move(payload), onCompletion, onChange);
}

CHIP_ERROR ColorControlCluster::ReadAttributeDriftCompensation(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadDriftCompensationAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeCompensationText(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadCompensationTextAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorTemperature(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorTemperatureAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReportAttributeColorTemperature(Callback::Callback<> * onCompletion,
                                                                Callback::Callback<> * onChange, uint16_t minInterval,
                                                                uint16_t maxInterval, uint16_t change)
{
    System::PacketBufferHandle payload =
        encodeColorControlClusterReportColorTemperatureAttribute(mEndpoint, minInterval, maxInterval, change);
    return RequestAttributeReporting(std::move(payload), onCompletion, onChange);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorMode(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorModeAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorControlOptions(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorControlOptionsAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::WriteAttributeColorControlOptions(Callback::Callback<> * onCompletion, uint8_t value)
{
    System::PacketBufferHandle payload = encodeColorControlClusterWriteColorControlOptionsAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeNumberOfPrimaries(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadNumberOfPrimariesAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary1X(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary1XAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary1Y(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary1YAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary1Intensity(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary1IntensityAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary2X(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary2XAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary2Y(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary2YAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary2Intensity(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary2IntensityAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary3X(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary3XAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary3Y(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary3YAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary3Intensity(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary3IntensityAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary4X(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary4XAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary4Y(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary4YAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary4Intensity(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary4IntensityAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary5X(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary5XAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary5Y(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary5YAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary5Intensity(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary5IntensityAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary6X(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary6XAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary6Y(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary6YAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributePrimary6Intensity(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadPrimary6IntensityAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeWhitePointX(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadWhitePointXAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::WriteAttributeWhitePointX(Callback::Callback<> * onCompletion, uint16_t value)
{
    System::PacketBufferHandle payload = encodeColorControlClusterWriteWhitePointXAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeWhitePointY(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadWhitePointYAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::WriteAttributeWhitePointY(Callback::Callback<> * onCompletion, uint16_t value)
{
    System::PacketBufferHandle payload = encodeColorControlClusterWriteWhitePointYAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorPointRX(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorPointRXAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::WriteAttributeColorPointRX(Callback::Callback<> * onCompletion, uint16_t value)
{
    System::PacketBufferHandle payload = encodeColorControlClusterWriteColorPointRXAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorPointRY(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorPointRYAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::WriteAttributeColorPointRY(Callback::Callback<> * onCompletion, uint16_t value)
{
    System::PacketBufferHandle payload = encodeColorControlClusterWriteColorPointRYAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorPointRIntensity(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorPointRIntensityAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::WriteAttributeColorPointRIntensity(Callback::Callback<> * onCompletion, uint8_t value)
{
    System::PacketBufferHandle payload = encodeColorControlClusterWriteColorPointRIntensityAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorPointGX(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorPointGXAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::WriteAttributeColorPointGX(Callback::Callback<> * onCompletion, uint16_t value)
{
    System::PacketBufferHandle payload = encodeColorControlClusterWriteColorPointGXAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorPointGY(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorPointGYAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::WriteAttributeColorPointGY(Callback::Callback<> * onCompletion, uint16_t value)
{
    System::PacketBufferHandle payload = encodeColorControlClusterWriteColorPointGYAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorPointGIntensity(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorPointGIntensityAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::WriteAttributeColorPointGIntensity(Callback::Callback<> * onCompletion, uint8_t value)
{
    System::PacketBufferHandle payload = encodeColorControlClusterWriteColorPointGIntensityAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorPointBX(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorPointBXAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::WriteAttributeColorPointBX(Callback::Callback<> * onCompletion, uint16_t value)
{
    System::PacketBufferHandle payload = encodeColorControlClusterWriteColorPointBXAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorPointBY(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorPointBYAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::WriteAttributeColorPointBY(Callback::Callback<> * onCompletion, uint16_t value)
{
    System::PacketBufferHandle payload = encodeColorControlClusterWriteColorPointBYAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorPointBIntensity(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorPointBIntensityAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::WriteAttributeColorPointBIntensity(Callback::Callback<> * onCompletion, uint8_t value)
{
    System::PacketBufferHandle payload = encodeColorControlClusterWriteColorPointBIntensityAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeEnhancedCurrentHue(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadEnhancedCurrentHueAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeEnhancedColorMode(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadEnhancedColorModeAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorLoopActive(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorLoopActiveAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorLoopDirection(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorLoopDirectionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorLoopTime(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorLoopTimeAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorCapabilities(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorCapabilitiesAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorTempPhysicalMin(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorTempPhysicalMinAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeColorTempPhysicalMax(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadColorTempPhysicalMaxAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeCoupleColorTempToLevelMinMireds(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadCoupleColorTempToLevelMinMiredsAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeStartUpColorTemperatureMireds(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadStartUpColorTemperatureMiredsAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::WriteAttributeStartUpColorTemperatureMireds(Callback::Callback<> * onCompletion, uint16_t value)
{
    System::PacketBufferHandle payload = encodeColorControlClusterWriteStartUpColorTemperatureMiredsAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ColorControlCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeColorControlClusterReadClusterRevisionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// DoorLock Cluster Commands
CHIP_ERROR DoorLockCluster::ClearAllPins(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterClearAllPinsCommand(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::ClearAllRfids(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterClearAllRfidsCommand(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::ClearHolidaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterClearHolidayScheduleCommand(mEndpoint, scheduleId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::ClearPin(Callback::Callback<> * onCompletion, uint16_t userId)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterClearPinCommand(mEndpoint, userId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::ClearRfid(Callback::Callback<> * onCompletion, uint16_t userId)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterClearRfidCommand(mEndpoint, userId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::ClearWeekdaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint16_t userId)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterClearWeekdayScheduleCommand(mEndpoint, scheduleId, userId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::ClearYeardaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint16_t userId)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterClearYeardayScheduleCommand(mEndpoint, scheduleId, userId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::GetHolidaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterGetHolidayScheduleCommand(mEndpoint, scheduleId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::GetLogRecord(Callback::Callback<> * onCompletion, uint16_t logIndex)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterGetLogRecordCommand(mEndpoint, logIndex);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::GetPin(Callback::Callback<> * onCompletion, uint16_t userId)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterGetPinCommand(mEndpoint, userId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::GetRfid(Callback::Callback<> * onCompletion, uint16_t userId)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterGetRfidCommand(mEndpoint, userId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::GetUserType(Callback::Callback<> * onCompletion, uint16_t userId)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterGetUserTypeCommand(mEndpoint, userId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::GetWeekdaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint16_t userId)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterGetWeekdayScheduleCommand(mEndpoint, scheduleId, userId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::GetYeardaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint16_t userId)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterGetYeardayScheduleCommand(mEndpoint, scheduleId, userId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::LockDoor(Callback::Callback<> * onCompletion, char * pin)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterLockDoorCommand(mEndpoint, pin);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::SetHolidaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint32_t localStartTime,
                                               uint32_t localEndTime, uint8_t operatingModeDuringHoliday)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterSetHolidayScheduleCommand(mEndpoint, scheduleId, localStartTime,
                                                                                        localEndTime, operatingModeDuringHoliday);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::SetPin(Callback::Callback<> * onCompletion, uint16_t userId, uint8_t userStatus, uint8_t userType,
                                   char * pin)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterSetPinCommand(mEndpoint, userId, userStatus, userType, pin);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::SetRfid(Callback::Callback<> * onCompletion, uint16_t userId, uint8_t userStatus, uint8_t userType,
                                    char * id)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterSetRfidCommand(mEndpoint, userId, userStatus, userType, id);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::SetUserType(Callback::Callback<> * onCompletion, uint16_t userId, uint8_t userType)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterSetUserTypeCommand(mEndpoint, userId, userType);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::SetWeekdaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint16_t userId,
                                               uint8_t daysMask, uint8_t startHour, uint8_t startMinute, uint8_t endHour,
                                               uint8_t endMinute)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterSetWeekdayScheduleCommand(mEndpoint, scheduleId, userId, daysMask,
                                                                                        startHour, startMinute, endHour, endMinute);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::SetYeardaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint16_t userId,
                                               uint32_t localStartTime, uint32_t localEndTime)
{
    System::PacketBufferHandle payload =
        encodeDoorLockClusterSetYeardayScheduleCommand(mEndpoint, scheduleId, userId, localStartTime, localEndTime);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::UnlockDoor(Callback::Callback<> * onCompletion, char * pin)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterUnlockDoorCommand(mEndpoint, pin);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::UnlockWithTimeout(Callback::Callback<> * onCompletion, uint16_t timeoutInSeconds, char * pin)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterUnlockWithTimeoutCommand(mEndpoint, timeoutInSeconds, pin);
    return SendCommand(std::move(payload), onCompletion);
}

// DoorLock Cluster Attributes
CHIP_ERROR DoorLockCluster::DiscoverAttributes(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterDiscoverAttributes(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}
CHIP_ERROR DoorLockCluster::ReadAttributeLockState(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterReadLockStateAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::ReportAttributeLockState(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange,
                                                     uint16_t minInterval, uint16_t maxInterval)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterReportLockStateAttribute(mEndpoint, minInterval, maxInterval);
    return RequestAttributeReporting(std::move(payload), onCompletion, onChange);
}

CHIP_ERROR DoorLockCluster::ReadAttributeLockType(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterReadLockTypeAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::ReadAttributeActuatorEnabled(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterReadActuatorEnabledAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR DoorLockCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeDoorLockClusterReadClusterRevisionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// Groups Cluster Commands
CHIP_ERROR GroupsCluster::AddGroup(Callback::Callback<> * onCompletion, uint16_t groupId, char * groupName)
{
    System::PacketBufferHandle payload = encodeGroupsClusterAddGroupCommand(mEndpoint, groupId, groupName);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR GroupsCluster::AddGroupIfIdentifying(Callback::Callback<> * onCompletion, uint16_t groupId, char * groupName)
{
    System::PacketBufferHandle payload = encodeGroupsClusterAddGroupIfIdentifyingCommand(mEndpoint, groupId, groupName);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR GroupsCluster::GetGroupMembership(Callback::Callback<> * onCompletion, uint8_t groupCount, uint16_t groupList)
{
    System::PacketBufferHandle payload = encodeGroupsClusterGetGroupMembershipCommand(mEndpoint, groupCount, groupList);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR GroupsCluster::RemoveAllGroups(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeGroupsClusterRemoveAllGroupsCommand(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR GroupsCluster::RemoveGroup(Callback::Callback<> * onCompletion, uint16_t groupId)
{
    System::PacketBufferHandle payload = encodeGroupsClusterRemoveGroupCommand(mEndpoint, groupId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR GroupsCluster::ViewGroup(Callback::Callback<> * onCompletion, uint16_t groupId)
{
    System::PacketBufferHandle payload = encodeGroupsClusterViewGroupCommand(mEndpoint, groupId);
    return SendCommand(std::move(payload), onCompletion);
}

// Groups Cluster Attributes
CHIP_ERROR GroupsCluster::DiscoverAttributes(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeGroupsClusterDiscoverAttributes(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}
CHIP_ERROR GroupsCluster::ReadAttributeNameSupport(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeGroupsClusterReadNameSupportAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR GroupsCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeGroupsClusterReadClusterRevisionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// IasZone Cluster Commands
// IasZone Cluster Attributes
CHIP_ERROR IasZoneCluster::DiscoverAttributes(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeIasZoneClusterDiscoverAttributes(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}
CHIP_ERROR IasZoneCluster::ReadAttributeZoneState(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeIasZoneClusterReadZoneStateAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR IasZoneCluster::ReadAttributeZoneType(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeIasZoneClusterReadZoneTypeAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR IasZoneCluster::ReadAttributeZoneStatus(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeIasZoneClusterReadZoneStatusAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR IasZoneCluster::ReadAttributeIasCieAddress(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeIasZoneClusterReadIasCieAddressAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR IasZoneCluster::WriteAttributeIasCieAddress(Callback::Callback<> * onCompletion, uint64_t value)
{
    System::PacketBufferHandle payload = encodeIasZoneClusterWriteIasCieAddressAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR IasZoneCluster::ReadAttributeZoneId(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeIasZoneClusterReadZoneIdAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR IasZoneCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeIasZoneClusterReadClusterRevisionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// Identify Cluster Commands
CHIP_ERROR IdentifyCluster::Identify(Callback::Callback<> * onCompletion, uint16_t identifyTime)
{
    System::PacketBufferHandle payload = encodeIdentifyClusterIdentifyCommand(mEndpoint, identifyTime);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR IdentifyCluster::IdentifyQuery(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeIdentifyClusterIdentifyQueryCommand(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// Identify Cluster Attributes
CHIP_ERROR IdentifyCluster::DiscoverAttributes(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeIdentifyClusterDiscoverAttributes(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}
CHIP_ERROR IdentifyCluster::ReadAttributeIdentifyTime(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeIdentifyClusterReadIdentifyTimeAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR IdentifyCluster::WriteAttributeIdentifyTime(Callback::Callback<> * onCompletion, uint16_t value)
{
    System::PacketBufferHandle payload = encodeIdentifyClusterWriteIdentifyTimeAttribute(mEndpoint, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR IdentifyCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeIdentifyClusterReadClusterRevisionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// LevelControl Cluster Commands
CHIP_ERROR LevelControlCluster::Move(Callback::Callback<> * onCompletion, uint8_t moveMode, uint8_t rate, uint8_t optionMask,
                                     uint8_t optionOverride)
{
    System::PacketBufferHandle payload =
        encodeLevelControlClusterMoveCommand(mEndpoint, moveMode, rate, optionMask, optionOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR LevelControlCluster::MoveToLevel(Callback::Callback<> * onCompletion, uint8_t level, uint16_t transitionTime,
                                            uint8_t optionMask, uint8_t optionOverride)
{
    System::PacketBufferHandle payload =
        encodeLevelControlClusterMoveToLevelCommand(mEndpoint, level, transitionTime, optionMask, optionOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR LevelControlCluster::MoveToLevelWithOnOff(Callback::Callback<> * onCompletion, uint8_t level, uint16_t transitionTime)
{
    System::PacketBufferHandle payload = encodeLevelControlClusterMoveToLevelWithOnOffCommand(mEndpoint, level, transitionTime);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR LevelControlCluster::MoveWithOnOff(Callback::Callback<> * onCompletion, uint8_t moveMode, uint8_t rate)
{
    System::PacketBufferHandle payload = encodeLevelControlClusterMoveWithOnOffCommand(mEndpoint, moveMode, rate);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR LevelControlCluster::Step(Callback::Callback<> * onCompletion, uint8_t stepMode, uint8_t stepSize,
                                     uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride)
{
    System::PacketBufferHandle payload =
        encodeLevelControlClusterStepCommand(mEndpoint, stepMode, stepSize, transitionTime, optionMask, optionOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR LevelControlCluster::StepWithOnOff(Callback::Callback<> * onCompletion, uint8_t stepMode, uint8_t stepSize,
                                              uint16_t transitionTime)
{
    System::PacketBufferHandle payload =
        encodeLevelControlClusterStepWithOnOffCommand(mEndpoint, stepMode, stepSize, transitionTime);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR LevelControlCluster::Stop(Callback::Callback<> * onCompletion, uint8_t optionMask, uint8_t optionOverride)
{
    System::PacketBufferHandle payload = encodeLevelControlClusterStopCommand(mEndpoint, optionMask, optionOverride);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR LevelControlCluster::StopWithOnOff(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeLevelControlClusterStopWithOnOffCommand(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// LevelControl Cluster Attributes
CHIP_ERROR LevelControlCluster::DiscoverAttributes(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeLevelControlClusterDiscoverAttributes(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}
CHIP_ERROR LevelControlCluster::ReadAttributeCurrentLevel(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeLevelControlClusterReadCurrentLevelAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR LevelControlCluster::ReportAttributeCurrentLevel(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange,
                                                            uint16_t minInterval, uint16_t maxInterval, uint8_t change)
{
    System::PacketBufferHandle payload =
        encodeLevelControlClusterReportCurrentLevelAttribute(mEndpoint, minInterval, maxInterval, change);
    return RequestAttributeReporting(std::move(payload), onCompletion, onChange);
}

CHIP_ERROR LevelControlCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeLevelControlClusterReadClusterRevisionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// OnOff Cluster Commands
CHIP_ERROR OnOffCluster::Off(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeOnOffClusterOffCommand(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR OnOffCluster::On(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeOnOffClusterOnCommand(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR OnOffCluster::Toggle(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeOnOffClusterToggleCommand(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// OnOff Cluster Attributes
CHIP_ERROR OnOffCluster::DiscoverAttributes(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeOnOffClusterDiscoverAttributes(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}
CHIP_ERROR OnOffCluster::ReadAttributeOnOff(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeOnOffClusterReadOnOffAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR OnOffCluster::ReportAttributeOnOff(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange,
                                              uint16_t minInterval, uint16_t maxInterval)
{
    System::PacketBufferHandle payload = encodeOnOffClusterReportOnOffAttribute(mEndpoint, minInterval, maxInterval);
    return RequestAttributeReporting(std::move(payload), onCompletion, onChange);
}

CHIP_ERROR OnOffCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeOnOffClusterReadClusterRevisionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// Scenes Cluster Commands
CHIP_ERROR ScenesCluster::AddScene(Callback::Callback<> * onCompletion, uint16_t groupId, uint8_t sceneId, uint16_t transitionTime,
                                   char * sceneName, chip::ClusterId clusterId, uint8_t length, uint8_t value)
{
    System::PacketBufferHandle payload =
        encodeScenesClusterAddSceneCommand(mEndpoint, groupId, sceneId, transitionTime, sceneName, clusterId, length, value);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ScenesCluster::GetSceneMembership(Callback::Callback<> * onCompletion, uint16_t groupId)
{
    System::PacketBufferHandle payload = encodeScenesClusterGetSceneMembershipCommand(mEndpoint, groupId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ScenesCluster::RecallScene(Callback::Callback<> * onCompletion, uint16_t groupId, uint8_t sceneId,
                                      uint16_t transitionTime)
{
    System::PacketBufferHandle payload = encodeScenesClusterRecallSceneCommand(mEndpoint, groupId, sceneId, transitionTime);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ScenesCluster::RemoveAllScenes(Callback::Callback<> * onCompletion, uint16_t groupId)
{
    System::PacketBufferHandle payload = encodeScenesClusterRemoveAllScenesCommand(mEndpoint, groupId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ScenesCluster::RemoveScene(Callback::Callback<> * onCompletion, uint16_t groupId, uint8_t sceneId)
{
    System::PacketBufferHandle payload = encodeScenesClusterRemoveSceneCommand(mEndpoint, groupId, sceneId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ScenesCluster::StoreScene(Callback::Callback<> * onCompletion, uint16_t groupId, uint8_t sceneId)
{
    System::PacketBufferHandle payload = encodeScenesClusterStoreSceneCommand(mEndpoint, groupId, sceneId);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ScenesCluster::ViewScene(Callback::Callback<> * onCompletion, uint16_t groupId, uint8_t sceneId)
{
    System::PacketBufferHandle payload = encodeScenesClusterViewSceneCommand(mEndpoint, groupId, sceneId);
    return SendCommand(std::move(payload), onCompletion);
}

// Scenes Cluster Attributes
CHIP_ERROR ScenesCluster::DiscoverAttributes(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeScenesClusterDiscoverAttributes(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}
CHIP_ERROR ScenesCluster::ReadAttributeSceneCount(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeScenesClusterReadSceneCountAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ScenesCluster::ReadAttributeCurrentScene(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeScenesClusterReadCurrentSceneAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ScenesCluster::ReadAttributeCurrentGroup(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeScenesClusterReadCurrentGroupAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ScenesCluster::ReadAttributeSceneValid(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeScenesClusterReadSceneValidAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ScenesCluster::ReadAttributeNameSupport(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeScenesClusterReadNameSupportAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR ScenesCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeScenesClusterReadClusterRevisionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

// TemperatureMeasurement Cluster Commands
// TemperatureMeasurement Cluster Attributes
CHIP_ERROR TemperatureMeasurementCluster::DiscoverAttributes(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeTemperatureMeasurementClusterDiscoverAttributes(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}
CHIP_ERROR TemperatureMeasurementCluster::ReadAttributeMeasuredValue(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeTemperatureMeasurementClusterReadMeasuredValueAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR TemperatureMeasurementCluster::ReportAttributeMeasuredValue(Callback::Callback<> * onCompletion,
                                                                       Callback::Callback<> * onChange, uint16_t minInterval,
                                                                       uint16_t maxInterval, int16_t change)
{
    System::PacketBufferHandle payload =
        encodeTemperatureMeasurementClusterReportMeasuredValueAttribute(mEndpoint, minInterval, maxInterval, change);
    return RequestAttributeReporting(std::move(payload), onCompletion, onChange);
}

CHIP_ERROR TemperatureMeasurementCluster::ReadAttributeMinMeasuredValue(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeTemperatureMeasurementClusterReadMinMeasuredValueAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR TemperatureMeasurementCluster::ReadAttributeMaxMeasuredValue(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeTemperatureMeasurementClusterReadMaxMeasuredValueAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

CHIP_ERROR TemperatureMeasurementCluster::ReadAttributeClusterRevision(Callback::Callback<> * onCompletion)
{
    System::PacketBufferHandle payload = encodeTemperatureMeasurementClusterReadClusterRevisionAttribute(mEndpoint);
    return SendCommand(std::move(payload), onCompletion);
}

} // namespace Controller
} // namespace chip
