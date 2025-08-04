/**
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

#import "CastingPlayerDiscoveryListenerCompat.h"

#import "DiscoveredNodeData.h"
#import "MCCastingPlayer.h"
#import "MCCastingPlayerDiscovery.h"
#import "MCErrorUtils.h"

#include <lib/support/logging/CHIPLogging.h>

@interface CastingPlayerDiscoveryListenerCompat ()

@property (class, nonatomic, strong) dispatch_queue_t clientQueue;
@property (class, nonatomic, copy, nullable) void (^discoveredCommissionerHandler)(DiscoveredNodeData * _Nullable);

+ (void)didAddDiscoveredCastingPlayers:(NSNotification * _Nonnull)notification;
+ (void)didUpdateDiscoveredCastingPlayers:(NSNotification * _Nonnull)notification;

@end

@implementation CastingPlayerDiscoveryListenerCompat

static dispatch_queue_t _clientQueue;
static void (^_discoveredCommissionerHandler)(DiscoveredNodeData * _Nullable);

+ (dispatch_queue_t)clientQueue
{
    return _clientQueue;
}

+ (void)setClientQueue:(dispatch_queue_t)clientQueue
{
    _clientQueue = clientQueue;
}

+ (void (^)(DiscoveredNodeData * _Nullable))discoveredCommissionerHandler
{
    return _discoveredCommissionerHandler;
}

+ (void)setDiscoveredCommissionerHandler:(void (^)(DiscoveredNodeData * _Nullable))discoveredCommissionerHandler
{
    _discoveredCommissionerHandler = discoveredCommissionerHandler;
}

+ (void)addObservers:(dispatch_queue_t _Nonnull)clientQueue discoveredCommissionerHandler:(nullable void (^)(DiscoveredNodeData * _Nonnull __strong))discoveredCommissionerHandler
{
    ChipLogProgress(AppServer, "CastingPlayerDiscoveryListenerCompat.addObservers() called");
    [self setClientQueue:clientQueue];
    [self setDiscoveredCommissionerHandler:discoveredCommissionerHandler];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(didAddDiscoveredCastingPlayers:)
                                                 name:ADD_CASTING_PLAYER_NOTIFICATION_NAME
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(didUpdateDiscoveredCastingPlayers:)
                                                 name:UPDATE_CASTING_PLAYER_NOTIFICATION_NAME
                                               object:nil];
}

+ (void)removeObservers
{
    ChipLogProgress(AppServer, "CastingPlayerDiscoveryListenerCompat.removeObservers() called");
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:ADD_CASTING_PLAYER_NOTIFICATION_NAME
                                                  object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:UPDATE_CASTING_PLAYER_NOTIFICATION_NAME
                                                  object:nil];
}

+ (void)didAddDiscoveredCastingPlayers:(NSNotification *)notification
{
    ChipLogProgress(AppServer, "CastingPlayerDiscoveryListenerCompat.didAddDiscoveredCastingPlayers() called");
    VerifyOrReturn(CastingPlayerDiscoveryListenerCompat.clientQueue != nil);
    VerifyOrReturn(CastingPlayerDiscoveryListenerCompat.discoveredCommissionerHandler != nil);

    NSDictionary * userInfo = notification.userInfo;
    MCCastingPlayer * castingPlayer = userInfo[CASTING_PLAYER_KEY];

    if (![castingPlayer isKindOfClass:[MCCastingPlayer class]]) {
        ChipLogError(AppServer, "CastingPlayerDiscoveryListenerCompat.didAddDiscoveredCastingPlayers called with no MCCastingPlayer");
        return;
    }

    ChipLogProgress(AppServer, "CastingPlayerDiscoveryListenerCompat.didAddDiscoveredCastingPlayers notified of a MCCastingPlayer with ID: %@", [castingPlayer identifier]);

    DiscoveredNodeData * discoveredNodeData = [[DiscoveredNodeData alloc] initWithCastingPlayer:castingPlayer];
    dispatch_async(CastingPlayerDiscoveryListenerCompat.clientQueue, ^{
        CastingPlayerDiscoveryListenerCompat.discoveredCommissionerHandler(discoveredNodeData);
    });
}

+ (void)didUpdateDiscoveredCastingPlayers:(NSNotification *)notification
{
    ChipLogProgress(AppServer, "CastingPlayerDiscoveryListenerCompat.didUpdateDiscoveredCastingPlayers() called");
    VerifyOrReturn(CastingPlayerDiscoveryListenerCompat.clientQueue != nil);
    VerifyOrReturn(CastingPlayerDiscoveryListenerCompat.discoveredCommissionerHandler != nil);

    NSDictionary * userInfo = notification.userInfo;
    MCCastingPlayer * castingPlayer = userInfo[CASTING_PLAYER_KEY];

    if (![castingPlayer isKindOfClass:[MCCastingPlayer class]]) {
        ChipLogError(AppServer, "CastingPlayerDiscoveryListenerCompat.didUpdateDiscoveredCastingPlayers called with no MCCastingPlayer");
        return;
    }

    ChipLogProgress(AppServer, "CastingPlayerDiscoveryListenerCompat.didUpdateDiscoveredCastingPlayers notified of a MCCastingPlayer with ID: %@", [castingPlayer identifier]);

    DiscoveredNodeData * discoveredNodeData = [[DiscoveredNodeData alloc] initWithCastingPlayer:castingPlayer];
    dispatch_async(CastingPlayerDiscoveryListenerCompat.clientQueue, ^{
        CastingPlayerDiscoveryListenerCompat.discoveredCommissionerHandler(discoveredNodeData);
    });
}

@end
