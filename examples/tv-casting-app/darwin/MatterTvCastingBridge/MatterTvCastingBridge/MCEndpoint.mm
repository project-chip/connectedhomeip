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

#import "MCEndpoint_Internal.h"

#import "MCCastingPlayer_Internal.h"
#import "MCCluster_Internal.h"
#import "MCDeviceTypeStruct.h"

#import "zap-generated/MCClusterObjects.h"

#import "clusters/Clusters.h"
#import "core/Endpoint.h"

#import <Foundation/Foundation.h>

@interface MCEndpoint ()

@property (nonatomic, readwrite) matter::casting::memory::Strong<matter::casting::core::Endpoint> cppEndpoint;

@end

@implementation MCEndpoint

- (instancetype _Nonnull)initWithCppEndpoint:(matter::casting::memory::Strong<matter::casting::core::Endpoint>)cppEndpoint
{
    if (self = [super init]) {
        _cppEndpoint = cppEndpoint;
    }
    return self;
}

- (NSMutableArray *)getServerList
{
    NSMutableArray * serverList = [NSMutableArray new];
    if (_cppEndpoint != nil) {
        std::vector<chip::ClusterId> cppServerList = _cppEndpoint->GetServerList();
        for (chip::ClusterId clusterId : cppServerList) {
            [serverList addObject:@(clusterId)];
        }
    }
    return serverList;
}

- (NSNumber * _Nonnull)identifier
{
    return [NSNumber numberWithUnsignedShort:_cppEndpoint->GetId()];
}

- (NSNumber * _Nonnull)productId
{
    return [NSNumber numberWithUnsignedShort:_cppEndpoint->GetProductId()];
}

- (NSNumber * _Nonnull)vendorId
{
    return [NSNumber numberWithUnsignedShort:_cppEndpoint->GetVendorId()];
}

- (NSArray * _Nonnull)deviceTypeList
{
    NSMutableArray * deviceTypeList = [NSMutableArray new];
    std::vector<chip::app::Clusters::Descriptor::Structs::DeviceTypeStruct::DecodableType> cppDeviceTypeList = _cppEndpoint->GetDeviceTypeList();
    for (chip::app::Clusters::Descriptor::Structs::DeviceTypeStruct::DecodableType cppDeviceTypeStruct : cppDeviceTypeList) {
        MCDeviceTypeStruct * deviceTypeStruct = [[MCDeviceTypeStruct alloc] initWithDeviceType:cppDeviceTypeStruct.deviceType revision:cppDeviceTypeStruct.revision];
        [deviceTypeList addObject:deviceTypeStruct];
    }
    return deviceTypeList;
}

- (MCCastingPlayer * _Nonnull)castingPlayer
{
    return [[MCCastingPlayer alloc] initWithCppCastingPlayer:std::shared_ptr<matter::casting::core::CastingPlayer>(_cppEndpoint->GetCastingPlayer())];
}

- (MCCluster * _Nullable)clusterForType:(MCEndpointClusterType)type
{
    switch (type) {
    case MCEndpointClusterTypeApplicationBasic:
        return [[MCApplicationBasicCluster alloc] initWithCppCluster:_cppEndpoint->GetCluster<matter::casting::clusters::application_basic::ApplicationBasicCluster>()];

    case MCEndpointClusterTypeApplicationLauncher:
        return [[MCApplicationLauncherCluster alloc] initWithCppCluster:_cppEndpoint->GetCluster<matter::casting::clusters::application_launcher::ApplicationLauncherCluster>()];

    case MCEndpointClusterTypeContentLauncher:
        return [[MCContentLauncherCluster alloc] initWithCppCluster:_cppEndpoint->GetCluster<matter::casting::clusters::content_launcher::ContentLauncherCluster>()];

    case MCEndpointClusterTypeKeypadInput:
        return [[MCKeypadInputCluster alloc] initWithCppCluster:_cppEndpoint->GetCluster<matter::casting::clusters::keypad_input::KeypadInputCluster>()];

    case MCEndpointClusterTypeMediaPlayback:
        return [[MCMediaPlaybackCluster alloc] initWithCppCluster:_cppEndpoint->GetCluster<matter::casting::clusters::media_playback::MediaPlaybackCluster>()];

    case MCEndpointClusterTypeOnOff:
        return [[MCOnOffCluster alloc] initWithCppCluster:_cppEndpoint->GetCluster<matter::casting::clusters::on_off::OnOffCluster>()];

    case MCEndpointClusterTypeTargetNavigator:
        return [[MCTargetNavigatorCluster alloc] initWithCppCluster:_cppEndpoint->GetCluster<matter::casting::clusters::target_navigator::TargetNavigatorCluster>()];

    default:
        ChipLogError(AppServer, "MCEndpointClusterType not found");
        break;
    }
    return nil;
}

- (BOOL)hasCluster:(MCEndpointClusterType)type
{
    return [self clusterForType:type] != nil;
}

- (BOOL)isEqualToMCEndpoint:(MCEndpoint * _Nullable)other
{
    return [self.identifier isEqualToNumber:other.identifier];
}

- (BOOL)isEqual:(id _Nullable)other
{
    if (other == nil) {
        return NO;
    }

    if (self == other) {
        return YES;
    }

    if (![other isKindOfClass:[MCEndpoint class]]) {
        return NO;
    }

    return [self isEqualToMCEndpoint:(MCEndpoint *) other];
}

- (NSUInteger)hash
{
    const NSUInteger prime = 31;
    NSUInteger result = 1;

    result = prime * result + [self.identifier hash];

    return result;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MCEndpoint: identifier=%@, vendorId=%@, productId=%@>", [self identifier], [self vendorId], [self productId]];
}

@end
