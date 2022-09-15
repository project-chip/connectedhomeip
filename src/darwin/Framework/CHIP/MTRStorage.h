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
 * This protocol is used by the Matter framework to read and write storage.
 *
 * All storage methods will be called on a single internal work queue (so the
 * implementation does not need to worry about reads and writes racing).
 */
@protocol MTRStorage <NSObject>
@required

/**
 * Get the data for the given key.  Returns nil if there is no data for the
 * key.
 */
- (NSData * _Nullable)storageDataForKey:(NSString *)key;

/**
 * Set the data for the viven key to the given value.  Returns YES if the key
 * was set successfully, NO otherwise.
 */
- (BOOL)setStorageData:(NSData *)value forKey:(NSString *)key;

/**
 * Delete the key and corresponding data.  Returns YES if the key was present,
 * NO if the key was not present.
 */
- (BOOL)removeStorageDataForKey:(NSString *)key;

@end

NS_ASSUME_NONNULL_END
