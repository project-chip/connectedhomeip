/**
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

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, MTROTAImageDigestType) {
    MTROTAImageDigestTypeSha256 = 1,
    MTROTAImageDigestTypeSha256_128,
    MTROTAImageDigestTypeSha256_120,
    MTROTAImageDigestTypeSha256_96,
    MTROTAImageDigestTypeSha256_64,
    MTROTAImageDigestTypeSha256_32,
    MTROTAImageDigestTypeSha384,
    MTROTAImageDigestTypeSha512,
    MTROTAImageDigestTypeSha3_224,
    MTROTAImageDigestTypeSha3_256,
    MTROTAImageDigestTypeSha3_384,
    MTROTAImageDigestTypeSha3_512,
};

@interface MTROTAHeader : NSObject

@property (nonatomic, strong) NSNumber * vendorID;
@property (nonatomic, strong) NSNumber * productID;
@property (nonatomic, strong) NSNumber * payloadSize;
@property (nonatomic, strong) NSNumber * softwareVersion;
@property (nonatomic, strong) NSString * softwareVersionString;
@property (nonatomic, strong) NSString * releaseNotesURL;
@property (nonatomic, strong) NSData * imageDigest;
@property (nonatomic, assign) MTROTAImageDigestType imageDigestType;
@property (nonatomic, strong) NSNumber * minApplicableVersion;
@property (nonatomic, strong) NSNumber * maxApplicableVersion;

@end

@interface MTROTAHeaderParser : NSObject
+ (nullable MTROTAHeader *)headerFromData:(NSData *)data error:(NSError * __autoreleasing *)error;
@end

NS_ASSUME_NONNULL_END
