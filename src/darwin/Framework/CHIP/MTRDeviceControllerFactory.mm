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

#import "MTRAttestationTrustStoreBridge.h"
#import "MTRCertificates.h"
#import "MTRControllerAccessControl.h"
#import "MTRDeviceController.h"
#import "MTRDeviceControllerStartupParams.h"
#import "MTRDeviceControllerStartupParams_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTRError_Internal.h"
#import "MTRFabricInfo_Internal.h"
#import "MTRFramework.h"
#import "MTRLogging_Internal.h"
#import "MTROTAProviderDelegateBridge.h"
#import "MTROperationalBrowser.h"
#import "MTRP256KeypairBridge.h"
#import "MTRPersistentStorageDelegateBridge.h"
#import "NSDataSpanConversion.h"

#import <os/lock.h>

#include <controller/CHIPDeviceControllerFactory.h>
#include <credentials/CHIPCert.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <crypto/RawKeySessionKeystore.h>
#include <lib/support/Pool.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/PlatformManager.h>

#include <cstdlib>

using namespace chip;
using namespace chip::Controller;

static NSString * const kErrorPersistentStorageInit = @"Init failure while creating a persistent storage delegate";
static NSString * const kErrorAttestationTrustStoreInit = @"Init failure while creating the attestation trust store";
static NSString * const kErrorDACVerifierInit = @"Init failure while creating the device attestation verifier";
static NSString * const kErrorGroupProviderInit = @"Init failure while initializing group data provider";
static NSString * const kErrorControllersInit = @"Init controllers array failure";
static NSString * const kErrorCertificateValidityPolicyInit = @"Init certificate validity policy failure";
static NSString * const kErrorControllerFactoryInit = @"Init failure while initializing controller factory";
static NSString * const kErrorKeystoreInit = @"Init failure while initializing persistent storage keystore";
static NSString * const kErrorCertStoreInit = @"Init failure while initializing persistent storage operational certificate store";
static NSString * const kErrorCDCertStoreInit = @"Init failure while initializing Certificate Declaration Signing Keys store";
static NSString * const kErrorOtaProviderInit = @"Init failure while creating an OTA provider delegate";
static NSString * const kErrorSessionKeystoreInit = @"Init failure while initializing session keystore";

static bool sExitHandlerRegistered = false;
static void ShutdownOnExit() { [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory]; }

@interface MTRDeviceControllerFactory ()

@property (atomic, readonly) dispatch_queue_t chipWorkQueue;
@property (readonly) DeviceControllerFactory * controllerFactory;
@property (readonly) PersistentStorageDelegate * persistentStorageDelegate;
@property (readonly) MTRAttestationTrustStoreBridge * attestationTrustStoreBridge;
@property (readonly) MTROTAProviderDelegateBridge * otaProviderDelegateBridge;
@property (readonly) Crypto::RawKeySessionKeystore * sessionKeystore;
// We use TestPersistentStorageDelegate just to get an in-memory store to back
// our group data provider impl.  We initialize this store correctly on every
// controller startup, so don't need to actually persist it.
@property (readonly) TestPersistentStorageDelegate * groupStorageDelegate;
@property (readonly) Credentials::GroupDataProviderImpl * groupDataProvider;
@property (readonly) NSMutableArray<MTRDeviceController *> * controllers;
@property (readonly) PersistentStorageOperationalKeystore * keystore;
@property (readonly) Credentials::PersistentStorageOpCertStore * opCertStore;
@property (readonly) MTROperationalBrowser * operationalBrowser;
@property () chip::Credentials::DeviceAttestationVerifier * deviceAttestationVerifier;
@property (readonly) BOOL advertiseOperational;
@property (nonatomic, readonly) Credentials::IgnoreCertificateValidityPeriodPolicy * certificateValidityPolicy;
// Lock used to serialize access to the "controllers" array, since it needs to
// be touched from both whatever queue is starting controllers and from the
// Matter queue.  The way this lock is used assumes that:
//
// 1) The only mutating accesses to the controllers array happen when the
//    current queue is not the Matter queue.  This is a good assumption, because
//    the implementation of the functions that mutate the array do sync dispatch
//    to the Matter queue, which would deadlock if they were called when that
//    queue was the current queue.
// 2) It's our API consumer's responsibility to serialize access to us from
//    outside.
//
// This means that we only take the lock around mutations of the array and
// accesses to the array that are from code running on the Matter queue.

@property (nonatomic, readonly) os_unfair_lock controllersLock;

- (BOOL)findMatchingFabric:(FabricTable &)fabricTable
                    params:(MTRDeviceControllerStartupParams *)params
                    fabric:(const FabricInfo * _Nullable * _Nonnull)fabric;

- (MTRDeviceController * _Nullable)maybeInitializeOTAProvider:(MTRDeviceController * _Nonnull)controller;
@end

@implementation MTRDeviceControllerFactory

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

    _running = NO;
    _chipWorkQueue = DeviceLayer::PlatformMgrImpl().GetWorkQueue();
    _controllerFactory = &DeviceControllerFactory::GetInstance();
    _controllersLock = OS_UNFAIR_LOCK_INIT;

    _sessionKeystore = new chip::Crypto::RawKeySessionKeystore();
    if ([self checkForInitError:(_sessionKeystore != nullptr) logMsg:kErrorSessionKeystoreInit]) {
        return nil;
    }

    _groupStorageDelegate = new chip::TestPersistentStorageDelegate();
    if ([self checkForInitError:(_groupStorageDelegate != nullptr) logMsg:kErrorGroupProviderInit]) {
        return nil;
    }

    // For now default args are fine, since we are just using this for the IPK.
    _groupDataProvider = new chip::Credentials::GroupDataProviderImpl();
    if ([self checkForInitError:(_groupDataProvider != nullptr) logMsg:kErrorGroupProviderInit]) {
        return nil;
    }

    _groupDataProvider->SetStorageDelegate(_groupStorageDelegate);
    _groupDataProvider->SetSessionKeystore(_sessionKeystore);
    CHIP_ERROR errorCode = _groupDataProvider->Init();
    if ([self checkForInitError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorGroupProviderInit]) {
        return nil;
    }

    _controllers = [[NSMutableArray alloc] init];
    if ([self checkForInitError:(_controllers != nil) logMsg:kErrorControllersInit]) {
        return nil;
    }

    _certificateValidityPolicy = new Credentials::IgnoreCertificateValidityPeriodPolicy();
    if ([self checkForInitError:(_certificateValidityPolicy != nil) logMsg:kErrorCertificateValidityPolicyInit]) {
        return nil;
    }

    return self;
}

- (void)dealloc
{
    [self stopControllerFactory];
    [self cleanupInitObjects];
}

- (void)_assertCurrentQueueIsNotMatterQueue
{
    VerifyOrDie(!DeviceLayer::PlatformMgrImpl().IsWorkQueueCurrentQueue());
}

- (BOOL)checkIsRunning:(NSError * __autoreleasing *)error
{
    [self _assertCurrentQueueIsNotMatterQueue];

    if ([self isRunning]) {
        return YES;
    }

    if (error != nil) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }

    return NO;
}

- (BOOL)checkForInitError:(BOOL)condition logMsg:(NSString *)logMsg
{
    if (condition) {
        return NO;
    }

    MTR_LOG_ERROR("Error: %@", logMsg);

    [self cleanupInitObjects];

    return YES;
}

- (void)cleanupInitObjects
{
    _controllers = nil;

    if (_groupDataProvider) {
        _groupDataProvider->Finish();
        delete _groupDataProvider;
        _groupDataProvider = nullptr;
    }

    if (_groupStorageDelegate) {
        delete _groupStorageDelegate;
        _groupStorageDelegate = nullptr;
    }

    if (_sessionKeystore) {
        delete _sessionKeystore;
        _sessionKeystore = nullptr;
    }

    if (_certificateValidityPolicy) {
        delete _certificateValidityPolicy;
        _certificateValidityPolicy = nullptr;
    }
}

- (void)cleanupStartupObjects
{
    if (_deviceAttestationVerifier) {
        delete _deviceAttestationVerifier;
        _deviceAttestationVerifier = nullptr;
    }

    if (_attestationTrustStoreBridge) {
        delete _attestationTrustStoreBridge;
        _attestationTrustStoreBridge = nullptr;
    }

    if (_otaProviderDelegateBridge) {
        delete _otaProviderDelegateBridge;
        _otaProviderDelegateBridge = nullptr;
    }

    if (_keystore) {
        _keystore->Finish();
        delete _keystore;
        _keystore = nullptr;
    }

    if (_opCertStore) {
        _opCertStore->Finish();
        delete _opCertStore;
        _opCertStore = nullptr;
    }

    if (_persistentStorageDelegate) {
        delete _persistentStorageDelegate;
        _persistentStorageDelegate = nullptr;
    }
}

- (nullable NSArray<MTRFabricInfo *> *)knownFabrics
{
    [self _assertCurrentQueueIsNotMatterQueue];

    if (!self.isRunning) {
        return nil;
    }

    __block NSMutableArray<MTRFabricInfo *> * fabricList;
    __block BOOL listFilled = NO;
    auto fillListBlock = ^{
        FabricTable fabricTable;
        CHIP_ERROR err = fabricTable.Init({ .storage = self->_persistentStorageDelegate,
            .operationalKeystore = self->_keystore,
            .opCertStore = self->_opCertStore });
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
    };

    if ([_controllers count] > 0) {
        // We have a controller running already, so our task queue is live.
        // Make sure we run on that queue so we don't race against it.
        dispatch_sync(_chipWorkQueue, fillListBlock);
    } else {
        // Not currently running the task queue; just run the block directly.
        fillListBlock();
    }

    if (listFilled == NO) {
        return nil;
    }

    return [NSArray arrayWithArray:fabricList];
}

- (BOOL)startControllerFactory:(MTRDeviceControllerFactoryParams *)startupParams error:(NSError * __autoreleasing *)error;
{
    [self _assertCurrentQueueIsNotMatterQueue];

    if ([self isRunning]) {
        MTR_LOG_DEBUG("Ignoring duplicate call to startup, Matter controller factory already started...");
        return YES;
    }

    DeviceLayer::PlatformMgrImpl().StartEventLoopTask();

    __block CHIP_ERROR errorCode = CHIP_NO_ERROR;
    dispatch_sync(_chipWorkQueue, ^{
        if ([self isRunning]) {
            return;
        }

        [MTRControllerAccessControl init];

        _persistentStorageDelegate = new MTRPersistentStorageDelegateBridge(startupParams.storage);
        if (_persistentStorageDelegate == nil) {
            MTR_LOG_ERROR("Error: %@", kErrorPersistentStorageInit);
            errorCode = CHIP_ERROR_NO_MEMORY;
            return;
        }

        if (startupParams.otaProviderDelegate) {
            if (![startupParams.otaProviderDelegate respondsToSelector:@selector(handleQueryImageForNodeID:
                                                                                                controller:params:completion:)]
                && ![startupParams.otaProviderDelegate
                    respondsToSelector:@selector(handleQueryImageForNodeID:controller:params:completionHandler:)]) {
                MTR_LOG_ERROR("Error: MTROTAProviderDelegate does not support handleQueryImageForNodeID");
                errorCode = CHIP_ERROR_INVALID_ARGUMENT;
                return;
            }
            if (![startupParams.otaProviderDelegate
                    respondsToSelector:@selector(handleApplyUpdateRequestForNodeID:controller:params:completion:)]
                && ![startupParams.otaProviderDelegate
                    respondsToSelector:@selector(handleApplyUpdateRequestForNodeID:controller:params:completionHandler:)]) {
                MTR_LOG_ERROR("Error: MTROTAProviderDelegate does not support handleApplyUpdateRequestForNodeID");
                errorCode = CHIP_ERROR_INVALID_ARGUMENT;
                return;
            }
            if (![startupParams.otaProviderDelegate
                    respondsToSelector:@selector(handleNotifyUpdateAppliedForNodeID:controller:params:completion:)]
                && ![startupParams.otaProviderDelegate
                    respondsToSelector:@selector(handleNotifyUpdateAppliedForNodeID:controller:params:completionHandler:)]) {
                MTR_LOG_ERROR("Error: MTROTAProviderDelegate does not support handleNotifyUpdateAppliedForNodeID");
                errorCode = CHIP_ERROR_INVALID_ARGUMENT;
                return;
            }
            if (![startupParams.otaProviderDelegate
                    respondsToSelector:@selector(handleBDXTransferSessionBeginForNodeID:
                                                                             controller:fileDesignator:offset:completion:)]
                && ![startupParams.otaProviderDelegate
                    respondsToSelector:@selector
                    (handleBDXTransferSessionBeginForNodeID:controller:fileDesignator:offset:completionHandler:)]) {
                MTR_LOG_ERROR("Error: MTROTAProviderDelegate does not support handleBDXTransferSessionBeginForNodeID");
                errorCode = CHIP_ERROR_INVALID_ARGUMENT;
                return;
            }
            if (![startupParams.otaProviderDelegate
                    respondsToSelector:@selector(handleBDXQueryForNodeID:controller:blockSize:blockIndex:bytesToSkip:completion:)]
                && ![startupParams.otaProviderDelegate
                    respondsToSelector:@selector(handleBDXQueryForNodeID:
                                                              controller:blockSize:blockIndex:bytesToSkip:completionHandler:)]) {
                MTR_LOG_ERROR("Error: MTROTAProviderDelegate does not support handleBDXQueryForNodeID");
                errorCode = CHIP_ERROR_INVALID_ARGUMENT;
                return;
            }
            _otaProviderDelegateBridge = new MTROTAProviderDelegateBridge(startupParams.otaProviderDelegate);
            if (_otaProviderDelegateBridge == nil) {
                MTR_LOG_ERROR("Error: %@", kErrorOtaProviderInit);
                errorCode = CHIP_ERROR_NO_MEMORY;
                return;
            }
        }

        // TODO: Allow passing a different keystore implementation via startupParams.
        _keystore = new PersistentStorageOperationalKeystore();
        if (_keystore == nullptr) {
            MTR_LOG_ERROR("Error: %@", kErrorKeystoreInit);
            errorCode = CHIP_ERROR_NO_MEMORY;
            return;
        }

        errorCode = _keystore->Init(_persistentStorageDelegate);
        if (errorCode != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error: %@", kErrorKeystoreInit);
            return;
        }

        // TODO Allow passing a different opcert store implementation via startupParams.
        _opCertStore = new Credentials::PersistentStorageOpCertStore();
        if (_opCertStore == nullptr) {
            MTR_LOG_ERROR("Error: %@", kErrorCertStoreInit);
            errorCode = CHIP_ERROR_NO_MEMORY;
            return;
        }

        errorCode = _opCertStore->Init(_persistentStorageDelegate);
        if (errorCode != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error: %@", kErrorCertStoreInit);
            return;
        }

        // Initialize device attestation verifier
        const Credentials::AttestationTrustStore * trustStore;
        if (startupParams.productAttestationAuthorityCertificates) {
            _attestationTrustStoreBridge
                = new MTRAttestationTrustStoreBridge(startupParams.productAttestationAuthorityCertificates);
            if (_attestationTrustStoreBridge == nullptr) {
                MTR_LOG_ERROR("Error: %@", kErrorAttestationTrustStoreInit);
                errorCode = CHIP_ERROR_NO_MEMORY;
                return;
            }
            trustStore = _attestationTrustStoreBridge;
        } else {
            // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
            trustStore = Credentials::GetTestAttestationTrustStore();
        }
        _deviceAttestationVerifier = new Credentials::DefaultDACVerifier(trustStore);
        if (_deviceAttestationVerifier == nullptr) {
            MTR_LOG_ERROR("Error: %@", kErrorDACVerifierInit);
            errorCode = CHIP_ERROR_NO_MEMORY;
            return;
        }

        if (startupParams.certificationDeclarationCertificates) {
            auto cdTrustStore = _deviceAttestationVerifier->GetCertificationDeclarationTrustStore();
            if (cdTrustStore == nullptr) {
                MTR_LOG_ERROR("Error: %@", kErrorCDCertStoreInit);
                errorCode = CHIP_ERROR_INCORRECT_STATE;
                return;
            }

            for (NSData * cdSigningCert in startupParams.certificationDeclarationCertificates) {
                errorCode = cdTrustStore->AddTrustedKey(AsByteSpan(cdSigningCert));
                if (errorCode != CHIP_NO_ERROR) {
                    MTR_LOG_ERROR("Error: %@", kErrorCDCertStoreInit);
                    return;
                }
            }
        }

        chip::Controller::FactoryInitParams params;
        if (startupParams.port != nil) {
            params.listenPort = [startupParams.port unsignedShortValue];
        }
        params.enableServerInteractions = startupParams.shouldStartServer;

        params.groupDataProvider = _groupDataProvider;
        params.sessionKeystore = _sessionKeystore;
        params.fabricIndependentStorage = _persistentStorageDelegate;
        params.operationalKeystore = _keystore;
        params.opCertStore = _opCertStore;
        params.certificateValidityPolicy = _certificateValidityPolicy;
        errorCode = _controllerFactory->Init(params);
        if (errorCode != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error: %@", kErrorControllerFactoryInit);
            return;
        }

        // This needs to happen after DeviceControllerFactory::Init,
        // because that creates (lazily, by calling functions with
        // static variables in them) some static-lifetime objects.
        if (!sExitHandlerRegistered) {
            int ret = atexit(ShutdownOnExit);
            if (ret != 0) {
                MTR_LOG_ERROR("Error registering exit handler: %d", ret);
                return;
            }
        }
        HeapObjectPoolExitHandling::IgnoreLeaksOnExit();

        // Make sure we don't leave a system state running while we have no
        // controllers started.  This is working around the fact that a system
        // state is brought up live on factory init, and not when it comes time
        // to actually start a controller, and does not actually clean itself up
        // until its refcount (which starts as 0) goes to 0.
        _controllerFactory->RetainSystemState();
        _controllerFactory->ReleaseSystemState();

        self->_advertiseOperational = startupParams.shouldStartServer;
        self->_running = YES;
    });

    // Make sure to stop the event loop again before returning, so we are not running it while we don't have any controllers.
    DeviceLayer::PlatformMgrImpl().StopEventLoopTask();

    if (![self isRunning]) {
        [self cleanupStartupObjects];
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:errorCode];
        }
    }

    return [self isRunning];
}

- (void)stopControllerFactory
{
    [self _assertCurrentQueueIsNotMatterQueue];

    if (![self isRunning]) {
        return;
    }

    while ([_controllers count] != 0) {
        [_controllers[0] shutdown];
    }

    MTR_LOG_DEBUG("Shutting down the Matter controller factory");
    _controllerFactory->Shutdown();

    [self cleanupStartupObjects];

    // NOTE: we do not call cleanupInitObjects because we can be restarted, and
    // that does not re-create the objects that we create inside init.
    // Maybe we should be creating them in startup?

    _running = NO;
}

/**
 * Helper function to start a device controller with the given startup params.
 * The fabricChecker block will run on the Matter queue, and is expected to
 * return nil if pre-startup fabric table checks fail, and set fabricError to
 * the right error value in that situation.
 */
- (MTRDeviceController * _Nullable)_startDeviceController:(MTRDeviceControllerStartupParams *)startupParams
                                            fabricChecker:(MTRDeviceControllerStartupParamsInternal * (^)(FabricTable * fabricTable,
                                                              MTRDeviceController * controller,
                                                              CHIP_ERROR & fabricError))fabricChecker
                                                    error:(NSError * __autoreleasing *)error
{
    [self _assertCurrentQueueIsNotMatterQueue];

    if (![self checkIsRunning:error]) {
        MTR_LOG_ERROR("Trying to start controller while Matter controller factory is not running");
        return nil;
    }

    // Create the controller, so we start the event loop, since we plan to do
    // our fabric table operations there.
    auto * controller = [self createController];
    if (controller == nil) {
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_NO_MEMORY];
        }
        return nil;
    }

    __block MTRDeviceControllerStartupParamsInternal * params = nil;
    __block CHIP_ERROR fabricError = CHIP_NO_ERROR;

    // We want the block to end up with just a pointer to the fabric table,
    // since we know our on-stack instance will outlive the block.
    FabricTable fabricTableInstance;
    FabricTable * fabricTable = &fabricTableInstance;

    dispatch_sync(_chipWorkQueue, ^{
        params = fabricChecker(fabricTable, controller, fabricError);
    });

    if (params == nil) {
        [self controllerShuttingDown:controller];
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:fabricError];
        }
        return nil;
    }

    BOOL ok = [controller startup:params];
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

    return [self _startDeviceController:startupParams
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
                                                                                     advertiseOperational:self.advertiseOperational
                                                                                                   params:startupParams];
                              if (params == nil) {
                                  fabricError = CHIP_ERROR_NO_MEMORY;
                              }

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

    return [self _startDeviceController:startupParams
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
                                                                                advertiseOperational:self.advertiseOperational
                                                                                              params:startupParams];
                              if (params == nil) {
                                  fabricError = CHIP_ERROR_NO_MEMORY;
                              }
                              return params;
                          }
                                  error:error];
}

- (MTRDeviceController * _Nullable)createController
{
    [self _assertCurrentQueueIsNotMatterQueue];

    MTRDeviceController * controller = [[MTRDeviceController alloc] initWithFactory:self queue:_chipWorkQueue];
    if (controller == nil) {
        MTR_LOG_ERROR("Failed to init controller");
        return nil;
    }

    if ([_controllers count] == 0) {
        // Bringing up the first controller.  Start the event loop now.  If we
        // fail to bring it up, its cleanup will stop the event loop again.
        chip::DeviceLayer::PlatformMgrImpl().StartEventLoopTask();
        dispatch_sync(_chipWorkQueue, ^{
            self->_operationalBrowser = new MTROperationalBrowser(self, self->_chipWorkQueue);
        });
    }

    // Add the controller to _controllers now, so if we fail partway through its
    // startup we will still do the right cleanups.
    os_unfair_lock_lock(&_controllersLock);
    [_controllers addObject:controller];
    os_unfair_lock_unlock(&_controllersLock);

    return controller;
}

// Finds a fabric that matches the given params, if one exists.
//
// Returns NO on failure, YES on success.  If YES is returned, the
// outparam will be written to, but possibly with a null value.
//
// fabricTable should be an un-initialized fabric table.  It needs to
// outlive the consumer's use of the FabricInfo we return, which is
// why it's provided by the caller.
- (BOOL)findMatchingFabric:(FabricTable &)fabricTable
                    params:(MTRDeviceControllerStartupParams *)params
                    fabric:(const FabricInfo * _Nullable * _Nonnull)fabric
{
    assertChipStackLockedByCurrentThread();

    CHIP_ERROR err = fabricTable.Init(
        { .storage = _persistentStorageDelegate, .operationalKeystore = _keystore, .opCertStore = _opCertStore });
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Can't initialize fabric table: %s", ErrorStr(err));
        return NO;
    }

    Crypto::P256PublicKey pubKey;
    if (params.rootCertificate != nil) {
        err = ExtractPubkeyFromX509Cert(AsByteSpan(params.rootCertificate), pubKey);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Can't extract public key from root certificate: %s", ErrorStr(err));
            return NO;
        }
    } else {
        // No root certificate means the nocSigner is using the root keys, because
        // consumers must provide a root certificate whenever an ICA is used.
        err = MTRP256KeypairBridge::MatterPubKeyFromSecKeyRef(params.nocSigner.publicKey, &pubKey);
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

    VerifyOrReturnValue(_otaProviderDelegateBridge != nil, controller);
    VerifyOrReturnValue([_controllers count] == 1, controller);

    __block CHIP_ERROR err;
    dispatch_sync(_chipWorkQueue, ^{
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

@end

@implementation MTRDeviceControllerFactory (InternalMethods)

- (void)controllerShuttingDown:(MTRDeviceController *)controller
{
    [self _assertCurrentQueueIsNotMatterQueue];

    if (![_controllers containsObject:controller]) {
        MTR_LOG_ERROR("Controller we don't know about shutting down");
        return;
    }

    if (_groupDataProvider != nullptr) {
        dispatch_sync(_chipWorkQueue, ^{
            FabricIndex idx = [controller fabricIndex];
            if (idx != kUndefinedFabricIndex) {
                // Clear out out group keys for this fabric index, just in case fabric
                // indices get reused later.  If a new controller is started on the
                // same fabric it will be handed the IPK at that point.
                self->_groupDataProvider->RemoveGroupKeys(idx);
            }
        });
    }

    os_unfair_lock_lock(&_controllersLock);
    [_controllers removeObject:controller];
    os_unfair_lock_unlock(&_controllersLock);

    if ([_controllers count] == 0) {
        dispatch_sync(_chipWorkQueue, ^{
            delete self->_operationalBrowser;
            self->_operationalBrowser = nullptr;
        });
        // That was our last controller.  Stop the event loop before it
        // shuts down, because shutdown of the last controller will tear
        // down most of the world.
        DeviceLayer::PlatformMgrImpl().StopEventLoopTask();

        if (_otaProviderDelegateBridge) {
            _otaProviderDelegateBridge->Shutdown();
        }

        [controller shutDownCppController];
    } else {
        // Do the controller shutdown on the Matter work queue.
        dispatch_sync(_chipWorkQueue, ^{
            if (_otaProviderDelegateBridge) {
                _otaProviderDelegateBridge->ControllerShuttingDown(controller);
            }

            [controller shutDownCppController];
        });
    }

    [controller deinitFromFactory];
}

- (NSArray<MTRDeviceController *> *)getRunningControllers
{
    os_unfair_lock_lock(&_controllersLock);
    NSArray<MTRDeviceController *> * controllersCopy = [_controllers copy];
    os_unfair_lock_unlock(&_controllersLock);
    return controllersCopy;
}

- (nullable MTRDeviceController *)runningControllerForFabricIndex:(chip::FabricIndex)fabricIndex
{
    assertChipStackLockedByCurrentThread();

    auto * controllersCopy = [self getRunningControllers];

    for (MTRDeviceController * existing in controllersCopy) {
        if ([existing fabricIndex] == fabricIndex) {
            return existing;
        }
    }

    return nil;
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

- (PersistentStorageDelegate *)storageDelegate
{
    return _persistentStorageDelegate;
}

- (Credentials::GroupDataProvider *)groupData
{
    return _groupDataProvider;
}

@end

@implementation MTRDeviceControllerFactoryParams

- (instancetype)initWithStorage:(id<MTRStorage>)storage
{
    if (!(self = [super init])) {
        return nil;
    }

    _storage = storage;
    _otaProviderDelegate = nil;
    _productAttestationAuthorityCertificates = nil;
    _certificationDeclarationCertificates = nil;
    _port = nil;
    _shouldStartServer = NO;

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
