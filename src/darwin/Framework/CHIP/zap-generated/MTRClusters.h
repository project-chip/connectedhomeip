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
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterIdentify : MTRGenericCluster

- (void)identifyWithParams:(MTRIdentifyClusterIdentifyParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)triggerEffectWithParams:(MTRIdentifyClusterTriggerEffectParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeIdentifyTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeIdentifyTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeIdentifyTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeIdentifyTypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterIdentify (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Groups
 *    Attributes and commands for group configuration and manipulation.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterGroups : MTRGenericCluster

- (void)addGroupWithParams:(MTRGroupsClusterAddGroupParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGroupsClusterAddGroupResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)viewGroupWithParams:(MTRGroupsClusterViewGroupParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGroupsClusterViewGroupResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getGroupMembershipWithParams:(MTRGroupsClusterGetGroupMembershipParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGroupsClusterGetGroupMembershipResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)removeGroupWithParams:(MTRGroupsClusterRemoveGroupParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGroupsClusterRemoveGroupResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)removeAllGroupsWithParams:(MTRGroupsClusterRemoveAllGroupsParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)removeAllGroupsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)addGroupIfIdentifyingWithParams:(MTRGroupsClusterAddGroupIfIdentifyingParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNameSupportWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterGroups (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster On/Off
 *    Attributes and commands for switching devices between 'On' and 'Off' states.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterOnOff : MTRGenericCluster

- (void)offWithParams:(MTROnOffClusterOffParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)offWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)onWithParams:(MTROnOffClusterOnParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)onWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)toggleWithParams:(MTROnOffClusterToggleParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)toggleWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)offWithEffectWithParams:(MTROnOffClusterOffWithEffectParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)onWithRecallGlobalSceneWithParams:(MTROnOffClusterOnWithRecallGlobalSceneParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)onWithRecallGlobalSceneWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)onWithTimedOffWithParams:(MTROnOffClusterOnWithTimedOffParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOnOffWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGlobalSceneControlWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOnTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOffWaitTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOffWaitTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOffWaitTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStartUpOnOffWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpOnOffWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpOnOffWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterOnOff (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Level Control
 *    Attributes and commands for controlling devices that can be set to a level between fully 'On' and fully 'Off.'
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterLevelControl : MTRGenericCluster

- (void)moveToLevelWithParams:(MTRLevelControlClusterMoveToLevelParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveWithParams:(MTRLevelControlClusterMoveParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stepWithParams:(MTRLevelControlClusterStepParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopWithParams:(MTRLevelControlClusterStopParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveToLevelWithOnOffWithParams:(MTRLevelControlClusterMoveToLevelWithOnOffParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveWithOnOffWithParams:(MTRLevelControlClusterMoveWithOnOffParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stepWithOnOffWithParams:(MTRLevelControlClusterStepWithOnOffParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopWithOnOffWithParams:(MTRLevelControlClusterStopWithOnOffParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveToClosestFrequencyWithParams:(MTRLevelControlClusterMoveToClosestFrequencyParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRemainingTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentFrequencyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinFrequencyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxFrequencyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOptionsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOptionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOptionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOnOffTransitionTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnOffTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnOffTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOnLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOnTransitionTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOffTransitionTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOffTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOffTransitionTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDefaultMoveRateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDefaultMoveRateWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDefaultMoveRateWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStartUpCurrentLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpCurrentLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpCurrentLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterLevelControl (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Pulse Width Modulation
 *    Cluster to control pulse width modulation
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterPulseWidthModulation : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterPulseWidthModulation (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster Descriptor
 *    The Descriptor Cluster is meant to replace the support from the Zigbee Device Object (ZDO) for describing a node, its endpoints and clusters.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterDescriptor : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDeviceTypeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeServerListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClientListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePartsListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTagListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterDescriptor (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Binding
 *    The Binding Cluster is meant to replace the support from the Zigbee Device Object (ZDO) for supporting the binding table.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterBinding : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBindingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBindingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBindingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterBinding (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Access Control
 *    The Access Control Cluster exposes a data model view of a
      Node's Access Control List (ACL), which codifies the rules used to manage
      and enforce Access Control for the Node's endpoints and their associated
      cluster instances.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterAccessControl : MTRGenericCluster

- (void)reviewFabricRestrictionsWithParams:(MTRAccessControlClusterReviewFabricRestrictionsParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRAccessControlClusterReviewFabricRestrictionsResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeACLWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeACLWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeACLWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeExtensionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeExtensionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeExtensionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSubjectsPerAccessControlEntryWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTargetsPerAccessControlEntryWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAccessControlEntriesPerFabricWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCommissioningARLWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeARLWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterAccessControl (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Actions
 *    This cluster provides a standardized way for a Node (typically a Bridge, but could be any Node) to expose action information.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterActions : MTRGenericCluster

- (void)instantActionWithParams:(MTRActionsClusterInstantActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)instantActionWithTransitionWithParams:(MTRActionsClusterInstantActionWithTransitionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)startActionWithParams:(MTRActionsClusterStartActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)startActionWithDurationWithParams:(MTRActionsClusterStartActionWithDurationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopActionWithParams:(MTRActionsClusterStopActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)pauseActionWithParams:(MTRActionsClusterPauseActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)pauseActionWithDurationWithParams:(MTRActionsClusterPauseActionWithDurationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)resumeActionWithParams:(MTRActionsClusterResumeActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enableActionWithParams:(MTRActionsClusterEnableActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enableActionWithDurationWithParams:(MTRActionsClusterEnableActionWithDurationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)disableActionWithParams:(MTRActionsClusterDisableActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)disableActionWithDurationWithParams:(MTRActionsClusterDisableActionWithDurationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActionListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEndpointListsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSetupURLWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterActions (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Basic Information
 *    This cluster provides attributes and events for determining basic information about Nodes, which supports both
      Commissioning and operational determination of Node characteristics, such as Vendor ID, Product ID and serial number,
      which apply to the whole Node. Also allows setting user device information such as location.
 */
MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRClusterBasicInformation : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDataModelRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeVendorNameWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeVendorIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProductNameWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProductIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNodeLabelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNodeLabelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNodeLabelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLocationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLocationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLocationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHardwareVersionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHardwareVersionStringWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSoftwareVersionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSoftwareVersionStringWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeManufacturingDateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePartNumberWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProductURLWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProductLabelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSerialNumberWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLocalConfigDisabledWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLocalConfigDisabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLocalConfigDisabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReachableWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUniqueIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCapabilityMinimaWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProductAppearanceWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSpecificationVersionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxPathsPerInvokeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterBasicInformation (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster OTA Software Update Provider
 *    Provides an interface for providing OTA software updates
 */
MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRClusterOTASoftwareUpdateProvider : MTRGenericCluster

- (void)queryImageWithParams:(MTROTASoftwareUpdateProviderClusterQueryImageParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROTASoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)applyUpdateRequestWithParams:(MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)notifyUpdateAppliedWithParams:(MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterOTASoftwareUpdateProvider (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster OTA Software Update Requestor
 *    Provides an interface for downloading and applying OTA software updates
 */
MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRClusterOTASoftwareUpdateRequestor : MTRGenericCluster

- (void)announceOTAProviderWithParams:(MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDefaultOTAProvidersWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeDefaultOTAProvidersWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeDefaultOTAProvidersWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUpdatePossibleWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUpdateStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUpdateStateProgressWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterOTASoftwareUpdateRequestor (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Localization Configuration
 *    Nodes should be expected to be deployed to any and all regions of the world. These global regions
      may have differing common languages, units of measurements, and numerical formatting
      standards. As such, Nodes that visually or audibly convey information need a mechanism by which
      they can be configured to use a user’s preferred language, units, etc
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterLocalizationConfiguration : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveLocaleWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeActiveLocaleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeActiveLocaleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedLocalesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterLocalizationConfiguration (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Time Format Localization
 *    Nodes should be expected to be deployed to any and all regions of the world. These global regions
      may have differing preferences for how dates and times are conveyed. As such, Nodes that visually
      or audibly convey time information need a mechanism by which they can be configured to use a
      user’s preferred format.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterTimeFormatLocalization : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHourFormatWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeHourFormatWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeHourFormatWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveCalendarTypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeActiveCalendarTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeActiveCalendarTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedCalendarTypesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterTimeFormatLocalization (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Unit Localization
 *    Nodes should be expected to be deployed to any and all regions of the world. These global regions
      may have differing preferences for the units in which values are conveyed in communication to a
      user. As such, Nodes that visually or audibly convey measurable values to the user need a
      mechanism by which they can be configured to use a user’s preferred unit.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterUnitLocalization : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTemperatureUnitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureUnitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureUnitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterUnitLocalization (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Power Source Configuration
 *    This cluster is used to describe the configuration and capabilities of a Device's power system.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterPowerSourceConfiguration : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSourcesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterPowerSourceConfiguration (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Power Source
 *    This cluster is used to describe the configuration and capabilities of a physical power source that provides power to the Node.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterPowerSource : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStatusWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOrderWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDescriptionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWiredAssessedInputVoltageWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWiredAssessedInputFrequencyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWiredCurrentTypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWiredAssessedCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWiredNominalVoltageWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWiredMaximumCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWiredPresentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveWiredFaultsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatVoltageWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatPercentRemainingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatTimeRemainingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatChargeLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatReplacementNeededWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatReplaceabilityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatPresentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveBatFaultsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatReplacementDescriptionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatCommonDesignationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatANSIDesignationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatIECDesignationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatApprovedChemistryWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatCapacityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatQuantityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatChargeStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatTimeToFullChargeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatFunctionalWhileChargingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatChargingCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveBatChargeFaultsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEndpointListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterPowerSource (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster General Commissioning
 *    This cluster is used to manage global aspects of the Commissioning flow.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterGeneralCommissioning : MTRGenericCluster

- (void)armFailSafeWithParams:(MTRGeneralCommissioningClusterArmFailSafeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGeneralCommissioningClusterArmFailSafeResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setRegulatoryConfigWithParams:(MTRGeneralCommissioningClusterSetRegulatoryConfigParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)commissioningCompleteWithParams:(MTRGeneralCommissioningClusterCommissioningCompleteParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGeneralCommissioningClusterCommissioningCompleteResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)commissioningCompleteWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGeneralCommissioningClusterCommissioningCompleteResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setTCAcknowledgementsWithParams:(MTRGeneralCommissioningClusterSetTCAcknowledgementsParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGeneralCommissioningClusterSetTCAcknowledgementsResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBreadcrumbWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBreadcrumbWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBreadcrumbWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBasicCommissioningInfoWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRegulatoryConfigWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLocationCapabilityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportsConcurrentConnectionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTCAcceptedVersionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTCMinRequiredVersionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTCAcknowledgementsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTCAcknowledgementsRequiredWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTCUpdateDeadlineWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterGeneralCommissioning (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Network Commissioning
 *    Functionality to configure, enable, disable network credentials and access on a Matter device.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterNetworkCommissioning : MTRGenericCluster

- (void)scanNetworksWithParams:(MTRNetworkCommissioningClusterScanNetworksParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRNetworkCommissioningClusterScanNetworksResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)scanNetworksWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRNetworkCommissioningClusterScanNetworksResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)addOrUpdateWiFiNetworkWithParams:(MTRNetworkCommissioningClusterAddOrUpdateWiFiNetworkParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)addOrUpdateThreadNetworkWithParams:(MTRNetworkCommissioningClusterAddOrUpdateThreadNetworkParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)removeNetworkWithParams:(MTRNetworkCommissioningClusterRemoveNetworkParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)connectNetworkWithParams:(MTRNetworkCommissioningClusterConnectNetworkParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRNetworkCommissioningClusterConnectNetworkResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)reorderNetworkWithParams:(MTRNetworkCommissioningClusterReorderNetworkParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)queryIdentityWithParams:(MTRNetworkCommissioningClusterQueryIdentityParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRNetworkCommissioningClusterQueryIdentityResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxNetworksWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNetworksWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeScanMaxTimeSecondsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeConnectMaxTimeSecondsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInterfaceEnabledWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeInterfaceEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeInterfaceEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLastNetworkingStatusWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLastNetworkIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLastConnectErrorValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedWiFiBandsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedThreadFeaturesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeThreadVersionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterNetworkCommissioning (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Diagnostic Logs
 *    The cluster provides commands for retrieving unstructured diagnostic logs from a Node that may be used to aid in diagnostics.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterDiagnosticLogs : MTRGenericCluster

- (void)retrieveLogsRequestWithParams:(MTRDiagnosticLogsClusterRetrieveLogsRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterDiagnosticLogs (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster General Diagnostics
 *    The General Diagnostics Cluster, along with other diagnostics clusters, provide a means to acquire standardized diagnostics metrics that MAY be used by a Node to assist a user or Administrative Node in diagnosing potential problems.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterGeneralDiagnostics : MTRGenericCluster

- (void)testEventTriggerWithParams:(MTRGeneralDiagnosticsClusterTestEventTriggerParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)timeSnapshotWithParams:(MTRGeneralDiagnosticsClusterTimeSnapshotParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGeneralDiagnosticsClusterTimeSnapshotResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)timeSnapshotWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGeneralDiagnosticsClusterTimeSnapshotResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)payloadTestRequestWithParams:(MTRGeneralDiagnosticsClusterPayloadTestRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGeneralDiagnosticsClusterPayloadTestResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNetworkInterfacesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRebootCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUpTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTotalOperationalHoursWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBootReasonWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveHardwareFaultsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveRadioFaultsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveNetworkFaultsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTestEventTriggersEnabledWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterGeneralDiagnostics (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Software Diagnostics
 *    The Software Diagnostics Cluster provides a means to acquire standardized diagnostics metrics that MAY be used by a Node to assist a user or Administrative Node in diagnosing potential problems.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterSoftwareDiagnostics : MTRGenericCluster

- (void)resetWatermarksWithParams:(MTRSoftwareDiagnosticsClusterResetWatermarksParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)resetWatermarksWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeThreadMetricsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentHeapFreeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentHeapUsedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentHeapHighWatermarkWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterSoftwareDiagnostics (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Thread Network Diagnostics
 *    The Thread Network Diagnostics Cluster provides a means to acquire standardized diagnostics metrics that MAY be used by a Node to assist a user or Administrative Node in diagnosing potential problems
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterThreadNetworkDiagnostics : MTRGenericCluster

- (void)resetCountsWithParams:(MTRThreadNetworkDiagnosticsClusterResetCountsParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeChannelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRoutingRoleWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNetworkNameWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePanIdWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeExtendedPanIdWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeshLocalPrefixWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOverrunCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNeighborTableWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRouteTableWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePartitionIdWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWeightingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDataVersionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStableDataVersionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLeaderRouterIdWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDetachedRoleCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeChildRoleCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRouterRoleCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLeaderRoleCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttachAttemptCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePartitionIdChangeCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBetterPartitionAttachAttemptCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeParentChangeCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxTotalCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxUnicastCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxBroadcastCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxAckRequestedCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxAckedCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxNoAckRequestedCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxDataCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxDataPollCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxBeaconCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxBeaconRequestCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxOtherCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxRetryCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxDirectMaxRetryExpiryCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxIndirectMaxRetryExpiryCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxErrCcaCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxErrAbortCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxErrBusyChannelCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxTotalCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxUnicastCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxBroadcastCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxDataCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxDataPollCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxBeaconCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxBeaconRequestCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxOtherCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxAddressFilteredCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxDestAddrFilteredCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxDuplicatedCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxErrNoFrameCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxErrUnknownNeighborCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxErrInvalidSrcAddrCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxErrSecCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxErrFcsCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRxErrOtherCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveTimestampWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePendingTimestampWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDelayWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSecurityPolicyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeChannelPage0MaskWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperationalDatasetComponentsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveNetworkFaultsListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterThreadNetworkDiagnostics (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Wi-Fi Network Diagnostics
 *    The Wi-Fi Network Diagnostics Cluster provides a means to acquire standardized diagnostics metrics that MAY be used by a Node to assist a user or Administrative Node in diagnosing potential problems.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterWiFiNetworkDiagnostics : MTRGenericCluster

- (void)resetCountsWithParams:(MTRWiFiNetworkDiagnosticsClusterResetCountsParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBSSIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSecurityTypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWiFiVersionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeChannelNumberWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRSSIWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBeaconLostCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBeaconRxCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePacketMulticastRxCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePacketMulticastTxCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePacketUnicastRxCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePacketUnicastTxCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentMaxRateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOverrunCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterWiFiNetworkDiagnostics (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Ethernet Network Diagnostics
 *    The Ethernet Network Diagnostics Cluster provides a means to acquire standardized diagnostics metrics that MAY be used by a Node to assist a user or Administrative Node in diagnosing potential problems.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterEthernetNetworkDiagnostics : MTRGenericCluster

- (void)resetCountsWithParams:(MTREthernetNetworkDiagnosticsClusterResetCountsParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePHYRateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFullDuplexWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePacketRxCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePacketTxCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTxErrCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCollisionCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOverrunCountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCarrierDetectWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTimeSinceResetWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterEthernetNetworkDiagnostics (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Time Synchronization
 *    Accurate time is required for a number of reasons, including scheduling, display and validating security materials.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterTimeSynchronization : MTRGenericCluster

- (void)setUTCTimeWithParams:(MTRTimeSynchronizationClusterSetUTCTimeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)setTrustedTimeSourceWithParams:(MTRTimeSynchronizationClusterSetTrustedTimeSourceParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)setTimeZoneWithParams:(MTRTimeSynchronizationClusterSetTimeZoneParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRTimeSynchronizationClusterSetTimeZoneResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)setDSTOffsetWithParams:(MTRTimeSynchronizationClusterSetDSTOffsetParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)setDefaultNTPWithParams:(MTRTimeSynchronizationClusterSetDefaultNTPParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUTCTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGranularityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTimeSourceWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTrustedTimeSourceWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDefaultNTPWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTimeZoneWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDSTOffsetWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLocalTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTimeZoneDatabaseWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNTPServerAvailableWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTimeZoneListMaxSizeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDSTOffsetListMaxSizeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportsDNSResolveWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterTimeSynchronization (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Bridged Device Basic Information
 *    This Cluster serves two purposes towards a Node communicating with a Bridge: indicate that the functionality on
          the Endpoint where it is placed (and its Parts) is bridged from a non-CHIP technology; and provide a centralized
          collection of attributes that the Node MAY collect to aid in conveying information regarding the Bridged Device to a user,
          such as the vendor name, the model name, or user-assigned name.
 */
MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRClusterBridgedDeviceBasicInformation : MTRGenericCluster

- (void)keepActiveWithParams:(MTRBridgedDeviceBasicInformationClusterKeepActiveParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeVendorNameWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeVendorIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProductNameWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProductIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNodeLabelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNodeLabelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNodeLabelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHardwareVersionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHardwareVersionStringWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSoftwareVersionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSoftwareVersionStringWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeManufacturingDateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePartNumberWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProductURLWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProductLabelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSerialNumberWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReachableWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUniqueIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProductAppearanceWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterBridgedDeviceBasicInformation (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Switch
 *    This cluster exposes interactions with a switch device, for the purpose of using those interactions by other devices.
Two types of switch devices are supported: latching switch (e.g. rocker switch) and momentary switch (e.g. push button), distinguished with their feature flags.
Interactions with the switch device are exposed as attributes (for the latching switch) and as events (for both types of switches). An interested party MAY subscribe to these attributes/events and thus be informed of the interactions, and can perform actions based on this, for example by sending commands to perform an action such as controlling a light or a window shade.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterSwitch : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfPositionsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentPositionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMultiPressMaxWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterSwitch (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Administrator Commissioning
 *    Commands to trigger a Node to allow a new Administrator to commission it.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterAdministratorCommissioning : MTRGenericCluster

- (void)openCommissioningWindowWithParams:(MTRAdministratorCommissioningClusterOpenCommissioningWindowParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)openBasicCommissioningWindowWithParams:(MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)revokeCommissioningWithParams:(MTRAdministratorCommissioningClusterRevokeCommissioningParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)revokeCommissioningWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWindowStatusWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAdminFabricIndexWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAdminVendorIdWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterAdministratorCommissioning (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Operational Credentials
 *    This cluster is used to add or remove Operational Credentials on a Commissionee or Node, as well as manage the associated Fabrics.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterOperationalCredentials : MTRGenericCluster

- (void)attestationRequestWithParams:(MTROperationalCredentialsClusterAttestationRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalCredentialsClusterAttestationResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)certificateChainRequestWithParams:(MTROperationalCredentialsClusterCertificateChainRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalCredentialsClusterCertificateChainResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)CSRRequestWithParams:(MTROperationalCredentialsClusterCSRRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalCredentialsClusterCSRResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)addNOCWithParams:(MTROperationalCredentialsClusterAddNOCParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)updateNOCWithParams:(MTROperationalCredentialsClusterUpdateNOCParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)updateFabricLabelWithParams:(MTROperationalCredentialsClusterUpdateFabricLabelParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)removeFabricWithParams:(MTROperationalCredentialsClusterRemoveFabricParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)addTrustedRootCertificateWithParams:(MTROperationalCredentialsClusterAddTrustedRootCertificateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setVidVerificationStatementWithParams:(MTROperationalCredentialsClusterSetVidVerificationStatementParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)setVidVerificationStatementWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)signVidVerificationRequestWithParams:(MTROperationalCredentialsClusterSignVidVerificationRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalCredentialsClusterSignVidVerificationResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNOCsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFabricsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedFabricsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCommissionedFabricsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTrustedRootCertificatesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentFabricIndexWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterOperationalCredentials (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Group Key Management
 *    The Group Key Management Cluster is the mechanism by which group keys are managed.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterGroupKeyManagement : MTRGenericCluster

- (void)keySetWriteWithParams:(MTRGroupKeyManagementClusterKeySetWriteParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)keySetReadWithParams:(MTRGroupKeyManagementClusterKeySetReadParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGroupKeyManagementClusterKeySetReadResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)keySetRemoveWithParams:(MTRGroupKeyManagementClusterKeySetRemoveParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)keySetReadAllIndicesWithParams:(MTRGroupKeyManagementClusterKeySetReadAllIndicesParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)keySetReadAllIndicesWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGroupKeyMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeGroupKeyMapWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeGroupKeyMapWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGroupTableWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxGroupsPerFabricWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxGroupKeysPerFabricWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterGroupKeyManagement (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Fixed Label
 *    The Fixed Label Cluster provides a feature for the device to tag an endpoint with zero or more read only
labels.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterFixedLabel : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLabelListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterFixedLabel (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster User Label
 *    The User Label Cluster provides a feature to tag an endpoint with zero or more labels.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterUserLabel : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLabelListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLabelListWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLabelListWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterUserLabel (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Boolean State
 *    This cluster provides an interface to a boolean state called StateValue.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterBooleanState : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStateValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterBooleanState (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster ICD Management
 *    Allows servers to ensure that listed clients are notified when a server is available for communication.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterICDManagement : MTRGenericCluster

- (void)registerClientWithParams:(MTRICDManagementClusterRegisterClientParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRICDManagementClusterRegisterClientResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)unregisterClientWithParams:(MTRICDManagementClusterUnregisterClientParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)stayActiveRequestWithParams:(MTRICDManagementClusterStayActiveRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRICDManagementClusterStayActiveResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeIdleModeDurationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveModeDurationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveModeThresholdWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRegisteredClientsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeICDCounterWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClientsSupportedPerFabricWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUserActiveModeTriggerHintWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUserActiveModeTriggerInstructionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperatingModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaximumCheckInBackOffWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterICDManagement (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Timer
 *    This cluster supports creating a simple timer functionality.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterTimer : MTRGenericCluster

- (void)setTimerWithParams:(MTRTimerClusterSetTimerParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)resetTimerWithParams:(MTRTimerClusterResetTimerParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)resetTimerWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)addTimeWithParams:(MTRTimerClusterAddTimeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)reduceTimeWithParams:(MTRTimerClusterReduceTimeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSetTimeWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTimeRemainingWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTimerStateWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterTimer (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster Oven Cavity Operational State
 *    This cluster supports remotely monitoring and, where supported, changing the operational state of an Oven.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterOvenCavityOperationalState : MTRGenericCluster

- (void)stopWithParams:(MTROvenCavityOperationalStateClusterStopParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROvenCavityOperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)stopWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROvenCavityOperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)startWithParams:(MTROvenCavityOperationalStateClusterStartParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROvenCavityOperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)startWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROvenCavityOperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePhaseListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentPhaseWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCountdownTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperationalStateListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperationalStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperationalErrorWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterOvenCavityOperationalState (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Oven Mode
 *    Attributes and commands for selecting a mode from a list of supported options.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterOvenMode : MTRGenericCluster

- (void)changeToModeWithParams:(MTROvenModeClusterChangeToModeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROvenModeClusterChangeToModeResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedModesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterOvenMode (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Laundry Dryer Controls
 *    This cluster provides a way to access options associated with the operation of
            a laundry dryer device type.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterLaundryDryerControls : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedDrynessLevelsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSelectedDrynessLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeSelectedDrynessLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeSelectedDrynessLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterLaundryDryerControls (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Mode Select
 *    Attributes and commands for selecting a mode from a list of supported options.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterModeSelect : MTRGenericCluster

- (void)changeToModeWithParams:(MTRModeSelectClusterChangeToModeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDescriptionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStandardNamespaceWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedModesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStartUpModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOnModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOnModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterModeSelect (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Laundry Washer Mode
 *    Attributes and commands for selecting a mode from a list of supported options.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterLaundryWasherMode : MTRGenericCluster

- (void)changeToModeWithParams:(MTRLaundryWasherModeClusterChangeToModeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRLaundryWasherModeClusterChangeToModeResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedModesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterLaundryWasherMode (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Refrigerator And Temperature Controlled Cabinet Mode
 *    Attributes and commands for selecting a mode from a list of supported options.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterRefrigeratorAndTemperatureControlledCabinetMode : MTRGenericCluster

- (void)changeToModeWithParams:(MTRRefrigeratorAndTemperatureControlledCabinetModeClusterChangeToModeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRRefrigeratorAndTemperatureControlledCabinetModeClusterChangeToModeResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedModesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterRefrigeratorAndTemperatureControlledCabinetMode (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Laundry Washer Controls
 *    This cluster supports remotely monitoring and controlling the different types of functionality available to a washing device, such as a washing machine.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterLaundryWasherControls : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSpinSpeedsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSpinSpeedCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeSpinSpeedCurrentWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeSpinSpeedCurrentWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfRinsesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeNumberOfRinsesWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeNumberOfRinsesWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedRinsesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterLaundryWasherControls (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster RVC Run Mode
 *    Attributes and commands for selecting a mode from a list of supported options.
 */
MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTRClusterRVCRunMode : MTRGenericCluster

- (void)changeToModeWithParams:(MTRRVCRunModeClusterChangeToModeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRRVCRunModeClusterChangeToModeResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedModesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterRVCRunMode (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

@end

/**
 * Cluster RVC Clean Mode
 *    Attributes and commands for selecting a mode from a list of supported options.
 */
MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTRClusterRVCCleanMode : MTRGenericCluster

- (void)changeToModeWithParams:(MTRRVCCleanModeClusterChangeToModeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRRVCCleanModeClusterChangeToModeResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedModesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterRVCCleanMode (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

@end

/**
 * Cluster Temperature Control
 *    Attributes and commands for configuring the temperature control, and reporting temperature.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterTemperatureControl : MTRGenericCluster

- (void)setTemperatureWithParams:(MTRTemperatureControlClusterSetTemperatureParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)setTemperatureWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTemperatureSetpointWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinTemperatureWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxTemperatureWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStepWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSelectedTemperatureLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedTemperatureLevelsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterTemperatureControl (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Refrigerator Alarm
 *    Attributes and commands for configuring the Refrigerator alarm.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterRefrigeratorAlarm : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaskWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterRefrigeratorAlarm (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Dishwasher Mode
 *    Attributes and commands for selecting a mode from a list of supported options.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterDishwasherMode : MTRGenericCluster

- (void)changeToModeWithParams:(MTRDishwasherModeClusterChangeToModeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRDishwasherModeClusterChangeToModeResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedModesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterDishwasherMode (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Air Quality
 *    Attributes for reporting air quality classification
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterAirQuality : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAirQualityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterAirQuality (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Smoke CO Alarm
 *    This cluster provides an interface for observing and managing the state of smoke and CO alarms.
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterSmokeCOAlarm : MTRGenericCluster

- (void)selfTestRequestWithParams:(MTRSmokeCOAlarmClusterSelfTestRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)selfTestRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeExpressedStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSmokeStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCOStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatteryAlertWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDeviceMutedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTestInProgressWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHardwareFaultAlertWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEndOfServiceAlertWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInterconnectSmokeAlarmWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInterconnectCOAlarmWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeContaminationStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSmokeSensitivityLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeSmokeSensitivityLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeSmokeSensitivityLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeExpiryDateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterSmokeCOAlarm (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Dishwasher Alarm
 *    Attributes and commands for configuring the Dishwasher alarm.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterDishwasherAlarm : MTRGenericCluster

- (void)resetWithParams:(MTRDishwasherAlarmClusterResetParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)modifyEnabledAlarmsWithParams:(MTRDishwasherAlarmClusterModifyEnabledAlarmsParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaskWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLatchWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterDishwasherAlarm (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Microwave Oven Mode
 *    Attributes and commands for selecting a mode from a list of supported options.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterMicrowaveOvenMode : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedModesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterMicrowaveOvenMode (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Microwave Oven Control
 *    Attributes and commands for configuring the microwave oven control, and reporting cooking stats.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterMicrowaveOvenControl : MTRGenericCluster

- (void)setCookingParametersWithParams:(MTRMicrowaveOvenControlClusterSetCookingParametersParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)setCookingParametersWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)addMoreTimeWithParams:(MTRMicrowaveOvenControlClusterAddMoreTimeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCookTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxCookTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerSettingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinPowerWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxPowerWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerStepWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedWattsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSelectedWattIndexWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWattRatingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterMicrowaveOvenControl (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Operational State
 *    This cluster supports remotely monitoring and, where supported, changing the operational state of any device where a state machine is a part of the operation.
 */
MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTRClusterOperationalState : MTRGenericCluster

- (void)pauseWithParams:(MTROperationalStateClusterPauseParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)pauseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)stopWithParams:(MTROperationalStateClusterStopParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)stopWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)startWithParams:(MTROperationalStateClusterStartParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)startWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)resumeWithParams:(MTROperationalStateClusterResumeParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)resumeWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTROperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePhaseListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentPhaseWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCountdownTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperationalStateListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperationalStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperationalErrorWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterOperationalState (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

@end

/**
 * Cluster RVC Operational State
 *    This cluster supports remotely monitoring and, where supported, changing the operational state of a Robotic Vacuum.
 */
MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4))
@interface MTRClusterRVCOperationalState : MTRGenericCluster

- (void)pauseWithParams:(MTRRVCOperationalStateClusterPauseParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRRVCOperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)pauseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRRVCOperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)resumeWithParams:(MTRRVCOperationalStateClusterResumeParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRRVCOperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)resumeWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRRVCOperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)goHomeWithParams:(MTRRVCOperationalStateClusterGoHomeParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRRVCOperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)goHomeWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRRVCOperationalStateClusterOperationalCommandResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePhaseListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentPhaseWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCountdownTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperationalStateListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperationalStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperationalErrorWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterRVCOperationalState (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

@end

/**
 * Cluster Scenes Management
 *    Attributes and commands for scene configuration and manipulation.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterScenesManagement : MTRGenericCluster

- (void)addSceneWithParams:(MTRScenesManagementClusterAddSceneParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRScenesManagementClusterAddSceneResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)viewSceneWithParams:(MTRScenesManagementClusterViewSceneParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRScenesManagementClusterViewSceneResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)removeSceneWithParams:(MTRScenesManagementClusterRemoveSceneParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRScenesManagementClusterRemoveSceneResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)removeAllScenesWithParams:(MTRScenesManagementClusterRemoveAllScenesParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRScenesManagementClusterRemoveAllScenesResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)storeSceneWithParams:(MTRScenesManagementClusterStoreSceneParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRScenesManagementClusterStoreSceneResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)recallSceneWithParams:(MTRScenesManagementClusterRecallSceneParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)getSceneMembershipWithParams:(MTRScenesManagementClusterGetSceneMembershipParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRScenesManagementClusterGetSceneMembershipResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)copySceneWithParams:(MTRScenesManagementClusterCopySceneParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRScenesManagementClusterCopySceneResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLastConfiguredByWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSceneTableSizeWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFabricSceneInfoWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterScenesManagement (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster HEPA Filter Monitoring
 *    Attributes and commands for monitoring HEPA filters in a device
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterHEPAFilterMonitoring : MTRGenericCluster

- (void)resetConditionWithParams:(MTRHEPAFilterMonitoringClusterResetConditionParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)resetConditionWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeConditionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDegradationDirectionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeChangeIndicationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInPlaceIndicatorWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLastChangedTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeLastChangedTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeLastChangedTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReplacementProductListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterHEPAFilterMonitoring (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Activated Carbon Filter Monitoring
 *    Attributes and commands for monitoring activated carbon filters in a device
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterActivatedCarbonFilterMonitoring : MTRGenericCluster

- (void)resetConditionWithParams:(MTRActivatedCarbonFilterMonitoringClusterResetConditionParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)resetConditionWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeConditionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDegradationDirectionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeChangeIndicationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInPlaceIndicatorWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLastChangedTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeLastChangedTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeLastChangedTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReplacementProductListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterActivatedCarbonFilterMonitoring (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Boolean State Configuration
 *    This cluster is used to configure a boolean sensor.
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterBooleanStateConfiguration : MTRGenericCluster

- (void)suppressAlarmWithParams:(MTRBooleanStateConfigurationClusterSuppressAlarmParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)enableDisableAlarmWithParams:(MTRBooleanStateConfigurationClusterEnableDisableAlarmParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentSensitivityLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeCurrentSensitivityLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeCurrentSensitivityLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedSensitivityLevelsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDefaultSensitivityLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAlarmsActiveWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAlarmsSuppressedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAlarmsEnabledWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAlarmsSupportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSensorFaultWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterBooleanStateConfiguration (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Valve Configuration and Control
 *    This cluster is used to configure a valve.
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterValveConfigurationAndControl : MTRGenericCluster

- (void)openWithParams:(MTRValveConfigurationAndControlClusterOpenParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)openWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)closeWithParams:(MTRValveConfigurationAndControlClusterCloseParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)closeWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOpenDurationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDefaultOpenDurationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeDefaultOpenDurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeDefaultOpenDurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAutoCloseTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRemainingDurationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTargetStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTargetLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDefaultOpenLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeDefaultOpenLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeDefaultOpenLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeValveFaultWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLevelStepWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterValveConfigurationAndControl (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Electrical Power Measurement
 *    This cluster provides a mechanism for querying data about electrical power as measured by the server.
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterElectricalPowerMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfMeasurementTypesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAccuracyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRangesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeVoltageWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReactiveCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApparentCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReactivePowerWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApparentPowerWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRMSVoltageWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRMSCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRMSPowerWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFrequencyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHarmonicCurrentsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHarmonicPhasesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerFactorWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNeutralCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterElectricalPowerMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Electrical Energy Measurement
 *    This cluster provides a mechanism for querying data about the electrical energy imported or provided by the server.
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterElectricalEnergyMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAccuracyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCumulativeEnergyImportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCumulativeEnergyExportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeriodicEnergyImportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeriodicEnergyExportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCumulativeEnergyResetWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterElectricalEnergyMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Water Heater Management
 *    This cluster is used to allow clients to control the operation of a hot water heating appliance so that it can be used with energy management.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterWaterHeaterManagement : MTRGenericCluster

- (void)boostWithParams:(MTRWaterHeaterManagementClusterBoostParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)cancelBoostWithParams:(MTRWaterHeaterManagementClusterCancelBoostParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)cancelBoostWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHeaterTypesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHeatDemandWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTankVolumeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEstimatedHeatRequiredWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTankPercentageWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBoostStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterWaterHeaterManagement (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Demand Response Load Control
 *    This cluster provides an interface to the functionality of Smart Energy Demand Response and Load Control.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterDemandResponseLoadControl : MTRGenericCluster

- (void)registerLoadControlProgramRequestWithParams:(MTRDemandResponseLoadControlClusterRegisterLoadControlProgramRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)unregisterLoadControlProgramRequestWithParams:(MTRDemandResponseLoadControlClusterUnregisterLoadControlProgramRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)addLoadControlEventRequestWithParams:(MTRDemandResponseLoadControlClusterAddLoadControlEventRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)removeLoadControlEventRequestWithParams:(MTRDemandResponseLoadControlClusterRemoveLoadControlEventRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)clearLoadControlEventsRequestWithParams:(MTRDemandResponseLoadControlClusterClearLoadControlEventsRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)clearLoadControlEventsRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLoadControlProgramsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfLoadControlProgramsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEventsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveEventsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfEventsPerProgramWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfTransitionsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDefaultRandomStartWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeDefaultRandomStartWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeDefaultRandomStartWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDefaultRandomDurationWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeDefaultRandomDurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeDefaultRandomDurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterDemandResponseLoadControl (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster Messages
 *    This cluster provides an interface for passing messages to be presented by a device.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterMessages : MTRGenericCluster

- (void)presentMessagesRequestWithParams:(MTRMessagesClusterPresentMessagesRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)cancelMessagesRequestWithParams:(MTRMessagesClusterCancelMessagesRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMessagesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveMessageIDsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterMessages (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Device Energy Management
 *    This cluster allows a client to manage the power draw of a device. An example of such a client could be an Energy Management System (EMS) which controls an Energy Smart Appliance (ESA).
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterDeviceEnergyManagement : MTRGenericCluster

- (void)powerAdjustRequestWithParams:(MTRDeviceEnergyManagementClusterPowerAdjustRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)cancelPowerAdjustRequestWithParams:(MTRDeviceEnergyManagementClusterCancelPowerAdjustRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)cancelPowerAdjustRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)startTimeAdjustRequestWithParams:(MTRDeviceEnergyManagementClusterStartTimeAdjustRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)pauseRequestWithParams:(MTRDeviceEnergyManagementClusterPauseRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)resumeRequestWithParams:(MTRDeviceEnergyManagementClusterResumeRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)resumeRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)modifyForecastRequestWithParams:(MTRDeviceEnergyManagementClusterModifyForecastRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)requestConstraintBasedForecastWithParams:(MTRDeviceEnergyManagementClusterRequestConstraintBasedForecastParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)cancelRequestWithParams:(MTRDeviceEnergyManagementClusterCancelRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)cancelRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeESATypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeESACanGenerateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeESAStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAbsMinPowerWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAbsMaxPowerWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerAdjustmentCapabilityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeForecastWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOptOutStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterDeviceEnergyManagement (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Energy EVSE
 *    Electric Vehicle Supply Equipment (EVSE) is equipment used to charge an Electric Vehicle (EV) or Plug-In Hybrid Electric Vehicle. This cluster provides an interface to the functionality of Electric Vehicle Supply Equipment (EVSE) management.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterEnergyEVSE : MTRGenericCluster

- (void)disableWithParams:(MTREnergyEVSEClusterDisableParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)disableWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)enableChargingWithParams:(MTREnergyEVSEClusterEnableChargingParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)enableDischargingWithParams:(MTREnergyEVSEClusterEnableDischargingParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)startDiagnosticsWithParams:(MTREnergyEVSEClusterStartDiagnosticsParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)startDiagnosticsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)setTargetsWithParams:(MTREnergyEVSEClusterSetTargetsParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)getTargetsWithParams:(MTREnergyEVSEClusterGetTargetsParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTREnergyEVSEClusterGetTargetsResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)getTargetsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTREnergyEVSEClusterGetTargetsResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)clearTargetsWithParams:(MTREnergyEVSEClusterClearTargetsParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)clearTargetsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupplyStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFaultStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeChargingEnabledUntilWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDischargingEnabledUntilWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCircuitCapacityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinimumChargeCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaximumChargeCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaximumDischargeCurrentWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUserMaximumChargeCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeUserMaximumChargeCurrentWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeUserMaximumChargeCurrentWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRandomizationDelayWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeRandomizationDelayWindowWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeRandomizationDelayWindowWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNextChargeStartTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNextChargeTargetTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNextChargeRequiredEnergyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNextChargeTargetSoCWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApproximateEVEfficiencyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeApproximateEVEfficiencyWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeApproximateEVEfficiencyWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStateOfChargeWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBatteryCapacityWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeVehicleIDWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSessionIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSessionDurationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSessionEnergyChargedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSessionEnergyDischargedWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterEnergyEVSE (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Energy Preference
 *    This cluster provides an interface to specify preferences for how devices should consume energy.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterEnergyPreference : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEnergyBalancesWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentEnergyBalanceWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeCurrentEnergyBalanceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeCurrentEnergyBalanceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEnergyPrioritiesWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLowPowerModeSensitivitiesWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentLowPowerModeSensitivityWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeCurrentLowPowerModeSensitivityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeCurrentLowPowerModeSensitivityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterEnergyPreference (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster Power Topology
 *    The Power Topology Cluster provides a mechanism for expressing how power is flowing between endpoints.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterPowerTopology : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAvailableEndpointsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveEndpointsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterPowerTopology (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Energy EVSE Mode
 *    Attributes and commands for selecting a mode from a list of supported options.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterEnergyEVSEMode : MTRGenericCluster

- (void)changeToModeWithParams:(MTREnergyEVSEModeClusterChangeToModeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTREnergyEVSEModeClusterChangeToModeResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedModesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterEnergyEVSEMode (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Water Heater Mode
 *    Attributes and commands for selecting a mode from a list of supported options.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterWaterHeaterMode : MTRGenericCluster

- (void)changeToModeWithParams:(MTRWaterHeaterModeClusterChangeToModeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRWaterHeaterModeClusterChangeToModeResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedModesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterWaterHeaterMode (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Device Energy Management Mode
 *    Attributes and commands for selecting a mode from a list of supported options.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterDeviceEnergyManagementMode : MTRGenericCluster

- (void)changeToModeWithParams:(MTRDeviceEnergyManagementModeClusterChangeToModeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRDeviceEnergyManagementModeClusterChangeToModeResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedModesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterDeviceEnergyManagementMode (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Door Lock
 *    An interface to a generic way to secure a door
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterDoorLock : MTRGenericCluster

- (void)lockDoorWithParams:(MTRDoorLockClusterLockDoorParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)lockDoorWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)unlockDoorWithParams:(MTRDoorLockClusterUnlockDoorParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)unlockDoorWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)unlockWithTimeoutWithParams:(MTRDoorLockClusterUnlockWithTimeoutParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setWeekDayScheduleWithParams:(MTRDoorLockClusterSetWeekDayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getWeekDayScheduleWithParams:(MTRDoorLockClusterGetWeekDayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRDoorLockClusterGetWeekDayScheduleResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearWeekDayScheduleWithParams:(MTRDoorLockClusterClearWeekDayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setYearDayScheduleWithParams:(MTRDoorLockClusterSetYearDayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getYearDayScheduleWithParams:(MTRDoorLockClusterGetYearDayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRDoorLockClusterGetYearDayScheduleResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearYearDayScheduleWithParams:(MTRDoorLockClusterClearYearDayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setHolidayScheduleWithParams:(MTRDoorLockClusterSetHolidayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getHolidayScheduleWithParams:(MTRDoorLockClusterGetHolidayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRDoorLockClusterGetHolidayScheduleResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearHolidayScheduleWithParams:(MTRDoorLockClusterClearHolidayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setUserWithParams:(MTRDoorLockClusterSetUserParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getUserWithParams:(MTRDoorLockClusterGetUserParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRDoorLockClusterGetUserResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearUserWithParams:(MTRDoorLockClusterClearUserParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setCredentialWithParams:(MTRDoorLockClusterSetCredentialParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRDoorLockClusterSetCredentialResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getCredentialStatusWithParams:(MTRDoorLockClusterGetCredentialStatusParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRDoorLockClusterGetCredentialStatusResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearCredentialWithParams:(MTRDoorLockClusterClearCredentialParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)unboltDoorWithParams:(MTRDoorLockClusterUnboltDoorParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)unboltDoorWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)setAliroReaderConfigWithParams:(MTRDoorLockClusterSetAliroReaderConfigParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)clearAliroReaderConfigWithParams:(MTRDoorLockClusterClearAliroReaderConfigParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)clearAliroReaderConfigWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLockStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLockTypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActuatorEnabledWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDoorStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDoorOpenEventsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDoorOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDoorOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDoorClosedEventsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDoorClosedEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeDoorClosedEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOpenPeriodWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfTotalUsersSupportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfPINUsersSupportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfRFIDUsersSupportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfWeekDaySchedulesSupportedPerUserWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfYearDaySchedulesSupportedPerUserWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfHolidaySchedulesSupportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxPINCodeLengthWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinPINCodeLengthWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxRFIDCodeLengthWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinRFIDCodeLengthWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCredentialRulesSupportWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfCredentialsSupportedPerUserWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLanguageWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLanguageWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLanguageWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLEDSettingsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLEDSettingsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLEDSettingsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAutoRelockTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeAutoRelockTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeAutoRelockTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSoundVolumeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSoundVolumeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSoundVolumeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperatingModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOperatingModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOperatingModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedOperatingModesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDefaultConfigurationRegisterWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEnableLocalProgrammingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnableLocalProgrammingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnableLocalProgrammingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEnableOneTouchLockingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnableOneTouchLockingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnableOneTouchLockingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEnableInsideStatusLEDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnableInsideStatusLEDWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnableInsideStatusLEDWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEnablePrivacyModeButtonWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnablePrivacyModeButtonWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEnablePrivacyModeButtonWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLocalProgrammingFeaturesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLocalProgrammingFeaturesWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLocalProgrammingFeaturesWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWrongCodeEntryLimitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWrongCodeEntryLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWrongCodeEntryLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUserCodeTemporaryDisableTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUserCodeTemporaryDisableTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUserCodeTemporaryDisableTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSendPINOverTheAirWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSendPINOverTheAirWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSendPINOverTheAirWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRequirePINforRemoteOperationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRequirePINforRemoteOperationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRequirePINforRemoteOperationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeExpiringUserTimeoutWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeExpiringUserTimeoutWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeExpiringUserTimeoutWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAliroReaderVerificationKeyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAliroReaderGroupIdentifierWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAliroReaderGroupSubIdentifierWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAliroExpeditedTransactionSupportedProtocolVersionsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAliroGroupResolvingKeyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAliroSupportedBLEUWBProtocolVersionsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAliroBLEAdvertisingVersionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfAliroCredentialIssuerKeysSupportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfAliroEndpointKeysSupportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterDoorLock (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Window Covering
 *    Provides an interface for controlling and adjusting automatic window coverings.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterWindowCovering : MTRGenericCluster

- (void)upOrOpenWithParams:(MTRWindowCoveringClusterUpOrOpenParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)upOrOpenWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)downOrCloseWithParams:(MTRWindowCoveringClusterDownOrCloseParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)downOrCloseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopMotionWithParams:(MTRWindowCoveringClusterStopMotionParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopMotionWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)goToLiftValueWithParams:(MTRWindowCoveringClusterGoToLiftValueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)goToLiftPercentageWithParams:(MTRWindowCoveringClusterGoToLiftPercentageParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)goToTiltValueWithParams:(MTRWindowCoveringClusterGoToTiltValueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)goToTiltPercentageWithParams:(MTRWindowCoveringClusterGoToTiltPercentageParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePhysicalClosedLimitLiftWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePhysicalClosedLimitTiltWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentPositionLiftWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentPositionTiltWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfActuationsLiftWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfActuationsTiltWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeConfigStatusWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentPositionLiftPercentageWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentPositionTiltPercentageWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperationalStatusWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTargetPositionLiftPercent100thsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTargetPositionTiltPercent100thsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEndProductTypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentPositionLiftPercent100thsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentPositionTiltPercent100thsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstalledOpenLimitLiftWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstalledClosedLimitLiftWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstalledOpenLimitTiltWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstalledClosedLimitTiltWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSafetyStatusWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterWindowCovering (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Closure Control
 *    This cluster provides an interface for controlling a Closure device.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterClosureControl : MTRGenericCluster

- (void)stopWithParams:(MTRClosureControlClusterStopParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)stopWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)moveToWithParams:(MTRClosureControlClusterMoveToParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)moveToWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)calibrateWithParams:(MTRClosureControlClusterCalibrateParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)calibrateWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)configureFallbackWithParams:(MTRClosureControlClusterConfigureFallbackParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)configureFallbackWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)cancelFallbackWithParams:(MTRClosureControlClusterCancelFallbackParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)cancelFallbackWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCountdownTimeWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMainStateWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentErrorListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOverallStateWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOverallTargetWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRestingProcedureWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTriggerConditionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTriggerPositionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWaitingDelayWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeKickoffTimerWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterClosureControl (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster Service Area
 *    The Service Area cluster provides an interface for controlling the areas where a device should operate, and for querying the current area being serviced.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterServiceArea : MTRGenericCluster

- (void)selectAreasWithParams:(MTRServiceAreaClusterSelectAreasParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRServiceAreaClusterSelectAreasResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)skipAreaWithParams:(MTRServiceAreaClusterSkipAreaParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRServiceAreaClusterSkipAreaResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedAreasWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedMapsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSelectedAreasWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentAreaWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEstimatedEndTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProgressWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterServiceArea (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Pump Configuration and Control
 *    An interface for configuring and controlling pumps.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterPumpConfigurationAndControl : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxPressureWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxSpeedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxFlowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinConstPressureWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxConstPressureWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinCompPressureWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxCompPressureWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinConstSpeedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxConstSpeedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinConstFlowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxConstFlowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinConstTempWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxConstTempWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePumpStatusWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEffectiveOperationModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEffectiveControlModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCapacityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSpeedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLifetimeRunningHoursWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLifetimeRunningHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLifetimeRunningHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLifetimeEnergyConsumedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLifetimeEnergyConsumedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLifetimeEnergyConsumedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOperationModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOperationModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOperationModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeControlModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeControlModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeControlModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterPumpConfigurationAndControl (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Thermostat
 *    An interface for configuring and controlling the functionality of a thermostat.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterThermostat : MTRGenericCluster

- (void)setpointRaiseLowerWithParams:(MTRThermostatClusterSetpointRaiseLowerParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setWeeklyScheduleWithParams:(MTRThermostatClusterSetWeeklyScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getWeeklyScheduleWithParams:(MTRThermostatClusterGetWeeklyScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRThermostatClusterGetWeeklyScheduleResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearWeeklyScheduleWithParams:(MTRThermostatClusterClearWeeklyScheduleParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)clearWeeklyScheduleWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)setActiveScheduleRequestWithParams:(MTRThermostatClusterSetActiveScheduleRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)setActivePresetRequestWithParams:(MTRThermostatClusterSetActivePresetRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)atomicRequestWithParams:(MTRThermostatClusterAtomicRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRThermostatClusterAtomicResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLocalTemperatureWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOutdoorTemperatureWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOccupancyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAbsMinHeatSetpointLimitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAbsMaxHeatSetpointLimitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAbsMinCoolSetpointLimitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAbsMaxCoolSetpointLimitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePICoolingDemandWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePIHeatingDemandWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHVACSystemTypeConfigurationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeHVACSystemTypeConfigurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeHVACSystemTypeConfigurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLocalTemperatureCalibrationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLocalTemperatureCalibrationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLocalTemperatureCalibrationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOccupiedCoolingSetpointWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOccupiedCoolingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOccupiedCoolingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOccupiedHeatingSetpointWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOccupiedHeatingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOccupiedHeatingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUnoccupiedCoolingSetpointWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUnoccupiedCoolingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUnoccupiedCoolingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUnoccupiedHeatingSetpointWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUnoccupiedHeatingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUnoccupiedHeatingSetpointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinHeatSetpointLimitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinHeatSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinHeatSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxHeatSetpointLimitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMaxHeatSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMaxHeatSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinCoolSetpointLimitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinCoolSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinCoolSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxCoolSetpointLimitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMaxCoolSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMaxCoolSetpointLimitWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinSetpointDeadBandWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinSetpointDeadBandWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinSetpointDeadBandWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRemoteSensingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRemoteSensingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRemoteSensingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeControlSequenceOfOperationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeControlSequenceOfOperationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeControlSequenceOfOperationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSystemModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSystemModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSystemModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeThermostatRunningModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStartOfWeekWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfWeeklyTransitionsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfDailyTransitionsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTemperatureSetpointHoldWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureSetpointHoldWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureSetpointHoldWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTemperatureSetpointHoldDurationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureSetpointHoldDurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureSetpointHoldDurationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeThermostatProgrammingOperationModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeThermostatProgrammingOperationModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeThermostatProgrammingOperationModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeThermostatRunningStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSetpointChangeSourceWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSetpointChangeAmountWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSetpointChangeSourceTimestampWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOccupiedSetbackWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOccupiedSetbackWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOccupiedSetbackWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOccupiedSetbackMinWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOccupiedSetbackMaxWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUnoccupiedSetbackWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUnoccupiedSetbackWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUnoccupiedSetbackWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUnoccupiedSetbackMinWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUnoccupiedSetbackMaxWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEmergencyHeatDeltaWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEmergencyHeatDeltaWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeEmergencyHeatDeltaWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeACTypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeACCapacityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACCapacityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACCapacityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeACRefrigerantTypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACRefrigerantTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACRefrigerantTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeACCompressorTypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACCompressorTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACCompressorTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeACErrorCodeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACErrorCodeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACErrorCodeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeACLouverPositionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACLouverPositionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACLouverPositionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeACCoilTemperatureWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeACCapacityformatWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACCapacityformatWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeACCapacityformatWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePresetTypesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeScheduleTypesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfPresetsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfSchedulesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfScheduleTransitionsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfScheduleTransitionPerDayWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePresetHandleWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveScheduleHandleWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePresetsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributePresetsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributePresetsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSchedulesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeSchedulesWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeSchedulesWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSetpointHoldExpiryTimestampWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterThermostat (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Fan Control
 *    An interface for controlling a fan in a heating/cooling system.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterFanControl : MTRGenericCluster

- (void)stepWithParams:(MTRFanControlClusterStepParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFanModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeFanModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeFanModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFanModeSequenceWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeFanModeSequenceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeFanModeSequenceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePercentSettingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePercentSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePercentSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePercentCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSpeedMaxWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSpeedSettingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSpeedSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSpeedSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSpeedCurrentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRockSupportWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRockSettingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRockSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeRockSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWindSupportWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWindSettingWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWindSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWindSettingWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAirflowDirectionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeAirflowDirectionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
- (void)writeAttributeAirflowDirectionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterFanControl (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Thermostat User Interface Configuration
 *    An interface for configuring the user interface of a thermostat (which may be remote from the thermostat).
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterThermostatUserInterfaceConfiguration : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTemperatureDisplayModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureDisplayModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeTemperatureDisplayModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeKeypadLockoutWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeKeypadLockoutWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeKeypadLockoutWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeScheduleProgrammingVisibilityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeScheduleProgrammingVisibilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeScheduleProgrammingVisibilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterThermostatUserInterfaceConfiguration (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Color Control
 *    Attributes and commands for controlling the color properties of a color-capable light.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterColorControl : MTRGenericCluster

- (void)moveToHueWithParams:(MTRColorControlClusterMoveToHueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveHueWithParams:(MTRColorControlClusterMoveHueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stepHueWithParams:(MTRColorControlClusterStepHueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveToSaturationWithParams:(MTRColorControlClusterMoveToSaturationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveSaturationWithParams:(MTRColorControlClusterMoveSaturationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stepSaturationWithParams:(MTRColorControlClusterStepSaturationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveToHueAndSaturationWithParams:(MTRColorControlClusterMoveToHueAndSaturationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveToColorWithParams:(MTRColorControlClusterMoveToColorParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveColorWithParams:(MTRColorControlClusterMoveColorParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stepColorWithParams:(MTRColorControlClusterStepColorParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveToColorTemperatureWithParams:(MTRColorControlClusterMoveToColorTemperatureParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enhancedMoveToHueWithParams:(MTRColorControlClusterEnhancedMoveToHueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enhancedMoveHueWithParams:(MTRColorControlClusterEnhancedMoveHueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enhancedStepHueWithParams:(MTRColorControlClusterEnhancedStepHueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)enhancedMoveToHueAndSaturationWithParams:(MTRColorControlClusterEnhancedMoveToHueAndSaturationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)colorLoopSetWithParams:(MTRColorControlClusterColorLoopSetParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopMoveStepWithParams:(MTRColorControlClusterStopMoveStepParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)moveColorTemperatureWithParams:(MTRColorControlClusterMoveColorTemperatureParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stepColorTemperatureWithParams:(MTRColorControlClusterStepColorTemperatureParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentHueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentSaturationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRemainingTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentXWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentYWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDriftCompensationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCompensationTextWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorTemperatureMiredsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOptionsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOptionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeOptionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNumberOfPrimariesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary1XWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary1YWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary1IntensityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary2XWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary2YWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary2IntensityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary3XWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary3YWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary3IntensityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary4XWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary4YWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary4IntensityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary5XWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary5YWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary5IntensityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary6XWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary6YWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePrimary6IntensityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWhitePointXWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWhitePointXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWhitePointXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWhitePointYWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWhitePointYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeWhitePointYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorPointRXWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointRXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointRXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorPointRYWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointRYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointRYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorPointRIntensityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointRIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointRIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorPointGXWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointGXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointGXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorPointGYWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointGYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointGYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorPointGIntensityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointGIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointGIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorPointBXWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointBXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointBXWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorPointBYWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointBYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointBYWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorPointBIntensityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointBIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeColorPointBIntensityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEnhancedCurrentHueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEnhancedColorModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorLoopActiveWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorLoopDirectionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorLoopTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorLoopStartEnhancedHueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorLoopStoredEnhancedHueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorCapabilitiesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorTempPhysicalMinMiredsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeColorTempPhysicalMaxMiredsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCoupleColorTempToLevelMinMiredsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStartUpColorTemperatureMiredsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpColorTemperatureMiredsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeStartUpColorTemperatureMiredsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterColorControl (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Ballast Configuration
 *    Attributes and commands for configuring a lighting ballast.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterBallastConfiguration : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributePhysicalMinLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePhysicalMaxLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBallastStatusWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMinLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxLevelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMaxLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeMaxLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeIntrinsicBallastFactorWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeIntrinsicBallastFactorWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeIntrinsicBallastFactorWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBallastFactorAdjustmentWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBallastFactorAdjustmentWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeBallastFactorAdjustmentWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLampQuantityWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLampTypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampTypeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLampManufacturerWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampManufacturerWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampManufacturerWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLampRatedHoursWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampRatedHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampRatedHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLampBurnHoursWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampBurnHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampBurnHoursWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLampAlarmModeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampAlarmModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampAlarmModeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLampBurnHoursTripPointWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampBurnHoursTripPointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeLampBurnHoursTripPointWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterBallastConfiguration (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Illuminance Measurement
 *    Attributes and commands for configuring the measurement of illuminance, and reporting illuminance measurements.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterIlluminanceMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLightSensorTypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterIlluminanceMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Temperature Measurement
 *    Attributes and commands for configuring the measurement of temperature, and reporting temperature measurements.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterTemperatureMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterTemperatureMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Pressure Measurement
 *    Attributes and commands for configuring the measurement of pressure, and reporting pressure measurements.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterPressureMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeScaledValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinScaledValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxScaledValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeScaledToleranceWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeScaleWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterPressureMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Flow Measurement
 *    Attributes and commands for configuring the measurement of flow, and reporting flow measurements.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterFlowMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterFlowMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Relative Humidity Measurement
 *    Attributes and commands for configuring the measurement of relative humidity, and reporting relative humidity measurements.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterRelativeHumidityMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeToleranceWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterRelativeHumidityMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Occupancy Sensing
 *    The server cluster provides an interface to occupancy sensing functionality based on one or more sensing modalities, including configuration and provision of notifications of occupancy status.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterOccupancySensing : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOccupancyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOccupancySensorTypeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOccupancySensorTypeBitmapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHoldTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeHoldTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributeHoldTimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHoldTimeLimitsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePIROccupiedToUnoccupiedDelayWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributePIROccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributePIROccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePIRUnoccupiedToOccupiedDelayWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributePIRUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributePIRUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePIRUnoccupiedToOccupiedThresholdWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributePIRUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributePIRUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUltrasonicOccupiedToUnoccupiedDelayWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUltrasonicOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUltrasonicOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUltrasonicUnoccupiedToOccupiedDelayWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUltrasonicUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUltrasonicUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUltrasonicUnoccupiedToOccupiedThresholdWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUltrasonicUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeUltrasonicUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePhysicalContactOccupiedToUnoccupiedDelayWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePhysicalContactOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePhysicalContactOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePhysicalContactUnoccupiedToOccupiedDelayWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePhysicalContactUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePhysicalContactUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePhysicalContactUnoccupiedToOccupiedThresholdWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePhysicalContactUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributePhysicalContactUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterOccupancySensing (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Carbon Monoxide Concentration Measurement
 *    Attributes for reporting carbon monoxide concentration measurements
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterCarbonMonoxideConcentrationMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUncertaintyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementUnitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementMediumWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLevelValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterCarbonMonoxideConcentrationMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Carbon Dioxide Concentration Measurement
 *    Attributes for reporting carbon dioxide concentration measurements
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterCarbonDioxideConcentrationMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUncertaintyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementUnitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementMediumWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLevelValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterCarbonDioxideConcentrationMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Nitrogen Dioxide Concentration Measurement
 *    Attributes for reporting nitrogen dioxide concentration measurements
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterNitrogenDioxideConcentrationMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUncertaintyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementUnitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementMediumWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLevelValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterNitrogenDioxideConcentrationMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Ozone Concentration Measurement
 *    Attributes for reporting ozone concentration measurements
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterOzoneConcentrationMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUncertaintyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementUnitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementMediumWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLevelValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterOzoneConcentrationMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster PM2.5 Concentration Measurement
 *    Attributes for reporting PM2.5 concentration measurements
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterPM25ConcentrationMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUncertaintyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementUnitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementMediumWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLevelValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterPM25ConcentrationMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Formaldehyde Concentration Measurement
 *    Attributes for reporting formaldehyde concentration measurements
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterFormaldehydeConcentrationMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUncertaintyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementUnitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementMediumWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLevelValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterFormaldehydeConcentrationMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster PM1 Concentration Measurement
 *    Attributes for reporting PM1 concentration measurements
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterPM1ConcentrationMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUncertaintyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementUnitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementMediumWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLevelValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterPM1ConcentrationMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster PM10 Concentration Measurement
 *    Attributes for reporting PM10 concentration measurements
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterPM10ConcentrationMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUncertaintyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementUnitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementMediumWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLevelValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterPM10ConcentrationMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Total Volatile Organic Compounds Concentration Measurement
 *    Attributes for reporting total volatile organic compounds concentration measurements
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterTotalVolatileOrganicCompoundsConcentrationMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUncertaintyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementUnitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementMediumWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLevelValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterTotalVolatileOrganicCompoundsConcentrationMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Radon Concentration Measurement
 *    Attributes for reporting radon concentration measurements
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRClusterRadonConcentrationMeasurement : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePeakMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageMeasuredValueWindowWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUncertaintyWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementUnitWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementMediumWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLevelValueWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterRadonConcentrationMeasurement (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Cluster Wi-Fi Network Management
 *    Functionality to retrieve operational information about a managed Wi-Fi network.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterWiFiNetworkManagement : MTRGenericCluster

- (void)networkPassphraseRequestWithParams:(MTRWiFiNetworkManagementClusterNetworkPassphraseRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRWiFiNetworkManagementClusterNetworkPassphraseResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)networkPassphraseRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRWiFiNetworkManagementClusterNetworkPassphraseResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSSIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePassphraseSurrogateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterWiFiNetworkManagement (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Thread Border Router Management
 *    Manage the Thread network of Thread Border Router
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterThreadBorderRouterManagement : MTRGenericCluster

- (void)getActiveDatasetRequestWithParams:(MTRThreadBorderRouterManagementClusterGetActiveDatasetRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRThreadBorderRouterManagementClusterDatasetResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)getActiveDatasetRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRThreadBorderRouterManagementClusterDatasetResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)getPendingDatasetRequestWithParams:(MTRThreadBorderRouterManagementClusterGetPendingDatasetRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRThreadBorderRouterManagementClusterDatasetResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)getPendingDatasetRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRThreadBorderRouterManagementClusterDatasetResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)setActiveDatasetRequestWithParams:(MTRThreadBorderRouterManagementClusterSetActiveDatasetRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)setPendingDatasetRequestWithParams:(MTRThreadBorderRouterManagementClusterSetPendingDatasetRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBorderRouterNameWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBorderAgentIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeThreadVersionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInterfaceEnabledWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveDatasetTimestampWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePendingDatasetTimestampWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterThreadBorderRouterManagement (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Thread Network Directory
 *    Manages the names and credentials of Thread networks visible to the user.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterThreadNetworkDirectory : MTRGenericCluster

- (void)addNetworkWithParams:(MTRThreadNetworkDirectoryClusterAddNetworkParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)removeNetworkWithParams:(MTRThreadNetworkDirectoryClusterRemoveNetworkParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)getOperationalDatasetWithParams:(MTRThreadNetworkDirectoryClusterGetOperationalDatasetParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRThreadNetworkDirectoryClusterOperationalDatasetResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePreferredExtendedPanIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributePreferredExtendedPanIDWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)writeAttributePreferredExtendedPanIDWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeThreadNetworksWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeThreadNetworkTableSizeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterThreadNetworkDirectory (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Wake on LAN
 *    This cluster provides an interface for managing low power mode on a device that supports the Wake On LAN protocol.
 */
MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRClusterWakeOnLAN : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMACAddressWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLinkLocalAddressWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterWakeOnLAN (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Channel
 *    This cluster provides an interface for controlling the current Channel on a device.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterChannel : MTRGenericCluster

- (void)changeChannelWithParams:(MTRChannelClusterChangeChannelParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRChannelClusterChangeChannelResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)changeChannelByNumberWithParams:(MTRChannelClusterChangeChannelByNumberParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)skipChannelWithParams:(MTRChannelClusterSkipChannelParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)getProgramGuideWithParams:(MTRChannelClusterGetProgramGuideParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRChannelClusterProgramGuideResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)getProgramGuideWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRChannelClusterProgramGuideResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)recordProgramWithParams:(MTRChannelClusterRecordProgramParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)cancelRecordProgramWithParams:(MTRChannelClusterCancelRecordProgramParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeChannelListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLineupWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentChannelWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterChannel (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Target Navigator
 *    This cluster provides an interface for UX navigation within a set of targets on a device or endpoint.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterTargetNavigator : MTRGenericCluster

- (void)navigateTargetWithParams:(MTRTargetNavigatorClusterNavigateTargetParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRTargetNavigatorClusterNavigateTargetResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTargetListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentTargetWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterTargetNavigator (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Media Playback
 *    This cluster provides an interface for controlling Media Playback (PLAY, PAUSE, etc) on a media device such as a TV or Speaker.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterMediaPlayback : MTRGenericCluster

- (void)playWithParams:(MTRMediaPlaybackClusterPlayParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)playWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)pauseWithParams:(MTRMediaPlaybackClusterPauseParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)pauseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopWithParams:(MTRMediaPlaybackClusterStopParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)startOverWithParams:(MTRMediaPlaybackClusterStartOverParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)startOverWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)previousWithParams:(MTRMediaPlaybackClusterPreviousParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)previousWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)nextWithParams:(MTRMediaPlaybackClusterNextParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)nextWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)rewindWithParams:(MTRMediaPlaybackClusterRewindParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)rewindWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)fastForwardWithParams:(MTRMediaPlaybackClusterFastForwardParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)fastForwardWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)skipForwardWithParams:(MTRMediaPlaybackClusterSkipForwardParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)skipBackwardWithParams:(MTRMediaPlaybackClusterSkipBackwardParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)seekWithParams:(MTRMediaPlaybackClusterSeekParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)activateAudioTrackWithParams:(MTRMediaPlaybackClusterActivateAudioTrackParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)activateTextTrackWithParams:(MTRMediaPlaybackClusterActivateTextTrackParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)deactivateTextTrackWithParams:(MTRMediaPlaybackClusterDeactivateTextTrackParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)deactivateTextTrackWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentStateWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStartTimeWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDurationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSampledPositionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePlaybackSpeedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSeekRangeEndWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSeekRangeStartWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveAudioTrackWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAvailableAudioTracksWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveTextTrackWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAvailableTextTracksWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterMediaPlayback (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Media Input
 *    This cluster provides an interface for controlling the Input Selector on a media device such as a TV.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterMediaInput : MTRGenericCluster

- (void)selectInputWithParams:(MTRMediaInputClusterSelectInputParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)showInputStatusWithParams:(MTRMediaInputClusterShowInputStatusParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)showInputStatusWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)hideInputStatusWithParams:(MTRMediaInputClusterHideInputStatusParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)hideInputStatusWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)renameInputWithParams:(MTRMediaInputClusterRenameInputParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInputListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentInputWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterMediaInput (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Low Power
 *    This cluster provides an interface for managing low power mode on a device.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterLowPower : MTRGenericCluster

- (void)sleepWithParams:(MTRLowPowerClusterSleepParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)sleepWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterLowPower (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Keypad Input
 *    This cluster provides an interface for controlling a device like a TV using action commands such as UP, DOWN, and SELECT.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterKeypadInput : MTRGenericCluster

- (void)sendKeyWithParams:(MTRKeypadInputClusterSendKeyParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRKeypadInputClusterSendKeyResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterKeypadInput (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Content Launcher
 *    This cluster provides an interface for launching content on a media player device such as a TV or Speaker.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterContentLauncher : MTRGenericCluster

- (void)launchContentWithParams:(MTRContentLauncherClusterLaunchContentParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRContentLauncherClusterLauncherResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)launchURLWithParams:(MTRContentLauncherClusterLaunchURLParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRContentLauncherClusterLauncherResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptHeaderWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedStreamingProtocolsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSupportedStreamingProtocolsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeSupportedStreamingProtocolsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterContentLauncher (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Audio Output
 *    This cluster provides an interface for controlling the Output on a media device such as a TV.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterAudioOutput : MTRGenericCluster

- (void)selectOutputWithParams:(MTRAudioOutputClusterSelectOutputParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)renameOutputWithParams:(MTRAudioOutputClusterRenameOutputParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOutputListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentOutputWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterAudioOutput (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Application Launcher
 *    This cluster provides an interface for launching content on a media player device such as a TV or Speaker.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterApplicationLauncher : MTRGenericCluster

- (void)launchAppWithParams:(MTRApplicationLauncherClusterLaunchAppParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)launchAppWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)stopAppWithParams:(MTRApplicationLauncherClusterStopAppParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)stopAppWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)hideAppWithParams:(MTRApplicationLauncherClusterHideAppParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)hideAppWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCatalogListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentAppWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeCurrentAppWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));
- (void)writeAttributeCurrentAppWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterApplicationLauncher (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Application Basic
 *    This cluster provides information about an application running on a TV or media player device which is represented as an endpoint.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterApplicationBasic : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeVendorNameWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeVendorIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApplicationNameWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProductIDWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApplicationWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStatusWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApplicationVersionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAllowedVendorListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterApplicationBasic (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Account Login
 *    This cluster provides commands that facilitate user account login on a Content App or a node. For example, a Content App running on a Video Player device, which is represented as an endpoint (see [TV Architecture]), can use this cluster to help make the user account on the Content App match the user account on the Client.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRClusterAccountLogin : MTRGenericCluster

- (void)getSetupPINWithParams:(MTRAccountLoginClusterGetSetupPINParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRAccountLoginClusterGetSetupPINResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)loginWithParams:(MTRAccountLoginClusterLoginParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)logoutWithParams:(MTRAccountLoginClusterLogoutParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)logoutWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterAccountLogin (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Content Control
 *    This cluster is used for managing the content control (including "parental control") settings on a media device such as a TV, or Set-top Box.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterContentControl : MTRGenericCluster

- (void)updatePINWithParams:(MTRContentControlClusterUpdatePINParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)resetPINWithParams:(MTRContentControlClusterResetPINParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRContentControlClusterResetPINResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)resetPINWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRContentControlClusterResetPINResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)enableWithParams:(MTRContentControlClusterEnableParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)enableWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)disableWithParams:(MTRContentControlClusterDisableParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)disableWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)addBonusTimeWithParams:(MTRContentControlClusterAddBonusTimeParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)addBonusTimeWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)setScreenDailyTimeWithParams:(MTRContentControlClusterSetScreenDailyTimeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)blockUnratedContentWithParams:(MTRContentControlClusterBlockUnratedContentParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)blockUnratedContentWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)unblockUnratedContentWithParams:(MTRContentControlClusterUnblockUnratedContentParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)unblockUnratedContentWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)setOnDemandRatingThresholdWithParams:(MTRContentControlClusterSetOnDemandRatingThresholdParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)setScheduledContentRatingThresholdWithParams:(MTRContentControlClusterSetScheduledContentRatingThresholdParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEnabledWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOnDemandRatingsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOnDemandRatingThresholdWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeScheduledContentRatingsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeScheduledContentRatingThresholdWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeScreenDailyTimeWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRemainingScreenTimeWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBlockUnratedWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterContentControl (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster Content App Observer
 *    This cluster provides an interface for sending targeted commands to an Observer of a Content App on a Video Player device such as a Streaming Media Player, Smart TV or Smart Screen. The cluster server for Content App Observer is implemented by an endpoint that communicates with a Content App, such as a Casting Video Client. The cluster client for Content App Observer is implemented by a Content App endpoint. A Content App is informed of the NodeId of an Observer when a binding is set on the Content App. The Content App can then send the ContentAppMessage to the Observer (server cluster), and the Observer responds with a ContentAppMessageResponse.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterContentAppObserver : MTRGenericCluster

- (void)contentAppMessageWithParams:(MTRContentAppObserverClusterContentAppMessageParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRContentAppObserverClusterContentAppMessageResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterContentAppObserver (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster Zone Management
 *    This cluster provides an interface to manage regions of interest, or Zones, which can be either manufacturer or user defined.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterZoneManagement : MTRGenericCluster

- (void)createTwoDCartesianZoneWithParams:(MTRZoneManagementClusterCreateTwoDCartesianZoneParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRZoneManagementClusterCreateTwoDCartesianZoneResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)updateTwoDCartesianZoneWithParams:(MTRZoneManagementClusterUpdateTwoDCartesianZoneParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)getTwoDCartesianZoneWithParams:(MTRZoneManagementClusterGetTwoDCartesianZoneParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRZoneManagementClusterGetTwoDCartesianZoneResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)getTwoDCartesianZoneWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRZoneManagementClusterGetTwoDCartesianZoneResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)removeZoneWithParams:(MTRZoneManagementClusterRemoveZoneParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedZoneSourcesWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeZonesWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTriggersWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeTriggersWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeTriggersWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSensitivityWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeSensitivityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeSensitivityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterZoneManagement (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster Camera AV Stream Management
 *    The Camera AV Stream Management cluster is used to allow clients to manage, control, and configure various audio, video, and snapshot streams on a camera.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterCameraAVStreamManagement : MTRGenericCluster

- (void)audioStreamAllocateWithParams:(MTRCameraAVStreamManagementClusterAudioStreamAllocateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRCameraAVStreamManagementClusterAudioStreamAllocateResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)audioStreamDeallocateWithParams:(MTRCameraAVStreamManagementClusterAudioStreamDeallocateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)videoStreamAllocateWithParams:(MTRCameraAVStreamManagementClusterVideoStreamAllocateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRCameraAVStreamManagementClusterVideoStreamAllocateResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)videoStreamModifyWithParams:(MTRCameraAVStreamManagementClusterVideoStreamModifyParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)videoStreamDeallocateWithParams:(MTRCameraAVStreamManagementClusterVideoStreamDeallocateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)snapshotStreamAllocateWithParams:(MTRCameraAVStreamManagementClusterSnapshotStreamAllocateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRCameraAVStreamManagementClusterSnapshotStreamAllocateResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)snapshotStreamModifyWithParams:(MTRCameraAVStreamManagementClusterSnapshotStreamModifyParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)snapshotStreamDeallocateWithParams:(MTRCameraAVStreamManagementClusterSnapshotStreamDeallocateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)setStreamPrioritiesWithParams:(MTRCameraAVStreamManagementClusterSetStreamPrioritiesParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)captureSnapshotWithParams:(MTRCameraAVStreamManagementClusterCaptureSnapshotParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRCameraAVStreamManagementClusterCaptureSnapshotResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxConcurrentVideoEncodersWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxEncodedPixelRateWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeVideoSensorParamsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNightVisionCapableWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMinViewportWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRateDistortionTradeOffPointsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxContentBufferSizeWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMicrophoneCapabilitiesWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSpeakerCapabilitiesWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTwoWayTalkSupportWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedSnapshotParamsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxNetworkBandwidthWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentFrameRateWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHDRModeEnabledWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeHDRModeEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeHDRModeEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedStreamUsagesWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAllocatedVideoStreamsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAllocatedAudioStreamsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAllocatedSnapshotStreamsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRankedVideoStreamPrioritiesListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSoftRecordingPrivacyModeEnabledWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeSoftRecordingPrivacyModeEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeSoftRecordingPrivacyModeEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSoftLivestreamPrivacyModeEnabledWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeSoftLivestreamPrivacyModeEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeSoftLivestreamPrivacyModeEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHardPrivacyModeOnWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNightVisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeNightVisionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeNightVisionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNightVisionIllumWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeNightVisionIllumWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeNightVisionIllumWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeViewportWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeViewportWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeViewportWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSpeakerMutedWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeSpeakerMutedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeSpeakerMutedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSpeakerVolumeLevelWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeSpeakerVolumeLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeSpeakerVolumeLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSpeakerMaxLevelWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSpeakerMinLevelWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMicrophoneMutedWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeMicrophoneMutedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeMicrophoneMutedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMicrophoneVolumeLevelWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeMicrophoneVolumeLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeMicrophoneVolumeLevelWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMicrophoneMaxLevelWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMicrophoneMinLevelWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMicrophoneAGCEnabledWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeMicrophoneAGCEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeMicrophoneAGCEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeImageRotationWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeImageRotationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeImageRotationWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeImageFlipHorizontalWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeImageFlipHorizontalWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeImageFlipHorizontalWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeImageFlipVerticalWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeImageFlipVerticalWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeImageFlipVerticalWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLocalVideoRecordingEnabledWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeLocalVideoRecordingEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeLocalVideoRecordingEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLocalSnapshotRecordingEnabledWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeLocalSnapshotRecordingEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeLocalSnapshotRecordingEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStatusLightEnabledWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeStatusLightEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeStatusLightEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStatusLightBrightnessWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeStatusLightBrightnessWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeStatusLightBrightnessWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterCameraAVStreamManagement (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster Camera AV Settings User Level Management
 *    This cluster provides an interface into controls associated with the operation of a device that provides pan, tilt, and zoom functions, either mechanically, or against a digital image.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterCameraAVSettingsUserLevelManagement : MTRGenericCluster

- (void)MPTZSetPositionWithParams:(MTRCameraAVSettingsUserLevelManagementClusterMPTZSetPositionParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)MPTZSetPositionWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)MPTZRelativeMoveWithParams:(MTRCameraAVSettingsUserLevelManagementClusterMPTZRelativeMoveParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)MPTZRelativeMoveWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)MPTZMoveToPresetWithParams:(MTRCameraAVSettingsUserLevelManagementClusterMPTZMoveToPresetParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)MPTZSavePresetWithParams:(MTRCameraAVSettingsUserLevelManagementClusterMPTZSavePresetParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)MPTZRemovePresetWithParams:(MTRCameraAVSettingsUserLevelManagementClusterMPTZRemovePresetParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)DPTZSetViewportWithParams:(MTRCameraAVSettingsUserLevelManagementClusterDPTZSetViewportParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)DPTZRelativeMoveWithParams:(MTRCameraAVSettingsUserLevelManagementClusterDPTZRelativeMoveParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMPTZPositionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxPresetsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMPTZPresetsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDPTZRelativeMoveWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeZoomMaxWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTiltMinWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTiltMaxWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributePanMinWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributePanMaxWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterCameraAVSettingsUserLevelManagement (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster WebRTC Transport Provider
 *    The WebRTC transport provider cluster provides a way for stream providers (e.g. Cameras) to stream or receive their data through WebRTC.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterWebRTCTransportProvider : MTRGenericCluster

- (void)solicitOfferWithParams:(MTRWebRTCTransportProviderClusterSolicitOfferParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRWebRTCTransportProviderClusterSolicitOfferResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)provideOfferWithParams:(MTRWebRTCTransportProviderClusterProvideOfferParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRWebRTCTransportProviderClusterProvideOfferResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)provideAnswerWithParams:(MTRWebRTCTransportProviderClusterProvideAnswerParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)provideICECandidatesWithParams:(MTRWebRTCTransportProviderClusterProvideICECandidatesParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)endSessionWithParams:(MTRWebRTCTransportProviderClusterEndSessionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentSessionsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterWebRTCTransportProvider (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster WebRTC Transport Requestor
 *    The WebRTC transport requestor cluster provides a way for stream consumers (e.g. Matter Stream Viewer) to establish a WebRTC connection with a stream provider.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterWebRTCTransportRequestor : MTRGenericCluster

- (void)offerWithParams:(MTRWebRTCTransportRequestorClusterOfferParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)answerWithParams:(MTRWebRTCTransportRequestorClusterAnswerParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)ICECandidatesWithParams:(MTRWebRTCTransportRequestorClusterICECandidatesParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)endWithParams:(MTRWebRTCTransportRequestorClusterEndParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentSessionsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterWebRTCTransportRequestor (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster Push AV Stream Transport
 *    This cluster implements the upload of Audio and Video streams from the Push AV Stream Transport Cluster using suitable push-based transports.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterPushAVStreamTransport : MTRGenericCluster

- (void)allocatePushTransportWithParams:(MTRPushAVStreamTransportClusterAllocatePushTransportParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRPushAVStreamTransportClusterAllocatePushTransportResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)deallocatePushTransportWithParams:(MTRPushAVStreamTransportClusterDeallocatePushTransportParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)modifyPushTransportWithParams:(MTRPushAVStreamTransportClusterModifyPushTransportParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)setTransportStatusWithParams:(MTRPushAVStreamTransportClusterSetTransportStatusParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)manuallyTriggerTransportWithParams:(MTRPushAVStreamTransportClusterManuallyTriggerTransportParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)findTransportWithParams:(MTRPushAVStreamTransportClusterFindTransportParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRPushAVStreamTransportClusterFindTransportResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)findTransportWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRPushAVStreamTransportClusterFindTransportResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedContainerFormatsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedIngestMethodsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentConnectionsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterPushAVStreamTransport (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster Chime
 *    This cluster provides facilities to configure and play Chime sounds, such as those used in a doorbell.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterChime : MTRGenericCluster

- (void)playChimeSoundWithParams:(MTRChimeClusterPlayChimeSoundParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)playChimeSoundWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstalledChimeSoundsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSelectedChimeWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeSelectedChimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeSelectedChimeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEnabledWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeEnabledWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterChime (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster Ecosystem Information
 *    Provides extended device information for all the logical devices represented by a Bridged Node.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterEcosystemInformation : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDeviceDirectoryWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLocationDirectoryWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterEcosystemInformation (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster Commissioner Control
 *    Supports the ability for clients to request the commissioning of themselves or other nodes onto a fabric which the cluster server can commission onto.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRClusterCommissionerControl : MTRGenericCluster

- (void)requestCommissioningApprovalWithParams:(MTRCommissionerControlClusterRequestCommissioningApprovalParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
- (void)commissionNodeWithParams:(MTRCommissionerControlClusterCommissionNodeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRCommissionerControlClusterReverseOpenCommissioningWindowParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSupportedDeviceCategoriesWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterCommissionerControl (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

/**
 * Cluster TLS Certificate Management
 *    This Cluster is used to manage TLS Client Certificates and to provision
      TLS endpoints with enough information to facilitate subsequent connection.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterTLSCertificateManagement : MTRGenericCluster

- (void)provisionRootCertificateWithParams:(MTRTLSCertificateManagementClusterProvisionRootCertificateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRTLSCertificateManagementClusterProvisionRootCertificateResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)findRootCertificateWithParams:(MTRTLSCertificateManagementClusterFindRootCertificateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRTLSCertificateManagementClusterFindRootCertificateResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)lookupRootCertificateWithParams:(MTRTLSCertificateManagementClusterLookupRootCertificateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRTLSCertificateManagementClusterLookupRootCertificateResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)removeRootCertificateWithParams:(MTRTLSCertificateManagementClusterRemoveRootCertificateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)TLSClientCSRWithParams:(MTRTLSCertificateManagementClusterTLSClientCSRParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRTLSCertificateManagementClusterTLSClientCSRResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)provisionClientCertificateWithParams:(MTRTLSCertificateManagementClusterProvisionClientCertificateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)findClientCertificateWithParams:(MTRTLSCertificateManagementClusterFindClientCertificateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRTLSCertificateManagementClusterFindClientCertificateResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)lookupClientCertificateWithParams:(MTRTLSCertificateManagementClusterLookupClientCertificateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRTLSCertificateManagementClusterLookupClientCertificateResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)removeClientCertificateWithParams:(MTRTLSCertificateManagementClusterRemoveClientCertificateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxRootCertificatesWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProvisionedRootCertificatesWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxClientCertificatesWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProvisionedClientCertificatesWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterTLSCertificateManagement (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster TLS Client Management
 *    This Cluster is used to provision TLS Endpoints with enough information to facilitate subsequent connection.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterTLSClientManagement : MTRGenericCluster

- (void)provisionEndpointWithParams:(MTRTLSClientManagementClusterProvisionEndpointParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRTLSClientManagementClusterProvisionEndpointResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)findEndpointWithParams:(MTRTLSClientManagementClusterFindEndpointParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRTLSClientManagementClusterFindEndpointResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)removeEndpointWithParams:(MTRTLSClientManagementClusterRemoveEndpointParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMaxProvisionedWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeProvisionedEndpointsWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterTLSClientManagement (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

/**
 * Cluster Unit Testing
 *    The Test Cluster is meant to validate the generated code
 */
MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRClusterUnitTesting : MTRGenericCluster

- (void)testWithParams:(MTRUnitTestingClusterTestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testNotHandledWithParams:(MTRUnitTestingClusterTestNotHandledParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testNotHandledWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testSpecificWithParams:(MTRUnitTestingClusterTestSpecificParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestSpecificResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testSpecificWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestSpecificResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testUnknownCommandWithParams:(MTRUnitTestingClusterTestUnknownCommandParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testUnknownCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testAddArgumentsWithParams:(MTRUnitTestingClusterTestAddArgumentsParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestAddArgumentsResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testSimpleArgumentRequestWithParams:(MTRUnitTestingClusterTestSimpleArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestSimpleArgumentResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testStructArrayArgumentRequestWithParams:(MTRUnitTestingClusterTestStructArrayArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestStructArrayArgumentResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testStructArgumentRequestWithParams:(MTRUnitTestingClusterTestStructArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterBooleanResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testNestedStructArgumentRequestWithParams:(MTRUnitTestingClusterTestNestedStructArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterBooleanResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testListStructArgumentRequestWithParams:(MTRUnitTestingClusterTestListStructArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterBooleanResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testListInt8UArgumentRequestWithParams:(MTRUnitTestingClusterTestListInt8UArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterBooleanResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testNestedStructListArgumentRequestWithParams:(MTRUnitTestingClusterTestNestedStructListArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterBooleanResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testListNestedStructListArgumentRequestWithParams:(MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterBooleanResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testListInt8UReverseRequestWithParams:(MTRUnitTestingClusterTestListInt8UReverseRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestListInt8UReverseResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testEnumsRequestWithParams:(MTRUnitTestingClusterTestEnumsRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestEnumsResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testNullableOptionalRequestWithParams:(MTRUnitTestingClusterTestNullableOptionalRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestNullableOptionalResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testNullableOptionalRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestNullableOptionalResponseParams * _Nullable data, NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)testComplexNullableOptionalRequestWithParams:(MTRUnitTestingClusterTestComplexNullableOptionalRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestComplexNullableOptionalResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)simpleStructEchoRequestWithParams:(MTRUnitTestingClusterSimpleStructEchoRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterSimpleStructResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)timedInvokeRequestWithParams:(MTRUnitTestingClusterTimedInvokeRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)timedInvokeRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testSimpleOptionalArgumentRequestWithParams:(MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testSimpleOptionalArgumentRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4));
- (void)testEmitTestEventRequestWithParams:(MTRUnitTestingClusterTestEmitTestEventRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestEmitTestEventResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testEmitTestFabricScopedEventRequestWithParams:(MTRUnitTestingClusterTestEmitTestFabricScopedEventRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)testBatchHelperRequestWithParams:(MTRUnitTestingClusterTestBatchHelperRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestBatchHelperResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)testSecondBatchHelperRequestWithParams:(MTRUnitTestingClusterTestSecondBatchHelperRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestBatchHelperResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)stringEchoRequestWithParams:(MTRUnitTestingClusterStringEchoRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterStringEchoResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)globalEchoRequestWithParams:(MTRUnitTestingClusterGlobalEchoRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterGlobalEchoResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;
- (void)testDifferentVendorMeiRequestWithParams:(MTRUnitTestingClusterTestDifferentVendorMeiRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRUnitTestingClusterTestDifferentVendorMeiResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBooleanWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBitmap8WithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBitmap16WithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBitmap32WithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap32WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap32WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBitmap64WithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap64WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeBitmap64WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt8uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt16uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt24uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt24uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt24uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt32uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt32uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt32uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt40uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt40uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt40uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt48uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt48uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt48uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt56uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt56uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt56uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt64uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt64uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt64uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt8sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt16sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt24sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt24sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt24sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt32sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt32sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt32sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt40sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt40sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt40sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt48sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt48sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt48sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt56sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt56sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt56sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInt64sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt64sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeInt64sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEnum8WithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEnum8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEnum8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEnum16WithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEnum16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEnum16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFloatSingleWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeFloatSingleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeFloatSingleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFloatDoubleWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeFloatDoubleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeFloatDoubleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOctetStringWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeListInt8uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeListOctetStringWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeListStructOctetStringWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListStructOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListStructOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLongOctetStringWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLongOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLongOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCharStringWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLongCharStringWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLongCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeLongCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEpochUsWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEpochUsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEpochUsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEpochSWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEpochSWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEpochSWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeVendorIdWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeVendorIdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeVendorIdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeListNullablesAndOptionalsStructWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListNullablesAndOptionalsStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListNullablesAndOptionalsStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeEnumAttrWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEnumAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeEnumAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStructAttrWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeStructAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeStructAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRangeRestrictedInt8uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRangeRestrictedInt8sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRangeRestrictedInt16uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRangeRestrictedInt16sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeRangeRestrictedInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeListLongOctetStringWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListLongOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListLongOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeListFabricScopedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListFabricScopedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeListFabricScopedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTimedWriteBooleanWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeTimedWriteBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeTimedWriteBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneralErrorBooleanWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeGeneralErrorBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeGeneralErrorBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterErrorBooleanWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeClusterErrorBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeClusterErrorBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGlobalEnumWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeGlobalEnumWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeGlobalEnumWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGlobalStructWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeGlobalStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeGlobalStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeUnsupportedWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeUnsupportedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeUnsupportedWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReadFailureCodeWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeReadFailureCodeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeReadFailureCodeWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFailureInt32UWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeFailureInt32UWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeFailureInt32UWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableBooleanWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBooleanWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableBitmap8WithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableBitmap16WithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableBitmap32WithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap32WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap32WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableBitmap64WithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap64WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableBitmap64WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt8uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt16uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt24uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt24uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt24uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt32uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt32uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt32uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt40uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt40uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt40uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt48uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt48uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt48uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt56uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt56uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt56uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt64uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt64uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt64uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt8sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt16sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt24sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt24sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt24sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt32sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt32sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt32sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt40sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt40sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt40sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt48sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt48sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt48sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt56sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt56sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt56sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableInt64sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt64sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableInt64sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableEnum8WithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableEnum8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableEnum8WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableEnum16WithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableEnum16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableEnum16WithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableFloatSingleWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableFloatSingleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableFloatSingleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableFloatDoubleWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableFloatDoubleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableFloatDoubleWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableOctetStringWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableOctetStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableCharStringWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableCharStringWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableEnumAttrWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableEnumAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableEnumAttrWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableStructWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableRangeRestrictedInt8uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableRangeRestrictedInt8sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt8sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableRangeRestrictedInt16uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt16uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableRangeRestrictedInt16sWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeNullableRangeRestrictedInt16sWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWriteOnlyInt8uWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeWriteOnlyInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
- (void)writeAttributeWriteOnlyInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableGlobalEnumWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeNullableGlobalEnumWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeNullableGlobalEnumWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNullableGlobalStructWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeNullableGlobalStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeNullableGlobalStructWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeiInt8uWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeMeiInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeMeiInt8uWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterUnitTesting (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

/**
 * Cluster Sample MEI
 *    The Sample MEI cluster showcases a cluster manufacturer extensions
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRClusterSampleMEI : MTRGenericCluster

- (void)pingWithParams:(MTRSampleMEIClusterPingParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_PROVISIONALLY_AVAILABLE;
- (void)pingWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_PROVISIONALLY_AVAILABLE;
- (void)addArgumentsWithParams:(MTRSampleMEIClusterAddArgumentsParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(void (^)(MTRSampleMEIClusterAddArgumentsResponseParams * _Nullable data, NSError * _Nullable error))completion MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFlipFlopWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeFlipFlopWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_PROVISIONALLY_AVAILABLE;
- (void)writeAttributeFlipFlopWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_PROVISIONALLY_AVAILABLE;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterSampleMEI (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_PROVISIONALLY_AVAILABLE;

@end

MTR_DEPRECATED("Please use MTRClusterBasicInformation", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRClusterBasic : MTRClusterBasicInformation
@end

MTR_DEPRECATED("Please use MTRClusterOTASoftwareUpdateProvider", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRClusterOtaSoftwareUpdateProvider : MTRClusterOTASoftwareUpdateProvider
@end

MTR_DEPRECATED("Please use MTRClusterOTASoftwareUpdateRequestor", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRClusterOtaSoftwareUpdateRequestor : MTRClusterOTASoftwareUpdateRequestor
@end

MTR_DEPRECATED("Please use MTRClusterBridgedDeviceBasicInformation", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
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
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)identifyWithParams:(MTRIdentifyClusterIdentifyParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use identifyWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)triggerEffectWithParams:(MTRIdentifyClusterTriggerEffectParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use triggerEffectWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterGroups (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)addGroupWithParams:(MTRGroupsClusterAddGroupParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRGroupsClusterAddGroupResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use addGroupWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)viewGroupWithParams:(MTRGroupsClusterViewGroupParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRGroupsClusterViewGroupResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use viewGroupWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getGroupMembershipWithParams:(MTRGroupsClusterGetGroupMembershipParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRGroupsClusterGetGroupMembershipResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use getGroupMembershipWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)removeGroupWithParams:(MTRGroupsClusterRemoveGroupParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRGroupsClusterRemoveGroupResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use removeGroupWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)removeAllGroupsWithParams:(MTRGroupsClusterRemoveAllGroupsParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use removeAllGroupsWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)removeAllGroupsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use removeAllGroupsWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)addGroupIfIdentifyingWithParams:(MTRGroupsClusterAddGroupIfIdentifyingParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use addGroupIfIdentifyingWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterOnOff (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)offWithParams:(MTROnOffClusterOffParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use offWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)offWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use offWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)onWithParams:(MTROnOffClusterOnParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use onWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)onWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use onWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)toggleWithParams:(MTROnOffClusterToggleParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use toggleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)toggleWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use toggleWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)offWithEffectWithParams:(MTROnOffClusterOffWithEffectParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use offWithEffectWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)onWithRecallGlobalSceneWithParams:(MTROnOffClusterOnWithRecallGlobalSceneParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use onWithRecallGlobalSceneWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)onWithRecallGlobalSceneWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use onWithRecallGlobalSceneWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)onWithTimedOffWithParams:(MTROnOffClusterOnWithTimedOffParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use onWithTimedOffWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterLevelControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)moveToLevelWithParams:(MTRLevelControlClusterMoveToLevelParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveToLevelWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveWithParams:(MTRLevelControlClusterMoveParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stepWithParams:(MTRLevelControlClusterStepParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use stepWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopWithParams:(MTRLevelControlClusterStopParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use stopWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveToLevelWithOnOffWithParams:(MTRLevelControlClusterMoveToLevelWithOnOffParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveToLevelWithOnOffWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveWithOnOffWithParams:(MTRLevelControlClusterMoveWithOnOffParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveWithOnOffWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stepWithOnOffWithParams:(MTRLevelControlClusterStepWithOnOffParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use stepWithOnOffWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopWithOnOffWithParams:(MTRLevelControlClusterStopWithOnOffParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use stopWithOnOffWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveToClosestFrequencyWithParams:(MTRLevelControlClusterMoveToClosestFrequencyParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveToClosestFrequencyWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterDescriptor (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (NSDictionary<NSString *, id> *)readAttributeDeviceListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("Please use readAttributeDeviceTypeListWithParams on MTRClusterDescriptor", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterBinding (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterAccessControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (NSDictionary<NSString *, id> *)readAttributeAclWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("Please use readAttributeACLWithParams on MTRClusterAccessControl", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeAclWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("Please use writeAttributeACLWithValue on MTRClusterAccessControl", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeAclWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("Please use writeAttributeACLWithValue on MTRClusterAccessControl", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterActions (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)instantActionWithParams:(MTRActionsClusterInstantActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use instantActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)instantActionWithTransitionWithParams:(MTRActionsClusterInstantActionWithTransitionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use instantActionWithTransitionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)startActionWithParams:(MTRActionsClusterStartActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use startActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)startActionWithDurationWithParams:(MTRActionsClusterStartActionWithDurationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use startActionWithDurationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopActionWithParams:(MTRActionsClusterStopActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use stopActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)pauseActionWithParams:(MTRActionsClusterPauseActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use pauseActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)pauseActionWithDurationWithParams:(MTRActionsClusterPauseActionWithDurationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use pauseActionWithDurationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)resumeActionWithParams:(MTRActionsClusterResumeActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use resumeActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enableActionWithParams:(MTRActionsClusterEnableActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use enableActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enableActionWithDurationWithParams:(MTRActionsClusterEnableActionWithDurationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use enableActionWithDurationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)disableActionWithParams:(MTRActionsClusterDisableActionParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use disableActionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)disableActionWithDurationWithParams:(MTRActionsClusterDisableActionWithDurationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use disableActionWithDurationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterBasic (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)mfgSpecificPingWithParams:(MTRBasicClusterMfgSpecificPingParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use mfgSpecificPingWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)mfgSpecificPingWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use mfgSpecificPingWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterOtaSoftwareUpdateProvider (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)queryImageWithParams:(MTROtaSoftwareUpdateProviderClusterQueryImageParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use queryImageWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)applyUpdateRequestWithParams:(MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use applyUpdateRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)notifyUpdateAppliedWithParams:(MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use notifyUpdateAppliedWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterOtaSoftwareUpdateRequestor (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)announceOtaProviderWithParams:(MTROtaSoftwareUpdateRequestorClusterAnnounceOtaProviderParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use announceOTAProviderWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributeDefaultOtaProvidersWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("Please use readAttributeDefaultOTAProvidersWithParams on MTRClusterOTASoftwareUpdateRequestor", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeDefaultOtaProvidersWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("Please use writeAttributeDefaultOTAProvidersWithValue on MTRClusterOTASoftwareUpdateRequestor", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeDefaultOtaProvidersWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("Please use writeAttributeDefaultOTAProvidersWithValue on MTRClusterOTASoftwareUpdateRequestor", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterLocalizationConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterTimeFormatLocalization (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterUnitLocalization (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterPowerSourceConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterPowerSource (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterGeneralCommissioning (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)armFailSafeWithParams:(MTRGeneralCommissioningClusterArmFailSafeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRGeneralCommissioningClusterArmFailSafeResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use armFailSafeWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setRegulatoryConfigWithParams:(MTRGeneralCommissioningClusterSetRegulatoryConfigParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use setRegulatoryConfigWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)commissioningCompleteWithParams:(MTRGeneralCommissioningClusterCommissioningCompleteParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRGeneralCommissioningClusterCommissioningCompleteResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use commissioningCompleteWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)commissioningCompleteWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRGeneralCommissioningClusterCommissioningCompleteResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use commissioningCompleteWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterNetworkCommissioning (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)scanNetworksWithParams:(MTRNetworkCommissioningClusterScanNetworksParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRNetworkCommissioningClusterScanNetworksResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use scanNetworksWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)addOrUpdateWiFiNetworkWithParams:(MTRNetworkCommissioningClusterAddOrUpdateWiFiNetworkParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use addOrUpdateWiFiNetworkWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)addOrUpdateThreadNetworkWithParams:(MTRNetworkCommissioningClusterAddOrUpdateThreadNetworkParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use addOrUpdateThreadNetworkWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)removeNetworkWithParams:(MTRNetworkCommissioningClusterRemoveNetworkParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use removeNetworkWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)connectNetworkWithParams:(MTRNetworkCommissioningClusterConnectNetworkParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRNetworkCommissioningClusterConnectNetworkResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use connectNetworkWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)reorderNetworkWithParams:(MTRNetworkCommissioningClusterReorderNetworkParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRNetworkCommissioningClusterNetworkConfigResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use reorderNetworkWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterDiagnosticLogs (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)retrieveLogsRequestWithParams:(MTRDiagnosticLogsClusterRetrieveLogsRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use retrieveLogsRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterGeneralDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)testEventTriggerWithParams:(MTRGeneralDiagnosticsClusterTestEventTriggerParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use testEventTriggerWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributeBootReasonsWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("Please use readAttributeBootReasonWithParams on MTRClusterGeneralDiagnostics", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterSoftwareDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)resetWatermarksWithParams:(MTRSoftwareDiagnosticsClusterResetWatermarksParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use resetWatermarksWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)resetWatermarksWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use resetWatermarksWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterThreadNetworkDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)resetCountsWithParams:(MTRThreadNetworkDiagnosticsClusterResetCountsParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use resetCountsWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use resetCountsWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributeNeighborTableListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("Please use readAttributeNeighborTableWithParams on MTRClusterThreadNetworkDiagnostics", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributeRouteTableListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("Please use readAttributeRouteTableWithParams on MTRClusterThreadNetworkDiagnostics", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterWiFiNetworkDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)resetCountsWithParams:(MTRWiFiNetworkDiagnosticsClusterResetCountsParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use resetCountsWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use resetCountsWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributeBssidWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("Please use readAttributeBSSIDWithParams on MTRClusterWiFiNetworkDiagnostics", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributeRssiWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("Please use readAttributeRSSIWithParams on MTRClusterWiFiNetworkDiagnostics", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterEthernetNetworkDiagnostics (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)resetCountsWithParams:(MTREthernetNetworkDiagnosticsClusterResetCountsParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use resetCountsWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)resetCountsWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use resetCountsWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterBridgedDeviceBasic (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterSwitch (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterAdministratorCommissioning (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)openCommissioningWindowWithParams:(MTRAdministratorCommissioningClusterOpenCommissioningWindowParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use openCommissioningWindowWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)openBasicCommissioningWindowWithParams:(MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use openBasicCommissioningWindowWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)revokeCommissioningWithParams:(MTRAdministratorCommissioningClusterRevokeCommissioningParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use revokeCommissioningWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)revokeCommissioningWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use revokeCommissioningWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterOperationalCredentials (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)attestationRequestWithParams:(MTROperationalCredentialsClusterAttestationRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTROperationalCredentialsClusterAttestationResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use attestationRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)certificateChainRequestWithParams:(MTROperationalCredentialsClusterCertificateChainRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTROperationalCredentialsClusterCertificateChainResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use certificateChainRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)CSRRequestWithParams:(MTROperationalCredentialsClusterCSRRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTROperationalCredentialsClusterCSRResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use CSRRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)addNOCWithParams:(MTROperationalCredentialsClusterAddNOCParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use addNOCWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)updateNOCWithParams:(MTROperationalCredentialsClusterUpdateNOCParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use updateNOCWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)updateFabricLabelWithParams:(MTROperationalCredentialsClusterUpdateFabricLabelParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use updateFabricLabelWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)removeFabricWithParams:(MTROperationalCredentialsClusterRemoveFabricParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use removeFabricWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)addTrustedRootCertificateWithParams:(MTROperationalCredentialsClusterAddTrustedRootCertificateParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use addTrustedRootCertificateWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterGroupKeyManagement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)keySetWriteWithParams:(MTRGroupKeyManagementClusterKeySetWriteParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use keySetWriteWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)keySetReadWithParams:(MTRGroupKeyManagementClusterKeySetReadParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRGroupKeyManagementClusterKeySetReadResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use keySetReadWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)keySetRemoveWithParams:(MTRGroupKeyManagementClusterKeySetRemoveParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use keySetRemoveWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)keySetReadAllIndicesWithParams:(MTRGroupKeyManagementClusterKeySetReadAllIndicesParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use keySetReadAllIndicesWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterFixedLabel (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterUserLabel (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterBooleanState (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterModeSelect (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)changeToModeWithParams:(MTRModeSelectClusterChangeToModeParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use changeToModeWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterDoorLock (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)lockDoorWithParams:(MTRDoorLockClusterLockDoorParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use lockDoorWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)unlockDoorWithParams:(MTRDoorLockClusterUnlockDoorParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use unlockDoorWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)unlockWithTimeoutWithParams:(MTRDoorLockClusterUnlockWithTimeoutParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use unlockWithTimeoutWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setWeekDayScheduleWithParams:(MTRDoorLockClusterSetWeekDayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use setWeekDayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getWeekDayScheduleWithParams:(MTRDoorLockClusterGetWeekDayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRDoorLockClusterGetWeekDayScheduleResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use getWeekDayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearWeekDayScheduleWithParams:(MTRDoorLockClusterClearWeekDayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use clearWeekDayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setYearDayScheduleWithParams:(MTRDoorLockClusterSetYearDayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use setYearDayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getYearDayScheduleWithParams:(MTRDoorLockClusterGetYearDayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRDoorLockClusterGetYearDayScheduleResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use getYearDayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearYearDayScheduleWithParams:(MTRDoorLockClusterClearYearDayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use clearYearDayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setHolidayScheduleWithParams:(MTRDoorLockClusterSetHolidayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use setHolidayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getHolidayScheduleWithParams:(MTRDoorLockClusterGetHolidayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRDoorLockClusterGetHolidayScheduleResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use getHolidayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearHolidayScheduleWithParams:(MTRDoorLockClusterClearHolidayScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use clearHolidayScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setUserWithParams:(MTRDoorLockClusterSetUserParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use setUserWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getUserWithParams:(MTRDoorLockClusterGetUserParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRDoorLockClusterGetUserResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use getUserWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearUserWithParams:(MTRDoorLockClusterClearUserParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use clearUserWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setCredentialWithParams:(MTRDoorLockClusterSetCredentialParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRDoorLockClusterSetCredentialResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use setCredentialWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getCredentialStatusWithParams:(MTRDoorLockClusterGetCredentialStatusParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRDoorLockClusterGetCredentialStatusResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use getCredentialStatusWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearCredentialWithParams:(MTRDoorLockClusterClearCredentialParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use clearCredentialWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterWindowCovering (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)upOrOpenWithParams:(MTRWindowCoveringClusterUpOrOpenParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use upOrOpenWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)upOrOpenWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use upOrOpenWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)downOrCloseWithParams:(MTRWindowCoveringClusterDownOrCloseParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use downOrCloseWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)downOrCloseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use downOrCloseWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopMotionWithParams:(MTRWindowCoveringClusterStopMotionParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use stopMotionWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopMotionWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use stopMotionWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)goToLiftValueWithParams:(MTRWindowCoveringClusterGoToLiftValueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use goToLiftValueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)goToLiftPercentageWithParams:(MTRWindowCoveringClusterGoToLiftPercentageParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use goToLiftPercentageWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)goToTiltValueWithParams:(MTRWindowCoveringClusterGoToTiltValueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use goToTiltValueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)goToTiltPercentageWithParams:(MTRWindowCoveringClusterGoToTiltPercentageParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use goToTiltPercentageWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterPumpConfigurationAndControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterThermostat (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)setpointRaiseLowerWithParams:(MTRThermostatClusterSetpointRaiseLowerParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use setpointRaiseLowerWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)setWeeklyScheduleWithParams:(MTRThermostatClusterSetWeeklyScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use setWeeklyScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getWeeklyScheduleWithParams:(MTRThermostatClusterGetWeeklyScheduleParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRThermostatClusterGetWeeklyScheduleResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use getWeeklyScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearWeeklyScheduleWithParams:(MTRThermostatClusterClearWeeklyScheduleParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use clearWeeklyScheduleWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)clearWeeklyScheduleWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use clearWeeklyScheduleWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterFanControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterThermostatUserInterfaceConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterColorControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)moveToHueWithParams:(MTRColorControlClusterMoveToHueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveToHueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveHueWithParams:(MTRColorControlClusterMoveHueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveHueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stepHueWithParams:(MTRColorControlClusterStepHueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use stepHueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveToSaturationWithParams:(MTRColorControlClusterMoveToSaturationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveToSaturationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveSaturationWithParams:(MTRColorControlClusterMoveSaturationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveSaturationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stepSaturationWithParams:(MTRColorControlClusterStepSaturationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use stepSaturationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveToHueAndSaturationWithParams:(MTRColorControlClusterMoveToHueAndSaturationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveToHueAndSaturationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveToColorWithParams:(MTRColorControlClusterMoveToColorParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveToColorWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveColorWithParams:(MTRColorControlClusterMoveColorParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveColorWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stepColorWithParams:(MTRColorControlClusterStepColorParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use stepColorWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveToColorTemperatureWithParams:(MTRColorControlClusterMoveToColorTemperatureParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveToColorTemperatureWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enhancedMoveToHueWithParams:(MTRColorControlClusterEnhancedMoveToHueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use enhancedMoveToHueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enhancedMoveHueWithParams:(MTRColorControlClusterEnhancedMoveHueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use enhancedMoveHueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enhancedStepHueWithParams:(MTRColorControlClusterEnhancedStepHueParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use enhancedStepHueWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)enhancedMoveToHueAndSaturationWithParams:(MTRColorControlClusterEnhancedMoveToHueAndSaturationParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use enhancedMoveToHueAndSaturationWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)colorLoopSetWithParams:(MTRColorControlClusterColorLoopSetParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use colorLoopSetWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopMoveStepWithParams:(MTRColorControlClusterStopMoveStepParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use stopMoveStepWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)moveColorTemperatureWithParams:(MTRColorControlClusterMoveColorTemperatureParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use moveColorTemperatureWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stepColorTemperatureWithParams:(MTRColorControlClusterStepColorTemperatureParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use stepColorTemperatureWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterBallastConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (NSDictionary<NSString *, id> *)readAttributeIntrinsicBalanceFactorWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("Please use readAttributeIntrinsicBallastFactorWithParams on MTRClusterBallastConfiguration", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeIntrinsicBalanceFactorWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("Please use writeAttributeIntrinsicBallastFactorWithValue on MTRClusterBallastConfiguration", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeIntrinsicBalanceFactorWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("Please use writeAttributeIntrinsicBallastFactorWithValue on MTRClusterBallastConfiguration", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterIlluminanceMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterTemperatureMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterPressureMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterFlowMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterRelativeHumidityMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterOccupancySensing (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (NSDictionary<NSString *, id> *)readAttributePirOccupiedToUnoccupiedDelayWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("Please use readAttributePIROccupiedToUnoccupiedDelayWithParams on MTRClusterOccupancySensing", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePirOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("Please use writeAttributePIROccupiedToUnoccupiedDelayWithValue on MTRClusterOccupancySensing", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePirOccupiedToUnoccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("Please use writeAttributePIROccupiedToUnoccupiedDelayWithValue on MTRClusterOccupancySensing", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributePirUnoccupiedToOccupiedDelayWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("Please use readAttributePIRUnoccupiedToOccupiedDelayWithParams on MTRClusterOccupancySensing", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePirUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("Please use writeAttributePIRUnoccupiedToOccupiedDelayWithValue on MTRClusterOccupancySensing", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePirUnoccupiedToOccupiedDelayWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("Please use writeAttributePIRUnoccupiedToOccupiedDelayWithValue on MTRClusterOccupancySensing", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (NSDictionary<NSString *, id> *)readAttributePirUnoccupiedToOccupiedThresholdWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("Please use readAttributePIRUnoccupiedToOccupiedThresholdWithParams on MTRClusterOccupancySensing", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePirUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("Please use writeAttributePIRUnoccupiedToOccupiedThresholdWithValue on MTRClusterOccupancySensing", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePirUnoccupiedToOccupiedThresholdWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("Please use writeAttributePIRUnoccupiedToOccupiedThresholdWithValue on MTRClusterOccupancySensing", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterWakeOnLan (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterChannel (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)changeChannelWithParams:(MTRChannelClusterChangeChannelParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRChannelClusterChangeChannelResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use changeChannelWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)changeChannelByNumberWithParams:(MTRChannelClusterChangeChannelByNumberParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use changeChannelByNumberWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)skipChannelWithParams:(MTRChannelClusterSkipChannelParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use skipChannelWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterTargetNavigator (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)navigateTargetWithParams:(MTRTargetNavigatorClusterNavigateTargetParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTargetNavigatorClusterNavigateTargetResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use navigateTargetWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterMediaPlayback (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)playWithParams:(MTRMediaPlaybackClusterPlayParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use playWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)playWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use playWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)pauseWithParams:(MTRMediaPlaybackClusterPauseParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use pauseWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)pauseWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use pauseWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopPlaybackWithParams:(MTRMediaPlaybackClusterStopPlaybackParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use stopWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopPlaybackWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use stopWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)startOverWithParams:(MTRMediaPlaybackClusterStartOverParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use startOverWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)startOverWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use startOverWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)previousWithParams:(MTRMediaPlaybackClusterPreviousParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use previousWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)previousWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use previousWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)nextWithParams:(MTRMediaPlaybackClusterNextParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use nextWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)nextWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use nextWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)rewindWithParams:(MTRMediaPlaybackClusterRewindParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use rewindWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)rewindWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use rewindWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)fastForwardWithParams:(MTRMediaPlaybackClusterFastForwardParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use fastForwardWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)fastForwardWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use fastForwardWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)skipForwardWithParams:(MTRMediaPlaybackClusterSkipForwardParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use skipForwardWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)skipBackwardWithParams:(MTRMediaPlaybackClusterSkipBackwardParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use skipBackwardWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)seekWithParams:(MTRMediaPlaybackClusterSeekParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRMediaPlaybackClusterPlaybackResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use seekWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterMediaInput (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)selectInputWithParams:(MTRMediaInputClusterSelectInputParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use selectInputWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)showInputStatusWithParams:(MTRMediaInputClusterShowInputStatusParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use showInputStatusWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)showInputStatusWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use showInputStatusWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)hideInputStatusWithParams:(MTRMediaInputClusterHideInputStatusParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use hideInputStatusWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)hideInputStatusWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use hideInputStatusWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)renameInputWithParams:(MTRMediaInputClusterRenameInputParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use renameInputWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterLowPower (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)sleepWithParams:(MTRLowPowerClusterSleepParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use sleepWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)sleepWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use sleepWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterKeypadInput (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)sendKeyWithParams:(MTRKeypadInputClusterSendKeyParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRKeypadInputClusterSendKeyResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use sendKeyWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterContentLauncher (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)launchContentWithParams:(MTRContentLauncherClusterLaunchContentParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRContentLauncherClusterLaunchResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use launchContentWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)launchURLWithParams:(MTRContentLauncherClusterLaunchURLParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRContentLauncherClusterLaunchResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use launchURLWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterAudioOutput (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)selectOutputWithParams:(MTRAudioOutputClusterSelectOutputParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use selectOutputWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)renameOutputWithParams:(MTRAudioOutputClusterRenameOutputParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use renameOutputWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterApplicationLauncher (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)launchAppWithParams:(MTRApplicationLauncherClusterLaunchAppParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use launchAppWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)stopAppWithParams:(MTRApplicationLauncherClusterStopAppParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use stopAppWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)hideAppWithParams:(MTRApplicationLauncherClusterHideAppParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRApplicationLauncherClusterLauncherResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use hideAppWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterApplicationBasic (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRClusterAccountLogin (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)getSetupPINWithParams:(MTRAccountLoginClusterGetSetupPINParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRAccountLoginClusterGetSetupPINResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use getSetupPINWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)loginWithParams:(MTRAccountLoginClusterLoginParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use loginWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)logoutWithParams:(MTRAccountLoginClusterLogoutParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use logoutWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)logoutWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use logoutWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

@interface MTRClusterTestCluster (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)testWithParams:(MTRTestClusterClusterTestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use testWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use testWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testNotHandledWithParams:(MTRTestClusterClusterTestNotHandledParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use testNotHandledWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testNotHandledWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use testNotHandledWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testSpecificWithParams:(MTRTestClusterClusterTestSpecificParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterTestSpecificResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testSpecificWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testSpecificWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterTestSpecificResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testSpecificWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testUnknownCommandWithParams:(MTRTestClusterClusterTestUnknownCommandParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use testUnknownCommandWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testUnknownCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use testUnknownCommandWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testAddArgumentsWithParams:(MTRTestClusterClusterTestAddArgumentsParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterTestAddArgumentsResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testAddArgumentsWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testSimpleArgumentRequestWithParams:(MTRTestClusterClusterTestSimpleArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterTestSimpleArgumentResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testSimpleArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testStructArrayArgumentRequestWithParams:(MTRTestClusterClusterTestStructArrayArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterTestStructArrayArgumentResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testStructArrayArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testStructArgumentRequestWithParams:(MTRTestClusterClusterTestStructArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testStructArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testNestedStructArgumentRequestWithParams:(MTRTestClusterClusterTestNestedStructArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testNestedStructArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testListStructArgumentRequestWithParams:(MTRTestClusterClusterTestListStructArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testListStructArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testListInt8UArgumentRequestWithParams:(MTRTestClusterClusterTestListInt8UArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testListInt8UArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testNestedStructListArgumentRequestWithParams:(MTRTestClusterClusterTestNestedStructListArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testNestedStructListArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testListNestedStructListArgumentRequestWithParams:(MTRTestClusterClusterTestListNestedStructListArgumentRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterBooleanResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testListNestedStructListArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testListInt8UReverseRequestWithParams:(MTRTestClusterClusterTestListInt8UReverseRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterTestListInt8UReverseResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testListInt8UReverseRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testEnumsRequestWithParams:(MTRTestClusterClusterTestEnumsRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterTestEnumsResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testEnumsRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testNullableOptionalRequestWithParams:(MTRTestClusterClusterTestNullableOptionalRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterTestNullableOptionalResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testNullableOptionalRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testNullableOptionalRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterTestNullableOptionalResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testNullableOptionalRequestWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testComplexNullableOptionalRequestWithParams:(MTRTestClusterClusterTestComplexNullableOptionalRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterTestComplexNullableOptionalResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testComplexNullableOptionalRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)simpleStructEchoRequestWithParams:(MTRTestClusterClusterSimpleStructEchoRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterSimpleStructResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use simpleStructEchoRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)timedInvokeRequestWithParams:(MTRTestClusterClusterTimedInvokeRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use timedInvokeRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)timedInvokeRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use timedInvokeRequestWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testSimpleOptionalArgumentRequestWithParams:(MTRTestClusterClusterTestSimpleOptionalArgumentRequestParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use testSimpleOptionalArgumentRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testSimpleOptionalArgumentRequestWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use testSimpleOptionalArgumentRequestWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testEmitTestEventRequestWithParams:(MTRTestClusterClusterTestEmitTestEventRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterTestEmitTestEventResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testEmitTestEventRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)testEmitTestFabricScopedEventRequestWithParams:(MTRTestClusterClusterTestEmitTestFabricScopedEventRequestParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(void (^)(MTRTestClusterClusterTestEmitTestFabricScopedEventResponseParams * _Nullable data, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use testEmitTestFabricScopedEventRequestWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

NS_ASSUME_NONNULL_END
