/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "ContentApp.h"

#include <app-common/zap-generated/ids/Clusters.h>

@implementation ContentApp

- (instancetype)init
{
    if (self = [super init]) {
        _isInitialized = false;
    }
    return self;
}

- (instancetype)initWithEndpointId:(uint16_t)endpointId clusterIds:(NSMutableArray *)clusterIds
{
    if (self = [super init]) {
        _endpointId = endpointId;
        _clusterIds = clusterIds;
        _isInitialized = true;
    }
    return self;
}

- (BOOL)supportsClusterWithId:(uint32_t)clusterId
{
    if (_clusterIds != nil) {
        for (NSNumber * clusterIdNSNumber in _clusterIds) {
            if ([clusterIdNSNumber unsignedIntValue] == clusterId) {
                return true;
            }
        }
    }
    return false;
}

- (BOOL)supportsApplicationLauncher
{
    return [self supportsClusterWithId:chip::app::Clusters::ApplicationLauncher::Id];
}

- (BOOL)supportsContentLauncher
{
    return [self supportsClusterWithId:chip::app::Clusters::ContentLauncher::Id];
}

- (BOOL)supportsMediaPlayback
{
    return [self supportsClusterWithId:chip::app::Clusters::MediaPlayback::Id];
}

- (BOOL)supportsLevelControl
{
    return [self supportsClusterWithId:chip::app::Clusters::LevelControl::Id];
}

- (BOOL)supportsTargetNavigator
{
    return [self supportsClusterWithId:chip::app::Clusters::TargetNavigator::Id];
}
@end
