/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#import <Matter/MTREndpointInfo.h>

#import "MTRDefines_Internal.h"

NS_ASSUME_NONNULL_BEGIN

MTR_TESTABLE_DIRECT_MEMBERS
@interface MTREndpointInfo ()

- (instancetype)initWithEndpointID:(NSNumber *)endpointID
                       deviceTypes:(NSArray<MTRDeviceTypeRevision *> *)deviceTypes
                         partsList:(NSArray<NSNumber *> *)partsList;

// Populates the children array for each endpoint in the provided dictionary.
// Returns YES if the endpoint hierarchy was populated correctly.
// A return value of NO indicates that there were some issues, but
// an effort has been made to populate a valid subset of the hierarchy.
+ (BOOL)populateChildrenForEndpoints:(NSDictionary<NSNumber *, MTREndpointInfo *> *)endpoints;

@end

NS_ASSUME_NONNULL_END
