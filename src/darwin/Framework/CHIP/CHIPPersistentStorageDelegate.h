/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

NS_ASSUME_NONNULL_BEGIN

/**
 * The protocol definition for the CHIPPersistentStorageDelegate
 * Storage is expected to be split across different CompressedFabricIds.
 * CHIP stores most of its operational data inside a "Fabric" scope with a handful
 * of metadata items in the "global" scope
 *
 * A Compressed FabricId of "0" indicates the "global" scope for storage.
 *
 * All delegate methods will be called on the supplied Delegate Queue.
 */
@protocol CHIPPersistentStorageDelegate <NSObject>
@required

/**
 * Get the value for the given key
 *
 */
- (nullable NSString *)CHIPGetKeyValueForFabric:(NSNumber *)CompressedFabricId key:(NSString *)key;

/**
 * Set the value of the key to the given value
 *
 */
- (void)CHIPSetKeyValueForFabric:(NSNumber *)CompressedFabricId key:(NSString *)key value:(NSString *)value;

/**
 * Delete the key and corresponding value
 *
 */
- (void)CHIPDeleteKeyValueForFabric:(NSNumber *)CompressedFabricId key:(NSString *)key;

@end

NS_ASSUME_NONNULL_END
