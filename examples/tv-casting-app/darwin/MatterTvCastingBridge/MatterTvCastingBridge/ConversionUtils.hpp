/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

@end

#endif /* ConversionUtils_h */
