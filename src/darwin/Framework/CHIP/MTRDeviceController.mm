/**
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
#import <Matter/MTRCommissionableBrowserResult.h>
#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceControllerParameters.h>

#import "MTRDeviceController_Internal.h"

#import "MTRAsyncWorkQueue.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRCommissioningParameters.h"
#import "MTRConversion.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceControllerFactory_Internal.h"
#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRDeviceControllerStartupParams.h"
#import "MTRDeviceControllerStartupParams_Internal.h"
#import "MTRDeviceControllerXPCParameters.h"
#import "MTRDeviceController_Concrete.h"
#import "MTRDeviceController_XPC.h"
#import "MTRDeviceController_XPC_Internal.h"
#import "MTRDevice_Concrete.h"
#import "MTRDevice_Internal.h"
#import "MTRError_Internal.h"
#import "MTRKeypair.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRMetricsCollector.h"
#import "MTRPersistentStorageDelegateBridge.h"
#import "MTRSetupPayload.h"
#import "MTRTimeUtils.h"
#import "MTRUnfairLock.h"
#import "MTRUtilities.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <app/server/Dnssd.h>
#include <crypto/CHIPCryptoPAL.h>

#include <atomic>
#include <dns_sd.h>
#include <string>

#import <os/lock.h>

static NSString * const kErrorSpake2pVerifierGenerationFailed = @"PASE verifier generation failed";
static NSString * const kErrorSpake2pVerifierSerializationFailed = @"PASE verifier serialization failed";

typedef void (^SyncWorkQueueBlock)(void);
typedef id (^SyncWorkQueueBlockWithReturnValue)(void);
typedef BOOL (^SyncWorkQueueBlockWithBoolReturnValue)(void);

using namespace chip::Tracing::DarwinFramework;

@interface MTRDeviceControllerDelegateInfo : NSObject
- (instancetype)initWithDelegate:(id<MTRDeviceControllerDelegate>)delegate queue:(dispatch_queue_t)queue;
@property (nonatomic, weak, readonly) id<MTRDeviceControllerDelegate> delegate;
@property (nonatomic, readonly) dispatch_queue_t queue;
@end

@implementation MTRDeviceControllerDelegateInfo
@synthesize delegate = _delegate, queue = _queue;
- (instancetype)initWithDelegate:(id<MTRDeviceControllerDelegate>)delegate queue:(dispatch_queue_t)queue
{
    if (!(self = [super init])) {
        return nil;
    }

    _delegate = delegate;
    _queue = queue;

    return self;
}
@end

@implementation MTRDeviceController {
    os_unfair_lock _underlyingDeviceMapLock;

    // For now, we just ensure that access to _suspended is atomic, but don't
    // guarantee atomicity of the the entire suspend/resume operation.  The
    // expectation is that suspend/resume on a given controller happen on some
    // specific queue, so can't race against each other.
    std::atomic<bool> _suspended;

    NSMutableArray<MTRDeviceControllerDelegateInfo *> * _delegates;
    id<MTRDeviceControllerDelegate> _strongDelegateForSetDelegateAPI;
}

@synthesize uniqueIdentifier = _uniqueIdentifier;

- (os_unfair_lock_t)deviceMapLock
{
    return &_underlyingDeviceMapLock;
}

- (instancetype)initForSubclasses:(BOOL)startSuspended
{
    if (self = [super init]) {
        // nothing, as superclass of MTRDeviceController is NSObject
    }
    _underlyingDeviceMapLock = OS_UNFAIR_LOCK_INIT;

    _suspended = startSuspended;

    _nodeIDToDeviceMap = [NSMapTable strongToWeakObjectsMapTable];

    _delegates = [NSMutableArray array];

    return self;
}

- (nullable MTRDeviceController *)initWithParameters:(MTRDeviceControllerAbstractParameters *)parameters error:(NSError * __autoreleasing *)error
{
    // Dispatch to the right non-abstract implementation.
    if ([parameters isKindOfClass:MTRXPCDeviceControllerParameters.class]) {
        MTR_LOG("Starting up with XPC Device Controller Parameters: %@", parameters);
        return [[MTRDeviceController_XPC alloc] initWithParameters:parameters error:error];
    }

    if ([parameters isKindOfClass:MTRDeviceControllerMachServiceXPCParameters.class]) {
        // TODO: This will need to at least pass in the uniqueIdentifier, no?  initWithMachServiceName:options: seems to
        // be declared but not actually implemented...
        auto * xpcParameters = static_cast<MTRDeviceControllerMachServiceXPCParameters *>(parameters);

        MTR_LOG("Starting up with Mach Service XPC Device Controller Parameters: %@", parameters);
        return [[MTRDeviceController_XPC alloc] initWithMachServiceName:xpcParameters.machServiceName options:xpcParameters.connectionOptions];
    }

    if ([parameters isKindOfClass:MTRDeviceControllerParameters.class]) {
        MTR_LOG("Starting up with Device Controller Parameters: %@", parameters);
        return [[MTRDeviceController_Concrete alloc] initWithParameters:parameters error:error];
    }

    MTR_LOG_ERROR("Unsupported type of MTRDeviceControllerAbstractParameters: %@", parameters);
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
    }
    return nil;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p, uuid: %@, suspended: %@>", NSStringFromClass(self.class), self, self.uniqueIdentifier, MTR_YES_NO(self.suspended)];
}

- (BOOL)isRunning
{
    MTR_ABSTRACT_METHOD();
    return NO;
}

#pragma mark - Suspend/resume support

- (BOOL)isSuspended
{
    return _suspended;
}

- (void)_notifyDelegatesOfSuspendState
{
    BOOL isSuspended = [self isSuspended];
    [self _callDelegatesWithBlock:^(id<MTRDeviceControllerDelegate> delegate) {
        if ([delegate respondsToSelector:@selector(controller:suspendedChangedTo:)]) {
            [delegate controller:self suspendedChangedTo:isSuspended];
        }
    } logString:__PRETTY_FUNCTION__];
}

- (void)suspend
{
    MTR_LOG("%@ suspending", self);

    if (![self isRunning]) {
        MTR_LOG_ERROR("%@ not running; can't suspend", self);
        return;
    }

    NSArray * devicesToSuspend;
    {
        std::lock_guard lock(*self.deviceMapLock);
        // Set _suspended under the device map lock.  This guarantees that
        // for any given device exactly one of two things is true:
        // * It is in the snapshot we are creating
        // * It is created after we have changed our _suspended state.
        if (_suspended) {
            MTR_LOG("%@ already suspended", self);
            return;
        }

        _suspended = YES;
        devicesToSuspend = [self.nodeIDToDeviceMap objectEnumerator].allObjects;
    }

    MTR_LOG("%@ found %lu devices to suspend", self, static_cast<unsigned long>(devicesToSuspend.count));
    for (MTRDevice * device in devicesToSuspend) {
        [device controllerSuspended];
    }

    // TODO: In the concrete class, consider what should happen with:
    //
    // * Active commissioning sessions (presumably close them?)
    // * CASE sessions in general.
    // * Possibly try to see whether we can change our fabric entry to not advertise and restart advertising.
    [self _notifyDelegatesOfSuspendState];

    [self _controllerSuspended];
}

- (void)_controllerSuspended
{
    // Subclass hook; nothing to do.
}

- (void)resume
{
    MTR_LOG("%@ resuming", self);

    if (![self isRunning]) {
        MTR_LOG_ERROR("%@ not running; can't resume", self);
        return;
    }

    NSArray * devicesToResume;
    {
        std::lock_guard lock(*self.deviceMapLock);
        // Set _suspended under the device map lock.  This guarantees that
        // for any given device exactly one of two things is true:
        // * It is in the snapshot we are creating
        // * It is created after we have changed our _suspended state.
        if (!_suspended) {
            MTR_LOG("%@ already not suspended", self);
            return;
        }

        _suspended = NO;
        devicesToResume = [self.nodeIDToDeviceMap objectEnumerator].allObjects;
    }

    MTR_LOG("%@ found %lu devices to resume", self, static_cast<unsigned long>(devicesToResume.count));
    for (MTRDevice * device in devicesToResume) {
        [device controllerResumed];
    }

    [self _notifyDelegatesOfSuspendState];

    [self _controllerResumed];
}

- (void)_controllerResumed
{
    // Subclass hook; nothing to do.
}

- (void)shutdown
{
    [self _clearDeviceControllerDelegates];
}

- (nullable NSNumber *)controllerNodeID
{
    MTR_ABSTRACT_METHOD();
    return nil;
}

- (BOOL)setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload
                                   newNodeID:(NSNumber *)newNodeID
                                       error:(NSError * __autoreleasing *)error
{
    MTR_ABSTRACT_METHOD();
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }
    return NO;
}

- (BOOL)setupCommissioningSessionWithDiscoveredDevice:(MTRCommissionableBrowserResult *)discoveredDevice
                                              payload:(MTRSetupPayload *)payload
                                            newNodeID:(NSNumber *)newNodeID
                                                error:(NSError * __autoreleasing *)error
{
    MTR_ABSTRACT_METHOD();
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }
    return NO;
}

- (BOOL)commissionNodeWithID:(NSNumber *)nodeID
         commissioningParams:(MTRCommissioningParameters *)commissioningParams
                       error:(NSError * __autoreleasing *)error
{
    MTR_ABSTRACT_METHOD();
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }
    return NO;
}

- (BOOL)continueCommissioningDevice:(void *)device
           ignoreAttestationFailure:(BOOL)ignoreAttestationFailure
                              error:(NSError * __autoreleasing *)error
{
    MTR_ABSTRACT_METHOD();
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }
    return NO;
}

- (BOOL)cancelCommissioningForNodeID:(NSNumber *)nodeID error:(NSError * __autoreleasing *)error
{
    MTR_ABSTRACT_METHOD();
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }
    return NO;
}

- (BOOL)startBrowseForCommissionables:(id<MTRCommissionableBrowserDelegate>)delegate queue:(dispatch_queue_t)queue
{
    MTR_ABSTRACT_METHOD();
    return NO;
}

- (BOOL)stopBrowseForCommissionables
{
    MTR_ABSTRACT_METHOD();
    return NO;
}

- (void)preWarmCommissioningSession
{
    MTR_ABSTRACT_METHOD();
}

- (nullable MTRBaseDevice *)deviceBeingCommissionedWithNodeID:(NSNumber *)nodeID error:(NSError * __autoreleasing *)error
{
    MTR_ABSTRACT_METHOD();
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }
    return nil;
}

- (MTRBaseDevice *)baseDeviceForNodeID:(NSNumber *)nodeID
{
    return [[MTRBaseDevice alloc] initWithNodeID:nodeID controller:self];
}

- (MTRDevice *)_setupDeviceForNodeID:(NSNumber *)nodeID prefetchedClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)prefetchedClusterData
{
    MTR_ABSTRACT_METHOD();
    // We promise to not return nil from this API... return an MTRDevice
    // instance, which will largely not be able to do anything useful.  This
    // only matters when someone subclasses MTRDeviceController in a weird way,
    // then tries to create an MTRDevice from their subclass.
    return [[MTRDevice alloc] initForSubclassesWithNodeID:nodeID controller:self];
}

- (MTRDevice * _Nullable)_deviceForNodeID:(NSNumber *)nodeID createIfNeeded:(BOOL)createIfNeeded
{
    std::lock_guard lock(*self.deviceMapLock);
    MTRDevice * deviceToReturn = [_nodeIDToDeviceMap objectForKey:nodeID];
    if (!deviceToReturn && createIfNeeded) {
        deviceToReturn = [self _setupDeviceForNodeID:nodeID prefetchedClusterData:nil];
    }

    return deviceToReturn;
}

- (MTRDevice *)deviceForNodeID:(NSNumber *)nodeID
{
    return [self _deviceForNodeID:nodeID createIfNeeded:YES];
}

- (void)forgetDeviceWithNodeID:(NSNumber *)nodeID
{
    MTRDevice * deviceToRemove;
    {
        std::lock_guard lock(*self.deviceMapLock);
        deviceToRemove = [_nodeIDToDeviceMap objectForKey:nodeID];
    }
    if (deviceToRemove != nil) {
        [self removeDevice:deviceToRemove];
    }
}

- (void)removeDevice:(MTRDevice *)device
{
    std::lock_guard lock(*self.deviceMapLock);
    auto * nodeID = device.nodeID;
    MTRDevice * deviceToRemove = [_nodeIDToDeviceMap objectForKey:nodeID];
    if (deviceToRemove == device) {
        [deviceToRemove invalidate];
        [_nodeIDToDeviceMap removeObjectForKey:nodeID];
    } else {
        MTR_LOG_ERROR("%@ Error: Cannot remove device %p with nodeID %llu", self, device, nodeID.unsignedLongLongValue);
    }
}

- (BOOL)setOperationalCertificateIssuer:(nullable id<MTROperationalCertificateIssuer>)operationalCertificateIssuer
                                  queue:(nullable dispatch_queue_t)queue
{
    MTR_ABSTRACT_METHOD();
    return NO;
}

+ (nullable NSData *)computePASEVerifierForSetupPasscode:(NSNumber *)setupPasscode
                                              iterations:(NSNumber *)iterations
                                                    salt:(NSData *)salt
                                                   error:(NSError * __autoreleasing *)error
{
    chip::Crypto::Spake2pVerifier verifier;
    CHIP_ERROR err = verifier.Generate(iterations.unsignedIntValue, AsByteSpan(salt), setupPasscode.unsignedIntValue);

    MATTER_LOG_METRIC_SCOPE(kMetricPASEVerifierForSetupCode, err);

    if ([MTRDeviceController checkForError:err logMsg:kErrorSpake2pVerifierGenerationFailed error:error]) {
        return nil;
    }

    uint8_t serializedBuffer[chip::Crypto::kSpake2p_VerifierSerialized_Length];
    chip::MutableByteSpan serializedBytes(serializedBuffer);
    err = verifier.Serialize(serializedBytes);
    if ([MTRDeviceController checkForError:err logMsg:kErrorSpake2pVerifierSerializationFailed error:error]) {
        return nil;
    }

    return AsData(serializedBytes);
}

- (NSData * _Nullable)attestationChallengeForDeviceID:(NSNumber *)deviceID
{
    MTR_ABSTRACT_METHOD();
    return nil;
}

- (BOOL)addServerEndpoint:(MTRServerEndpoint *)endpoint
{
    MTR_ABSTRACT_METHOD();
    return NO;
}

- (void)removeServerEndpoint:(MTRServerEndpoint *)endpoint queue:(dispatch_queue_t)queue completion:(dispatch_block_t)completion
{
    [self removeServerEndpointInternal:endpoint queue:queue completion:completion];
}

- (void)removeServerEndpoint:(MTRServerEndpoint *)endpoint
{
    [self removeServerEndpointInternal:endpoint queue:nil completion:nil];
}

- (void)removeServerEndpointInternal:(MTRServerEndpoint *)endpoint queue:(dispatch_queue_t _Nullable)queue completion:(dispatch_block_t _Nullable)completion
{
    MTR_ABSTRACT_METHOD();
    if (queue != nil && completion != nil) {
        dispatch_async(queue, completion);
    }
}

+ (BOOL)checkForError:(CHIP_ERROR)errorCode logMsg:(NSString *)logMsg error:(NSError * __autoreleasing *)error
{
    if (CHIP_NO_ERROR == errorCode) {
        return NO;
    }

    MTR_LOG_ERROR("Error(%" CHIP_ERROR_FORMAT "): %@ %s", errorCode.Format(), self, [logMsg UTF8String]);
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:errorCode];
    }

    return YES;
}

- (void)asyncDispatchToMatterQueue:(dispatch_block_t)block errorHandler:(nullable MTRDeviceErrorHandler)errorHandler
{
    // TODO: Figure out how to get callsites to have an MTRDeviceController_Concrete.
    MTR_ABSTRACT_METHOD();
    errorHandler([MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
}

- (nullable NSNumber *)compressedFabricID
{
    MTR_ABSTRACT_METHOD();
    return nil;
}

#ifdef DEBUG
+ (void)forceLocalhostAdvertisingOnly
{
    auto interfaceIndex = chip::Inet::InterfaceId::PlatformType(kDNSServiceInterfaceIndexLocalOnly);
    auto interfaceId = chip::Inet::InterfaceId(interfaceIndex);
    chip::app::DnssdServer::Instance().SetInterfaceId(interfaceId);
}
#endif // DEBUG

- (NSArray<NSNumber *> *)nodesWithStoredData
{
    MTR_ABSTRACT_METHOD();
    return @[];
}

#pragma mark - MTRDeviceControllerDelegate management

// Note these are implemented in the base class so that XPC subclass can use it as well
- (void)setDeviceControllerDelegate:(id<MTRDeviceControllerDelegate>)delegate queue:(dispatch_queue_t)queue
{
    @synchronized(self) {
        if (_strongDelegateForSetDelegateAPI) {
            if (_strongDelegateForSetDelegateAPI == delegate) {
                MTR_LOG("%@ setDeviceControllerDelegate: delegate %p is already set", self, delegate);
                return;
            }

            MTR_LOG("%@ setDeviceControllerDelegate: replacing %p with %p", self, _strongDelegateForSetDelegateAPI, delegate);
            [self removeDeviceControllerDelegate:_strongDelegateForSetDelegateAPI];
        }
        _strongDelegateForSetDelegateAPI = delegate;
        [self addDeviceControllerDelegate:delegate queue:queue];
    }
}

- (void)addDeviceControllerDelegate:(id<MTRDeviceControllerDelegate>)delegate queue:(dispatch_queue_t)queue
{
    @synchronized(self) {
        __block BOOL delegateAlreadyAdded = NO;
        [self _iterateDelegateInfoWithBlock:^(MTRDeviceControllerDelegateInfo * delegateInfo) {
            if (delegateInfo.delegate == delegate) {
                delegateAlreadyAdded = YES;
            }
        }];
        if (delegateAlreadyAdded) {
            MTR_LOG("%@ addDeviceControllerDelegate: delegate already added", self);
            return;
        }

        MTRDeviceControllerDelegateInfo * newDelegateInfo = [[MTRDeviceControllerDelegateInfo alloc] initWithDelegate:delegate queue:queue];
        [_delegates addObject:newDelegateInfo];
        MTR_LOG("%@ addDeviceControllerDelegate: added %p total %lu", self, delegate, static_cast<unsigned long>(_delegates.count));
    }
}

- (void)removeDeviceControllerDelegate:(id<MTRDeviceControllerDelegate>)delegate
{
    @synchronized(self) {
        if (_strongDelegateForSetDelegateAPI == delegate) {
            _strongDelegateForSetDelegateAPI = nil;
        }

        __block MTRDeviceControllerDelegateInfo * delegateInfoToRemove = nil;
        [self _iterateDelegateInfoWithBlock:^(MTRDeviceControllerDelegateInfo * delegateInfo) {
            if (delegateInfo.delegate == delegate) {
                delegateInfoToRemove = delegateInfo;
            }
        }];

        if (delegateInfoToRemove) {
            [_delegates removeObject:delegateInfoToRemove];
            MTR_LOG("%@ removeDeviceControllerDelegate: removed %p remaining %lu", self, delegate, static_cast<unsigned long>(_delegates.count));
        } else {
            MTR_LOG("%@ removeDeviceControllerDelegate: delegate %p not found in %lu", self, delegate, static_cast<unsigned long>(_delegates.count));
        }
    }
}

- (void)_clearDeviceControllerDelegates
{
    @synchronized(self) {
        _strongDelegateForSetDelegateAPI = nil;
        [_delegates removeAllObjects];
    }
}

// Iterates the delegates, and remove delegate info objects if the delegate object has dealloc'ed
// Returns number of delegates called
- (NSUInteger)_iterateDelegateInfoWithBlock:(void (^_Nullable)(MTRDeviceControllerDelegateInfo * delegateInfo))block
{
    @synchronized(self) {
        if (!_delegates.count) {
            MTR_LOG("%@ No delegates to iterate", self);
            return 0;
        }

        // Opportunistically remove defunct delegate references on every iteration
        NSMutableArray * delegatesToRemove = nil;
        for (MTRDeviceControllerDelegateInfo * delegateInfo in _delegates) {
            id<MTRDeviceControllerDelegate> strongDelegate = delegateInfo.delegate;
            if (strongDelegate) {
                if (block) {
                    block(delegateInfo);
                }
            } else {
                if (!delegatesToRemove) {
                    delegatesToRemove = [NSMutableArray array];
                }
                [delegatesToRemove addObject:delegateInfo];
            }
        }

        if (delegatesToRemove.count) {
            [_delegates removeObjectsInArray:delegatesToRemove];
            MTR_LOG("%@ _iterateDelegatesWithBlock: removed %lu remaining %lu", self, static_cast<unsigned long>(delegatesToRemove.count), static_cast<unsigned long>(_delegates.count));
        }

        return _delegates.count;
    }
}

- (void)_callDelegatesWithBlock:(void (^_Nullable)(id<MTRDeviceControllerDelegate> delegate))block logString:(const char *)logString
{
    NSUInteger delegatesCalled = [self _iterateDelegateInfoWithBlock:^(MTRDeviceControllerDelegateInfo * delegateInfo) {
        id<MTRDeviceControllerDelegate> strongDelegate = delegateInfo.delegate;
        dispatch_async(delegateInfo.queue, ^{
            block(strongDelegate);
        });
    }];

    MTR_LOG("%@ %lu delegates called for %s", self, static_cast<unsigned long>(delegatesCalled), logString);
}

#if DEBUG
- (NSUInteger)unitTestDelegateCount
{
    return [self _iterateDelegateInfoWithBlock:nil];
}
#endif

- (void)controller:(MTRDeviceController *)controller statusUpdate:(MTRCommissioningStatus)status
{
    [self _callDelegatesWithBlock:^(id<MTRDeviceControllerDelegate> delegate) {
        if ([delegate respondsToSelector:@selector(controller:statusUpdate:)]) {
            [delegate controller:controller statusUpdate:status];
        }
    } logString:__PRETTY_FUNCTION__];
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error
{
    [self _callDelegatesWithBlock:^(id<MTRDeviceControllerDelegate> delegate) {
        if ([delegate respondsToSelector:@selector(controller:commissioningSessionEstablishmentDone:)]) {
            [delegate controller:controller commissioningSessionEstablishmentDone:error];
        }
    } logString:__PRETTY_FUNCTION__];
}

- (void)controller:(MTRDeviceController *)controller
    commissioningComplete:(NSError * _Nullable)error
                   nodeID:(NSNumber * _Nullable)nodeID
                  metrics:(MTRMetrics *)metrics
{
    [self _callDelegatesWithBlock:^(id<MTRDeviceControllerDelegate> delegate) {
        if ([delegate respondsToSelector:@selector(controller:commissioningComplete:nodeID:metrics:)]) {
            [delegate controller:controller commissioningComplete:error nodeID:nodeID metrics:metrics];
        } else if ([delegate respondsToSelector:@selector(controller:commissioningComplete:nodeID:)]) {
            [delegate controller:controller commissioningComplete:error nodeID:nodeID];
        } else if ([delegate respondsToSelector:@selector(controller:commissioningComplete:)]) {
            [delegate controller:controller commissioningComplete:error];
        }
    } logString:__PRETTY_FUNCTION__];
}

- (void)controller:(MTRDeviceController *)controller readCommissioneeInfo:(MTRCommissioneeInfo *)info
{
    [self _callDelegatesWithBlock:^(id<MTRDeviceControllerDelegate> delegate) {
        if ([delegate respondsToSelector:@selector(controller:readCommissioneeInfo:)]) {
            [delegate controller:controller readCommissioneeInfo:info];
        } else if ([delegate respondsToSelector:@selector(controller:readCommissioningInfo:)]) {
            [delegate controller:controller readCommissioningInfo:info.productIdentity];
        }
    } logString:__PRETTY_FUNCTION__];
}

@end

// TODO: This should not be in the superclass: either move to
// MTRDeviceController_Concrete.mm, or move into a separate .h/.mm pair of
// files.
@implementation MTRDevicePairingDelegateShim
- (instancetype)initWithDelegate:(id<MTRDevicePairingDelegate>)delegate
{
    if (self = [super init]) {
        _delegate = delegate;
    }
    return self;
}

- (BOOL)respondsToSelector:(SEL)selector
{
    if (selector == @selector(controller:statusUpdate:)) {
        return [self.delegate respondsToSelector:@selector(onStatusUpdate:)];
    }

    if (selector == @selector(controller:commissioningSessionEstablishmentDone:)) {
        return [self.delegate respondsToSelector:@selector(onPairingComplete:)];
    }

    if (selector == @selector(controller:commissioningComplete:)) {
        return [self.delegate respondsToSelector:@selector(onCommissioningComplete:)];
    }

    return [super respondsToSelector:selector];
}

- (void)controller:(MTRDeviceController *)controller statusUpdate:(MTRCommissioningStatus)status
{
    [self.delegate onStatusUpdate:static_cast<MTRPairingStatus>(status)];
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error
{
    [self.delegate onPairingComplete:error];
}

- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError * _Nullable)error
{
    [self.delegate onCommissioningComplete:error];
}

- (void)onPairingDeleted:(NSError * _Nullable)error
{
    [self.delegate onPairingDeleted:error];
}

@end

/**
 * Shim to allow us to treat an MTRNOCChainIssuer as an
 * MTROperationalCertificateIssuer.
 */
// TODO: This should not be in the superclass: either move to
// MTRDeviceController_Concrete.mm, or move into a separate .h/.mm pair of
// files.
@interface MTROperationalCertificateChainIssuerShim : NSObject <MTROperationalCertificateIssuer>
@property (nonatomic, readonly) id<MTRNOCChainIssuer> nocChainIssuer;
@property (nonatomic, readonly) BOOL shouldSkipAttestationCertificateValidation;
- (instancetype)initWithIssuer:(id<MTRNOCChainIssuer>)nocChainIssuer;
@end

@implementation MTROperationalCertificateChainIssuerShim
- (instancetype)initWithIssuer:(id<MTRNOCChainIssuer>)nocChainIssuer
{
    if (self = [super init]) {
        _nocChainIssuer = nocChainIssuer;
        _shouldSkipAttestationCertificateValidation = YES;
    }
    return self;
}

- (void)issueOperationalCertificateForRequest:(MTROperationalCSRInfo *)csrInfo
                              attestationInfo:(MTRDeviceAttestationInfo *)attestationInfo
                                   controller:(MTRDeviceController *)controller
                                   completion:(void (^)(MTROperationalCertificateChain * _Nullable info,
                                                  NSError * _Nullable error))completion
{
    CSRInfo * oldCSRInfo = [[CSRInfo alloc] initWithNonce:csrInfo.csrNonce
                                                 elements:csrInfo.csrElementsTLV
                                        elementsSignature:csrInfo.attestationSignature
                                                      csr:csrInfo.csr];
    NSData * _Nullable firmwareInfo = attestationInfo.firmwareInfo;
    if (firmwareInfo == nil) {
        firmwareInfo = [NSData data];
    }
    AttestationInfo * oldAttestationInfo =
        [[AttestationInfo alloc] initWithChallenge:attestationInfo.challenge
                                             nonce:attestationInfo.nonce
                                          elements:attestationInfo.elementsTLV
                                 elementsSignature:attestationInfo.elementsSignature
                                               dac:attestationInfo.deviceAttestationCertificate
                                               pai:attestationInfo.productAttestationIntermediateCertificate
                          certificationDeclaration:attestationInfo.certificationDeclaration
                                      firmwareInfo:firmwareInfo];
    [self.nocChainIssuer
          onNOCChainGenerationNeeded:oldCSRInfo
                     attestationInfo:oldAttestationInfo
        onNOCChainGenerationComplete:^(NSData * operationalCertificate, NSData * intermediateCertificate, NSData * rootCertificate,
            NSData * _Nullable ipk, NSNumber * _Nullable adminSubject, NSError * __autoreleasing * error) {
            auto * chain = [[MTROperationalCertificateChain alloc] initWithOperationalCertificate:operationalCertificate
                                                                          intermediateCertificate:intermediateCertificate
                                                                                  rootCertificate:rootCertificate
                                                                                     adminSubject:adminSubject];
            completion(chain, nil);
            if (error != nil) {
                *error = nil;
            }
        }];
}

@end

@implementation MTRDeviceController (Deprecated)

- (NSNumber *)controllerNodeId
{
    return self.controllerNodeID;
}

- (nullable NSData *)fetchAttestationChallengeForDeviceId:(uint64_t)deviceId
{
    return [self attestationChallengeForDeviceID:@(deviceId)];
}

- (BOOL)getBaseDevice:(uint64_t)deviceID queue:(dispatch_queue_t)queue completionHandler:(MTRDeviceConnectionCallback)completion
{
    MTR_ABSTRACT_METHOD();
    return NO;
}

- (BOOL)pairDevice:(uint64_t)deviceID
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
{
    MTR_ABSTRACT_METHOD();
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }
    return NO;
}

- (BOOL)pairDevice:(uint64_t)deviceID
           address:(NSString *)address
              port:(uint16_t)port
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
{
    MTR_ABSTRACT_METHOD();
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }
    return NO;
}

- (BOOL)pairDevice:(uint64_t)deviceID onboardingPayload:(NSString *)onboardingPayload error:(NSError * __autoreleasing *)error
{
    MTR_ABSTRACT_METHOD();
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }
    return NO;
}

- (BOOL)commissionDevice:(uint64_t)deviceID
     commissioningParams:(MTRCommissioningParameters *)commissioningParams
                   error:(NSError * __autoreleasing *)error
{
    return [self commissionNodeWithID:@(deviceID) commissioningParams:commissioningParams error:error];
}

- (BOOL)stopDevicePairing:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    return [self cancelCommissioningForNodeID:@(deviceID) error:error];
}

- (MTRBaseDevice *)getDeviceBeingCommissioned:(uint64_t)deviceId error:(NSError * __autoreleasing *)error
{
    return [self deviceBeingCommissionedWithNodeID:@(deviceId) error:error];
}

- (BOOL)openPairingWindow:(uint64_t)deviceID duration:(NSUInteger)duration error:(NSError * __autoreleasing *)error
{
    MTR_ABSTRACT_METHOD();
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }
    return NO;
}

- (nullable NSString *)openPairingWindowWithPIN:(uint64_t)deviceID
                                       duration:(NSUInteger)duration
                                  discriminator:(NSUInteger)discriminator
                                       setupPIN:(NSUInteger)setupPIN
                                          error:(NSError * __autoreleasing *)error
{
    MTR_ABSTRACT_METHOD();
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }
    return nil;
}

- (nullable NSData *)computePaseVerifier:(uint32_t)setupPincode iterations:(uint32_t)iterations salt:(NSData *)salt
{
    return [MTRDeviceController computePASEVerifierForSetupPasscode:@(setupPincode) iterations:@(iterations) salt:salt error:nil];
}

- (void)setPairingDelegate:(id<MTRDevicePairingDelegate>)delegate queue:(dispatch_queue_t)queue
{
    auto * delegateShim = [[MTRDevicePairingDelegateShim alloc] initWithDelegate:delegate];
    [self setDeviceControllerDelegate:delegateShim queue:queue];
}

- (void)setNocChainIssuer:(id<MTRNOCChainIssuer>)nocChainIssuer queue:(dispatch_queue_t)queue
{
    [self setOperationalCertificateIssuer:[[MTROperationalCertificateChainIssuerShim alloc] initWithIssuer:nocChainIssuer]
                                    queue:queue];
}
@end
