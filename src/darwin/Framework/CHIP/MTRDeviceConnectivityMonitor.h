/**
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "MTRDefines_Internal.h"

NS_ASSUME_NONNULL_BEGIN

typedef void (^MTRDeviceConnectivityMonitorHandler)(void);

/**
 * Class that a matter dns-sd instance name, and monitors connectivity to the device.
 */
MTR_TESTABLE
@interface MTRDeviceConnectivityMonitor : NSObject
- (instancetype)initWithCompressedFabricID:(NSNumber *)compressedFabricID nodeID:(NSNumber *)nodeID;

/**
 * Any time a path becomes satisfied or route becomes viable, the registered handler will be called.
 */
- (void)startMonitoringWithHandler:(MTRDeviceConnectivityMonitorHandler)handler queue:(dispatch_queue_t)queue;

/**
 * Stops the monitoring. After this method returns no more calls to the handler will be made.
 */
- (void)stopMonitoring;
@end

NS_ASSUME_NONNULL_END
