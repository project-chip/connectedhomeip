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

#import "MTRCastingPlayerDiscovery.h"

#import "MTRCastingApp.h"

#import "MTRCastingPlayer.mm"
#import "MTRErrorUtils.h"

#include "core/CastingPlayer.h"
#include "core/CastingPlayerDiscovery.h"
#include "core/Types.h"

#include <platform/CHIPDeviceLayer.h>

#import <Foundation/Foundation.h>

using namespace matter::casting;

/**
 * @brief Singleton that reacts to CastingPlayer discovery results
 */
class MTRDiscoveryDelegateImpl : public matter::casting::core::DiscoveryDelegate {
private:
    MTRDiscoveryDelegateImpl() {};
    static MTRDiscoveryDelegateImpl * _discoveryDelegateImpl;

public:
    static MTRDiscoveryDelegateImpl * GetInstance();
    void HandleOnAdded(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> player) override;
    void HandleOnUpdated(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> player) override;
};

@implementation MTRCastingPlayerDiscovery

NSString * const ADD_CASTING_PLAYER_NOTIFICATION_NAME = @"didAddCastingPlayersNotification";
NSString * const REMOVE_CASTING_PLAYER_NOTIFICATION_NAME = @"didRemoveCastingPlayersNotification";
NSString * const UPDATE_CASTING_PLAYER_NOTIFICATION_NAME = @"didUpdateCastingPlayersNotification";
NSString * const CASTING_PLAYER_KEY = @"castingPlayer";

- init
{
    self = [super init];
    if (self) {
        dispatch_queue_t workQueue = [[MTRCastingApp getSharedInstance] getWorkQueue];
        dispatch_sync(workQueue, ^{
            core::CastingPlayerDiscovery::GetInstance()->SetDelegate(MTRDiscoveryDelegateImpl::GetInstance());
        });
    }
    return self;
}

+ (MTRCastingPlayerDiscovery *)sharedInstance
{
    static MTRCastingPlayerDiscovery * instance = nil;
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
    ChipLogProgress(AppServer, "MTRCastingPlayerDiscovery.start called");
    VerifyOrReturnValue([[MTRCastingApp getSharedInstance] isRunning], [MTRErrorUtils NSErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);

    dispatch_queue_t workQueue = [[MTRCastingApp getSharedInstance] getWorkQueue];
    __block CHIP_ERROR err = CHIP_NO_ERROR;
    dispatch_sync(workQueue, ^{
        err = core::CastingPlayerDiscovery::GetInstance()->StartDiscovery(filterBydeviceType);
    });

    return [MTRErrorUtils NSErrorFromChipError:err];
}

- (NSError *)stop
{
    ChipLogProgress(AppServer, "MTRCastingPlayerDiscovery.stop called");
    VerifyOrReturnValue([[MTRCastingApp getSharedInstance] isRunning], [MTRErrorUtils NSErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);

    dispatch_queue_t workQueue = [[MTRCastingApp getSharedInstance] getWorkQueue];
    __block CHIP_ERROR err = CHIP_NO_ERROR;
    dispatch_sync(workQueue, ^{
        err = core::CastingPlayerDiscovery::GetInstance()->StopDiscovery();
    });

    return [MTRErrorUtils NSErrorFromChipError:err];
}

@end

MTRDiscoveryDelegateImpl * MTRDiscoveryDelegateImpl::_discoveryDelegateImpl = nullptr;

MTRDiscoveryDelegateImpl * MTRDiscoveryDelegateImpl::GetInstance()
{
    if (_discoveryDelegateImpl == nullptr) {
        _discoveryDelegateImpl = new MTRDiscoveryDelegateImpl();
    }
    return _discoveryDelegateImpl;
}

void MTRDiscoveryDelegateImpl::HandleOnAdded(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> castingPlayer)
{
    ChipLogProgress(AppServer, "MTRDiscoveryDelegateImpl::HandleOnAdded called with CastingPlayer ID: %s", castingPlayer->GetId());
    dispatch_queue_t clientQueue = [[MTRCastingApp getSharedInstance] getClientQueue];
    VerifyOrReturn(clientQueue != nil, ChipLogError(AppServer, "MTRDiscoveryDelegateImpl::HandleOnAdded ClientQueue was nil"));
    VerifyOrReturn(castingPlayer != nil, ChipLogError(AppServer, "MTRDiscoveryDelegateImpl::HandleOnAdded Cpp CastingPlayer was nil"));
    dispatch_async(clientQueue, ^{
        NSDictionary * dictionary = @ { CASTING_PLAYER_KEY : [[MTRCastingPlayer alloc] initWithCppCastingPlayer:castingPlayer] };
        [[NSNotificationCenter defaultCenter] postNotificationName:ADD_CASTING_PLAYER_NOTIFICATION_NAME object:nil userInfo:dictionary];
    });
}

void MTRDiscoveryDelegateImpl::HandleOnUpdated(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> castingPlayer)
{
    ChipLogProgress(AppServer, "MTRDiscoveryDelegateImpl::HandleOnUpdated called with CastingPlayer ID: %s", castingPlayer->GetId());
    dispatch_queue_t clientQueue = [[MTRCastingApp getSharedInstance] getClientQueue];
    VerifyOrReturn(clientQueue != nil);
    dispatch_async(clientQueue, ^{
        NSDictionary * dictionary = @ { CASTING_PLAYER_KEY : [[MTRCastingPlayer alloc] initWithCppCastingPlayer:castingPlayer] };
        [[NSNotificationCenter defaultCenter] postNotificationName:UPDATE_CASTING_PLAYER_NOTIFICATION_NAME object:nil userInfo:dictionary];
    });
}
