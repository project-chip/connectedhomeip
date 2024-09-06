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

#import "MTRTestCase.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRTestCase (ServerAppRunner)

/**
 * Start a server app with the given app name, arguments, and setup payload.
 *
 * The payload will be used to determine the discriminator and passcode
 * arguments the app should use, in addition to the provided arguments.
 * Discriminators should be at least 1111 (see documentation for port below).
 *
 * The --KVS argument for the app will be automatically set to something of the
 * format "/tmp/chip-${APPNAME}-kvs${UNIQUE_ID}".
 *
 * The port argument for the app will be determined automatically, by
 * subtracting 1111 from the discriminator and adding 5542 (so as not to collide
 * with any existing Matter things running on 5540/5541).
 */
- (BOOL)startAppWithName:(NSString *)name arguments:(NSArray<NSString *> *)arguments payload:(NSString *)payload;

/**
 * Same thing, but the server will be killed at the end of the current suite,
 * and is not bound to a particular test in the suite.
 */
+ (BOOL)startAppWithName:(NSString *)name arguments:(NSArray<NSString *> *)arguments payload:(NSString *)payload;

/**
 * Get the unique index that will be used for the next initialization.  This
 * allows including that index in the arguments provided.
 *
 * TODO: Should we scan the provided arguments for %u and replace with the index
 * instead?
 */
+ (unsigned)nextUniqueIndex;

@end

NS_ASSUME_NONNULL_END
