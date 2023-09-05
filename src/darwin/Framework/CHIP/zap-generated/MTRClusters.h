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

#import <Matter/MTRBaseClusters.h>
#import <Matter/MTRCluster.h>
#import <Matter/MTRCommandPayloadsObjc.h>
#import <Matter/MTRDefines.h>
#import <Matter/MTRDevice.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Cluster Identify
 *    Attributes and commands for putting a device into Identification mode (e.g. flashing a light).
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterIdentify : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)identifyWithParams:(MTRIdentifyClusterIdentifyParams *)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)triggerEffectWithParams:(MTRIdentifyClusterTriggerEffectParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeIdentifyTimeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeIdentifyTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeIdentifyTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeIdentifyTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Groups
 *    Attributes and commands for group configuration and manipulation.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterGroups : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)addGroupWithParams:(MTRGroupsClusterAddGroupParams *)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completion:(void (^)(MTRGroupsClusterAddGroupResponseParams * _Nullable data, NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)viewGroupWithParams:(MTRGroupsClusterViewGroupParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:
                     (void (^)(MTRGroupsClusterViewGroupResponseParams * _Nullable data, NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getGroupMembershipWithParams:(MTRGroupsClusterGetGroupMembershipParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTRGroupsClusterGetGroupMembershipResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)removeGroupWithParams:(MTRGroupsClusterRemoveGroupParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:
                       (void (^)(MTRGroupsClusterRemoveGroupResponseParams * _Nullable data, NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)removeAllGroupsWithParams:(MTRGroupsClusterRemoveAllGroupsParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)removeAllGroupsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)addGroupIfIdentifyingWithParams:(MTRGroupsClusterAddGroupIfIdentifyingParams *)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNameSupportWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Scenes
 *    Attributes and commands for scene configuration and manipulation.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterScenes : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)addSceneWithParams:(MTRScenesClusterAddSceneParams *)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completion:(void (^)(MTRScenesClusterAddSceneResponseParams * _Nullable data, NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)viewSceneWithParams:(MTRScenesClusterViewSceneParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:
                     (void (^)(MTRScenesClusterViewSceneResponseParams * _Nullable data, NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)removeSceneWithParams:(MTRScenesClusterRemoveSceneParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:
                       (void (^)(MTRScenesClusterRemoveSceneResponseParams * _Nullable data, NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)removeAllScenesWithParams:(MTRScenesClusterRemoveAllScenesParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(void (^)(MTRScenesClusterRemoveAllScenesResponseParams * _Nullable data,
                                      NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)storeSceneWithParams:(MTRScenesClusterStoreSceneParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:
                      (void (^)(MTRScenesClusterStoreSceneResponseParams * _Nullable data, NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)recallSceneWithParams:(MTRScenesClusterRecallSceneParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getSceneMembershipWithParams:(MTRScenesClusterGetSceneMembershipParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTRScenesClusterGetSceneMembershipResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enhancedAddSceneWithParams:(MTRScenesClusterEnhancedAddSceneParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completion:(void (^)(MTRScenesClusterEnhancedAddSceneResponseParams * _Nullable data,
                                       NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enhancedViewSceneWithParams:(MTRScenesClusterEnhancedViewSceneParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completion:(void (^)(MTRScenesClusterEnhancedViewSceneResponseParams * _Nullable data,
                                        NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)copySceneWithParams:(MTRScenesClusterCopySceneParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:
                     (void (^)(MTRScenesClusterCopySceneResponseParams * _Nullable data, NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeSceneCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentSceneWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentGroupWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSceneValidWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNameSupportWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLastConfiguredByWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster On/Off
 *    Attributes and commands for switching devices between 'On' and 'Off' states.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterOnOff : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)offWithParams:(MTROnOffClusterOffParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)offWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)onWithParams:(MTROnOffClusterOnParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)onWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                  completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)toggleWithParams:(MTROnOffClusterToggleParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)toggleWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                      completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)offWithEffectWithParams:(MTROnOffClusterOffWithEffectParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)onWithRecallGlobalSceneWithParams:(MTROnOffClusterOnWithRecallGlobalSceneParams * _Nullable)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)onWithRecallGlobalSceneWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)onWithTimedOffWithParams:(MTROnOffClusterOnWithTimedOffParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeOnOffWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGlobalSceneControlWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOnTimeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOffWaitTimeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOffWaitTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOffWaitTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeStartUpOnOffWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpOnOffWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpOnOffWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster On/off Switch Configuration
 *    Attributes and commands for configuring On/Off switching devices.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterOnOffSwitchConfiguration : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeSwitchTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSwitchActionsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSwitchActionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSwitchActionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Level Control
 *    Attributes and commands for controlling devices that can be set to a level between fully 'On' and fully 'Off.'
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterLevelControl : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)moveToLevelWithParams:(MTRLevelControlClusterMoveToLevelParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveWithParams:(MTRLevelControlClusterMoveParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stepWithParams:(MTRLevelControlClusterStepParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopWithParams:(MTRLevelControlClusterStopParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveToLevelWithOnOffWithParams:(MTRLevelControlClusterMoveToLevelWithOnOffParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveWithOnOffWithParams:(MTRLevelControlClusterMoveWithOnOffParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stepWithOnOffWithParams:(MTRLevelControlClusterStepWithOnOffParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopWithOnOffWithParams:(MTRLevelControlClusterStopWithOnOffParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveToClosestFrequencyWithParams:(MTRLevelControlClusterMoveToClosestFrequencyParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                              completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeCurrentLevelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRemainingTimeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinLevelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxLevelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentFrequencyWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinFrequencyWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxFrequencyWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOptionsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOptionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOptionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOnOffTransitionTimeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnOffTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnOffTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOnLevelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOnTransitionTimeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOffTransitionTimeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOffTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOffTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDefaultMoveRateWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDefaultMoveRateWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDefaultMoveRateWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeStartUpCurrentLevelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpCurrentLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpCurrentLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Binary Input (Basic)
 *    An interface for reading the value of a binary measurement and accessing various characteristics of that measurement.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterBinaryInputBasic : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeActiveTextWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeActiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeActiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDescriptionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDescriptionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDescriptionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeInactiveTextWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeInactiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeInactiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOutOfServiceWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOutOfServiceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOutOfServiceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePolarityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePresentValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePresentValueWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePresentValueWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeReliabilityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeReliabilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeReliabilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeStatusFlagsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeApplicationTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Pulse Width Modulation
 *    Cluster to control pulse width modulation
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterPulseWidthModulation : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Descriptor
 *    The Descriptor Cluster is meant to replace the support from the Zigbee Device Object (ZDO) for describing a node, its
 * endpoints and clusters.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterDescriptor : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeDeviceTypeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeServerListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClientListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePartsListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Binding
 *    The Binding Cluster is meant to replace the support from the Zigbee Device Object (ZDO) for supporting the binding table.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterBinding : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeBindingWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBindingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBindingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

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
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterAccessControl : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeACLWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeACLWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeACLWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeExtensionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeExtensionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeExtensionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSubjectsPerAccessControlEntryWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTargetsPerAccessControlEntryWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAccessControlEntriesPerFabricWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Actions
 *    This cluster provides a standardized way for a Node (typically a Bridge, but could be any Node) to expose action information.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterActions : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)instantActionWithParams:(MTRActionsClusterInstantActionParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)instantActionWithTransitionWithParams:(MTRActionsClusterInstantActionWithTransitionParams *)params
                               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                   completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)startActionWithParams:(MTRActionsClusterStartActionParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)startActionWithDurationWithParams:(MTRActionsClusterStartActionWithDurationParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopActionWithParams:(MTRActionsClusterStopActionParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)pauseActionWithParams:(MTRActionsClusterPauseActionParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)pauseActionWithDurationWithParams:(MTRActionsClusterPauseActionWithDurationParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)resumeActionWithParams:(MTRActionsClusterResumeActionParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enableActionWithParams:(MTRActionsClusterEnableActionParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enableActionWithDurationWithParams:(MTRActionsClusterEnableActionWithDurationParams *)params
                            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)disableActionWithParams:(MTRActionsClusterDisableActionParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)disableActionWithDurationWithParams:(MTRActionsClusterDisableActionWithDurationParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                 completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeActionListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEndpointListsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSetupURLWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Basic Information
 *    This cluster provides attributes and events for determining basic information about Nodes, which supports both
      Commissioning and operational determination of Node characteristics, such as Vendor ID, Product ID and serial number,
      which apply to the whole Node. Also allows setting user device information such as location.
 */
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRClusterBasicInformation : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeDataModelRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeVendorNameWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeVendorIDWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeProductNameWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeProductIDWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNodeLabelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNodeLabelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNodeLabelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeLocationWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLocationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLocationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeHardwareVersionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeHardwareVersionStringWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeSoftwareVersionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeSoftwareVersionStringWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeManufacturingDateWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributePartNumberWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeProductURLWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeProductLabelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeSerialNumberWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeLocalConfigDisabledWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLocalConfigDisabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLocalConfigDisabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeReachableWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeUniqueIDWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeCapabilityMinimaWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeProductAppearanceWithParams:(MTRReadParams * _Nullable)params MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster OTA Software Update Provider
 *    Provides an interface for providing OTA software updates
 */
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRClusterOTASoftwareUpdateProvider : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)queryImageWithParams:(MTROTASoftwareUpdateProviderClusterQueryImageParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(void (^)(MTROTASoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                                 NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)applyUpdateRequestWithParams:(MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)notifyUpdateAppliedWithParams:(MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster OTA Software Update Requestor
 *    Provides an interface for downloading and applying OTA software updates
 */
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRClusterOTASoftwareUpdateRequestor : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)announceOTAProviderWithParams:(MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeDefaultOTAProvidersWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeDefaultOTAProvidersWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeDefaultOTAProvidersWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeUpdatePossibleWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeUpdateStateWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeUpdateStateProgressWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterLocalizationConfiguration : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeActiveLocaleWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeActiveLocaleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeActiveLocaleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSupportedLocalesWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

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
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterTimeFormatLocalization : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeHourFormatWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeHourFormatWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeHourFormatWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActiveCalendarTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeActiveCalendarTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeActiveCalendarTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                           params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSupportedCalendarTypesWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

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
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterUnitLocalization : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeTemperatureUnitWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureUnitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureUnitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Power Source Configuration
 *    This cluster is used to describe the configuration and capabilities of a Device's power system.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterPowerSourceConfiguration : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeSourcesWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Power Source
 *    This cluster is used to describe the configuration and capabilities of a physical power source that provides power to the
 * Node.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterPowerSource : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeStatusWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOrderWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDescriptionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWiredAssessedInputVoltageWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWiredAssessedInputFrequencyWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWiredCurrentTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWiredAssessedCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWiredNominalVoltageWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWiredMaximumCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWiredPresentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActiveWiredFaultsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatVoltageWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatPercentRemainingWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatTimeRemainingWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatChargeLevelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatReplacementNeededWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatReplaceabilityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatPresentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActiveBatFaultsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatReplacementDescriptionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatCommonDesignationWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatANSIDesignationWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatIECDesignationWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatApprovedChemistryWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatCapacityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatQuantityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatChargeStateWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatTimeToFullChargeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatFunctionalWhileChargingWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBatChargingCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActiveBatChargeFaultsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster General Commissioning
 *    This cluster is used to manage global aspects of the Commissioning flow.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterGeneralCommissioning : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)armFailSafeWithParams:(MTRGeneralCommissioningClusterArmFailSafeParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(void (^)(MTRGeneralCommissioningClusterArmFailSafeResponseParams * _Nullable data,
                                  NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setRegulatoryConfigWithParams:(MTRGeneralCommissioningClusterSetRegulatoryConfigParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(void (^)(MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams * _Nullable data,
                                          NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)commissioningCompleteWithParams:(MTRGeneralCommissioningClusterCommissioningCompleteParams * _Nullable)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completion:
                                 (void (^)(MTRGeneralCommissioningClusterCommissioningCompleteResponseParams * _Nullable data,
                                     NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)commissioningCompleteWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     completion:
                                         (void (^)(
                                             MTRGeneralCommissioningClusterCommissioningCompleteResponseParams * _Nullable data,
                                             NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeBreadcrumbWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBreadcrumbWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBreadcrumbWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBasicCommissioningInfoWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRegulatoryConfigWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLocationCapabilityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSupportsConcurrentConnectionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Network Commissioning
 *    Functionality to configure, enable, disable network credentials and access on a Matter device.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterNetworkCommissioning : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)scanNetworksWithParams:(MTRNetworkCommissioningClusterScanNetworksParams * _Nullable)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(void (^)(MTRNetworkCommissioningClusterScanNetworksResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)addOrUpdateWiFiNetworkWithParams:(MTRNetworkCommissioningClusterAddOrUpdateWiFiNetworkParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                              completion:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                             NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)addOrUpdateThreadNetworkWithParams:(MTRNetworkCommissioningClusterAddOrUpdateThreadNetworkParams *)params
                            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                completion:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                               NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)removeNetworkWithParams:(MTRNetworkCommissioningClusterRemoveNetworkParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                    NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)connectNetworkWithParams:(MTRNetworkCommissioningClusterConnectNetworkParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(void (^)(MTRNetworkCommissioningClusterConnectNetworkResponseParams * _Nullable data,
                                     NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)reorderNetworkWithParams:(MTRNetworkCommissioningClusterReorderNetworkParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                     NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeMaxNetworksWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNetworksWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeScanMaxTimeSecondsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeConnectMaxTimeSecondsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeInterfaceEnabledWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeInterfaceEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeInterfaceEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLastNetworkingStatusWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLastNetworkIDWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLastConnectErrorValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Diagnostic Logs
 *    The cluster provides commands for retrieving unstructured diagnostic logs from a Node that may be used to aid in diagnostics.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterDiagnosticLogs : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)retrieveLogsRequestWithParams:(MTRDiagnosticLogsClusterRetrieveLogsRequestParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(void (^)(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable data,
                                          NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster General Diagnostics
 *    The General Diagnostics Cluster, along with other diagnostics clusters, provide a means to acquire standardized diagnostics
 * metrics that MAY be used by a Node to assist a user or Administrative Node in diagnosing potential problems.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterGeneralDiagnostics : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)testEventTriggerWithParams:(MTRGeneralDiagnosticsClusterTestEventTriggerParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNetworkInterfacesWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRebootCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeUpTimeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTotalOperationalHoursWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBootReasonWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeActiveHardwareFaultsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActiveRadioFaultsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActiveNetworkFaultsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTestEventTriggersEnabledWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Software Diagnostics
 *    The Software Diagnostics Cluster provides a means to acquire standardized diagnostics metrics that MAY be used by a Node to
 * assist a user or Administrative Node in diagnosing potential problems.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterSoftwareDiagnostics : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)resetWatermarksWithParams:(MTRSoftwareDiagnosticsClusterResetWatermarksParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)resetWatermarksWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeThreadMetricsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentHeapFreeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentHeapUsedWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentHeapHighWatermarkWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Thread Network Diagnostics
 *    The Thread Network Diagnostics Cluster provides a means to acquire standardized diagnostics metrics that MAY be used by a Node
 * to assist a user or Administrative Node in diagnosing potential problems
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterThreadNetworkDiagnostics : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)resetCountsWithParams:(MTRThreadNetworkDiagnosticsClusterResetCountsParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeChannelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRoutingRoleWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNetworkNameWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePanIdWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeExtendedPanIdWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMeshLocalPrefixWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOverrunCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNeighborTableWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeRouteTableWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributePartitionIdWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWeightingWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDataVersionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeStableDataVersionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLeaderRouterIdWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDetachedRoleCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeChildRoleCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRouterRoleCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLeaderRoleCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAttachAttemptCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePartitionIdChangeCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBetterPartitionAttachAttemptCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeParentChangeCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxTotalCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxUnicastCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxBroadcastCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxAckRequestedCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxAckedCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxNoAckRequestedCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxDataCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxDataPollCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxBeaconCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxBeaconRequestCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxOtherCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxRetryCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxDirectMaxRetryExpiryCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxIndirectMaxRetryExpiryCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxErrCcaCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxErrAbortCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxErrBusyChannelCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxTotalCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxUnicastCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxBroadcastCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxDataCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxDataPollCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxBeaconCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxBeaconRequestCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxOtherCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxAddressFilteredCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxDestAddrFilteredCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxDuplicatedCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxErrNoFrameCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxErrUnknownNeighborCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxErrInvalidSrcAddrCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxErrSecCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxErrFcsCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRxErrOtherCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActiveTimestampWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePendingTimestampWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDelayWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSecurityPolicyWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeChannelPage0MaskWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOperationalDatasetComponentsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActiveNetworkFaultsListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster WiFi Network Diagnostics
 *    The Wi-Fi Network Diagnostics Cluster provides a means to acquire standardized diagnostics metrics that MAY be used by a Node
 * to assist a user or Administrative Node in diagnosing potential problems.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterWiFiNetworkDiagnostics : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)resetCountsWithParams:(MTRWiFiNetworkDiagnosticsClusterResetCountsParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeBSSIDWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeSecurityTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWiFiVersionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeChannelNumberWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRSSIWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeBeaconLostCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBeaconRxCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePacketMulticastRxCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePacketMulticastTxCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePacketUnicastRxCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePacketUnicastTxCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentMaxRateWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOverrunCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Ethernet Network Diagnostics
 *    The Ethernet Network Diagnostics Cluster provides a means to acquire standardized diagnostics metrics that MAY be used by a
 * Node to assist a user or Administrative Node in diagnosing potential problems.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterEthernetNetworkDiagnostics : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)resetCountsWithParams:(MTREthernetNetworkDiagnosticsClusterResetCountsParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributePHYRateWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFullDuplexWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePacketRxCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePacketTxCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTxErrCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCollisionCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOverrunCountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCarrierDetectWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTimeSinceResetWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Time Synchronization
 *    Accurate time is required for a number of reasons, including scheduling, display and validating
security materials.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterTimeSynchronization : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_PROVISIONALLY_AVAILABLE;

- (void)setUtcTimeWithParams:(MTRTimeSynchronizationClusterSetUtcTimeParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeUTCTimeWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGranularityWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeTimeSourceWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeTrustedTimeNodeIdWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeTrustedTimeNodeIdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeTrustedTimeNodeIdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeDefaultNtpWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeDefaultNtpWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeDefaultNtpWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeTimeZoneWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeTimeZoneWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeTimeZoneWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeDSTOffsetWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeDSTOffsetWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeDSTOffsetWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeLocalTimeWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeTimeZoneDatabaseWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeNtpServerPortWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Bridged Device Basic Information
 *    This Cluster serves two purposes towards a Node communicating with a Bridge: indicate that the functionality on
          the Endpoint where it is placed (and its Parts) is bridged from a non-CHIP technology; and provide a centralized
          collection of attributes that the Node MAY collect to aid in conveying information regarding the Bridged Device to a user,
          such as the vendor name, the model name, or user-assigned name.
 */
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRClusterBridgedDeviceBasicInformation : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeVendorNameWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeVendorIDWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeProductNameWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNodeLabelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNodeLabelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNodeLabelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeHardwareVersionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeHardwareVersionStringWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeSoftwareVersionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeSoftwareVersionStringWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeManufacturingDateWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributePartNumberWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeProductURLWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeProductLabelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeSerialNumberWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeReachableWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeUniqueIDWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeProductAppearanceWithParams:(MTRReadParams * _Nullable)params MTR_NEWLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

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
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterSwitch : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNumberOfPositionsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMultiPressMaxWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Administrator Commissioning
 *    Commands to trigger a Node to allow a new Administrator to commission it.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterAdministratorCommissioning : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)openCommissioningWindowWithParams:(MTRAdministratorCommissioningClusterOpenCommissioningWindowParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)openBasicCommissioningWindowWithParams:(MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams *)params
                                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                    completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)revokeCommissioningWithParams:(MTRAdministratorCommissioningClusterRevokeCommissioningParams * _Nullable)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)revokeCommissioningWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeWindowStatusWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAdminFabricIndexWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAdminVendorIdWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Operational Credentials
 *    This cluster is used to add or remove Operational Credentials on a Commissionee or Node, as well as manage the associated
 * Fabrics.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterOperationalCredentials : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)attestationRequestWithParams:(MTROperationalCredentialsClusterAttestationRequestParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTROperationalCredentialsClusterAttestationResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)certificateChainRequestWithParams:(MTROperationalCredentialsClusterCertificateChainRequestParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completion:(void (^)(MTROperationalCredentialsClusterCertificateChainResponseParams * _Nullable data,
                                              NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)CSRRequestWithParams:(MTROperationalCredentialsClusterCSRRequestParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(void (^)(MTROperationalCredentialsClusterCSRResponseParams * _Nullable data,
                                 NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)addNOCWithParams:(MTROperationalCredentialsClusterAddNOCParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                              NSError * _Nullable error))completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)updateNOCWithParams:(MTROperationalCredentialsClusterUpdateNOCParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)updateFabricLabelWithParams:(MTROperationalCredentialsClusterUpdateFabricLabelParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completion:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                        NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)removeFabricWithParams:(MTROperationalCredentialsClusterRemoveFabricParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)addTrustedRootCertificateWithParams:(MTROperationalCredentialsClusterAddTrustedRootCertificateParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                 completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNOCsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFabricsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSupportedFabricsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCommissionedFabricsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTrustedRootCertificatesWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentFabricIndexWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Group Key Management
 *    The Group Key Management Cluster is the mechanism by which group keys are managed.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterGroupKeyManagement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)keySetWriteWithParams:(MTRGroupKeyManagementClusterKeySetWriteParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)keySetReadWithParams:(MTRGroupKeyManagementClusterKeySetReadParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(void (^)(MTRGroupKeyManagementClusterKeySetReadResponseParams * _Nullable data,
                                 NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)keySetRemoveWithParams:(MTRGroupKeyManagementClusterKeySetRemoveParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)keySetReadAllIndicesWithParams:(MTRGroupKeyManagementClusterKeySetReadAllIndicesParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(void (^)(MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams * _Nullable data,
                                           NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeGroupKeyMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeGroupKeyMapWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeGroupKeyMapWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGroupTableWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxGroupsPerFabricWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxGroupKeysPerFabricWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Fixed Label
 *    The Fixed Label Cluster provides a feature for the device to tag an endpoint with zero or more read only
labels.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterFixedLabel : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeLabelListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster User Label
 *    The User Label Cluster provides a feature to tag an endpoint with zero or more labels.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterUserLabel : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeLabelListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLabelListWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLabelListWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Proxy Configuration
 *    Cluster to control Proxy Configuration
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterProxyConfiguration : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Proxy Discovery
 *    Cluster to control Proxy Discovery
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterProxyDiscovery : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Proxy Valid
 *    Cluster to control Proxy Valid
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterProxyValid : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Boolean State
 *    This cluster provides an interface to a boolean state called StateValue.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterBooleanState : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeStateValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Mode Select
 *    Attributes and commands for selecting a mode from a list of supported options.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterModeSelect : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)changeToModeWithParams:(MTRModeSelectClusterChangeToModeParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeDescriptionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeStandardNamespaceWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSupportedModesWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeStartUpModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOnModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Door Lock
 *    An interface to a generic way to secure a door
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterDoorLock : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)lockDoorWithParams:(MTRDoorLockClusterLockDoorParams * _Nullable)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)unlockDoorWithParams:(MTRDoorLockClusterUnlockDoorParams * _Nullable)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)unlockWithTimeoutWithParams:(MTRDoorLockClusterUnlockWithTimeoutParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setWeekDayScheduleWithParams:(MTRDoorLockClusterSetWeekDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getWeekDayScheduleWithParams:(MTRDoorLockClusterGetWeekDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTRDoorLockClusterGetWeekDayScheduleResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearWeekDayScheduleWithParams:(MTRDoorLockClusterClearWeekDayScheduleParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setYearDayScheduleWithParams:(MTRDoorLockClusterSetYearDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getYearDayScheduleWithParams:(MTRDoorLockClusterGetYearDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTRDoorLockClusterGetYearDayScheduleResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearYearDayScheduleWithParams:(MTRDoorLockClusterClearYearDayScheduleParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setHolidayScheduleWithParams:(MTRDoorLockClusterSetHolidayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getHolidayScheduleWithParams:(MTRDoorLockClusterGetHolidayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(void (^)(MTRDoorLockClusterGetHolidayScheduleResponseParams * _Nullable data,
                                         NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearHolidayScheduleWithParams:(MTRDoorLockClusterClearHolidayScheduleParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setUserWithParams:(MTRDoorLockClusterSetUserParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getUserWithParams:(MTRDoorLockClusterGetUserParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRDoorLockClusterGetUserResponseParams * _Nullable data, NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearUserWithParams:(MTRDoorLockClusterClearUserParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setCredentialWithParams:(MTRDoorLockClusterSetCredentialParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(void (^)(MTRDoorLockClusterSetCredentialResponseParams * _Nullable data,
                                    NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getCredentialStatusWithParams:(MTRDoorLockClusterGetCredentialStatusParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(void (^)(MTRDoorLockClusterGetCredentialStatusResponseParams * _Nullable data,
                                          NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearCredentialWithParams:(MTRDoorLockClusterClearCredentialParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeLockStateWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLockTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActuatorEnabledWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDoorStateWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDoorOpenEventsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDoorOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDoorOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDoorClosedEventsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDoorClosedEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDoorClosedEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOpenPeriodWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNumberOfTotalUsersSupportedWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNumberOfPINUsersSupportedWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNumberOfRFIDUsersSupportedWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNumberOfWeekDaySchedulesSupportedPerUserWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNumberOfYearDaySchedulesSupportedPerUserWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNumberOfHolidaySchedulesSupportedWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxPINCodeLengthWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinPINCodeLengthWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxRFIDCodeLengthWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinRFIDCodeLengthWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCredentialRulesSupportWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNumberOfCredentialsSupportedPerUserWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLanguageWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLanguageWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLanguageWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLEDSettingsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLEDSettingsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLEDSettingsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAutoRelockTimeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeAutoRelockTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeAutoRelockTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSoundVolumeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSoundVolumeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSoundVolumeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOperatingModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOperatingModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOperatingModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSupportedOperatingModesWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDefaultConfigurationRegisterWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEnableLocalProgrammingWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnableLocalProgrammingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnableLocalProgrammingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEnableOneTouchLockingWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnableOneTouchLockingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnableOneTouchLockingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                              params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEnableInsideStatusLEDWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnableInsideStatusLEDWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnableInsideStatusLEDWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                              params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEnablePrivacyModeButtonWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnablePrivacyModeButtonWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnablePrivacyModeButtonWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLocalProgrammingFeaturesWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLocalProgrammingFeaturesWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLocalProgrammingFeaturesWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWrongCodeEntryLimitWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWrongCodeEntryLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWrongCodeEntryLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeUserCodeTemporaryDisableTimeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUserCodeTemporaryDisableTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUserCodeTemporaryDisableTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSendPINOverTheAirWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSendPINOverTheAirWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSendPINOverTheAirWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRequirePINforRemoteOperationWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRequirePINforRemoteOperationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRequirePINforRemoteOperationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeExpiringUserTimeoutWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeExpiringUserTimeoutWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeExpiringUserTimeoutWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Window Covering
 *    Provides an interface for controlling and adjusting automatic window coverings.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterWindowCovering : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)upOrOpenWithParams:(MTRWindowCoveringClusterUpOrOpenParams * _Nullable)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)upOrOpenWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)downOrCloseWithParams:(MTRWindowCoveringClusterDownOrCloseParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)downOrCloseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopMotionWithParams:(MTRWindowCoveringClusterStopMotionParams * _Nullable)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopMotionWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)goToLiftValueWithParams:(MTRWindowCoveringClusterGoToLiftValueParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)goToLiftPercentageWithParams:(MTRWindowCoveringClusterGoToLiftPercentageParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)goToTiltValueWithParams:(MTRWindowCoveringClusterGoToTiltValueParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)goToTiltPercentageWithParams:(MTRWindowCoveringClusterGoToTiltPercentageParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePhysicalClosedLimitLiftWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePhysicalClosedLimitTiltWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionLiftWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionTiltWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNumberOfActuationsLiftWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNumberOfActuationsTiltWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeConfigStatusWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionLiftPercentageWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionTiltPercentageWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOperationalStatusWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTargetPositionLiftPercent100thsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTargetPositionTiltPercent100thsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEndProductTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionLiftPercent100thsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentPositionTiltPercent100thsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeInstalledOpenLimitLiftWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeInstalledClosedLimitLiftWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeInstalledOpenLimitTiltWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeInstalledClosedLimitTiltWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                             params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSafetyStatusWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Barrier Control
 *    This cluster provides control of a barrier (garage door).
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterBarrierControl : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)barrierControlGoToPercentWithParams:(MTRBarrierControlClusterBarrierControlGoToPercentParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                 completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)barrierControlStopWithParams:(MTRBarrierControlClusterBarrierControlStopParams * _Nullable)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)barrierControlStopWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeBarrierMovingStateWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBarrierSafetyStatusWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBarrierCapabilitiesWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBarrierOpenEventsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBarrierOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBarrierOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBarrierCloseEventsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBarrierCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBarrierCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                           params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBarrierCommandOpenEventsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBarrierCommandCloseEventsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                  params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBarrierOpenPeriodWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBarrierClosePeriodWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBarrierClosePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBarrierClosePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                           params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBarrierPositionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Pump Configuration and Control
 *    An interface for configuring and controlling pumps.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterPumpConfigurationAndControl : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeMaxPressureWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxSpeedWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxFlowWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinConstPressureWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxConstPressureWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinCompPressureWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxCompPressureWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinConstSpeedWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxConstSpeedWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinConstFlowWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxConstFlowWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinConstTempWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxConstTempWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePumpStatusWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEffectiveOperationModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEffectiveControlModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCapacityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSpeedWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLifetimeRunningHoursWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLifetimeRunningHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLifetimeRunningHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePowerWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLifetimeEnergyConsumedWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLifetimeEnergyConsumedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLifetimeEnergyConsumedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOperationModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOperationModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOperationModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeControlModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeControlModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeControlModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Thermostat
 *    An interface for configuring and controlling the functionality of a thermostat.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterThermostat : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)setpointRaiseLowerWithParams:(MTRThermostatClusterSetpointRaiseLowerParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setWeeklyScheduleWithParams:(MTRThermostatClusterSetWeeklyScheduleParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getWeeklyScheduleWithParams:(MTRThermostatClusterGetWeeklyScheduleParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completion:(void (^)(MTRThermostatClusterGetWeeklyScheduleResponseParams * _Nullable data,
                                        NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearWeeklyScheduleWithParams:(MTRThermostatClusterClearWeeklyScheduleParams * _Nullable)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearWeeklyScheduleWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeLocalTemperatureWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOutdoorTemperatureWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOccupancyWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAbsMinHeatSetpointLimitWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAbsMaxHeatSetpointLimitWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAbsMinCoolSetpointLimitWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAbsMaxCoolSetpointLimitWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePICoolingDemandWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePIHeatingDemandWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeHVACSystemTypeConfigurationWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeHVACSystemTypeConfigurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeHVACSystemTypeConfigurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLocalTemperatureCalibrationWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLocalTemperatureCalibrationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLocalTemperatureCalibrationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOccupiedCoolingSetpointWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOccupiedCoolingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOccupiedCoolingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOccupiedHeatingSetpointWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOccupiedHeatingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOccupiedHeatingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeUnoccupiedCoolingSetpointWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUnoccupiedCoolingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUnoccupiedCoolingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                  params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeUnoccupiedHeatingSetpointWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUnoccupiedHeatingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUnoccupiedHeatingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                  params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinHeatSetpointLimitWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinHeatSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinHeatSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxHeatSetpointLimitWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMaxHeatSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMaxHeatSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinCoolSetpointLimitWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinCoolSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinCoolSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxCoolSetpointLimitWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMaxCoolSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMaxCoolSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinSetpointDeadBandWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinSetpointDeadBandWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinSetpointDeadBandWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRemoteSensingWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRemoteSensingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRemoteSensingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeControlSequenceOfOperationWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeControlSequenceOfOperationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeControlSequenceOfOperationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                   params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSystemModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSystemModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSystemModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeThermostatRunningModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeStartOfWeekWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNumberOfWeeklyTransitionsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNumberOfDailyTransitionsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTemperatureSetpointHoldWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureSetpointHoldWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureSetpointHoldWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTemperatureSetpointHoldDurationWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureSetpointHoldDurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureSetpointHoldDurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                        params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeThermostatProgrammingOperationModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeThermostatProgrammingOperationModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeThermostatProgrammingOperationModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                           params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeThermostatRunningStateWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSetpointChangeSourceWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSetpointChangeAmountWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSetpointChangeSourceTimestampWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOccupiedSetbackWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOccupiedSetbackWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOccupiedSetbackWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOccupiedSetbackMinWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOccupiedSetbackMaxWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeUnoccupiedSetbackWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUnoccupiedSetbackWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUnoccupiedSetbackWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeUnoccupiedSetbackMinWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeUnoccupiedSetbackMaxWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEmergencyHeatDeltaWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEmergencyHeatDeltaWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEmergencyHeatDeltaWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                           params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeACTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeACCapacityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACCapacityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACCapacityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeACRefrigerantTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACRefrigerantTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACRefrigerantTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeACCompressorTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACCompressorTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACCompressorTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeACErrorCodeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACErrorCodeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACErrorCodeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeACLouverPositionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACLouverPositionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACLouverPositionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeACCoilTemperatureWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeACCapacityformatWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACCapacityformatWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACCapacityformatWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Fan Control
 *    An interface for controlling a fan in a heating/cooling system.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterFanControl : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeFanModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeFanModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeFanModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFanModeSequenceWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeFanModeSequenceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeFanModeSequenceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePercentSettingWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePercentSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePercentSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePercentCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSpeedMaxWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSpeedSettingWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSpeedSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSpeedSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSpeedCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRockSupportWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRockSettingWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRockSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRockSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWindSupportWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWindSettingWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWindSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWindSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Thermostat User Interface Configuration
 *    An interface for configuring the user interface of a thermostat (which may be remote from the thermostat).
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterThermostatUserInterfaceConfiguration : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeTemperatureDisplayModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureDisplayModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureDisplayModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeKeypadLockoutWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeKeypadLockoutWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeKeypadLockoutWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeScheduleProgrammingVisibilityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeScheduleProgrammingVisibilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeScheduleProgrammingVisibilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Color Control
 *    Attributes and commands for controlling the color properties of a color-capable light.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterColorControl : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)moveToHueWithParams:(MTRColorControlClusterMoveToHueParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveHueWithParams:(MTRColorControlClusterMoveHueParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stepHueWithParams:(MTRColorControlClusterStepHueParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveToSaturationWithParams:(MTRColorControlClusterMoveToSaturationParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveSaturationWithParams:(MTRColorControlClusterMoveSaturationParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stepSaturationWithParams:(MTRColorControlClusterStepSaturationParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveToHueAndSaturationWithParams:(MTRColorControlClusterMoveToHueAndSaturationParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                              completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveToColorWithParams:(MTRColorControlClusterMoveToColorParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveColorWithParams:(MTRColorControlClusterMoveColorParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stepColorWithParams:(MTRColorControlClusterStepColorParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveToColorTemperatureWithParams:(MTRColorControlClusterMoveToColorTemperatureParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                              completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enhancedMoveToHueWithParams:(MTRColorControlClusterEnhancedMoveToHueParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enhancedMoveHueWithParams:(MTRColorControlClusterEnhancedMoveHueParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enhancedStepHueWithParams:(MTRColorControlClusterEnhancedStepHueParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enhancedMoveToHueAndSaturationWithParams:(MTRColorControlClusterEnhancedMoveToHueAndSaturationParams *)params
                                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                      completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)colorLoopSetWithParams:(MTRColorControlClusterColorLoopSetParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopMoveStepWithParams:(MTRColorControlClusterStopMoveStepParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveColorTemperatureWithParams:(MTRColorControlClusterMoveColorTemperatureParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stepColorTemperatureWithParams:(MTRColorControlClusterStepColorTemperatureParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeCurrentHueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentSaturationWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRemainingTimeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentXWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentYWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDriftCompensationWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCompensationTextWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorTemperatureMiredsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOptionsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOptionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOptionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNumberOfPrimariesWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary1XWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary1YWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary1IntensityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary2XWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary2YWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary2IntensityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary3XWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary3YWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary3IntensityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary4XWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary4YWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary4IntensityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary5XWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary5YWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary5IntensityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary6XWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary6YWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePrimary6IntensityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWhitePointXWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWhitePointXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWhitePointXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeWhitePointYWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWhitePointYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWhitePointYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorPointRXWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointRXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointRXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorPointRYWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointRYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointRYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorPointRIntensityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointRIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointRIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorPointGXWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointGXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointGXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorPointGYWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointGYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointGYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorPointGIntensityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointGIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointGIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorPointBXWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointBXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointBXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorPointBYWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointBYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointBYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorPointBIntensityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointBIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointBIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEnhancedCurrentHueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEnhancedColorModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorLoopActiveWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorLoopDirectionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorLoopTimeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorLoopStartEnhancedHueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorLoopStoredEnhancedHueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorCapabilitiesWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorTempPhysicalMinMiredsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeColorTempPhysicalMaxMiredsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCoupleColorTempToLevelMinMiredsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeStartUpColorTemperatureMiredsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpColorTemperatureMiredsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpColorTemperatureMiredsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Ballast Configuration
 *    Attributes and commands for configuring a lighting ballast.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterBallastConfiguration : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributePhysicalMinLevelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePhysicalMaxLevelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeBallastStatusWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinLevelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxLevelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMaxLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMaxLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeIntrinsicBallastFactorWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeIntrinsicBallastFactorWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeIntrinsicBallastFactorWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeBallastFactorAdjustmentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBallastFactorAdjustmentWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBallastFactorAdjustmentWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLampQuantityWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLampTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLampManufacturerWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampManufacturerWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampManufacturerWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLampRatedHoursWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampRatedHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampRatedHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLampBurnHoursWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampBurnHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampBurnHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLampAlarmModeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampAlarmModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampAlarmModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLampBurnHoursTripPointWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampBurnHoursTripPointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampBurnHoursTripPointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Illuminance Measurement
 *    Attributes and commands for configuring the measurement of illuminance, and reporting illuminance measurements.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterIlluminanceMeasurement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLightSensorTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Temperature Measurement
 *    Attributes and commands for configuring the measurement of temperature, and reporting temperature measurements.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterTemperatureMeasurement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Pressure Measurement
 *    Attributes and commands for configuring the measurement of pressure, and reporting pressure measurements.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterPressureMeasurement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeScaledValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinScaledValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxScaledValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeScaledToleranceWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeScaleWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Flow Measurement
 *    Attributes and commands for configuring the measurement of flow, and reporting flow measurements.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterFlowMeasurement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Relative Humidity Measurement
 *    Attributes and commands for configuring the measurement of relative humidity, and reporting relative humidity measurements.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterRelativeHumidityMeasurement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Occupancy Sensing
 *    Attributes and commands for configuring occupancy sensing, and reporting occupancy status.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterOccupancySensing : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeOccupancyWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOccupancySensorTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOccupancySensorTypeBitmapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePIROccupiedToUnoccupiedDelayWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributePIROccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributePIROccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributePIRUnoccupiedToOccupiedDelayWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributePIRUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributePIRUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributePIRUnoccupiedToOccupiedThresholdWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributePIRUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributePIRUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                         params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeUltrasonicOccupiedToUnoccupiedDelayWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUltrasonicOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUltrasonicOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeUltrasonicUnoccupiedToOccupiedDelayWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUltrasonicUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUltrasonicUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeUltrasonicUnoccupiedToOccupiedThresholdWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUltrasonicUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUltrasonicUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePhysicalContactOccupiedToUnoccupiedDelayWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePhysicalContactOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePhysicalContactOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePhysicalContactUnoccupiedToOccupiedDelayWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePhysicalContactUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePhysicalContactUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePhysicalContactUnoccupiedToOccupiedThresholdWithParams:
    (MTRReadParams * _Nullable)params API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePhysicalContactUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePhysicalContactUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Wake on LAN
 *    This cluster provides an interface for managing low power mode on a device that supports the Wake On LAN protocol.
 */
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRClusterWakeOnLAN : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeMACAddressWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Channel
 *    This cluster provides an interface for controlling the current Channel on a device.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterChannel : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)changeChannelWithParams:(MTRChannelClusterChangeChannelParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(void (^)(MTRChannelClusterChangeChannelResponseParams * _Nullable data,
                                    NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)changeChannelByNumberWithParams:(MTRChannelClusterChangeChannelByNumberParams *)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)skipChannelWithParams:(MTRChannelClusterSkipChannelParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeChannelListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLineupWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentChannelWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Target Navigator
 *    This cluster provides an interface for UX navigation within a set of targets on a device or endpoint.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterTargetNavigator : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)navigateTargetWithParams:(MTRTargetNavigatorClusterNavigateTargetParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(void (^)(MTRTargetNavigatorClusterNavigateTargetResponseParams * _Nullable data,
                                     NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeTargetListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentTargetWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Media Playback
 *    This cluster provides an interface for controlling Media Playback (PLAY, PAUSE, etc) on a media device such as a TV or
 * Speaker.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterMediaPlayback : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)playWithParams:(MTRMediaPlaybackClusterPlayParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                              NSError * _Nullable error))completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)playWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)pauseWithParams:(MTRMediaPlaybackClusterPauseParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                              NSError * _Nullable error))completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)pauseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                     completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                    NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopWithParams:(MTRMediaPlaybackClusterStopParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                              NSError * _Nullable error))completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)startOverWithParams:(MTRMediaPlaybackClusterStartOverParams * _Nullable)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:
                     (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)startOverWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                         completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                        NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)previousWithParams:(MTRMediaPlaybackClusterPreviousParams * _Nullable)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completion:
                    (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)previousWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                       NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)nextWithParams:(MTRMediaPlaybackClusterNextParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                              NSError * _Nullable error))completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)nextWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)rewindWithParams:(MTRMediaPlaybackClusterRewindParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                              NSError * _Nullable error))completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)rewindWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                      completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                     NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)fastForwardWithParams:(MTRMediaPlaybackClusterFastForwardParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                  NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)fastForwardWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                          NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)skipForwardWithParams:(MTRMediaPlaybackClusterSkipForwardParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                  NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)skipBackwardWithParams:(MTRMediaPlaybackClusterSkipBackwardParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)seekWithParams:(MTRMediaPlaybackClusterSeekParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                              NSError * _Nullable error))completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeCurrentStateWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeStartTimeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDurationWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSampledPositionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePlaybackSpeedWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSeekRangeEndWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSeekRangeStartWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Media Input
 *    This cluster provides an interface for controlling the Input Selector on a media device such as a TV.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterMediaInput : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)selectInputWithParams:(MTRMediaInputClusterSelectInputParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)showInputStatusWithParams:(MTRMediaInputClusterShowInputStatusParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)showInputStatusWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)hideInputStatusWithParams:(MTRMediaInputClusterHideInputStatusParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)hideInputStatusWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)renameInputWithParams:(MTRMediaInputClusterRenameInputParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInputListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentInputWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Low Power
 *    This cluster provides an interface for managing low power mode on a device.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterLowPower : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)sleepWithParams:(MTRLowPowerClusterSleepParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)sleepWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                     completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Keypad Input
 *    This cluster provides an interface for controlling a device like a TV using action commands such as UP, DOWN, and SELECT.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterKeypadInput : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)sendKeyWithParams:(MTRKeypadInputClusterSendKeyParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRKeypadInputClusterSendKeyResponseParams * _Nullable data,
                              NSError * _Nullable error))completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Content Launcher
 *    This cluster provides an interface for launching content on a media player device such as a TV or Speaker.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterContentLauncher : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)launchContentWithParams:(MTRContentLauncherClusterLaunchContentParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completion:(void (^)(MTRContentLauncherClusterLauncherResponseParams * _Nullable data,
                                    NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)launchURLWithParams:(MTRContentLauncherClusterLaunchURLParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(void (^)(MTRContentLauncherClusterLauncherResponseParams * _Nullable data,
                                NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeAcceptHeaderWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeSupportedStreamingProtocolsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSupportedStreamingProtocolsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSupportedStreamingProtocolsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Audio Output
 *    This cluster provides an interface for controlling the Output on a media device such as a TV.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterAudioOutput : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)selectOutputWithParams:(MTRAudioOutputClusterSelectOutputParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)renameOutputWithParams:(MTRAudioOutputClusterRenameOutputParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeOutputListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentOutputWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Application Launcher
 *    This cluster provides an interface for launching content on a media player device such as a TV or Speaker.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterApplicationLauncher : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)launchAppWithParams:(MTRApplicationLauncherClusterLaunchAppParams * _Nullable)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completion:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data,
                                NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopAppWithParams:(MTRApplicationLauncherClusterStopAppParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data,
                              NSError * _Nullable error))completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)hideAppWithParams:(MTRApplicationLauncherClusterHideAppParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data,
                              NSError * _Nullable error))completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeCatalogListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentAppWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeCurrentAppWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeCurrentAppWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Application Basic
 *    This cluster provides information about an application running on a TV or media player device which is represented as an
 * endpoint.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterApplicationBasic : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeVendorNameWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeVendorIDWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeApplicationNameWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeProductIDWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeApplicationWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeStatusWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeApplicationVersionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAllowedVendorListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Account Login
 *    This cluster provides commands that facilitate user account login on a Content App or a node. For example, a Content App
 * running on a Video Player device, which is represented as an endpoint (see [TV Architecture]), can use this cluster to help make
 * the user account on the Content App match the user account on the Client.
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterAccountLogin : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)getSetupPINWithParams:(MTRAccountLoginClusterGetSetupPINParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(void (^)(MTRAccountLoginClusterGetSetupPINResponseParams * _Nullable data,
                                  NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)loginWithParams:(MTRAccountLoginClusterLoginParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)logoutWithParams:(MTRAccountLoginClusterLogoutParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)logoutWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                      completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Electrical Measurement
 *    Attributes related to the electrical properties of a device. This cluster is used by power outlets and other devices that need
 * to provide instantaneous data as opposed to metrology data which should be retrieved from the metering cluster..
 */
API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterElectricalMeasurement : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)getProfileInfoCommandWithParams:(MTRElectricalMeasurementClusterGetProfileInfoCommandParams * _Nullable)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getProfileInfoCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                     completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getMeasurementProfileCommandWithParams:(MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams *)params
                                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                    completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeMeasurementTypeWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcVoltageWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcVoltageMinWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcVoltageMaxWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcCurrentMinWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcCurrentMaxWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcPowerWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcPowerMinWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcPowerMaxWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcVoltageMultiplierWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcVoltageDivisorWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcCurrentDivisorWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcPowerMultiplierWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeDcPowerDivisorWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcFrequencyWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcFrequencyMinWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcFrequencyMaxWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeNeutralCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTotalActivePowerWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTotalReactivePowerWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeTotalApparentPowerWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMeasured1stHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMeasured3rdHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMeasured5thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMeasured7thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMeasured9thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMeasured11thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMeasuredPhase1stHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMeasuredPhase3rdHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMeasuredPhase5thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMeasuredPhase7thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMeasuredPhase9thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeMeasuredPhase11thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcFrequencyMultiplierWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcFrequencyDivisorWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePowerMultiplierWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePowerDivisorWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeHarmonicCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePhaseHarmonicCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeInstantaneousVoltageWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeInstantaneousLineCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeInstantaneousActiveCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeInstantaneousReactiveCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeInstantaneousPowerWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageMinWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageMaxWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentMinWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentMaxWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActivePowerWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActivePowerMinWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActivePowerMaxWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeReactivePowerWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeApparentPowerWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePowerFactorWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsVoltageMeasurementPeriodWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                           params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsUnderVoltageCounterWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeOverVoltagePeriodWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeUnderVoltagePeriodWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSagPeriodWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSwellPeriodWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                              params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcVoltageMultiplierWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcVoltageDivisorWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcCurrentDivisorWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcPowerMultiplierWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcPowerDivisorWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeOverloadAlarmsMaskWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                           params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeVoltageOverloadWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeCurrentOverloadWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcOverloadAlarmsMaskWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcVoltageOverloadWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcCurrentOverloadWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcActivePowerOverloadWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcReactivePowerOverloadWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsOverVoltageWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsUnderVoltageWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeOverVoltageWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeUnderVoltageWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSagWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSwellWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLineCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActiveCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeReactiveCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltagePhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageMinPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageMaxPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentMinPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentMaxPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActivePowerPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActivePowerMinPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActivePowerMaxPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeReactivePowerPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeApparentPowerPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePowerFactorPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsOverVoltageCounterPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsUnderVoltageCounterPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeOverVoltagePeriodPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSagPeriodPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSwellPeriodPhaseBWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeLineCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActiveCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeReactiveCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltagePhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageMinPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageMaxPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentMinPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsCurrentMaxPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActivePowerPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActivePowerMinPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeActivePowerMaxPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeReactivePowerPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeApparentPowerPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributePowerFactorPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsOverVoltageCounterPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAverageRmsUnderVoltageCounterPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeOverVoltagePeriodPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSagPeriodPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeRmsVoltageSwellPeriodPhaseCWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Client Monitoring
 *    Client Monitoring allows for ensuring that listed clients meet the required monitoring conditions on the server.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterClientMonitoring : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_PROVISIONALLY_AVAILABLE;

- (void)registerClientMonitoringWithParams:(MTRClientMonitoringClusterRegisterClientMonitoringParams *)params
                            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)unregisterClientMonitoringWithParams:(MTRClientMonitoringClusterUnregisterClientMonitoringParams *)params
                              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                  completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)stayAwakeRequestWithParams:(MTRClientMonitoringClusterStayAwakeRequestParams * _Nullable)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)stayAwakeRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeIdleModeIntervalWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeActiveModeIntervalWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeActiveModeThresholdWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeExpectedClientsWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Unit Testing
 *    The Test Cluster is meant to validate the generated code
 */
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRClusterUnitTesting : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)testWithParams:(MTRUnitTestingClusterTestParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testNotHandledWithParams:(MTRUnitTestingClusterTestNotHandledParams * _Nullable)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completion:(MTRStatusCompletion)completion API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testNotHandledWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                              completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testSpecificWithParams:(MTRUnitTestingClusterTestSpecificParams * _Nullable)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completion:(void (^)(MTRUnitTestingClusterTestSpecificResponseParams * _Nullable data,
                                   NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testSpecificWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                            completion:(void (^)(MTRUnitTestingClusterTestSpecificResponseParams * _Nullable data,
                                           NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testUnknownCommandWithParams:(MTRUnitTestingClusterTestUnknownCommandParams * _Nullable)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testUnknownCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testAddArgumentsWithParams:(MTRUnitTestingClusterTestAddArgumentsParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completion:(void (^)(MTRUnitTestingClusterTestAddArgumentsResponseParams * _Nullable data,
                                       NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testSimpleArgumentRequestWithParams:(MTRUnitTestingClusterTestSimpleArgumentRequestParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                 completion:(void (^)(MTRUnitTestingClusterTestSimpleArgumentResponseParams * _Nullable data,
                                                NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testStructArrayArgumentRequestWithParams:(MTRUnitTestingClusterTestStructArrayArgumentRequestParams *)params
                                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                      completion:
                                          (void (^)(MTRUnitTestingClusterTestStructArrayArgumentResponseParams * _Nullable data,
                                              NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testStructArgumentRequestWithParams:(MTRUnitTestingClusterTestStructArgumentRequestParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                 completion:(void (^)(MTRUnitTestingClusterBooleanResponseParams * _Nullable data,
                                                NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testNestedStructArgumentRequestWithParams:(MTRUnitTestingClusterTestNestedStructArgumentRequestParams *)params
                                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                       completion:(void (^)(MTRUnitTestingClusterBooleanResponseParams * _Nullable data,
                                                      NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testListStructArgumentRequestWithParams:(MTRUnitTestingClusterTestListStructArgumentRequestParams *)params
                                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                     completion:(void (^)(MTRUnitTestingClusterBooleanResponseParams * _Nullable data,
                                                    NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testListInt8UArgumentRequestWithParams:(MTRUnitTestingClusterTestListInt8UArgumentRequestParams *)params
                                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                    completion:(void (^)(MTRUnitTestingClusterBooleanResponseParams * _Nullable data,
                                                   NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testNestedStructListArgumentRequestWithParams:(MTRUnitTestingClusterTestNestedStructListArgumentRequestParams *)params
                                       expectedValues:
                                           (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                           completion:(void (^)(MTRUnitTestingClusterBooleanResponseParams * _Nullable data,
                                                          NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testListNestedStructListArgumentRequestWithParams:
            (MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams *)params
                                           expectedValues:
                                               (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                               completion:(void (^)(MTRUnitTestingClusterBooleanResponseParams * _Nullable data,
                                                              NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testListInt8UReverseRequestWithParams:(MTRUnitTestingClusterTestListInt8UReverseRequestParams *)params
                               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                   completion:(void (^)(MTRUnitTestingClusterTestListInt8UReverseResponseParams * _Nullable data,
                                                  NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testEnumsRequestWithParams:(MTRUnitTestingClusterTestEnumsRequestParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completion:(void (^)(MTRUnitTestingClusterTestEnumsResponseParams * _Nullable data,
                                       NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testNullableOptionalRequestWithParams:(MTRUnitTestingClusterTestNullableOptionalRequestParams * _Nullable)params
                               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                   completion:(void (^)(MTRUnitTestingClusterTestNullableOptionalResponseParams * _Nullable data,
                                                  NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)
    testComplexNullableOptionalRequestWithParams:(MTRUnitTestingClusterTestComplexNullableOptionalRequestParams *)params
                                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                      completion:
                                          (void (^)(MTRUnitTestingClusterTestComplexNullableOptionalResponseParams * _Nullable data,
                                              NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)simpleStructEchoRequestWithParams:(MTRUnitTestingClusterSimpleStructEchoRequestParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completion:(void (^)(MTRUnitTestingClusterSimpleStructResponseParams * _Nullable data,
                                              NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)timedInvokeRequestWithParams:(MTRUnitTestingClusterTimedInvokeRequestParams * _Nullable)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)timedInvokeRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testSimpleOptionalArgumentRequestWithParams:(MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams * _Nullable)params
                                     expectedValues:
                                         (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                         completion:(MTRStatusCompletion)completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testEmitTestEventRequestWithParams:(MTRUnitTestingClusterTestEmitTestEventRequestParams *)params
                            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                completion:(void (^)(MTRUnitTestingClusterTestEmitTestEventResponseParams * _Nullable data,
                                               NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)
    testEmitTestFabricScopedEventRequestWithParams:(MTRUnitTestingClusterTestEmitTestFabricScopedEventRequestParams *)params
                                    expectedValues:
                                        (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                        completion:
                                            (void (^)(
                                                MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams * _Nullable data,
                                                NSError * _Nullable error))completion
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeBooleanWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeBitmap8WithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeBitmap16WithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeBitmap32WithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap32WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap32WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeBitmap64WithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap64WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap64WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt8uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                              params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt16uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt24uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt24uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt24uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt32uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt32uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt32uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt40uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt40uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt40uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt48uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt48uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt48uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt56uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt56uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt56uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt64uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt64uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt64uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt8sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                              params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt16sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt24sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt24sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt24sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt32sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt32sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt32sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt40sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt40sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt40sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt48sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt48sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt48sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt56sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt56sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt56sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeInt64sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt64sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt64sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeEnum8WithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEnum8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEnum8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                              params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeEnum16WithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEnum16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEnum16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeFloatSingleWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeFloatSingleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeFloatSingleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeFloatDoubleWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeFloatDoubleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeFloatDoubleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeOctetStringWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeListInt8uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                  params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeListOctetStringWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeListStructOctetStringWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListStructOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListStructOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                              params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeLongOctetStringWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLongOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLongOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeCharStringWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeLongCharStringWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLongCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLongCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeEpochUsWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEpochUsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEpochUsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeEpochSWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEpochSWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEpochSWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                               params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeVendorIdWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeVendorIdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeVendorIdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeListNullablesAndOptionalsStructWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListNullablesAndOptionalsStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListNullablesAndOptionalsStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                        params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeEnumAttrWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEnumAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEnumAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                  expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                 params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeStructAttrWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeStructAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeStructAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                   params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeRangeRestrictedInt8uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeRangeRestrictedInt8sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                             params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeRangeRestrictedInt16uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                              params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeRangeRestrictedInt16sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                              params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeListLongOctetStringWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListLongOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListLongOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeListFabricScopedWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListFabricScopedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListFabricScopedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeTimedWriteBooleanWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeTimedWriteBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeTimedWriteBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                          params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeGeneralErrorBooleanWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeGeneralErrorBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeGeneralErrorBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeClusterErrorBooleanWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeClusterErrorBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeClusterErrorBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeUnsupportedWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeUnsupportedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeUnsupportedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                     expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                    params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableBooleanWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableBitmap8WithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                        params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableBitmap16WithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableBitmap32WithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap32WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap32WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableBitmap64WithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap64WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap64WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt8uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt16uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt24uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt24uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt24uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt32uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt32uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt32uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt40uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt40uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt40uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt48uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt48uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt48uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt56uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt56uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt56uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt64uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt64uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt64uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt8sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt16sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt24sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt24sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt24sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt32sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt32sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt32sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt40sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt40sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt40sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt48sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt48sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt48sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt56sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt56sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt56sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableInt64sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt64sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt64sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableEnum8WithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableEnum8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableEnum8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableEnum16WithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableEnum16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableEnum16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableFloatSingleWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableFloatSingleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableFloatSingleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableFloatDoubleWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableFloatDoubleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableFloatDoubleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableOctetStringWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableCharStringWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                           params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableEnumAttrWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableEnumAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableEnumAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                         params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableStructWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableRangeRestrictedInt8uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableRangeRestrictedInt8sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableRangeRestrictedInt16uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeNullableRangeRestrictedInt16sWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                      params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeWriteOnlyInt8uWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeWriteOnlyInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeWriteOnlyInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                       params:(MTRWriteParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

/**
 * Cluster Fault Injection
 *    The Fault Injection Cluster provide a means for a test harness to configure faults(for example triggering a fault in the
 * system).
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterFaultInjection : MTRCluster

- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue NS_DESIGNATED_INITIALIZER MTR_PROVISIONALLY_AVAILABLE;

- (void)failAtFaultWithParams:(MTRFaultInjectionClusterFailAtFaultParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)failRandomlyAtFaultWithParams:(MTRFaultInjectionClusterFailRandomlyAtFaultParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                           completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeEventListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> *)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
    MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

MTR_DEPRECATED("Please use MTRClusterBasicInformation", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRClusterBasic : MTRClusterBasicInformation
@end

MTR_DEPRECATED(
    "Please use MTRClusterOTASoftwareUpdateProvider", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRClusterOtaSoftwareUpdateProvider : MTRClusterOTASoftwareUpdateProvider
@end

MTR_DEPRECATED(
    "Please use MTRClusterOTASoftwareUpdateRequestor", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRClusterOtaSoftwareUpdateRequestor : MTRClusterOTASoftwareUpdateRequestor
@end

API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterBridgedDeviceBasic : MTRClusterBridgedDeviceBasicInformation
@end

MTR_DEPRECATED("Please use MTRClusterWakeOnLAN", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRClusterWakeOnLan : MTRClusterWakeOnLAN
@end

MTR_DEPRECATED("Please use MTRClusterUnitTesting", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRClusterTestCluster : MTRClusterUnitTesting
@end

@interface MTRClusterIdentify (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)identifyWithParams:(MTRIdentifyClusterIdentifyParams *)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
         completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use identifyWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)triggerEffectWithParams:(MTRIdentifyClusterTriggerEffectParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use triggerEffectWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterGroups (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)addGroupWithParams:(MTRGroupsClusterAddGroupParams *)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
         completionHandler:
             (void (^)(MTRGroupsClusterAddGroupResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use addGroupWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)viewGroupWithParams:(MTRGroupsClusterViewGroupParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:
              (void (^)(MTRGroupsClusterViewGroupResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use viewGroupWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getGroupMembershipWithParams:(MTRGroupsClusterGetGroupMembershipParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTRGroupsClusterGetGroupMembershipResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use getGroupMembershipWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)removeGroupWithParams:(MTRGroupsClusterRemoveGroupParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:
                (void (^)(MTRGroupsClusterRemoveGroupResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use removeGroupWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)removeAllGroupsWithParams:(MTRGroupsClusterRemoveAllGroupsParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use removeAllGroupsWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)removeAllGroupsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use removeAllGroupsWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)addGroupIfIdentifyingWithParams:(MTRGroupsClusterAddGroupIfIdentifyingParams *)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use addGroupIfIdentifyingWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterScenes (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)addSceneWithParams:(MTRScenesClusterAddSceneParams *)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
         completionHandler:
             (void (^)(MTRScenesClusterAddSceneResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use addSceneWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)viewSceneWithParams:(MTRScenesClusterViewSceneParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:
              (void (^)(MTRScenesClusterViewSceneResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use viewSceneWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)removeSceneWithParams:(MTRScenesClusterRemoveSceneParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:
                (void (^)(MTRScenesClusterRemoveSceneResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use removeSceneWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)removeAllScenesWithParams:(MTRScenesClusterRemoveAllScenesParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(void (^)(MTRScenesClusterRemoveAllScenesResponseParams * _Nullable data,
                                      NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use removeAllScenesWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)storeSceneWithParams:(MTRScenesClusterStoreSceneParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:
               (void (^)(MTRScenesClusterStoreSceneResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use storeSceneWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)recallSceneWithParams:(MTRScenesClusterRecallSceneParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use recallSceneWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getSceneMembershipWithParams:(MTRScenesClusterGetSceneMembershipParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTRScenesClusterGetSceneMembershipResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use getSceneMembershipWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enhancedAddSceneWithParams:(MTRScenesClusterEnhancedAddSceneParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completionHandler:(void (^)(MTRScenesClusterEnhancedAddSceneResponseParams * _Nullable data,
                                       NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use enhancedAddSceneWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enhancedViewSceneWithParams:(MTRScenesClusterEnhancedViewSceneParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completionHandler:(void (^)(MTRScenesClusterEnhancedViewSceneResponseParams * _Nullable data,
                                        NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use enhancedViewSceneWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)copySceneWithParams:(MTRScenesClusterCopySceneParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:
              (void (^)(MTRScenesClusterCopySceneResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use copySceneWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterOnOff (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)offWithParams:(MTROnOffClusterOffParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use offWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)offWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use offWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)onWithParams:(MTROnOffClusterOnParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use onWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)onWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use onWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)toggleWithParams:(MTROnOffClusterToggleParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use toggleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)toggleWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use toggleWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)offWithEffectWithParams:(MTROnOffClusterOffWithEffectParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use offWithEffectWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)onWithRecallGlobalSceneWithParams:(MTROnOffClusterOnWithRecallGlobalSceneParams * _Nullable)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use onWithRecallGlobalSceneWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)onWithRecallGlobalSceneWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                            expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use onWithRecallGlobalSceneWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)onWithTimedOffWithParams:(MTROnOffClusterOnWithTimedOffParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use onWithTimedOffWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterOnOffSwitchConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterLevelControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)moveToLevelWithParams:(MTRLevelControlClusterMoveToLevelParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveToLevelWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveWithParams:(MTRLevelControlClusterMoveParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stepWithParams:(MTRLevelControlClusterStepParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use stepWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopWithParams:(MTRLevelControlClusterStopParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use stopWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveToLevelWithOnOffWithParams:(MTRLevelControlClusterMoveToLevelWithOnOffParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveToLevelWithOnOffWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveWithOnOffWithParams:(MTRLevelControlClusterMoveWithOnOffParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveWithOnOffWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stepWithOnOffWithParams:(MTRLevelControlClusterStepWithOnOffParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use stepWithOnOffWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopWithOnOffWithParams:(MTRLevelControlClusterStopWithOnOffParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use stopWithOnOffWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveToClosestFrequencyWithParams:(MTRLevelControlClusterMoveToClosestFrequencyParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveToClosestFrequencyWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterBinaryInputBasic (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterDescriptor (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (NSDictionary<NSString *, id> *)readAttributeDeviceListWithParams:(MTRReadParams * _Nullable)params
    MTR_DEPRECATED("Please use readAttributeDeviceTypeListWithParams on MTRClusterDescriptor", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterBinding (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterAccessControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (NSDictionary<NSString *, id> *)readAttributeAclWithParams:(MTRReadParams * _Nullable)params
    MTR_DEPRECATED("Please use readAttributeACLWithParams on MTRClusterAccessControl", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeAclWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    MTR_DEPRECATED("Please use writeAttributeACLWithValue on MTRClusterAccessControl", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeAclWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                            params:(MTRWriteParams * _Nullable)params
    MTR_DEPRECATED("Please use writeAttributeACLWithValue on MTRClusterAccessControl", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterActions (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)instantActionWithParams:(MTRActionsClusterInstantActionParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use instantActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)instantActionWithTransitionWithParams:(MTRActionsClusterInstantActionWithTransitionParams *)params
                               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use instantActionWithTransitionWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)startActionWithParams:(MTRActionsClusterStartActionParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use startActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)startActionWithDurationWithParams:(MTRActionsClusterStartActionWithDurationParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use startActionWithDurationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopActionWithParams:(MTRActionsClusterStopActionParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use stopActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)pauseActionWithParams:(MTRActionsClusterPauseActionParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use pauseActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)pauseActionWithDurationWithParams:(MTRActionsClusterPauseActionWithDurationParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use pauseActionWithDurationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)resumeActionWithParams:(MTRActionsClusterResumeActionParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use resumeActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enableActionWithParams:(MTRActionsClusterEnableActionParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use enableActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enableActionWithDurationWithParams:(MTRActionsClusterEnableActionWithDurationParams *)params
                            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use enableActionWithDurationWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)disableActionWithParams:(MTRActionsClusterDisableActionParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use disableActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)disableActionWithDurationWithParams:(MTRActionsClusterDisableActionWithDurationParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use disableActionWithDurationWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterBasic (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)mfgSpecificPingWithParams:(MTRBasicClusterMfgSpecificPingParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use mfgSpecificPingWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)mfgSpecificPingWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use mfgSpecificPingWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterOtaSoftwareUpdateProvider (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)queryImageWithParams:(MTROtaSoftwareUpdateProviderClusterQueryImageParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:(void (^)(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                                 NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use queryImageWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)applyUpdateRequestWithParams:(MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use applyUpdateRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)notifyUpdateAppliedWithParams:(MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use notifyUpdateAppliedWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterOtaSoftwareUpdateRequestor (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)announceOtaProviderWithParams:(MTROtaSoftwareUpdateRequestorClusterAnnounceOtaProviderParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use announceOTAProviderWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributeDefaultOtaProvidersWithParams:(MTRReadParams * _Nullable)params
    MTR_DEPRECATED("Please use readAttributeDefaultOTAProvidersWithParams on MTRClusterOTASoftwareUpdateRequestor", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeDefaultOtaProvidersWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    MTR_DEPRECATED("Please use writeAttributeDefaultOTAProvidersWithValue on MTRClusterOTASoftwareUpdateRequestor", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeDefaultOtaProvidersWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                            params:(MTRWriteParams * _Nullable)params
    MTR_DEPRECATED("Please use writeAttributeDefaultOTAProvidersWithValue on MTRClusterOTASoftwareUpdateRequestor", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterLocalizationConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterTimeFormatLocalization (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterUnitLocalization (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterPowerSourceConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterPowerSource (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterGeneralCommissioning (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)armFailSafeWithParams:(MTRGeneralCommissioningClusterArmFailSafeParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(void (^)(MTRGeneralCommissioningClusterArmFailSafeResponseParams * _Nullable data,
                                  NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use armFailSafeWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setRegulatoryConfigWithParams:(MTRGeneralCommissioningClusterSetRegulatoryConfigParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(void (^)(MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams * _Nullable data,
                                          NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use setRegulatoryConfigWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)commissioningCompleteWithParams:(MTRGeneralCommissioningClusterCommissioningCompleteParams * _Nullable)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completionHandler:
                          (void (^)(MTRGeneralCommissioningClusterCommissioningCompleteResponseParams * _Nullable data,
                              NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use commissioningCompleteWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)commissioningCompleteWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                              completionHandler:
                                  (void (^)(MTRGeneralCommissioningClusterCommissioningCompleteResponseParams * _Nullable data,
                                      NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use commissioningCompleteWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterNetworkCommissioning (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)scanNetworksWithParams:(MTRNetworkCommissioningClusterScanNetworksParams * _Nullable)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(void (^)(MTRNetworkCommissioningClusterScanNetworksResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use scanNetworksWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)addOrUpdateWiFiNetworkWithParams:(MTRNetworkCommissioningClusterAddOrUpdateWiFiNetworkParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completionHandler:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                             NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use addOrUpdateWiFiNetworkWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)addOrUpdateThreadNetworkWithParams:(MTRNetworkCommissioningClusterAddOrUpdateThreadNetworkParams *)params
                            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completionHandler:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                               NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use addOrUpdateThreadNetworkWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)removeNetworkWithParams:(MTRNetworkCommissioningClusterRemoveNetworkParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                    NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use removeNetworkWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)connectNetworkWithParams:(MTRNetworkCommissioningClusterConnectNetworkParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(void (^)(MTRNetworkCommissioningClusterConnectNetworkResponseParams * _Nullable data,
                                     NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use connectNetworkWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)reorderNetworkWithParams:(MTRNetworkCommissioningClusterReorderNetworkParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data,
                                     NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use reorderNetworkWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterDiagnosticLogs (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)retrieveLogsRequestWithParams:(MTRDiagnosticLogsClusterRetrieveLogsRequestParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(void (^)(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable data,
                                          NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use retrieveLogsRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterGeneralDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)testEventTriggerWithParams:(MTRGeneralDiagnosticsClusterTestEventTriggerParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use testEventTriggerWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributeBootReasonsWithParams:(MTRReadParams * _Nullable)params
    MTR_DEPRECATED("Please use readAttributeBootReasonWithParams on MTRClusterGeneralDiagnostics", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterSoftwareDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)resetWatermarksWithParams:(MTRSoftwareDiagnosticsClusterResetWatermarksParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use resetWatermarksWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)resetWatermarksWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use resetWatermarksWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterThreadNetworkDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)resetCountsWithParams:(MTRThreadNetworkDiagnosticsClusterResetCountsParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use resetCountsWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use resetCountsWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributeNeighborTableListWithParams:(MTRReadParams * _Nullable)params
    MTR_DEPRECATED("Please use readAttributeNeighborTableWithParams on MTRClusterThreadNetworkDiagnostics", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributeRouteTableListWithParams:(MTRReadParams * _Nullable)params
    MTR_DEPRECATED("Please use readAttributeRouteTableWithParams on MTRClusterThreadNetworkDiagnostics", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterWiFiNetworkDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)resetCountsWithParams:(MTRWiFiNetworkDiagnosticsClusterResetCountsParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use resetCountsWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use resetCountsWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributeBssidWithParams:(MTRReadParams * _Nullable)params
    MTR_DEPRECATED("Please use readAttributeBSSIDWithParams on MTRClusterWiFiNetworkDiagnostics", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributeRssiWithParams:(MTRReadParams * _Nullable)params
    MTR_DEPRECATED("Please use readAttributeRSSIWithParams on MTRClusterWiFiNetworkDiagnostics", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterEthernetNetworkDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)resetCountsWithParams:(MTREthernetNetworkDiagnosticsClusterResetCountsParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use resetCountsWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use resetCountsWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterBridgedDeviceBasic (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterSwitch (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterAdministratorCommissioning (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)openCommissioningWindowWithParams:(MTRAdministratorCommissioningClusterOpenCommissioningWindowParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use openCommissioningWindowWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)openBasicCommissioningWindowWithParams:(MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams *)params
                                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use openBasicCommissioningWindowWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)revokeCommissioningWithParams:(MTRAdministratorCommissioningClusterRevokeCommissioningParams * _Nullable)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use revokeCommissioningWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)revokeCommissioningWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use revokeCommissioningWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterOperationalCredentials (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)attestationRequestWithParams:(MTROperationalCredentialsClusterAttestationRequestParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTROperationalCredentialsClusterAttestationResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use attestationRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)certificateChainRequestWithParams:(MTROperationalCredentialsClusterCertificateChainRequestParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completionHandler:(void (^)(MTROperationalCredentialsClusterCertificateChainResponseParams * _Nullable data,
                                              NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use certificateChainRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)CSRRequestWithParams:(MTROperationalCredentialsClusterCSRRequestParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:(void (^)(MTROperationalCredentialsClusterCSRResponseParams * _Nullable data,
                                 NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use CSRRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)addNOCWithParams:(MTROperationalCredentialsClusterAddNOCParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                              NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use addNOCWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)updateNOCWithParams:(MTROperationalCredentialsClusterUpdateNOCParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use updateNOCWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)updateFabricLabelWithParams:(MTROperationalCredentialsClusterUpdateFabricLabelParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completionHandler:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                        NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use updateFabricLabelWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)removeFabricWithParams:(MTROperationalCredentialsClusterRemoveFabricParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use removeFabricWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)addTrustedRootCertificateWithParams:(MTROperationalCredentialsClusterAddTrustedRootCertificateParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use addTrustedRootCertificateWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterGroupKeyManagement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)keySetWriteWithParams:(MTRGroupKeyManagementClusterKeySetWriteParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use keySetWriteWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)keySetReadWithParams:(MTRGroupKeyManagementClusterKeySetReadParams *)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:(void (^)(MTRGroupKeyManagementClusterKeySetReadResponseParams * _Nullable data,
                                 NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use keySetReadWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)keySetRemoveWithParams:(MTRGroupKeyManagementClusterKeySetRemoveParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use keySetRemoveWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)keySetReadAllIndicesWithParams:(MTRGroupKeyManagementClusterKeySetReadAllIndicesParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(void (^)(MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams * _Nullable data,
                                           NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use keySetReadAllIndicesWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterFixedLabel (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterUserLabel (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterBooleanState (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterModeSelect (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)changeToModeWithParams:(MTRModeSelectClusterChangeToModeParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use changeToModeWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterDoorLock (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)lockDoorWithParams:(MTRDoorLockClusterLockDoorParams * _Nullable)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
         completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use lockDoorWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)unlockDoorWithParams:(MTRDoorLockClusterUnlockDoorParams * _Nullable)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use unlockDoorWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)unlockWithTimeoutWithParams:(MTRDoorLockClusterUnlockWithTimeoutParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use unlockWithTimeoutWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setWeekDayScheduleWithParams:(MTRDoorLockClusterSetWeekDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use setWeekDayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getWeekDayScheduleWithParams:(MTRDoorLockClusterGetWeekDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTRDoorLockClusterGetWeekDayScheduleResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use getWeekDayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearWeekDayScheduleWithParams:(MTRDoorLockClusterClearWeekDayScheduleParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use clearWeekDayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setYearDayScheduleWithParams:(MTRDoorLockClusterSetYearDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use setYearDayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getYearDayScheduleWithParams:(MTRDoorLockClusterGetYearDayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTRDoorLockClusterGetYearDayScheduleResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use getYearDayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearYearDayScheduleWithParams:(MTRDoorLockClusterClearYearDayScheduleParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use clearYearDayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setHolidayScheduleWithParams:(MTRDoorLockClusterSetHolidayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use setHolidayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getHolidayScheduleWithParams:(MTRDoorLockClusterGetHolidayScheduleParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(void (^)(MTRDoorLockClusterGetHolidayScheduleResponseParams * _Nullable data,
                                         NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use getHolidayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearHolidayScheduleWithParams:(MTRDoorLockClusterClearHolidayScheduleParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use clearHolidayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setUserWithParams:(MTRDoorLockClusterSetUserParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use setUserWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getUserWithParams:(MTRDoorLockClusterGetUserParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRDoorLockClusterGetUserResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use getUserWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearUserWithParams:(MTRDoorLockClusterClearUserParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use clearUserWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setCredentialWithParams:(MTRDoorLockClusterSetCredentialParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(void (^)(MTRDoorLockClusterSetCredentialResponseParams * _Nullable data,
                                    NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use setCredentialWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getCredentialStatusWithParams:(MTRDoorLockClusterGetCredentialStatusParams *)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(void (^)(MTRDoorLockClusterGetCredentialStatusResponseParams * _Nullable data,
                                          NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use getCredentialStatusWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearCredentialWithParams:(MTRDoorLockClusterClearCredentialParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use clearCredentialWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterWindowCovering (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)upOrOpenWithParams:(MTRWindowCoveringClusterUpOrOpenParams * _Nullable)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
         completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use upOrOpenWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)upOrOpenWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                 completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use upOrOpenWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)downOrCloseWithParams:(MTRWindowCoveringClusterDownOrCloseParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use downOrCloseWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)downOrCloseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use downOrCloseWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopMotionWithParams:(MTRWindowCoveringClusterStopMotionParams * _Nullable)params
              expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
       expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use stopMotionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopMotionWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
               expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use stopMotionWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)goToLiftValueWithParams:(MTRWindowCoveringClusterGoToLiftValueParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use goToLiftValueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)goToLiftPercentageWithParams:(MTRWindowCoveringClusterGoToLiftPercentageParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use goToLiftPercentageWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)goToTiltValueWithParams:(MTRWindowCoveringClusterGoToTiltValueParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use goToTiltValueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)goToTiltPercentageWithParams:(MTRWindowCoveringClusterGoToTiltPercentageParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use goToTiltPercentageWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterBarrierControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)barrierControlGoToPercentWithParams:(MTRBarrierControlClusterBarrierControlGoToPercentParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use barrierControlGoToPercentWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)barrierControlStopWithParams:(MTRBarrierControlClusterBarrierControlStopParams * _Nullable)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use barrierControlStopWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)barrierControlStopWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use barrierControlStopWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterPumpConfigurationAndControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterThermostat (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)setpointRaiseLowerWithParams:(MTRThermostatClusterSetpointRaiseLowerParams *)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use setpointRaiseLowerWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setWeeklyScheduleWithParams:(MTRThermostatClusterSetWeeklyScheduleParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use setWeeklyScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getWeeklyScheduleWithParams:(MTRThermostatClusterGetWeeklyScheduleParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completionHandler:(void (^)(MTRThermostatClusterGetWeeklyScheduleResponseParams * _Nullable data,
                                        NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use getWeeklyScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearWeeklyScheduleWithParams:(MTRThermostatClusterClearWeeklyScheduleParams * _Nullable)params
                       expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                    completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use clearWeeklyScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearWeeklyScheduleWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                        expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use clearWeeklyScheduleWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterFanControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterThermostatUserInterfaceConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterColorControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)moveToHueWithParams:(MTRColorControlClusterMoveToHueParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveToHueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveHueWithParams:(MTRColorControlClusterMoveHueParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveHueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stepHueWithParams:(MTRColorControlClusterStepHueParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use stepHueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveToSaturationWithParams:(MTRColorControlClusterMoveToSaturationParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveToSaturationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveSaturationWithParams:(MTRColorControlClusterMoveSaturationParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveSaturationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stepSaturationWithParams:(MTRColorControlClusterStepSaturationParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use stepSaturationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveToHueAndSaturationWithParams:(MTRColorControlClusterMoveToHueAndSaturationParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveToHueAndSaturationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveToColorWithParams:(MTRColorControlClusterMoveToColorParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveToColorWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveColorWithParams:(MTRColorControlClusterMoveColorParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveColorWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stepColorWithParams:(MTRColorControlClusterStepColorParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use stepColorWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveToColorTemperatureWithParams:(MTRColorControlClusterMoveToColorTemperatureParams *)params
                          expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                   expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                       completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveToColorTemperatureWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enhancedMoveToHueWithParams:(MTRColorControlClusterEnhancedMoveToHueParams *)params
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                  completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use enhancedMoveToHueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enhancedMoveHueWithParams:(MTRColorControlClusterEnhancedMoveHueParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use enhancedMoveHueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enhancedStepHueWithParams:(MTRColorControlClusterEnhancedStepHueParams *)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use enhancedStepHueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enhancedMoveToHueAndSaturationWithParams:(MTRColorControlClusterEnhancedMoveToHueAndSaturationParams *)params
                                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use enhancedMoveToHueAndSaturationWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)colorLoopSetWithParams:(MTRColorControlClusterColorLoopSetParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use colorLoopSetWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopMoveStepWithParams:(MTRColorControlClusterStopMoveStepParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use stopMoveStepWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveColorTemperatureWithParams:(MTRColorControlClusterMoveColorTemperatureParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use moveColorTemperatureWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stepColorTemperatureWithParams:(MTRColorControlClusterStepColorTemperatureParams *)params
                        expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                     completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use stepColorTemperatureWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterBallastConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (NSDictionary<NSString *, id> *)readAttributeIntrinsicBalanceFactorWithParams:(MTRReadParams * _Nullable)params
    MTR_DEPRECATED("Please use readAttributeIntrinsicBallastFactorWithParams on MTRClusterBallastConfiguration", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeIntrinsicBalanceFactorWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    MTR_DEPRECATED("Please use writeAttributeIntrinsicBallastFactorWithValue on MTRClusterBallastConfiguration", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeIntrinsicBalanceFactorWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                               params:(MTRWriteParams * _Nullable)params
    MTR_DEPRECATED("Please use writeAttributeIntrinsicBallastFactorWithValue on MTRClusterBallastConfiguration", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterIlluminanceMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterTemperatureMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterPressureMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterFlowMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterRelativeHumidityMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterOccupancySensing (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (NSDictionary<NSString *, id> *)readAttributePirOccupiedToUnoccupiedDelayWithParams:(MTRReadParams * _Nullable)params
    MTR_DEPRECATED("Please use readAttributePIROccupiedToUnoccupiedDelayWithParams on MTRClusterOccupancySensing", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePirOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    MTR_DEPRECATED("Please use writeAttributePIROccupiedToUnoccupiedDelayWithValue on MTRClusterOccupancySensing", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePirOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params
    MTR_DEPRECATED("Please use writeAttributePIROccupiedToUnoccupiedDelayWithValue on MTRClusterOccupancySensing", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributePirUnoccupiedToOccupiedDelayWithParams:(MTRReadParams * _Nullable)params
    MTR_DEPRECATED("Please use readAttributePIRUnoccupiedToOccupiedDelayWithParams on MTRClusterOccupancySensing", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePirUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    MTR_DEPRECATED("Please use writeAttributePIRUnoccupiedToOccupiedDelayWithValue on MTRClusterOccupancySensing", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePirUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                      expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                     params:(MTRWriteParams * _Nullable)params
    MTR_DEPRECATED("Please use writeAttributePIRUnoccupiedToOccupiedDelayWithValue on MTRClusterOccupancySensing", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributePirUnoccupiedToOccupiedThresholdWithParams:(MTRReadParams * _Nullable)params
    MTR_DEPRECATED("Please use readAttributePIRUnoccupiedToOccupiedThresholdWithParams on MTRClusterOccupancySensing",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePirUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
    MTR_DEPRECATED("Please use writeAttributePIRUnoccupiedToOccupiedThresholdWithValue on MTRClusterOccupancySensing",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePirUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary
                                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                                                         params:(MTRWriteParams * _Nullable)params
    MTR_DEPRECATED("Please use writeAttributePIRUnoccupiedToOccupiedThresholdWithValue on MTRClusterOccupancySensing",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterWakeOnLan (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterChannel (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)changeChannelWithParams:(MTRChannelClusterChangeChannelParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(void (^)(MTRChannelClusterChangeChannelResponseParams * _Nullable data,
                                    NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use changeChannelWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)changeChannelByNumberWithParams:(MTRChannelClusterChangeChannelByNumberParams *)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use changeChannelByNumberWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)skipChannelWithParams:(MTRChannelClusterSkipChannelParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use skipChannelWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterTargetNavigator (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)navigateTargetWithParams:(MTRTargetNavigatorClusterNavigateTargetParams *)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(void (^)(MTRTargetNavigatorClusterNavigateTargetResponseParams * _Nullable data,
                                     NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use navigateTargetWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterMediaPlayback (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)playWithParams:(MTRMediaPlaybackClusterPlayParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use playWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)playWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
             completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use playWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)pauseWithParams:(MTRMediaPlaybackClusterPauseParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use pauseWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)pauseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
              completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                    NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use pauseWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopPlaybackWithParams:(MTRMediaPlaybackClusterStopPlaybackParams * _Nullable)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use stopWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopPlaybackWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                     completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                           NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use stopWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)startOverWithParams:(MTRMediaPlaybackClusterStartOverParams * _Nullable)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:
              (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use startOverWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)startOverWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
              expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                  completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                        NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use startOverWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)previousWithParams:(MTRMediaPlaybackClusterPreviousParams * _Nullable)params
            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
         completionHandler:
             (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use previousWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)previousWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
             expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                 completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                       NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use previousWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)nextWithParams:(MTRMediaPlaybackClusterNextParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use nextWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)nextWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
             completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use nextWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)rewindWithParams:(MTRMediaPlaybackClusterRewindParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use rewindWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)rewindWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
               completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                     NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use rewindWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)fastForwardWithParams:(MTRMediaPlaybackClusterFastForwardParams * _Nullable)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                  NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use fastForwardWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)fastForwardWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                    completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                          NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use fastForwardWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)skipForwardWithParams:(MTRMediaPlaybackClusterSkipForwardParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                  NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use skipForwardWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)skipBackwardWithParams:(MTRMediaPlaybackClusterSkipBackwardParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use skipBackwardWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)seekWithParams:(MTRMediaPlaybackClusterSeekParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use seekWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterMediaInput (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)selectInputWithParams:(MTRMediaInputClusterSelectInputParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use selectInputWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)showInputStatusWithParams:(MTRMediaInputClusterShowInputStatusParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use showInputStatusWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)showInputStatusWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use showInputStatusWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)hideInputStatusWithParams:(MTRMediaInputClusterHideInputStatusParams * _Nullable)params
                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use hideInputStatusWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)hideInputStatusWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                    expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use hideInputStatusWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)renameInputWithParams:(MTRMediaInputClusterRenameInputParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use renameInputWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterLowPower (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)sleepWithParams:(MTRLowPowerClusterSleepParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use sleepWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)sleepWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use sleepWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterKeypadInput (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)sendKeyWithParams:(MTRKeypadInputClusterSendKeyParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:
            (void (^)(MTRKeypadInputClusterSendKeyResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use sendKeyWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterContentLauncher (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)launchContentWithParams:(MTRContentLauncherClusterLaunchContentParams *)params
                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
              completionHandler:(void (^)(MTRContentLauncherClusterLaunchResponseParams * _Nullable data,
                                    NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use launchContentWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)launchURLWithParams:(MTRContentLauncherClusterLaunchURLParams *)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:
              (void (^)(MTRContentLauncherClusterLaunchResponseParams * _Nullable data, NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use launchURLWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterAudioOutput (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)selectOutputWithParams:(MTRAudioOutputClusterSelectOutputParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use selectOutputWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)renameOutputWithParams:(MTRAudioOutputClusterRenameOutputParams *)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use renameOutputWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterApplicationLauncher (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)launchAppWithParams:(MTRApplicationLauncherClusterLaunchAppParams * _Nullable)params
             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
          completionHandler:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data,
                                NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use launchAppWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopAppWithParams:(MTRApplicationLauncherClusterStopAppParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data,
                              NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use stopAppWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)hideAppWithParams:(MTRApplicationLauncherClusterHideAppParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data,
                              NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use hideAppWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterApplicationBasic (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterAccountLogin (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)getSetupPINWithParams:(MTRAccountLoginClusterGetSetupPINParams *)params
               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
            completionHandler:(void (^)(MTRAccountLoginClusterGetSetupPINResponseParams * _Nullable data,
                                  NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use getSetupPINWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)loginWithParams:(MTRAccountLoginClusterLoginParams *)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use loginWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)logoutWithParams:(MTRAccountLoginClusterLogoutParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use logoutWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)logoutWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
           expectedValueInterval:(NSNumber *)expectedValueIntervalMs
               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use logoutWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterElectricalMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)getProfileInfoCommandWithParams:(MTRElectricalMeasurementClusterGetProfileInfoCommandParams * _Nullable)params
                         expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                  expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                      completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use getProfileInfoCommandWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getProfileInfoCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                          expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                              completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use getProfileInfoCommandWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getMeasurementProfileCommandWithParams:(MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams *)params
                                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use getMeasurementProfileCommandWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterTestCluster (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
    MTR_DEPRECATED(
        "Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)testWithParams:(MTRTestClusterClusterTestParams * _Nullable)params
           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
        completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use testWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
         expectedValueInterval:(NSNumber *)expectedValueIntervalMs
             completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use testWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testNotHandledWithParams:(MTRTestClusterClusterTestNotHandledParams * _Nullable)params
                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
               completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use testNotHandledWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testNotHandledWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                   expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                       completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use testNotHandledWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testSpecificWithParams:(MTRTestClusterClusterTestSpecificParams * _Nullable)params
                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
             completionHandler:(void (^)(MTRTestClusterClusterTestSpecificResponseParams * _Nullable data,
                                   NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testSpecificWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testSpecificWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                 expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                     completionHandler:(void (^)(MTRTestClusterClusterTestSpecificResponseParams * _Nullable data,
                                           NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testSpecificWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testUnknownCommandWithParams:(MTRTestClusterClusterTestUnknownCommandParams * _Nullable)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use testUnknownCommandWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testUnknownCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use testUnknownCommandWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testAddArgumentsWithParams:(MTRTestClusterClusterTestAddArgumentsParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completionHandler:(void (^)(MTRTestClusterClusterTestAddArgumentsResponseParams * _Nullable data,
                                       NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testAddArgumentsWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testSimpleArgumentRequestWithParams:(MTRTestClusterClusterTestSimpleArgumentRequestParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completionHandler:(void (^)(MTRTestClusterClusterTestSimpleArgumentResponseParams * _Nullable data,
                                                NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testSimpleArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testStructArrayArgumentRequestWithParams:(MTRTestClusterClusterTestStructArrayArgumentRequestParams *)params
                                  expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                           expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                               completionHandler:
                                   (void (^)(MTRTestClusterClusterTestStructArrayArgumentResponseParams * _Nullable data,
                                       NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testStructArrayArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testStructArgumentRequestWithParams:(MTRTestClusterClusterTestStructArgumentRequestParams *)params
                             expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                      expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                          completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testStructArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testNestedStructArgumentRequestWithParams:(MTRTestClusterClusterTestNestedStructArgumentRequestParams *)params
                                   expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                            expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                      NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testNestedStructArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testListStructArgumentRequestWithParams:(MTRTestClusterClusterTestListStructArgumentRequestParams *)params
                                 expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                          expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                              completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                    NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testListStructArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testListInt8UArgumentRequestWithParams:(MTRTestClusterClusterTestListInt8UArgumentRequestParams *)params
                                expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                         expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                             completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                   NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testListInt8UArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testNestedStructListArgumentRequestWithParams:(MTRTestClusterClusterTestNestedStructListArgumentRequestParams *)params
                                       expectedValues:
                                           (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                                expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                    completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                          NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testNestedStructListArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testListNestedStructListArgumentRequestWithParams:
            (MTRTestClusterClusterTestListNestedStructListArgumentRequestParams *)params
                                           expectedValues:
                                               (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                        completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data,
                                                              NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testListNestedStructListArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testListInt8UReverseRequestWithParams:(MTRTestClusterClusterTestListInt8UReverseRequestParams *)params
                               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completionHandler:(void (^)(MTRTestClusterClusterTestListInt8UReverseResponseParams * _Nullable data,
                                                  NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testListInt8UReverseRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testEnumsRequestWithParams:(MTRTestClusterClusterTestEnumsRequestParams *)params
                    expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
             expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                 completionHandler:(void (^)(MTRTestClusterClusterTestEnumsResponseParams * _Nullable data,
                                       NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testEnumsRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testNullableOptionalRequestWithParams:(MTRTestClusterClusterTestNullableOptionalRequestParams * _Nullable)params
                               expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                        expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                            completionHandler:(void (^)(MTRTestClusterClusterTestNullableOptionalResponseParams * _Nullable data,
                                                  NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testNullableOptionalRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testComplexNullableOptionalRequestWithParams:(MTRTestClusterClusterTestComplexNullableOptionalRequestParams *)params
                                      expectedValues:
                                          (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                   completionHandler:
                                       (void (^)(MTRTestClusterClusterTestComplexNullableOptionalResponseParams * _Nullable data,
                                           NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testComplexNullableOptionalRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)simpleStructEchoRequestWithParams:(MTRTestClusterClusterSimpleStructEchoRequestParams *)params
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                        completionHandler:(void (^)(MTRTestClusterClusterSimpleStructResponseParams * _Nullable data,
                                              NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use simpleStructEchoRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)timedInvokeRequestWithParams:(MTRTestClusterClusterTimedInvokeRequestParams * _Nullable)params
                      expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
               expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                   completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use timedInvokeRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)timedInvokeRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues
                       expectedValueInterval:(NSNumber *)expectedValueIntervalMs
                           completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use timedInvokeRequestWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testSimpleOptionalArgumentRequestWithParams:(MTRTestClusterClusterTestSimpleOptionalArgumentRequestParams * _Nullable)params
                                     expectedValues:
                                         (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                              expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                  completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use testSimpleOptionalArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testEmitTestEventRequestWithParams:(MTRTestClusterClusterTestEmitTestEventRequestParams *)params
                            expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                     expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                         completionHandler:(void (^)(MTRTestClusterClusterTestEmitTestEventResponseParams * _Nullable data,
                                               NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testEmitTestEventRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testEmitTestFabricScopedEventRequestWithParams:(MTRTestClusterClusterTestEmitTestFabricScopedEventRequestParams *)params
                                        expectedValues:
                                            (NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries
                                 expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs
                                     completionHandler:
                                         (void (^)(
                                             MTRTestClusterClusterTestEmitTestFabricScopedEventResponseParams * _Nullable data,
                                             NSError * _Nullable error))completionHandler
    MTR_DEPRECATED("Please use testEmitTestFabricScopedEventRequestWithParams:expectedValues:expectedValueInterval:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

NS_ASSUME_NONNULL_END
