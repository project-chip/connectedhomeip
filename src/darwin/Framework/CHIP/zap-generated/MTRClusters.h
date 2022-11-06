/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#import <Foundation/Foundation.h>

#import <Matter/MTRAttributeCacheContainer.h>

#import <Matter/MTRBaseClusters.h>
#import <Matter/MTRCluster.h>
#import <Matter/MTRCommandPayloadsObjc.h>
#import <Matter/MTRDevice.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Cluster Identify
 *    Attributes and commands for putting a device into Identification mode (e.g. flashing a light).
 */
@interface MTRClusterIdentify : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)identifyWithParams:(MTRIdentifyClusterIdentifyParams *)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)triggerEffectWithParams:(MTRIdentifyClusterTriggerEffectParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeIdentifyTimeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeIdentifyTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeIdentifyTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeIdentifyTypeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Groups
 *    Attributes and commands for group configuration and manipulation.
 */
@interface MTRClusterGroups : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)addGroupWithParams:(MTRGroupsClusterAddGroupParams *)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completion:(void (^)(MTRGroupsClusterAddGroupResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_NEWLY_AVAILABLE;
- (void)viewGroupWithParams:(MTRGroupsClusterViewGroupParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(void (^)(MTRGroupsClusterViewGroupResponseParams * _Nullable data,
                                NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)getGroupMembershipWithParams:(MTRGroupsClusterGetGroupMembershipParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTRGroupsClusterGetGroupMembershipResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)removeGroupWithParams:(MTRGroupsClusterRemoveGroupParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(void (^)(MTRGroupsClusterRemoveGroupResponseParams * _Nullable data,
                                  NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)removeAllGroupsWithParams:(MTRGroupsClusterRemoveAllGroupsParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)removeAllGroupsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)addGroupIfIdentifyingWithParams:(MTRGroupsClusterAddGroupIfIdentifyingParams *)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeNameSupportWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Scenes
 *    Attributes and commands for scene configuration and manipulation.
 */
@interface MTRClusterScenes : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)addSceneWithParams:(MTRScenesClusterAddSceneParams *)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completion:(void (^)(MTRScenesClusterAddSceneResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_NEWLY_AVAILABLE;
- (void)viewSceneWithParams:(MTRScenesClusterViewSceneParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(void (^)(MTRScenesClusterViewSceneResponseParams * _Nullable data,
                                NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)removeSceneWithParams:(MTRScenesClusterRemoveSceneParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(void (^)(MTRScenesClusterRemoveSceneResponseParams * _Nullable data,
                                  NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)removeAllScenesWithParams:(MTRScenesClusterRemoveAllScenesParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(void (^)(MTRScenesClusterRemoveAllScenesResponseParams * _Nullable data,
                                      NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)storeSceneWithParams:(MTRScenesClusterStoreSceneParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(void (^)(MTRScenesClusterStoreSceneResponseParams * _Nullable data,
                                 NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)recallSceneWithParams:(MTRScenesClusterRecallSceneParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)getSceneMembershipWithParams:(MTRScenesClusterGetSceneMembershipParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTRScenesClusterGetSceneMembershipResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)enhancedAddSceneWithParams:(MTRScenesClusterEnhancedAddSceneParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completion:(void (^)(MTRScenesClusterEnhancedAddSceneResponseParams * _Nullable data,
                                       NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)enhancedViewSceneWithParams:(MTRScenesClusterEnhancedViewSceneParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completion:(void (^)(MTRScenesClusterEnhancedViewSceneResponseParams * _Nullable data,
                                        NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)copySceneWithParams:(MTRScenesClusterCopySceneParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(void (^)(MTRScenesClusterCopySceneResponseParams * _Nullable data,
                                NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeSceneCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentSceneWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentGroupWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSceneValidWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNameSupportWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLastConfiguredByWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster On/Off
 *    Attributes and commands for switching devices between 'On' and 'Off' states.
 */
@interface MTRClusterOnOff : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)offWithParams:(MTROnOffClusterOffParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)offWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)onWithParams:(MTROnOffClusterOnParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)onWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                  completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)toggleWithParams:(MTROnOffClusterToggleParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)toggleWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                      completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)offWithEffectWithParams:(MTROnOffClusterOffWithEffectParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)onWithRecallGlobalSceneWithParams:(MTROnOffClusterOnWithRecallGlobalSceneParams * _Nullable)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)onWithRecallGlobalSceneWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)onWithTimedOffWithParams:(MTROnOffClusterOnWithTimedOffParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeOnOffWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGlobalSceneControlWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOnTimeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOnTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOnTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOffWaitTimeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOffWaitTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOffWaitTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeStartUpOnOffWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeStartUpOnOffWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeStartUpOnOffWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster On/off Switch Configuration
 *    Attributes and commands for configuring On/Off switching devices.
 */
@interface MTRClusterOnOffSwitchConfiguration : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeSwitchTypeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSwitchActionsWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeSwitchActionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeSwitchActionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Level Control
 *    Attributes and commands for controlling devices that can be set to a level between fully 'On' and fully 'Off.'
 */
@interface MTRClusterLevelControl : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)moveToLevelWithParams:(MTRLevelControlClusterMoveToLevelParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)moveWithParams:(MTRLevelControlClusterMoveParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)stepWithParams:(MTRLevelControlClusterStepParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)stopWithParams:(MTRLevelControlClusterStopParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)moveToLevelWithOnOffWithParams:(MTRLevelControlClusterMoveToLevelWithOnOffParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)moveWithOnOffWithParams:(MTRLevelControlClusterMoveWithOnOffParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)stepWithOnOffWithParams:(MTRLevelControlClusterStepWithOnOffParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)stopWithOnOffWithParams:(MTRLevelControlClusterStopWithOnOffParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)moveToClosestFrequencyWithParams:(MTRLevelControlClusterMoveToClosestFrequencyParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                              completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeCurrentLevelWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRemainingTimeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinLevelWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxLevelWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentFrequencyWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinFrequencyWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxFrequencyWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOptionsWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOptionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOptionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOnOffTransitionTimeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOnOffTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOnOffTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOnLevelWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOnLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOnLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOnTransitionTimeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOnTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOnTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOffTransitionTimeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOffTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOffTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDefaultMoveRateWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeDefaultMoveRateWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeDefaultMoveRateWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeStartUpCurrentLevelWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeStartUpCurrentLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeStartUpCurrentLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Binary Input (Basic)
 *    An interface for reading the value of a binary measurement and accessing various characteristics of that measurement.
 */
@interface MTRClusterBinaryInputBasic : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeActiveTextWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeActiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeActiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDescriptionWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeDescriptionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeDescriptionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInactiveTextWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInactiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInactiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOutOfServiceWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOutOfServiceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOutOfServiceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePolarityWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePresentValueWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributePresentValueWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributePresentValueWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeReliabilityWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeReliabilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeReliabilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeStatusFlagsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeApplicationTypeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Descriptor
 *    The Descriptor Cluster is meant to replace the support from the Zigbee Device Object (ZDO) for describing a node, its
 * endpoints and clusters.
 */
@interface MTRClusterDescriptor : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeDeviceListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeServerListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClientListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePartsListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Binding
 *    The Binding Cluster is meant to replace the support from the Zigbee Device Object (ZDO) for supporting the binding table.
 */
@interface MTRClusterBinding : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeBindingWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBindingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBindingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Access Control
 *    The Access Control Cluster exposes a data model view of a
      Node's Access Control List (ACL), which codifies the rules used to manage
      and enforce Access Control for the Node's endpoints and their associated
      cluster instances.
 */
@interface MTRClusterAccessControl : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAclWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeAclWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeAclWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeExtensionWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeExtensionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeExtensionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSubjectsPerAccessControlEntryWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTargetsPerAccessControlEntryWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAccessControlEntriesPerFabricWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Actions
 *    This cluster provides a standardized way for a Node (typically a Bridge, but could be any Node) to expose action information.
 */
@interface MTRClusterActions : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)instantActionWithParams:(MTRActionsClusterInstantActionParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)instantActionWithTransitionWithParams:(MTRActionsClusterInstantActionWithTransitionParams *)params
                               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)startActionWithParams:(MTRActionsClusterStartActionParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)startActionWithDurationWithParams:(MTRActionsClusterStartActionWithDurationParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)stopActionWithParams:(MTRActionsClusterStopActionParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)pauseActionWithParams:(MTRActionsClusterPauseActionParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)pauseActionWithDurationWithParams:(MTRActionsClusterPauseActionWithDurationParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)resumeActionWithParams:(MTRActionsClusterResumeActionParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)enableActionWithParams:(MTRActionsClusterEnableActionParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)enableActionWithDurationWithParams:(MTRActionsClusterEnableActionWithDurationParams *)params
                            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)disableActionWithParams:(MTRActionsClusterDisableActionParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)disableActionWithDurationWithParams:(MTRActionsClusterDisableActionWithDurationParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                 completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeActionListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEndpointListsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSetupURLWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Basic
 *    This cluster provides attributes and events for determining basic information about Nodes, which supports both
      Commissioning and operational determination of Node characteristics, such as Vendor ID, Product ID and serial number,
      which apply to the whole Node. Also allows setting user device information such as location.
 */
@interface MTRClusterBasic : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)mfgSpecificPingWithParams:(MTRBasicClusterMfgSpecificPingParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)mfgSpecificPingWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeDataModelRevisionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeVendorNameWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeVendorIDWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeProductNameWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeProductIDWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNodeLabelWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNodeLabelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNodeLabelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLocationWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLocationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLocationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeHardwareVersionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeHardwareVersionStringWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSoftwareVersionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSoftwareVersionStringWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeManufacturingDateWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePartNumberWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeProductURLWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeProductLabelWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSerialNumberWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLocalConfigDisabledWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLocalConfigDisabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLocalConfigDisabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeReachableWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUniqueIDWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCapabilityMinimaWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster OTA Software Update Provider
 *    Provides an interface for providing OTA software updates
 */
@interface MTRClusterOtaSoftwareUpdateProvider : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)queryImageWithParams:(MTROtaSoftwareUpdateProviderClusterQueryImageParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(void (^)(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                                 NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)applyUpdateRequestWithParams:(MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)notifyUpdateAppliedWithParams:(MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster OTA Software Update Requestor
 *    Provides an interface for downloading and applying OTA software updates
 */
@interface MTRClusterOtaSoftwareUpdateRequestor : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)announceOtaProviderWithParams:(MTROtaSoftwareUpdateRequestorClusterAnnounceOtaProviderParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeDefaultOtaProvidersWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeDefaultOtaProvidersWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeDefaultOtaProvidersWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUpdatePossibleWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUpdateStateWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUpdateStateProgressWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Localization Configuration
 *    Nodes should be expected to be deployed to any and all regions of the world. These global regions
      may have differing common languages, units of measurements, and numerical formatting
      standards. As such, Nodes that visually or audibly convey information need a mechanism by which
      they can be configured to use a user’s preferred language, units, etc
 */
@interface MTRClusterLocalizationConfiguration : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeActiveLocaleWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeActiveLocaleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeActiveLocaleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSupportedLocalesWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Time Format Localization
 *    Nodes should be expected to be deployed to any and all regions of the world. These global regions
      may have differing preferences for how dates and times are conveyed. As such, Nodes that visually
      or audibly convey time information need a mechanism by which they can be configured to use a
      user’s preferred format.
 */
@interface MTRClusterTimeFormatLocalization : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeHourFormatWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeHourFormatWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeHourFormatWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActiveCalendarTypeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeActiveCalendarTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeActiveCalendarTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                           params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSupportedCalendarTypesWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Unit Localization
 *    Nodes should be expected to be deployed to any and all regions of the world. These global regions
      may have differing preferences for the units in which values are conveyed in communication to a
      user. As such, Nodes that visually or audibly convey measurable values to the user need a
      mechanism by which they can be configured to use a user’s preferred unit.
 */
@interface MTRClusterUnitLocalization : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeTemperatureUnitWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeTemperatureUnitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeTemperatureUnitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Power Source Configuration
 *    This cluster is used to describe the configuration and capabilities of a Device's power system.
 */
@interface MTRClusterPowerSourceConfiguration : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeSourcesWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Power Source
 *    This cluster is used to describe the configuration and capabilities of a physical power source that provides power to the
 * Node.
 */
@interface MTRClusterPowerSource : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeStatusWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOrderWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDescriptionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWiredAssessedInputVoltageWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWiredAssessedInputFrequencyWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWiredCurrentTypeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWiredAssessedCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWiredNominalVoltageWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWiredMaximumCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWiredPresentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActiveWiredFaultsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatVoltageWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatPercentRemainingWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatTimeRemainingWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatChargeLevelWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatReplacementNeededWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatReplaceabilityWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatPresentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActiveBatFaultsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatReplacementDescriptionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatCommonDesignationWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatANSIDesignationWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatIECDesignationWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatApprovedChemistryWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatCapacityWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatQuantityWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatChargeStateWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatTimeToFullChargeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatFunctionalWhileChargingWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBatChargingCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActiveBatChargeFaultsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster General Commissioning
 *    This cluster is used to manage global aspects of the Commissioning flow.
 */
@interface MTRClusterGeneralCommissioning : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)armFailSafeWithParams:(MTRGeneralCommissioningClusterArmFailSafeParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(void (^)(MTRGeneralCommissioningClusterArmFailSafeResponseParams * _Nullable data,
                                  NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)setRegulatoryConfigWithParams:(MTRGeneralCommissioningClusterSetRegulatoryConfigParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(void (^)(MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams * _Nullable data,
                                          NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)commissioningCompleteWithParams:(MTRGeneralCommissioningClusterCommissioningCompleteParams * _Nullable)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completion:
                                 (void (^)(MTRGeneralCommissioningClusterCommissioningCompleteResponseParams * _Nullable data,
                                     NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)commissioningCompleteWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     completion:
                                         (void (^)(
                                             MTRGeneralCommissioningClusterCommissioningCompleteResponseParams * _Nullable data,
                                             NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeBreadcrumbWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBreadcrumbWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBreadcrumbWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBasicCommissioningInfoWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRegulatoryConfigWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLocationCapabilityWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSupportsConcurrentConnectionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Network Commissioning
 *    Functionality to configure, enable, disable network credentials and access on a Matter device.
 */
@interface MTRClusterNetworkCommissioning : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)scanNetworksWithParams:(MTRNetworkCommissioningClusterScanNetworksParams * _Nullable)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(void (^)(MTRNetworkCommissioningClusterScanNetworksResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)addOrUpdateWiFiNetworkWithParams:(MTRNetworkCommissioningClusterAddOrUpdateWiFiNetworkParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                              completion:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                             NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)addOrUpdateThreadNetworkWithParams:(MTRNetworkCommissioningClusterAddOrUpdateThreadNetworkParams *)params
                            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                completion:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                               NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)removeNetworkWithParams:(MTRNetworkCommissioningClusterRemoveNetworkParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                    NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)connectNetworkWithParams:(MTRNetworkCommissioningClusterConnectNetworkParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(void (^)(MTRNetworkCommissioningClusterConnectNetworkResponseParams * _Nullable data,
                                     NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)reorderNetworkWithParams:(MTRNetworkCommissioningClusterReorderNetworkParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                     NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeMaxNetworksWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNetworksWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeScanMaxTimeSecondsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeConnectMaxTimeSecondsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInterfaceEnabledWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInterfaceEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInterfaceEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLastNetworkingStatusWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLastNetworkIDWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLastConnectErrorValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Diagnostic Logs
 *    The cluster provides commands for retrieving unstructured diagnostic logs from a Node that may be used to aid in diagnostics.
 */
@interface MTRClusterDiagnosticLogs : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)retrieveLogsRequestWithParams:(MTRDiagnosticLogsClusterRetrieveLogsRequestParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(void (^)(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable data,
                                          NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster General Diagnostics
 *    The General Diagnostics Cluster, along with other diagnostics clusters, provide a means to acquire standardized diagnostics
 * metrics that MAY be used by a Node to assist a user or Administrative Node in diagnosing potential problems.
 */
@interface MTRClusterGeneralDiagnostics : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)testEventTriggerWithParams:(MTRGeneralDiagnosticsClusterTestEventTriggerParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeNetworkInterfacesWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRebootCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUpTimeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTotalOperationalHoursWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBootReasonsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActiveHardwareFaultsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActiveRadioFaultsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActiveNetworkFaultsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTestEventTriggersEnabledWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Software Diagnostics
 *    The Software Diagnostics Cluster provides a means to acquire standardized diagnostics metrics that MAY be used by a Node to
 * assist a user or Administrative Node in diagnosing potential problems.
 */
@interface MTRClusterSoftwareDiagnostics : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)resetWatermarksWithParams:(MTRSoftwareDiagnosticsClusterResetWatermarksParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)resetWatermarksWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeThreadMetricsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentHeapFreeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentHeapUsedWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentHeapHighWatermarkWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Thread Network Diagnostics
 *    The Thread Network Diagnostics Cluster provides a means to acquire standardized diagnostics metrics that MAY be used by a Node
 * to assist a user or Administrative Node in diagnosing potential problems
 */
@interface MTRClusterThreadNetworkDiagnostics : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)resetCountsWithParams:(MTRThreadNetworkDiagnosticsClusterResetCountsParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeChannelWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRoutingRoleWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNetworkNameWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePanIdWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeExtendedPanIdWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMeshLocalPrefixWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOverrunCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNeighborTableListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRouteTableListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePartitionIdWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWeightingWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDataVersionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeStableDataVersionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLeaderRouterIdWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDetachedRoleCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeChildRoleCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRouterRoleCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLeaderRoleCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttachAttemptCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePartitionIdChangeCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBetterPartitionAttachAttemptCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeParentChangeCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxTotalCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxUnicastCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxBroadcastCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxAckRequestedCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxAckedCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxNoAckRequestedCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxDataCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxDataPollCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxBeaconCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxBeaconRequestCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxOtherCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxRetryCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxDirectMaxRetryExpiryCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxIndirectMaxRetryExpiryCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxErrCcaCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxErrAbortCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxErrBusyChannelCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxTotalCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxUnicastCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxBroadcastCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxDataCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxDataPollCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxBeaconCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxBeaconRequestCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxOtherCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxAddressFilteredCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxDestAddrFilteredCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxDuplicatedCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxErrNoFrameCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxErrUnknownNeighborCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxErrInvalidSrcAddrCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxErrSecCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxErrFcsCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRxErrOtherCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActiveTimestampWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePendingTimestampWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDelayWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSecurityPolicyWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeChannelPage0MaskWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOperationalDatasetComponentsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActiveNetworkFaultsListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster WiFi Network Diagnostics
 *    The Wi-Fi Network Diagnostics Cluster provides a means to acquire standardized diagnostics metrics that MAY be used by a Node
 * to assist a user or Administrative Node in diagnosing potential problems.
 */
@interface MTRClusterWiFiNetworkDiagnostics : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)resetCountsWithParams:(MTRWiFiNetworkDiagnosticsClusterResetCountsParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeBssidWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSecurityTypeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWiFiVersionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeChannelNumberWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRssiWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBeaconLostCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBeaconRxCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePacketMulticastRxCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePacketMulticastTxCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePacketUnicastRxCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePacketUnicastTxCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentMaxRateWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOverrunCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Ethernet Network Diagnostics
 *    The Ethernet Network Diagnostics Cluster provides a means to acquire standardized diagnostics metrics that MAY be used by a
 * Node to assist a user or Administrative Node in diagnosing potential problems.
 */
@interface MTRClusterEthernetNetworkDiagnostics : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)resetCountsWithParams:(MTREthernetNetworkDiagnosticsClusterResetCountsParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributePHYRateWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFullDuplexWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePacketRxCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePacketTxCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTxErrCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCollisionCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOverrunCountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCarrierDetectWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTimeSinceResetWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Bridged Device Basic
 *    This Cluster serves two purposes towards a Node communicating with a Bridge: indicate that the functionality on
          the Endpoint where it is placed (and its Parts) is bridged from a non-CHIP technology; and provide a centralized
          collection of attributes that the Node MAY collect to aid in conveying information regarding the Bridged Device to a user,
          such as the vendor name, the model name, or user-assigned name.
 */
@interface MTRClusterBridgedDeviceBasic : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeVendorNameWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeVendorIDWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeProductNameWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNodeLabelWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNodeLabelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNodeLabelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeHardwareVersionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeHardwareVersionStringWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSoftwareVersionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSoftwareVersionStringWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeManufacturingDateWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePartNumberWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeProductURLWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeProductLabelWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSerialNumberWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeReachableWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUniqueIDWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Switch
 *    This cluster exposes interactions with a switch device, for the purpose of using those interactions by other devices.
Two types of switch devices are supported: latching switch (e.g. rocker switch) and momentary switch (e.g. push button),
distinguished with their feature flags. Interactions with the switch device are exposed as attributes (for the latching switch) and
as events (for both types of switches). An interested party MAY subscribe to these attributes/events and thus be informed of the
interactions, and can perform actions based on this, for example by sending commands to perform an action such as controlling a
light or a window shade.
 */
@interface MTRClusterSwitch : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeNumberOfPositionsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMultiPressMaxWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster AdministratorCommissioning
 *    Commands to trigger a Node to allow a new Administrator to commission it.
 */
@interface MTRClusterAdministratorCommissioning : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)openCommissioningWindowWithParams:(MTRAdministratorCommissioningClusterOpenCommissioningWindowParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)openBasicCommissioningWindowWithParams:(MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams *)params
                                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                    completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)revokeCommissioningWithParams:(MTRAdministratorCommissioningClusterRevokeCommissioningParams * _Nullable)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)revokeCommissioningWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeWindowStatusWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAdminFabricIndexWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAdminVendorIdWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Operational Credentials
 *    This cluster is used to add or remove Operational Credentials on a Commissionee or Node, as well as manage the associated
 * Fabrics.
 */
@interface MTRClusterOperationalCredentials : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)attestationRequestWithParams:(MTROperationalCredentialsClusterAttestationRequestParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTROperationalCredentialsClusterAttestationResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)certificateChainRequestWithParams:(MTROperationalCredentialsClusterCertificateChainRequestParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completion:(void (^)(MTROperationalCredentialsClusterCertificateChainResponseParams * _Nullable data,
                                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)CSRRequestWithParams:(MTROperationalCredentialsClusterCSRRequestParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(void (^)(MTROperationalCredentialsClusterCSRResponseParams * _Nullable data,
                                 NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)addNOCWithParams:(MTROperationalCredentialsClusterAddNOCParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)updateNOCWithParams:(MTROperationalCredentialsClusterUpdateNOCParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)updateFabricLabelWithParams:(MTROperationalCredentialsClusterUpdateFabricLabelParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completion:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                        NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)removeFabricWithParams:(MTROperationalCredentialsClusterRemoveFabricParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)addTrustedRootCertificateWithParams:(MTROperationalCredentialsClusterAddTrustedRootCertificateParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                 completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeNOCsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFabricsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSupportedFabricsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCommissionedFabricsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTrustedRootCertificatesWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentFabricIndexWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Group Key Management
 *    The Group Key Management Cluster is the mechanism by which group keys are managed.
 */
@interface MTRClusterGroupKeyManagement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)keySetWriteWithParams:(MTRGroupKeyManagementClusterKeySetWriteParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)keySetReadWithParams:(MTRGroupKeyManagementClusterKeySetReadParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(void (^)(MTRGroupKeyManagementClusterKeySetReadResponseParams * _Nullable data,
                                 NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)keySetRemoveWithParams:(MTRGroupKeyManagementClusterKeySetRemoveParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)keySetReadAllIndicesWithParams:(MTRGroupKeyManagementClusterKeySetReadAllIndicesParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(void (^)(MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams * _Nullable data,
                                           NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGroupKeyMapWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeGroupKeyMapWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeGroupKeyMapWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGroupTableWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxGroupsPerFabricWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxGroupKeysPerFabricWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Fixed Label
 *    The Fixed Label Cluster provides a feature for the device to tag an endpoint with zero or more read only
labels.
 */
@interface MTRClusterFixedLabel : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeLabelListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster User Label
 *    The User Label Cluster provides a feature to tag an endpoint with zero or more labels.
 */
@interface MTRClusterUserLabel : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeLabelListWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLabelListWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLabelListWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Boolean State
 *    This cluster provides an interface to a boolean state called StateValue.
 */
@interface MTRClusterBooleanState : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeStateValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Mode Select
 *    Attributes and commands for selecting a mode from a list of supported options.
 */
@interface MTRClusterModeSelect : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)changeToModeWithParams:(MTRModeSelectClusterChangeToModeParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeDescriptionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeStandardNamespaceWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSupportedModesWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentModeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeStartUpModeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeStartUpModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeStartUpModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOnModeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOnModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOnModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Door Lock
 *    An interface to a generic way to secure a door
 */
@interface MTRClusterDoorLock : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)lockDoorWithParams:(MTRDoorLockClusterLockDoorParams * _Nullable)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)unlockDoorWithParams:(MTRDoorLockClusterUnlockDoorParams * _Nullable)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)unlockWithTimeoutWithParams:(MTRDoorLockClusterUnlockWithTimeoutParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)setWeekDayScheduleWithParams:(MTRDoorLockClusterSetWeekDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)getWeekDayScheduleWithParams:(MTRDoorLockClusterGetWeekDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTRDoorLockClusterGetWeekDayScheduleResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)clearWeekDayScheduleWithParams:(MTRDoorLockClusterClearWeekDayScheduleParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)setYearDayScheduleWithParams:(MTRDoorLockClusterSetYearDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)getYearDayScheduleWithParams:(MTRDoorLockClusterGetYearDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTRDoorLockClusterGetYearDayScheduleResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)clearYearDayScheduleWithParams:(MTRDoorLockClusterClearYearDayScheduleParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)setHolidayScheduleWithParams:(MTRDoorLockClusterSetHolidayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)getHolidayScheduleWithParams:(MTRDoorLockClusterGetHolidayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTRDoorLockClusterGetHolidayScheduleResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)clearHolidayScheduleWithParams:(MTRDoorLockClusterClearHolidayScheduleParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)setUserWithParams:(MTRDoorLockClusterSetUserParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)getUserWithParams:(MTRDoorLockClusterGetUserParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRDoorLockClusterGetUserResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_NEWLY_AVAILABLE;
- (void)clearUserWithParams:(MTRDoorLockClusterClearUserParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)setCredentialWithParams:(MTRDoorLockClusterSetCredentialParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(void (^)(MTRDoorLockClusterSetCredentialResponseParams * _Nullable data,
                                    NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)getCredentialStatusWithParams:(MTRDoorLockClusterGetCredentialStatusParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(void (^)(MTRDoorLockClusterGetCredentialStatusResponseParams * _Nullable data,
                                          NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)clearCredentialWithParams:(MTRDoorLockClusterClearCredentialParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeLockStateWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLockTypeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActuatorEnabledWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDoorStateWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDoorOpenEventsWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeDoorOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeDoorOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDoorClosedEventsWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeDoorClosedEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeDoorClosedEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOpenPeriodWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNumberOfTotalUsersSupportedWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNumberOfPINUsersSupportedWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNumberOfRFIDUsersSupportedWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNumberOfWeekDaySchedulesSupportedPerUserWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNumberOfYearDaySchedulesSupportedPerUserWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNumberOfHolidaySchedulesSupportedWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxPINCodeLengthWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinPINCodeLengthWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxRFIDCodeLengthWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinRFIDCodeLengthWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCredentialRulesSupportWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNumberOfCredentialsSupportedPerUserWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLanguageWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLanguageWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLanguageWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLEDSettingsWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLEDSettingsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLEDSettingsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAutoRelockTimeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeAutoRelockTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeAutoRelockTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSoundVolumeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeSoundVolumeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeSoundVolumeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOperatingModeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOperatingModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOperatingModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSupportedOperatingModesWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDefaultConfigurationRegisterWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEnableLocalProgrammingWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeEnableLocalProgrammingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeEnableLocalProgrammingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEnableOneTouchLockingWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeEnableOneTouchLockingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeEnableOneTouchLockingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                              params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEnableInsideStatusLEDWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeEnableInsideStatusLEDWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeEnableInsideStatusLEDWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                              params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEnablePrivacyModeButtonWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeEnablePrivacyModeButtonWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeEnablePrivacyModeButtonWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLocalProgrammingFeaturesWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLocalProgrammingFeaturesWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLocalProgrammingFeaturesWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWrongCodeEntryLimitWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeWrongCodeEntryLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeWrongCodeEntryLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUserCodeTemporaryDisableTimeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeUserCodeTemporaryDisableTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeUserCodeTemporaryDisableTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSendPINOverTheAirWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeSendPINOverTheAirWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeSendPINOverTheAirWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRequirePINforRemoteOperationWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeRequirePINforRemoteOperationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeRequirePINforRemoteOperationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeExpiringUserTimeoutWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeExpiringUserTimeoutWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeExpiringUserTimeoutWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Window Covering
 *    Provides an interface for controlling and adjusting automatic window coverings.
 */
@interface MTRClusterWindowCovering : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)upOrOpenWithParams:(MTRWindowCoveringClusterUpOrOpenParams * _Nullable)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)upOrOpenWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)downOrCloseWithParams:(MTRWindowCoveringClusterDownOrCloseParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)downOrCloseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)stopMotionWithParams:(MTRWindowCoveringClusterStopMotionParams * _Nullable)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)stopMotionWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)goToLiftValueWithParams:(MTRWindowCoveringClusterGoToLiftValueParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)goToLiftPercentageWithParams:(MTRWindowCoveringClusterGoToLiftPercentageParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)goToTiltValueWithParams:(MTRWindowCoveringClusterGoToTiltValueParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)goToTiltPercentageWithParams:(MTRWindowCoveringClusterGoToTiltPercentageParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeTypeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePhysicalClosedLimitLiftWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePhysicalClosedLimitTiltWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionLiftWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionTiltWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNumberOfActuationsLiftWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNumberOfActuationsTiltWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeConfigStatusWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionLiftPercentageWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionTiltPercentageWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOperationalStatusWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTargetPositionLiftPercent100thsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTargetPositionTiltPercent100thsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEndProductTypeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionLiftPercent100thsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionTiltPercent100thsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInstalledOpenLimitLiftWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInstalledClosedLimitLiftWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInstalledOpenLimitTiltWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInstalledClosedLimitTiltWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeModeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
              expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                             params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSafetyStatusWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Barrier Control
 *    This cluster provides control of a barrier (garage door).
 */
@interface MTRClusterBarrierControl : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)barrierControlGoToPercentWithParams:(MTRBarrierControlClusterBarrierControlGoToPercentParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                 completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)barrierControlStopWithParams:(MTRBarrierControlClusterBarrierControlStopParams * _Nullable)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)barrierControlStopWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeBarrierMovingStateWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBarrierSafetyStatusWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBarrierCapabilitiesWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBarrierOpenEventsWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBarrierOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBarrierOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBarrierCloseEventsWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBarrierCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBarrierCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                           params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBarrierCommandOpenEventsWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBarrierCommandCloseEventsWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                  params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBarrierOpenPeriodWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBarrierClosePeriodWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBarrierClosePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBarrierClosePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                           params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBarrierPositionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Pump Configuration and Control
 *    An interface for configuring and controlling pumps.
 */
@interface MTRClusterPumpConfigurationAndControl : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeMaxPressureWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxSpeedWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxFlowWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinConstPressureWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxConstPressureWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinCompPressureWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxCompPressureWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinConstSpeedWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxConstSpeedWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinConstFlowWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxConstFlowWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinConstTempWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxConstTempWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePumpStatusWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEffectiveOperationModeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEffectiveControlModeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCapacityWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSpeedWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLifetimeRunningHoursWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLifetimeRunningHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLifetimeRunningHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePowerWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLifetimeEnergyConsumedWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLifetimeEnergyConsumedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLifetimeEnergyConsumedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOperationModeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOperationModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOperationModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeControlModeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeControlModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeControlModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Thermostat
 *    An interface for configuring and controlling the functionality of a thermostat.
 */
@interface MTRClusterThermostat : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)setpointRaiseLowerWithParams:(MTRThermostatClusterSetpointRaiseLowerParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)setWeeklyScheduleWithParams:(MTRThermostatClusterSetWeeklyScheduleParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)getWeeklyScheduleWithParams:(MTRThermostatClusterGetWeeklyScheduleParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completion:(void (^)(MTRThermostatClusterGetWeeklyScheduleResponseParams * _Nullable data,
                                        NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)clearWeeklyScheduleWithParams:(MTRThermostatClusterClearWeeklyScheduleParams * _Nullable)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)clearWeeklyScheduleWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeLocalTemperatureWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOutdoorTemperatureWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOccupancyWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAbsMinHeatSetpointLimitWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAbsMaxHeatSetpointLimitWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAbsMinCoolSetpointLimitWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAbsMaxCoolSetpointLimitWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePICoolingDemandWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePIHeatingDemandWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeHVACSystemTypeConfigurationWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeHVACSystemTypeConfigurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeHVACSystemTypeConfigurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLocalTemperatureCalibrationWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLocalTemperatureCalibrationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLocalTemperatureCalibrationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOccupiedCoolingSetpointWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOccupiedCoolingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOccupiedCoolingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOccupiedHeatingSetpointWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOccupiedHeatingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOccupiedHeatingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUnoccupiedCoolingSetpointWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeUnoccupiedCoolingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeUnoccupiedCoolingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                  params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUnoccupiedHeatingSetpointWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeUnoccupiedHeatingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeUnoccupiedHeatingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                  params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinHeatSetpointLimitWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeMinHeatSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeMinHeatSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxHeatSetpointLimitWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeMaxHeatSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeMaxHeatSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinCoolSetpointLimitWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeMinCoolSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeMinCoolSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxCoolSetpointLimitWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeMaxCoolSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeMaxCoolSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinSetpointDeadBandWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeMinSetpointDeadBandWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeMinSetpointDeadBandWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRemoteSensingWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeRemoteSensingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeRemoteSensingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeControlSequenceOfOperationWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeControlSequenceOfOperationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeControlSequenceOfOperationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                   params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSystemModeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeSystemModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeSystemModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeThermostatRunningModeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeStartOfWeekWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNumberOfWeeklyTransitionsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNumberOfDailyTransitionsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTemperatureSetpointHoldWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeTemperatureSetpointHoldWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeTemperatureSetpointHoldWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTemperatureSetpointHoldDurationWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeTemperatureSetpointHoldDurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeTemperatureSetpointHoldDurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                        params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeThermostatProgrammingOperationModeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeThermostatProgrammingOperationModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeThermostatProgrammingOperationModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                           params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeThermostatRunningStateWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSetpointChangeSourceWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSetpointChangeAmountWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSetpointChangeSourceTimestampWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOccupiedSetbackWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOccupiedSetbackWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOccupiedSetbackWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOccupiedSetbackMinWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOccupiedSetbackMaxWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUnoccupiedSetbackWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeUnoccupiedSetbackWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeUnoccupiedSetbackWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUnoccupiedSetbackMinWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUnoccupiedSetbackMaxWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEmergencyHeatDeltaWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeEmergencyHeatDeltaWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeEmergencyHeatDeltaWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                           params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeACTypeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeACTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeACTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeACCapacityWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeACCapacityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeACCapacityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeACRefrigerantTypeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeACRefrigerantTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeACRefrigerantTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeACCompressorTypeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeACCompressorTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeACCompressorTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeACErrorCodeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeACErrorCodeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeACErrorCodeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeACLouverPositionWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeACLouverPositionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeACLouverPositionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeACCoilTemperatureWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeACCapacityformatWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeACCapacityformatWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeACCapacityformatWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Fan Control
 *    An interface for controlling a fan in a heating/cooling system.
 */
@interface MTRClusterFanControl : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeFanModeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeFanModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeFanModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFanModeSequenceWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeFanModeSequenceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeFanModeSequenceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePercentSettingWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributePercentSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributePercentSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePercentCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSpeedMaxWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSpeedSettingWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeSpeedSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeSpeedSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSpeedCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRockSupportWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRockSettingWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeRockSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeRockSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWindSupportWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWindSettingWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeWindSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeWindSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Thermostat User Interface Configuration
 *    An interface for configuring the user interface of a thermostat (which may be remote from the thermostat).
 */
@interface MTRClusterThermostatUserInterfaceConfiguration : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeTemperatureDisplayModeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeTemperatureDisplayModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeTemperatureDisplayModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeKeypadLockoutWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeKeypadLockoutWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeKeypadLockoutWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeScheduleProgrammingVisibilityWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeScheduleProgrammingVisibilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeScheduleProgrammingVisibilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Color Control
 *    Attributes and commands for controlling the color properties of a color-capable light.
 */
@interface MTRClusterColorControl : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)moveToHueWithParams:(MTRColorControlClusterMoveToHueParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)moveHueWithParams:(MTRColorControlClusterMoveHueParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)stepHueWithParams:(MTRColorControlClusterStepHueParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)moveToSaturationWithParams:(MTRColorControlClusterMoveToSaturationParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)moveSaturationWithParams:(MTRColorControlClusterMoveSaturationParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)stepSaturationWithParams:(MTRColorControlClusterStepSaturationParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)moveToHueAndSaturationWithParams:(MTRColorControlClusterMoveToHueAndSaturationParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                              completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)moveToColorWithParams:(MTRColorControlClusterMoveToColorParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)moveColorWithParams:(MTRColorControlClusterMoveColorParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)stepColorWithParams:(MTRColorControlClusterStepColorParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)moveToColorTemperatureWithParams:(MTRColorControlClusterMoveToColorTemperatureParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                              completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)enhancedMoveToHueWithParams:(MTRColorControlClusterEnhancedMoveToHueParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)enhancedMoveHueWithParams:(MTRColorControlClusterEnhancedMoveHueParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)enhancedStepHueWithParams:(MTRColorControlClusterEnhancedStepHueParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)enhancedMoveToHueAndSaturationWithParams:(MTRColorControlClusterEnhancedMoveToHueAndSaturationParams *)params
                                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                      completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)colorLoopSetWithParams:(MTRColorControlClusterColorLoopSetParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)stopMoveStepWithParams:(MTRColorControlClusterStopMoveStepParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)moveColorTemperatureWithParams:(MTRColorControlClusterMoveColorTemperatureParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)stepColorTemperatureWithParams:(MTRColorControlClusterStepColorTemperatureParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeCurrentHueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentSaturationWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRemainingTimeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentXWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentYWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDriftCompensationWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCompensationTextWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorTemperatureMiredsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorModeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOptionsWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOptionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOptionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNumberOfPrimariesWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary1XWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary1YWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary1IntensityWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary2XWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary2YWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary2IntensityWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary3XWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary3YWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary3IntensityWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary4XWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary4YWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary4IntensityWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary5XWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary5YWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary5IntensityWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary6XWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary6YWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePrimary6IntensityWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWhitePointXWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeWhitePointXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeWhitePointXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWhitePointYWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeWhitePointYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeWhitePointYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorPointRXWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeColorPointRXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeColorPointRXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorPointRYWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeColorPointRYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeColorPointRYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorPointRIntensityWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeColorPointRIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeColorPointRIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorPointGXWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeColorPointGXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeColorPointGXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorPointGYWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeColorPointGYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeColorPointGYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorPointGIntensityWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeColorPointGIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeColorPointGIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorPointBXWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeColorPointBXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeColorPointBXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorPointBYWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeColorPointBYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeColorPointBYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorPointBIntensityWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeColorPointBIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeColorPointBIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEnhancedCurrentHueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEnhancedColorModeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorLoopActiveWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorLoopDirectionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorLoopTimeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorLoopStartEnhancedHueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorLoopStoredEnhancedHueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorCapabilitiesWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorTempPhysicalMinMiredsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeColorTempPhysicalMaxMiredsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCoupleColorTempToLevelMinMiredsWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeStartUpColorTemperatureMiredsWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeStartUpColorTemperatureMiredsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeStartUpColorTemperatureMiredsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Ballast Configuration
 *    Attributes and commands for configuring a lighting ballast.
 */
@interface MTRClusterBallastConfiguration : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributePhysicalMinLevelWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePhysicalMaxLevelWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBallastStatusWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinLevelWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeMinLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeMinLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxLevelWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeMaxLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeMaxLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeIntrinsicBalanceFactorWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeIntrinsicBalanceFactorWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeIntrinsicBalanceFactorWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBallastFactorAdjustmentWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBallastFactorAdjustmentWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBallastFactorAdjustmentWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLampQuantityWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLampTypeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLampTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLampTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLampManufacturerWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLampManufacturerWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLampManufacturerWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLampRatedHoursWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLampRatedHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLampRatedHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLampBurnHoursWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLampBurnHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLampBurnHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLampAlarmModeWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLampAlarmModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLampAlarmModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLampBurnHoursTripPointWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLampBurnHoursTripPointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLampBurnHoursTripPointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Illuminance Measurement
 *    Attributes and commands for configuring the measurement of illuminance, and reporting illuminance measurements.
 */
@interface MTRClusterIlluminanceMeasurement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLightSensorTypeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Temperature Measurement
 *    Attributes and commands for configuring the measurement of temperature, and reporting temperature measurements.
 */
@interface MTRClusterTemperatureMeasurement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Pressure Measurement
 *    Attributes and commands for configuring the measurement of pressure, and reporting pressure measurements.
 */
@interface MTRClusterPressureMeasurement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeScaledValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinScaledValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxScaledValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeScaledToleranceWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeScaleWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Flow Measurement
 *    Attributes and commands for configuring the measurement of flow, and reporting flow measurements.
 */
@interface MTRClusterFlowMeasurement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Relative Humidity Measurement
 *    Attributes and commands for configuring the measurement of relative humidity, and reporting relative humidity measurements.
 */
@interface MTRClusterRelativeHumidityMeasurement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Occupancy Sensing
 *    Attributes and commands for configuring occupancy sensing, and reporting occupancy status.
 */
@interface MTRClusterOccupancySensing : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeOccupancyWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOccupancySensorTypeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOccupancySensorTypeBitmapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePirOccupiedToUnoccupiedDelayWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributePirOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributePirOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePirUnoccupiedToOccupiedDelayWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributePirUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributePirUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePirUnoccupiedToOccupiedThresholdWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributePirUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributePirUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                         params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUltrasonicOccupiedToUnoccupiedDelayWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeUltrasonicOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeUltrasonicOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUltrasonicUnoccupiedToOccupiedDelayWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeUltrasonicUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeUltrasonicUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUltrasonicUnoccupiedToOccupiedThresholdWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeUltrasonicUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeUltrasonicUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePhysicalContactOccupiedToUnoccupiedDelayWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributePhysicalContactOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributePhysicalContactOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePhysicalContactUnoccupiedToOccupiedDelayWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributePhysicalContactUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributePhysicalContactUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePhysicalContactUnoccupiedToOccupiedThresholdWithParams:
    (MTRReadParams * _Nullable)params;
- (void)writeAttributePhysicalContactUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributePhysicalContactUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Wake on LAN
 *    This cluster provides an interface for managing low power mode on a device that supports the Wake On LAN protocol.
 */
@interface MTRClusterWakeOnLan : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeMACAddressWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Channel
 *    This cluster provides an interface for controlling the current Channel on a device.
 */
@interface MTRClusterChannel : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)changeChannelWithParams:(MTRChannelClusterChangeChannelParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(void (^)(MTRChannelClusterChangeChannelResponseParams * _Nullable data,
                                    NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)changeChannelByNumberWithParams:(MTRChannelClusterChangeChannelByNumberParams *)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)skipChannelWithParams:(MTRChannelClusterSkipChannelParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeChannelListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLineupWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentChannelWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Target Navigator
 *    This cluster provides an interface for UX navigation within a set of targets on a device or endpoint.
 */
@interface MTRClusterTargetNavigator : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)navigateTargetWithParams:(MTRTargetNavigatorClusterNavigateTargetParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(void (^)(MTRTargetNavigatorClusterNavigateTargetResponseParams * _Nullable data,
                                     NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeTargetListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentTargetWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Media Playback
 *    This cluster provides an interface for controlling Media Playback (PLAY, PAUSE, etc) on a media device such as a TV or
 * Speaker.
 */
@interface MTRClusterMediaPlayback : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)playWithParams:(MTRMediaPlaybackClusterPlayParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)playWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)pauseWithParams:(MTRMediaPlaybackClusterPauseParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)pauseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                     completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                    NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)stopPlaybackWithParams:(MTRMediaPlaybackClusterStopPlaybackParams * _Nullable)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)stopPlaybackWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                            completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                           NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)startOverWithParams:(MTRMediaPlaybackClusterStartOverParams * _Nullable)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)startOverWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                         completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                        NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)previousWithParams:(MTRMediaPlaybackClusterPreviousParams * _Nullable)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                               NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)previousWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                       NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)nextWithParams:(MTRMediaPlaybackClusterNextParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)nextWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)rewindWithParams:(MTRMediaPlaybackClusterRewindParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)rewindWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                      completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                     NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)fastForwardWithParams:(MTRMediaPlaybackClusterFastForwardParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                  NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)fastForwardWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                          NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)skipForwardWithParams:(MTRMediaPlaybackClusterSkipForwardParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                  NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)skipBackwardWithParams:(MTRMediaPlaybackClusterSkipBackwardParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)seekWithParams:(MTRMediaPlaybackClusterSeekParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeCurrentStateWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeStartTimeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDurationWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSampledPositionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePlaybackSpeedWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSeekRangeEndWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSeekRangeStartWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Media Input
 *    This cluster provides an interface for controlling the Input Selector on a media device such as a TV.
 */
@interface MTRClusterMediaInput : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)selectInputWithParams:(MTRMediaInputClusterSelectInputParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)showInputStatusWithParams:(MTRMediaInputClusterShowInputStatusParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)showInputStatusWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)hideInputStatusWithParams:(MTRMediaInputClusterHideInputStatusParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)hideInputStatusWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)renameInputWithParams:(MTRMediaInputClusterRenameInputParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeInputListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentInputWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Low Power
 *    This cluster provides an interface for managing low power mode on a device.
 */
@interface MTRClusterLowPower : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)sleepWithParams:(MTRLowPowerClusterSleepParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)sleepWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Keypad Input
 *    This cluster provides an interface for controlling a device like a TV using action commands such as UP, DOWN, and SELECT.
 */
@interface MTRClusterKeypadInput : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)sendKeyWithParams:(MTRKeypadInputClusterSendKeyParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRKeypadInputClusterSendKeyResponseParams * _Nullable data,
                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Content Launcher
 *    This cluster provides an interface for launching content on a media player device such as a TV or Speaker.
 */
@interface MTRClusterContentLauncher : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)launchContentWithParams:(MTRContentLauncherClusterLaunchContentParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(void (^)(MTRContentLauncherClusterLaunchResponseParams * _Nullable data,
                                    NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)launchURLWithParams:(MTRContentLauncherClusterLaunchURLParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(void (^)(MTRContentLauncherClusterLaunchResponseParams * _Nullable data,
                                NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAcceptHeaderWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeSupportedStreamingProtocolsWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeSupportedStreamingProtocolsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeSupportedStreamingProtocolsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Audio Output
 *    This cluster provides an interface for controlling the Output on a media device such as a TV.
 */
@interface MTRClusterAudioOutput : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)selectOutputWithParams:(MTRAudioOutputClusterSelectOutputParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)renameOutputWithParams:(MTRAudioOutputClusterRenameOutputParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeOutputListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentOutputWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Application Launcher
 *    This cluster provides an interface for launching content on a media player device such as a TV or Speaker.
 */
@interface MTRClusterApplicationLauncher : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)launchAppWithParams:(MTRApplicationLauncherClusterLaunchAppParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data,
                                NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)stopAppWithParams:(MTRApplicationLauncherClusterStopAppParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data,
                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)hideAppWithParams:(MTRApplicationLauncherClusterHideAppParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data,
                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeCatalogListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentAppWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeCurrentAppWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeCurrentAppWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Application Basic
 *    This cluster provides information about an application running on a TV or media player device which is represented as an
 * endpoint.
 */
@interface MTRClusterApplicationBasic : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeVendorNameWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeVendorIDWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeApplicationNameWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeProductIDWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeApplicationWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeStatusWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeApplicationVersionWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAllowedVendorListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Account Login
 *    This cluster provides commands that facilitate user account login on a Content App or a node. For example, a Content App
 * running on a Video Player device, which is represented as an endpoint (see [TV Architecture]), can use this cluster to help make
 * the user account on the Content App match the user account on the Client.
 */
@interface MTRClusterAccountLogin : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)getSetupPINWithParams:(MTRAccountLoginClusterGetSetupPINParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(void (^)(MTRAccountLoginClusterGetSetupPINResponseParams * _Nullable data,
                                  NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)loginWithParams:(MTRAccountLoginClusterLoginParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)logoutWithParams:(MTRAccountLoginClusterLogoutParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)logoutWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                      completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Electrical Measurement
 *    Attributes related to the electrical properties of a device. This cluster is used by power outlets and other devices that need
 * to provide instantaneous data as opposed to metrology data which should be retrieved from the metering cluster..
 */
@interface MTRClusterElectricalMeasurement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)getProfileInfoCommandWithParams:(MTRElectricalMeasurementClusterGetProfileInfoCommandParams * _Nullable)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)getProfileInfoCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)getMeasurementProfileCommandWithParams:(MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams *)params
                                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                    completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeMeasurementTypeWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcVoltageWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcVoltageMinWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcVoltageMaxWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcCurrentMinWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcCurrentMaxWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcPowerWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcPowerMinWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcPowerMaxWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcVoltageMultiplierWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcVoltageDivisorWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcCurrentDivisorWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcPowerMultiplierWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeDcPowerDivisorWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcFrequencyWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcFrequencyMinWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcFrequencyMaxWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNeutralCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTotalActivePowerWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTotalReactivePowerWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTotalApparentPowerWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMeasured1stHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMeasured3rdHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMeasured5thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMeasured7thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMeasured9thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMeasured11thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMeasuredPhase1stHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMeasuredPhase3rdHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMeasuredPhase5thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMeasuredPhase7thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMeasuredPhase9thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeMeasuredPhase11thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcFrequencyMultiplierWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcFrequencyDivisorWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePowerMultiplierWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePowerDivisorWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeHarmonicCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePhaseHarmonicCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInstantaneousVoltageWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInstantaneousLineCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInstantaneousActiveCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInstantaneousReactiveCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInstantaneousPowerWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageMinWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageMaxWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentMinWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentMaxWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActivePowerWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActivePowerMinWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActivePowerMaxWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeReactivePowerWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeApparentPowerWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePowerFactorWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsVoltageMeasurementPeriodWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                           params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsUnderVoltageCounterWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeOverVoltagePeriodWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeUnderVoltagePeriodWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSagPeriodWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSwellPeriodWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                              params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcVoltageMultiplierWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcVoltageDivisorWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcCurrentDivisorWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcPowerMultiplierWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcPowerDivisorWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOverloadAlarmsMaskWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                           params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeVoltageOverloadWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCurrentOverloadWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcOverloadAlarmsMaskWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcVoltageOverloadWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcCurrentOverloadWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcActivePowerOverloadWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcReactivePowerOverloadWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsOverVoltageWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsUnderVoltageWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeOverVoltageWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeUnderVoltageWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSagWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSwellWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLineCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActiveCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeReactiveCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltagePhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageMinPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageMaxPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentMinPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentMaxPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActivePowerPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActivePowerMinPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActivePowerMaxPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeReactivePowerPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeApparentPowerPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePowerFactorPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsOverVoltageCounterPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsUnderVoltageCounterPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeOverVoltagePeriodPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSagPeriodPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSwellPeriodPhaseBWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLineCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActiveCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeReactiveCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltagePhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageMinPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageMaxPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentMinPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentMaxPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActivePowerPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActivePowerMinPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeActivePowerMaxPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeReactivePowerPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeApparentPowerPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributePowerFactorPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsOverVoltageCounterPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsUnderVoltageCounterPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeOverVoltagePeriodPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSagPeriodPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSwellPeriodPhaseCWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Test Cluster
 *    The Test Cluster is meant to validate the generated code
 */
@interface MTRClusterTestCluster : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_NEWLY_AVAILABLE;

- (void)testWithParams:(MTRTestClusterClusterTestParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)testWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)testNotHandledWithParams:(MTRTestClusterClusterTestNotHandledParams * _Nullable)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)testNotHandledWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                              completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)testSpecificWithParams:(MTRTestClusterClusterTestSpecificParams * _Nullable)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(void (^)(MTRTestClusterClusterTestSpecificResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)testSpecificWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                            completion:(void (^)(MTRTestClusterClusterTestSpecificResponseParams * _Nullable data,
                                           NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)testUnknownCommandWithParams:(MTRTestClusterClusterTestUnknownCommandParams * _Nullable)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)testUnknownCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)testAddArgumentsWithParams:(MTRTestClusterClusterTestAddArgumentsParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completion:(void (^)(MTRTestClusterClusterTestAddArgumentsResponseParams * _Nullable data,
                                       NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)testSimpleArgumentRequestWithParams:(MTRTestClusterClusterTestSimpleArgumentRequestParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                 completion:(void (^)(MTRTestClusterClusterTestSimpleArgumentResponseParams * _Nullable data,
                                                NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)testStructArrayArgumentRequestWithParams:(MTRTestClusterClusterTestStructArrayArgumentRequestParams *)params
                                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                      completion:
                                          (void (^)(MTRTestClusterClusterTestStructArrayArgumentResponseParams * _Nullable data,
                                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)testStructArgumentRequestWithParams:(MTRTestClusterClusterTestStructArgumentRequestParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                 completion:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)testNestedStructArgumentRequestWithParams:(MTRTestClusterClusterTestNestedStructArgumentRequestParams *)params
                                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                       completion:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                      NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)testListStructArgumentRequestWithParams:(MTRTestClusterClusterTestListStructArgumentRequestParams *)params
                                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                     completion:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                    NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)testListInt8UArgumentRequestWithParams:(MTRTestClusterClusterTestListInt8UArgumentRequestParams *)params
                                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                    completion:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                   NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)testNestedStructListArgumentRequestWithParams:(MTRTestClusterClusterTestNestedStructListArgumentRequestParams *)params
                                       expectedValues:
                                           (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                           completion:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                          NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)testListNestedStructListArgumentRequestWithParams:
            (MTRTestClusterClusterTestListNestedStructListArgumentRequestParams *)params
                                           expectedValues:
                                               (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                               completion:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)testListInt8UReverseRequestWithParams:(MTRTestClusterClusterTestListInt8UReverseRequestParams *)params
                               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                   completion:(void (^)(MTRTestClusterClusterTestListInt8UReverseResponseParams * _Nullable data,
                                                  NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)testEnumsRequestWithParams:(MTRTestClusterClusterTestEnumsRequestParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completion:(void (^)(MTRTestClusterClusterTestEnumsResponseParams * _Nullable data,
                                       NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)testNullableOptionalRequestWithParams:(MTRTestClusterClusterTestNullableOptionalRequestParams * _Nullable)params
                               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                   completion:(void (^)(MTRTestClusterClusterTestNullableOptionalResponseParams * _Nullable data,
                                                  NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)
    testComplexNullableOptionalRequestWithParams:(MTRTestClusterClusterTestComplexNullableOptionalRequestParams *)params
                                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                      completion:
                                          (void (^)(MTRTestClusterClusterTestComplexNullableOptionalResponseParams * _Nullable data,
                                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)simpleStructEchoRequestWithParams:(MTRTestClusterClusterSimpleStructEchoRequestParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completion:(void (^)(MTRTestClusterClusterSimpleStructResponseParams * _Nullable data,
                                              NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)timedInvokeRequestWithParams:(MTRTestClusterClusterTimedInvokeRequestParams * _Nullable)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)timedInvokeRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)testSimpleOptionalArgumentRequestWithParams:(MTRTestClusterClusterTestSimpleOptionalArgumentRequestParams * _Nullable)params
                                     expectedValues:
                                         (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                         completion:(MTRStatusCompletion)completion MTR_NEWLY_AVAILABLE;
- (void)testEmitTestEventRequestWithParams:(MTRTestClusterClusterTestEmitTestEventRequestParams *)params
                            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                completion:(void (^)(MTRTestClusterClusterTestEmitTestEventResponseParams * _Nullable data,
                                               NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;
- (void)
    testEmitTestFabricScopedEventRequestWithParams:(MTRTestClusterClusterTestEmitTestFabricScopedEventRequestParams *)params
                                    expectedValues:
                                        (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                        completion:
                                            (void (^)(
                                                MTRTestClusterClusterTestEmitTestFabricScopedEventResponseParams * _Nullable data,
                                                NSError * _Nullable error))completion MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeBooleanWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBitmap8WithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBitmap8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBitmap8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBitmap16WithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBitmap16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBitmap16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBitmap32WithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBitmap32WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBitmap32WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeBitmap64WithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeBitmap64WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeBitmap64WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt8uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                              params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt16uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt24uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt24uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt24uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt32uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt32uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt32uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt40uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt40uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt40uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt48uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt48uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt48uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt56uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt56uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt56uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt64uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt64uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt64uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt8sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                              params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt16sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt24sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt24sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt24sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt32sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt32sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt32sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt40sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt40sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt40sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt48sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt48sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt48sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt56sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt56sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt56sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeInt64sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeInt64sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeInt64sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEnum8WithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeEnum8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeEnum8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                              params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEnum16WithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeEnum16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeEnum16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFloatSingleWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeFloatSingleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeFloatSingleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFloatDoubleWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeFloatDoubleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeFloatDoubleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeOctetStringWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeListInt8uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeListInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeListInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeListOctetStringWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeListOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeListOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeListStructOctetStringWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeListStructOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeListStructOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                              params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLongOctetStringWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLongOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLongOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeCharStringWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeLongCharStringWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeLongCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeLongCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEpochUsWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeEpochUsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeEpochUsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEpochSWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeEpochSWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeEpochSWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeVendorIdWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeVendorIdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeVendorIdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeListNullablesAndOptionalsStructWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeListNullablesAndOptionalsStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeListNullablesAndOptionalsStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                        params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeEnumAttrWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeEnumAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeEnumAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeStructAttrWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeStructAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeStructAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRangeRestrictedInt8uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeRangeRestrictedInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeRangeRestrictedInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRangeRestrictedInt8sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeRangeRestrictedInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeRangeRestrictedInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRangeRestrictedInt16uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeRangeRestrictedInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeRangeRestrictedInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                              params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeRangeRestrictedInt16sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeRangeRestrictedInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeRangeRestrictedInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                              params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeListLongOctetStringWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeListLongOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeListLongOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeListFabricScopedWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeListFabricScopedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeListFabricScopedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeTimedWriteBooleanWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeTimedWriteBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeTimedWriteBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeGeneralErrorBooleanWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeGeneralErrorBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeGeneralErrorBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterErrorBooleanWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeClusterErrorBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeClusterErrorBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeUnsupportedWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeUnsupportedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeUnsupportedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableBooleanWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableBitmap8WithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableBitmap8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableBitmap8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableBitmap16WithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableBitmap16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableBitmap16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableBitmap32WithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableBitmap32WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableBitmap32WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableBitmap64WithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableBitmap64WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableBitmap64WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt8uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt16uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt24uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt24uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt24uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt32uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt32uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt32uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt40uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt40uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt40uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt48uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt48uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt48uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt56uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt56uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt56uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt64uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt64uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt64uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt8sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt16sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt24sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt24sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt24sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt32sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt32sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt32sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt40sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt40sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt40sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt48sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt48sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt48sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt56sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt56sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt56sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableInt64sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableInt64sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableInt64sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableEnum8WithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableEnum8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableEnum8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableEnum16WithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableEnum16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableEnum16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableFloatSingleWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableFloatSingleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableFloatSingleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableFloatDoubleWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableFloatDoubleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableFloatDoubleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableOctetStringWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableCharStringWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                           params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableEnumAttrWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableEnumAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableEnumAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableStructWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableRangeRestrictedInt8uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableRangeRestrictedInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableRangeRestrictedInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableRangeRestrictedInt8sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableRangeRestrictedInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableRangeRestrictedInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableRangeRestrictedInt16uWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableRangeRestrictedInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableRangeRestrictedInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeNullableRangeRestrictedInt16sWithParams:(MTRReadParams * _Nullable)params;
- (void)writeAttributeNullableRangeRestrictedInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs;
- (void)writeAttributeNullableRangeRestrictedInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                      params:(MTRWriteParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeWriteOnlyInt8uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));
- (void)writeAttributeWriteOnlyInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));
- (void)writeAttributeWriteOnlyInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterIdentify (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)identifyWithParams:(MTRIdentifyClusterIdentifyParams *)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
         completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use identifyWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)triggerEffectWithParams:(MTRIdentifyClusterTriggerEffectParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use triggerEffectWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterGroups (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)addGroupWithParams:(MTRGroupsClusterAddGroupParams *)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
         completionHandler:
             (void (^)(MTRGroupsClusterAddGroupResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use addGroupWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)viewGroupWithParams:(MTRGroupsClusterViewGroupParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:
              (void (^)(MTRGroupsClusterViewGroupResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use viewGroupWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)getGroupMembershipWithParams:(MTRGroupsClusterGetGroupMembershipParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTRGroupsClusterGetGroupMembershipResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use getGroupMembershipWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)removeGroupWithParams:(MTRGroupsClusterRemoveGroupParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:
                (void (^)(MTRGroupsClusterRemoveGroupResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use removeGroupWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)removeAllGroupsWithParams:(MTRGroupsClusterRemoveAllGroupsParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use removeAllGroupsWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)removeAllGroupsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use removeAllGroupsWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)addGroupIfIdentifyingWithParams:(MTRGroupsClusterAddGroupIfIdentifyingParams *)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use addGroupIfIdentifyingWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterScenes (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)addSceneWithParams:(MTRScenesClusterAddSceneParams *)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
         completionHandler:
             (void (^)(MTRScenesClusterAddSceneResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use addSceneWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)viewSceneWithParams:(MTRScenesClusterViewSceneParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:
              (void (^)(MTRScenesClusterViewSceneResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use viewSceneWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)removeSceneWithParams:(MTRScenesClusterRemoveSceneParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:
                (void (^)(MTRScenesClusterRemoveSceneResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use removeSceneWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)removeAllScenesWithParams:(MTRScenesClusterRemoveAllScenesParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(void (^)(MTRScenesClusterRemoveAllScenesResponseParams * _Nullable data,
                                      NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use removeAllScenesWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)storeSceneWithParams:(MTRScenesClusterStoreSceneParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:
               (void (^)(MTRScenesClusterStoreSceneResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use storeSceneWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)recallSceneWithParams:(MTRScenesClusterRecallSceneParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use recallSceneWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)getSceneMembershipWithParams:(MTRScenesClusterGetSceneMembershipParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTRScenesClusterGetSceneMembershipResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use getSceneMembershipWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)enhancedAddSceneWithParams:(MTRScenesClusterEnhancedAddSceneParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completionHandler:(void (^)(MTRScenesClusterEnhancedAddSceneResponseParams * _Nullable data,
                                       NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use enhancedAddSceneWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)enhancedViewSceneWithParams:(MTRScenesClusterEnhancedViewSceneParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completionHandler:(void (^)(MTRScenesClusterEnhancedViewSceneResponseParams * _Nullable data,
                                        NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use enhancedViewSceneWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)copySceneWithParams:(MTRScenesClusterCopySceneParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:
              (void (^)(MTRScenesClusterCopySceneResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use copySceneWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterOnOff (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)offWithParams:(MTROnOffClusterOffParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use offWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)offWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use offWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)onWithParams:(MTROnOffClusterOnParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use onWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)onWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use onWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)toggleWithParams:(MTROnOffClusterToggleParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use toggleWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)toggleWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use toggleWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)offWithEffectWithParams:(MTROnOffClusterOffWithEffectParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use offWithEffectWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)onWithRecallGlobalSceneWithParams:(MTROnOffClusterOnWithRecallGlobalSceneParams * _Nullable)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use onWithRecallGlobalSceneWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)onWithRecallGlobalSceneWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use onWithRecallGlobalSceneWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)onWithTimedOffWithParams:(MTROnOffClusterOnWithTimedOffParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use onWithTimedOffWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterOnOffSwitchConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterLevelControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)moveToLevelWithParams:(MTRLevelControlClusterMoveToLevelParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveToLevelWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)moveWithParams:(MTRLevelControlClusterMoveParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)stepWithParams:(MTRLevelControlClusterStepParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use stepWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)stopWithParams:(MTRLevelControlClusterStopParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use stopWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)moveToLevelWithOnOffWithParams:(MTRLevelControlClusterMoveToLevelWithOnOffParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveToLevelWithOnOffWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)moveWithOnOffWithParams:(MTRLevelControlClusterMoveWithOnOffParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveWithOnOffWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)stepWithOnOffWithParams:(MTRLevelControlClusterStepWithOnOffParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use stepWithOnOffWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)stopWithOnOffWithParams:(MTRLevelControlClusterStopWithOnOffParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use stopWithOnOffWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)moveToClosestFrequencyWithParams:(MTRLevelControlClusterMoveToClosestFrequencyParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveToClosestFrequencyWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterBinaryInputBasic (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterDescriptor (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterBinding (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterAccessControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterActions (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)instantActionWithParams:(MTRActionsClusterInstantActionParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use instantActionWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)instantActionWithTransitionWithParams:(MTRActionsClusterInstantActionWithTransitionParams *)params
                               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use instantActionWithTransitionWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)startActionWithParams:(MTRActionsClusterStartActionParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use startActionWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)startActionWithDurationWithParams:(MTRActionsClusterStartActionWithDurationParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use startActionWithDurationWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)stopActionWithParams:(MTRActionsClusterStopActionParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use stopActionWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)pauseActionWithParams:(MTRActionsClusterPauseActionParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use pauseActionWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)pauseActionWithDurationWithParams:(MTRActionsClusterPauseActionWithDurationParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use pauseActionWithDurationWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)resumeActionWithParams:(MTRActionsClusterResumeActionParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use resumeActionWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)enableActionWithParams:(MTRActionsClusterEnableActionParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use enableActionWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)enableActionWithDurationWithParams:(MTRActionsClusterEnableActionWithDurationParams *)params
                            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use enableActionWithDurationWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)disableActionWithParams:(MTRActionsClusterDisableActionParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use disableActionWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)disableActionWithDurationWithParams:(MTRActionsClusterDisableActionWithDurationParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use disableActionWithDurationWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterBasic (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)mfgSpecificPingWithParams:(MTRBasicClusterMfgSpecificPingParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use mfgSpecificPingWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)mfgSpecificPingWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use mfgSpecificPingWithExpectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterOtaSoftwareUpdateProvider (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)queryImageWithParams:(MTROtaSoftwareUpdateProviderClusterQueryImageParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:(void (^)(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                                 NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use queryImageWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)applyUpdateRequestWithParams:(MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use applyUpdateRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)notifyUpdateAppliedWithParams:(MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use notifyUpdateAppliedWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterOtaSoftwareUpdateRequestor (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)announceOtaProviderWithParams:(MTROtaSoftwareUpdateRequestorClusterAnnounceOtaProviderParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use announceOtaProviderWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterLocalizationConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterTimeFormatLocalization (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterUnitLocalization (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterPowerSourceConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterPowerSource (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterGeneralCommissioning (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)armFailSafeWithParams:(MTRGeneralCommissioningClusterArmFailSafeParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(void (^)(MTRGeneralCommissioningClusterArmFailSafeResponseParams * _Nullable data,
                                  NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use armFailSafeWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)setRegulatoryConfigWithParams:(MTRGeneralCommissioningClusterSetRegulatoryConfigParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(void (^)(MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams * _Nullable data,
                                          NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use setRegulatoryConfigWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)commissioningCompleteWithParams:(MTRGeneralCommissioningClusterCommissioningCompleteParams * _Nullable)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completionHandler:
                          (void (^)(MTRGeneralCommissioningClusterCommissioningCompleteResponseParams * _Nullable data,
                              NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use commissioningCompleteWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)commissioningCompleteWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                              completionHandler:
                                  (void (^)(MTRGeneralCommissioningClusterCommissioningCompleteResponseParams * _Nullable data,
                                      NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use commissioningCompleteWithExpectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterNetworkCommissioning (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)scanNetworksWithParams:(MTRNetworkCommissioningClusterScanNetworksParams * _Nullable)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(void (^)(MTRNetworkCommissioningClusterScanNetworksResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use scanNetworksWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)addOrUpdateWiFiNetworkWithParams:(MTRNetworkCommissioningClusterAddOrUpdateWiFiNetworkParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completionHandler:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                             NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use addOrUpdateWiFiNetworkWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)addOrUpdateThreadNetworkWithParams:(MTRNetworkCommissioningClusterAddOrUpdateThreadNetworkParams *)params
                            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completionHandler:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                               NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use addOrUpdateThreadNetworkWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)removeNetworkWithParams:(MTRNetworkCommissioningClusterRemoveNetworkParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                    NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use removeNetworkWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)connectNetworkWithParams:(MTRNetworkCommissioningClusterConnectNetworkParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(void (^)(MTRNetworkCommissioningClusterConnectNetworkResponseParams * _Nullable data,
                                     NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use connectNetworkWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)reorderNetworkWithParams:(MTRNetworkCommissioningClusterReorderNetworkParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                     NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use reorderNetworkWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterDiagnosticLogs (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)retrieveLogsRequestWithParams:(MTRDiagnosticLogsClusterRetrieveLogsRequestParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(void (^)(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable data,
                                          NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use retrieveLogsRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterGeneralDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)testEventTriggerWithParams:(MTRGeneralDiagnosticsClusterTestEventTriggerParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use testEventTriggerWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterSoftwareDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)resetWatermarksWithParams:(MTRSoftwareDiagnosticsClusterResetWatermarksParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use resetWatermarksWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)resetWatermarksWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use resetWatermarksWithExpectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterThreadNetworkDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)resetCountsWithParams:(MTRThreadNetworkDiagnosticsClusterResetCountsParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use resetCountsWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use resetCountsWithExpectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterWiFiNetworkDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)resetCountsWithParams:(MTRWiFiNetworkDiagnosticsClusterResetCountsParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use resetCountsWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use resetCountsWithExpectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterEthernetNetworkDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)resetCountsWithParams:(MTREthernetNetworkDiagnosticsClusterResetCountsParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use resetCountsWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use resetCountsWithExpectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterBridgedDeviceBasic (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterSwitch (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterAdministratorCommissioning (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)openCommissioningWindowWithParams:(MTRAdministratorCommissioningClusterOpenCommissioningWindowParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use openCommissioningWindowWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)openBasicCommissioningWindowWithParams:(MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams *)params
                                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use openBasicCommissioningWindowWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)revokeCommissioningWithParams:(MTRAdministratorCommissioningClusterRevokeCommissioningParams * _Nullable)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use revokeCommissioningWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)revokeCommissioningWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use revokeCommissioningWithExpectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterOperationalCredentials (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)attestationRequestWithParams:(MTROperationalCredentialsClusterAttestationRequestParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTROperationalCredentialsClusterAttestationResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use attestationRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)certificateChainRequestWithParams:(MTROperationalCredentialsClusterCertificateChainRequestParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completionHandler:(void (^)(MTROperationalCredentialsClusterCertificateChainResponseParams * _Nullable data,
                                              NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use certificateChainRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)CSRRequestWithParams:(MTROperationalCredentialsClusterCSRRequestParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:(void (^)(MTROperationalCredentialsClusterCSRResponseParams * _Nullable data,
                                 NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use CSRRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)addNOCWithParams:(MTROperationalCredentialsClusterAddNOCParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                              NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use addNOCWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)updateNOCWithParams:(MTROperationalCredentialsClusterUpdateNOCParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use updateNOCWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)updateFabricLabelWithParams:(MTROperationalCredentialsClusterUpdateFabricLabelParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completionHandler:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                        NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use updateFabricLabelWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)removeFabricWithParams:(MTROperationalCredentialsClusterRemoveFabricParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use removeFabricWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)addTrustedRootCertificateWithParams:(MTROperationalCredentialsClusterAddTrustedRootCertificateParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use addTrustedRootCertificateWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterGroupKeyManagement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)keySetWriteWithParams:(MTRGroupKeyManagementClusterKeySetWriteParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use keySetWriteWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)keySetReadWithParams:(MTRGroupKeyManagementClusterKeySetReadParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:(void (^)(MTRGroupKeyManagementClusterKeySetReadResponseParams * _Nullable data,
                                 NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use keySetReadWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)keySetRemoveWithParams:(MTRGroupKeyManagementClusterKeySetRemoveParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use keySetRemoveWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)keySetReadAllIndicesWithParams:(MTRGroupKeyManagementClusterKeySetReadAllIndicesParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(void (^)(MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams * _Nullable data,
                                           NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use keySetReadAllIndicesWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterFixedLabel (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterUserLabel (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterBooleanState (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterModeSelect (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)changeToModeWithParams:(MTRModeSelectClusterChangeToModeParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use changeToModeWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterDoorLock (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)lockDoorWithParams:(MTRDoorLockClusterLockDoorParams * _Nullable)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
         completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use lockDoorWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)unlockDoorWithParams:(MTRDoorLockClusterUnlockDoorParams * _Nullable)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use unlockDoorWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)unlockWithTimeoutWithParams:(MTRDoorLockClusterUnlockWithTimeoutParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use unlockWithTimeoutWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)setWeekDayScheduleWithParams:(MTRDoorLockClusterSetWeekDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use setWeekDayScheduleWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)getWeekDayScheduleWithParams:(MTRDoorLockClusterGetWeekDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTRDoorLockClusterGetWeekDayScheduleResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use getWeekDayScheduleWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)clearWeekDayScheduleWithParams:(MTRDoorLockClusterClearWeekDayScheduleParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use clearWeekDayScheduleWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)setYearDayScheduleWithParams:(MTRDoorLockClusterSetYearDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use setYearDayScheduleWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)getYearDayScheduleWithParams:(MTRDoorLockClusterGetYearDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTRDoorLockClusterGetYearDayScheduleResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use getYearDayScheduleWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)clearYearDayScheduleWithParams:(MTRDoorLockClusterClearYearDayScheduleParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use clearYearDayScheduleWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)setHolidayScheduleWithParams:(MTRDoorLockClusterSetHolidayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use setHolidayScheduleWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)getHolidayScheduleWithParams:(MTRDoorLockClusterGetHolidayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTRDoorLockClusterGetHolidayScheduleResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use getHolidayScheduleWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)clearHolidayScheduleWithParams:(MTRDoorLockClusterClearHolidayScheduleParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use clearHolidayScheduleWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)setUserWithParams:(MTRDoorLockClusterSetUserParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use setUserWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)getUserWithParams:(MTRDoorLockClusterGetUserParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRDoorLockClusterGetUserResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use getUserWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)clearUserWithParams:(MTRDoorLockClusterClearUserParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use clearUserWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)setCredentialWithParams:(MTRDoorLockClusterSetCredentialParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(void (^)(MTRDoorLockClusterSetCredentialResponseParams * _Nullable data,
                                    NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use setCredentialWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)getCredentialStatusWithParams:(MTRDoorLockClusterGetCredentialStatusParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(void (^)(MTRDoorLockClusterGetCredentialStatusResponseParams * _Nullable data,
                                          NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use getCredentialStatusWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)clearCredentialWithParams:(MTRDoorLockClusterClearCredentialParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use clearCredentialWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterWindowCovering (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)upOrOpenWithParams:(MTRWindowCoveringClusterUpOrOpenParams * _Nullable)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
         completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use upOrOpenWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)upOrOpenWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                 completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use upOrOpenWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)downOrCloseWithParams:(MTRWindowCoveringClusterDownOrCloseParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use downOrCloseWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)downOrCloseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use downOrCloseWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)stopMotionWithParams:(MTRWindowCoveringClusterStopMotionParams * _Nullable)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use stopMotionWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)stopMotionWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use stopMotionWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)goToLiftValueWithParams:(MTRWindowCoveringClusterGoToLiftValueParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use goToLiftValueWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)goToLiftPercentageWithParams:(MTRWindowCoveringClusterGoToLiftPercentageParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use goToLiftPercentageWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)goToTiltValueWithParams:(MTRWindowCoveringClusterGoToTiltValueParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use goToTiltValueWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)goToTiltPercentageWithParams:(MTRWindowCoveringClusterGoToTiltPercentageParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use goToTiltPercentageWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterBarrierControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)barrierControlGoToPercentWithParams:(MTRBarrierControlClusterBarrierControlGoToPercentParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use barrierControlGoToPercentWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)barrierControlStopWithParams:(MTRBarrierControlClusterBarrierControlStopParams * _Nullable)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use barrierControlStopWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)barrierControlStopWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use barrierControlStopWithExpectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterPumpConfigurationAndControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterThermostat (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)setpointRaiseLowerWithParams:(MTRThermostatClusterSetpointRaiseLowerParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use setpointRaiseLowerWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)setWeeklyScheduleWithParams:(MTRThermostatClusterSetWeeklyScheduleParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use setWeeklyScheduleWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)getWeeklyScheduleWithParams:(MTRThermostatClusterGetWeeklyScheduleParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completionHandler:(void (^)(MTRThermostatClusterGetWeeklyScheduleResponseParams * _Nullable data,
                                        NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use getWeeklyScheduleWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)clearWeeklyScheduleWithParams:(MTRThermostatClusterClearWeeklyScheduleParams * _Nullable)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use clearWeeklyScheduleWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)clearWeeklyScheduleWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use clearWeeklyScheduleWithExpectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterFanControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterThermostatUserInterfaceConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterColorControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)moveToHueWithParams:(MTRColorControlClusterMoveToHueParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveToHueWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)moveHueWithParams:(MTRColorControlClusterMoveHueParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveHueWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)stepHueWithParams:(MTRColorControlClusterStepHueParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use stepHueWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)moveToSaturationWithParams:(MTRColorControlClusterMoveToSaturationParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveToSaturationWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)moveSaturationWithParams:(MTRColorControlClusterMoveSaturationParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveSaturationWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)stepSaturationWithParams:(MTRColorControlClusterStepSaturationParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use stepSaturationWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)moveToHueAndSaturationWithParams:(MTRColorControlClusterMoveToHueAndSaturationParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveToHueAndSaturationWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)moveToColorWithParams:(MTRColorControlClusterMoveToColorParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveToColorWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)moveColorWithParams:(MTRColorControlClusterMoveColorParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveColorWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)stepColorWithParams:(MTRColorControlClusterStepColorParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use stepColorWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)moveToColorTemperatureWithParams:(MTRColorControlClusterMoveToColorTemperatureParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveToColorTemperatureWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)enhancedMoveToHueWithParams:(MTRColorControlClusterEnhancedMoveToHueParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use enhancedMoveToHueWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)enhancedMoveHueWithParams:(MTRColorControlClusterEnhancedMoveHueParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use enhancedMoveHueWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)enhancedStepHueWithParams:(MTRColorControlClusterEnhancedStepHueParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use enhancedStepHueWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)enhancedMoveToHueAndSaturationWithParams:(MTRColorControlClusterEnhancedMoveToHueAndSaturationParams *)params
                                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use enhancedMoveToHueAndSaturationWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)colorLoopSetWithParams:(MTRColorControlClusterColorLoopSetParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use colorLoopSetWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)stopMoveStepWithParams:(MTRColorControlClusterStopMoveStepParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use stopMoveStepWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)moveColorTemperatureWithParams:(MTRColorControlClusterMoveColorTemperatureParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use moveColorTemperatureWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)stepColorTemperatureWithParams:(MTRColorControlClusterStepColorTemperatureParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use stepColorTemperatureWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterBallastConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterIlluminanceMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterTemperatureMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterPressureMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterFlowMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterRelativeHumidityMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterOccupancySensing (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterWakeOnLan (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterChannel (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)changeChannelWithParams:(MTRChannelClusterChangeChannelParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(void (^)(MTRChannelClusterChangeChannelResponseParams * _Nullable data,
                                    NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use changeChannelWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)changeChannelByNumberWithParams:(MTRChannelClusterChangeChannelByNumberParams *)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use changeChannelByNumberWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)skipChannelWithParams:(MTRChannelClusterSkipChannelParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use skipChannelWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterTargetNavigator (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)navigateTargetWithParams:(MTRTargetNavigatorClusterNavigateTargetParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(void (^)(MTRTargetNavigatorClusterNavigateTargetResponseParams * _Nullable data,
                                     NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use navigateTargetWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterMediaPlayback (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)playWithParams:(MTRMediaPlaybackClusterPlayParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use playWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)playWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
             completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use playWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)pauseWithParams:(MTRMediaPlaybackClusterPauseParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use pauseWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)pauseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
              completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                    NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use pauseWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)stopPlaybackWithParams:(MTRMediaPlaybackClusterStopPlaybackParams * _Nullable)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use stopPlaybackWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)stopPlaybackWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                     completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                           NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use stopPlaybackWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)startOverWithParams:(MTRMediaPlaybackClusterStartOverParams * _Nullable)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:
              (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use startOverWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)startOverWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                  completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                        NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use startOverWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)previousWithParams:(MTRMediaPlaybackClusterPreviousParams * _Nullable)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
         completionHandler:
             (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use previousWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)previousWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                 completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                       NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use previousWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)nextWithParams:(MTRMediaPlaybackClusterNextParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use nextWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)nextWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
             completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use nextWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)rewindWithParams:(MTRMediaPlaybackClusterRewindParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use rewindWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)rewindWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
               completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                     NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use rewindWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)fastForwardWithParams:(MTRMediaPlaybackClusterFastForwardParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                  NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use fastForwardWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)fastForwardWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                          NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use fastForwardWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)skipForwardWithParams:(MTRMediaPlaybackClusterSkipForwardParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                  NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use skipForwardWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)skipBackwardWithParams:(MTRMediaPlaybackClusterSkipBackwardParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use skipBackwardWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)seekWithParams:(MTRMediaPlaybackClusterSeekParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use seekWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterMediaInput (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)selectInputWithParams:(MTRMediaInputClusterSelectInputParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use selectInputWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)showInputStatusWithParams:(MTRMediaInputClusterShowInputStatusParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use showInputStatusWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)showInputStatusWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use showInputStatusWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)hideInputStatusWithParams:(MTRMediaInputClusterHideInputStatusParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use hideInputStatusWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)hideInputStatusWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use hideInputStatusWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)renameInputWithParams:(MTRMediaInputClusterRenameInputParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use renameInputWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterLowPower (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)sleepWithParams:(MTRLowPowerClusterSleepParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use sleepWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)sleepWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use sleepWithExpectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterKeypadInput (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)sendKeyWithParams:(MTRKeypadInputClusterSendKeyParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRKeypadInputClusterSendKeyResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use sendKeyWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterContentLauncher (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)launchContentWithParams:(MTRContentLauncherClusterLaunchContentParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(void (^)(MTRContentLauncherClusterLaunchResponseParams * _Nullable data,
                                    NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use launchContentWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)launchURLWithParams:(MTRContentLauncherClusterLaunchURLParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:
              (void (^)(MTRContentLauncherClusterLaunchResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use launchURLWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterAudioOutput (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)selectOutputWithParams:(MTRAudioOutputClusterSelectOutputParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use selectOutputWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)renameOutputWithParams:(MTRAudioOutputClusterRenameOutputParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use renameOutputWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterApplicationLauncher (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)launchAppWithParams:(MTRApplicationLauncherClusterLaunchAppParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data,
                                NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use launchAppWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)stopAppWithParams:(MTRApplicationLauncherClusterStopAppParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data,
                              NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use stopAppWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)hideAppWithParams:(MTRApplicationLauncherClusterHideAppParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data,
                              NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use hideAppWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterApplicationBasic (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

@end

@interface MTRClusterAccountLogin (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)getSetupPINWithParams:(MTRAccountLoginClusterGetSetupPINParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(void (^)(MTRAccountLoginClusterGetSetupPINResponseParams * _Nullable data,
                                  NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use getSetupPINWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)loginWithParams:(MTRAccountLoginClusterLoginParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use loginWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)logoutWithParams:(MTRAccountLoginClusterLogoutParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use logoutWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)logoutWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use logoutWithExpectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterElectricalMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)getProfileInfoCommandWithParams:(MTRElectricalMeasurementClusterGetProfileInfoCommandParams * _Nullable)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use getProfileInfoCommandWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)getProfileInfoCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use getProfileInfoCommandWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)getMeasurementProfileCommandWithParams:(MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams *)params
                                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use getMeasurementProfileCommandWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

@interface MTRClusterTestCluster (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use initWithDevice:endpoindID:queue:");

- (void)testWithParams:(MTRTestClusterClusterTestParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use testWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use testWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)testNotHandledWithParams:(MTRTestClusterClusterTestNotHandledParams * _Nullable)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use testNotHandledWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testNotHandledWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                       completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use testNotHandledWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)testSpecificWithParams:(MTRTestClusterClusterTestSpecificParams * _Nullable)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(void (^)(MTRTestClusterClusterTestSpecificResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use testSpecificWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testSpecificWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                     completionHandler:(void (^)(MTRTestClusterClusterTestSpecificResponseParams * _Nullable data,
                                           NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use testSpecificWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)testUnknownCommandWithParams:(MTRTestClusterClusterTestUnknownCommandParams * _Nullable)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use testUnknownCommandWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testUnknownCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use testUnknownCommandWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)testAddArgumentsWithParams:(MTRTestClusterClusterTestAddArgumentsParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completionHandler:(void (^)(MTRTestClusterClusterTestAddArgumentsResponseParams * _Nullable data,
                                       NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use testAddArgumentsWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testSimpleArgumentRequestWithParams:(MTRTestClusterClusterTestSimpleArgumentRequestParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completionHandler:(void (^)(MTRTestClusterClusterTestSimpleArgumentResponseParams * _Nullable data,
                                                NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use testSimpleArgumentRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testStructArrayArgumentRequestWithParams:(MTRTestClusterClusterTestStructArrayArgumentRequestParams *)params
                                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completionHandler:
                                   (void (^)(MTRTestClusterClusterTestStructArrayArgumentResponseParams * _Nullable data,
                                       NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use testStructArrayArgumentRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testStructArgumentRequestWithParams:(MTRTestClusterClusterTestStructArgumentRequestParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use testStructArgumentRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testNestedStructArgumentRequestWithParams:(MTRTestClusterClusterTestNestedStructArgumentRequestParams *)params
                                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                      NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use testNestedStructArgumentRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testListStructArgumentRequestWithParams:(MTRTestClusterClusterTestListStructArgumentRequestParams *)params
                                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                              completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                    NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use testListStructArgumentRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testListInt8UArgumentRequestWithParams:(MTRTestClusterClusterTestListInt8UArgumentRequestParams *)params
                                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                   NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use testListInt8UArgumentRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testNestedStructListArgumentRequestWithParams:(MTRTestClusterClusterTestNestedStructListArgumentRequestParams *)params
                                       expectedValues:
                                           (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                    completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                          NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED(
        "Please use testNestedStructListArgumentRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testListNestedStructListArgumentRequestWithParams:
            (MTRTestClusterClusterTestListNestedStructListArgumentRequestParams *)params
                                           expectedValues:
                                               (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                        completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                              NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED(
        "Please use testListNestedStructListArgumentRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testListInt8UReverseRequestWithParams:(MTRTestClusterClusterTestListInt8UReverseRequestParams *)params
                               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completionHandler:(void (^)(MTRTestClusterClusterTestListInt8UReverseResponseParams * _Nullable data,
                                                  NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use testListInt8UReverseRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testEnumsRequestWithParams:(MTRTestClusterClusterTestEnumsRequestParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completionHandler:(void (^)(MTRTestClusterClusterTestEnumsResponseParams * _Nullable data,
                                       NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use testEnumsRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testNullableOptionalRequestWithParams:(MTRTestClusterClusterTestNullableOptionalRequestParams * _Nullable)params
                               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completionHandler:(void (^)(MTRTestClusterClusterTestNullableOptionalResponseParams * _Nullable data,
                                                  NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use testNullableOptionalRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testComplexNullableOptionalRequestWithParams:(MTRTestClusterClusterTestComplexNullableOptionalRequestParams *)params
                                      expectedValues:
                                          (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                   completionHandler:
                                       (void (^)(MTRTestClusterClusterTestComplexNullableOptionalResponseParams * _Nullable data,
                                           NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED(
        "Please use testComplexNullableOptionalRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)simpleStructEchoRequestWithParams:(MTRTestClusterClusterSimpleStructEchoRequestParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completionHandler:(void (^)(MTRTestClusterClusterSimpleStructResponseParams * _Nullable data,
                                              NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use simpleStructEchoRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)timedInvokeRequestWithParams:(MTRTestClusterClusterTimedInvokeRequestParams * _Nullable)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use timedInvokeRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)timedInvokeRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED("Please use timedInvokeRequestWithExpectedValues:expectedValueIntervalMs:completion:");
- (void)testSimpleOptionalArgumentRequestWithParams:(MTRTestClusterClusterTestSimpleOptionalArgumentRequestParams * _Nullable)params
                                     expectedValues:
                                         (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                  completionHandler:(MTRStatusCompletion)completionHandler
    MTR_NEWLY_DEPRECATED(
        "Please use testSimpleOptionalArgumentRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testEmitTestEventRequestWithParams:(MTRTestClusterClusterTestEmitTestEventRequestParams *)params
                            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completionHandler:(void (^)(MTRTestClusterClusterTestEmitTestEventResponseParams * _Nullable data,
                                               NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED("Please use testEmitTestEventRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
- (void)testEmitTestFabricScopedEventRequestWithParams:(MTRTestClusterClusterTestEmitTestFabricScopedEventRequestParams *)params
                                        expectedValues:
                                            (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                     completionHandler:
                                         (void (^)(
                                             MTRTestClusterClusterTestEmitTestFabricScopedEventResponseParams * _Nullable data,
                                             NSError * _Nullable error))completionHandler
    MTR_NEWLY_DEPRECATED(
        "Please use testEmitTestFabricScopedEventRequestWithParams:expectedValues:expectedValueIntervalMs:completion:");
@end

NS_ASSUME_NONNULL_END
