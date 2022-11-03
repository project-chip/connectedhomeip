/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
                completionHandler:(void (^_Nonnull)(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                                      NSError * _Nullable error))completionHandler;

- (void)handleApplyUpdateRequestForNodeID:(NSNumber * _Nonnull)nodeID
                               controller:(MTRDeviceController * _Nonnull)controller
                                   params:(MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams * _Nonnull)params
                        completionHandler:
                            (void (^_Nonnull)(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                NSError * _Nullable error))completionHandler;

- (void)handleNotifyUpdateAppliedForNodeID:(NSNumber * _Nonnull)nodeID
                                controller:(MTRDeviceController * _Nonnull)controller
                                    params:(MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams * _Nonnull)params
                         completionHandler:(StatusCompletion _Nonnull)completionHandler;

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
