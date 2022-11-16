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
#pragma once
#import <Matter/Matter.h>

typedef NS_ENUM(uint8_t, UserConsentState) {
    OTAProviderUserGranted = 0x00,
    OTAProviderUserObtaining = 0x01,
    OTAProviderUserDenied = 0x02,
    OTAProviderUserUnknown = 0x03,
};

@interface DeviceSoftwareVersionModelData : MTROtaSoftwareUpdateProviderClusterQueryImageParams
@property BOOL softwareVersionValid;
@property (strong, nonatomic, nullable) NSNumber * cDVersionNumber;
@property (strong, nonatomic, nullable) NSNumber * minApplicableSoftwareVersion;
@property (strong, nonatomic, nullable) NSNumber * maxApplicableSoftwareVersion;
@property (strong, nonatomic, nullable) NSString * otaURL;
@end

@interface DeviceSoftwareVersionModel : MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams
@property (strong, nonatomic, nullable) DeviceSoftwareVersionModelData * deviceModelData;
- (NSComparisonResult)CompareSoftwareVersions:(DeviceSoftwareVersionModel * _Nullable)otherObject;
@end

@interface OTAProviderDelegate : NSObject <MTROTAProviderDelegate>
- (void)handleQueryImageForNodeID:(NSNumber * _Nonnull)nodeID
                       controller:(MTRDeviceController * _Nonnull)controller
                           params:(MTROtaSoftwareUpdateProviderClusterQueryImageParams * _Nonnull)params
                       completion:(void (^_Nonnull)(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                                      NSError * _Nullable error))completion;

- (void)handleApplyUpdateRequestForNodeID:(NSNumber * _Nonnull)nodeID
                               controller:(MTRDeviceController * _Nonnull)controller
                                   params:(MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams * _Nonnull)params
                               completion:
                                   (void (^_Nonnull)(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                       NSError * _Nullable error))completion;

- (void)handleNotifyUpdateAppliedForNodeID:(NSNumber * _Nonnull)nodeID
                                controller:(MTRDeviceController * _Nonnull)controller
                                    params:(MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams * _Nonnull)params
                                completion:(MTRStatusCompletion _Nonnull)completion;

@property (strong, nonatomic, nullable) NSArray<DeviceSoftwareVersionModel *> * candidates;
@property (strong, nonatomic, nullable) DeviceSoftwareVersionModel * selectedCandidate;
@property (strong, nonatomic, nullable) NSNumber * nodeID;
@property (nonatomic, readwrite) MTROtaSoftwareUpdateProviderOTAQueryStatus queryImageStatus;
@property (nonatomic, readwrite) UserConsentState userConsentState;
@property (nonatomic, readwrite) MTROtaSoftwareUpdateProviderOTAApplyUpdateAction action;
@property (nonatomic, readwrite, nullable) NSNumber * delayedActionTime;
@property (nonatomic, readwrite, nullable) NSNumber * timedInvokeTimeoutMs;
@property (nonatomic, readwrite, nullable) NSNumber * userConsentNeeded;

@end
