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
#import "MCCastingApp_Internal.h"
#import "MCCommissionerDeclaration_Internal.h"
#import "MCConnectionCallbacks.h"
#import "MCEndpoint_Internal.h"
#import "MCErrorUtils.h"
#import "MCIdentificationDeclarationOptions_Internal.h"

#import "core/CastingPlayer.h" // from tv-casting-common
#import "core/ConnectionCallbacks.h" // from tv-casting-common
#import "core/IdentificationDeclarationOptions.h" // from tv-casting-common

#import <Foundation/Foundation.h>

@interface MCCastingPlayer ()

@property (nonatomic, readwrite) matter::casting::memory::Strong<matter::casting::core::CastingPlayer> cppCastingPlayer;

- (matter::casting::core::IdentificationDeclarationOptions)setupCppIdOptions:(MCIdentificationDeclarationOptions * _Nullable)identificationDeclarationOptions;

@end

@implementation MCCastingPlayer

static const NSInteger kMinCommissioningWindowTimeoutSec = matter::casting::core::kCommissioningWindowTimeoutSec;

+ (NSInteger)kMinCommissioningWindowTimeoutSec
{
    return kMinCommissioningWindowTimeoutSec;
}

- (NSError *)verifyOrEstablishConnectionWithCallbacks:(MCConnectionCallbacks * _Nonnull)connectionCallbacks
{
    ChipLogProgress(AppServer, "MCCastingPlayer.verifyOrEstablishConnectionWithCallbacks() called, MCConnectionCallbacks parameter only");
    return [self verifyOrEstablishConnectionWithCallbacks:connectionCallbacks
                                                  timeout:kMinCommissioningWindowTimeoutSec
                         identificationDeclarationOptions:nil];
}

- (NSError *)verifyOrEstablishConnectionWithCallbacks:(MCConnectionCallbacks * _Nonnull)connectionCallbacks
                     identificationDeclarationOptions:(MCIdentificationDeclarationOptions * _Nullable)identificationDeclarationOptions
{
    ChipLogProgress(AppServer, "MCCastingPlayer.verifyOrEstablishConnectionWithCallbacks() called, MCConnectionCallbacks and MCIdentificationDeclarationOptions parameters");
    return [self verifyOrEstablishConnectionWithCallbacks:connectionCallbacks
                                                  timeout:kMinCommissioningWindowTimeoutSec
                         identificationDeclarationOptions:identificationDeclarationOptions];
}

- (NSError *)verifyOrEstablishConnectionWithCallbacks:(MCConnectionCallbacks * _Nonnull)connectionCallbacks
                                              timeout:(long)timeout
                     identificationDeclarationOptions:(MCIdentificationDeclarationOptions * _Nullable)identificationDeclarationOptions
{
    ChipLogProgress(AppServer, "MCCastingPlayer.verifyOrEstablishConnectionWithCallbacks() called, MCConnectionCallbacks, timeout and MCIdentificationDeclarationOptions parameters");
    VerifyOrReturnValue([[MCCastingApp getSharedInstance] isRunning],
        [MCErrorUtils NSErrorFromChipError:CHIP_ERROR_INCORRECT_STATE],
        ChipLogError(AppServer, "MCCastingPlayer.verifyOrEstablishConnectionWithCallbacks() MCCastingApp NOT running"));

    dispatch_queue_t workQueue = [[MCCastingApp getSharedInstance] getWorkQueue];
    dispatch_sync(workQueue, ^{
        matter::casting::core::IdentificationDeclarationOptions cppIdOptions = [self setupCppIdOptions:identificationDeclarationOptions];

        // Handles the connection complete event and calls the MCConnectionCallbacks connectionCompleteCallback callback provided by
        // the Swift client. This callback is called by the cpp layer when the connection process has ended, regardless of whether it
        // was successful or not.
        void (^connectCallback)(CHIP_ERROR, matter::casting::core::CastingPlayer *) = ^(CHIP_ERROR err, matter::casting::core::CastingPlayer * castingPlayer) {
            ChipLogProgress(AppServer, "MCCastingPlayer.verifyOrEstablishConnectionWithCallbacks() connectCallback() called");
            dispatch_queue_t clientQueue = [[MCCastingApp getSharedInstance] getClientQueue];
            dispatch_async(clientQueue, ^{
                if (connectionCallbacks.connectionCompleteCallback) {
                    connectionCallbacks.connectionCompleteCallback(err == CHIP_NO_ERROR ? nil : [MCErrorUtils NSErrorFromChipError:err]);
                } else {
                    ChipLogError(AppServer, "MCCastingPlayer.verifyOrEstablishConnectionWithCallbacks() connectCallback(), client failed to set the connectionCompleteCallback() callback");
                }
            });
        };
        // Handles the Commissioner Declaration event and calls the MCConnectionCallbacks commissionerDeclarationCallback callback
        // provided by the Swift client. This callback is called by the cpp layer when the Commissionee receives a
        // CommissionerDeclaration message from the CastingPlayer/Commissioner.
        void (^commissionerDeclarationCallback)(const chip::Transport::PeerAddress & source, const chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration cppCommissionerDeclaration) = ^(const chip::Transport::PeerAddress &
                                                                                                                                                                                                           source,
            const chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration cppCommissionerDeclaration) {
            ChipLogProgress(AppServer, "MCCastingPlayer.verifyOrEstablishConnectionWithCallbacks() commissionerDeclarationCallback() called with cpp CommissionerDeclaration message");
            dispatch_queue_t clientQueue = [[MCCastingApp getSharedInstance] getClientQueue];
            dispatch_async(clientQueue, ^{
                if (connectionCallbacks.commissionerDeclarationCallback) {
                    // convert cppCommissionerDeclaration to a shared_ptr<CommissionerDeclaration> and pass it to the client callback
                    auto cppCommissionerDeclarationPtr = std::make_shared<chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration>(cppCommissionerDeclaration);
                    MCCommissionerDeclaration * objcCommissionerDeclaration = [[MCCommissionerDeclaration alloc]
                        initWithCppCommissionerDeclaration:cppCommissionerDeclarationPtr];
                    connectionCallbacks.commissionerDeclarationCallback(objcCommissionerDeclaration);
                } else {
                    ChipLogError(AppServer, "MCCastingPlayer.verifyOrEstablishConnectionWithCallbacks() commissionerDeclarationCallback(), client failed to set the optional commissionerDeclarationCallback() callback");
                }
            });
        };

        matter::casting::core::ConnectionCallbacks cppConnectionCallbacks;
        cppConnectionCallbacks.mOnConnectionComplete = connectCallback;
        if (connectionCallbacks.commissionerDeclarationCallback) {
            cppConnectionCallbacks.mCommissionerDeclarationCallback = commissionerDeclarationCallback;
        } else {
            ChipLogProgress(AppServer, "MCCastingPlayer.verifyOrEstablishConnectionWithCallbacks(), client did not set the optional commissionerDeclarationCallback()");
        }

        ChipLogProgress(AppServer, "MCCastingPlayer.verifyOrEstablishConnectionWithCallbacks() calling cpp CastingPlayer.VerifyOrEstablishConnection()");
        _cppCastingPlayer->VerifyOrEstablishConnection(cppConnectionCallbacks, timeout, cppIdOptions);
    });
    return nil;
}

- (matter::casting::core::IdentificationDeclarationOptions)setupCppIdOptions:(MCIdentificationDeclarationOptions * _Nullable)identificationDeclarationOptions
{
    matter::casting::core::IdentificationDeclarationOptions cppIdOptions;
    if (identificationDeclarationOptions != nil) {
        cppIdOptions = [identificationDeclarationOptions getCppIdentificationDeclarationOptions];
    } else {
        ChipLogProgress(AppServer, "MCCastingPlayer.setupCppIdOptions() Client did not set the optional MCIdentificationDeclarationOptions using default options");
    }
    return cppIdOptions;
}

- (NSError *)continueConnecting
{
    ChipLogProgress(AppServer, "MCCastingPlayer.continueConnecting() called");
    VerifyOrReturnValue([[MCCastingApp getSharedInstance] isRunning], [MCErrorUtils NSErrorFromChipError:CHIP_ERROR_INCORRECT_STATE],
        ChipLogError(AppServer, "MCCastingPlayer.continueConnecting() MCCastingApp NOT running"));

    ChipLogProgress(AppServer, "MCCastingPlayer.continueConnecting() calling MCCastingApp.updateCommissionableDataProvider()");
    NSError * updateError = [[MCCastingApp getSharedInstance] updateCommissionableDataProvider];
    VerifyOrReturnValue(updateError == nil, updateError, ChipLogError(AppServer, "MCCastingPlayer.continueConnecting() call to updateCommissionableDataProvider() failed with error: %@", updateError));

    __block CHIP_ERROR err = CHIP_NO_ERROR;
    dispatch_queue_t workQueue = [[MCCastingApp getSharedInstance] getWorkQueue];
    dispatch_sync(workQueue, ^{
        err = _cppCastingPlayer->ContinueConnecting();
    });
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "MCCastingPlayer.continueConnecting() call to cppCastingPlayer->ContinueConnecting() failed due to %" CHIP_ERROR_FORMAT,
            err.Format());
        return [MCErrorUtils NSErrorFromChipError:err];
    }
    return nil;
}

- (NSError *)stopConnecting
{
    ChipLogProgress(AppServer, "MCCastingPlayer.stopConnecting() called");
    VerifyOrReturnValue([[MCCastingApp getSharedInstance] isRunning], [MCErrorUtils NSErrorFromChipError:CHIP_ERROR_INCORRECT_STATE], ChipLogError(AppServer, "MCCastingPlayer.stopConnecting() MCCastingApp NOT running"));

    __block CHIP_ERROR err = CHIP_NO_ERROR;
    dispatch_queue_t workQueue = [[MCCastingApp getSharedInstance] getWorkQueue];
    dispatch_sync(workQueue, ^{
        err = _cppCastingPlayer->StopConnecting();
    });
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "MCCastingPlayer.continueConnecting() call to cppCastingPlayer->StopConnecting() failed due to %" CHIP_ERROR_FORMAT,
            err.Format());
        return [MCErrorUtils NSErrorFromChipError:err];
    }
    return nil;
}

- (void)disconnect
{
    ChipLogProgress(AppServer, "MCCastingPlayer.disconnect() called");
    VerifyOrReturn([[MCCastingApp getSharedInstance] isRunning], ChipLogError(AppServer, "MCCastingPlayer.disconnect() MCCastingApp NOT running"));

    dispatch_queue_t workQueue = [[MCCastingApp getSharedInstance] getWorkQueue];
    dispatch_sync(workQueue, ^{
        _cppCastingPlayer->Disconnect();
    });
}

- (NSError * _Nullable)getConnectionState:(MCCastingPlayerConnectionState * _Nonnull)state;
{
    ChipLogProgress(AppServer, "MCCastingPlayer.getConnectionState() called");
    VerifyOrReturnValue([[MCCastingApp getSharedInstance] isRunning], [MCErrorUtils NSErrorFromChipError:CHIP_ERROR_INCORRECT_STATE], ChipLogError(AppServer, "MCCastingPlayer.getConnectionState() MCCastingApp NOT running"));

    __block matter::casting::core::ConnectionState native_state = matter::casting::core::ConnectionState::CASTING_PLAYER_NOT_CONNECTED;
    dispatch_queue_t workQueue = [[MCCastingApp getSharedInstance] getWorkQueue];
    dispatch_sync(workQueue, ^{
        native_state = _cppCastingPlayer->GetConnectionState();
    });

    switch (native_state) {
    case matter::casting::core::ConnectionState::CASTING_PLAYER_NOT_CONNECTED:
        *state = MC_CASTING_PLAYER_NOT_CONNECTED;
        break;
    case matter::casting::core::ConnectionState::CASTING_PLAYER_CONNECTING:
        *state = MC_CASTING_PLAYER_CONNECTING;
        break;
    case matter::casting::core::ConnectionState::CASTING_PLAYER_CONNECTED:
        *state = MC_CASTING_PLAYER_CONNECTED;
        break;
    default:
        [NSException raise:@"Unhandled matter::casting::core::ConnectionState" format:@"%d is not handled", native_state];
        break;
    }

    return nil;
}

- (instancetype _Nonnull)initWithCppCastingPlayer:(matter::casting::memory::Strong<matter::casting::core::CastingPlayer>)cppCastingPlayer
{
    if (self = [super init]) {
        _cppCastingPlayer = cppCastingPlayer;
    }
    return self;
}

+ (MCCastingPlayer * _Nullable)getTargetCastingPlayer
{
    ChipLogProgress(AppServer, "MCCastingPlayer.getTargetCastingPlayer called");
    VerifyOrReturnValue([[MCCastingApp getSharedInstance] isRunning], nil, ChipLogError(AppServer, "MCCastingApp NOT running"));
    __block MCCastingPlayer * castingPlayer = nil;
    dispatch_sync([[MCCastingApp getSharedInstance] getWorkQueue], ^{
        matter::casting::core::CastingPlayer * cppCastingPlayer = matter::casting::core::CastingPlayer::GetTargetCastingPlayer();
        if (cppCastingPlayer != nullptr) {
            castingPlayer = [[MCCastingPlayer alloc] initWithCppCastingPlayer:std::make_shared<matter::casting::core::CastingPlayer>(*cppCastingPlayer)];
        }
    });
    return castingPlayer;
}

- (NSString * _Nonnull)description
{
    return [NSString stringWithFormat:@"%@ with Product ID: %hu and Vendor ID: %hu. Resolved IPAddr?: %@. Supports Commissioner-Generated Passcode?: %@.",
                     self.deviceName, self.productId, self.vendorId, self.ipAddresses != nil && self.ipAddresses.count > 0 ? @"YES" : @"NO",
                     self.supportsCommissionerGeneratedPasscode ? @"YES" : @"NO"];
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

- (bool)supportsCommissionerGeneratedPasscode
{
    return _cppCastingPlayer->GetSupportsCommissionerGeneratedPasscode();
}

- (NSString * _Nonnull)hostName
{
    return [NSString stringWithCString:_cppCastingPlayer->GetHostName() encoding:NSUTF8StringEncoding];
}

- (NSString * _Nonnull)instanceName
{
    return [NSString stringWithCString:_cppCastingPlayer->GetInstanceName() encoding:NSUTF8StringEncoding];
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

- (void)logAllEndpoints
{
    NSArray<MCEndpoint *> * endpointsArray = [self endpoints];
    ChipLogDetail(AppServer, "MCCastingPlayer logAllEndpoints():");
    for (MCEndpoint * endpoint in endpointsArray) {
        ChipLogDetail(AppServer, "MCCastingPlayer MCEndpoint details: %@", [endpoint description]);
    }
}

@end
