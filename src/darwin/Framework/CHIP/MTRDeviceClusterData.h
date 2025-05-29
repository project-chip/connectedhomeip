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

#import <Foundation/Foundation.h>

#import "MTRDefines_Internal.h"

NS_ASSUME_NONNULL_BEGIN

/**
 * Information about a cluster: data version and known attribute values.
 */
MTR_TESTABLE
@interface MTRDeviceClusterData : NSObject <NSSecureCoding, NSCopying>
@property (nonatomic, nullable) NSNumber * dataVersion;
@property (nonatomic, nullable) NSNumber * pendingDataVersion; // for holding new data version during a report, and not encoded
@property (nonatomic, readonly) NSDictionary<NSNumber *, MTRDeviceDataValueDictionary> * attributes; // attributeID => data-value dictionary

- (void)storeValue:(MTRDeviceDataValueDictionary _Nullable)value forAttribute:(NSNumber *)attribute;
- (void)removeValueForAttribute:(NSNumber *)attribute;

- (nullable instancetype)initWithDataVersion:(NSNumber * _Nullable)dataVersion attributes:(NSDictionary<NSNumber *, MTRDeviceDataValueDictionary> * _Nullable)attributes;
@end

NS_ASSUME_NONNULL_END
