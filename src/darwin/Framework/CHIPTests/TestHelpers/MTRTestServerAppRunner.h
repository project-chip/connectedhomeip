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

@class MTRTestCase;

NS_ASSUME_NONNULL_BEGIN

/**
 * A representation of a server application instance.
 *
 * Server applications are assumed to be compiled into out/debug/${APPNAME}-app,
 * with the binary being out/debug/${APPNAME}-app/chip-${APPNAME}-app.
 */
@interface MTRTestServerAppRunner : NSObject

/**
 * Initialize the app runner with the given app name, arguments, setup payload, and testcase
 * instance.
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
- (instancetype)initWithAppName:(NSString *)name arguments:(NSArray<NSString *> *)arguments payload:(NSString *)payload testcase:(MTRTestCase *)testcase;

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
