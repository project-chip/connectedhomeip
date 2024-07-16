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
#import "MCEndpoint.h"
#import <Foundation/Foundation.h>

#ifndef MCCluster_h
#define MCCluster_h

@class MCEndpoint;

@interface MCCluster : NSObject

/**
 * @brief Get the MCEndpoint corresponding to this MCCluster
 */
- (MCEndpoint * _Nonnull)endpoint;

@end

#endif /* MCCluster_h */
