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
#include <fstream>

constexpr uint8_t kUpdateTokenLen = 32;

@interface OTAProviderDelegate ()
@property NSString * mOTAFilePath;
@property NSFileHandle * mFileHandle;
@property NSNumber * mFileOffset;
@property NSNumber * mFileEndOffset;
@property DeviceSoftwareVersionModel * candidate;
@end

@implementation OTAProviderDelegate

- (instancetype)init
{
    if (self = [super init]) {
        _selectedCandidate = [[DeviceSoftwareVersionModel alloc] init];
        _userConsentState = OTAProviderUserUnknown;
    }
    return self;
}

- (void)handleQueryImageForNodeID:(NSNumber * _Nonnull)nodeID
                       controller:(MTRDeviceController * _Nonnull)controller
                           params:(MTROtaSoftwareUpdateProviderClusterQueryImageParams * _Nonnull)params
                completionHandler:(void (^_Nonnull)(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                                      NSError * _Nullable error))completionHandler
{
    NSError * error;

    auto isBDXProtocolSupported =
        [params.protocolsSupported containsObject:@(MTROtaSoftwareUpdateProviderOTADownloadProtocolBDXSynchronous)];
    if (!isBDXProtocolSupported) {
        _selectedCandidate.status = @(MTROtaSoftwareUpdateProviderOTAQueryStatusDownloadProtocolNotSupported);
        error =
            [[NSError alloc] initWithDomain:@"OTAProviderDomain"
                                       code:MTRErrorCodeGeneralError
                                   userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Protocol is not supported.", nil) }];
        completionHandler(_selectedCandidate, error);
        return;
    }

    auto hasCandidate = [self SelectOTACandidate:params.vendorId rPID:params.productId rSV:params.softwareVersion];
    if (!hasCandidate) {
        NSLog(@"Unable to select OTA Image.");
        _selectedCandidate.status = @(MTROtaSoftwareUpdateProviderOTAQueryStatusNotAvailable);
        error = [[NSError alloc]
            initWithDomain:@"OTAProviderDomain"
                      code:MTRErrorCodeInvalidState
                  userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(@"Unable to select Candidate.", nil) }];
        return;
    }

    _selectedCandidate.updateToken = [self generateUpdateToken];

    if (params.requestorCanConsent.integerValue == 1) {
        _selectedCandidate.status = @(MTROtaSoftwareUpdateProviderOTAQueryStatusUpdateAvailable);
        _selectedCandidate.userConsentNeeded
            = (_userConsentState == OTAProviderUserUnknown || _userConsentState == OTAProviderUserDenied) ? @(1) : @(0);
        NSLog(@"User Consent Needed: %@", _selectedCandidate.userConsentNeeded);
        completionHandler(_selectedCandidate, error);
        return;
    }

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
    completionHandler(_selectedCandidate, error);
}

- (void)handleApplyUpdateRequestForNodeID:(NSNumber * _Nonnull)nodeID
                               controller:(MTRDeviceController * _Nonnull)controller
                                   params:(MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams * _Nonnull)params
                        completionHandler:
                            (void (^_Nonnull)(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                NSError * _Nullable error))completionHandler
{
    MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * applyUpdateResponseParams =
        [[MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams alloc] init];
    applyUpdateResponseParams.action = @(MTROtaSoftwareUpdateProviderOTAApplyUpdateActionProceed);
    completionHandler(applyUpdateResponseParams, nil);
}

- (void)handleNotifyUpdateAppliedForNodeID:(NSNumber * _Nonnull)nodeID
                                controller:(MTRDeviceController * _Nonnull)controller
                                    params:(MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams * _Nonnull)params
                         completionHandler:(StatusCompletion _Nonnull)completionHandler
{
    completionHandler(nil);
}

- (void)handleBDXTransferSessionBeginForNodeID:(NSNumber * _Nonnull)nodeID
                                    controller:(MTRDeviceController * _Nonnull)controller
                                fileDesignator:(NSString * _Nonnull)fileDesignator
                                        offset:(NSNumber * _Nonnull)offset
                             completionHandler:(void (^)(NSError * error))completionHandler
{
    NSLog(@"BDX TransferSession begin with %@ (offset: %@)", fileDesignator, offset);

    auto * handle = [NSFileHandle fileHandleForReadingAtPath:fileDesignator];
    if (handle == nil) {
        auto errorString = [NSString stringWithFormat:@"Error accessing file at at %@", fileDesignator];
        auto error = [[NSError alloc] initWithDomain:@"OTAProviderDomain"
                                                code:MTRErrorCodeGeneralError
                                            userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(errorString, nil) }];
        completionHandler(error);
        return;
    }

    NSError * seekError = nil;
    [handle seekToOffset:[offset unsignedLongValue] error:&seekError];
    if (seekError != nil) {
        auto errorString = [NSString stringWithFormat:@"Error seeking file (%@) to offset %@", fileDesignator, offset];
        auto error = [[NSError alloc] initWithDomain:@"OTAProviderDomain"
                                                code:MTRErrorCodeGeneralError
                                            userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(errorString, nil) }];
        completionHandler(error);
        return;
    }

    uint64_t endOffset;
    if (![handle seekToEndReturningOffset:&endOffset error:&seekError]) {
        auto errorString = [NSString stringWithFormat:@"Error seeking file (%@) to end offset", fileDesignator];
        auto error = [[NSError alloc] initWithDomain:@"OTAProviderDomain"
                                                code:MTRErrorCodeGeneralError
                                            userInfo:@{ NSLocalizedDescriptionKey : NSLocalizedString(errorString, nil) }];
        completionHandler(error);
        return;
    }

    _mFileHandle = handle;
    _mFileOffset = offset;
    _mFileEndOffset = @(endOffset);
    completionHandler(nil);
}

- (void)handleBDXTransferSessionEndForNodeID:(NSNumber * _Nonnull)nodeID
                                  controller:(MTRDeviceController * _Nonnull)controller
                                       error:(NSError * _Nullable)error
{
    NSLog(@"BDX TransferSession end with error: %@", error);
    _mFileHandle = nil;
    _mFileOffset = nil;
    _mFileEndOffset = nil;
}

- (void)handleBDXQueryForNodeID:(NSNumber * _Nonnull)nodeID
                     controller:(MTRDeviceController * _Nonnull)controller
                      blockSize:(NSNumber * _Nonnull)blockSize
                     blockIndex:(NSNumber * _Nonnull)blockIndex
                    bytesToSkip:(NSNumber * _Nonnull)bytesToSkip
              completionHandler:(void (^)(NSData * _Nullable data, BOOL isEOF))completionHandler
{
    NSLog(@"BDX Query received blockSize: %@, blockIndex: %@", blockSize, blockIndex);

    NSError * error = nil;
    auto offset = [_mFileOffset unsignedLongValue] + [bytesToSkip unsignedLongLongValue]
        + ([blockSize unsignedLongValue] * [blockIndex unsignedLongValue]);
    [_mFileHandle seekToOffset:offset error:&error];
    if (error != nil) {
        NSLog(@"Error seeking to offset %@", @(offset));
        completionHandler(nil, NO);
        return;
    }

    NSData * data = [_mFileHandle readDataUpToLength:[blockSize unsignedLongValue] error:&error];
    if (error != nil) {
        NSLog(@"Error reading file %@", _mFileHandle);
        completionHandler(nil, NO);
        return;
    }

    BOOL isEOF = offset + [blockSize unsignedLongValue] >= [_mFileEndOffset unsignedLongLongValue];
    completionHandler(data, isEOF);
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
    auto vendorId = [requestorVendorID unsignedIntValue];
    auto productId = [requestorProductID unsignedIntValue];
    auto softwareVersion = [requestorSoftwareVersion unsignedLongValue];

    bool candidateFound = false;
    NSArray * sortedArray = [_candidates sortedArrayUsingSelector:@selector(CompareSoftwareVersions:)];
    for (DeviceSoftwareVersionModel * candidate : sortedArray) {
        auto candidateSoftwareVersionValid = candidate.deviceModelData.softwareVersionValid;
        auto candidateSoftwareVersion = [candidate.softwareVersion unsignedLongValue];
        auto candidateMinApplicableSoftwareVersion = [candidate.deviceModelData.minApplicableSoftwareVersion unsignedLongValue];
        auto candidateMaxApplicableSoftwareVersion = [candidate.deviceModelData.maxApplicableSoftwareVersion unsignedLongValue];
        auto candidateVendorId = [candidate.deviceModelData.vendorId unsignedIntValue];
        auto candidateProductId = [candidate.deviceModelData.productId unsignedIntValue];

        if (candidateSoftwareVersionValid && (softwareVersion < candidateSoftwareVersion)
            && (softwareVersion >= candidateMinApplicableSoftwareVersion)
            && (softwareVersion <= candidateMaxApplicableSoftwareVersion) && (vendorId == candidateVendorId)
            && (productId == candidateProductId)) {
            _selectedCandidate = candidate;
            _selectedCandidate.imageURI = candidate.deviceModelData.otaURL;
            candidateFound = true;
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
