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

#include "OTAProviderDelegate.h"
#import <Matter/Matter.h>

constexpr uint8_t kUpdateTokenLen = 32;

@interface OTAProviderDelegate ()
@property NSString * mOTAFilePath;
@property DeviceSoftwareVersionModel * candidate;
@end

@implementation OTAProviderDelegate

- (instancetype)init
{
    if (self = [super init]) {
        _nodeID = @(0);
        _selectedCandidate = [[DeviceSoftwareVersionModel alloc] init];
        _userConsentState = OTAProviderUserUnknown;
    }
    return self;
}

// TODO: When BDX is added to Matter.framework, update to initialize
// it when there is an update available.
- (void)handleQueryImage:(MTROtaSoftwareUpdateProviderClusterQueryImageParams * _Nonnull)params
       completionHandler:(void (^_Nonnull)(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                             NSError * _Nullable error))completionHandler
{
    NSError * error;
    _selectedCandidate.status = @(MTROtaSoftwareUpdateProviderOTAQueryStatusNotAvailable);
    if (![params.protocolsSupported containsObject:@(MTROtaSoftwareUpdateProviderOTADownloadProtocolBDXSynchronous)]) {
        _selectedCandidate.status = @(MTROtaSoftwareUpdateProviderOTAQueryStatusDownloadProtocolNotSupported);
        error =
            [[NSError alloc] initWithDomain:@"OTAProviderDomain"
                                       code:MTRErrorCodeGeneralError
                                   userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Protocol is not supported.", nil) }];
        completionHandler(_selectedCandidate, error);
        return;
    }

    if ([self SelectOTACandidate:params.vendorId rPID:params.productId rSV:params.softwareVersion]) {
        _selectedCandidate.status = @(MTROtaSoftwareUpdateProviderOTAQueryStatusUpdateAvailable);
        _selectedCandidate.updateToken = [self generateUpdateToken];
        if (params.requestorCanConsent.integerValue == 1) {
            _selectedCandidate.userConsentNeeded
                = (_userConsentState == OTAProviderUserUnknown || _userConsentState == OTAProviderUserDenied) ? @(1) : @(0);
            NSLog(@"User Consent Needed: %@", _selectedCandidate.userConsentNeeded);
        } else {
            NSLog(@"Requestor cannot obtain user consent. Our State: %hhu", _userConsentState);
            switch (_userConsentState) {
            case OTAProviderUserGranted:
                NSLog(@"User Consent Granted");
                _queryImageStatus = MTROtaSoftwareUpdateProviderOTAQueryStatusUpdateAvailable;
                break;

            case OTAProviderUserObtaining:
                NSLog(@"User Consent Obtaining");
                _queryImageStatus = MTROtaSoftwareUpdateProviderOTAQueryStatusBusy;
                break;

            case OTAProviderUserDenied:
            case OTAProviderUserUnknown:
                NSLog(@"User Consent Denied or Uknown");
                _queryImageStatus = MTROtaSoftwareUpdateProviderOTAQueryStatusNotAvailable;
                break;
            }
            _selectedCandidate.status = @(_queryImageStatus);
        }
    } else {
        NSLog(@"Unable to select OTA Image.");
        _selectedCandidate.status = @(MTROtaSoftwareUpdateProviderOTAQueryStatusNotAvailable);
        error = [[NSError alloc]
            initWithDomain:@"OTAProviderDomain"
                      code:MTRErrorCodeInvalidState
                  userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Unable to select Candidate.", nil) }];
    }
    completionHandler(_selectedCandidate, error);
}

- (void)handleApplyUpdateRequest:(MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams * _Nonnull)params
               completionHandler:(void (^_Nonnull)(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                     NSError * _Nullable error))completionHandler
{
    MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * applyUpdateResponsePrams =
        [[MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams alloc] init];
    applyUpdateResponsePrams.action = @(MTROtaSoftwareUpdateProviderOTAApplyUpdateActionProceed);
    completionHandler(applyUpdateResponsePrams, nil);
}

- (void)handleNotifyUpdateApplied:(MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams * _Nonnull)params
                completionHandler:(StatusCompletion _Nonnull)completionHandler
{
    completionHandler(nil);
}

- (void)SetOTAFilePath:(const char *)path
{
    _mOTAFilePath = [NSString stringWithUTF8String:path];
}

- (NSData *)generateUpdateToken
{
    NSMutableData * updateTokenData = [NSMutableData dataWithCapacity:kUpdateTokenLen];
    for (unsigned int i = 0; i < kUpdateTokenLen / 4; ++i) {
        u_int32_t randomBits = arc4random();
        [updateTokenData appendBytes:(void *) &randomBits length:4];
    }

    return [NSData dataWithData:updateTokenData];
}

- (bool)SelectOTACandidate:(NSNumber *)requestorVendorID
                      rPID:(NSNumber *)requestorProductID
                       rSV:(NSNumber *)requestorSoftwareVersion
{
    bool candidateFound = false;
    NSArray * sortedArray = [_candidates sortedArrayUsingSelector:@selector(CompareSoftwareVersions:)];
    for (DeviceSoftwareVersionModel * candidate : sortedArray) {
        if (candidate.deviceModelData.softwareVersionValid
            && ([requestorSoftwareVersion unsignedLongValue] < [candidate.softwareVersion unsignedLongValue])
            && ([requestorSoftwareVersion unsignedLongValue] >=
                [candidate.deviceModelData.minApplicableSoftwareVersion unsignedLongValue])
            && ([requestorSoftwareVersion unsignedLongValue] <=
                [candidate.deviceModelData.maxApplicableSoftwareVersion unsignedLongValue])
            && ([requestorVendorID unsignedIntValue] == [candidate.deviceModelData.vendorId unsignedIntValue])
            && ([requestorProductID unsignedIntValue] == [candidate.deviceModelData.productId unsignedIntValue])) {
            candidateFound = true;
            _selectedCandidate = candidate;
            _selectedCandidate.imageURI = [NSString
                stringWithFormat:@"bdx://%016llX/%@", [_nodeID unsignedLongLongValue], _selectedCandidate.deviceModelData.otaURL];
        }
    }
    return candidateFound;
}

@end

@implementation DeviceSoftwareVersionModelData
- (instancetype)init
{
    if (self = [super init]) {
        _cDVersionNumber = nil;

        _minApplicableSoftwareVersion = nil;

        _maxApplicableSoftwareVersion = nil;

        _otaURL = nil;
    }
    return self;
}

@end

@implementation DeviceSoftwareVersionModel
- (instancetype)init
{
    if (self = [super init]) {
        _deviceModelData = [[DeviceSoftwareVersionModelData alloc] init];
    }
    return self;
}
- (NSComparisonResult)CompareSoftwareVersions:(DeviceSoftwareVersionModel * _Nullable)otherObject
{
    return [self.deviceModelData.softwareVersion compare:otherObject.deviceModelData.softwareVersion];
}
@end
