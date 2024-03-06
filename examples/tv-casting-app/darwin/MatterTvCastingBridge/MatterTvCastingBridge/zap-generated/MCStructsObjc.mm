/**
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

#import "MCStructsObjc.h"

#import <Foundation/Foundation.h>

@implementation MCMediaPlaybackClusterPlaybackPositionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _updatedAt = @(0);

        _position = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCMediaPlaybackClusterPlaybackPositionStruct alloc] init];

    other.updatedAt = self.updatedAt;
    other.position = self.position;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: updatedAt:%@; position:%@; >", NSStringFromClass([self class]), _updatedAt, _position];
    return descriptionString;
}

@end

@implementation MCMediaPlaybackClusterPlaybackPosition : MCMediaPlaybackClusterPlaybackPositionStruct
@dynamic updatedAt;
@dynamic position;
@end

@implementation MCMediaPlaybackClusterTrackAttributesStruct
- (instancetype)init
{
    if (self = [super init]) {

        _languageCode = @"";

        _displayName = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCMediaPlaybackClusterTrackAttributesStruct alloc] init];

    other.languageCode = self.languageCode;
    other.displayName = self.displayName;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: languageCode:%@; displayName:%@; >", NSStringFromClass([self class]), _languageCode, _displayName];
    return descriptionString;
}

@end

@implementation MCMediaPlaybackClusterTrackStruct
- (instancetype)init
{
    if (self = [super init]) {

        _id = @"";

        _trackAttributes = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCMediaPlaybackClusterTrackStruct alloc] init];

    other.id = self.id;
    other.trackAttributes = self.trackAttributes;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: id:%@; trackAttributes:%@; >", NSStringFromClass([self class]), _id, _trackAttributes];
    return descriptionString;
}

@end

@implementation MCApplicationBasicClusterApplicationStruct
- (instancetype)init
{
    if (self = [super init]) {

        _catalogVendorID = @(0);

        _applicationID = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCApplicationBasicClusterApplicationStruct alloc] init];

    other.catalogVendorID = self.catalogVendorID;
    other.applicationID = self.applicationID;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: catalogVendorID:%@; applicationID:%@; >", NSStringFromClass([self class]), _catalogVendorID, _applicationID];
    return descriptionString;
}

- (void)setCatalogVendorId:(NSNumber * _Nonnull)catalogVendorId
{
    self.catalogVendorID = catalogVendorId;
}

- (NSNumber * _Nonnull)catalogVendorId
{
    return self.catalogVendorID;
}

- (void)setApplicationId:(NSString * _Nonnull)applicationId
{
    self.applicationID = applicationId;
}

- (NSString * _Nonnull)applicationId
{
    return self.applicationID;
}

@end

@implementation MCApplicationBasicClusterApplicationBasicApplication : MCApplicationBasicClusterApplicationStruct
@end

@implementation MCTargetNavigatorClusterTargetInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _identifier = @(0);

        _name = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCTargetNavigatorClusterTargetInfoStruct alloc] init];

    other.identifier = self.identifier;
    other.name = self.name;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: identifier:%@; name:%@; >", NSStringFromClass([self class]), _identifier, _name];
    return descriptionString;
}

@end

@implementation MCTargetNavigatorClusterTargetInfo : MCTargetNavigatorClusterTargetInfoStruct
@dynamic identifier;
@dynamic name;
@end

@implementation MCApplicationLauncherClusterApplicationStruct
- (instancetype)init
{
    if (self = [super init]) {

        _catalogVendorID = @(0);

        _applicationID = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCApplicationLauncherClusterApplicationStruct alloc] init];

    other.catalogVendorID = self.catalogVendorID;
    other.applicationID = self.applicationID;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: catalogVendorID:%@; applicationID:%@; >", NSStringFromClass([self class]), _catalogVendorID, _applicationID];
    return descriptionString;
}

- (void)setCatalogVendorId:(NSNumber * _Nonnull)catalogVendorId
{
    self.catalogVendorID = catalogVendorId;
}

- (NSNumber * _Nonnull)catalogVendorId
{
    return self.catalogVendorID;
}

- (void)setApplicationId:(NSString * _Nonnull)applicationId
{
    self.applicationID = applicationId;
}

- (NSString * _Nonnull)applicationId
{
    return self.applicationID;
}

@end

@implementation MCApplicationLauncherClusterApplication : MCApplicationLauncherClusterApplicationStruct
@end

@implementation MCApplicationLauncherClusterApplicationEPStruct
- (instancetype)init
{
    if (self = [super init]) {

        _application = [MCApplicationLauncherClusterApplicationStruct new];

        _endpoint = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCApplicationLauncherClusterApplicationEPStruct alloc] init];

    other.application = self.application;
    other.endpoint = self.endpoint;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: application:%@; endpoint:%@; >", NSStringFromClass([self class]), _application, _endpoint];
    return descriptionString;
}

@end

@implementation MCApplicationLauncherClusterApplicationEP : MCApplicationLauncherClusterApplicationEPStruct
@dynamic application;
@dynamic endpoint;
@end
