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

#import "MCCastingPlayer.h"

#import "MCCastingApp.h"
#import "MCEndpoint_Internal.h"
#import "MCErrorUtils.h"

#import "core/CastingPlayer.h"

#import <Foundation/Foundation.h>

@interface MCCastingPlayer ()

@property (nonatomic, readwrite) matter::casting::memory::Strong<matter::casting::core::CastingPlayer> cppCastingPlayer;

@end

@implementation MCCastingPlayer

static const NSInteger kMinCommissioningWindowTimeoutSec = matter::casting::core::kCommissioningWindowTimeoutSec;

+ (NSInteger)kMinCommissioningWindowTimeoutSec
{
    return kMinCommissioningWindowTimeoutSec;
}

- (void)verifyOrEstablishConnectionWithCompletionBlock:(void (^_Nonnull)(NSError * _Nullable))completion desiredEndpointFilter:(MCEndpointFilter * _Nullable)desiredEndpointFilter
{
    [self verifyOrEstablishConnectionWithCompletionBlock:completion timeout:kMinCommissioningWindowTimeoutSec desiredEndpointFilter:desiredEndpointFilter];
}

- (void)verifyOrEstablishConnectionWithCompletionBlock:(void (^_Nonnull)(NSError * _Nullable))completion timeout:(long long)timeout desiredEndpointFilter:(MCEndpointFilter * _Nullable)desiredEndpointFilter
{
    ChipLogProgress(AppServer, "MCCastingPlayer.verifyOrEstablishConnectionWithCompletionBlock called");
    VerifyOrReturn([[MCCastingApp getSharedInstance] isRunning], ChipLogError(AppServer, "MCCastingApp NOT running"));

    dispatch_queue_t workQueue = [[MCCastingApp getSharedInstance] getWorkQueue];
    dispatch_sync(workQueue, ^{
        __block matter::casting::core::EndpointFilter cppDesiredEndpointFilter;
        if (desiredEndpointFilter != nil) {
            cppDesiredEndpointFilter.vendorId = desiredEndpointFilter.vendorId;
            cppDesiredEndpointFilter.productId = desiredEndpointFilter.productId;
        }

        _cppCastingPlayer->VerifyOrEstablishConnection(
            [completion](CHIP_ERROR err, matter::casting::core::CastingPlayer * castingPlayer) {
                dispatch_queue_t clientQueue = [[MCCastingApp getSharedInstance] getClientQueue];
                dispatch_async(clientQueue, ^{
                    completion(err == CHIP_NO_ERROR ? nil : [MCErrorUtils NSErrorFromChipError:err]);
                });
            }, timeout, cppDesiredEndpointFilter);
    });
}

- (void)disconnect
{
    ChipLogProgress(AppServer, "MCCastingPlayer.disconnect called");
    VerifyOrReturn([[MCCastingApp getSharedInstance] isRunning], ChipLogError(AppServer, "MCCastingApp NOT running"));

    dispatch_queue_t workQueue = [[MCCastingApp getSharedInstance] getWorkQueue];
    dispatch_sync(workQueue, ^{
        _cppCastingPlayer->Disconnect();
    });
}

- (instancetype _Nonnull)initWithCppCastingPlayer:(matter::casting::memory::Strong<matter::casting::core::CastingPlayer>)cppCastingPlayer
{
    if (self = [super init]) {
        _cppCastingPlayer = cppCastingPlayer;
    }
    return self;
}

- (NSString * _Nonnull)description
{
    return [NSString stringWithFormat:@"%@ with Product ID: %hu and Vendor ID: %hu. Resolved IPAddr?: %@",
                     self.deviceName, self.productId, self.vendorId, self.ipAddresses != nil && self.ipAddresses.count > 0 ? @"YES" : @"NO"];
}

- (NSString * _Nonnull)identifier
{
    return [NSString stringWithCString:_cppCastingPlayer->GetId() encoding:NSUTF8StringEncoding];
}

- (NSString * _Nonnull)deviceName
{
    return [NSString stringWithCString:_cppCastingPlayer->GetDeviceName() encoding:NSUTF8StringEncoding];
}

- (uint16_t)productId
{
    return _cppCastingPlayer->GetProductId();
}

- (uint16_t)vendorId
{
    return _cppCastingPlayer->GetVendorId();
}

- (uint32_t)deviceType
{
    return _cppCastingPlayer->GetDeviceType();
}

- (NSArray * _Nonnull)ipAddresses
{
    NSMutableArray * ipAddresses = [NSMutableArray new];
    for (size_t i = 0; i < _cppCastingPlayer->GetNumIPs(); i++) {
        char addrCString[chip::Inet::IPAddress::kMaxStringLength];
        _cppCastingPlayer->GetIPAddresses()[i].ToString(addrCString, chip::Inet::IPAddress::kMaxStringLength);
        ipAddresses[i] = [NSString stringWithCString:addrCString encoding:NSASCIIStringEncoding];
    }
    return ipAddresses;
}

- (NSArray<MCEndpoint *> * _Nonnull)endpoints
{
    NSMutableArray * endpoints = [NSMutableArray new];
    const std::vector<matter::casting::memory::Strong<matter::casting::core::Endpoint>> cppEndpoints = _cppCastingPlayer->GetEndpoints();
    for (matter::casting::memory::Strong<matter::casting::core::Endpoint> cppEndpoint : cppEndpoints) {
        MCEndpoint * endpoint = [[MCEndpoint alloc] initWithCppEndpoint:cppEndpoint];
        [endpoints addObject:endpoint];
    }
    return endpoints;
}

- (BOOL)isEqualToMCCastingPlayer:(MCCastingPlayer * _Nullable)other
{
    return [self.identifier isEqualToString:other.identifier];
}

- (BOOL)isEqual:(id _Nullable)other
{
    if (other == nil) {
        return NO;
    }

    if (self == other) {
        return YES;
    }

    if (![other isKindOfClass:[MCCastingPlayer class]]) {
        return NO;
    }

    return [self isEqualToMCCastingPlayer:(MCCastingPlayer *) other];
}

- (NSUInteger)hash
{
    const NSUInteger prime = 31;
    NSUInteger result = 1;

    result = prime * result + [self.identifier hash];

    return result;
}

@end
