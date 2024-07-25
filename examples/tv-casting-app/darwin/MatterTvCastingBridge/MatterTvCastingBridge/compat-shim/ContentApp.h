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

#import "../MCEndpoint.h"

#ifndef ContentApp_h
#define ContentApp_h

__attribute__((deprecated("Use the APIs described in /examples/tv-casting-app/APIs.md instead.")))
@interface ContentApp : NSObject

@property uint16_t endpointId;

@property NSMutableArray * clusterIds;

/**
 @brief true, if all the fields are initialized, false otherwise
 */
@property BOOL isInitialized;

- (instancetype)initWithEndpointId:(uint16_t)endpointId clusterIds:(NSMutableArray *)clusterIds;

- (instancetype)initWithEndpoint:(MCEndpoint *)endpoint;

- (BOOL)supportsClusterWithId:(uint32_t)clusterId;

- (BOOL)supportsApplicationLauncher;

- (BOOL)supportsContentLauncher;

- (BOOL)supportsMediaPlayback;

- (BOOL)supportsLevelControl;

- (BOOL)supportsTargetNavigator;

@end

#endif /* ContentApp_h */
