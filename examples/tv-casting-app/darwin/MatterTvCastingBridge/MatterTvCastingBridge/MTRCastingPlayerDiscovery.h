/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#import "MTRCastingPlayer.h"
#import "MatterError.h"

#ifndef MTRCastingPlayerDiscovery_h
#define MTRCastingPlayerDiscovery_h

/**
 * MTRCastingPlayerDiscovery sends notification with ADD_CASTING_PLAYER_NOTIFICATION_NAME
 * through the NSNotificationCenter if a new MTRCastingPlayer is added to the network
 */
extern NSString * _Nonnull const ADD_CASTING_PLAYER_NOTIFICATION_NAME;

/**
 * MTRCastingPlayerDiscovery sends notification with REMOVE_CASTING_PLAYER_NOTIFICATION_NAME
 * through the NSNotificationCenter if a MTRCastingPlayer is removed from the network
 */
extern NSString * _Nonnull const REMOVE_CASTING_PLAYER_NOTIFICATION_NAME;

/**
 * MTRCastingPlayerDiscovery sends notification with UPDATE_CASTING_PLAYER_NOTIFICATION_NAME
 * through the NSNotificationCenter if a previously added MTRCastingPlayer is updated
 */
extern NSString * _Nonnull const UPDATE_CASTING_PLAYER_NOTIFICATION_NAME;

/**
 * MTRCastingPlayerDiscovery sends ADD / REMOVE / UPDATE notifications through the
 * NSNotificationCenter with userInfo set to an NSDictionary that has CASTING_PLAYER_KEY as the
 * key to a MTRCastingPlayer object as value.
 */
extern NSString * _Nonnull const CASTING_PLAYER_KEY;

/**
 * @brief MTRCastingPlayerDiscovery is a singleton utility class for discovering MTRCastingPlayers.
 */
@interface MTRCastingPlayerDiscovery : NSObject
+ (MTRCastingPlayerDiscovery * _Nonnull)sharedInstance;

- (nonnull instancetype)init UNAVAILABLE_ATTRIBUTE;
+ (nonnull instancetype)new UNAVAILABLE_ATTRIBUTE;

@property (nonatomic, strong) NSArray<MTRCastingPlayer *> * _Nonnull castingPlayers;

/**
 * @brief Starts the discovery for MTRCastingPlayers
 *
 * @return Returns nil if discovery for CastingPlayers started successfully, NSError * describing the error otherwise.
 */
- (NSError * _Nullable)start;

/**
 * @brief Starts the discovery for MTRCastingPlayers
 *
 * @param filterBydeviceType if passed as a non-zero value, MTRCastingPlayerDiscovery will only discover
 * MTRCastingPlayers whose deviceType matches filterBydeviceType
 * @return Returns nil if discovery for MTRCastingPlayers started successfully, NSError * describing the error otherwise.
 */
- (NSError * _Nullable)start:(const uint32_t)filterBydeviceType;

/**
 * @brief Stop the discovery for MTRCastingPlayers
 *
 * @return Returns nil if discovery for MTRCastingPlayers stopped successfully, NSError * describing the error otherwise.
 */
- (NSError * _Nullable)stop;

@end

#endif /* MTRCastingPlayerDiscovery_h */
