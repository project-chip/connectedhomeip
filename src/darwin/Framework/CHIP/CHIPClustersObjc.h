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

#ifndef CHIP_CLUSTERS_H
#define CHIP_CLUSTERS_H

#import <CHIP/CHIPDeviceCallback.h>
#import <Foundation/Foundation.h>

@class CHIPDevice;

NS_ASSUME_NONNULL_BEGIN

@interface CHIPBarrierControl : NSObject

- (nullable instancetype)initWithDevice:(CHIPDevice *)device endpoint:(uint8_t)endpoint queue:(dispatch_queue_t)queue;
- (BOOL)barrierControlGoToPercent:(CHIPDeviceCallback)onCompletion percentOpen:(uint8_t)percentOpen;
- (BOOL)barrierControlStop:(CHIPDeviceCallback)onCompletion;

- (BOOL)readAttributeBarrierMovingState:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeBarrierSafetyStatus:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeBarrierCapabilities:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeBarrierPosition:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeClusterRevision:(CHIPDeviceCallback)onCompletion;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

NS_ASSUME_NONNULL_BEGIN

@interface CHIPBasic : NSObject

- (nullable instancetype)initWithDevice:(CHIPDevice *)device endpoint:(uint8_t)endpoint queue:(dispatch_queue_t)queue;
- (BOOL)mfgSpecificPing:(CHIPDeviceCallback)onCompletion;
- (BOOL)resetToFactoryDefaults:(CHIPDeviceCallback)onCompletion;

- (BOOL)readAttributeZclVersion:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePowerSource:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeClusterRevision:(CHIPDeviceCallback)onCompletion;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

NS_ASSUME_NONNULL_BEGIN

@interface CHIPBinding : NSObject

- (nullable instancetype)initWithDevice:(CHIPDevice *)device endpoint:(uint8_t)endpoint queue:(dispatch_queue_t)queue;
- (BOOL)bind:(CHIPDeviceCallback)onCompletion
        nodeId:(uint64_t)nodeId
       groupId:(uint16_t)groupId
    endpointId:(uint8_t)endpointId
     clusterId:(uint16_t)clusterId;
- (BOOL)unbind:(CHIPDeviceCallback)onCompletion
        nodeId:(uint64_t)nodeId
       groupId:(uint16_t)groupId
    endpointId:(uint8_t)endpointId
     clusterId:(uint16_t)clusterId;

- (BOOL)readAttributeClusterRevision:(CHIPDeviceCallback)onCompletion;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

NS_ASSUME_NONNULL_BEGIN

@interface CHIPColorControl : NSObject

- (nullable instancetype)initWithDevice:(CHIPDevice *)device endpoint:(uint8_t)endpoint queue:(dispatch_queue_t)queue;
- (BOOL)moveColor:(CHIPDeviceCallback)onCompletion
              rateX:(int16_t)rateX
              rateY:(int16_t)rateY
        optionsMask:(uint8_t)optionsMask
    optionsOverride:(uint8_t)optionsOverride;
- (BOOL)moveColorTemperature:(CHIPDeviceCallback)onCompletion
                    moveMode:(uint8_t)moveMode
                        rate:(uint16_t)rate
     colorTemperatureMinimum:(uint16_t)colorTemperatureMinimum
     colorTemperatureMaximum:(uint16_t)colorTemperatureMaximum
                 optionsMask:(uint8_t)optionsMask
             optionsOverride:(uint8_t)optionsOverride;
- (BOOL)moveHue:(CHIPDeviceCallback)onCompletion
           moveMode:(uint8_t)moveMode
               rate:(uint8_t)rate
        optionsMask:(uint8_t)optionsMask
    optionsOverride:(uint8_t)optionsOverride;
- (BOOL)moveSaturation:(CHIPDeviceCallback)onCompletion
              moveMode:(uint8_t)moveMode
                  rate:(uint8_t)rate
           optionsMask:(uint8_t)optionsMask
       optionsOverride:(uint8_t)optionsOverride;
- (BOOL)moveToColor:(CHIPDeviceCallback)onCompletion
             colorX:(uint16_t)colorX
             colorY:(uint16_t)colorY
     transitionTime:(uint16_t)transitionTime
        optionsMask:(uint8_t)optionsMask
    optionsOverride:(uint8_t)optionsOverride;
- (BOOL)moveToColorTemperature:(CHIPDeviceCallback)onCompletion
              colorTemperature:(uint16_t)colorTemperature
                transitionTime:(uint16_t)transitionTime
                   optionsMask:(uint8_t)optionsMask
               optionsOverride:(uint8_t)optionsOverride;
- (BOOL)moveToHue:(CHIPDeviceCallback)onCompletion
                hue:(uint8_t)hue
          direction:(uint8_t)direction
     transitionTime:(uint16_t)transitionTime
        optionsMask:(uint8_t)optionsMask
    optionsOverride:(uint8_t)optionsOverride;
- (BOOL)moveToHueAndSaturation:(CHIPDeviceCallback)onCompletion
                           hue:(uint8_t)hue
                    saturation:(uint8_t)saturation
                transitionTime:(uint16_t)transitionTime
                   optionsMask:(uint8_t)optionsMask
               optionsOverride:(uint8_t)optionsOverride;
- (BOOL)moveToSaturation:(CHIPDeviceCallback)onCompletion
              saturation:(uint8_t)saturation
          transitionTime:(uint16_t)transitionTime
             optionsMask:(uint8_t)optionsMask
         optionsOverride:(uint8_t)optionsOverride;
- (BOOL)stepColor:(CHIPDeviceCallback)onCompletion
              stepX:(int16_t)stepX
              stepY:(int16_t)stepY
     transitionTime:(uint16_t)transitionTime
        optionsMask:(uint8_t)optionsMask
    optionsOverride:(uint8_t)optionsOverride;
- (BOOL)stepColorTemperature:(CHIPDeviceCallback)onCompletion
                    stepMode:(uint8_t)stepMode
                    stepSize:(uint16_t)stepSize
              transitionTime:(uint16_t)transitionTime
     colorTemperatureMinimum:(uint16_t)colorTemperatureMinimum
     colorTemperatureMaximum:(uint16_t)colorTemperatureMaximum
                 optionsMask:(uint8_t)optionsMask
             optionsOverride:(uint8_t)optionsOverride;
- (BOOL)stepHue:(CHIPDeviceCallback)onCompletion
           stepMode:(uint8_t)stepMode
           stepSize:(uint8_t)stepSize
     transitionTime:(uint8_t)transitionTime
        optionsMask:(uint8_t)optionsMask
    optionsOverride:(uint8_t)optionsOverride;
- (BOOL)stepSaturation:(CHIPDeviceCallback)onCompletion
              stepMode:(uint8_t)stepMode
              stepSize:(uint8_t)stepSize
        transitionTime:(uint8_t)transitionTime
           optionsMask:(uint8_t)optionsMask
       optionsOverride:(uint8_t)optionsOverride;
- (BOOL)stopMoveStep:(CHIPDeviceCallback)onCompletion optionsMask:(uint8_t)optionsMask optionsOverride:(uint8_t)optionsOverride;

- (BOOL)readAttributeCurrentHue:(CHIPDeviceCallback)onCompletion;
- (BOOL)reportAttributeCurrentHue:(CHIPDeviceCallback)onCompletion
                         onChange:(CHIPDeviceCallback)onChange
                      minInterval:(uint16_t)minInterval
                      maxInterval:(uint16_t)maxInterval
                           change:(uint8_t)change;
- (BOOL)readAttributeCurrentSaturation:(CHIPDeviceCallback)onCompletion;
- (BOOL)reportAttributeCurrentSaturation:(CHIPDeviceCallback)onCompletion
                                onChange:(CHIPDeviceCallback)onChange
                             minInterval:(uint16_t)minInterval
                             maxInterval:(uint16_t)maxInterval
                                  change:(uint8_t)change;
- (BOOL)readAttributeRemainingTime:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeCurrentX:(CHIPDeviceCallback)onCompletion;
- (BOOL)reportAttributeCurrentX:(CHIPDeviceCallback)onCompletion
                       onChange:(CHIPDeviceCallback)onChange
                    minInterval:(uint16_t)minInterval
                    maxInterval:(uint16_t)maxInterval
                         change:(uint16_t)change;
- (BOOL)readAttributeCurrentY:(CHIPDeviceCallback)onCompletion;
- (BOOL)reportAttributeCurrentY:(CHIPDeviceCallback)onCompletion
                       onChange:(CHIPDeviceCallback)onChange
                    minInterval:(uint16_t)minInterval
                    maxInterval:(uint16_t)maxInterval
                         change:(uint16_t)change;
- (BOOL)readAttributeDriftCompensation:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeCompensationText:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeColorTemperature:(CHIPDeviceCallback)onCompletion;
- (BOOL)reportAttributeColorTemperature:(CHIPDeviceCallback)onCompletion
                               onChange:(CHIPDeviceCallback)onChange
                            minInterval:(uint16_t)minInterval
                            maxInterval:(uint16_t)maxInterval
                                 change:(uint16_t)change;
- (BOOL)readAttributeColorMode:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeColorControlOptions:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeColorControlOptions:(CHIPDeviceCallback)onCompletion value:(uint8_t)value;
- (BOOL)readAttributeNumberOfPrimaries:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary1X:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary1Y:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary1Intensity:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary2X:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary2Y:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary2Intensity:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary3X:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary3Y:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary3Intensity:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary4X:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary4Y:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary4Intensity:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary5X:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary5Y:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary5Intensity:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary6X:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary6Y:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributePrimary6Intensity:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeWhitePointX:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeWhitePointX:(CHIPDeviceCallback)onCompletion value:(uint16_t)value;
- (BOOL)readAttributeWhitePointY:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeWhitePointY:(CHIPDeviceCallback)onCompletion value:(uint16_t)value;
- (BOOL)readAttributeColorPointRX:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeColorPointRX:(CHIPDeviceCallback)onCompletion value:(uint16_t)value;
- (BOOL)readAttributeColorPointRY:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeColorPointRY:(CHIPDeviceCallback)onCompletion value:(uint16_t)value;
- (BOOL)readAttributeColorPointRIntensity:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeColorPointRIntensity:(CHIPDeviceCallback)onCompletion value:(uint8_t)value;
- (BOOL)readAttributeColorPointGX:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeColorPointGX:(CHIPDeviceCallback)onCompletion value:(uint16_t)value;
- (BOOL)readAttributeColorPointGY:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeColorPointGY:(CHIPDeviceCallback)onCompletion value:(uint16_t)value;
- (BOOL)readAttributeColorPointGIntensity:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeColorPointGIntensity:(CHIPDeviceCallback)onCompletion value:(uint8_t)value;
- (BOOL)readAttributeColorPointBX:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeColorPointBX:(CHIPDeviceCallback)onCompletion value:(uint16_t)value;
- (BOOL)readAttributeColorPointBY:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeColorPointBY:(CHIPDeviceCallback)onCompletion value:(uint16_t)value;
- (BOOL)readAttributeColorPointBIntensity:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeColorPointBIntensity:(CHIPDeviceCallback)onCompletion value:(uint8_t)value;
- (BOOL)readAttributeEnhancedCurrentHue:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeEnhancedColorMode:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeColorLoopActive:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeColorLoopDirection:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeColorLoopTime:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeColorCapabilities:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeColorTempPhysicalMin:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeColorTempPhysicalMax:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeCoupleColorTempToLevelMinMireds:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeStartUpColorTemperatureMireds:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeStartUpColorTemperatureMireds:(CHIPDeviceCallback)onCompletion value:(uint16_t)value;
- (BOOL)readAttributeClusterRevision:(CHIPDeviceCallback)onCompletion;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

NS_ASSUME_NONNULL_BEGIN

@interface CHIPDoorLock : NSObject

- (nullable instancetype)initWithDevice:(CHIPDevice *)device endpoint:(uint8_t)endpoint queue:(dispatch_queue_t)queue;
- (BOOL)clearAllPins:(CHIPDeviceCallback)onCompletion;
- (BOOL)clearAllRfids:(CHIPDeviceCallback)onCompletion;
- (BOOL)clearHolidaySchedule:(CHIPDeviceCallback)onCompletion scheduleId:(uint8_t)scheduleId;
- (BOOL)clearPin:(CHIPDeviceCallback)onCompletion userId:(uint16_t)userId;
- (BOOL)clearRfid:(CHIPDeviceCallback)onCompletion userId:(uint16_t)userId;
- (BOOL)clearWeekdaySchedule:(CHIPDeviceCallback)onCompletion scheduleId:(uint8_t)scheduleId userId:(uint16_t)userId;
- (BOOL)clearYeardaySchedule:(CHIPDeviceCallback)onCompletion scheduleId:(uint8_t)scheduleId userId:(uint16_t)userId;
- (BOOL)getHolidaySchedule:(CHIPDeviceCallback)onCompletion scheduleId:(uint8_t)scheduleId;
- (BOOL)getLogRecord:(CHIPDeviceCallback)onCompletion logIndex:(uint16_t)logIndex;
- (BOOL)getPin:(CHIPDeviceCallback)onCompletion userId:(uint16_t)userId;
- (BOOL)getRfid:(CHIPDeviceCallback)onCompletion userId:(uint16_t)userId;
- (BOOL)getUserType:(CHIPDeviceCallback)onCompletion userId:(uint16_t)userId;
- (BOOL)getWeekdaySchedule:(CHIPDeviceCallback)onCompletion scheduleId:(uint8_t)scheduleId userId:(uint16_t)userId;
- (BOOL)getYeardaySchedule:(CHIPDeviceCallback)onCompletion scheduleId:(uint8_t)scheduleId userId:(uint16_t)userId;
- (BOOL)lockDoor:(CHIPDeviceCallback)onCompletion pin:(char *)pin;
- (BOOL)setHolidaySchedule:(CHIPDeviceCallback)onCompletion
                    scheduleId:(uint8_t)scheduleId
                localStartTime:(uint32_t)localStartTime
                  localEndTime:(uint32_t)localEndTime
    operatingModeDuringHoliday:(uint8_t)operatingModeDuringHoliday;
- (BOOL)setPin:(CHIPDeviceCallback)onCompletion
        userId:(uint16_t)userId
    userStatus:(uint8_t)userStatus
      userType:(uint8_t)userType
           pin:(char *)pin;
- (BOOL)setRfid:(CHIPDeviceCallback)onCompletion
         userId:(uint16_t)userId
     userStatus:(uint8_t)userStatus
       userType:(uint8_t)userType
             id:(char *)id;
- (BOOL)setUserType:(CHIPDeviceCallback)onCompletion userId:(uint16_t)userId userType:(uint8_t)userType;
- (BOOL)setWeekdaySchedule:(CHIPDeviceCallback)onCompletion
                scheduleId:(uint8_t)scheduleId
                    userId:(uint16_t)userId
                  daysMask:(uint8_t)daysMask
                 startHour:(uint8_t)startHour
               startMinute:(uint8_t)startMinute
                   endHour:(uint8_t)endHour
                 endMinute:(uint8_t)endMinute;
- (BOOL)setYeardaySchedule:(CHIPDeviceCallback)onCompletion
                scheduleId:(uint8_t)scheduleId
                    userId:(uint16_t)userId
            localStartTime:(uint32_t)localStartTime
              localEndTime:(uint32_t)localEndTime;
- (BOOL)unlockDoor:(CHIPDeviceCallback)onCompletion pin:(char *)pin;
- (BOOL)unlockWithTimeout:(CHIPDeviceCallback)onCompletion timeoutInSeconds:(uint16_t)timeoutInSeconds pin:(char *)pin;

- (BOOL)readAttributeLockState:(CHIPDeviceCallback)onCompletion;
- (BOOL)reportAttributeLockState:(CHIPDeviceCallback)onCompletion
                        onChange:(CHIPDeviceCallback)onChange
                     minInterval:(uint16_t)minInterval
                     maxInterval:(uint16_t)maxInterval;
- (BOOL)readAttributeLockType:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeActuatorEnabled:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeClusterRevision:(CHIPDeviceCallback)onCompletion;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

NS_ASSUME_NONNULL_BEGIN

@interface CHIPGroups : NSObject

- (nullable instancetype)initWithDevice:(CHIPDevice *)device endpoint:(uint8_t)endpoint queue:(dispatch_queue_t)queue;
- (BOOL)addGroup:(CHIPDeviceCallback)onCompletion groupId:(uint16_t)groupId groupName:(char *)groupName;
- (BOOL)addGroupIfIdentifying:(CHIPDeviceCallback)onCompletion groupId:(uint16_t)groupId groupName:(char *)groupName;
- (BOOL)getGroupMembership:(CHIPDeviceCallback)onCompletion groupCount:(uint8_t)groupCount groupList:(uint16_t)groupList;
- (BOOL)removeAllGroups:(CHIPDeviceCallback)onCompletion;
- (BOOL)removeGroup:(CHIPDeviceCallback)onCompletion groupId:(uint16_t)groupId;
- (BOOL)viewGroup:(CHIPDeviceCallback)onCompletion groupId:(uint16_t)groupId;

- (BOOL)readAttributeNameSupport:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeClusterRevision:(CHIPDeviceCallback)onCompletion;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

NS_ASSUME_NONNULL_BEGIN

@interface CHIPIasZone : NSObject

- (nullable instancetype)initWithDevice:(CHIPDevice *)device endpoint:(uint8_t)endpoint queue:(dispatch_queue_t)queue;

- (BOOL)readAttributeZoneState:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeZoneType:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeZoneStatus:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeIasCieAddress:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeIasCieAddress:(CHIPDeviceCallback)onCompletion value:(uint64_t)value;
- (BOOL)readAttributeZoneId:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeClusterRevision:(CHIPDeviceCallback)onCompletion;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

NS_ASSUME_NONNULL_BEGIN

@interface CHIPIdentify : NSObject

- (nullable instancetype)initWithDevice:(CHIPDevice *)device endpoint:(uint8_t)endpoint queue:(dispatch_queue_t)queue;
- (BOOL)identify:(CHIPDeviceCallback)onCompletion identifyTime:(uint16_t)identifyTime;
- (BOOL)identifyQuery:(CHIPDeviceCallback)onCompletion;

- (BOOL)readAttributeIdentifyTime:(CHIPDeviceCallback)onCompletion;
- (BOOL)writeAttributeIdentifyTime:(CHIPDeviceCallback)onCompletion value:(uint16_t)value;
- (BOOL)readAttributeClusterRevision:(CHIPDeviceCallback)onCompletion;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

NS_ASSUME_NONNULL_BEGIN

@interface CHIPLevelControl : NSObject

- (nullable instancetype)initWithDevice:(CHIPDevice *)device endpoint:(uint8_t)endpoint queue:(dispatch_queue_t)queue;
- (BOOL)move:(CHIPDeviceCallback)onCompletion
          moveMode:(uint8_t)moveMode
              rate:(uint8_t)rate
        optionMask:(uint8_t)optionMask
    optionOverride:(uint8_t)optionOverride;
- (BOOL)moveToLevel:(CHIPDeviceCallback)onCompletion
              level:(uint8_t)level
     transitionTime:(uint16_t)transitionTime
         optionMask:(uint8_t)optionMask
     optionOverride:(uint8_t)optionOverride;
- (BOOL)moveToLevelWithOnOff:(CHIPDeviceCallback)onCompletion level:(uint8_t)level transitionTime:(uint16_t)transitionTime;
- (BOOL)moveWithOnOff:(CHIPDeviceCallback)onCompletion moveMode:(uint8_t)moveMode rate:(uint8_t)rate;
- (BOOL)step:(CHIPDeviceCallback)onCompletion
          stepMode:(uint8_t)stepMode
          stepSize:(uint8_t)stepSize
    transitionTime:(uint16_t)transitionTime
        optionMask:(uint8_t)optionMask
    optionOverride:(uint8_t)optionOverride;
- (BOOL)stepWithOnOff:(CHIPDeviceCallback)onCompletion
             stepMode:(uint8_t)stepMode
             stepSize:(uint8_t)stepSize
       transitionTime:(uint16_t)transitionTime;
- (BOOL)stop:(CHIPDeviceCallback)onCompletion optionMask:(uint8_t)optionMask optionOverride:(uint8_t)optionOverride;
- (BOOL)stopWithOnOff:(CHIPDeviceCallback)onCompletion;

- (BOOL)readAttributeCurrentLevel:(CHIPDeviceCallback)onCompletion;
- (BOOL)reportAttributeCurrentLevel:(CHIPDeviceCallback)onCompletion
                           onChange:(CHIPDeviceCallback)onChange
                        minInterval:(uint16_t)minInterval
                        maxInterval:(uint16_t)maxInterval
                             change:(uint8_t)change;
- (BOOL)readAttributeClusterRevision:(CHIPDeviceCallback)onCompletion;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

NS_ASSUME_NONNULL_BEGIN

@interface CHIPOnOff : NSObject

- (nullable instancetype)initWithDevice:(CHIPDevice *)device endpoint:(uint8_t)endpoint queue:(dispatch_queue_t)queue;
- (BOOL)off:(CHIPDeviceCallback)onCompletion;
- (BOOL)on:(CHIPDeviceCallback)onCompletion;
- (BOOL)toggle:(CHIPDeviceCallback)onCompletion;

- (BOOL)readAttributeOnOff:(CHIPDeviceCallback)onCompletion;
- (BOOL)reportAttributeOnOff:(CHIPDeviceCallback)onCompletion
                    onChange:(CHIPDeviceCallback)onChange
                 minInterval:(uint16_t)minInterval
                 maxInterval:(uint16_t)maxInterval;
- (BOOL)readAttributeClusterRevision:(CHIPDeviceCallback)onCompletion;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

NS_ASSUME_NONNULL_BEGIN

@interface CHIPScenes : NSObject

- (nullable instancetype)initWithDevice:(CHIPDevice *)device endpoint:(uint8_t)endpoint queue:(dispatch_queue_t)queue;
- (BOOL)addScene:(CHIPDeviceCallback)onCompletion
           groupId:(uint16_t)groupId
           sceneId:(uint8_t)sceneId
    transitionTime:(uint16_t)transitionTime
         sceneName:(char *)sceneName
         clusterId:(uint16_t)clusterId
            length:(uint8_t)length
             value:(uint8_t)value;
- (BOOL)getSceneMembership:(CHIPDeviceCallback)onCompletion groupId:(uint16_t)groupId;
- (BOOL)recallScene:(CHIPDeviceCallback)onCompletion
            groupId:(uint16_t)groupId
            sceneId:(uint8_t)sceneId
     transitionTime:(uint16_t)transitionTime;
- (BOOL)removeAllScenes:(CHIPDeviceCallback)onCompletion groupId:(uint16_t)groupId;
- (BOOL)removeScene:(CHIPDeviceCallback)onCompletion groupId:(uint16_t)groupId sceneId:(uint8_t)sceneId;
- (BOOL)storeScene:(CHIPDeviceCallback)onCompletion groupId:(uint16_t)groupId sceneId:(uint8_t)sceneId;
- (BOOL)viewScene:(CHIPDeviceCallback)onCompletion groupId:(uint16_t)groupId sceneId:(uint8_t)sceneId;

- (BOOL)readAttributeSceneCount:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeCurrentScene:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeCurrentGroup:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeSceneValid:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeNameSupport:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeClusterRevision:(CHIPDeviceCallback)onCompletion;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

NS_ASSUME_NONNULL_BEGIN

@interface CHIPTemperatureMeasurement : NSObject

- (nullable instancetype)initWithDevice:(CHIPDevice *)device endpoint:(uint8_t)endpoint queue:(dispatch_queue_t)queue;

- (BOOL)readAttributeMeasuredValue:(CHIPDeviceCallback)onCompletion;
- (BOOL)reportAttributeMeasuredValue:(CHIPDeviceCallback)onCompletion
                            onChange:(CHIPDeviceCallback)onChange
                         minInterval:(uint16_t)minInterval
                         maxInterval:(uint16_t)maxInterval
                              change:(int16_t)change;
- (BOOL)readAttributeMinMeasuredValue:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeMaxMeasuredValue:(CHIPDeviceCallback)onCompletion;
- (BOOL)readAttributeClusterRevision:(CHIPDeviceCallback)onCompletion;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

#endif /* CHIP_CLUSTERS_H */
