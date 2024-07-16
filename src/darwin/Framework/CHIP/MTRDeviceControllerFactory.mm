/**
 *    Copyright (c) 2022 Project CHIP Authors
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

#import "MTRDeviceControllerFactory.h"
#import "MTRDeviceControllerFactory_Internal.h"

#import <Matter/MTRDefines.h>

#import <Matter/MTRClusterConstants.h>
#import <Matter/MTRDeviceControllerParameters.h>
#import <Matter/MTRServerCluster.h>

#import "MTRCertificates.h"
#import "MTRDemuxingStorage.h"
#import "MTRDeviceController.h"
#import "MTRDeviceControllerStartupParams.h"
#import "MTRDeviceControllerStartupParams_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTRDiagnosticLogsDownloader.h"
#import "MTRError_Internal.h"
#import "MTRFabricInfo_Internal.h"
#import "MTRFramework.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRMetricsCollector.h"
#import "MTROTAProviderDelegateBridge.h"
#import "MTROperationalBrowser.h"
#import "MTRP256KeypairBridge.h"
#import "MTRPersistentStorageDelegateBridge.h"
#import "MTRServerAccessControl.h"
#import "MTRServerCluster_Internal.h"
#import "MTRServerEndpoint_Internal.h"
#import "MTRSessionResumptionStorageBridge.h"
#import "MTRUnfairLock.h"
#import "NSDataSpanConversion.h"

#import <os/lock.h>

#include <app/server/Dnssd.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <credentials/CHIPCert.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <crypto/RawKeySessionKeystore.h>
#include <lib/support/Pool.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <messaging/ReliableMessageMgr.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <platform/PlatformManager.h>

#include <cstdlib>

using namespace chip;
using namespace chip::Controller;
using namespace chip::Tracing::DarwinFramework;

static bool sExitHandlerRegistered = false;
static void ShutdownOnExit()
{
    MTR_LOG("ShutdownOnExit invoked on exit");
    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
}

@interface MTRDeviceControllerFactoryParams ()

// Flag to keep track of whether our .storage is real consumer-provided storage
// or just the fake thing we made up.
@property (nonatomic, assign) BOOL hasStorage;

@end

MTR_DIRECT_MEMBERS
@interface MTRDeviceControllerFactory ()
- (void)preWarmCommissioningSessionDone;
@end

MTR_DIRECT_MEMBERS
@implementation MTRDeviceControllerFactory {
    dispatch_queue_t _chipWorkQueue;
    DeviceControllerFactory * _controllerFactory;

    Credentials::IgnoreCertificateValidityPeriodPolicy _certificateValidityPolicy;
    Crypto::RawKeySessionKeystore _sessionKeystore;
    // We use TestPersistentStorageDelegate just to get an in-memory store to back
    // our group data provider impl.  We initialize this store correctly on every
    // controller startup, so don't need to actually persist it.
    TestPersistentStorageDelegate _groupStorageDelegate;
    Credentials::GroupDataProviderImpl _groupDataProvider;

    // _usingPerControllerStorage is only written once, during controller
    // factory start.  After that it is only read, and can be read from
    // arbitrary threads.
    BOOL _usingPerControllerStorage;
    PersistentStorageDelegate * _persistentStorageDelegate;
    MTRSessionResumptionStorageBridge * _sessionResumptionStorage;
    PersistentStorageOperationalKeystore * _keystore;
    Credentials::PersistentStorageOpCertStore * _opCertStore;
    MTROperationalBrowser * _operationalBrowser;

    // productAttestationAuthorityCertificates and certificationDeclarationCertificates are just copied
    // from MTRDeviceControllerFactoryParams.
    NSArray<MTRCertificateDERBytes> * _Nullable _productAttestationAuthorityCertificates;
    NSArray<MTRCertificateDERBytes> * _Nullable _certificationDeclarationCertificates;

    BOOL _advertiseOperational;

    // Lock used to serialize access to the "controllers" array and the
    // "_controllerBeingStarted" and "_controllerBeingShutDown" ivars, since those
    // need to be touched from both whatever queue is starting controllers and from
    // the Matter queue.  The way this lock is used assumes that:
    //
    // 1) The only mutating accesses to the controllers array and the ivars happen
    //    when the current queue is not the Matter queue or in a block that was
    //    sync-dispatched to the Matter queue.  This is a good assumption, because
    //    the implementations of the functions that mutate these do sync dispatch to
    //    the Matter queue, which would deadlock if they were called when that queue
    //    was the current queue.
    //
    // 2) It's our API consumer's responsibility to serialize access to us from
    //    outside.
    //
    // These assumptions mean that if we are in a block that was sync-dispatched to
    // the Matter queue, that block cannot race with either the Matter queue nor the
    // non-Matter queue.  Similarly, if we are in a situation where the Matter queue
    // has been shut down, any accesses to the variables cannot race anything else.
    //
    // This means that:
    //
    // A. In a sync-dispatched block, or if the Matter queue has been shut down, we
    //    do not need to lock and can do read or write access.
    // B. Apart from item A, mutations of the array and ivars must happen outside the
    //    Matter queue and must lock.
    // C. Apart from item A, accesses on the Matter queue must be reads only and
    //    must lock.
    // D. Locking around reads not from the Matter queue is OK but not required.
    os_unfair_lock _controllersLock;
    NSMutableArray<MTRDeviceController *> * _controllers;
    MTRDeviceController * _controllerBeingStarted;
    MTRDeviceController * _controllerBeingShutDown;

    // Next available fabric index.  Only valid when _controllerBeingStarted is
    // non-nil, and then it corresponds to the controller being started.  This
    // is only accessed on the Matter queue or after the Matter queue has shut
    // down.
    FabricIndex _nextAvailableFabricIndex;

    id<MTROTAProviderDelegate> _Nullable _otaProviderDelegate;
    dispatch_queue_t _Nullable _otaProviderDelegateQueue;
    MTRServerEndpoint * _otaProviderEndpoint;
    std::unique_ptr<MTROTAProviderDelegateBridge> _otaProviderDelegateBridge;

    MTRDiagnosticLogsDownloader * _Nullable _diagnosticLogsDownloader;

    // Array of all server endpoints across all controllers, used to ensure
    // in an atomic way that endpoint IDs are unique.
    NSMutableArray<MTRServerEndpoint *> * _serverEndpoints;
    os_unfair_lock _serverEndpointsLock; // Protects access to _serverEndpoints.

    class final : public DeviceLayer::BleScannerDelegate {
        void OnBleScanStopped() override
        {
            [MTRDeviceControllerFactory.sharedInstance preWarmCommissioningSessionDone];
        }
    } _preWarmingDelegate;
}

+ (void)initialize
{
    MTRFrameworkInit();
}

+ (MTRDeviceControllerFactory *)sharedInstance
{
    static MTRDeviceControllerFactory * factory = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // initialize the factory.
        factory = [[MTRDeviceControllerFactory alloc] init];
    });
    return factory;
}

- (instancetype)init
{
    if (!(self = [super init])) {
        return nil;
    }

    // Start the work queue and leave it running. There is no performance
    // cost to having an idle dispatch queue, and it simplifies our logic.
    DeviceLayer::PlatformMgrImpl().StartEventLoopTask();

    _chipWorkQueue = DeviceLayer::PlatformMgrImpl().GetWorkQueue();
    _controllerFactory = &DeviceControllerFactory::GetInstance();

    // Initialize our default-constructed GroupDataProviderImpl.
    // For now default args are fine, since we are just using this for the IPK.
    _groupDataProvider.SetStorageDelegate(&_groupStorageDelegate);
    _groupDataProvider.SetSessionKeystore(&_sessionKeystore);
    CHIP_ERROR err = _groupDataProvider.Init();
    VerifyOrDieWithMsg(err == CHIP_NO_ERROR, NotSpecified,
        "GroupDataProviderImpl::Init() failed: %" CHIP_ERROR_FORMAT, err.Format());

    _controllersLock = OS_UNFAIR_LOCK_INIT;
    _controllers = [[NSMutableArray alloc] init];

    _serverEndpointsLock = OS_UNFAIR_LOCK_INIT;
    _serverEndpoints = [[NSMutableArray alloc] init];

    return self;
}

- (void)dealloc
{
    [self stopControllerFactory];
    _groupDataProvider.Finish();
}

- (void)_assertCurrentQueueIsNotMatterQueue
{
    VerifyOrDie(!DeviceLayer::PlatformMgrImpl().IsWorkQueueCurrentQueue());
}

- (void)cleanupStartupObjects
{
    assertChipStackLockedByCurrentThread();
    MTR_LOG("Cleaning startup objects in controller factory");

    // Make sure the deinit order here is the reverse of the init order in
    // startControllerFactory:
    _certificationDeclarationCertificates = nil;
    _productAttestationAuthorityCertificates = nil;

    if (_opCertStore) {
        _opCertStore->Finish();
        delete _opCertStore;
        _opCertStore = nullptr;
    }

    if (_keystore) {
        _keystore->Finish();
        delete _keystore;
        _keystore = nullptr;
    }

    _otaProviderDelegateQueue = nil;
    _otaProviderDelegate = nil;

    if (_sessionResumptionStorage) {
        delete _sessionResumptionStorage;
        _sessionResumptionStorage = nullptr;
    }

    if (_persistentStorageDelegate) {
        delete _persistentStorageDelegate;
        _persistentStorageDelegate = nullptr;
    }

    _diagnosticLogsDownloader = nil;

    ShutdownMetricsCollection();
}

- (CHIP_ERROR)_initFabricTable:(FabricTable &)fabricTable
{
    return fabricTable.Init(
        { .storage = _persistentStorageDelegate, .operationalKeystore = _keystore, .opCertStore = _opCertStore });
}

- (nullable NSArray<MTRFabricInfo *> *)knownFabrics
{
    [self _assertCurrentQueueIsNotMatterQueue];

    if (!_running) { // Note: reading _running from outside of the Matter work queue
        return nil;
    }

    __block NSMutableArray<MTRFabricInfo *> * fabricList;
    __block BOOL listFilled = NO;
    dispatch_sync(_chipWorkQueue, ^{
        FabricTable fabricTable;
        CHIP_ERROR err = [self _initFabricTable:fabricTable];
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Can't initialize fabric table when getting known fabrics: %s", err.AsString());
            return;
        }

        fabricList = [NSMutableArray<MTRFabricInfo *> arrayWithCapacity:fabricTable.FabricCount()];
        for (const auto & fabricInfo : fabricTable) {
            auto * info = [[MTRFabricInfo alloc] initWithFabricTable:fabricTable fabricInfo:fabricInfo];
            if (info == nil) {
                // Failed to read one of our fabrics.
                return;
            }

            [fabricList addObject:info];
        }

        listFilled = YES;
    });

    if (listFilled == NO) {
        return nil;
    }

    return fabricList;
}

- (BOOL)startControllerFactory:(MTRDeviceControllerFactoryParams *)startupParams error:(NSError * __autoreleasing *)error
{
    return [self _startControllerFactory:startupParams startingController:NO error:error];
}

- (BOOL)_startControllerFactory:(MTRDeviceControllerFactoryParams *)startupParams
             startingController:(BOOL)startingController
                          error:(NSError * __autoreleasing *)error
{
    [self _assertCurrentQueueIsNotMatterQueue];

    __block CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    dispatch_sync(_chipWorkQueue, ^{
        if (_running) {
            // TODO: When treating a duplicate call as success we should validate parameters match
            MTR_LOG_DEBUG("Ignoring duplicate call to startup, Matter controller factory already started...");
            ExitNow(err = CHIP_NO_ERROR);
        }

        StartupMetricsCollection();
        InitializeServerAccessControl();

        if (startupParams.hasStorage) {
            _persistentStorageDelegate = new MTRPersistentStorageDelegateBridge(startupParams.storage);
            _sessionResumptionStorage = nullptr;
            _usingPerControllerStorage = NO;
        } else {
            _persistentStorageDelegate = new MTRDemuxingStorage(self);
            _sessionResumptionStorage = new MTRSessionResumptionStorageBridge(self);
            _usingPerControllerStorage = YES;
        }

        _otaProviderDelegate = startupParams.otaProviderDelegate;
        if (_otaProviderDelegate != nil) {
            _otaProviderDelegateQueue = dispatch_queue_create(
                "org.csa-iot.matter.framework.otaprovider.workqueue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        }

        // TODO: Allow passing a different keystore implementation via startupParams.
        _keystore = new PersistentStorageOperationalKeystore();
        SuccessOrExit(err = _keystore->Init(_persistentStorageDelegate));

        // TODO Allow passing a different opcert store implementation via startupParams.
        _opCertStore = new Credentials::PersistentStorageOpCertStore();
        SuccessOrExit(err = _opCertStore->Init(_persistentStorageDelegate));

        _productAttestationAuthorityCertificates = [startupParams.productAttestationAuthorityCertificates copy];
        _certificationDeclarationCertificates = [startupParams.certificationDeclarationCertificates copy];

        {
            chip::Controller::FactoryInitParams params;
            if (startupParams.port != nil) {
                params.listenPort = [startupParams.port unsignedShortValue];
            }
            params.enableServerInteractions = startupParams.shouldStartServer;

            params.groupDataProvider = &_groupDataProvider;
            params.sessionKeystore = &_sessionKeystore;
            params.fabricIndependentStorage = _persistentStorageDelegate;
            params.operationalKeystore = _keystore;
            params.opCertStore = _opCertStore;
            params.certificateValidityPolicy = &_certificateValidityPolicy;
            params.sessionResumptionStorage = _sessionResumptionStorage;
            SuccessOrExit(err = _controllerFactory->Init(params));
        }

        // This needs to happen after DeviceControllerFactory::Init,
        // because that creates (lazily, by calling functions with
        // static variables in them) some static-lifetime objects.
        if (!sExitHandlerRegistered) {
            if (atexit(ShutdownOnExit) != 0) {
                char error[128];
                strerror_r(errno, error, sizeof(error));
                MTR_LOG_ERROR("Warning: Failed to register atexit handler: %s", error);
            }
            sExitHandlerRegistered = true;
        }
        HeapObjectPoolExitHandling::IgnoreLeaksOnExit();

        // Make sure we don't leave a system state running while we have no
        // controllers started.  This is working around the fact that a system
        // state is brought up live on factory init, and not when it comes time
        // to actually start a controller, and does not actually clean itself up
        // until its refcount (which starts as 0) goes to 0.
        // TODO: Don't cause a stack shutdown and restart if startingController
        _controllerFactory->RetainSystemState();
        _controllerFactory->ReleaseSystemState();

        _advertiseOperational = startupParams.shouldStartServer;
        _running = YES;
        err = CHIP_NO_ERROR;

    exit:
        if (err != CHIP_NO_ERROR) {
            // Note: Since we have failed no later than _controllerFactory->Init(),
            // there is no need to call _controllerFactory->Shutdown() here.
            [self cleanupStartupObjects];
        }
    });
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Failed to start Matter controller factory: %" CHIP_ERROR_FORMAT, err.Format());
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:err];
        }
        return NO;
    }
    return YES;
}

- (void)stopControllerFactory
{
    [self _assertCurrentQueueIsNotMatterQueue];

    while ([_controllers count] != 0) {
        [_controllers[0] shutdown];
    }

    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn(_running);

        MTR_LOG("Shutting down the Matter controller factory");
        _controllerFactory->Shutdown();
        [self cleanupStartupObjects];
        _running = NO;
        _advertiseOperational = NO;
    });
}

/**
 * Helper function to start a device controller with the given startup params.
 * The fabricChecker block will run on the Matter queue, and is expected to
 * return nil if pre-startup fabric table checks fail, and set fabricError to
 * the right error value in that situation.
 *
 * The provided controller is expected to have just been allocated and to not be
 * initialized yet.
 */
- (MTRDeviceController * _Nullable)_startDeviceController:(MTRDeviceController *)controller
                                            startupParams:(id)startupParams
                                            fabricChecker:(MTRDeviceControllerStartupParamsInternal * (^)(FabricTable * fabricTable,
                                                              MTRDeviceController * controller,
                                                              CHIP_ERROR & fabricError))fabricChecker
                                                    error:(NSError * __autoreleasing *)error
{
    [self _assertCurrentQueueIsNotMatterQueue];

    id<MTRDeviceControllerStorageDelegate> _Nullable storageDelegate;
    dispatch_queue_t _Nullable storageDelegateQueue;
    NSUUID * uniqueIdentifier;
    id<MTROTAProviderDelegate> _Nullable otaProviderDelegate;
    dispatch_queue_t _Nullable otaProviderDelegateQueue;
    NSUInteger concurrentSubscriptionPoolSize = 0;
    MTRDeviceStorageBehaviorConfiguration * storageBehaviorConfiguration = nil;
    if ([startupParams isKindOfClass:[MTRDeviceControllerParameters class]]) {
        MTRDeviceControllerParameters * params = startupParams;
        storageDelegate = params.storageDelegate;
        storageDelegateQueue = params.storageDelegateQueue;
        uniqueIdentifier = params.uniqueIdentifier;
        otaProviderDelegate = params.otaProviderDelegate;
        otaProviderDelegateQueue = params.otaProviderDelegateQueue;
        concurrentSubscriptionPoolSize = params.concurrentSubscriptionEstablishmentsAllowedOnThread;
        storageBehaviorConfiguration = params.storageBehaviorConfiguration;
    } else if ([startupParams isKindOfClass:[MTRDeviceControllerStartupParams class]]) {
        MTRDeviceControllerStartupParams * params = startupParams;
        storageDelegate = nil;
        storageDelegateQueue = nil;
        uniqueIdentifier = params.uniqueIdentifier;
        otaProviderDelegate = nil;
        otaProviderDelegateQueue = nil;
    } else {
        MTR_LOG_ERROR("Unknown kind of startup params: %@", startupParams);
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    if (!_running) { // Note: reading _running from outside of the Matter work queue
        if (storageDelegate != nil) {
            MTR_LOG("Auto-starting Matter controller factory in per-controller storage mode");
            auto * params = [[MTRDeviceControllerFactoryParams alloc] initWithoutStorage];
            if (![self _startControllerFactory:params startingController:YES error:error]) {
                return nil;
            }
        } else {
            MTR_LOG_ERROR("Trying to start controller while Matter controller factory is not running");
            if (error != nil) {
                *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
            }
            return nil;
        }
    }

    if (_usingPerControllerStorage && storageDelegate == nil) {
        MTR_LOG_ERROR("Must have a controller storage delegate when we do not have storage for the controller factory");
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    if (!_usingPerControllerStorage && storageDelegate != nil) {
        MTR_LOG_ERROR("Must not have a controller storage delegate when we have storage for the controller factory");
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    // Fall back to the factory-wide OTA provider delegate if one is not
    // provided in the startup params.
    if (otaProviderDelegate == nil) {
        otaProviderDelegate = _otaProviderDelegate;
        otaProviderDelegateQueue = _otaProviderDelegateQueue;
    }

    controller = [controller initWithFactory:self
                                       queue:_chipWorkQueue
                             storageDelegate:storageDelegate
                        storageDelegateQueue:storageDelegateQueue
                         otaProviderDelegate:otaProviderDelegate
                    otaProviderDelegateQueue:otaProviderDelegateQueue
                            uniqueIdentifier:uniqueIdentifier
              concurrentSubscriptionPoolSize:concurrentSubscriptionPoolSize
                storageBehaviorConfiguration:storageBehaviorConfiguration];
    if (controller == nil) {
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    if ([_controllers count] == 0) {
        dispatch_sync(_chipWorkQueue, ^{
            self->_operationalBrowser = new MTROperationalBrowser(self, self->_chipWorkQueue);
        });
    }

    // Add the controller to _controllers now, so if we fail partway through its
    // startup we will still do the right cleanups.
    os_unfair_lock_lock(&_controllersLock);
    [_controllers addObject:controller];
    os_unfair_lock_unlock(&_controllersLock);

    __block MTRDeviceControllerStartupParamsInternal * params = nil;
    __block CHIP_ERROR fabricError = CHIP_ERROR_INTERNAL;

    // Create a temporary FabricTable instance for the fabricChecker logic.
    // We want the block to end up with just a pointer to the fabric table,
    // since we know our on-stack instance will outlive the block.
    FabricTable fabricTableInstance;
    FabricTable * fabricTable = &fabricTableInstance;

    dispatch_sync(_chipWorkQueue, ^{
        fabricError = [self _initFabricTable:*fabricTable];
        if (fabricError != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Can't initialize fabric table: %s", fabricError.AsString());
            return;
        }

        params = fabricChecker(fabricTable, controller, fabricError);

        if (params == nil) {
            return;
        }

        // Check that we are not trying to start a controller with a uniqueIdentifier that
        // matches a running controller.
        auto * controllersCopy = [self getRunningControllers];
        for (MTRDeviceController * existing in controllersCopy) {
            if (existing != controller && [existing.uniqueIdentifier isEqual:params.uniqueIdentifier]) {
                MTR_LOG_ERROR("Already have running controller with uniqueIdentifier %@", existing.uniqueIdentifier);
                fabricError = CHIP_ERROR_INVALID_ARGUMENT;
                params = nil;
                return;
            }
        }

        // Save off the next available fabric index, in case we are starting a
        // controller with a new fabric index.  This just needs to happen before
        // we set _controllerBeingStarted below.
        fabricError = fabricTable->PeekFabricIndexForNextAddition(self->_nextAvailableFabricIndex);
        if (fabricError != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Out of space in the fabric table");
            params = nil;
            return;
        }
    });

    if (params == nil) {
        [self controllerShuttingDown:controller];
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:fabricError];
        }
        return nil;
    }

    os_unfair_lock_lock(&_controllersLock);
    _controllerBeingStarted = controller;
    os_unfair_lock_unlock(&_controllersLock);

    BOOL ok = [controller startup:params];

    os_unfair_lock_lock(&_controllersLock);
    _controllerBeingStarted = nil;
    os_unfair_lock_unlock(&_controllersLock);

    if (ok == NO) {
        // TODO: get error from controller's startup.
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTERNAL];
        }
        return nil;
    }

    controller = [self maybeInitializeOTAProvider:controller];
    if (controller == nil) {
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTERNAL];
        }
    }
    return controller;
}

- (MTRDeviceController * _Nullable)createControllerOnExistingFabric:(MTRDeviceControllerStartupParams *)startupParams
                                                              error:(NSError * __autoreleasing *)error
{
    [self _assertCurrentQueueIsNotMatterQueue];

    if (_usingPerControllerStorage) {
        // We can never have an "existing fabric" for a new controller to be
        // created on, in the sense of createControllerOnExistingFabric.
        MTR_LOG_ERROR("Can't createControllerOnExistingFabric when using per-controller data store");
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
        }
        return nil;
    }

    return [self _startDeviceController:[MTRDeviceController alloc]
                          startupParams:startupParams
                          fabricChecker:^MTRDeviceControllerStartupParamsInternal *(
                              FabricTable * fabricTable, MTRDeviceController * controller, CHIP_ERROR & fabricError) {
                              const FabricInfo * fabric = nullptr;
                              BOOL ok = [self findMatchingFabric:*fabricTable params:startupParams fabric:&fabric];
                              if (!ok) {
                                  MTR_LOG_ERROR("Can't start on existing fabric: fabric matching failed");
                                  fabricError = CHIP_ERROR_INTERNAL;
                                  return nil;
                              }

                              if (fabric == nullptr) {
                                  MTR_LOG_ERROR("Can't start on existing fabric: fabric not found");
                                  fabricError = CHIP_ERROR_NOT_FOUND;
                                  return nil;
                              }

                              auto * controllersCopy = [self getRunningControllers];

                              for (MTRDeviceController * existing in controllersCopy) {
                                  BOOL isRunning = YES; // assume the worst
                                  if ([existing isRunningOnFabric:fabricTable
                                                      fabricIndex:fabric->GetFabricIndex()
                                                        isRunning:&isRunning]
                                      != CHIP_NO_ERROR) {
                                      MTR_LOG_ERROR("Can't tell what fabric a controller is running on.  Not safe to start.");
                                      fabricError = CHIP_ERROR_INTERNAL;
                                      return nil;
                                  }

                                  if (isRunning) {
                                      MTR_LOG_ERROR("Can't start on existing fabric: another controller is running on it");
                                      fabricError = CHIP_ERROR_INCORRECT_STATE;
                                      return nil;
                                  }
                              }

                              auto * params =
                                  [[MTRDeviceControllerStartupParamsInternal alloc] initForExistingFabric:fabricTable
                                                                                              fabricIndex:fabric->GetFabricIndex()
                                                                                                 keystore:self->_keystore
                                                                                     advertiseOperational:self->_advertiseOperational
                                                                                                   params:startupParams];
                              params.productAttestationAuthorityCertificates = self->_productAttestationAuthorityCertificates;
                              params.certificationDeclarationCertificates = self->_certificationDeclarationCertificates;
                              return params;
                          }
                                  error:error];
}

- (MTRDeviceController * _Nullable)createControllerOnNewFabric:(MTRDeviceControllerStartupParams *)startupParams
                                                         error:(NSError * __autoreleasing *)error
{
    [self _assertCurrentQueueIsNotMatterQueue];

    if (startupParams.vendorID == nil) {
        MTR_LOG_ERROR("Must provide vendor id when starting controller on new fabric");
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    if (startupParams.intermediateCertificate != nil && startupParams.rootCertificate == nil) {
        MTR_LOG_ERROR("Must provide a root certificate when using an intermediate certificate");
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    return [self _startDeviceController:[MTRDeviceController alloc]
                          startupParams:startupParams
                          fabricChecker:^MTRDeviceControllerStartupParamsInternal *(
                              FabricTable * fabricTable, MTRDeviceController * controller, CHIP_ERROR & fabricError) {
                              const FabricInfo * fabric = nullptr;
                              BOOL ok = [self findMatchingFabric:*fabricTable params:startupParams fabric:&fabric];
                              if (!ok) {
                                  MTR_LOG_ERROR("Can't start on new fabric: fabric matching failed");
                                  fabricError = CHIP_ERROR_INTERNAL;
                                  return nil;
                              }

                              if (fabric != nullptr) {
                                  MTR_LOG_ERROR("Can't start on new fabric that matches existing fabric");
                                  fabricError = CHIP_ERROR_INCORRECT_STATE;
                                  return nil;
                              }

                              auto * params =
                                  [[MTRDeviceControllerStartupParamsInternal alloc] initForNewFabric:fabricTable
                                                                                            keystore:self->_keystore
                                                                                advertiseOperational:self->_advertiseOperational
                                                                                              params:startupParams];
                              params.productAttestationAuthorityCertificates = self->_productAttestationAuthorityCertificates;
                              params.certificationDeclarationCertificates = self->_certificationDeclarationCertificates;
                              return params;
                          }
                                  error:error];
}

- (void)preWarmCommissioningSession
{
    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CHIP_ERROR_INCORRECT_STATE;
        if (!self->_running) {
            MTR_LOG_ERROR("Can't pre-warm, Matter controller factory is not running");
        } else {
            MTR_LOG("Pre-warming commissioning session");
            self->_controllerFactory->EnsureAndRetainSystemState();
            err = DeviceLayer::PlatformMgrImpl().StartBleScan(&self->_preWarmingDelegate, DeviceLayer::BleScanMode::kPreWarm);
            if (err != CHIP_NO_ERROR) {
                MTR_LOG_ERROR("Pre-warming failed: %" CHIP_ERROR_FORMAT, err.Format());
                self->_controllerFactory->ReleaseSystemState();
            }
        }
        MATTER_LOG_METRIC(kMetricPreWarmCommissioning, err);
    });
}

- (void)preWarmCommissioningSessionDone
{
    assertChipStackLockedByCurrentThread();
    MTR_LOG("Pre-warming done");
    self->_controllerFactory->ReleaseSystemState();
}

// Finds a fabric that matches the given params, if one exists.
//
// Returns NO on failure, YES on success.  If YES is returned, the
// outparam will be written to, but possibly with a null value.
//
// fabricTable should be an initialized fabric table.  It needs to
// outlive the consumer's use of the FabricInfo we return, which is
// why it's provided by the caller.
- (BOOL)findMatchingFabric:(FabricTable &)fabricTable
                    params:(MTRDeviceControllerStartupParams *)params
                    fabric:(const FabricInfo * _Nullable * _Nonnull)fabric
{
    assertChipStackLockedByCurrentThread();

    Crypto::P256PublicKey pubKey;
    if (params.rootCertificate != nil) {
        CHIP_ERROR err = ExtractPubkeyFromX509Cert(AsByteSpan(params.rootCertificate), pubKey);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Can't extract public key from root certificate: %s", ErrorStr(err));
            return NO;
        }
    } else {
        // No root certificate means the nocSigner is using the root keys, because
        // consumers must provide a root certificate whenever an ICA is used.
        CHIP_ERROR err = MTRP256KeypairBridge::MatterPubKeyFromSecKeyRef(params.nocSigner.publicKey, &pubKey);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Can't extract public key from MTRKeypair: %s", ErrorStr(err));
            return NO;
        }
    }

    *fabric = fabricTable.FindFabric(pubKey, params.fabricID.unsignedLongLongValue);
    return YES;
}

// Initialize the MTROTAProviderDelegateBridge if it has not been initialized already
//
// Returns nil on failure, the input controller on success.
// If the provider has been initialized already, it is not considered as a failure.
//
- (MTRDeviceController * _Nullable)maybeInitializeOTAProvider:(MTRDeviceController * _Nonnull)controller
{
    [self _assertCurrentQueueIsNotMatterQueue];

    VerifyOrReturnValue([_controllers count] == 1, controller);

    _otaProviderEndpoint = [MTRServerEndpoint rootNodeEndpoint];

    // TODO: Have the OTA Provider cluster revision accessible somewhere?
    auto * otaProviderCluster = [[MTRServerCluster alloc] initWithClusterID:@(MTRClusterIDTypeOTASoftwareUpdateProviderID) revision:@(1)];
    otaProviderCluster.acceptedCommands = @[
        @(MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageID),
        @(MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateRequestID),
        @(MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandNotifyUpdateAppliedID),
    ];
    otaProviderCluster.generatedCommands = @[
        @(MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageResponseID),
        @(MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateResponseID),
    ];
    [otaProviderCluster addAccessGrant:[MTRAccessGrant accessGrantForAllNodesWithPrivilege:MTRAccessControlEntryPrivilegeOperate]];

    // Not expected to fail, since we are following the rules for clusters here.
    [_otaProviderEndpoint addServerCluster:otaProviderCluster];

    if (![self addServerEndpoint:_otaProviderEndpoint]) {
        MTR_LOG_ERROR("Failed to add OTA endpoint on factory.  Why?");
        [controller shutdown];
        return nil;
    }

    // This endpoint is not actually associated with a specific controller; we
    // just need to have a working Matter event loop to bring it up.
    [_otaProviderEndpoint associateWithController:nil];

    __block CHIP_ERROR err;
    dispatch_sync(_chipWorkQueue, ^{
        [self->_otaProviderEndpoint registerMatterEndpoint];

        // Now that our endpoint exists, go ahead and create the OTA delegate
        // bridge.  Its constructor relies on the endpoint existing.
        _otaProviderDelegateBridge = std::make_unique<MTROTAProviderDelegateBridge>();

        auto systemState = _controllerFactory->GetSystemState();
        err = _otaProviderDelegateBridge->Init(systemState->SystemLayer(), systemState->ExchangeMgr());
    });
    if (CHIP_NO_ERROR != err) {
        MTR_LOG_ERROR("Failed to init provider delegate bridge: %" CHIP_ERROR_FORMAT, err.Format());
        [controller shutdown];
        return nil;
    }

    return controller;
}

- (void)resetOperationalAdvertising
{
    assertChipStackLockedByCurrentThread();

    // If we're not advertising, then there's no need to reset anything.
    VerifyOrReturn(_advertiseOperational);

    // If there are no running controllers there will be no advertisements to reset.
    {
        std::lock_guard lock(_controllersLock);
        VerifyOrReturn(_controllers.count > 0);
    }

    // StartServer() is the only API we have for resetting DNS-SD advertising.
    // It sure would be nice if there were a "restart" that was a no-op if the
    // DNS-SD server was not already running.
    app::DnssdServer::Instance().StartServer();
}

- (void)controllerShuttingDown:(MTRDeviceController *)controller
{
    [self _assertCurrentQueueIsNotMatterQueue];

    if (![_controllers containsObject:controller]) {
        MTR_LOG_ERROR("Controller we don't know about shutting down");
        return;
    }

    os_unfair_lock_lock(&_controllersLock);
    // Make sure to set _controllerBeingShutDown and do the remove in the same
    // locked section, so there is never a time when the controller is gone from
    // both places as viewed from the Matter thread, as long as it's locking
    // around its reads.
    _controllerBeingShutDown = controller;
    [_controllers removeObject:controller];
    os_unfair_lock_unlock(&_controllersLock);

    // Do the controller shutdown on the Matter work queue.
    dispatch_sync(_chipWorkQueue, ^{
        FabricIndex fabricIndex = controller.fabricIndex;
        if (fabricIndex != kUndefinedFabricIndex) {
            // Clear out out group keys for this fabric index, in case fabric
            // indices get reused later.  If a new controller is started on the
            // same fabric it will be handed the IPK at that point.
            self->_groupDataProvider.RemoveGroupKeys(fabricIndex);
        }

        // If there are no other controllers left, we can shut down some things.
        // Do this before we shut down the controller itself, because the
        // OtaProviderDelegateBridge uses some services provided by the system
        // state without retaining it.
        if (_controllers.count == 0) {
            delete self->_operationalBrowser;
            self->_operationalBrowser = nullptr;

            if (_otaProviderDelegateBridge) {
                _otaProviderDelegateBridge->Shutdown();
                _otaProviderDelegateBridge.reset();
            }

            if (_otaProviderEndpoint != nil) {
                [_otaProviderEndpoint unregisterMatterEndpoint];
                [_otaProviderEndpoint invalidate];
                [self removeServerEndpoint:_otaProviderEndpoint];
                _otaProviderEndpoint = nil;
            }
        } else if (_otaProviderDelegateBridge) {
            _otaProviderDelegateBridge->ControllerShuttingDown(controller);
        }

        [controller shutDownCppController];

        self->_controllerBeingShutDown = nil;
        if (self->_controllerBeingStarted == controller) {
            self->_controllerBeingStarted = nil;
        }
    });

    [controller deinitFromFactory];
}

- (NSArray<MTRDeviceController *> *)getRunningControllers
{
    std::lock_guard lock(_controllersLock);
    return [_controllers copy];
}

- (nullable MTRDeviceController *)runningControllerForFabricIndex:(FabricIndex)fabricIndex
                                      includeControllerStartingUp:(BOOL)includeControllerStartingUp
                                    includeControllerShuttingDown:(BOOL)includeControllerShuttingDown
{
    assertChipStackLockedByCurrentThread();

    auto * controllersCopy = [self getRunningControllers];

    os_unfair_lock_lock(&_controllersLock);
    MTRDeviceController * controllerBeingStarted = _controllerBeingStarted;
    MTRDeviceController * controllerBeingShutDown = _controllerBeingShutDown;
    os_unfair_lock_unlock(&_controllersLock);

    for (MTRDeviceController * existing in controllersCopy) {
        if (existing.fabricIndex == fabricIndex) {
            return existing;
        }
    }

    if (includeControllerStartingUp == YES && controllerBeingStarted != nil && fabricIndex == _nextAvailableFabricIndex) {
        return controllerBeingStarted;
    }

    if (includeControllerShuttingDown == YES && controllerBeingShutDown != nil
        && controllerBeingShutDown.fabricIndex == fabricIndex) {
        return controllerBeingShutDown;
    }

    return nil;
}

- (nullable MTRDeviceController *)runningControllerForFabricIndex:(chip::FabricIndex)fabricIndex
{
    return [self runningControllerForFabricIndex:fabricIndex includeControllerStartingUp:YES includeControllerShuttingDown:YES];
}

- (BOOL)addServerEndpoint:(MTRServerEndpoint *)endpoint
{
    os_unfair_lock_lock(&_serverEndpointsLock);
    if (_serverEndpoints.count == CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT) {
        os_unfair_lock_unlock(&_serverEndpointsLock);

        MTR_LOG_ERROR("Can't add a server endpoint with endpoint ID %u, because we already have %u endpoints defined", static_cast<EndpointId>(endpoint.endpointID.unsignedLongLongValue), CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT);

        return NO;
    }

    BOOL haveExisting = NO;
    for (MTRServerEndpoint * existing in _serverEndpoints) {
        if ([endpoint.endpointID isEqual:existing.endpointID]) {
            haveExisting = YES;
            break;
        }
    }

    if (!haveExisting) {
        [_serverEndpoints addObject:endpoint];
    }
    os_unfair_lock_unlock(&_serverEndpointsLock);

    if (haveExisting) {
        MTR_LOG_ERROR("Trying to add a server endpoint with endpoint ID %u, which already exists", static_cast<EndpointId>(endpoint.endpointID.unsignedLongLongValue));
    }

    return !haveExisting;
}

- (void)removeServerEndpoint:(MTRServerEndpoint *)endpoint
{
    std::lock_guard lock(_serverEndpointsLock);
    [_serverEndpoints removeObject:endpoint];
}

- (NSArray<MTRAccessGrant *> *)accessGrantsForFabricIndex:(chip::FabricIndex)fabricIndex clusterPath:(MTRClusterPath *)clusterPath
{
    assertChipStackLockedByCurrentThread();

    if ([clusterPath.endpoint isEqual:_otaProviderEndpoint.endpointID]) {
        return [_otaProviderEndpoint matterAccessGrantsForCluster:clusterPath.cluster];
    }

    // We do not want to use _serverEndpoints here, because that might contain
    // endpoints that are still being set up and whatnot.  Ask the controller
    // for the relevant fabric index what the relevant access grants are.

    // Include controllers that are shutting down, since this may be an accesss
    // check for event reports they emit as they shut down.
    auto * controller = [self runningControllerForFabricIndex:fabricIndex includeControllerStartingUp:NO includeControllerShuttingDown:YES];
    if (controller == nil) {
        return @[];
    }

    return [controller accessGrantsForClusterPath:clusterPath];
}

- (nullable NSNumber *)neededReadPrivilegeForClusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID
{
    assertChipStackLockedByCurrentThread();

    for (MTRServerCluster * cluster in _otaProviderEndpoint.serverClusters) {
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

    for (MTRDeviceController * controller in [self getRunningControllers]) {
        NSNumber * _Nullable neededPrivilege = [controller neededReadPrivilegeForClusterID:clusterID attributeID:attributeID];
        if (neededPrivilege != nil) {
            return neededPrivilege;
        }
    }

    return nil;
}

- (void)downloadLogFromNodeWithID:(NSNumber *)nodeID
                       controller:(MTRDeviceController *)controller
                             type:(MTRDiagnosticLogType)type
                          timeout:(NSTimeInterval)timeout
                            queue:(dispatch_queue_t)queue
                       completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
{
    assertChipStackLockedByCurrentThread();

    if (_diagnosticLogsDownloader == nil) {
        _diagnosticLogsDownloader = [[MTRDiagnosticLogsDownloader alloc] init];
        auto systemState = _controllerFactory->GetSystemState();
        systemState->BDXTransferServer()->SetDelegate([_diagnosticLogsDownloader getBridge]);
    }

    [_diagnosticLogsDownloader downloadLogFromNodeWithID:nodeID
                                              controller:controller
                                                    type:type
                                                 timeout:timeout
                                                   queue:queue
                                              completion:completion];
}

- (void)operationalInstanceAdded:(chip::PeerId &)operationalID
{
    assertChipStackLockedByCurrentThread();

    auto * controllersCopy = [self getRunningControllers];

    for (MTRDeviceController * controller in controllersCopy) {
        auto * compressedFabricId = controller.compressedFabricID;
        if (compressedFabricId != nil && compressedFabricId.unsignedLongLongValue == operationalID.GetCompressedFabricId()) {
            ChipLogProgress(Controller, "Notifying controller at fabric index %u about new operational node 0x" ChipLogFormatX64,
                controller.fabricIndex, ChipLogValueX64(operationalID.GetNodeId()));
            [controller operationalInstanceAdded:operationalID.GetNodeId()];
        }

        // Keep going: more than one controller might match a given compressed
        // fabric id, though the chances are low.
    }
}

- (MTRDeviceController * _Nullable)initializeController:(MTRDeviceController *)controller
                                         withParameters:(MTRDeviceControllerParameters *)parameters
                                                  error:(NSError * __autoreleasing *)error
{
    [self _assertCurrentQueueIsNotMatterQueue];

    return [self _startDeviceController:controller
                          startupParams:parameters
                          fabricChecker:^MTRDeviceControllerStartupParamsInternal *(
                              FabricTable * fabricTable, MTRDeviceController * controller, CHIP_ERROR & fabricError) {
                              auto advertiseOperational = self->_advertiseOperational && parameters.shouldAdvertiseOperational;
                              auto * params =
                                  [[MTRDeviceControllerStartupParamsInternal alloc] initForNewController:controller
                                                                                             fabricTable:fabricTable
                                                                                                keystore:self->_keystore
                                                                                    advertiseOperational:advertiseOperational
                                                                                                  params:parameters
                                                                                                   error:fabricError];
                              if (params != nil) {
                                  if (params.productAttestationAuthorityCertificates == nil) {
                                      params.productAttestationAuthorityCertificates = self->_productAttestationAuthorityCertificates;
                                  }
                                  if (params.certificationDeclarationCertificates == nil) {
                                      params.certificationDeclarationCertificates = self->_certificationDeclarationCertificates;
                                  }
                              }
                              return params;
                          }
                                  error:error];
}

- (void)setMessageReliabilityProtocolIdleRetransmitMs:(nullable NSNumber *)idleRetransmitMs
                                   activeRetransmitMs:(nullable NSNumber *)activeRetransmitMs
                                    activeThresholdMs:(nullable NSNumber *)activeThresholdMs
                          additionalRetransmitDelayMs:(nullable NSNumber *)additionalRetransmitDelayMs
{
    [self _assertCurrentQueueIsNotMatterQueue];
    dispatch_async(_chipWorkQueue, ^{
        bool resetAdvertising;
        if (idleRetransmitMs == nil && activeRetransmitMs == nil && activeThresholdMs == nil && additionalRetransmitDelayMs == nil) {
            Messaging::ReliableMessageMgr::SetAdditionalMRPBackoffTime(NullOptional);
            resetAdvertising = ReliableMessageProtocolConfig::SetLocalMRPConfig(NullOptional);
        } else {
            if (additionalRetransmitDelayMs != nil) {
                System::Clock::Timeout additionalBackoff(additionalRetransmitDelayMs.unsignedLongValue);
                Messaging::ReliableMessageMgr::SetAdditionalMRPBackoffTime(MakeOptional(additionalBackoff));
            }

            // Get current MRP parameters, then override the things we were asked to
            // override.
            ReliableMessageProtocolConfig mrpConfig = GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig());
            if (idleRetransmitMs != nil) {
                mrpConfig.mIdleRetransTimeout = System::Clock::Milliseconds32(idleRetransmitMs.unsignedLongValue);
            }
            if (activeRetransmitMs != nil) {
                mrpConfig.mActiveRetransTimeout = System::Clock::Milliseconds32(activeRetransmitMs.unsignedLongValue);
            }
            if (activeThresholdMs != nil) {
                mrpConfig.mActiveThresholdTime = System::Clock::Milliseconds32(activeThresholdMs.unsignedLongValue);
            }

            resetAdvertising = ReliableMessageProtocolConfig::SetLocalMRPConfig(MakeOptional(mrpConfig));
        }

        if (resetAdvertising) {
            [self resetOperationalAdvertising];
        }
    });
}

- (PersistentStorageDelegate *)storageDelegate
{
    return _persistentStorageDelegate;
}

- (Credentials::GroupDataProvider *)groupDataProvider
{
    return &_groupDataProvider;
}

@end

@interface MTRDummyStorage : NSObject <MTRStorage>
@end

@implementation MTRDummyStorage
- (nullable NSData *)storageDataForKey:(NSString *)key
{
    return nil;
}
- (BOOL)setStorageData:(NSData *)value forKey:(NSString *)key
{
    return NO;
}
- (BOOL)removeStorageDataForKey:(NSString *)key
{
    return NO;
}
@end

@implementation MTRDeviceControllerFactoryParams

- (instancetype)initWithStorage:(id<MTRStorage>)storage
{
    if (!(self = [super init])) {
        return nil;
    }

    _storage = storage;
    _hasStorage = YES;
    _otaProviderDelegate = nil;
    _productAttestationAuthorityCertificates = nil;
    _certificationDeclarationCertificates = nil;
    _port = nil;
    _shouldStartServer = NO;

    return self;
}

- (instancetype)initWithoutStorage
{
    if (!(self = [super init])) {
        return nil;
    }

    // We promise to have a non-null storage for purposes of our attribute, but
    // now we're allowing initialization without storage.  Make up a dummy
    // storage just so we don't have nil there.
    _storage = [[MTRDummyStorage alloc] init];
    _hasStorage = NO;
    _otaProviderDelegate = nil;
    _productAttestationAuthorityCertificates = nil;
    _certificationDeclarationCertificates = nil;
    _port = nil;
    _shouldStartServer = YES;

    return self;
}

@end

@implementation MTRControllerFactory
- (BOOL)isRunning
{
    return [[MTRDeviceControllerFactory sharedInstance] isRunning];
}

+ (MTRControllerFactory *)sharedInstance
{
    // We could try to delegate to MTRDeviceControllerFactory's sharedInstance
    // here, but then we would have to add the backwards-compar selectors to
    // MTRDeviceControllerFactory, etc.  Just forward things along instead.
    // This works because we never accept an MTRControllerFactory as an argument
    // in any of our public APIs.
    static MTRControllerFactory * factory = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // initialize the factory.
        factory = [[MTRControllerFactory alloc] init];
    });
    return factory;
}

- (BOOL)startup:(MTRControllerFactoryParams *)startupParams
{
    return [[MTRDeviceControllerFactory sharedInstance] startControllerFactory:startupParams error:nil];
}

- (void)shutdown
{
    return [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
}

- (MTRDeviceController * _Nullable)startControllerOnExistingFabric:(MTRDeviceControllerStartupParams *)startupParams
{
    return [[MTRDeviceControllerFactory sharedInstance] createControllerOnExistingFabric:startupParams error:nil];
}

- (MTRDeviceController * _Nullable)startControllerOnNewFabric:(MTRDeviceControllerStartupParams *)startupParams
{
    return [[MTRDeviceControllerFactory sharedInstance] createControllerOnNewFabric:startupParams error:nil];
}

@end

@implementation MTRControllerFactoryParams

- (id<MTRPersistentStorageDelegate>)storageDelegate
{
    // Cast is safe, because MTRPersistentStorageDelegate doesn't add
    // any selectors to MTRStorage, so anything implementing
    // MTRStorage also implements MTRPersistentStorageDelegate.
    return static_cast<id<MTRPersistentStorageDelegate>>(self.storage);
}

- (BOOL)startServer
{
    return self.shouldStartServer;
}

- (void)setStartServer:(BOOL)startServer
{
    self.shouldStartServer = startServer;
}

- (nullable NSArray<NSData *> *)paaCerts
{
    return self.productAttestationAuthorityCertificates;
}

- (void)setPaaCerts:(nullable NSArray<NSData *> *)paaCerts
{
    self.productAttestationAuthorityCertificates = paaCerts;
}

- (nullable NSArray<NSData *> *)cdCerts
{
    return self.certificationDeclarationCertificates;
}

- (void)setCdCerts:(nullable NSArray<NSData *> *)cdCerts
{
    self.certificationDeclarationCertificates = cdCerts;
}

@end

void MTRSetMessageReliabilityParameters(NSNumber * _Nullable idleRetransmitMs,
    NSNumber * _Nullable activeRetransmitMs,
    NSNumber * _Nullable activeThresholdMs,
    NSNumber * _Nullable additionalRetransmitDelayMs)
{
    [MTRDeviceControllerFactory.sharedInstance setMessageReliabilityProtocolIdleRetransmitMs:idleRetransmitMs
                                                                          activeRetransmitMs:activeThresholdMs
                                                                           activeThresholdMs:activeThresholdMs
                                                                 additionalRetransmitDelayMs:additionalRetransmitDelayMs];
}
