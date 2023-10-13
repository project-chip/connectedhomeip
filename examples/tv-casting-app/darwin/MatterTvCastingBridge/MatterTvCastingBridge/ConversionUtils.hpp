/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#import "AppParameters.h"
#import "ContentApp.h"
#import "DiscoveredNodeData.h"
#import "VideoPlayer.h"

#import <AppParams.h>
#import <TargetEndpointInfo.h>
#import <TargetVideoPlayerInfo.h>
#include <lib/dnssd/Resolver.h>

#ifndef ConversionUtils_h
#define ConversionUtils_h

@interface ConversionUtils : NSObject
/**
 * @brief Objective C to C++ converters
 */
+ (CHIP_ERROR)convertToCppAppParamsInfoFrom:(AppParameters * _Nonnull)objCAppParameters outAppParams:(AppParams &)outAppParams;

+ (CHIP_ERROR)convertToCppTargetEndpointInfoFrom:(ContentApp * _Nonnull)objCContentApp
                           outTargetEndpointInfo:(TargetEndpointInfo &)outTargetEndpointInfo;

+ (CHIP_ERROR)convertToCppTargetVideoPlayerInfoFrom:(VideoPlayer * _Nonnull)objCVideoPlayer
                           outTargetVideoPlayerInfo:(TargetVideoPlayerInfo &)outTargetVideoPlayerInfo;

+ (CHIP_ERROR)convertToCppDiscoveredNodeDataFrom:(DiscoveredNodeData * _Nonnull)objCDiscoveredNodeData
                           outDiscoveredNodeData:(chip::Dnssd::DiscoveredNodeData &)outDiscoveredNodeData;

/**
 * @brief C++ to Objective C converters
 */
+ (ContentApp * _Nonnull)convertToObjCContentAppFrom:(TargetEndpointInfo * _Nonnull)cppTargetEndpointInfo;

+ (DiscoveredNodeData * _Nonnull)convertToObjCDiscoveredNodeDataFrom:
    (const chip::Dnssd::DiscoveredNodeData * _Nonnull)cppDiscoveredNodedata;

+ (VideoPlayer * _Nonnull)convertToObjCVideoPlayerFrom:(TargetVideoPlayerInfo * _Nonnull)cppTargetVideoPlayerInfo;

/**
 * @brief inter-object converters
 */
+ (DiscoveredNodeData * _Nonnull)convertToDiscoveredNodeDataFrom:(TargetVideoPlayerInfo * _Nonnull)cppTargetVideoPlayerInfo;

@end

#endif /* ConversionUtils_h */
