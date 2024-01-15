/**
 *
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

#import "MCDataSource.h"
#import "MatterError.h"

#ifndef MCCastingApp_h
#define MCCastingApp_h

/**
 * @brief MCCastingApp represents an app that can cast content to a Casting Player.
 */
@interface MCCastingApp : NSObject

/**
 * Returns a shared instance of the MCCastingApp
 */
+ (MCCastingApp * _Nullable)getSharedInstance;

- (dispatch_queue_t _Nullable)getWorkQueue;

- (dispatch_queue_t _Nullable)getClientQueue;

/**
 * @brief Initializes the MCCastingApp with an MCDataSource
 *
 * @param dataSource provides all the parameters required to initialize the MCCastingApp
 */
- (NSError * _Nullable)initializeWithDataSource:(id<MCDataSource> _Nonnull)dataSource;

/**
 * @brief (async) Starts the Matter server that the MCCastingApp runs on and registers all the necessary delegates
 */
- (void)startWithCompletionBlock:(void (^_Nonnull __strong)(NSError * _Nullable __strong))completion;

/**
 * @brief (async) Stops the Matter server that the MCCastingApp runs on
 */
- (void)stopWithCompletionBlock:(void (^_Nonnull __strong)(NSError * _Nullable __strong))completion;

/**
 * @brief true, if MCCastingApp is running. false otherwise
 */
- (bool)isRunning;

/**
 * @brief Tears down all active subscriptions.
 */
- (NSError * _Nullable)ShutdownAllSubscriptions;

/**
 * @brief Clears app cache that contains the information about CastingPlayers previously connected to
 */
- (NSError * _Nullable)ClearCache;

@end

#endif /* MCCastingApp_h */
