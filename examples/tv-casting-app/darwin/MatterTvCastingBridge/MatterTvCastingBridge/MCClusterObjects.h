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

#import "MCAttributeObjects.h"
#import "MCCluster.h"
#import "MCCommandObjects.h"
#import <Foundation/Foundation.h>

#ifndef MCClusterObjects_h
#define MCClusterObjects_h

@interface MCContentLauncherCluster : MCCluster
/**
 * @brief Returns non-nil pointer to MCContentLauncherClusterLaunchURLCommand if supported, nil otherwise
 */
- (MCContentLauncherClusterLaunchURLCommand * _Nullable)launchURLCommand;
@end

@interface MCApplicationBasicCluster : MCCluster
/**
 * @brief Returns non-nil pointer to MCApplicationBasicClusterVendorIDAttribute if supported, nil otherwise
 */
- (MCApplicationBasicClusterVendorIDAttribute * _Nullable)vendorIDAttribute;
@end

@interface MCMediaPlaybackCluster : MCCluster
/**
 * @brief Returns non-nil pointer to MCMediaPlaybackClusterCurrentStateAttribute if supported, nil otherwise
 */
- (MCMediaPlaybackClusterCurrentStateAttribute * _Nullable)currentStateAttribute;
@end

#endif /* MCClusterObjects_h */
