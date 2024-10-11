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

#import "MCInteractionModelStructs.h"

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

@implementation MCContentLauncherClusterContentSearchStruct
- (instancetype)init
{
    if (self = [super init]) {

        _parameterList = [NSArray array];
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCContentLauncherClusterContentSearchStruct alloc] init];

    other.parameterList = self.parameterList;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: parameterList:%@; >", NSStringFromClass([self class]), _parameterList];
    return descriptionString;
}
@end

@implementation MCContentLauncherClusterTrackPreferenceStruct
- (instancetype)init
{
    if (self = [super init]) {

        _languageCode = @"";

        _characteristics = nil;

        _audioOutputIndex = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCContentLauncherClusterTrackPreferenceStruct alloc] init];

    other.languageCode = self.languageCode;
    other.characteristics = self.characteristics;
    other.audioOutputIndex = self.audioOutputIndex;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: languageCode:%@; characteristics:%@; audioOutputIndex:%@; >", NSStringFromClass([self class]), _languageCode, _characteristics, _audioOutputIndex];
    return descriptionString;
}
@end

@implementation MCContentLauncherClusterPlaybackPreferencesStruct
- (instancetype)init
{
    if (self = [super init]) {

        _playbackPosition = @(0);

        _textTrack = [MCContentLauncherClusterTrackPreferenceStruct new];

        _audioTracks = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCContentLauncherClusterPlaybackPreferencesStruct alloc] init];

    other.playbackPosition = self.playbackPosition;
    other.textTrack = self.textTrack;
    other.audioTracks = self.audioTracks;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: playbackPosition:%@; textTrack:%@; audioTracks:%@; >", NSStringFromClass([self class]), _playbackPosition, _textTrack, _audioTracks];
    return descriptionString;
}
@end

@implementation MCContentLauncherClusterDimensionStruct
- (instancetype)init
{
    if (self = [super init]) {

        _width = @(0);

        _height = @(0);

        _metric = @(0);
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCContentLauncherClusterDimensionStruct alloc] init];

    other.width = self.width;
    other.height = self.height;
    other.metric = self.metric;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: width:%@; height:%@; metric:%@; >", NSStringFromClass([self class]), _width, _height, _metric];
    return descriptionString;
}
@end

@implementation MCContentLauncherClusterStyleInformationStruct
- (instancetype)init
{
    if (self = [super init]) {

        _imageURL = nil;

        _color = nil;

        _size = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCContentLauncherClusterStyleInformationStruct alloc] init];

    other.imageURL = self.imageURL;
    other.color = self.color;
    other.size = self.size;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: imageURL:%@; color:%@; size:%@; >", NSStringFromClass([self class]), _imageURL, _color, _size];
    return descriptionString;
}

- (void)setImageUrl:(NSString * _Nullable)imageUrl
{
    self.imageURL = imageUrl;
}

- (NSString * _Nullable)imageUrl
{
    return self.imageURL;
}
@end

@implementation MCContentLauncherClusterBrandingInformationStruct
- (instancetype)init
{
    if (self = [super init]) {

        _providerName = @"";

        _background = nil;

        _logo = nil;

        _progressBar = nil;

        _splash = nil;

        _waterMark = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCContentLauncherClusterBrandingInformationStruct alloc] init];

    other.providerName = self.providerName;
    other.background = self.background;
    other.logo = self.logo;
    other.progressBar = self.progressBar;
    other.splash = self.splash;
    other.waterMark = self.waterMark;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: providerName:%@; background:%@; logo:%@; progressBar:%@; splash:%@; waterMark:%@; >", NSStringFromClass([self class]), _providerName, _background, _logo, _progressBar, _splash, _waterMark];
    return descriptionString;
}
@end

@implementation MCContentLauncherClusterParameterStruct
- (instancetype)init
{
    if (self = [super init]) {

        _type = @(0);

        _value = @"";

        _externalIDList = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCContentLauncherClusterParameterStruct alloc] init];

    other.type = self.type;
    other.value = self.value;
    other.externalIDList = self.externalIDList;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: type:%@; value:%@; externalIDList:%@; >", NSStringFromClass([self class]), _type, _value, _externalIDList];
    return descriptionString;
}
@end

@implementation MCContentLauncherClusterAdditionalInfoStruct
- (instancetype)init
{
    if (self = [super init]) {

        _name = @"";

        _value = @"";
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    auto other = [[MCContentLauncherClusterAdditionalInfoStruct alloc] init];

    other.name = self.name;
    other.value = self.value;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: name:%@; value:%@; >", NSStringFromClass([self class]), _name, _value];
    return descriptionString;
}
@end
