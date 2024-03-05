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

#import <Foundation/Foundation.h>

#ifndef MCStructsObjc_h
#define MCStructsObjc_h

@interface MCMediaPlaybackClusterPlaybackPositionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull updatedAt;
@property (nonatomic, copy) NSNumber * _Nullable position;
@end

@interface MCMediaPlaybackClusterTrackAttributesStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull languageCode;
@property (nonatomic, copy) NSString * _Nullable displayName;
@end

@interface MCMediaPlaybackClusterTrackStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull id;
@property (nonatomic, copy) MCMediaPlaybackClusterTrackAttributesStruct * _Nullable trackAttributes;
@end

@interface MCApplicationBasicClusterApplicationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorID;
@property (nonatomic, copy) NSString * _Nonnull applicationID;
@end

@interface MCTargetNavigatorClusterTargetInfoStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull identifier;
@property (nonatomic, copy) NSString * _Nonnull name;
@end

@interface MCApplicationLauncherClusterApplicationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull catalogVendorID;
@property (nonatomic, copy) NSString * _Nonnull applicationID;
@end

@interface MCApplicationLauncherClusterApplicationEPStruct : NSObject <NSCopying>
@property (nonatomic, copy) MCApplicationLauncherClusterApplicationStruct * _Nonnull application;
@property (nonatomic, copy) NSNumber * _Nullable endpoint;
@end

#endif /* MCStructsObjc_h */
