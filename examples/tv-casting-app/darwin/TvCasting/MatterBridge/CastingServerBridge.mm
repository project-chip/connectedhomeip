/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#import "CastingServerBridge.h"
#import "CastingServer.h"

#include <lib/support/CHIPMem.h>
#include <platform/PlatformManager.h>

@implementation CastingServerBridge

+ (CastingServerBridge *)getSharedInstance
{
    static CastingServerBridge * instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (instancetype)init
{
    if (self = [super init]) {
        CHIP_ERROR err = chip::Platform::MemoryInit();
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "MemoryInit failed: %s", ErrorStr(err));
            return nil;
        }

        err = chip::DeviceLayer::PlatformMgr().InitChipStack();
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "InitChipStack failed: %s", ErrorStr(err));
            return nil;
        }

        // init app Server
        static chip::CommonCaseDeviceServerInitParams initParams;
        err = initParams.InitializeStaticResourcesBeforeServerInit();
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "InitializeStaticResourcesBeforeServerInit failed: %s", ErrorStr(err));
            return nil;
        }
        err = chip::Server::GetInstance().Init(initParams);
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "chip::Server init failed: %s", ErrorStr(err));
            return nil;
        }

        chip::DeviceLayer::PlatformMgrImpl().StartEventLoopTask();

        CastingServer::GetInstance()->Init();
    }
    return self;
}

- (bool)discoverCommissioners
{
    ChipLogProgress(AppServer, "CastingServerBridge().discoverCommissioners() called");
    CHIP_ERROR err = CastingServer::GetInstance()->DiscoverCommissioners();
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "CastingServerBridge().discoverCommissioners() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }
    return true;
}

- (DiscoveredNodeData *)getDiscoveredCommissioner:(int)index
{
    ChipLogProgress(AppServer, "CastingServerBridge().getDiscoveredCommissioner() called");

    DiscoveredNodeData * commissioner = nil;
    const chip::Dnssd::DiscoveredNodeData * chipDiscoveredNodeData = CastingServer::GetInstance()->GetDiscoveredCommissioner(index);
    if (chipDiscoveredNodeData != nullptr) {
        commissioner = [[DiscoveredNodeData alloc] initWithChipDiscoveredNodeData:(void *) chipDiscoveredNodeData];
    }
    return commissioner;
}

- (bool)sendUserDirectedCommissioningRequest:(NSString *)commissionerIpAddress
                            commissionerPort:(uint16_t)commissionerPort
                           platformInterface:(unsigned int)platformInterface
{
    ChipLogProgress(
        AppServer, "CastingServerBridge().sendUserDirectedCommissioningRequest() called with port %d", commissionerPort);
    chip::Inet::IPAddress commissionerAddrInet;
    if (chip::Inet::IPAddress::FromString([commissionerIpAddress UTF8String], commissionerAddrInet) == false) {
        ChipLogError(AppServer, "CastingServerBridge().sendUserDirectedCommissioningRequest() failed to parse IP address");
        return false;
    }

    chip::Inet::InterfaceId interfaceId = chip::Inet::InterfaceId(platformInterface);

    chip::Transport::PeerAddress commissionerPeerAddress
        = chip::Transport::PeerAddress::UDP(commissionerAddrInet, commissionerPort, interfaceId);

    CHIP_ERROR err = CastingServer::GetInstance()->SendUserDirectedCommissioningRequest(commissionerPeerAddress);
    if (err != CHIP_NO_ERROR) {
        ChipLogError(
            AppServer, "CastingServerBridge().sendUserDirectedCommissioningRequest() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }
    return true;
}
@end
