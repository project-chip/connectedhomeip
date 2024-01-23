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

#import "MCCastingPlayerDiscovery.h"

#import "MCCastingApp.h"

#import "MCCastingPlayer.mm"
#import "MCErrorUtils.h"

#include "core/CastingPlayer.h"
#include "core/CastingPlayerDiscovery.h"
#include "core/Types.h"

#include <platform/CHIPDeviceLayer.h>

#import <Foundation/Foundation.h>

using namespace matter::casting;

/**
 * @brief Singleton that reacts to CastingPlayer discovery results
 */
class MCDiscoveryDelegateImpl : public matter::casting::core::DiscoveryDelegate {
private:
    MCDiscoveryDelegateImpl() {};
    static MCDiscoveryDelegateImpl * _discoveryDelegateImpl;

public:
    static MCDiscoveryDelegateImpl * GetInstance();
    void HandleOnAdded(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> player) override;
    void HandleOnUpdated(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> player) override;
};

@implementation MCCastingPlayerDiscovery

NSString * const ADD_CASTING_PLAYER_NOTIFICATION_NAME = @"didAddCastingPlayersNotification";
NSString * const REMOVE_CASTING_PLAYER_NOTIFICATION_NAME = @"didRemoveCastingPlayersNotification";
NSString * const UPDATE_CASTING_PLAYER_NOTIFICATION_NAME = @"didUpdateCastingPlayersNotification";
NSString * const CASTING_PLAYER_KEY = @"castingPlayer";

- init
{
    self = [super init];
    if (self) {
        dispatch_queue_t workQueue = [[MCCastingApp getSharedInstance] getWorkQueue];
        dispatch_sync(workQueue, ^{
            core::CastingPlayerDiscovery::GetInstance()->SetDelegate(MCDiscoveryDelegateImpl::GetInstance());
        });
    }
    return self;
}

+ (MCCastingPlayerDiscovery *)sharedInstance
{
    static MCCastingPlayerDiscovery * instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (NSError *)start
{
    return [self start:0]; // default to filterBydeviceType: 0
}

- (NSError *)start:(const uint32_t)filterBydeviceType
{
    ChipLogProgress(AppServer, "MCCastingPlayerDiscovery.start called");
    VerifyOrReturnValue([[MCCastingApp getSharedInstance] isRunning], [MCErrorUtils NSErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);

    dispatch_queue_t workQueue = [[MCCastingApp getSharedInstance] getWorkQueue];
    __block CHIP_ERROR err = CHIP_NO_ERROR;
    dispatch_sync(workQueue, ^{
        err = core::CastingPlayerDiscovery::GetInstance()->StartDiscovery(filterBydeviceType);
    });

    return [MCErrorUtils NSErrorFromChipError:err];
}

- (NSError *)stop
{
    ChipLogProgress(AppServer, "MCCastingPlayerDiscovery.stop called");
    VerifyOrReturnValue([[MCCastingApp getSharedInstance] isRunning], [MCErrorUtils NSErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);

    dispatch_queue_t workQueue = [[MCCastingApp getSharedInstance] getWorkQueue];
    __block CHIP_ERROR err = CHIP_NO_ERROR;
    dispatch_sync(workQueue, ^{
        err = core::CastingPlayerDiscovery::GetInstance()->StopDiscovery();
    });

    return [MCErrorUtils NSErrorFromChipError:err];
}

@end

MCDiscoveryDelegateImpl * MCDiscoveryDelegateImpl::_discoveryDelegateImpl = nullptr;

MCDiscoveryDelegateImpl * MCDiscoveryDelegateImpl::GetInstance()
{
    if (_discoveryDelegateImpl == nullptr) {
        _discoveryDelegateImpl = new MCDiscoveryDelegateImpl();
    }
    return _discoveryDelegateImpl;
}

void MCDiscoveryDelegateImpl::HandleOnAdded(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> castingPlayer)
{
    ChipLogProgress(AppServer, "MCDiscoveryDelegateImpl::HandleOnAdded called with CastingPlayer ID: %s", castingPlayer->GetId());
    dispatch_queue_t clientQueue = [[MCCastingApp getSharedInstance] getClientQueue];
    VerifyOrReturn(clientQueue != nil, ChipLogError(AppServer, "MCDiscoveryDelegateImpl::HandleOnAdded ClientQueue was nil"));
    VerifyOrReturn(castingPlayer != nil, ChipLogError(AppServer, "MCDiscoveryDelegateImpl::HandleOnAdded Cpp CastingPlayer was nil"));
    dispatch_async(clientQueue, ^{
        NSDictionary * dictionary = @ { CASTING_PLAYER_KEY : [[MCCastingPlayer alloc] initWithCppCastingPlayer:castingPlayer] };
        [[NSNotificationCenter defaultCenter] postNotificationName:ADD_CASTING_PLAYER_NOTIFICATION_NAME object:nil userInfo:dictionary];
    });
}

void MCDiscoveryDelegateImpl::HandleOnUpdated(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> castingPlayer)
{
    ChipLogProgress(AppServer, "MCDiscoveryDelegateImpl::HandleOnUpdated called with CastingPlayer ID: %s", castingPlayer->GetId());
    dispatch_queue_t clientQueue = [[MCCastingApp getSharedInstance] getClientQueue];
    VerifyOrReturn(clientQueue != nil);
    dispatch_async(clientQueue, ^{
        NSDictionary * dictionary = @ { CASTING_PLAYER_KEY : [[MCCastingPlayer alloc] initWithCppCastingPlayer:castingPlayer] };
        [[NSNotificationCenter defaultCenter] postNotificationName:UPDATE_CASTING_PLAYER_NOTIFICATION_NAME object:nil userInfo:dictionary];
    });
}
