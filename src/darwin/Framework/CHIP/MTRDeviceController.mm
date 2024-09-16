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
#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceControllerParameters.h>

#import "MTRDeviceController_Internal.h"

#import "MTRAsyncWorkQueue.h"
#import "MTRAttestationTrustStoreBridge.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRCommissionableBrowser.h"
#import "MTRCommissionableBrowserResult_Internal.h"
#import "MTRCommissioningParameters.h"
#import "MTRConversion.h"
#import "MTRDeviceControllerDelegateBridge.h"
#import "MTRDeviceControllerFactory_Internal.h"
#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRDeviceControllerStartupParams.h"
#import "MTRDeviceControllerStartupParams_Internal.h"
#import "MTRDeviceController_Concrete.h"
#import "MTRDeviceController_XPC.h"
#import "MTRDevice_Concrete.h"
#import "MTRDevice_Internal.h"
#import "MTRError_Internal.h"
#import "MTRKeypair.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRMetricsCollector.h"
#import "MTROperationalCredentialsDelegate.h"
#import "MTRP256KeypairBridge.h"
#import "MTRPersistentStorageDelegateBridge.h"
#import "MTRServerEndpoint_Internal.h"
#import "MTRSetupPayload.h"
#import "MTRTimeUtils.h"
#import "MTRUnfairLock.h"
#import "MTRUtilities.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"
#import <setup_payload/ManualSetupPayloadGenerator.h>
#import <setup_payload/SetupPayload.h>
#import <zap-generated/MTRBaseClusters.h>

#import "MTRDeviceAttestationDelegateBridge.h"
#import "MTRDeviceConnectionBridge.h"

#include <platform/CHIPDeviceConfig.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/List.h>
#include <app/server/Dnssd.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/CommissioningWindowOpener.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <credentials/attestation_verifier/DacOnlyPartialAttestationVerifier.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <inet/InetInterface.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <platform/LockTracker.h>
#include <platform/PlatformManager.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <system/SystemClock.h>

#include <atomic>
#include <dns_sd.h>
#include <optional>
#include <string>

#import <os/lock.h>

// TODO: These strings and their consumers in this file should probably go away,
// since none of them really apply to all controllers.
static NSString * const kErrorCommissionerInit = @"Init failure while initializing a commissioner";
static NSString * const kErrorIPKInit = @"Init failure while initializing IPK";
static NSString * const kErrorSigningKeypairInit = @"Init failure while creating signing keypair bridge";
static NSString * const kErrorOperationalCredentialsInit = @"Init failure while creating operational credentials delegate";
static NSString * const kErrorOperationalKeypairInit = @"Init failure while creating operational keypair bridge";
static NSString * const kErrorPairingInit = @"Init failure while creating a pairing delegate";
static NSString * const kErrorPartialDacVerifierInit = @"Init failure while creating a partial DAC verifier";
static NSString * const kErrorPairDevice = @"Failure while pairing the device";
static NSString * const kErrorStopPairing = @"Failure while trying to stop the pairing process";
static NSString * const kErrorOpenPairingWindow = @"Open Pairing Window failed";
static NSString * const kErrorNotRunning = @"Controller is not running. Call startup first.";
static NSString * const kErrorSetupCodeGen = @"Generating Manual Pairing Code failed";
static NSString * const kErrorGenerateNOC = @"Generating operational certificate failed";
static NSString * const kErrorKeyAllocation = @"Generating new operational key failed";
static NSString * const kErrorCSRValidation = @"Extracting public key from CSR failed";
static NSString * const kErrorGetCommissionee = @"Failure obtaining device being commissioned";
static NSString * const kErrorGetAttestationChallenge = @"Failure getting attestation challenge";
static NSString * const kErrorSpake2pVerifierGenerationFailed = @"PASE verifier generation failed";
static NSString * const kErrorSpake2pVerifierSerializationFailed = @"PASE verifier serialization failed";
static NSString * const kErrorCDCertStoreInit = @"Init failure while initializing Certificate Declaration Signing Keys store";

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
    chip::Controller::DeviceCommissioner * _cppCommissioner;
    chip::Credentials::PartialDACVerifier * _partialDACVerifier;
    chip::Credentials::DefaultDACVerifier * _defaultDACVerifier;
    MTRDeviceControllerDelegateBridge * _deviceControllerDelegateBridge;
    MTROperationalCredentialsDelegate * _operationalCredentialsDelegate;
    MTRDeviceAttestationDelegateBridge * _deviceAttestationDelegateBridge;
    MTRDeviceControllerFactory * _factory;
    os_unfair_lock _underlyingDeviceMapLock;
    MTRCommissionableBrowser * _commissionableBrowser;
    MTRAttestationTrustStoreBridge * _attestationTrustStoreBridge;

    // _serverEndpoints is only touched on the Matter queue.
    NSMutableArray<MTRServerEndpoint *> * _serverEndpoints;

    MTRDeviceStorageBehaviorConfiguration * _storageBehaviorConfiguration;
    std::atomic<chip::FabricIndex> _storedFabricIndex;
    std::atomic<std::optional<uint64_t>> _storedCompressedFabricID;
    MTRP256KeypairBridge _signingKeypairBridge;
    MTRP256KeypairBridge _operationalKeypairBridge;

    // For now, we just ensure that access to _suspended is atomic, but don't
    // guarantee atomicity of the the entire suspend/resume operation.  The
    // expectation is that suspend/resume on a given controller happen on some
    // specific queue, so can't race against each other.
    std::atomic<bool> _suspended;

    // Counters to track assertion status and access controlled by the _assertionLock
    NSUInteger _keepRunningAssertionCounter;
    BOOL _shutdownPending;
    os_unfair_lock _assertionLock;

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

    // Setup assertion variables
    _keepRunningAssertionCounter = 0;
    _shutdownPending = NO;
    _assertionLock = OS_UNFAIR_LOCK_INIT;

    _suspended = startSuspended;

    _nodeIDToDeviceMap = [NSMapTable strongToWeakObjectsMapTable];

    _delegates = [NSMutableArray array];

    return self;
}

- (nullable MTRDeviceController *)initWithParameters:(MTRDeviceControllerAbstractParameters *)parameters error:(NSError * __autoreleasing *)error
{
    if ([parameters isKindOfClass:MTRXPCDeviceControllerParameters.class]) {
        MTR_LOG("Starting up with XPC Device Controller Parameters: %@", parameters);
        return [[MTRDeviceController_XPC alloc] initWithParameters:parameters error:error];
    } else if (![parameters isKindOfClass:MTRDeviceControllerParameters.class]) {
        MTR_LOG_ERROR("Unsupported type of MTRDeviceControllerAbstractParameters: %@", parameters);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }
    auto * controllerParameters = static_cast<MTRDeviceControllerParameters *>(parameters);

    // MTRDeviceControllerFactory will auto-start in per-controller-storage mode if necessary
    return [MTRDeviceControllerFactory.sharedInstance initializeController:[MTRDeviceController_Concrete alloc] withParameters:controllerParameters error:error];
}

- (instancetype)initWithFactory:(MTRDeviceControllerFactory *)factory
                             queue:(dispatch_queue_t)queue
                   storageDelegate:(id<MTRDeviceControllerStorageDelegate> _Nullable)storageDelegate
              storageDelegateQueue:(dispatch_queue_t _Nullable)storageDelegateQueue
               otaProviderDelegate:(id<MTROTAProviderDelegate> _Nullable)otaProviderDelegate
          otaProviderDelegateQueue:(dispatch_queue_t _Nullable)otaProviderDelegateQueue
                  uniqueIdentifier:(NSUUID *)uniqueIdentifier
    concurrentSubscriptionPoolSize:(NSUInteger)concurrentSubscriptionPoolSize
      storageBehaviorConfiguration:(MTRDeviceStorageBehaviorConfiguration *)storageBehaviorConfiguration
                    startSuspended:(BOOL)startSuspended
{
    if (self = [super init]) {
        // Make sure our storage is all set up to work as early as possible,
        // before we start doing anything else with the controller.
        _uniqueIdentifier = uniqueIdentifier;

        // Setup assertion variables
        _keepRunningAssertionCounter = 0;
        _shutdownPending = NO;
        _assertionLock = OS_UNFAIR_LOCK_INIT;

        _suspended = startSuspended;

        if (storageDelegate != nil) {
            if (storageDelegateQueue == nil) {
                MTR_LOG_ERROR("storageDelegate provided without storageDelegateQueue");
                return nil;
            }

            id<MTRDeviceControllerStorageDelegate> storageDelegateToUse = storageDelegate;
            if (MTRDeviceControllerLocalTestStorage.localTestStorageEnabled) {
                storageDelegateToUse = [[MTRDeviceControllerLocalTestStorage alloc] initWithPassThroughStorage:storageDelegate];
            }
            _controllerDataStore = [[MTRDeviceControllerDataStore alloc] initWithController:self
                                                                            storageDelegate:storageDelegateToUse
                                                                       storageDelegateQueue:storageDelegateQueue];
            if (_controllerDataStore == nil) {
                return nil;
            }
        } else {
            if (MTRDeviceControllerLocalTestStorage.localTestStorageEnabled) {
                dispatch_queue_t localTestStorageQueue = dispatch_queue_create("org.csa-iot.matter.framework.devicecontroller.localteststorage", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
                MTRDeviceControllerLocalTestStorage * localTestStorage = [[MTRDeviceControllerLocalTestStorage alloc] initWithPassThroughStorage:nil];
                _controllerDataStore = [[MTRDeviceControllerDataStore alloc] initWithController:self
                                                                                storageDelegate:localTestStorage
                                                                           storageDelegateQueue:localTestStorageQueue];
                if (_controllerDataStore == nil) {
                    return nil;
                }
            }
        }

        // Ensure the otaProviderDelegate, if any, is valid.
        if (otaProviderDelegate == nil && otaProviderDelegateQueue != nil) {
            MTR_LOG_ERROR("Must have otaProviderDelegate when we have otaProviderDelegateQueue");
            return nil;
        }

        if (otaProviderDelegate != nil && otaProviderDelegateQueue == nil) {
            MTR_LOG_ERROR("Must have otaProviderDelegateQueue when we have otaProviderDelegate");
            return nil;
        }

        if (otaProviderDelegate != nil) {
            if (![otaProviderDelegate respondsToSelector:@selector(handleQueryImageForNodeID:controller:params:completion:)]
                && ![otaProviderDelegate respondsToSelector:@selector(handleQueryImageForNodeID:controller:params:completionHandler:)]) {
                MTR_LOG_ERROR("Error: MTROTAProviderDelegate does not support handleQueryImageForNodeID");
                return nil;
            }
            if (![otaProviderDelegate respondsToSelector:@selector(handleApplyUpdateRequestForNodeID:controller:params:completion:)]
                && ![otaProviderDelegate respondsToSelector:@selector(handleApplyUpdateRequestForNodeID:controller:params:completionHandler:)]) {
                MTR_LOG_ERROR("Error: MTROTAProviderDelegate does not support handleApplyUpdateRequestForNodeID");
                return nil;
            }
            if (![otaProviderDelegate respondsToSelector:@selector(handleNotifyUpdateAppliedForNodeID:controller:params:completion:)]
                && ![otaProviderDelegate
                    respondsToSelector:@selector(handleNotifyUpdateAppliedForNodeID:controller:params:completionHandler:)]) {
                MTR_LOG_ERROR("Error: MTROTAProviderDelegate does not support handleNotifyUpdateAppliedForNodeID");
                return nil;
            }
            if (![otaProviderDelegate respondsToSelector:@selector(handleBDXTransferSessionBeginForNodeID:controller:fileDesignator:offset:completion:)]
                && ![otaProviderDelegate respondsToSelector:@selector(handleBDXTransferSessionBeginForNodeID:controller:fileDesignator:offset:completionHandler:)]) {
                MTR_LOG_ERROR("Error: MTROTAProviderDelegate does not support handleBDXTransferSessionBeginForNodeID");
                return nil;
            }
            if (![otaProviderDelegate respondsToSelector:@selector(handleBDXQueryForNodeID:controller:blockSize:blockIndex:bytesToSkip:completion:)]
                && ![otaProviderDelegate respondsToSelector:@selector(handleBDXQueryForNodeID:controller:blockSize:blockIndex:bytesToSkip:completionHandler:)]) {
                MTR_LOG_ERROR("Error: MTROTAProviderDelegate does not support handleBDXQueryForNodeID");
                return nil;
            }
        }

        _otaProviderDelegate = otaProviderDelegate;
        _otaProviderDelegateQueue = otaProviderDelegateQueue;

        _chipWorkQueue = queue;
        _factory = factory;
        _underlyingDeviceMapLock = OS_UNFAIR_LOCK_INIT;
        _nodeIDToDeviceMap = [NSMapTable strongToWeakObjectsMapTable];
        _serverEndpoints = [[NSMutableArray alloc] init];
        _commissionableBrowser = nil;

        _deviceControllerDelegateBridge = new MTRDeviceControllerDelegateBridge();
        if ([self checkForInitError:(_deviceControllerDelegateBridge != nullptr) logMsg:kErrorPairingInit]) {
            return nil;
        }

        _partialDACVerifier = new chip::Credentials::PartialDACVerifier();
        if ([self checkForInitError:(_partialDACVerifier != nullptr) logMsg:kErrorPartialDacVerifierInit]) {
            return nil;
        }

        _operationalCredentialsDelegate = new MTROperationalCredentialsDelegate(self);
        if ([self checkForInitError:(_operationalCredentialsDelegate != nullptr) logMsg:kErrorOperationalCredentialsInit]) {
            return nil;
        }

        // Provide a way to test different subscription pool sizes without code change
        NSUserDefaults * defaults = [NSUserDefaults standardUserDefaults];
        if ([defaults objectForKey:kDefaultSubscriptionPoolSizeOverrideKey]) {
            NSInteger subscriptionPoolSizeOverride = [defaults integerForKey:kDefaultSubscriptionPoolSizeOverrideKey];
            if (subscriptionPoolSizeOverride < 1) {
                concurrentSubscriptionPoolSize = 1;
            } else {
                concurrentSubscriptionPoolSize = static_cast<NSUInteger>(subscriptionPoolSizeOverride);
            }

            MTR_LOG(" *** Overriding pool size of MTRDeviceController with: %lu", static_cast<unsigned long>(concurrentSubscriptionPoolSize));
        }

        if (!concurrentSubscriptionPoolSize) {
            concurrentSubscriptionPoolSize = 1;
        }

        MTR_LOG("%@ Setting up pool size of MTRDeviceController with: %lu", self, static_cast<unsigned long>(concurrentSubscriptionPoolSize));

        _concurrentSubscriptionPool = [[MTRAsyncWorkQueue alloc] initWithContext:self width:concurrentSubscriptionPoolSize];

        _storedFabricIndex = chip::kUndefinedFabricIndex;
        _storedCompressedFabricID = std::nullopt;
        self.nodeID = nil;
        self.fabricID = nil;
        self.rootPublicKey = nil;

        _storageBehaviorConfiguration = storageBehaviorConfiguration;
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p, uuid: %@, suspended: %@>", NSStringFromClass(self.class), self, self.uniqueIdentifier, MTR_YES_NO(self.suspended)];
}

- (BOOL)isRunning
{
    return _cppCommissioner != nullptr;
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

- (BOOL)matchesPendingShutdownControllerWithOperationalCertificate:(nullable MTRCertificateDERBytes)operationalCertificate andRootCertificate:(nullable MTRCertificateDERBytes)rootCertificate
{
    if (!operationalCertificate || !rootCertificate) {
        return FALSE;
    }
    NSNumber * nodeID = [MTRDeviceControllerParameters nodeIDFromNOC:operationalCertificate];
    NSNumber * fabricID = [MTRDeviceControllerParameters fabricIDFromNOC:operationalCertificate];
    NSData * publicKey = [MTRDeviceControllerParameters publicKeyFromCertificate:rootCertificate];

    std::lock_guard lock(_assertionLock);

    // If any of the local above are nil, the return will be false since MTREqualObjects handles them correctly
    return _keepRunningAssertionCounter > 0 && _shutdownPending && MTREqualObjects(nodeID, self.nodeID) && MTREqualObjects(fabricID, self.fabricID) && MTREqualObjects(publicKey, self.rootPublicKey);
}

- (void)addRunAssertion
{
    std::lock_guard lock(_assertionLock);

    // Only take an assertion if running
    if ([self isRunning]) {
        ++_keepRunningAssertionCounter;
        MTR_LOG("%@ Adding keep running assertion, total %lu", self, static_cast<unsigned long>(_keepRunningAssertionCounter));
    }
}

- (void)removeRunAssertion;
{
    std::lock_guard lock(_assertionLock);

    if (_keepRunningAssertionCounter > 0) {
        --_keepRunningAssertionCounter;
        MTR_LOG("%@ Removing keep running assertion, total %lu", self, static_cast<unsigned long>(_keepRunningAssertionCounter));

        if ([self isRunning] && _keepRunningAssertionCounter == 0 && _shutdownPending) {
            MTR_LOG("%@ All assertions removed and shutdown is pending, shutting down", self);
            [self finalShutdown];
        }
    }
}

- (void)clearPendingShutdown
{
    std::lock_guard lock(_assertionLock);
    _shutdownPending = NO;
}

- (void)shutdown
{
    std::lock_guard lock(_assertionLock);

    if (_keepRunningAssertionCounter > 0) {
        MTR_LOG("%@ Pending shutdown since %lu assertions are present", self, static_cast<unsigned long>(_keepRunningAssertionCounter));
        _shutdownPending = YES;
        return;
    }
    [self finalShutdown];
}

- (void)finalShutdown
{
    os_unfair_lock_assert_owner(&_assertionLock);

    MTR_LOG("%@ shutdown called", self);
    if (_cppCommissioner == nullptr) {
        // Already shut down.
        return;
    }

    MTR_LOG("Shutting down %@: %@", NSStringFromClass(self.class), self);
    [self cleanupAfterStartup];
}

// Clean up from a state where startup was called.
- (void)cleanupAfterStartup
{
    // Invalidate our MTRDevice instances before we shut down our secure
    // sessions and whatnot, so they don't start trying to resubscribe when we
    // do the secure session shutdowns.  Since we don't want to hold the lock
    // while calling out into arbitrary invalidation code, snapshot the list of
    // devices before we start invalidating.
    MTR_LOG("%s: %@", __PRETTY_FUNCTION__, self);
    os_unfair_lock_lock(self.deviceMapLock);
    auto * devices = [self.nodeIDToDeviceMap objectEnumerator].allObjects;
    [_nodeIDToDeviceMap removeAllObjects];
    os_unfair_lock_unlock(self.deviceMapLock);

    for (MTRDevice * device in devices) {
        [device invalidate];
    }
    [self stopBrowseForCommissionables];

    [_factory controllerShuttingDown:self];
}

// Part of cleanupAfterStartup that has to interact with the Matter work queue
// in a very specific way that only MTRDeviceControllerFactory knows about.
- (void)shutDownCppController
{
    MTR_LOG("%s: %p", __PRETTY_FUNCTION__, self);
    assertChipStackLockedByCurrentThread();

    // Shut down all our endpoints.
    for (MTRServerEndpoint * endpoint in [_serverEndpoints copy]) {
        [self removeServerEndpointOnMatterQueue:endpoint];
    }

    if (_cppCommissioner) {
        auto * commissionerToShutDown = _cppCommissioner;
        // Flag ourselves as not running before we start shutting down
        // _cppCommissioner, so we're not in a state where we claim to be
        // running but are actually partially shut down.
        _cppCommissioner = nullptr;
        commissionerToShutDown->Shutdown();
        // Don't clear out our fabric index association until controller
        // shutdown completes, in case it wants to write to storage as it
        // shuts down.
        _storedFabricIndex = chip::kUndefinedFabricIndex;
        _storedCompressedFabricID = std::nullopt;
        self.nodeID = nil;
        self.fabricID = nil;
        self.rootPublicKey = nil;

        delete commissionerToShutDown;
        if (_operationalCredentialsDelegate != nil) {
            _operationalCredentialsDelegate->SetDeviceCommissioner(nullptr);
        }
    }
    _shutdownPending = NO;
}

- (void)deinitFromFactory
{
    [self cleanup];
}

// Clean up any members we might have allocated.
- (void)cleanup
{
    VerifyOrDie(_cppCommissioner == nullptr);

    if (_defaultDACVerifier) {
        delete _defaultDACVerifier;
        _defaultDACVerifier = nullptr;
    }

    if (_attestationTrustStoreBridge) {
        delete _attestationTrustStoreBridge;
        _attestationTrustStoreBridge = nullptr;
    }

    [self clearDeviceAttestationDelegateBridge];

    if (_operationalCredentialsDelegate) {
        delete _operationalCredentialsDelegate;
        _operationalCredentialsDelegate = nullptr;
    }

    if (_partialDACVerifier) {
        delete _partialDACVerifier;
        _partialDACVerifier = nullptr;
    }

    if (_deviceControllerDelegateBridge) {
        delete _deviceControllerDelegateBridge;
        _deviceControllerDelegateBridge = nullptr;
        @synchronized(self) {
            _strongDelegateForSetDelegateAPI = nil;
            [_delegates removeAllObjects];
        }
    }
}

- (BOOL)startup:(MTRDeviceControllerStartupParamsInternal *)startupParams
{
    __block BOOL commissionerInitialized = NO;
    if ([self isRunning]) {
        MTR_LOG_ERROR("%@ Unexpected duplicate call to startup", self);
        return NO;
    }

    dispatch_sync(_chipWorkQueue, ^{
        if ([self isRunning]) {
            return;
        }

        if (startupParams.vendorID == nil || [startupParams.vendorID unsignedShortValue] == chip::VendorId::Common) {
            // Shouldn't be using the "standard" vendor ID for actual devices.
            MTR_LOG_ERROR("%@ %@ is not a valid vendorID to initialize a device controller with", self, startupParams.vendorID);
            return;
        }

        if (startupParams.operationalCertificate == nil && startupParams.nodeID == nil) {
            MTR_LOG_ERROR("%@ Can't start a controller if we don't know what node id it is", self);
            return;
        }

        if ([startupParams keypairsMatchCertificates] == NO) {
            MTR_LOG_ERROR("%@ Provided keypairs do not match certificates", self);
            return;
        }

        if (startupParams.operationalCertificate != nil && startupParams.operationalKeypair == nil
            && (!startupParams.fabricIndex.HasValue()
                || !startupParams.keystore->HasOpKeypairForFabric(startupParams.fabricIndex.Value()))) {
            MTR_LOG_ERROR("%@ Have no operational keypair for our operational certificate", self);
            return;
        }

        CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;

        // create a MTRP256KeypairBridge here and pass it to the operationalCredentialsDelegate
        chip::Crypto::P256Keypair * signingKeypair = nullptr;
        if (startupParams.nocSigner) {
            errorCode = _signingKeypairBridge.Init(startupParams.nocSigner);
            if ([self checkForStartError:errorCode logMsg:kErrorSigningKeypairInit]) {
                return;
            }
            signingKeypair = &_signingKeypairBridge;
        }
        errorCode = _operationalCredentialsDelegate->Init(
            signingKeypair, startupParams.ipk, startupParams.rootCertificate, startupParams.intermediateCertificate);
        if ([self checkForStartError:errorCode logMsg:kErrorOperationalCredentialsInit]) {
            return;
        }

        _cppCommissioner = new chip::Controller::DeviceCommissioner();

        // nocBuffer might not be used, but if it is it needs to live
        // long enough (until after we are done using
        // commissionerParams).
        uint8_t nocBuffer[chip::Controller::kMaxCHIPDERCertLength];

        chip::Controller::SetupParams commissionerParams;

        commissionerParams.pairingDelegate = _deviceControllerDelegateBridge;

        _operationalCredentialsDelegate->SetDeviceCommissioner(_cppCommissioner);

        commissionerParams.operationalCredentialsDelegate = _operationalCredentialsDelegate;

        commissionerParams.controllerRCAC = _operationalCredentialsDelegate->RootCertSpan();
        commissionerParams.controllerICAC = _operationalCredentialsDelegate->IntermediateCertSpan();

        if (startupParams.operationalKeypair != nil) {
            errorCode = _operationalKeypairBridge.Init(startupParams.operationalKeypair);
            if ([self checkForStartError:errorCode logMsg:kErrorOperationalKeypairInit]) {
                return;
            }
            commissionerParams.operationalKeypair = &_operationalKeypairBridge;
            commissionerParams.hasExternallyOwnedOperationalKeypair = true;
        }

        if (startupParams.operationalCertificate) {
            commissionerParams.controllerNOC = AsByteSpan(startupParams.operationalCertificate);
        } else {
            chip::MutableByteSpan noc(nocBuffer);

            chip::CATValues cats = chip::kUndefinedCATs;
            if (startupParams.caseAuthenticatedTags != nil) {
                errorCode = SetToCATValues(startupParams.caseAuthenticatedTags, cats);
                if (errorCode != CHIP_NO_ERROR) {
                    // SetToCATValues already handles logging.
                    return;
                }
            }

            if (commissionerParams.operationalKeypair != nullptr) {
                errorCode = _operationalCredentialsDelegate->GenerateNOC(startupParams.nodeID.unsignedLongLongValue,
                    startupParams.fabricID.unsignedLongLongValue, cats, commissionerParams.operationalKeypair->Pubkey(), noc);

                if ([self checkForStartError:errorCode logMsg:kErrorGenerateNOC]) {
                    return;
                }
            } else {
                // Generate a new random keypair.
                uint8_t csrBuffer[chip::Crypto::kMIN_CSR_Buffer_Size];
                chip::MutableByteSpan csr(csrBuffer);
                errorCode = startupParams.fabricTable->AllocatePendingOperationalKey(startupParams.fabricIndex, csr);
                if ([self checkForStartError:errorCode logMsg:kErrorKeyAllocation]) {
                    return;
                }

                chip::Crypto::P256PublicKey pubKey;
                errorCode = VerifyCertificateSigningRequest(csr.data(), csr.size(), pubKey);
                if ([self checkForStartError:errorCode logMsg:kErrorCSRValidation]) {
                    return;
                }

                errorCode = _operationalCredentialsDelegate->GenerateNOC(
                    startupParams.nodeID.unsignedLongLongValue, startupParams.fabricID.unsignedLongLongValue, cats, pubKey, noc);

                if ([self checkForStartError:errorCode logMsg:kErrorGenerateNOC]) {
                    return;
                }
            }
            commissionerParams.controllerNOC = noc;
        }
        commissionerParams.controllerVendorId = static_cast<chip::VendorId>([startupParams.vendorID unsignedShortValue]);
        commissionerParams.enableServerInteractions = startupParams.advertiseOperational;

        // We never want plain "removal" from the fabric table since this leaves
        // the in-memory state out of sync with what's in storage. In per-controller
        // storage mode, have the controller delete itself from the fabric table on shutdown.
        // In factory storage mode we need to keep fabric information around so we can
        // start another controller on that existing fabric at a later time.
        commissionerParams.removeFromFabricTableOnShutdown = false;
        commissionerParams.deleteFromFabricTableOnShutdown = (startupParams.storageDelegate != nil);

        commissionerParams.permitMultiControllerFabrics = startupParams.allowMultipleControllersPerFabric;

        // Set up our attestation verifier.  Assume we want to use the default
        // one, until something tells us otherwise.
        const chip::Credentials::AttestationTrustStore * trustStore;
        if (startupParams.productAttestationAuthorityCertificates) {
            _attestationTrustStoreBridge
                = new MTRAttestationTrustStoreBridge(startupParams.productAttestationAuthorityCertificates);
            trustStore = _attestationTrustStoreBridge;
        } else {
            // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
            trustStore = chip::Credentials::GetTestAttestationTrustStore();
        }

        _defaultDACVerifier = new chip::Credentials::DefaultDACVerifier(trustStore);

        if (startupParams.certificationDeclarationCertificates) {
            auto cdTrustStore = _defaultDACVerifier->GetCertificationDeclarationTrustStore();
            if (cdTrustStore == nullptr) {
                errorCode = CHIP_ERROR_INCORRECT_STATE;
            }
            if ([self checkForStartError:errorCode logMsg:kErrorCDCertStoreInit]) {
                return;
            }

            for (NSData * cdSigningCert in startupParams.certificationDeclarationCertificates) {
                errorCode = cdTrustStore->AddTrustedKey(AsByteSpan(cdSigningCert));
                if ([self checkForStartError:errorCode logMsg:kErrorCDCertStoreInit]) {
                    return;
                }
            }
        }

        commissionerParams.deviceAttestationVerifier = _defaultDACVerifier;

        auto & factory = chip::Controller::DeviceControllerFactory::GetInstance();

        errorCode = factory.SetupCommissioner(commissionerParams, *_cppCommissioner);
        if ([self checkForStartError:errorCode logMsg:kErrorCommissionerInit]) {
            return;
        }

        chip::FabricIndex fabricIdx = _cppCommissioner->GetFabricIndex();

        uint8_t compressedIdBuffer[sizeof(uint64_t)];
        chip::MutableByteSpan compressedId(compressedIdBuffer);
        errorCode = _cppCommissioner->GetCompressedFabricIdBytes(compressedId);
        if ([self checkForStartError:errorCode logMsg:kErrorIPKInit]) {
            return;
        }

        errorCode = chip::Credentials::SetSingleIpkEpochKey(
            _factory.groupDataProvider, fabricIdx, _operationalCredentialsDelegate->GetIPK(), compressedId);
        if ([self checkForStartError:errorCode logMsg:kErrorIPKInit]) {
            return;
        }

        self->_storedFabricIndex = fabricIdx;
        self->_storedCompressedFabricID = _cppCommissioner->GetCompressedFabricId();

        chip::Crypto::P256PublicKey rootPublicKey;
        if (_cppCommissioner->GetRootPublicKey(rootPublicKey) == CHIP_NO_ERROR) {
            self.rootPublicKey = [NSData dataWithBytes:rootPublicKey.Bytes() length:rootPublicKey.Length()];
            self.nodeID = @(_cppCommissioner->GetNodeId());
            self.fabricID = @(_cppCommissioner->GetFabricId());
        }

        commissionerInitialized = YES;

        MTR_LOG("%@ startup succeeded for nodeID 0x%016llX", self, self->_cppCommissioner->GetNodeId());
    });

    if (commissionerInitialized == NO) {
        MTR_LOG_ERROR("%@ startup failed", self);
        [self cleanupAfterStartup];
        return NO;
    }

    // TODO: Once setNocChainIssuer no longer needs to be supported,
    // we can just move the internals of
    // setOperationalCertificateIssuer into the sync-dispatched block
    // above.
    if (![self setOperationalCertificateIssuer:startupParams.operationalCertificateIssuer
                                         queue:startupParams.operationalCertificateIssuerQueue]) {
        MTR_LOG_ERROR("%@ operationalCertificateIssuer and operationalCertificateIssuerQueue must both be nil or both be non-nil", self);
        [self cleanupAfterStartup];
        return NO;
    }

    if (_controllerDataStore) {
        // If the storage delegate supports the bulk read API, then a dictionary of nodeID => cluster data dictionary would be passed to the handler. Otherwise this would be a no-op, and stored attributes for MTRDevice objects will be loaded lazily in -deviceForNodeID:.
        [_controllerDataStore fetchAttributeDataForAllDevices:^(NSDictionary<NSNumber *, NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *> * _Nonnull clusterDataByNode) {
            MTR_LOG("%@ Loaded attribute values for %lu nodes from storage for controller uuid %@", self, static_cast<unsigned long>(clusterDataByNode.count), self->_uniqueIdentifier);

            std::lock_guard lock(*self.deviceMapLock);
            NSMutableArray * deviceList = [NSMutableArray array];
            for (NSNumber * nodeID in clusterDataByNode) {
                NSDictionary * clusterData = clusterDataByNode[nodeID];
                MTRDevice * device = [self _setupDeviceForNodeID:nodeID prefetchedClusterData:clusterData];
                MTR_LOG("%@ Loaded %lu cluster data from storage for %@", self, static_cast<unsigned long>(clusterData.count), device);

                [deviceList addObject:device];
            }

#define kSecondsToWaitBeforeAPIClientRetainsMTRDevice 60
            // Keep the devices retained for a while, in case API client doesn't immediately retain them.
            //
            // Note that this is just an optimization to avoid throwing the information away and immediately
            // re-reading it from storage.
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (kSecondsToWaitBeforeAPIClientRetainsMTRDevice * NSEC_PER_SEC)), dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
                MTR_LOG("%@ un-retain devices loaded at startup %lu", self, static_cast<unsigned long>(deviceList.count));
            });
        }];
    }
    MTR_LOG("%@ startup: %@", NSStringFromClass(self.class), self);

    return YES;
}

- (NSNumber *)controllerNodeID
{
    auto block = ^NSNumber * { return @(self->_cppCommissioner->GetNodeId()); };

    NSNumber * nodeID = [self syncRunOnWorkQueueWithReturnValue:block error:nil];
    if (!nodeID) {
        MTR_LOG_ERROR("%@ A controller has no node id if it has not been started", self);
    }

    return nodeID;
}

static inline void emitMetricForSetupPayload(MTRSetupPayload * payload)
{
    MATTER_LOG_METRIC(kMetricDeviceVendorID, [payload.vendorID unsignedIntValue]);
    MATTER_LOG_METRIC(kMetricDeviceProductID, [payload.productID unsignedIntValue]);
}

- (BOOL)setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload
                                   newNodeID:(NSNumber *)newNodeID
                                       error:(NSError * __autoreleasing *)error
{
    MTR_LOG("Setting up commissioning session for device ID 0x%016llX with setup payload %@", newNodeID.unsignedLongLongValue, payload);

    [[MTRMetricsCollector sharedInstance] resetMetrics];

    // Track overall commissioning
    MATTER_LOG_METRIC_BEGIN(kMetricDeviceCommissioning);
    emitMetricForSetupPayload(payload);

    // Capture in a block variable to avoid losing granularity for metrics,
    // when translating CHIP_ERROR to NSError
    __block CHIP_ERROR errorCode = CHIP_NO_ERROR;

    auto block = ^BOOL {
        // Track work until end of scope
        MATTER_LOG_METRIC_SCOPE(kMetricSetupWithPayload, errorCode);

        // Try to get a QR code if possible (because it has a better
        // discriminator, etc), then fall back to manual code if that fails.
        NSString * pairingCode = [payload qrCodeString:nil];
        if (pairingCode == nil) {
            pairingCode = [payload manualEntryCode];
        }
        if (pairingCode == nil) {
            errorCode = CHIP_ERROR_INVALID_ARGUMENT;
            return ![MTRDeviceController checkForError:errorCode logMsg:kErrorSetupCodeGen error:error];
        }

        chip::NodeId nodeId = [newNodeID unsignedLongLongValue];
        self->_operationalCredentialsDelegate->SetDeviceID(nodeId);

        MATTER_LOG_METRIC_BEGIN(kMetricSetupPASESession);
        errorCode = self->_cppCommissioner->EstablishPASEConnection(nodeId, [pairingCode UTF8String]);
        if (CHIP_NO_ERROR == errorCode) {
            self->_deviceControllerDelegateBridge->SetDeviceNodeID(nodeId);
        } else {
            MATTER_LOG_METRIC_END(kMetricSetupPASESession, errorCode);
        }

        return ![MTRDeviceController checkForError:errorCode logMsg:kErrorPairDevice error:error];
    };

    auto success = [self syncRunOnWorkQueueWithBoolReturnValue:block error:error];
    if (!success) {
        MATTER_LOG_METRIC_END(kMetricDeviceCommissioning, errorCode);
    }
    return success;
}

- (BOOL)setupCommissioningSessionWithDiscoveredDevice:(MTRCommissionableBrowserResult *)discoveredDevice
                                              payload:(MTRSetupPayload *)payload
                                            newNodeID:(NSNumber *)newNodeID
                                                error:(NSError * __autoreleasing *)error
{
    MTR_LOG("%@ Setting up commissioning session for already-discovered device %@ and device ID 0x%016llX with setup payload %@", self, discoveredDevice, newNodeID.unsignedLongLongValue, payload);

    [[MTRMetricsCollector sharedInstance] resetMetrics];

    // Track overall commissioning
    MATTER_LOG_METRIC_BEGIN(kMetricDeviceCommissioning);
    emitMetricForSetupPayload(payload);

    // Capture in a block variable to avoid losing granularity for metrics,
    // when translating CHIP_ERROR to NSError
    __block CHIP_ERROR errorCode = CHIP_NO_ERROR;

    auto block = ^BOOL {
        // Track work until end of scope
        MATTER_LOG_METRIC_SCOPE(kMetricSetupWithDiscovered, errorCode);

        chip::NodeId nodeId = [newNodeID unsignedLongLongValue];
        self->_operationalCredentialsDelegate->SetDeviceID(nodeId);

        errorCode = CHIP_ERROR_INVALID_ARGUMENT;
        chip::Optional<chip::Controller::SetUpCodePairerParameters> params = discoveredDevice.params;
        if (params.HasValue()) {
            auto pinCode = static_cast<uint32_t>(payload.setupPasscode.unsignedLongValue);
            params.Value().SetSetupPINCode(pinCode);

            MATTER_LOG_METRIC_BEGIN(kMetricSetupPASESession);
            errorCode = self->_cppCommissioner->EstablishPASEConnection(nodeId, params.Value());
            if (CHIP_NO_ERROR == errorCode) {
                self->_deviceControllerDelegateBridge->SetDeviceNodeID(nodeId);
            } else {
                MATTER_LOG_METRIC_END(kMetricSetupPASESession, errorCode);
            }
        } else {
            // Try to get a QR code if possible (because it has a better
            // discriminator, etc), then fall back to manual code if that fails.
            NSString * pairingCode = [payload qrCodeString:nil];
            if (pairingCode == nil) {
                pairingCode = [payload manualEntryCode];
            }
            if (pairingCode == nil) {
                errorCode = CHIP_ERROR_INVALID_ARGUMENT;
                return ![MTRDeviceController checkForError:errorCode logMsg:kErrorSetupCodeGen error:error];
            }

            for (id key in discoveredDevice.interfaces) {
                auto resolutionData = discoveredDevice.interfaces[key].resolutionData;
                if (!resolutionData.HasValue()) {
                    continue;
                }

                MATTER_LOG_METRIC_BEGIN(kMetricSetupPASESession);
                errorCode = self->_cppCommissioner->EstablishPASEConnection(
                    nodeId, [pairingCode UTF8String], chip::Controller::DiscoveryType::kDiscoveryNetworkOnly, resolutionData);
                if (CHIP_NO_ERROR == errorCode) {
                    self->_deviceControllerDelegateBridge->SetDeviceNodeID(nodeId);
                } else {
                    MATTER_LOG_METRIC_END(kMetricSetupPASESession, errorCode);
                    break;
                }
            }
        }

        return ![MTRDeviceController checkForError:errorCode logMsg:kErrorPairDevice error:error];
    };

    auto success = [self syncRunOnWorkQueueWithBoolReturnValue:block error:error];
    if (!success) {
        MATTER_LOG_METRIC_END(kMetricDeviceCommissioning, errorCode);
    }
    return success;
}

- (BOOL)commissionNodeWithID:(NSNumber *)nodeID
         commissioningParams:(MTRCommissioningParameters *)commissioningParams
                       error:(NSError * __autoreleasing *)error
{
    auto block = ^BOOL {
        chip::Controller::CommissioningParameters params;
        if (commissioningParams.csrNonce) {
            params.SetCSRNonce(AsByteSpan(commissioningParams.csrNonce));
        }
        if (commissioningParams.attestationNonce) {
            params.SetAttestationNonce(AsByteSpan(commissioningParams.attestationNonce));
        }
        if (commissioningParams.threadOperationalDataset) {
            params.SetThreadOperationalDataset(AsByteSpan(commissioningParams.threadOperationalDataset));
        }
        params.SetSkipCommissioningComplete(commissioningParams.skipCommissioningComplete);
        if (commissioningParams.wifiSSID) {
            chip::ByteSpan ssid = AsByteSpan(commissioningParams.wifiSSID);
            chip::ByteSpan credentials;
            if (commissioningParams.wifiCredentials != nil) {
                credentials = AsByteSpan(commissioningParams.wifiCredentials);
            }
            chip::Controller::WiFiCredentials wifiCreds(ssid, credentials);
            params.SetWiFiCredentials(wifiCreds);
        }
        if (commissioningParams.deviceAttestationDelegate) {
            [self clearDeviceAttestationDelegateBridge];

            chip::Optional<uint16_t> timeoutSecs;
            if (commissioningParams.failSafeTimeout) {
                timeoutSecs = chip::MakeOptional(static_cast<uint16_t>([commissioningParams.failSafeTimeout unsignedIntValue]));
            }
            BOOL shouldWaitAfterDeviceAttestation = NO;
            if ([commissioningParams.deviceAttestationDelegate
                    respondsToSelector:@selector(deviceAttestationCompletedForController:
                                                                      opaqueDeviceHandle:attestationDeviceInfo:error:)]
                || [commissioningParams.deviceAttestationDelegate
                    respondsToSelector:@selector(deviceAttestation:completedForDevice:attestationDeviceInfo:error:)]) {
                shouldWaitAfterDeviceAttestation = YES;
            }
            self->_deviceAttestationDelegateBridge = new MTRDeviceAttestationDelegateBridge(
                self, commissioningParams.deviceAttestationDelegate, timeoutSecs, shouldWaitAfterDeviceAttestation);
            params.SetDeviceAttestationDelegate(self->_deviceAttestationDelegateBridge);
        }
        if (commissioningParams.countryCode != nil) {
            params.SetCountryCode(AsCharSpan(commissioningParams.countryCode));
        }

        // Set up the right timezone and DST information.  For timezone, just
        // use our current timezone and don't schedule any sort of timezone
        // change.
        auto * tz = [NSTimeZone localTimeZone];
        using TimeZoneType = chip::app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type;
        TimeZoneType timeZone;
        timeZone.validAt = 0;
        timeZone.offset = static_cast<int32_t>(tz.secondsFromGMT - tz.daylightSavingTimeOffset);
        timeZone.name.Emplace(AsCharSpan(tz.name));

        params.SetTimeZone(chip::app::DataModel::List<TimeZoneType>(&timeZone, 1));

        // For DST, there is no limit to the number of transitions we could try
        // to add, but in practice devices likely support only 2 and
        // AutoCommissioner caps the list at 10.  Let's do up to 4 transitions
        // for now.
        constexpr size_t dstOffsetMaxCount = 4;
        using DSTOffsetType = chip::app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type;
        // dstOffsets needs to live long enough, so its existence is not
        // conditional on having offsets.
        DSTOffsetType dstOffsets[dstOffsetMaxCount];

        auto * offsets = MTRComputeDSTOffsets(dstOffsetMaxCount);
        if (offsets != nil) {
            size_t dstOffsetCount = 0;
            for (MTRTimeSynchronizationClusterDSTOffsetStruct * offset in offsets) {
                if (dstOffsetCount >= dstOffsetMaxCount) {
                    // Really shouldn't happen, but let's be extra careful about
                    // buffer overruns.
                    break;
                }
                auto & targetOffset = dstOffsets[dstOffsetCount];
                targetOffset.offset = offset.offset.intValue;
                targetOffset.validStarting = offset.validStarting.unsignedLongLongValue;
                if (offset.validUntil == nil) {
                    targetOffset.validUntil.SetNull();
                } else {
                    targetOffset.validUntil.SetNonNull(offset.validUntil.unsignedLongLongValue);
                }
                ++dstOffsetCount;
            }

            params.SetDSTOffsets(chip::app::DataModel::List<DSTOffsetType>(dstOffsets, dstOffsetCount));
        }

        chip::NodeId deviceId = [nodeID unsignedLongLongValue];
        self->_operationalCredentialsDelegate->SetDeviceID(deviceId);
        auto errorCode = self->_cppCommissioner->Commission(deviceId, params);
        MATTER_LOG_METRIC(kMetricCommissionNode, errorCode);
        return ![MTRDeviceController checkForError:errorCode logMsg:kErrorPairDevice error:error];
    };

    return [self syncRunOnWorkQueueWithBoolReturnValue:block error:error];
}

- (BOOL)continueCommissioningDevice:(void *)device
           ignoreAttestationFailure:(BOOL)ignoreAttestationFailure
                              error:(NSError * __autoreleasing *)error
{
    auto block = ^BOOL {
        auto lastAttestationResult = self->_deviceAttestationDelegateBridge
            ? self->_deviceAttestationDelegateBridge->attestationVerificationResult()
            : chip::Credentials::AttestationVerificationResult::kSuccess;

        auto deviceProxy = static_cast<chip::DeviceProxy *>(device);
        auto errorCode = self->_cppCommissioner->ContinueCommissioningAfterDeviceAttestation(deviceProxy,
            ignoreAttestationFailure ? chip::Credentials::AttestationVerificationResult::kSuccess : lastAttestationResult);
        // Emit metric on stage after continuing post attestation
        MATTER_LOG_METRIC(kMetricContinueCommissioningAfterAttestation, errorCode);
        return ![MTRDeviceController checkForError:errorCode logMsg:kErrorPairDevice error:error];
    };

    return [self syncRunOnWorkQueueWithBoolReturnValue:block error:error];
}

- (BOOL)cancelCommissioningForNodeID:(NSNumber *)nodeID error:(NSError * __autoreleasing *)error
{
    auto block = ^BOOL {
        self->_operationalCredentialsDelegate->ResetDeviceID();
        auto errorCode = self->_cppCommissioner->StopPairing([nodeID unsignedLongLongValue]);
        // Emit metric on status of cancel
        MATTER_LOG_METRIC(kMetricCancelCommissioning, errorCode);
        return ![MTRDeviceController checkForError:errorCode logMsg:kErrorStopPairing error:error];
    };

    return [self syncRunOnWorkQueueWithBoolReturnValue:block error:error];
}

- (BOOL)startBrowseForCommissionables:(id<MTRCommissionableBrowserDelegate>)delegate queue:(dispatch_queue_t)queue
{
    auto block = ^BOOL {
        VerifyOrReturnValueWithMetric(kMetricStartBrowseForCommissionables, self->_commissionableBrowser == nil, NO);

        auto commissionableBrowser = [[MTRCommissionableBrowser alloc] initWithDelegate:delegate controller:self queue:queue];
        VerifyOrReturnValueWithMetric(kMetricStartBrowseForCommissionables, [commissionableBrowser start], NO);

        self->_commissionableBrowser = commissionableBrowser;
        return YES;
    };

    return [self syncRunOnWorkQueueWithBoolReturnValue:block error:nil];
}

- (BOOL)stopBrowseForCommissionables
{
    auto block = ^BOOL {
        VerifyOrReturnValueWithMetric(kMetricStopBrowseForCommissionables, self->_commissionableBrowser != nil, NO);

        auto commissionableBrowser = self->_commissionableBrowser;
        VerifyOrReturnValueWithMetric(kMetricStopBrowseForCommissionables, [commissionableBrowser stop], NO);

        self->_commissionableBrowser = nil;
        return YES;
    };

    return [self syncRunOnWorkQueueWithBoolReturnValue:block error:nil];
}

- (void)preWarmCommissioningSession
{
    [_factory preWarmCommissioningSession];
}

- (MTRBaseDevice *)deviceBeingCommissionedWithNodeID:(NSNumber *)nodeID error:(NSError * __autoreleasing *)error
{
    auto block = ^MTRBaseDevice *
    {
        chip::CommissioneeDeviceProxy * deviceProxy;

        auto errorCode = self->_cppCommissioner->GetDeviceBeingCommissioned(nodeID.unsignedLongLongValue, &deviceProxy);
        MATTER_LOG_METRIC(kMetricDeviceBeingCommissioned, errorCode);

        VerifyOrReturnValue(![MTRDeviceController checkForError:errorCode logMsg:kErrorGetCommissionee error:error], nil);

        return [[MTRBaseDevice alloc] initWithPASEDevice:deviceProxy controller:self];
    };

    MTRBaseDevice * device = [self syncRunOnWorkQueueWithReturnValue:block error:error];
    MTR_LOG("%@ Getting device being commissioned with node ID 0x%016llX: %@ (error: %@)", self, nodeID.unsignedLongLongValue, device, (error ? *error : nil));
    return device;
}

- (MTRBaseDevice *)baseDeviceForNodeID:(NSNumber *)nodeID
{
    return [[MTRBaseDevice alloc] initWithNodeID:nodeID controller:self];
}

// If prefetchedClusterData is not provided, load attributes individually from controller data store
- (MTRDevice *)_setupDeviceForNodeID:(NSNumber *)nodeID prefetchedClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)prefetchedClusterData
{
    os_unfair_lock_assert_owner(self.deviceMapLock);

    MTRDevice * deviceToReturn = [[MTRDevice_Concrete alloc] initWithNodeID:nodeID controller:self];
    // If we're not running, don't add the device to our map.  That would
    // create a cycle that nothing would break.  Just return the device,
    // which will be in exactly the state it would be in if it were created
    // while we were running and then we got shut down.
    if ([self isRunning]) {
        [_nodeIDToDeviceMap setObject:deviceToReturn forKey:nodeID];
    }

    if (prefetchedClusterData) {
        if (prefetchedClusterData.count) {
            [deviceToReturn setPersistedClusterData:prefetchedClusterData];
        }
    } else if (_controllerDataStore) {
        // Load persisted cluster data if they exist.
        NSDictionary * clusterData = [_controllerDataStore getStoredClusterDataForNodeID:nodeID];
        MTR_LOG("%@ Loaded %lu cluster data from storage for %@", self, static_cast<unsigned long>(clusterData.count), deviceToReturn);
        if (clusterData.count) {
            [deviceToReturn setPersistedClusterData:clusterData];
        }
    }

    // TODO: Figure out how to get the device data as part of our bulk-read bits.
    if (_controllerDataStore) {
        auto * deviceData = [_controllerDataStore getStoredDeviceDataForNodeID:nodeID];
        if (deviceData.count) {
            [deviceToReturn setPersistedDeviceData:deviceData];
        }
    }

    [deviceToReturn setStorageBehaviorConfiguration:_storageBehaviorConfiguration];

    return deviceToReturn;
}

- (MTRDevice *)deviceForNodeID:(NSNumber *)nodeID
{
    std::lock_guard lock(*self.deviceMapLock);
    MTRDevice * deviceToReturn = [_nodeIDToDeviceMap objectForKey:nodeID];
    if (!deviceToReturn) {
        deviceToReturn = [self _setupDeviceForNodeID:nodeID prefetchedClusterData:nil];
    }

    return deviceToReturn;
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

#ifdef DEBUG
- (NSDictionary<NSNumber *, NSNumber *> *)unitTestGetDeviceAttributeCounts
{
    std::lock_guard lock(*self.deviceMapLock);
    NSMutableDictionary<NSNumber *, NSNumber *> * deviceAttributeCounts = [NSMutableDictionary dictionary];
    for (NSNumber * nodeID in _nodeIDToDeviceMap) {
        deviceAttributeCounts[nodeID] = @([[_nodeIDToDeviceMap objectForKey:nodeID] unitTestAttributeCount]);
    }
    return deviceAttributeCounts;
}
#endif

- (BOOL)setOperationalCertificateIssuer:(nullable id<MTROperationalCertificateIssuer>)operationalCertificateIssuer
                                  queue:(nullable dispatch_queue_t)queue
{
    if ((operationalCertificateIssuer != nil && queue == nil) || (operationalCertificateIssuer == nil && queue != nil)) {
        return NO;
    }

    auto block = ^{
        BOOL usePartialDACVerifier = NO;
        if (operationalCertificateIssuer != nil) {
            self->_operationalCredentialsDelegate->SetOperationalCertificateIssuer(operationalCertificateIssuer, queue);
            usePartialDACVerifier = operationalCertificateIssuer.shouldSkipAttestationCertificateValidation;
        }
        if (usePartialDACVerifier) {
            self->_cppCommissioner->SetDeviceAttestationVerifier(self->_partialDACVerifier);
        } else {
            // TODO: Once we are not supporting setNocChainIssuer this
            // branch can just go away.
            self->_cppCommissioner->SetDeviceAttestationVerifier(self->_defaultDACVerifier);
        }
        return YES;
    };

    return [self syncRunOnWorkQueueWithBoolReturnValue:block error:nil];
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
    auto block = ^NSData *
    {
        chip::CommissioneeDeviceProxy * deviceProxy;

        auto errorCode = CHIP_NO_ERROR;
        MATTER_LOG_METRIC_SCOPE(kMetricAttestationChallengeForDevice, errorCode);

        errorCode = self->_cppCommissioner->GetDeviceBeingCommissioned([deviceID unsignedLongLongValue], &deviceProxy);
        VerifyOrReturnValue(![MTRDeviceController checkForError:errorCode logMsg:kErrorGetCommissionee error:nil], nil);

        uint8_t challengeBuffer[chip::Crypto::kAES_CCM128_Key_Length];
        chip::ByteSpan challenge(challengeBuffer);

        errorCode = deviceProxy->GetAttestationChallenge(challenge);
        VerifyOrReturnValue(![MTRDeviceController checkForError:errorCode logMsg:kErrorGetAttestationChallenge error:nil], nil);

        return AsData(challenge);
    };

    return [self syncRunOnWorkQueueWithReturnValue:block error:nil];
}

- (BOOL)addServerEndpoint:(MTRServerEndpoint *)endpoint
{
    VerifyOrReturnValue([self checkIsRunning], NO);

    if (![_factory addServerEndpoint:endpoint]) {
        return NO;
    }

    if (![endpoint associateWithController:self]) {
        MTR_LOG_ERROR("%@ Failed to associate MTRServerEndpoint with %@", self, NSStringFromClass(self.class));
        [_factory removeServerEndpoint:endpoint];
        return NO;
    }

    [self asyncDispatchToMatterQueue:^() {
        [self->_serverEndpoints addObject:endpoint];
        [endpoint registerMatterEndpoint];
        MTR_LOG("%@ Added server endpoint %u to controller %@", self, static_cast<chip::EndpointId>(endpoint.endpointID.unsignedLongLongValue),
            self->_uniqueIdentifier);
    }
        errorHandler:^(NSError * error) {
            MTR_LOG_ERROR("%@ Unexpected failure dispatching to Matter queue on running controller in addServerEndpoint, adding endpoint %u", self,
                static_cast<chip::EndpointId>(endpoint.endpointID.unsignedLongLongValue));
        }];
    return YES;
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
    VerifyOrReturn([self checkIsRunning]);

    // We need to unhook the endpoint from the Matter side before we can start
    // tearing it down.
    [self asyncDispatchToMatterQueue:^() {
        [self removeServerEndpointOnMatterQueue:endpoint];
        MTR_LOG("%@ Removed server endpoint %u from controller %@", self, static_cast<chip::EndpointId>(endpoint.endpointID.unsignedLongLongValue),
            self->_uniqueIdentifier);
        if (queue != nil && completion != nil) {
            dispatch_async(queue, completion);
        }
    }
        errorHandler:^(NSError * error) {
            // Error means we got shut down, so the endpoint is removed now.
            MTR_LOG("%@ controller already shut down, so endpoint %u has already been removed", self,
                static_cast<chip::EndpointId>(endpoint.endpointID.unsignedLongLongValue));
            if (queue != nil && completion != nil) {
                dispatch_async(queue, completion);
            }
        }];
}

- (void)removeServerEndpointOnMatterQueue:(MTRServerEndpoint *)endpoint
{
    assertChipStackLockedByCurrentThread();

    [endpoint unregisterMatterEndpoint];
    [_serverEndpoints removeObject:endpoint];
    [endpoint invalidate];

    [_factory removeServerEndpoint:endpoint];
}

- (BOOL)checkForInitError:(BOOL)condition logMsg:(NSString *)logMsg
{
    if (condition) {
        return NO;
    }

    MTR_LOG_ERROR("%@ Error: %@", self, logMsg);

    [self cleanup];

    return YES;
}

- (void)clearDeviceAttestationDelegateBridge
{
    if (_deviceAttestationDelegateBridge) {
        delete _deviceAttestationDelegateBridge;
        _deviceAttestationDelegateBridge = nullptr;
    }
}

- (BOOL)checkForStartError:(CHIP_ERROR)errorCode logMsg:(NSString *)logMsg
{
    if (CHIP_NO_ERROR == errorCode) {
        return NO;
    }

    MTR_LOG_ERROR("Error(%" CHIP_ERROR_FORMAT "): %@ %@", errorCode.Format(), self, logMsg);

    return YES;
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

- (BOOL)checkIsRunning
{
    return [self checkIsRunning:nil];
}

- (BOOL)checkIsRunning:(NSError * __autoreleasing *)error
{
    if ([self isRunning]) {
        return YES;
    }

    MTR_LOG_ERROR("%@: %@ Error: %s", NSStringFromClass(self.class), self, [kErrorNotRunning UTF8String]);
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }

    return NO;
}

- (void)getSessionForNode:(chip::NodeId)nodeID completion:(MTRInternalDeviceConnectionCallback)completion
{
    // Get the corresponding MTRDevice object to determine if the case/subscription pool is to be used
    MTRDevice * device = [self deviceForNodeID:@(nodeID)];

    // In the case that this device is known to use thread, queue this with subscription attempts as well, to
    // help with throttling Thread traffic.
    if ([device deviceUsesThread]) {
        MTRAsyncWorkItem * workItem = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
        [workItem setReadyHandler:^(id _Nonnull context, NSInteger retryCount, MTRAsyncWorkCompletionBlock _Nonnull workItemCompletion) {
            MTRInternalDeviceConnectionCallback completionWrapper = ^(chip::Messaging::ExchangeManager * _Nullable exchangeManager,
                const chip::Optional<chip::SessionHandle> & session, NSError * _Nullable error, NSNumber * _Nullable retryDelay) {
                completion(exchangeManager, session, error, retryDelay);
                workItemCompletion(MTRAsyncWorkComplete);
            };
            [self directlyGetSessionForNode:nodeID completion:completionWrapper];
        }];

        [_concurrentSubscriptionPool enqueueWorkItem:workItem descriptionWithFormat:@"device controller getSessionForNode nodeID: 0x%016llX", nodeID];
    } else {
        [self directlyGetSessionForNode:nodeID completion:completion];
    }
}

- (void)directlyGetSessionForNode:(chip::NodeId)nodeID completion:(MTRInternalDeviceConnectionCallback)completion
{
    [self
        asyncGetCommissionerOnMatterQueue:^(chip::Controller::DeviceCommissioner * commissioner) {
            auto connectionBridge = new MTRDeviceConnectionBridge(completion);

            // MTRDeviceConnectionBridge always delivers errors async via
            // completion.
            connectionBridge->connect(commissioner, nodeID);
        }
        errorHandler:^(NSError * error) {
            completion(nullptr, chip::NullOptional, error, nil);
        }];
}

- (void)getSessionForCommissioneeDevice:(chip::NodeId)deviceID completion:(MTRInternalDeviceConnectionCallback)completion
{
    [self
        asyncGetCommissionerOnMatterQueue:^(chip::Controller::DeviceCommissioner * commissioner) {
            chip::CommissioneeDeviceProxy * deviceProxy;
            CHIP_ERROR err = commissioner->GetDeviceBeingCommissioned(deviceID, &deviceProxy);
            if (err != CHIP_NO_ERROR) {
                completion(nullptr, chip::NullOptional, [MTRError errorForCHIPErrorCode:err], nil);
                return;
            }

            chip::Optional<chip::SessionHandle> session = deviceProxy->GetSecureSession();
            if (!session.HasValue() || !session.Value()->AsSecureSession()->IsPASESession()) {
                completion(nullptr, chip::NullOptional, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE], nil);
                return;
            }

            completion(deviceProxy->GetExchangeManager(), session, nil, nil);
        }
        errorHandler:^(NSError * error) {
            completion(nullptr, chip::NullOptional, error, nil);
        }];
}

- (MTRTransportType)sessionTransportTypeForDevice:(MTRBaseDevice *)device
{
    VerifyOrReturnValue([self checkIsRunning], MTRTransportTypeUndefined);

    __block MTRTransportType result = MTRTransportTypeUndefined;
    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning]);

        if (device.isPASEDevice) {
            chip::CommissioneeDeviceProxy * deviceProxy;
            VerifyOrReturn(CHIP_NO_ERROR == self->_cppCommissioner->GetDeviceBeingCommissioned(device.nodeID, &deviceProxy));
            result = MTRMakeTransportType(deviceProxy->GetDeviceTransportType());
        } else {
            auto scopedNodeID = self->_cppCommissioner->GetPeerScopedId(device.nodeID);
            auto sessionHandle = self->_cppCommissioner->SessionMgr()->FindSecureSessionForNode(scopedNodeID);
            VerifyOrReturn(sessionHandle.HasValue());
            result = MTRMakeTransportType(sessionHandle.Value()->AsSecureSession()->GetPeerAddress().GetTransportType());
        }
    });
    return result;
}

- (void)asyncGetCommissionerOnMatterQueue:(void (^)(chip::Controller::DeviceCommissioner *))block
                             errorHandler:(nullable MTRDeviceErrorHandler)errorHandler
{
    {
        NSError * error;
        if (![self checkIsRunning:&error]) {
            if (errorHandler != nil) {
                errorHandler(error);
            }
            return;
        }
    }

    dispatch_async(_chipWorkQueue, ^{
        NSError * error;
        if (![self checkIsRunning:&error]) {
            if (errorHandler != nil) {
                errorHandler(error);
            }
            return;
        }

        block(self->_cppCommissioner);
    });
}

- (void)asyncDispatchToMatterQueue:(dispatch_block_t)block errorHandler:(nullable MTRDeviceErrorHandler)errorHandler
{
    auto adapter = ^(chip::Controller::DeviceCommissioner *) {
        block();
    };
    [self asyncGetCommissionerOnMatterQueue:adapter errorHandler:errorHandler];
}

- (void)syncRunOnWorkQueue:(SyncWorkQueueBlock)block error:(NSError * __autoreleasing *)error
{
    VerifyOrDie(!chip::DeviceLayer::PlatformMgrImpl().IsWorkQueueCurrentQueue());
    VerifyOrReturn([self checkIsRunning:error]);

    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning:error]);
        block();
    });
}

- (id)syncRunOnWorkQueueWithReturnValue:(SyncWorkQueueBlockWithReturnValue)block error:(NSError * __autoreleasing *)error
{
    __block id rv = nil;
    auto adapter = ^{
        rv = block();
    };

    [self syncRunOnWorkQueue:adapter error:error];

    return rv;
}

- (BOOL)syncRunOnWorkQueueWithBoolReturnValue:(SyncWorkQueueBlockWithBoolReturnValue)block error:(NSError * __autoreleasing *)error
{
    __block BOOL success = NO;
    auto adapter = ^{
        success = block();
    };
    [self syncRunOnWorkQueue:adapter error:error];

    return success;
}

- (chip::FabricIndex)fabricIndex
{
    return _storedFabricIndex;
}

- (nullable NSNumber *)compressedFabricID
{
    auto storedValue = _storedCompressedFabricID.load();
    return storedValue.has_value() ? @(storedValue.value()) : nil;
}

- (CHIP_ERROR)isRunningOnFabric:(chip::FabricTable *)fabricTable
                    fabricIndex:(chip::FabricIndex)fabricIndex
                      isRunning:(BOOL *)isRunning
{
    assertChipStackLockedByCurrentThread();

    if (![self isRunning]) {
        *isRunning = NO;
        return CHIP_NO_ERROR;
    }

    const chip::FabricInfo * otherFabric = fabricTable->FindFabricWithIndex(fabricIndex);
    if (!otherFabric) {
        // Should not happen...
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (_cppCommissioner->GetFabricId() != otherFabric->GetFabricId()) {
        *isRunning = NO;
        return CHIP_NO_ERROR;
    }

    chip::Crypto::P256PublicKey ourRootPublicKey, otherRootPublicKey;
    ReturnErrorOnFailure(_cppCommissioner->GetRootPublicKey(ourRootPublicKey));
    ReturnErrorOnFailure(fabricTable->FetchRootPubkey(otherFabric->GetFabricIndex(), otherRootPublicKey));

    *isRunning = (ourRootPublicKey.Matches(otherRootPublicKey));
    return CHIP_NO_ERROR;
}

- (void)invalidateCASESessionForNode:(chip::NodeId)nodeID;
{
    auto block = ^{
        auto sessionMgr = self->_cppCommissioner->SessionMgr();
        VerifyOrDie(sessionMgr != nullptr);

        sessionMgr->MarkSessionsAsDefunct(
            self->_cppCommissioner->GetPeerScopedId(nodeID), chip::MakeOptional(chip::Transport::SecureSession::Type::kCASE));
    };

    [self syncRunOnWorkQueue:block error:nil];
}

- (void)operationalInstanceAdded:(chip::NodeId)nodeID
{
    // Don't use deviceForNodeID here, because we don't want to create the
    // device if it does not already exist.
    os_unfair_lock_lock(self.deviceMapLock);
    MTRDevice * device = [_nodeIDToDeviceMap objectForKey:@(nodeID)];
    os_unfair_lock_unlock(self.deviceMapLock);

    if (device == nil) {
        return;
    }

    ChipLogProgress(Controller, "Notifying device about node 0x" ChipLogFormatX64 " advertising", ChipLogValueX64(nodeID));
    [device nodeMayBeAdvertisingOperational];
}

- (void)downloadLogFromNodeWithID:(NSNumber *)nodeID
                             type:(MTRDiagnosticLogType)type
                          timeout:(NSTimeInterval)timeout
                            queue:(dispatch_queue_t)queue
                       completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
{
    [self asyncDispatchToMatterQueue:^() {
        [self->_factory downloadLogFromNodeWithID:nodeID
                                       controller:self
                                             type:type
                                          timeout:timeout
                                            queue:queue
                                       completion:completion];
    }
        errorHandler:^(NSError * error) {
            completion(nil, error);
        }];
}

- (NSArray<MTRAccessGrant *> *)accessGrantsForClusterPath:(MTRClusterPath *)clusterPath
{
    assertChipStackLockedByCurrentThread();

    for (MTRServerEndpoint * endpoint in _serverEndpoints) {
        if ([clusterPath.endpoint isEqual:endpoint.endpointID]) {
            return [endpoint matterAccessGrantsForCluster:clusterPath.cluster];
        }
    }

    // Nothing matched, no grants.
    return @[];
}

- (nullable NSNumber *)neededReadPrivilegeForClusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID
{
    assertChipStackLockedByCurrentThread();

    for (MTRServerEndpoint * endpoint in _serverEndpoints) {
        for (MTRServerCluster * cluster in endpoint.serverClusters) {
            if (![cluster.clusterID isEqual:clusterID]) {
                continue;
            }

            for (MTRServerAttribute * attr in cluster.attributes) {
                if (![attr.attributeID isEqual:attributeID]) {
                    continue;
                }

                return @(attr.requiredReadPrivilege);
            }
        }
    }

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

- (void)_callDelegatesWithBlock:(void (^_Nullable)(id<MTRDeviceControllerDelegate> delegate))block logString:(const char *)logString;
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

- (void)controller:(MTRDeviceController *)controller readCommissioningInfo:(MTRProductIdentity *)info
{
    [self _callDelegatesWithBlock:^(id<MTRDeviceControllerDelegate> delegate) {
        if ([delegate respondsToSelector:@selector(controller:readCommissioningInfo:)]) {
            [delegate controller:controller readCommissioningInfo:info];
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
    NSError * error;
    if (![self checkIsRunning:&error]) {
        dispatch_async(queue, ^{
            completion(nil, error);
        });
        return NO;
    }

    // We know getSessionForNode will return YES here, since we already checked
    // that we are running.
    [self getSessionForNode:deviceID
                 completion:^(chip::Messaging::ExchangeManager * _Nullable exchangeManager,
                     const chip::Optional<chip::SessionHandle> & session, NSError * _Nullable error, NSNumber * _Nullable retryDelay) {
                     // Create an MTRBaseDevice for the node id involved, now that our
                     // CASE session is primed.  We don't actually care about the session
                     // information here.
                     dispatch_async(queue, ^{
                         MTRBaseDevice * device;
                         if (error == nil) {
                             device = [[MTRBaseDevice alloc] initWithNodeID:@(deviceID) controller:self];
                         } else {
                             device = nil;
                         }
                         completion(device, error);
                     });
                 }];

    return YES;
}

- (BOOL)pairDevice:(uint64_t)deviceID
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
{
    [[MTRMetricsCollector sharedInstance] resetMetrics];

    // Track overall commissioning
    MATTER_LOG_METRIC_BEGIN(kMetricDeviceCommissioning);

    // Capture in a block variable to avoid losing granularity for metrics,
    // when translating CHIP_ERROR to NSError
    __block CHIP_ERROR errorCode = CHIP_NO_ERROR;

    auto block = ^BOOL {
        // Track work until end of scope
        MATTER_LOG_METRIC_SCOPE(kMetricPairDevice, errorCode);

        std::string manualPairingCode;
        chip::SetupPayload payload;
        payload.discriminator.SetLongValue(discriminator);
        payload.setUpPINCode = setupPINCode;

        errorCode = chip::ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualPairingCode);
        VerifyOrReturnValue(![MTRDeviceController checkForError:errorCode logMsg:kErrorSetupCodeGen error:error], NO);

        self->_operationalCredentialsDelegate->SetDeviceID(deviceID);

        MATTER_LOG_METRIC_BEGIN(kMetricSetupPASESession);
        errorCode = self->_cppCommissioner->EstablishPASEConnection(deviceID, manualPairingCode.c_str());
        if (CHIP_NO_ERROR == errorCode) {
            self->_deviceControllerDelegateBridge->SetDeviceNodeID(deviceID);
        } else {
            MATTER_LOG_METRIC_END(kMetricSetupPASESession, errorCode);
        }

        return ![MTRDeviceController checkForError:errorCode logMsg:kErrorPairDevice error:error];
    };

    auto success = [self syncRunOnWorkQueueWithBoolReturnValue:block error:error];
    if (!success) {
        MATTER_LOG_METRIC_END(kMetricDeviceCommissioning, errorCode);
    }
    return success;
}

- (BOOL)pairDevice:(uint64_t)deviceID
           address:(NSString *)address
              port:(uint16_t)port
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
{
    [[MTRMetricsCollector sharedInstance] resetMetrics];

    // Track overall commissioning
    MATTER_LOG_METRIC_BEGIN(kMetricDeviceCommissioning);

    // Capture in a block variable to avoid losing granularity for metrics,
    // when translating CHIP_ERROR to NSError
    __block CHIP_ERROR errorCode = CHIP_NO_ERROR;

    auto block = ^BOOL {
        // Track work until end of scope
        MATTER_LOG_METRIC_SCOPE(kMetricPairDevice, errorCode);

        chip::Inet::IPAddress addr;
        chip::Inet::IPAddress::FromString([address UTF8String], addr);
        chip::Transport::PeerAddress peerAddress = chip::Transport::PeerAddress::UDP(addr, port);

        self->_operationalCredentialsDelegate->SetDeviceID(deviceID);

        auto params = chip::RendezvousParameters().SetSetupPINCode(setupPINCode).SetPeerAddress(peerAddress);

        MATTER_LOG_METRIC_BEGIN(kMetricSetupPASESession);
        errorCode = self->_cppCommissioner->EstablishPASEConnection(deviceID, params);
        if (CHIP_NO_ERROR == errorCode) {
            self->_deviceControllerDelegateBridge->SetDeviceNodeID(deviceID);
        } else {
            MATTER_LOG_METRIC_END(kMetricSetupPASESession, errorCode);
        }

        return ![MTRDeviceController checkForError:errorCode logMsg:kErrorPairDevice error:error];
    };

    auto success = [self syncRunOnWorkQueueWithBoolReturnValue:block error:error];
    if (!success) {
        MATTER_LOG_METRIC_END(kMetricDeviceCommissioning, errorCode);
    }
    return success;
}

- (BOOL)pairDevice:(uint64_t)deviceID onboardingPayload:(NSString *)onboardingPayload error:(NSError * __autoreleasing *)error
{
    [[MTRMetricsCollector sharedInstance] resetMetrics];

    // Track overall commissioning
    MATTER_LOG_METRIC_BEGIN(kMetricDeviceCommissioning);
    emitMetricForSetupPayload([MTRSetupPayload setupPayloadWithOnboardingPayload:onboardingPayload error:nil]);

    // Capture in a block variable to avoid losing granularity for metrics,
    // when translating CHIP_ERROR to NSError
    __block CHIP_ERROR errorCode = CHIP_NO_ERROR;

    auto block = ^BOOL {
        // Track work until end of scope
        MATTER_LOG_METRIC_SCOPE(kMetricPairDevice, errorCode);

        self->_operationalCredentialsDelegate->SetDeviceID(deviceID);

        MATTER_LOG_METRIC_BEGIN(kMetricSetupPASESession);
        errorCode = self->_cppCommissioner->EstablishPASEConnection(deviceID, [onboardingPayload UTF8String]);
        if (CHIP_NO_ERROR == errorCode) {
            self->_deviceControllerDelegateBridge->SetDeviceNodeID(deviceID);
        } else {
            MATTER_LOG_METRIC_END(kMetricSetupPASESession, errorCode);
        }

        return ![MTRDeviceController checkForError:errorCode logMsg:kErrorPairDevice error:error];
    };

    auto success = [self syncRunOnWorkQueueWithBoolReturnValue:block error:error];
    if (!success) {
        MATTER_LOG_METRIC_END(kMetricDeviceCommissioning, errorCode);
    }
    return success;
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
    if (duration > UINT16_MAX) {
        MTR_LOG_ERROR("%@ Error: Duration %lu is too large. Max value %d", self, static_cast<unsigned long>(duration), UINT16_MAX);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE];
        }
        return NO;
    }

    auto block = ^BOOL {
        CHIP_ERROR errorCode = CHIP_NO_ERROR;
        MATTER_LOG_METRIC_SCOPE(kMetricOpenPairingWindow, errorCode);

        errorCode = chip::Controller::AutoCommissioningWindowOpener::OpenBasicCommissioningWindow(
            self->_cppCommissioner, deviceID, chip::System::Clock::Seconds16(static_cast<uint16_t>(duration)));
        return ![MTRDeviceController checkForError:errorCode logMsg:kErrorOpenPairingWindow error:error];
    };

    return [self syncRunOnWorkQueueWithBoolReturnValue:block error:error];
}

- (NSString *)openPairingWindowWithPIN:(uint64_t)deviceID
                              duration:(NSUInteger)duration
                         discriminator:(NSUInteger)discriminator
                              setupPIN:(NSUInteger)setupPIN
                                 error:(NSError * __autoreleasing *)error
{
    if (duration > UINT16_MAX) {
        MTR_LOG_ERROR("%@ Error: Duration %lu is too large. Max value %d", self, static_cast<unsigned long>(duration), UINT16_MAX);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE];
        }
        return nil;
    }

    if (discriminator > 0xfff) {
        MTR_LOG_ERROR("%@ Error: Discriminator %lu is too large. Max value %d", self, static_cast<unsigned long>(discriminator), 0xfff);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE];
        }
        return nil;
    }

    __block CHIP_ERROR errorCode = CHIP_NO_ERROR;
    MATTER_LOG_METRIC_SCOPE(kMetricOpenPairingWindow, errorCode);

    if (!chip::CanCastTo<uint32_t>(setupPIN) || !chip::SetupPayload::IsValidSetupPIN(static_cast<uint32_t>(setupPIN))) {
        MTR_LOG_ERROR("%@ Error: Setup pin %lu is not valid", self, static_cast<unsigned long>(setupPIN));
        errorCode = CHIP_ERROR_INVALID_INTEGER_VALUE;
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:errorCode];
        }
        return nil;
    }

    auto block = ^NSString *
    {
        chip::SetupPayload setupPayload;
        errorCode = chip::Controller::AutoCommissioningWindowOpener::OpenCommissioningWindow(self->_cppCommissioner, deviceID,
            chip::System::Clock::Seconds16(static_cast<uint16_t>(duration)), chip::Crypto::kSpake2p_Min_PBKDF_Iterations,
            static_cast<uint16_t>(discriminator), chip::MakeOptional(static_cast<uint32_t>(setupPIN)), chip::NullOptional,
            setupPayload);

        VerifyOrReturnValue(![MTRDeviceController checkForError:errorCode logMsg:kErrorOpenPairingWindow error:error], nil);

        chip::ManualSetupPayloadGenerator generator(setupPayload);
        std::string outCode;

        if (CHIP_NO_ERROR != (errorCode = generator.payloadDecimalStringRepresentation(outCode))) {
            MTR_LOG_ERROR("%@ Failed to get decimal setup code", self);
            return nil;
        }

        MTR_LOG_ERROR("%@ Setup code is %s", self, outCode.c_str());
        return [NSString stringWithCString:outCode.c_str() encoding:[NSString defaultCStringEncoding]];
    };

    return [self syncRunOnWorkQueueWithReturnValue:block error:error];
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
