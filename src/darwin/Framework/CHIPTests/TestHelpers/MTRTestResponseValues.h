/**
 *    Copyright (c) 2026 Project CHIP Authors
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
#import <Matter/Matter.h>

NS_ASSUME_NONNULL_BEGIN

// Report values and paths for tests that inject attribute and event reports into an MTRDevice.

// An unsigned integer attribute report whose value and data version are both testValue.
NSDictionary<NSString *, id> * TestAttributeResponseValue(NSNumber * endpointID, NSNumber * clusterID, NSNumber * attributeID, unsigned int testValue);

// An event report with an empty payload. Event numbers come from one counter shared by every caller and start high,
// so in practice they are larger than any event number a device has already seen.
NSDictionary<NSString *, id> * TestEventResponseValue(NSNumber * endpointID, NSNumber * clusterID, NSNumber * eventID);

// Path subclasses for tests that check how a device treats subclasses of the path classes.
@interface MTRInterestedPathsTestClusterPath : MTRClusterPath
@end

@interface MTRInterestedPathsTestAttributePath : MTRAttributePath
@end

@interface MTRInterestedPathsTestEventPath : MTREventPath
@end

// Returns a copy of path whose class is subclass, made by archiving path under subclass's name.
id _Nullable MTRInterestedPathsTestPathAsSubclass(MTRClusterPath * path, Class subclass);

NS_ASSUME_NONNULL_END
