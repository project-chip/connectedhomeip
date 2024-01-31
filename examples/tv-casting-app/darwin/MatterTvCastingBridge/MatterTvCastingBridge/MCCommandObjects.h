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

#import "MCCommand.h"
#import <Foundation/Foundation.h>

#ifndef MCCommandObjects_h
#define MCCommandObjects_h

@interface MCContentLauncherClusterDimensionStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull width;
@property (nonatomic, copy) NSNumber * _Nonnull height;
@property (nonatomic, copy) NSNumber * _Nonnull metric;
@end

@interface MCContentLauncherClusterStyleInformationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nullable imageURL;
@property (nonatomic, copy) NSString * _Nullable imageUrl;
@property (nonatomic, copy) NSString * _Nullable color;
@property (nonatomic, copy) MCContentLauncherClusterDimensionStruct * _Nullable size;
@end

@interface MCContentLauncherClusterBrandingInformationStruct : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull providerName;
@property (nonatomic, copy) MCContentLauncherClusterStyleInformationStruct * _Nullable background;
@property (nonatomic, copy) MCContentLauncherClusterStyleInformationStruct * _Nullable logo;
@property (nonatomic, copy) MCContentLauncherClusterStyleInformationStruct * _Nullable progressBar;
@property (nonatomic, copy) MCContentLauncherClusterStyleInformationStruct * _Nullable splash;
@property (nonatomic, copy) MCContentLauncherClusterStyleInformationStruct * _Nullable waterMark;
@end

@interface MCContentLauncherClusterLaunchURLRequest : NSObject <NSCopying>
@property (nonatomic, copy) NSString * _Nonnull contentURL;
@property (nonatomic, copy) NSString * _Nullable displayString;
@property (nonatomic, copy) MCContentLauncherClusterBrandingInformationStruct * _Nullable brandingInformation;
@end

@interface MCContentLauncherClusterLauncherResponse : NSObject <NSCopying>
@property (nonatomic, copy) NSNumber * _Nonnull status;
@property (nonatomic, copy) NSString * _Nullable data;
@end

@interface MCContentLauncherClusterLaunchURLCommand : MCCommand <MCContentLauncherClusterLaunchURLRequest *, MCContentLauncherClusterLauncherResponse *>
@end

#endif /* MCCommandObjects_h */
