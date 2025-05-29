/**
 *    Copyright (c) 2023 Project CHIP Authors
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

#import <Matter/MTRCluster.h>
#import <Matter/MTRClusterStateCacheContainer.h>
#import <Matter/MTRCommandPayloadsObjc.h>
#import <Matter/MTRDefines.h>
#import <Matter/MTRStructsObjc.h>

/**
 * This file defines manual backwards-compat shims of various sorts to handle
 * API changes that happened.
 */

NS_ASSUME_NONNULL_BEGIN

@interface MTRGroupKeyManagementClusterKeySetReadAllIndicesParams ()
/**
 * This command used to incorrectly have a groupKeySetIDs field.
 */
@property (nonatomic, copy) NSArray * groupKeySetIDs MTR_DEPRECATED(
    "This field has been removed", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));

@end

/**
 * FanControl used to have WindSettingMask and WindSupportMask that had
 * identical values.  Those got replaced with a single WindBitmap.  We codegen
 * WindSupportMask as an alias of WindBitmap, but we need a manual shim for
 * WindSettingMask.
 */
typedef NS_OPTIONS(uint8_t, MTRFanControlWindSettingMask) {
    MTRFanControlWindSettingMaskSleepWind MTR_DEPRECATED(
        "Please use MTRFanControlWindBitmapSleepWind", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
    = 0x1,
    MTRFanControlWindSettingMaskNaturalWind MTR_DEPRECATED(
        "Please use MTRFanControlWindBitmapNaturalWind", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
    = 0x2,
} MTR_DEPRECATED("Please use MTRFanControlWindBitmap", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));

/**
 * For the OTA clusters, first we changed the names of the clusters, and then we
 * changed the names of the enums.  That means we now have three names for the
 * enums (newest name, name before enum name change, name before cluster name
 * change).  We can only track one old name for things, so just manually add the
 * intermediate names.
 */
typedef NS_ENUM(uint8_t, MTROTASoftwareUpdateProviderOTAApplyUpdateAction) {
    MTROTASoftwareUpdateProviderOTAApplyUpdateActionProceed MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderApplyUpdateActionProceed", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x00,
    MTROTASoftwareUpdateProviderOTAApplyUpdateActionAwaitNextAction
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderApplyUpdateActionAwaitNextAction", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x01,
    MTROTASoftwareUpdateProviderOTAApplyUpdateActionDiscontinue
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderApplyUpdateActionDiscontinue", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x02,
} MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderApplyUpdateAction", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2));

typedef NS_ENUM(uint8_t, MTROTASoftwareUpdateProviderOTADownloadProtocol) {
    MTROTASoftwareUpdateProviderOTADownloadProtocolBDXSynchronous
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderDownloadProtocolBDXSynchronous", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x00,
    MTROTASoftwareUpdateProviderOTADownloadProtocolBDXAsynchronous
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderDownloadProtocolBDXAsynchronous", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x01,
    MTROTASoftwareUpdateProviderOTADownloadProtocolHTTPS
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderDownloadProtocolHTTPS", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x02,
    MTROTASoftwareUpdateProviderOTADownloadProtocolVendorSpecific
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderDownloadProtocolVendorSpecific", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x03,
} MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderDownloadProtocol", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2));

typedef NS_ENUM(uint8_t, MTROTASoftwareUpdateProviderOTAQueryStatus) {
    MTROTASoftwareUpdateProviderOTAQueryStatusUpdateAvailable
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderStatusUpdateAvailable", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x00,
    MTROTASoftwareUpdateProviderOTAQueryStatusBusy
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderStatusBusy", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x01,
    MTROTASoftwareUpdateProviderOTAQueryStatusNotAvailable
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderStatusNotAvailable", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x02,
    MTROTASoftwareUpdateProviderOTAQueryStatusDownloadProtocolNotSupported
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderStatusDownloadProtocolNotSupported", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x03,
} MTR_DEPRECATED("Please use MTROTASoftwareUpdateProviderStatus", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2));

typedef NS_ENUM(uint8_t, MTROTASoftwareUpdateRequestorOTAAnnouncementReason) {
    MTROTASoftwareUpdateRequestorOTAAnnouncementReasonSimpleAnnouncement
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorAnnouncementReasonSimpleAnnouncement", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x00,
    MTROTASoftwareUpdateRequestorOTAAnnouncementReasonUpdateAvailable
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorAnnouncementReasonUpdateAvailable", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x01,
    MTROTASoftwareUpdateRequestorOTAAnnouncementReasonUrgentUpdateAvailable
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorAnnouncementReasonUrgentUpdateAvailable", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x02,
} MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorAnnouncementReason", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2));

typedef NS_ENUM(uint8_t, MTROTASoftwareUpdateRequestorOTAChangeReason) {
    MTROTASoftwareUpdateRequestorOTAChangeReasonUnknown
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorChangeReasonUnknown", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x00,
    MTROTASoftwareUpdateRequestorOTAChangeReasonSuccess
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorChangeReasonSuccess", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x01,
    MTROTASoftwareUpdateRequestorOTAChangeReasonFailure
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorChangeReasonFailure", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x02,
    MTROTASoftwareUpdateRequestorOTAChangeReasonTimeOut
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorChangeReasonTimeOut", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x03,
    MTROTASoftwareUpdateRequestorOTAChangeReasonDelayByProvider
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorChangeReasonDelayByProvider", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x04,
} MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorChangeReason", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2));

typedef NS_ENUM(uint8_t, MTROTASoftwareUpdateRequestorOTAUpdateState) {
    MTROTASoftwareUpdateRequestorOTAUpdateStateUnknown
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateUnknown", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x00,
    MTROTASoftwareUpdateRequestorOTAUpdateStateIdle
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateIdle", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x01,
    MTROTASoftwareUpdateRequestorOTAUpdateStateQuerying
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateQuerying", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x02,
    MTROTASoftwareUpdateRequestorOTAUpdateStateDelayedOnQuery
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateDelayedOnQuery", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x03,
    MTROTASoftwareUpdateRequestorOTAUpdateStateDownloading
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateDownloading", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x04,
    MTROTASoftwareUpdateRequestorOTAUpdateStateApplying
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateApplying", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x05,
    MTROTASoftwareUpdateRequestorOTAUpdateStateDelayedOnApply
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateDelayedOnApply", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x06,
    MTROTASoftwareUpdateRequestorOTAUpdateStateRollingBack
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateRollingBack", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x07,
    MTROTASoftwareUpdateRequestorOTAUpdateStateDelayedOnUserConsent
        MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateDelayedOnUserConsent", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2))
    = 0x08,
} MTR_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateState", ios(16.4, 17.2), macos(13.3, 14.2), watchos(9.4, 10.2), tvos(16.4, 17.2));

/**
 * ColorControl used to have HueMoveMode/SaturationMoveMode and HueStepMode/SaturationStepMode that had
 * identical values.  Those got replaced with MoveModeEnum and StepModeEnum respectively.  We codegen
 * HueMoveMode and HueStepMode as aliases of MoveModeEnum and StepModeEnum, but we need manual shims for
 * SaturationMoveMode and SaturationStepMode.
 */
typedef NS_ENUM(uint8_t, MTRColorControlSaturationMoveMode) {
    MTRColorControlSaturationMoveModeStop MTR_DEPRECATED("Please use MTRColorControlMoveModeStop", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
    = 0x00,
    MTRColorControlSaturationMoveModeUp MTR_DEPRECATED("Please use MTRColorControlMoveModeUp", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
    = 0x01,
    MTRColorControlSaturationMoveModeDown MTR_DEPRECATED("Please use MTRColorControlMoveModeDown", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
    = 0x03,
} MTR_DEPRECATED("Please use MTRColorControlMoveMode", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

typedef NS_ENUM(uint8_t, MTRColorControlSaturationStepMode) {
    MTRColorControlSaturationStepModeUp MTR_DEPRECATED("Please use MTRColorControlStepModeUp", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
    = 0x01,
    MTRColorControlSaturationStepModeDown MTR_DEPRECATED("Please use MTRColorControlStepModeDown", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
    = 0x03,
} MTR_DEPRECATED("Please use MTRColorControlStepMode", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

#pragma mark - Clusters that were removed wholesale: OnOffSwitchConfiguration

/**
 * Cluster On/off Switch Configuration
 *
 * Attributes and commands for configuring On/Off switching devices.
 */
MTR_DEPRECATED("OnOffSwitchConfiguration is deprecated and will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRBaseClusterOnOffSwitchConfiguration : MTRGenericBaseCluster

- (void)readAttributeSwitchTypeWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeSwitchTypeWithParams:(MTRSubscribeParams *)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeSwitchTypeWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeSwitchActionsWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeSwitchActionsWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeSwitchActionsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeSwitchActionsWithParams:(MTRSubscribeParams *)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeSwitchActionsWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeGeneratedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeGeneratedCommandListWithParams:(MTRSubscribeParams *)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeGeneratedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcceptedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcceptedCommandListWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcceptedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAttributeListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAttributeListWithParams:(MTRSubscribeParams *)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAttributeListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeFeatureMapWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeFeatureMapWithParams:(MTRSubscribeParams *)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeFeatureMapWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeClusterRevisionWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeClusterRevisionWithParams:(MTRSubscribeParams *)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeClusterRevisionWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRBaseClusterOnOffSwitchConfiguration (Availability)

/**
 * For all instance methods (reads, writes, commands) that take a completion,
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRBaseDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_DEPRECATED("OnOffSwitchConfiguration is deprecated and will be removed", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

@end

@interface MTRBaseClusterOnOffSwitchConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRBaseDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpointID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeSwitchTypeWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeSwitchTypeWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeSwitchTypeWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeSwitchTypeWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeSwitchTypeWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeSwitchTypeWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeSwitchActionsWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeSwitchActionsWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeSwitchActionsWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeSwitchActionsWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeSwitchActionsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeSwitchActionsWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeSwitchActionsWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeSwitchActionsWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeSwitchActionsWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeSwitchActionsWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeGeneratedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeGeneratedCommandListWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeGeneratedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeGeneratedCommandListWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeGeneratedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeGeneratedCommandListWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcceptedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcceptedCommandListWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcceptedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcceptedCommandListWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcceptedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcceptedCommandListWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAttributeListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAttributeListWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAttributeListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAttributeListWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAttributeListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAttributeListWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeFeatureMapWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeFeatureMapWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeFeatureMapWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeFeatureMapWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeFeatureMapWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeFeatureMapWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeClusterRevisionWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeClusterRevisionWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeClusterRevisionWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeClusterRevisionWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeClusterRevisionWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeClusterRevisionWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

/**
 * Cluster On/off Switch Configuration
 *    Attributes and commands for configuring On/Off switching devices.
 */
MTR_DEPRECATED("OnOffSwitchConfiguration is deprecated and will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRClusterOnOffSwitchConfiguration : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSwitchTypeWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSwitchActionsWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeSwitchActionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeSwitchActionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterOnOffSwitchConfiguration (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_DEPRECATED("OnOffSwitchConfiguration is deprecated and will be removed", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

@end

@interface MTRClusterOnOffSwitchConfiguration (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

#pragma mark - Clusters that were removed wholesale: BinaryInputBasic

/**
 * Cluster Binary Input (Basic)
 *
 * An interface for reading the value of a binary measurement and accessing various characteristics of that measurement.
 */
MTR_DEPRECATED("BinaryInputBasic is deprecated and will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRBaseClusterBinaryInputBasic : MTRGenericBaseCluster

- (void)readAttributeActiveTextWithCompletion:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeActiveTextWithValue:(NSString * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeActiveTextWithValue:(NSString * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeActiveTextWithParams:(MTRSubscribeParams *)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeActiveTextWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDescriptionWithCompletion:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeDescriptionWithValue:(NSString * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeDescriptionWithValue:(NSString * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDescriptionWithParams:(MTRSubscribeParams *)params
                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                  reportHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDescriptionWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeInactiveTextWithCompletion:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeInactiveTextWithValue:(NSString * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeInactiveTextWithValue:(NSString * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeInactiveTextWithParams:(MTRSubscribeParams *)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeInactiveTextWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeOutOfServiceWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeOutOfServiceWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeOutOfServiceWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeOutOfServiceWithParams:(MTRSubscribeParams *)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeOutOfServiceWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributePolarityWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributePolarityWithParams:(MTRSubscribeParams *)params
                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributePolarityWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributePresentValueWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributePresentValueWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributePresentValueWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributePresentValueWithParams:(MTRSubscribeParams *)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributePresentValueWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeReliabilityWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeReliabilityWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeReliabilityWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeReliabilityWithParams:(MTRSubscribeParams *)params
                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeReliabilityWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeStatusFlagsWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeStatusFlagsWithParams:(MTRSubscribeParams *)params
                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeStatusFlagsWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeApplicationTypeWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeApplicationTypeWithParams:(MTRSubscribeParams *)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeApplicationTypeWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeGeneratedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeGeneratedCommandListWithParams:(MTRSubscribeParams *)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeGeneratedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcceptedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcceptedCommandListWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcceptedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAttributeListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAttributeListWithParams:(MTRSubscribeParams *)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAttributeListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeFeatureMapWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeFeatureMapWithParams:(MTRSubscribeParams *)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeFeatureMapWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeClusterRevisionWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeClusterRevisionWithParams:(MTRSubscribeParams *)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeClusterRevisionWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRBaseClusterBinaryInputBasic (Availability)

/**
 * For all instance methods (reads, writes, commands) that take a completion,
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRBaseDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_DEPRECATED("BinaryInputBasic is deprecated and will be removed", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

@end

@interface MTRBaseClusterBinaryInputBasic (Deprecated)

- (nullable instancetype)initWithDevice:(MTRBaseDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpointID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeActiveTextWithCompletionHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActiveTextWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeActiveTextWithValue:(NSString * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeActiveTextWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeActiveTextWithValue:(NSString * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeActiveTextWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeActiveTextWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeActiveTextWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeActiveTextWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActiveTextWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDescriptionWithCompletionHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDescriptionWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeDescriptionWithValue:(NSString * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeDescriptionWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeDescriptionWithValue:(NSString * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeDescriptionWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDescriptionWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                              params:(MTRSubscribeParams * _Nullable)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                       reportHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDescriptionWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDescriptionWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDescriptionWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeInactiveTextWithCompletionHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeInactiveTextWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeInactiveTextWithValue:(NSString * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeInactiveTextWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeInactiveTextWithValue:(NSString * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeInactiveTextWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeInactiveTextWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeInactiveTextWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeInactiveTextWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeInactiveTextWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeOutOfServiceWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeOutOfServiceWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeOutOfServiceWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeOutOfServiceWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeOutOfServiceWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeOutOfServiceWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeOutOfServiceWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeOutOfServiceWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeOutOfServiceWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeOutOfServiceWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributePolarityWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePolarityWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributePolarityWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                           params:(MTRSubscribeParams * _Nullable)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributePolarityWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributePolarityWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePolarityWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributePresentValueWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePresentValueWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePresentValueWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributePresentValueWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributePresentValueWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributePresentValueWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributePresentValueWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributePresentValueWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributePresentValueWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePresentValueWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeReliabilityWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeReliabilityWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeReliabilityWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeReliabilityWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeReliabilityWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeReliabilityWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeReliabilityWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                              params:(MTRSubscribeParams * _Nullable)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeReliabilityWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeReliabilityWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeReliabilityWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeStatusFlagsWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeStatusFlagsWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeStatusFlagsWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                              params:(MTRSubscribeParams * _Nullable)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeStatusFlagsWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeStatusFlagsWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeStatusFlagsWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeApplicationTypeWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeApplicationTypeWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeApplicationTypeWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeApplicationTypeWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeApplicationTypeWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeApplicationTypeWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeGeneratedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeGeneratedCommandListWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeGeneratedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeGeneratedCommandListWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeGeneratedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeGeneratedCommandListWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcceptedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcceptedCommandListWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcceptedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcceptedCommandListWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcceptedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcceptedCommandListWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAttributeListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAttributeListWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAttributeListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAttributeListWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAttributeListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAttributeListWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeFeatureMapWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeFeatureMapWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeFeatureMapWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeFeatureMapWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeFeatureMapWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeFeatureMapWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeClusterRevisionWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeClusterRevisionWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeClusterRevisionWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeClusterRevisionWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeClusterRevisionWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeClusterRevisionWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

/**
 * Cluster Binary Input (Basic)
 *    An interface for reading the value of a binary measurement and accessing various characteristics of that measurement.
 */
MTR_DEPRECATED("BinaryInputBasic is deprecated and will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRClusterBinaryInputBasic : MTRGenericCluster

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveTextWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeActiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeActiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDescriptionWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeDescriptionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeDescriptionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInactiveTextWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeInactiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeInactiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOutOfServiceWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeOutOfServiceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeOutOfServiceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePolarityWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePresentValueWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributePresentValueWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributePresentValueWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReliabilityWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeReliabilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeReliabilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStatusFlagsWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApplicationTypeWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterBinaryInputBasic (Availability)

/**
 * The queue is currently unused, but may be used in the future for calling completions
 * for command invocations if commands are added to this cluster.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_DEPRECATED("BinaryInputBasic is deprecated and will be removed", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

@end

@interface MTRClusterBinaryInputBasic (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

#pragma mark - Clusters that were removed wholesale: BarrierControl

MTR_DEPRECATED("This command has been removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRBarrierControlClusterBarrierControlGoToPercentParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull percentOpen MTR_DEPRECATED("The percentOpen field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("This command has been removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRBarrierControlClusterBarrierControlStopParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

/**
 * Cluster Barrier Control
 *
 * This cluster provides control of a barrier (garage door).
 */
MTR_DEPRECATED("BarrierControl is deprecated and will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRBaseClusterBarrierControl : MTRGenericBaseCluster

/**
 * Command BarrierControlGoToPercent
 *
 * Command to instruct a barrier to go to a percent open state.
 */
- (void)barrierControlGoToPercentWithParams:(MTRBarrierControlClusterBarrierControlGoToPercentParams *)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This command is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
/**
 * Command BarrierControlStop
 *
 * Command that instructs the barrier to stop moving.
 */
- (void)barrierControlStopWithParams:(MTRBarrierControlClusterBarrierControlStopParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This command is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)barrierControlStopWithCompletion:(MTRStatusCompletion)completion
    MTR_DEPRECATED("This command is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeBarrierMovingStateWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeBarrierMovingStateWithParams:(MTRSubscribeParams *)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeBarrierMovingStateWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeBarrierSafetyStatusWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeBarrierSafetyStatusWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeBarrierSafetyStatusWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeBarrierCapabilitiesWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeBarrierCapabilitiesWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeBarrierCapabilitiesWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeBarrierOpenEventsWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeBarrierOpenEventsWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeBarrierOpenEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeBarrierOpenEventsWithParams:(MTRSubscribeParams *)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeBarrierOpenEventsWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeBarrierCloseEventsWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeBarrierCloseEventsWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeBarrierCloseEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeBarrierCloseEventsWithParams:(MTRSubscribeParams *)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeBarrierCloseEventsWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeBarrierCommandOpenEventsWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeBarrierCommandOpenEventsWithParams:(MTRSubscribeParams *)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeBarrierCommandOpenEventsWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeBarrierCommandCloseEventsWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeBarrierCommandCloseEventsWithParams:(MTRSubscribeParams *)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeBarrierCommandCloseEventsWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeBarrierOpenPeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeBarrierOpenPeriodWithParams:(MTRSubscribeParams *)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeBarrierOpenPeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeBarrierClosePeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeBarrierClosePeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeBarrierClosePeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeBarrierClosePeriodWithParams:(MTRSubscribeParams *)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeBarrierClosePeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeBarrierPositionWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeBarrierPositionWithParams:(MTRSubscribeParams *)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeBarrierPositionWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeGeneratedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeGeneratedCommandListWithParams:(MTRSubscribeParams *)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeGeneratedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcceptedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcceptedCommandListWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcceptedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAttributeListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAttributeListWithParams:(MTRSubscribeParams *)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAttributeListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeFeatureMapWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeFeatureMapWithParams:(MTRSubscribeParams *)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeFeatureMapWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeClusterRevisionWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeClusterRevisionWithParams:(MTRSubscribeParams *)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeClusterRevisionWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRBaseClusterBarrierControl (Availability)

/**
 * For all instance methods (reads, writes, commands) that take a completion,
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRBaseDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_DEPRECATED("BarrierControl is deprecated and will be removed", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

@end

@interface MTRBaseClusterBarrierControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRBaseDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpointID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)barrierControlGoToPercentWithParams:(MTRBarrierControlClusterBarrierControlGoToPercentParams *)params completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use barrierControlGoToPercentWithParams:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)barrierControlStopWithParams:(MTRBarrierControlClusterBarrierControlStopParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use barrierControlStopWithParams:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)barrierControlStopWithCompletionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use barrierControlStopWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeBarrierMovingStateWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierMovingStateWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeBarrierMovingStateWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeBarrierMovingStateWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeBarrierMovingStateWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierMovingStateWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeBarrierSafetyStatusWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierSafetyStatusWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeBarrierSafetyStatusWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeBarrierSafetyStatusWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeBarrierSafetyStatusWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierSafetyStatusWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeBarrierCapabilitiesWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierCapabilitiesWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeBarrierCapabilitiesWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeBarrierCapabilitiesWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeBarrierCapabilitiesWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierCapabilitiesWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeBarrierOpenEventsWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierOpenEventsWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeBarrierOpenEventsWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeBarrierOpenEventsWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeBarrierOpenEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeBarrierOpenEventsWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeBarrierOpenEventsWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeBarrierOpenEventsWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeBarrierOpenEventsWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierOpenEventsWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeBarrierCloseEventsWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierCloseEventsWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeBarrierCloseEventsWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeBarrierCloseEventsWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeBarrierCloseEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeBarrierCloseEventsWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeBarrierCloseEventsWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeBarrierCloseEventsWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeBarrierCloseEventsWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierCloseEventsWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeBarrierCommandOpenEventsWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierCommandOpenEventsWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeBarrierCommandOpenEventsWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeBarrierCommandOpenEventsWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeBarrierCommandOpenEventsWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                           params:(MTRSubscribeParams * _Nullable)params
                                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeBarrierCommandOpenEventsWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeBarrierCommandOpenEventsWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierCommandOpenEventsWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeBarrierCommandCloseEventsWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierCommandCloseEventsWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeBarrierCommandCloseEventsWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeBarrierCommandCloseEventsWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeBarrierCommandCloseEventsWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                            params:(MTRSubscribeParams * _Nullable)params
                                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeBarrierCommandCloseEventsWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeBarrierCommandCloseEventsWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierCommandCloseEventsWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeBarrierOpenPeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierOpenPeriodWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeBarrierOpenPeriodWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeBarrierOpenPeriodWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeBarrierOpenPeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeBarrierOpenPeriodWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeBarrierOpenPeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierOpenPeriodWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeBarrierClosePeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierClosePeriodWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeBarrierClosePeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeBarrierClosePeriodWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeBarrierClosePeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeBarrierClosePeriodWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeBarrierClosePeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeBarrierClosePeriodWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeBarrierClosePeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierClosePeriodWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeBarrierPositionWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierPositionWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeBarrierPositionWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeBarrierPositionWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeBarrierPositionWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeBarrierPositionWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeGeneratedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeGeneratedCommandListWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeGeneratedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeGeneratedCommandListWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeGeneratedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeGeneratedCommandListWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcceptedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcceptedCommandListWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcceptedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcceptedCommandListWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcceptedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcceptedCommandListWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAttributeListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAttributeListWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAttributeListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAttributeListWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAttributeListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAttributeListWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeFeatureMapWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeFeatureMapWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeFeatureMapWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeFeatureMapWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeFeatureMapWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeFeatureMapWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeClusterRevisionWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeClusterRevisionWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeClusterRevisionWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeClusterRevisionWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeClusterRevisionWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeClusterRevisionWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

/**
 * Cluster Barrier Control
 *    This cluster provides control of a barrier (garage door).
 */
MTR_DEPRECATED("BarrierControl is deprecated and will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRClusterBarrierControl : MTRGenericCluster

- (void)barrierControlGoToPercentWithParams:(MTRBarrierControlClusterBarrierControlGoToPercentParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This command is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)barrierControlStopWithParams:(MTRBarrierControlClusterBarrierControlStopParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This command is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)barrierControlStopWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_DEPRECATED("This command is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierMovingStateWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierSafetyStatusWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierCapabilitiesWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierOpenEventsWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeBarrierOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeBarrierOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierCloseEventsWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeBarrierCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeBarrierCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierCommandOpenEventsWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierCommandCloseEventsWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierOpenPeriodWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierClosePeriodWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeBarrierClosePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeBarrierClosePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierPositionWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterBarrierControl (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_DEPRECATED("BarrierControl is deprecated and will be removed", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

@end

@interface MTRClusterBarrierControl (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)barrierControlGoToPercentWithParams:(MTRBarrierControlClusterBarrierControlGoToPercentParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use barrierControlGoToPercentWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)barrierControlStopWithParams:(MTRBarrierControlClusterBarrierControlStopParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use barrierControlStopWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)barrierControlStopWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use barrierControlStopWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

#pragma mark - Clusters that were removed wholesale: ElectricalMeasurement

MTR_DEPRECATED("This command has been removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRElectricalMeasurementClusterGetProfileInfoResponseCommandParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull profileCount MTR_DEPRECATED("The profileCount field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

@property (nonatomic, copy) NSNumber * _Nonnull profileIntervalPeriod MTR_DEPRECATED("The profileIntervalPeriod field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

@property (nonatomic, copy) NSNumber * _Nonnull maxNumberOfIntervals MTR_DEPRECATED("The maxNumberOfIntervals field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

@property (nonatomic, copy) NSArray * _Nonnull listOfAttributes MTR_DEPRECATED("The listOfAttributes field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRElectricalMeasurementClusterGetProfileInfoResponseCommandParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_DEPRECATED("This command has been removed", ios(17.0, 18.2), macos(14.0, 15.2), watchos(10.0, 11.2), tvos(17.0, 18.2));
@end

MTR_DEPRECATED("This command has been removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRElectricalMeasurementClusterGetProfileInfoCommandParams : NSObject <NSCopying>
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

MTR_DEPRECATED("This command has been removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRElectricalMeasurementClusterGetMeasurementProfileResponseCommandParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull startTime MTR_DEPRECATED("The startTime field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

@property (nonatomic, copy) NSNumber * _Nonnull status MTR_DEPRECATED("The status field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

@property (nonatomic, copy) NSNumber * _Nonnull profileIntervalPeriod MTR_DEPRECATED("The profileIntervalPeriod field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

@property (nonatomic, copy) NSNumber * _Nonnull numberOfIntervalsDelivered MTR_DEPRECATED("The numberOfIntervalsDelivered field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

@property (nonatomic, copy) NSNumber * _Nonnull attributeId MTR_DEPRECATED("The attributeId field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

@property (nonatomic, copy) NSArray * _Nonnull intervals MTR_DEPRECATED("The intervals field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs MTR_DEPRECATED("Timed invoke does not make sense for server to client commands", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Initialize an MTRElectricalMeasurementClusterGetMeasurementProfileResponseCommandParams with a response-value dictionary
 * of the sort that MTRDeviceResponseHandler would receive.
 *
 * Will return nil and hand out an error if the response-value dictionary is not
 * a command data response or is not the right command response.
 *
 * Will return nil and hand out an error if the data response does not match the known
 * schema for this command.
 */
- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error MTR_DEPRECATED("This command has been removed", ios(17.0, 18.2), macos(14.0, 15.2), watchos(10.0, 11.2), tvos(17.0, 18.2));
@end

MTR_DEPRECATED("This command has been removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams : NSObject <NSCopying>

@property (nonatomic, copy) NSNumber * _Nonnull attributeId MTR_DEPRECATED("The attributeId field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

@property (nonatomic, copy) NSNumber * _Nonnull startTime MTR_DEPRECATED("The startTime field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

@property (nonatomic, copy) NSNumber * _Nonnull numberOfIntervals MTR_DEPRECATED("The numberOfIntervals field will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
/**
 * Controls whether the command is a timed command (using Timed Invoke).
 *
 * If nil (the default value), a regular invoke is done for commands that do
 * not require a timed invoke and a timed invoke with some default timed request
 * timeout is done for commands that require a timed invoke.
 *
 * If not nil, a timed invoke is done, with the provided value used as the timed
 * request timeout.  The value should be chosen small enough to provide the
 * desired security properties but large enough that it will allow a round-trip
 * from the sever to the client (for the status response and actual invoke
 * request) within the timeout window.
 *
 */
@property (nonatomic, copy, nullable) NSNumber * timedInvokeTimeoutMs;

/**
 * Controls how much time, in seconds, we will allow for the server to process the command.
 *
 * The command will then time out if that much time, plus an allowance for retransmits due to network failures, passes.
 *
 * If nil, the framework will try to select an appropriate timeout value itself.
 */
@property (nonatomic, copy, nullable) NSNumber * serverSideProcessingTimeout;
@end

/**
 * Cluster Electrical Measurement
 *
 * Attributes related to the electrical properties of a device. This cluster is used by power outlets and other devices that need to provide instantaneous data as opposed to metrology data which should be retrieved from the metering cluster..
 */
MTR_DEPRECATED("ElectricalMeasurement is deprecated and will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRBaseClusterElectricalMeasurement : MTRGenericBaseCluster

/**
 * Command GetProfileInfoCommand
 *
 * A function which retrieves the power profiling information from the electrical measurement server.
 */
- (void)getProfileInfoCommandWithParams:(MTRElectricalMeasurementClusterGetProfileInfoCommandParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This command is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)getProfileInfoCommandWithCompletion:(MTRStatusCompletion)completion
    MTR_DEPRECATED("This command is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
/**
 * Command GetMeasurementProfileCommand
 *
 * A function which retrieves an electricity measurement profile from the electricity measurement server for a specific attribute Id requested.
 */
- (void)getMeasurementProfileCommandWithParams:(MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams *)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This command is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeMeasurementTypeWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeMeasurementTypeWithParams:(MTRSubscribeParams *)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeMeasurementTypeWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcVoltageWithParams:(MTRSubscribeParams *)params
                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcVoltageMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcVoltageMinWithParams:(MTRSubscribeParams *)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcVoltageMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcVoltageMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcVoltageMaxWithParams:(MTRSubscribeParams *)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcVoltageMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcCurrentWithParams:(MTRSubscribeParams *)params
                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcCurrentMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcCurrentMinWithParams:(MTRSubscribeParams *)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcCurrentMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcCurrentMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcCurrentMaxWithParams:(MTRSubscribeParams *)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcCurrentMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcPowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcPowerWithParams:(MTRSubscribeParams *)params
                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcPowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcPowerMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcPowerMinWithParams:(MTRSubscribeParams *)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcPowerMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcPowerMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcPowerMaxWithParams:(MTRSubscribeParams *)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcPowerMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcVoltageMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcVoltageMultiplierWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcVoltageMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcVoltageDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcVoltageDivisorWithParams:(MTRSubscribeParams *)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcVoltageDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcCurrentMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcCurrentMultiplierWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcCurrentMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcCurrentDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcCurrentDivisorWithParams:(MTRSubscribeParams *)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcCurrentDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcPowerMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcPowerMultiplierWithParams:(MTRSubscribeParams *)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcPowerMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeDcPowerDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeDcPowerDivisorWithParams:(MTRSubscribeParams *)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeDcPowerDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcFrequencyWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcFrequencyWithParams:(MTRSubscribeParams *)params
                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcFrequencyWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcFrequencyMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcFrequencyMinWithParams:(MTRSubscribeParams *)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcFrequencyMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcFrequencyMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcFrequencyMaxWithParams:(MTRSubscribeParams *)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcFrequencyMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeNeutralCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeNeutralCurrentWithParams:(MTRSubscribeParams *)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeNeutralCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeTotalActivePowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeTotalActivePowerWithParams:(MTRSubscribeParams *)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeTotalActivePowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeTotalReactivePowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeTotalReactivePowerWithParams:(MTRSubscribeParams *)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeTotalReactivePowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeTotalApparentPowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeTotalApparentPowerWithParams:(MTRSubscribeParams *)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeTotalApparentPowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeMeasured1stHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeMeasured1stHarmonicCurrentWithParams:(MTRSubscribeParams *)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeMeasured1stHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeMeasured3rdHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeMeasured3rdHarmonicCurrentWithParams:(MTRSubscribeParams *)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeMeasured3rdHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeMeasured5thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeMeasured5thHarmonicCurrentWithParams:(MTRSubscribeParams *)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeMeasured5thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeMeasured7thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeMeasured7thHarmonicCurrentWithParams:(MTRSubscribeParams *)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeMeasured7thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeMeasured9thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeMeasured9thHarmonicCurrentWithParams:(MTRSubscribeParams *)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeMeasured9thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeMeasured11thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeMeasured11thHarmonicCurrentWithParams:(MTRSubscribeParams *)params
                                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeMeasured11thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeMeasuredPhase1stHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeMeasuredPhase1stHarmonicCurrentWithParams:(MTRSubscribeParams *)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeMeasuredPhase1stHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeMeasuredPhase3rdHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeMeasuredPhase3rdHarmonicCurrentWithParams:(MTRSubscribeParams *)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeMeasuredPhase3rdHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeMeasuredPhase5thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeMeasuredPhase5thHarmonicCurrentWithParams:(MTRSubscribeParams *)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeMeasuredPhase5thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeMeasuredPhase7thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeMeasuredPhase7thHarmonicCurrentWithParams:(MTRSubscribeParams *)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeMeasuredPhase7thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeMeasuredPhase9thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeMeasuredPhase9thHarmonicCurrentWithParams:(MTRSubscribeParams *)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeMeasuredPhase9thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeMeasuredPhase11thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeMeasuredPhase11thHarmonicCurrentWithParams:(MTRSubscribeParams *)params
                                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeMeasuredPhase11thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcFrequencyMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcFrequencyMultiplierWithParams:(MTRSubscribeParams *)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcFrequencyMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcFrequencyDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcFrequencyDivisorWithParams:(MTRSubscribeParams *)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcFrequencyDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributePowerMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributePowerMultiplierWithParams:(MTRSubscribeParams *)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributePowerMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributePowerDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributePowerDivisorWithParams:(MTRSubscribeParams *)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributePowerDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeHarmonicCurrentMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeHarmonicCurrentMultiplierWithParams:(MTRSubscribeParams *)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeHarmonicCurrentMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributePhaseHarmonicCurrentMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributePhaseHarmonicCurrentMultiplierWithParams:(MTRSubscribeParams *)params
                                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributePhaseHarmonicCurrentMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeInstantaneousVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeInstantaneousVoltageWithParams:(MTRSubscribeParams *)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeInstantaneousVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeInstantaneousLineCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeInstantaneousLineCurrentWithParams:(MTRSubscribeParams *)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeInstantaneousLineCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeInstantaneousActiveCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeInstantaneousActiveCurrentWithParams:(MTRSubscribeParams *)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeInstantaneousActiveCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeInstantaneousReactiveCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeInstantaneousReactiveCurrentWithParams:(MTRSubscribeParams *)params
                                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeInstantaneousReactiveCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeInstantaneousPowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeInstantaneousPowerWithParams:(MTRSubscribeParams *)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeInstantaneousPowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageWithParams:(MTRSubscribeParams *)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageMinWithParams:(MTRSubscribeParams *)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageMaxWithParams:(MTRSubscribeParams *)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsCurrentWithParams:(MTRSubscribeParams *)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsCurrentMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsCurrentMinWithParams:(MTRSubscribeParams *)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsCurrentMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsCurrentMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsCurrentMaxWithParams:(MTRSubscribeParams *)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsCurrentMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeActivePowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeActivePowerWithParams:(MTRSubscribeParams *)params
                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeActivePowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeActivePowerMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeActivePowerMinWithParams:(MTRSubscribeParams *)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeActivePowerMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeActivePowerMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeActivePowerMaxWithParams:(MTRSubscribeParams *)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeActivePowerMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeReactivePowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeReactivePowerWithParams:(MTRSubscribeParams *)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeReactivePowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeApparentPowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeApparentPowerWithParams:(MTRSubscribeParams *)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeApparentPowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributePowerFactorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributePowerFactorWithParams:(MTRSubscribeParams *)params
                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributePowerFactorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAverageRmsVoltageMeasurementPeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAverageRmsVoltageMeasurementPeriodWithParams:(MTRSubscribeParams *)params
                                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAverageRmsVoltageMeasurementPeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAverageRmsUnderVoltageCounterWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAverageRmsUnderVoltageCounterWithParams:(MTRSubscribeParams *)params
                                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAverageRmsUnderVoltageCounterWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsExtremeOverVoltagePeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsExtremeOverVoltagePeriodWithParams:(MTRSubscribeParams *)params
                                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsExtremeOverVoltagePeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsExtremeUnderVoltagePeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsExtremeUnderVoltagePeriodWithParams:(MTRSubscribeParams *)params
                                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsExtremeUnderVoltagePeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageSagPeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageSagPeriodWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageSagPeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageSwellPeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageSwellPeriodWithParams:(MTRSubscribeParams *)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageSwellPeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcVoltageMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcVoltageMultiplierWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcVoltageMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcVoltageDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcVoltageDivisorWithParams:(MTRSubscribeParams *)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcVoltageDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcCurrentMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcCurrentMultiplierWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcCurrentMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcCurrentDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcCurrentDivisorWithParams:(MTRSubscribeParams *)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcCurrentDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcPowerMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcPowerMultiplierWithParams:(MTRSubscribeParams *)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcPowerMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcPowerDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcPowerDivisorWithParams:(MTRSubscribeParams *)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcPowerDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeOverloadAlarmsMaskWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeOverloadAlarmsMaskWithParams:(MTRSubscribeParams *)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeOverloadAlarmsMaskWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeVoltageOverloadWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeVoltageOverloadWithParams:(MTRSubscribeParams *)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeVoltageOverloadWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeCurrentOverloadWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeCurrentOverloadWithParams:(MTRSubscribeParams *)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeCurrentOverloadWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcOverloadAlarmsMaskWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcOverloadAlarmsMaskWithParams:(MTRSubscribeParams *)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcOverloadAlarmsMaskWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcVoltageOverloadWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcVoltageOverloadWithParams:(MTRSubscribeParams *)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcVoltageOverloadWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcCurrentOverloadWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcCurrentOverloadWithParams:(MTRSubscribeParams *)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcCurrentOverloadWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcActivePowerOverloadWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcActivePowerOverloadWithParams:(MTRSubscribeParams *)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcActivePowerOverloadWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcReactivePowerOverloadWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcReactivePowerOverloadWithParams:(MTRSubscribeParams *)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcReactivePowerOverloadWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAverageRmsOverVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAverageRmsOverVoltageWithParams:(MTRSubscribeParams *)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAverageRmsOverVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAverageRmsUnderVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAverageRmsUnderVoltageWithParams:(MTRSubscribeParams *)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAverageRmsUnderVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsExtremeOverVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsExtremeOverVoltageWithParams:(MTRSubscribeParams *)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsExtremeOverVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsExtremeUnderVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsExtremeUnderVoltageWithParams:(MTRSubscribeParams *)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsExtremeUnderVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageSagWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageSagWithParams:(MTRSubscribeParams *)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageSagWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageSwellWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageSwellWithParams:(MTRSubscribeParams *)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageSwellWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeLineCurrentPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeLineCurrentPhaseBWithParams:(MTRSubscribeParams *)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeLineCurrentPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeActiveCurrentPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeActiveCurrentPhaseBWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeActiveCurrentPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeReactiveCurrentPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeReactiveCurrentPhaseBWithParams:(MTRSubscribeParams *)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeReactiveCurrentPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltagePhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltagePhaseBWithParams:(MTRSubscribeParams *)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltagePhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageMinPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageMinPhaseBWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageMinPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageMaxPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageMaxPhaseBWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageMaxPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsCurrentPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsCurrentPhaseBWithParams:(MTRSubscribeParams *)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsCurrentPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsCurrentMinPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsCurrentMinPhaseBWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsCurrentMinPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsCurrentMaxPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsCurrentMaxPhaseBWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsCurrentMaxPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeActivePowerPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeActivePowerPhaseBWithParams:(MTRSubscribeParams *)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeActivePowerPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeActivePowerMinPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeActivePowerMinPhaseBWithParams:(MTRSubscribeParams *)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeActivePowerMinPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeActivePowerMaxPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeActivePowerMaxPhaseBWithParams:(MTRSubscribeParams *)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeActivePowerMaxPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeReactivePowerPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeReactivePowerPhaseBWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeReactivePowerPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeApparentPowerPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeApparentPowerPhaseBWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeApparentPowerPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributePowerFactorPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributePowerFactorPhaseBWithParams:(MTRSubscribeParams *)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributePowerFactorPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithParams:(MTRSubscribeParams *)params
                                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAverageRmsOverVoltageCounterPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAverageRmsOverVoltageCounterPhaseBWithParams:(MTRSubscribeParams *)params
                                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAverageRmsOverVoltageCounterPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAverageRmsUnderVoltageCounterPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAverageRmsUnderVoltageCounterPhaseBWithParams:(MTRSubscribeParams *)params
                                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAverageRmsUnderVoltageCounterPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsExtremeOverVoltagePeriodPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsExtremeOverVoltagePeriodPhaseBWithParams:(MTRSubscribeParams *)params
                                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsExtremeOverVoltagePeriodPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsExtremeUnderVoltagePeriodPhaseBWithParams:(MTRSubscribeParams *)params
                                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageSagPeriodPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageSagPeriodPhaseBWithParams:(MTRSubscribeParams *)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageSagPeriodPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageSwellPeriodPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageSwellPeriodPhaseBWithParams:(MTRSubscribeParams *)params
                                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageSwellPeriodPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeLineCurrentPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeLineCurrentPhaseCWithParams:(MTRSubscribeParams *)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeLineCurrentPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeActiveCurrentPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeActiveCurrentPhaseCWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeActiveCurrentPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeReactiveCurrentPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeReactiveCurrentPhaseCWithParams:(MTRSubscribeParams *)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeReactiveCurrentPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltagePhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltagePhaseCWithParams:(MTRSubscribeParams *)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltagePhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageMinPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageMinPhaseCWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageMinPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageMaxPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageMaxPhaseCWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageMaxPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsCurrentPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsCurrentPhaseCWithParams:(MTRSubscribeParams *)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsCurrentPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsCurrentMinPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsCurrentMinPhaseCWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsCurrentMinPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsCurrentMaxPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsCurrentMaxPhaseCWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsCurrentMaxPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeActivePowerPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeActivePowerPhaseCWithParams:(MTRSubscribeParams *)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeActivePowerPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeActivePowerMinPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeActivePowerMinPhaseCWithParams:(MTRSubscribeParams *)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeActivePowerMinPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeActivePowerMaxPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeActivePowerMaxPhaseCWithParams:(MTRSubscribeParams *)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeActivePowerMaxPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeReactivePowerPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeReactivePowerPhaseCWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeReactivePowerPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeApparentPowerPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeApparentPowerPhaseCWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeApparentPowerPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributePowerFactorPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributePowerFactorPhaseCWithParams:(MTRSubscribeParams *)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributePowerFactorPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithParams:(MTRSubscribeParams *)params
                                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAverageRmsOverVoltageCounterPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAverageRmsOverVoltageCounterPhaseCWithParams:(MTRSubscribeParams *)params
                                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAverageRmsOverVoltageCounterPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAverageRmsUnderVoltageCounterPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAverageRmsUnderVoltageCounterPhaseCWithParams:(MTRSubscribeParams *)params
                                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAverageRmsUnderVoltageCounterPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsExtremeOverVoltagePeriodPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsExtremeOverVoltagePeriodPhaseCWithParams:(MTRSubscribeParams *)params
                                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsExtremeOverVoltagePeriodPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsExtremeUnderVoltagePeriodPhaseCWithParams:(MTRSubscribeParams *)params
                                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageSagPeriodPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageSagPeriodPhaseCWithParams:(MTRSubscribeParams *)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageSagPeriodPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeRmsVoltageSwellPeriodPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeRmsVoltageSwellPeriodPhaseCWithParams:(MTRSubscribeParams *)params
                                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeRmsVoltageSwellPeriodPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeGeneratedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeGeneratedCommandListWithParams:(MTRSubscribeParams *)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeGeneratedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAcceptedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAcceptedCommandListWithParams:(MTRSubscribeParams *)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAcceptedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeAttributeListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeAttributeListWithParams:(MTRSubscribeParams *)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeAttributeListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeFeatureMapWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeFeatureMapWithParams:(MTRSubscribeParams *)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeFeatureMapWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (void)readAttributeClusterRevisionWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)subscribeAttributeClusterRevisionWithParams:(MTRSubscribeParams *)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
+ (void)readAttributeClusterRevisionWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion MTR_DEPRECATED("This attribute is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRBaseClusterElectricalMeasurement (Availability)

/**
 * For all instance methods (reads, writes, commands) that take a completion,
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRBaseDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_DEPRECATED("ElectricalMeasurement is deprecated and will be removed", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

@end

@interface MTRBaseClusterElectricalMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRBaseDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpointID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)getProfileInfoCommandWithParams:(MTRElectricalMeasurementClusterGetProfileInfoCommandParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use getProfileInfoCommandWithParams:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getProfileInfoCommandWithCompletionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use getProfileInfoCommandWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getMeasurementProfileCommandWithParams:(MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams *)params completionHandler:(MTRStatusCompletion)completionHandler
    MTR_DEPRECATED("Please use getMeasurementProfileCommandWithParams:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeMeasurementTypeWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasurementTypeWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeMeasurementTypeWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeMeasurementTypeWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeMeasurementTypeWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasurementTypeWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcVoltageWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                            params:(MTRSubscribeParams * _Nullable)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcVoltageWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcVoltageWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcVoltageMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcVoltageMinWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcVoltageMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcVoltageMinWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcVoltageMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcVoltageMinWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcVoltageMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcVoltageMaxWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcVoltageMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcVoltageMaxWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcVoltageMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcVoltageMaxWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                            params:(MTRSubscribeParams * _Nullable)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcCurrentMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcCurrentMinWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcCurrentMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcCurrentMinWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcCurrentMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcCurrentMinWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcCurrentMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcCurrentMaxWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcCurrentMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcCurrentMaxWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcCurrentMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcCurrentMaxWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcPowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcPowerWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcPowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                          params:(MTRSubscribeParams * _Nullable)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcPowerWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcPowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcPowerWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcPowerMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcPowerMinWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcPowerMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcPowerMinWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcPowerMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcPowerMinWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcPowerMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcPowerMaxWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcPowerMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcPowerMaxWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcPowerMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcPowerMaxWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcVoltageMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcVoltageMultiplierWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcVoltageMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcVoltageMultiplierWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcVoltageMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcVoltageMultiplierWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcVoltageDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcVoltageDivisorWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcVoltageDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcVoltageDivisorWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcVoltageDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcVoltageDivisorWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcCurrentMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcCurrentMultiplierWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcCurrentMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcCurrentMultiplierWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcCurrentMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcCurrentMultiplierWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcCurrentDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcCurrentDivisorWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcCurrentDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcCurrentDivisorWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcCurrentDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcCurrentDivisorWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcPowerMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcPowerMultiplierWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcPowerMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcPowerMultiplierWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcPowerMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcPowerMultiplierWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeDcPowerDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcPowerDivisorWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeDcPowerDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeDcPowerDivisorWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeDcPowerDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeDcPowerDivisorWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcFrequencyWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcFrequencyWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcFrequencyWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                              params:(MTRSubscribeParams * _Nullable)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcFrequencyWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcFrequencyWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcFrequencyWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcFrequencyMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcFrequencyMinWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcFrequencyMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcFrequencyMinWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcFrequencyMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcFrequencyMinWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcFrequencyMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcFrequencyMaxWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcFrequencyMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcFrequencyMaxWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcFrequencyMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcFrequencyMaxWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeNeutralCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeNeutralCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeNeutralCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeNeutralCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeNeutralCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeNeutralCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeTotalActivePowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeTotalActivePowerWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeTotalActivePowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeTotalActivePowerWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeTotalActivePowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeTotalActivePowerWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeTotalReactivePowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeTotalReactivePowerWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeTotalReactivePowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeTotalReactivePowerWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeTotalReactivePowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeTotalReactivePowerWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeTotalApparentPowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeTotalApparentPowerWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeTotalApparentPowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeTotalApparentPowerWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeTotalApparentPowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeTotalApparentPowerWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeMeasured1stHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasured1stHarmonicCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeMeasured1stHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                             params:(MTRSubscribeParams * _Nullable)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeMeasured1stHarmonicCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeMeasured1stHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasured1stHarmonicCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeMeasured3rdHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasured3rdHarmonicCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeMeasured3rdHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                             params:(MTRSubscribeParams * _Nullable)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeMeasured3rdHarmonicCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeMeasured3rdHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasured3rdHarmonicCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeMeasured5thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasured5thHarmonicCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeMeasured5thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                             params:(MTRSubscribeParams * _Nullable)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeMeasured5thHarmonicCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeMeasured5thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasured5thHarmonicCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeMeasured7thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasured7thHarmonicCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeMeasured7thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                             params:(MTRSubscribeParams * _Nullable)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeMeasured7thHarmonicCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeMeasured7thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasured7thHarmonicCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeMeasured9thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasured9thHarmonicCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeMeasured9thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                             params:(MTRSubscribeParams * _Nullable)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeMeasured9thHarmonicCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeMeasured9thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasured9thHarmonicCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeMeasured11thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasured11thHarmonicCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeMeasured11thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                              params:(MTRSubscribeParams * _Nullable)params
                                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeMeasured11thHarmonicCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeMeasured11thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasured11thHarmonicCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeMeasuredPhase1stHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasuredPhase1stHarmonicCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeMeasuredPhase1stHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                  params:(MTRSubscribeParams * _Nullable)params
                                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeMeasuredPhase1stHarmonicCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeMeasuredPhase1stHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasuredPhase1stHarmonicCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeMeasuredPhase3rdHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasuredPhase3rdHarmonicCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeMeasuredPhase3rdHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                  params:(MTRSubscribeParams * _Nullable)params
                                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeMeasuredPhase3rdHarmonicCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeMeasuredPhase3rdHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasuredPhase3rdHarmonicCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeMeasuredPhase5thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasuredPhase5thHarmonicCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeMeasuredPhase5thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                  params:(MTRSubscribeParams * _Nullable)params
                                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeMeasuredPhase5thHarmonicCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeMeasuredPhase5thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasuredPhase5thHarmonicCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeMeasuredPhase7thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasuredPhase7thHarmonicCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeMeasuredPhase7thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                  params:(MTRSubscribeParams * _Nullable)params
                                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeMeasuredPhase7thHarmonicCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeMeasuredPhase7thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasuredPhase7thHarmonicCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeMeasuredPhase9thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasuredPhase9thHarmonicCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeMeasuredPhase9thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                  params:(MTRSubscribeParams * _Nullable)params
                                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeMeasuredPhase9thHarmonicCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeMeasuredPhase9thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasuredPhase9thHarmonicCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeMeasuredPhase11thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasuredPhase11thHarmonicCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeMeasuredPhase11thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                   params:(MTRSubscribeParams * _Nullable)params
                                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeMeasuredPhase11thHarmonicCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeMeasuredPhase11thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeMeasuredPhase11thHarmonicCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcFrequencyMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcFrequencyMultiplierWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcFrequencyMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcFrequencyMultiplierWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcFrequencyMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcFrequencyMultiplierWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcFrequencyDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcFrequencyDivisorWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcFrequencyDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcFrequencyDivisorWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcFrequencyDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcFrequencyDivisorWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributePowerMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePowerMultiplierWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributePowerMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributePowerMultiplierWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributePowerMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePowerMultiplierWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributePowerDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePowerDivisorWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributePowerDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributePowerDivisorWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributePowerDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePowerDivisorWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeHarmonicCurrentMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeHarmonicCurrentMultiplierWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeHarmonicCurrentMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                            params:(MTRSubscribeParams * _Nullable)params
                                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeHarmonicCurrentMultiplierWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeHarmonicCurrentMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeHarmonicCurrentMultiplierWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributePhaseHarmonicCurrentMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePhaseHarmonicCurrentMultiplierWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributePhaseHarmonicCurrentMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                 params:(MTRSubscribeParams * _Nullable)params
                                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributePhaseHarmonicCurrentMultiplierWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributePhaseHarmonicCurrentMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePhaseHarmonicCurrentMultiplierWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeInstantaneousVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeInstantaneousVoltageWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeInstantaneousVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeInstantaneousVoltageWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeInstantaneousVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeInstantaneousVoltageWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeInstantaneousLineCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeInstantaneousLineCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeInstantaneousLineCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                           params:(MTRSubscribeParams * _Nullable)params
                                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeInstantaneousLineCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeInstantaneousLineCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeInstantaneousLineCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeInstantaneousActiveCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeInstantaneousActiveCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeInstantaneousActiveCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                             params:(MTRSubscribeParams * _Nullable)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeInstantaneousActiveCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeInstantaneousActiveCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeInstantaneousActiveCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeInstantaneousReactiveCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeInstantaneousReactiveCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeInstantaneousReactiveCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                               params:(MTRSubscribeParams * _Nullable)params
                                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeInstantaneousReactiveCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeInstantaneousReactiveCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeInstantaneousReactiveCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeInstantaneousPowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeInstantaneousPowerWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeInstantaneousPowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeInstantaneousPowerWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeInstantaneousPowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeInstantaneousPowerWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageMinWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageMinWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageMinWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageMaxWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageMaxWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageMaxWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsCurrentWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsCurrentMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentMinWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsCurrentMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsCurrentMinWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsCurrentMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentMinWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsCurrentMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentMaxWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsCurrentMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsCurrentMaxWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsCurrentMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentMaxWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeActivePowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeActivePowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                              params:(MTRSubscribeParams * _Nullable)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeActivePowerWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeActivePowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeActivePowerMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerMinWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeActivePowerMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeActivePowerMinWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeActivePowerMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerMinWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeActivePowerMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerMaxWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeActivePowerMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeActivePowerMaxWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeActivePowerMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerMaxWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeReactivePowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeReactivePowerWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeReactivePowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeReactivePowerWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeReactivePowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeReactivePowerWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeApparentPowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeApparentPowerWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeApparentPowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeApparentPowerWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeApparentPowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeApparentPowerWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributePowerFactorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePowerFactorWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributePowerFactorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                              params:(MTRSubscribeParams * _Nullable)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributePowerFactorWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributePowerFactorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePowerFactorWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAverageRmsVoltageMeasurementPeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsVoltageMeasurementPeriodWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAverageRmsVoltageMeasurementPeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                     params:(MTRSubscribeParams * _Nullable)params
                                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAverageRmsVoltageMeasurementPeriodWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAverageRmsVoltageMeasurementPeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsVoltageMeasurementPeriodWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAverageRmsUnderVoltageCounterWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsUnderVoltageCounterWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeAverageRmsUnderVoltageCounterWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeAverageRmsUnderVoltageCounterWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAverageRmsUnderVoltageCounterWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                params:(MTRSubscribeParams * _Nullable)params
                                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAverageRmsUnderVoltageCounterWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAverageRmsUnderVoltageCounterWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsUnderVoltageCounterWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsExtremeOverVoltagePeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeOverVoltagePeriodWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeRmsExtremeOverVoltagePeriodWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeRmsExtremeOverVoltagePeriodWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsExtremeOverVoltagePeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                              params:(MTRSubscribeParams * _Nullable)params
                                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsExtremeOverVoltagePeriodWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsExtremeOverVoltagePeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeOverVoltagePeriodWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsExtremeUnderVoltagePeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeUnderVoltagePeriodWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeRmsExtremeUnderVoltagePeriodWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeRmsExtremeUnderVoltagePeriodWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsExtremeUnderVoltagePeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                               params:(MTRSubscribeParams * _Nullable)params
                                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsExtremeUnderVoltagePeriodWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsExtremeUnderVoltagePeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeUnderVoltagePeriodWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageSagPeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSagPeriodWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeRmsVoltageSagPeriodWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeRmsVoltageSagPeriodWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageSagPeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageSagPeriodWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageSagPeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSagPeriodWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageSwellPeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSwellPeriodWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeRmsVoltageSwellPeriodWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeRmsVoltageSwellPeriodWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageSwellPeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageSwellPeriodWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageSwellPeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSwellPeriodWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcVoltageMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcVoltageMultiplierWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcVoltageMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcVoltageMultiplierWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcVoltageMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcVoltageMultiplierWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcVoltageDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcVoltageDivisorWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcVoltageDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcVoltageDivisorWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcVoltageDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcVoltageDivisorWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcCurrentMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcCurrentMultiplierWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcCurrentMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcCurrentMultiplierWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcCurrentMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcCurrentMultiplierWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcCurrentDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcCurrentDivisorWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcCurrentDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcCurrentDivisorWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcCurrentDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcCurrentDivisorWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcPowerMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcPowerMultiplierWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcPowerMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcPowerMultiplierWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcPowerMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcPowerMultiplierWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcPowerDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcPowerDivisorWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcPowerDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcPowerDivisorWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcPowerDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcPowerDivisorWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeOverloadAlarmsMaskWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeOverloadAlarmsMaskWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeOverloadAlarmsMaskWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeOverloadAlarmsMaskWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeOverloadAlarmsMaskWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeOverloadAlarmsMaskWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeOverloadAlarmsMaskWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeOverloadAlarmsMaskWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeVoltageOverloadWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeVoltageOverloadWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeVoltageOverloadWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeVoltageOverloadWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeVoltageOverloadWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeVoltageOverloadWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeCurrentOverloadWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeCurrentOverloadWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeCurrentOverloadWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeCurrentOverloadWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeCurrentOverloadWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeCurrentOverloadWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcOverloadAlarmsMaskWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcOverloadAlarmsMaskWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeAcOverloadAlarmsMaskWithValue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use writeAttributeAcOverloadAlarmsMaskWithValue:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcOverloadAlarmsMaskWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcOverloadAlarmsMaskWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcOverloadAlarmsMaskWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcOverloadAlarmsMaskWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcVoltageOverloadWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcVoltageOverloadWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcVoltageOverloadWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcVoltageOverloadWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcVoltageOverloadWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcVoltageOverloadWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcCurrentOverloadWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcCurrentOverloadWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcCurrentOverloadWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcCurrentOverloadWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcCurrentOverloadWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcCurrentOverloadWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcActivePowerOverloadWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcActivePowerOverloadWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcActivePowerOverloadWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcActivePowerOverloadWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcActivePowerOverloadWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcActivePowerOverloadWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcReactivePowerOverloadWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcReactivePowerOverloadWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcReactivePowerOverloadWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                          params:(MTRSubscribeParams * _Nullable)params
                                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcReactivePowerOverloadWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcReactivePowerOverloadWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcReactivePowerOverloadWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAverageRmsOverVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsOverVoltageWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAverageRmsOverVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAverageRmsOverVoltageWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAverageRmsOverVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsOverVoltageWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAverageRmsUnderVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsUnderVoltageWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAverageRmsUnderVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                         params:(MTRSubscribeParams * _Nullable)params
                                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAverageRmsUnderVoltageWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAverageRmsUnderVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsUnderVoltageWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsExtremeOverVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeOverVoltageWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsExtremeOverVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsExtremeOverVoltageWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsExtremeOverVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeOverVoltageWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsExtremeUnderVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeUnderVoltageWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsExtremeUnderVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                         params:(MTRSubscribeParams * _Nullable)params
                                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsExtremeUnderVoltageWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsExtremeUnderVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeUnderVoltageWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageSagWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSagWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageSagWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageSagWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageSagWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSagWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageSwellWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSwellWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageSwellWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageSwellWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageSwellWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSwellWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeLineCurrentPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeLineCurrentPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeLineCurrentPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeLineCurrentPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeLineCurrentPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeLineCurrentPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeActiveCurrentPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActiveCurrentPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeActiveCurrentPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeActiveCurrentPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeActiveCurrentPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActiveCurrentPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeReactiveCurrentPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeReactiveCurrentPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeReactiveCurrentPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeReactiveCurrentPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeReactiveCurrentPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeReactiveCurrentPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltagePhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltagePhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltagePhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltagePhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltagePhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltagePhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageMinPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageMinPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageMinPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageMinPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageMinPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageMinPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageMaxPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageMaxPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageMaxPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageMaxPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageMaxPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageMaxPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsCurrentPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsCurrentPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsCurrentPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsCurrentPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsCurrentMinPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentMinPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsCurrentMinPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsCurrentMinPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsCurrentMinPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentMinPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsCurrentMaxPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentMaxPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsCurrentMaxPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsCurrentMaxPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsCurrentMaxPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentMaxPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeActivePowerPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeActivePowerPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeActivePowerPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeActivePowerPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeActivePowerMinPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerMinPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeActivePowerMinPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeActivePowerMinPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeActivePowerMinPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerMinPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeActivePowerMaxPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerMaxPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeActivePowerMaxPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeActivePowerMaxPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeActivePowerMaxPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerMaxPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeReactivePowerPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeReactivePowerPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeReactivePowerPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeReactivePowerPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeReactivePowerPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeReactivePowerPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeApparentPowerPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeApparentPowerPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeApparentPowerPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeApparentPowerPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeApparentPowerPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeApparentPowerPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributePowerFactorPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePowerFactorPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributePowerFactorPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributePowerFactorPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributePowerFactorPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePowerFactorPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                           params:(MTRSubscribeParams * _Nullable)params
                                                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAverageRmsOverVoltageCounterPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsOverVoltageCounterPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAverageRmsOverVoltageCounterPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                     params:(MTRSubscribeParams * _Nullable)params
                                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAverageRmsOverVoltageCounterPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAverageRmsOverVoltageCounterPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsOverVoltageCounterPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAverageRmsUnderVoltageCounterPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsUnderVoltageCounterPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAverageRmsUnderVoltageCounterPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                      params:(MTRSubscribeParams * _Nullable)params
                                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAverageRmsUnderVoltageCounterPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAverageRmsUnderVoltageCounterPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsUnderVoltageCounterPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsExtremeOverVoltagePeriodPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeOverVoltagePeriodPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsExtremeOverVoltagePeriodPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                    params:(MTRSubscribeParams * _Nullable)params
                                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsExtremeOverVoltagePeriodPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsExtremeOverVoltagePeriodPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeOverVoltagePeriodPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsExtremeUnderVoltagePeriodPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                     params:(MTRSubscribeParams * _Nullable)params
                                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsExtremeUnderVoltagePeriodPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageSagPeriodPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSagPeriodPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageSagPeriodPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                            params:(MTRSubscribeParams * _Nullable)params
                                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageSagPeriodPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageSagPeriodPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSagPeriodPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageSwellPeriodPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSwellPeriodPhaseBWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageSwellPeriodPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                              params:(MTRSubscribeParams * _Nullable)params
                                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageSwellPeriodPhaseBWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageSwellPeriodPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSwellPeriodPhaseBWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeLineCurrentPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeLineCurrentPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeLineCurrentPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeLineCurrentPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeLineCurrentPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeLineCurrentPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeActiveCurrentPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActiveCurrentPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeActiveCurrentPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeActiveCurrentPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeActiveCurrentPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActiveCurrentPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeReactiveCurrentPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeReactiveCurrentPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeReactiveCurrentPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeReactiveCurrentPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeReactiveCurrentPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeReactiveCurrentPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltagePhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltagePhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltagePhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltagePhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltagePhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltagePhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageMinPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageMinPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageMinPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageMinPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageMinPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageMinPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageMaxPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageMaxPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageMaxPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageMaxPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageMaxPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageMaxPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsCurrentPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsCurrentPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsCurrentPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsCurrentPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsCurrentMinPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentMinPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsCurrentMinPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsCurrentMinPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsCurrentMinPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentMinPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsCurrentMaxPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentMaxPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsCurrentMaxPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsCurrentMaxPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsCurrentMaxPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsCurrentMaxPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeActivePowerPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeActivePowerPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeActivePowerPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeActivePowerPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeActivePowerMinPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerMinPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeActivePowerMinPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeActivePowerMinPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeActivePowerMinPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerMinPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeActivePowerMaxPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerMaxPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeActivePowerMaxPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeActivePowerMaxPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeActivePowerMaxPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeActivePowerMaxPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeReactivePowerPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeReactivePowerPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeReactivePowerPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeReactivePowerPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeReactivePowerPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeReactivePowerPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeApparentPowerPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeApparentPowerPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeApparentPowerPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeApparentPowerPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeApparentPowerPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeApparentPowerPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributePowerFactorPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePowerFactorPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributePowerFactorPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributePowerFactorPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributePowerFactorPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributePowerFactorPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                           params:(MTRSubscribeParams * _Nullable)params
                                                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAverageRmsOverVoltageCounterPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsOverVoltageCounterPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAverageRmsOverVoltageCounterPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                     params:(MTRSubscribeParams * _Nullable)params
                                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAverageRmsOverVoltageCounterPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAverageRmsOverVoltageCounterPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsOverVoltageCounterPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAverageRmsUnderVoltageCounterPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsUnderVoltageCounterPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAverageRmsUnderVoltageCounterPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                      params:(MTRSubscribeParams * _Nullable)params
                                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAverageRmsUnderVoltageCounterPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAverageRmsUnderVoltageCounterPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAverageRmsUnderVoltageCounterPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsExtremeOverVoltagePeriodPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeOverVoltagePeriodPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsExtremeOverVoltagePeriodPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                    params:(MTRSubscribeParams * _Nullable)params
                                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsExtremeOverVoltagePeriodPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsExtremeOverVoltagePeriodPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeOverVoltagePeriodPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsExtremeUnderVoltagePeriodPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                     params:(MTRSubscribeParams * _Nullable)params
                                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsExtremeUnderVoltagePeriodPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageSagPeriodPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSagPeriodPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageSagPeriodPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                            params:(MTRSubscribeParams * _Nullable)params
                                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageSagPeriodPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageSagPeriodPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSagPeriodPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeRmsVoltageSwellPeriodPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSwellPeriodPhaseCWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeRmsVoltageSwellPeriodPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                              params:(MTRSubscribeParams * _Nullable)params
                                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeRmsVoltageSwellPeriodPhaseCWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeRmsVoltageSwellPeriodPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeRmsVoltageSwellPeriodPhaseCWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeGeneratedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeGeneratedCommandListWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeGeneratedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeGeneratedCommandListWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeGeneratedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeGeneratedCommandListWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAcceptedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcceptedCommandListWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAcceptedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAcceptedCommandListWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAcceptedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAcceptedCommandListWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeAttributeListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAttributeListWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeAttributeListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeAttributeListWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeAttributeListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeAttributeListWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeFeatureMapWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeFeatureMapWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeFeatureMapWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeFeatureMapWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeFeatureMapWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeFeatureMapWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)readAttributeClusterRevisionWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeClusterRevisionWithCompletion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)subscribeAttributeClusterRevisionWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler MTR_DEPRECATED("Please use subscribeAttributeClusterRevisionWithParams:subscriptionEstablished:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (void)readAttributeClusterRevisionWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler MTR_DEPRECATED("Please use readAttributeClusterRevisionWithAttributeCache:endpoint:queue:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

/**
 * Cluster Electrical Measurement
 *    Attributes related to the electrical properties of a device. This cluster is used by power outlets and other devices that need to provide instantaneous data as opposed to metrology data which should be retrieved from the metering cluster..
 */
MTR_DEPRECATED("ElectricalMeasurement is deprecated and will be removed", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2))
@interface MTRClusterElectricalMeasurement : MTRGenericCluster

- (void)getProfileInfoCommandWithParams:(MTRElectricalMeasurementClusterGetProfileInfoCommandParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This command is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)getProfileInfoCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
    MTR_DEPRECATED("This command is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));
- (void)getMeasurementProfileCommandWithParams:(MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion MTR_DEPRECATED("This command is deprecated", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementTypeWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcVoltageWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcVoltageMinWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcVoltageMaxWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcCurrentMinWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcCurrentMaxWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcPowerWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcPowerMinWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcPowerMaxWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcVoltageMultiplierWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcVoltageDivisorWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcCurrentDivisorWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcPowerMultiplierWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcPowerDivisorWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcFrequencyWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcFrequencyMinWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcFrequencyMaxWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNeutralCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTotalActivePowerWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTotalReactivePowerWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTotalApparentPowerWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasured1stHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasured3rdHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasured5thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasured7thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasured9thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasured11thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredPhase1stHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredPhase3rdHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredPhase5thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredPhase7thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredPhase9thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredPhase11thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcFrequencyMultiplierWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcFrequencyDivisorWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerMultiplierWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerDivisorWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHarmonicCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePhaseHarmonicCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstantaneousVoltageWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstantaneousLineCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstantaneousActiveCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstantaneousReactiveCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstantaneousPowerWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageMinWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageMaxWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentMinWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentMaxWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerMinWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerMaxWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReactivePowerWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApparentPowerWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerFactorWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsVoltageMeasurementPeriodWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsUnderVoltageCounterWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeOverVoltagePeriodWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeUnderVoltagePeriodWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSagPeriodWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSwellPeriodWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcVoltageMultiplierWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcVoltageDivisorWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcCurrentDivisorWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcPowerMultiplierWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcPowerDivisorWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOverloadAlarmsMaskWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeVoltageOverloadWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentOverloadWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcOverloadAlarmsMaskWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcVoltageOverloadWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcCurrentOverloadWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcActivePowerOverloadWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcReactivePowerOverloadWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsOverVoltageWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsUnderVoltageWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeOverVoltageWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeUnderVoltageWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSagWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSwellWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLineCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReactiveCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltagePhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageMinPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageMaxPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentMinPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentMaxPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerMinPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerMaxPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReactivePowerPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApparentPowerPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerFactorPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsOverVoltageCounterPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsUnderVoltageCounterPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeOverVoltagePeriodPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSagPeriodPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSwellPeriodPhaseBWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLineCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReactiveCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltagePhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageMinPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageMaxPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentMinPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentMaxPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerMinPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerMaxPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReactivePowerPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApparentPowerPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerFactorPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsOverVoltageCounterPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsUnderVoltageCounterPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeOverVoltagePeriodPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSagPeriodPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSwellPeriodPhaseCWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params MTR_DEPRECATED("This attribute is deprecated", ios(16.1, 18.2), macos(13.0, 15.2), watchos(9.1, 11.2), tvos(16.1, 18.2));

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRClusterElectricalMeasurement (Availability)

/**
 * For all instance methods that take a completion (i.e. command invocations),
 * the completion will be called on the provided queue.
 */
- (instancetype _Nullable)initWithDevice:(MTRDevice *)device
                              endpointID:(NSNumber *)endpointID
                                   queue:(dispatch_queue_t)queue MTR_DEPRECATED("ElectricalMeasurement is deprecated and will be removed", ios(16.4, 18.2), macos(13.3, 15.2), watchos(9.4, 11.2), tvos(16.4, 18.2));

@end

@interface MTRClusterElectricalMeasurement (Deprecated)

- (nullable instancetype)initWithDevice:(MTRDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue MTR_DEPRECATED("Please use initWithDevice:endpoindID:queue:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)getProfileInfoCommandWithParams:(MTRElectricalMeasurementClusterGetProfileInfoCommandParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use getProfileInfoCommandWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getProfileInfoCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use getProfileInfoCommandWithExpectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (void)getMeasurementProfileCommandWithParams:(MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler MTR_DEPRECATED("Please use getMeasurementProfileCommandWithParams:expectedValues:expectedValueInterval:completion:", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

// Note: Add new shims that are not entire cluster removals above the "Clusters that were removed wholesale:" bits

NS_ASSUME_NONNULL_END
