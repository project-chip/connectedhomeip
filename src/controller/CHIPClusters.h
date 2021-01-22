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

// Prevent multiple inclusion
#pragma once

#include <controller/CHIPCluster.h>
#include <core/CHIPCallback.h>

namespace chip {
namespace Controller {

constexpr ClusterId kBarrierControlClusterId         = 0x0103;
constexpr ClusterId kBasicClusterId                  = 0x0000;
constexpr ClusterId kBindingClusterId                = 0xF000;
constexpr ClusterId kColorControlClusterId           = 0x0300;
constexpr ClusterId kDoorLockClusterId               = 0x0101;
constexpr ClusterId kGroupsClusterId                 = 0x0004;
constexpr ClusterId kIasZoneClusterId                = 0x0500;
constexpr ClusterId kIdentifyClusterId               = 0x0003;
constexpr ClusterId kLevelControlClusterId           = 0x0008;
constexpr ClusterId kOnOffClusterId                  = 0x0006;
constexpr ClusterId kScenesClusterId                 = 0x0005;
constexpr ClusterId kTemperatureMeasurementClusterId = 0x0402;

class DLL_EXPORT BarrierControlCluster : public ClusterBase
{
public:
    BarrierControlCluster() : ClusterBase(kBarrierControlClusterId) {}
    ~BarrierControlCluster() {}

    // Cluster Commands
    CHIP_ERROR BarrierControlGoToPercent(Callback::Callback<> * onCompletion, uint8_t percentOpen);
    CHIP_ERROR BarrierControlStop(Callback::Callback<> * onCompletion);

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeBarrierMovingState(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeBarrierSafetyStatus(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeBarrierCapabilities(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeBarrierPosition(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Callback<> * onCompletion);
};

class DLL_EXPORT BasicCluster : public ClusterBase
{
public:
    BasicCluster() : ClusterBase(kBasicClusterId) {}
    ~BasicCluster() {}

    // Cluster Commands
    CHIP_ERROR MfgSpecificPing(Callback::Callback<> * onCompletion);
    CHIP_ERROR ResetToFactoryDefaults(Callback::Callback<> * onCompletion);

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeZclVersion(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePowerSource(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Callback<> * onCompletion);
};

class DLL_EXPORT BindingCluster : public ClusterBase
{
public:
    BindingCluster() : ClusterBase(kBindingClusterId) {}
    ~BindingCluster() {}

    // Cluster Commands
    CHIP_ERROR Bind(Callback::Callback<> * onCompletion, chip::NodeId nodeId, chip::GroupId groupId, chip::EndpointId endpointId,
                    chip::ClusterId clusterId);
    CHIP_ERROR Unbind(Callback::Callback<> * onCompletion, chip::NodeId nodeId, chip::GroupId groupId, chip::EndpointId endpointId,
                      chip::ClusterId clusterId);

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Callback<> * onCompletion);
};

class DLL_EXPORT ColorControlCluster : public ClusterBase
{
public:
    ColorControlCluster() : ClusterBase(kColorControlClusterId) {}
    ~ColorControlCluster() {}

    // Cluster Commands
    CHIP_ERROR MoveColor(Callback::Callback<> * onCompletion, int16_t rateX, int16_t rateY, uint8_t optionsMask,
                         uint8_t optionsOverride);
    CHIP_ERROR MoveColorTemperature(Callback::Callback<> * onCompletion, uint8_t moveMode, uint16_t rate,
                                    uint16_t colorTemperatureMinimum, uint16_t colorTemperatureMaximum, uint8_t optionsMask,
                                    uint8_t optionsOverride);
    CHIP_ERROR MoveHue(Callback::Callback<> * onCompletion, uint8_t moveMode, uint8_t rate, uint8_t optionsMask,
                       uint8_t optionsOverride);
    CHIP_ERROR MoveSaturation(Callback::Callback<> * onCompletion, uint8_t moveMode, uint8_t rate, uint8_t optionsMask,
                              uint8_t optionsOverride);
    CHIP_ERROR MoveToColor(Callback::Callback<> * onCompletion, uint16_t colorX, uint16_t colorY, uint16_t transitionTime,
                           uint8_t optionsMask, uint8_t optionsOverride);
    CHIP_ERROR MoveToColorTemperature(Callback::Callback<> * onCompletion, uint16_t colorTemperature, uint16_t transitionTime,
                                      uint8_t optionsMask, uint8_t optionsOverride);
    CHIP_ERROR MoveToHue(Callback::Callback<> * onCompletion, uint8_t hue, uint8_t direction, uint16_t transitionTime,
                         uint8_t optionsMask, uint8_t optionsOverride);
    CHIP_ERROR MoveToHueAndSaturation(Callback::Callback<> * onCompletion, uint8_t hue, uint8_t saturation, uint16_t transitionTime,
                                      uint8_t optionsMask, uint8_t optionsOverride);
    CHIP_ERROR MoveToSaturation(Callback::Callback<> * onCompletion, uint8_t saturation, uint16_t transitionTime,
                                uint8_t optionsMask, uint8_t optionsOverride);
    CHIP_ERROR StepColor(Callback::Callback<> * onCompletion, int16_t stepX, int16_t stepY, uint16_t transitionTime,
                         uint8_t optionsMask, uint8_t optionsOverride);
    CHIP_ERROR StepColorTemperature(Callback::Callback<> * onCompletion, uint8_t stepMode, uint16_t stepSize,
                                    uint16_t transitionTime, uint16_t colorTemperatureMinimum, uint16_t colorTemperatureMaximum,
                                    uint8_t optionsMask, uint8_t optionsOverride);
    CHIP_ERROR StepHue(Callback::Callback<> * onCompletion, uint8_t stepMode, uint8_t stepSize, uint8_t transitionTime,
                       uint8_t optionsMask, uint8_t optionsOverride);
    CHIP_ERROR StepSaturation(Callback::Callback<> * onCompletion, uint8_t stepMode, uint8_t stepSize, uint8_t transitionTime,
                              uint8_t optionsMask, uint8_t optionsOverride);
    CHIP_ERROR StopMoveStep(Callback::Callback<> * onCompletion, uint8_t optionsMask, uint8_t optionsOverride);

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeCurrentHue(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeCurrentSaturation(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeRemainingTime(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeCurrentX(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeCurrentY(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeDriftCompensation(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeCompensationText(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorTemperature(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorMode(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorControlOptions(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeNumberOfPrimaries(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary1X(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary1Y(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary1Intensity(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary2X(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary2Y(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary2Intensity(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary3X(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary3Y(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary3Intensity(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary4X(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary4Y(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary4Intensity(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary5X(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary5Y(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary5Intensity(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary6X(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary6Y(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributePrimary6Intensity(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeWhitePointX(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeWhitePointY(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorPointRX(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorPointRY(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorPointRIntensity(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorPointGX(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorPointGY(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorPointGIntensity(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorPointBX(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorPointBY(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorPointBIntensity(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeEnhancedCurrentHue(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeEnhancedColorMode(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorLoopActive(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorLoopDirection(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorLoopTime(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorCapabilities(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorTempPhysicalMin(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeColorTempPhysicalMax(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeCoupleColorTempToLevelMinMireds(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeStartUpColorTemperatureMireds(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Callback<> * onCompletion);
    CHIP_ERROR WriteAttributeColorControlOptions(Callback::Callback<> * onCompletion, uint8_t value);
    CHIP_ERROR WriteAttributeWhitePointX(Callback::Callback<> * onCompletion, uint16_t value);
    CHIP_ERROR WriteAttributeWhitePointY(Callback::Callback<> * onCompletion, uint16_t value);
    CHIP_ERROR WriteAttributeColorPointRX(Callback::Callback<> * onCompletion, uint16_t value);
    CHIP_ERROR WriteAttributeColorPointRY(Callback::Callback<> * onCompletion, uint16_t value);
    CHIP_ERROR WriteAttributeColorPointRIntensity(Callback::Callback<> * onCompletion, uint8_t value);
    CHIP_ERROR WriteAttributeColorPointGX(Callback::Callback<> * onCompletion, uint16_t value);
    CHIP_ERROR WriteAttributeColorPointGY(Callback::Callback<> * onCompletion, uint16_t value);
    CHIP_ERROR WriteAttributeColorPointGIntensity(Callback::Callback<> * onCompletion, uint8_t value);
    CHIP_ERROR WriteAttributeColorPointBX(Callback::Callback<> * onCompletion, uint16_t value);
    CHIP_ERROR WriteAttributeColorPointBY(Callback::Callback<> * onCompletion, uint16_t value);
    CHIP_ERROR WriteAttributeColorPointBIntensity(Callback::Callback<> * onCompletion, uint8_t value);
    CHIP_ERROR WriteAttributeStartUpColorTemperatureMireds(Callback::Callback<> * onCompletion, uint16_t value);
    CHIP_ERROR ReportAttributeCurrentHue(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange, uint16_t minInterval,
                                         uint16_t maxInterval, uint8_t change);
    CHIP_ERROR ReportAttributeCurrentSaturation(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange,
                                                uint16_t minInterval, uint16_t maxInterval, uint8_t change);
    CHIP_ERROR ReportAttributeCurrentX(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange, uint16_t minInterval,
                                       uint16_t maxInterval, uint16_t change);
    CHIP_ERROR ReportAttributeCurrentY(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange, uint16_t minInterval,
                                       uint16_t maxInterval, uint16_t change);
    CHIP_ERROR ReportAttributeColorTemperature(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange,
                                               uint16_t minInterval, uint16_t maxInterval, uint16_t change);
};

class DLL_EXPORT DoorLockCluster : public ClusterBase
{
public:
    DoorLockCluster() : ClusterBase(kDoorLockClusterId) {}
    ~DoorLockCluster() {}

    // Cluster Commands
    CHIP_ERROR ClearAllPins(Callback::Callback<> * onCompletion);
    CHIP_ERROR ClearAllRfids(Callback::Callback<> * onCompletion);
    CHIP_ERROR ClearHolidaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId);
    CHIP_ERROR ClearPin(Callback::Callback<> * onCompletion, uint16_t userId);
    CHIP_ERROR ClearRfid(Callback::Callback<> * onCompletion, uint16_t userId);
    CHIP_ERROR ClearWeekdaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint16_t userId);
    CHIP_ERROR ClearYeardaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint16_t userId);
    CHIP_ERROR GetHolidaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId);
    CHIP_ERROR GetLogRecord(Callback::Callback<> * onCompletion, uint16_t logIndex);
    CHIP_ERROR GetPin(Callback::Callback<> * onCompletion, uint16_t userId);
    CHIP_ERROR GetRfid(Callback::Callback<> * onCompletion, uint16_t userId);
    CHIP_ERROR GetUserType(Callback::Callback<> * onCompletion, uint16_t userId);
    CHIP_ERROR GetWeekdaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint16_t userId);
    CHIP_ERROR GetYeardaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint16_t userId);
    CHIP_ERROR LockDoor(Callback::Callback<> * onCompletion, char * pin);
    CHIP_ERROR SetHolidaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint32_t localStartTime,
                                  uint32_t localEndTime, uint8_t operatingModeDuringHoliday);
    CHIP_ERROR SetPin(Callback::Callback<> * onCompletion, uint16_t userId, uint8_t userStatus, uint8_t userType, char * pin);
    CHIP_ERROR SetRfid(Callback::Callback<> * onCompletion, uint16_t userId, uint8_t userStatus, uint8_t userType, char * id);
    CHIP_ERROR SetUserType(Callback::Callback<> * onCompletion, uint16_t userId, uint8_t userType);
    CHIP_ERROR SetWeekdaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint16_t userId, uint8_t daysMask,
                                  uint8_t startHour, uint8_t startMinute, uint8_t endHour, uint8_t endMinute);
    CHIP_ERROR SetYeardaySchedule(Callback::Callback<> * onCompletion, uint8_t scheduleId, uint16_t userId, uint32_t localStartTime,
                                  uint32_t localEndTime);
    CHIP_ERROR UnlockDoor(Callback::Callback<> * onCompletion, char * pin);
    CHIP_ERROR UnlockWithTimeout(Callback::Callback<> * onCompletion, uint16_t timeoutInSeconds, char * pin);

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeLockState(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeLockType(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeActuatorEnabled(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReportAttributeLockState(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange, uint16_t minInterval,
                                        uint16_t maxInterval);
};

class DLL_EXPORT GroupsCluster : public ClusterBase
{
public:
    GroupsCluster() : ClusterBase(kGroupsClusterId) {}
    ~GroupsCluster() {}

    // Cluster Commands
    CHIP_ERROR AddGroup(Callback::Callback<> * onCompletion, uint16_t groupId, char * groupName);
    CHIP_ERROR AddGroupIfIdentifying(Callback::Callback<> * onCompletion, uint16_t groupId, char * groupName);
    CHIP_ERROR GetGroupMembership(Callback::Callback<> * onCompletion, uint8_t groupCount, uint16_t groupList);
    CHIP_ERROR RemoveAllGroups(Callback::Callback<> * onCompletion);
    CHIP_ERROR RemoveGroup(Callback::Callback<> * onCompletion, uint16_t groupId);
    CHIP_ERROR ViewGroup(Callback::Callback<> * onCompletion, uint16_t groupId);

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeNameSupport(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Callback<> * onCompletion);
};

class DLL_EXPORT IasZoneCluster : public ClusterBase
{
public:
    IasZoneCluster() : ClusterBase(kIasZoneClusterId) {}
    ~IasZoneCluster() {}

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeZoneState(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeZoneType(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeZoneStatus(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeIasCieAddress(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeZoneId(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Callback<> * onCompletion);
    CHIP_ERROR WriteAttributeIasCieAddress(Callback::Callback<> * onCompletion, uint64_t value);
};

class DLL_EXPORT IdentifyCluster : public ClusterBase
{
public:
    IdentifyCluster() : ClusterBase(kIdentifyClusterId) {}
    ~IdentifyCluster() {}

    // Cluster Commands
    CHIP_ERROR Identify(Callback::Callback<> * onCompletion, uint16_t identifyTime);
    CHIP_ERROR IdentifyQuery(Callback::Callback<> * onCompletion);

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeIdentifyTime(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Callback<> * onCompletion);
    CHIP_ERROR WriteAttributeIdentifyTime(Callback::Callback<> * onCompletion, uint16_t value);
};

class DLL_EXPORT LevelControlCluster : public ClusterBase
{
public:
    LevelControlCluster() : ClusterBase(kLevelControlClusterId) {}
    ~LevelControlCluster() {}

    // Cluster Commands
    CHIP_ERROR Move(Callback::Callback<> * onCompletion, uint8_t moveMode, uint8_t rate, uint8_t optionMask,
                    uint8_t optionOverride);
    CHIP_ERROR MoveToLevel(Callback::Callback<> * onCompletion, uint8_t level, uint16_t transitionTime, uint8_t optionMask,
                           uint8_t optionOverride);
    CHIP_ERROR MoveToLevelWithOnOff(Callback::Callback<> * onCompletion, uint8_t level, uint16_t transitionTime);
    CHIP_ERROR MoveWithOnOff(Callback::Callback<> * onCompletion, uint8_t moveMode, uint8_t rate);
    CHIP_ERROR Step(Callback::Callback<> * onCompletion, uint8_t stepMode, uint8_t stepSize, uint16_t transitionTime,
                    uint8_t optionMask, uint8_t optionOverride);
    CHIP_ERROR StepWithOnOff(Callback::Callback<> * onCompletion, uint8_t stepMode, uint8_t stepSize, uint16_t transitionTime);
    CHIP_ERROR Stop(Callback::Callback<> * onCompletion, uint8_t optionMask, uint8_t optionOverride);
    CHIP_ERROR StopWithOnOff(Callback::Callback<> * onCompletion);

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeCurrentLevel(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReportAttributeCurrentLevel(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange,
                                           uint16_t minInterval, uint16_t maxInterval, uint8_t change);
};

class DLL_EXPORT OnOffCluster : public ClusterBase
{
public:
    OnOffCluster() : ClusterBase(kOnOffClusterId) {}
    ~OnOffCluster() {}

    // Cluster Commands
    CHIP_ERROR Off(Callback::Callback<> * onCompletion);
    CHIP_ERROR On(Callback::Callback<> * onCompletion);
    CHIP_ERROR Toggle(Callback::Callback<> * onCompletion);

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeOnOff(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReportAttributeOnOff(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange, uint16_t minInterval,
                                    uint16_t maxInterval);
};

class DLL_EXPORT ScenesCluster : public ClusterBase
{
public:
    ScenesCluster() : ClusterBase(kScenesClusterId) {}
    ~ScenesCluster() {}

    // Cluster Commands
    CHIP_ERROR AddScene(Callback::Callback<> * onCompletion, uint16_t groupId, uint8_t sceneId, uint16_t transitionTime,
                        char * sceneName, chip::ClusterId clusterId, uint8_t length, uint8_t value);
    CHIP_ERROR GetSceneMembership(Callback::Callback<> * onCompletion, uint16_t groupId);
    CHIP_ERROR RecallScene(Callback::Callback<> * onCompletion, uint16_t groupId, uint8_t sceneId, uint16_t transitionTime);
    CHIP_ERROR RemoveAllScenes(Callback::Callback<> * onCompletion, uint16_t groupId);
    CHIP_ERROR RemoveScene(Callback::Callback<> * onCompletion, uint16_t groupId, uint8_t sceneId);
    CHIP_ERROR StoreScene(Callback::Callback<> * onCompletion, uint16_t groupId, uint8_t sceneId);
    CHIP_ERROR ViewScene(Callback::Callback<> * onCompletion, uint16_t groupId, uint8_t sceneId);

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeSceneCount(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeCurrentScene(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeCurrentGroup(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeSceneValid(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeNameSupport(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Callback<> * onCompletion);
};

class DLL_EXPORT TemperatureMeasurementCluster : public ClusterBase
{
public:
    TemperatureMeasurementCluster() : ClusterBase(kTemperatureMeasurementClusterId) {}
    ~TemperatureMeasurementCluster() {}

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeMeasuredValue(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeMinMeasuredValue(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeMaxMeasuredValue(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Callback<> * onCompletion);
    CHIP_ERROR ReportAttributeMeasuredValue(Callback::Callback<> * onCompletion, Callback::Callback<> * onChange,
                                            uint16_t minInterval, uint16_t maxInterval, int16_t change);
};

} // namespace Controller
} // namespace chip
