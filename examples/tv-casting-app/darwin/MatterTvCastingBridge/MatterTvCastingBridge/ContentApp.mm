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
