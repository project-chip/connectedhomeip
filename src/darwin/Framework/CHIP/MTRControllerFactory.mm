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

#import "MTRControllerFactory.h"
#import "MTRControllerFactory_Internal.h"

#import "MTRAttestationTrustStoreBridge.h"
#import "MTRCertificates.h"
#import "MTRControllerAccessControl.h"
#import "MTRDeviceController.h"
#import "MTRDeviceControllerStartupParams.h"
#import "MTRDeviceControllerStartupParams_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTRLogging.h"
#import "MTRMemory.h"
#import "MTROTAProviderDelegateBridge.h"
#import "MTRP256KeypairBridge.h"
#import "MTRPersistentStorageDelegateBridge.h"
#import "NSDataSpanConversion.h"

#include <controller/CHIPDeviceControllerFactory.h>
#include <credentials/CHIPCert.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/support/Pool.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::Controller;

static NSString * const kErrorPersistentStorageInit = @"Init failure while creating a persistent storage delegate";
static NSString * const kErrorAttestationTrustStoreInit = @"Init failure while creating the attestation trust store";
static NSString * const kErrorDACVerifierInit = @"Init failure while creating the device attestation verifier";
static NSString * const kInfoFactoryShutdown = @"Shutting down the Matter controller factory";
static NSString * const kErrorGroupProviderInit = @"Init failure while initializing group data provider";
static NSString * const kErrorControllersInit = @"Init controllers array failure";
static NSString * const kErrorControllerFactoryInit = @"Init failure while initializing controller factory";
static NSString * const kErrorKeystoreInit = @"Init failure while initializing persistent storage keystore";
static NSString * const kErrorCertStoreInit = @"Init failure while initializing persistent storage operational certificate store";
static NSString * const kErrorCDCertStoreInit = @"Init failure while initializing Certificate Declaration Signing Keys store";
static NSString * const kErrorOtaProviderInit = @"Init failure while creating an OTA provider delegate";

@interface MTRControllerFactory ()

@property (atomic, readonly) dispatch_queue_t chipWorkQueue;
@property (readonly) DeviceControllerFactory * controllerFactory;
@property (readonly) MTRPersistentStorageDelegateBridge * persistentStorageDelegateBridge;
@property (readonly) MTRAttestationTrustStoreBridge * attestationTrustStoreBridge;
@property (readonly) MTROTAProviderDelegateBridge * otaProviderDelegateBridge;
// We use TestPersistentStorageDelegate just to get an in-memory store to back
// our group data provider impl.  We initialize this store correctly on every
// controller startup, so don't need to actually persist it.
@property (readonly) TestPersistentStorageDelegate * groupStorageDelegate;
@property (readonly) Credentials::GroupDataProviderImpl * groupDataProvider;
@property (readonly) NSMutableArray<MTRDeviceController *> * controllers;
@property (readonly) PersistentStorageOperationalKeystore * keystore;
@property (readonly) Credentials::PersistentStorageOpCertStore * opCertStore;
@property () chip::Credentials::DeviceAttestationVerifier * deviceAttestationVerifier;

- (BOOL)findMatchingFabric:(FabricTable &)fabricTable
                    params:(MTRDeviceControllerStartupParams *)params
                    fabric:(const FabricInfo * _Nullable * _Nonnull)fabric;

- (MTRDeviceController * _Nullable)maybeInitializeOTAProvider:(MTRDeviceController * _Nonnull)controller;
@end

@implementation MTRControllerFactory

+ (instancetype)sharedInstance
{
    static MTRControllerFactory * factory = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // initialize the factory.
        factory = [[MTRControllerFactory alloc] init];
    });
    return factory;
}

- (instancetype)init
{
    if (!(self = [super init])) {
        return nil;
    }

    _isRunning = NO;
    _chipWorkQueue = DeviceLayer::PlatformMgrImpl().GetWorkQueue();
    _controllerFactory = &DeviceControllerFactory::GetInstance();
    [MTRMemory ensureInit];

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
    CHIP_ERROR errorCode = _groupDataProvider->Init();
    if ([self checkForInitError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorGroupProviderInit]) {
        return nil;
    }

    _controllers = [[NSMutableArray alloc] init];
    if ([self checkForInitError:(_controllers != nil) logMsg:kErrorControllersInit]) {
        return nil;
    }

    return self;
}

- (void)dealloc
{
    [self shutdown];
    [self cleanupInitObjects];
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

    if (_persistentStorageDelegateBridge) {
        delete _persistentStorageDelegateBridge;
        _persistentStorageDelegateBridge = nullptr;
    }
}

- (BOOL)startup:(MTRControllerFactoryParams *)startupParams
{
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

        _persistentStorageDelegateBridge = new MTRPersistentStorageDelegateBridge(startupParams.storage);
        if (_persistentStorageDelegateBridge == nil) {
            MTR_LOG_ERROR("Error: %@", kErrorPersistentStorageInit);
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
                return;
            }
        }

        // TODO: Allow passing a different keystore implementation via startupParams.
        _keystore = new PersistentStorageOperationalKeystore();
        if (_keystore == nullptr) {
            MTR_LOG_ERROR("Error: %@", kErrorKeystoreInit);
            return;
        }

        errorCode = _keystore->Init(_persistentStorageDelegateBridge);
        if (errorCode != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error: %@", kErrorKeystoreInit);
            return;
        }

        // TODO Allow passing a different opcert store implementation via startupParams.
        _opCertStore = new Credentials::PersistentStorageOpCertStore();
        if (_opCertStore == nullptr) {
            MTR_LOG_ERROR("Error: %@", kErrorCertStoreInit);
            return;
        }

        errorCode = _opCertStore->Init(_persistentStorageDelegateBridge);
        if (errorCode != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error: %@", kErrorCertStoreInit);
            return;
        }

        // Initialize device attestation verifier
        const Credentials::AttestationTrustStore * trustStore;
        if (startupParams.paaCerts) {
            _attestationTrustStoreBridge = new MTRAttestationTrustStoreBridge(startupParams.paaCerts);
            if (_attestationTrustStoreBridge == nullptr) {
                MTR_LOG_ERROR("Error: %@", kErrorAttestationTrustStoreInit);
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
            return;
        }

        if (startupParams.cdCerts) {
            auto cdTrustStore = _deviceAttestationVerifier->GetCertificationDeclarationTrustStore();
            if (cdTrustStore == nullptr) {
                MTR_LOG_ERROR("Error: %@", kErrorCDCertStoreInit);
                return;
            }

            for (NSData * cdSigningCert in startupParams.cdCerts) {
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
        if (startupParams.startServer == YES) {
            params.enableServerInteractions = true;
        }

        params.groupDataProvider = _groupDataProvider;
        params.fabricIndependentStorage = _persistentStorageDelegateBridge;
        params.operationalKeystore = _keystore;
        params.opCertStore = _opCertStore;
        errorCode = _controllerFactory->Init(params);
        if (errorCode != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error: %@", kErrorControllerFactoryInit);
            return;
        }

        // This needs to happen after DeviceControllerFactory::Init,
        // because that creates (lazily, by calling functions with
        // static variables in them) some static-lifetime objects.
        chip::HeapObjectPoolExitHandling::IgnoreLeaksOnExit();

        // Make sure we don't leave a system state running while we have no
        // controllers started.  This is working around the fact that a system
        // state is brought up live on factory init, and not when it comes time
        // to actually start a controller, and does not actually clean itself up
        // until its refcount (which starts as 0) goes to 0.
        _controllerFactory->RetainSystemState();
        _controllerFactory->ReleaseSystemState();

        self->_isRunning = YES;
    });

    // Make sure to stop the event loop again before returning, so we are not running it while we don't have any controllers.
    DeviceLayer::PlatformMgrImpl().StopEventLoopTask();

    if (![self isRunning]) {
        [self cleanupStartupObjects];
    }

    return [self isRunning];
}

- (void)shutdown
{
    if (![self isRunning]) {
        return;
    }

    while ([_controllers count] != 0) {
        [_controllers[0] shutdown];
    }

    MTR_LOG_DEBUG("%@", kInfoFactoryShutdown);
    _controllerFactory->Shutdown();

    [self cleanupStartupObjects];

    // NOTE: we do not call cleanupInitObjects because we can be restarted, and
    // that does not re-create the objects that we create inside init.
    // Maybe we should be creating them in startup?

    _isRunning = NO;
}

- (MTRDeviceController * _Nullable)startControllerOnExistingFabric:(MTRDeviceControllerStartupParams *)startupParams
{
    if (![self isRunning]) {
        MTR_LOG_ERROR("Trying to start controller while Matter controller factory is not running");
        return nil;
    }

    // Create the controller, so we start the event loop, since we plan to do
    // our fabric table operations there.
    auto * controller = [self createController];
    if (controller == nil) {
        return nil;
    }

    __block MTRDeviceControllerStartupParamsInternal * params = nil;
    // We want the block to end up with just a pointer to the fabric table,
    // since we know our on-stack instance will outlive the block.
    FabricTable fabricTableInstance;
    FabricTable * fabricTable = &fabricTableInstance;
    dispatch_sync(_chipWorkQueue, ^{
        const FabricInfo * fabric = nullptr;
        BOOL ok = [self findMatchingFabric:*fabricTable params:startupParams fabric:&fabric];
        if (!ok) {
            MTR_LOG_ERROR("Can't start on existing fabric: fabric matching failed");
            return;
        }

        if (fabric == nullptr) {
            MTR_LOG_ERROR("Can't start on existing fabric: fabric not found");
            return;
        }

        for (MTRDeviceController * existing in _controllers) {
            BOOL isRunning = YES; // assume the worst
            if ([existing isRunningOnFabric:fabricTable fabricIndex:fabric->GetFabricIndex() isRunning:&isRunning]
                != CHIP_NO_ERROR) {
                MTR_LOG_ERROR("Can't tell what fabric a controller is running on.  Not safe to start.");
                return;
            }

            if (isRunning) {
                MTR_LOG_ERROR("Can't start on existing fabric: another controller is running on it");
                return;
            }
        }

        params = [[MTRDeviceControllerStartupParamsInternal alloc] initForExistingFabric:fabricTable
                                                                             fabricIndex:fabric->GetFabricIndex()
                                                                                keystore:_keystore
                                                                                  params:startupParams];
    });

    if (params == nil) {
        [self controllerShuttingDown:controller];
        return nil;
    }

    BOOL ok = [controller startup:params];
    if (ok == NO) {
        return nil;
    }

    return [self maybeInitializeOTAProvider:controller];
}

- (MTRDeviceController * _Nullable)startControllerOnNewFabric:(MTRDeviceControllerStartupParams *)startupParams
{
    if (![self isRunning]) {
        MTR_LOG_ERROR("Trying to start controller while Matter controller factory is not running");
        return nil;
    }

    if (startupParams.vendorID == nil) {
        MTR_LOG_ERROR("Must provide vendor id when starting controller on new fabric");
        return nil;
    }

    if (startupParams.intermediateCertificate != nil && startupParams.rootCertificate == nil) {
        MTR_LOG_ERROR("Must provide a root certificate when using an intermediate certificate");
        return nil;
    }

    // Create the controller, so we start the event loop, since we plan to do
    // our fabric table operations there.
    auto * controller = [self createController];
    if (controller == nil) {
        return nil;
    }

    __block MTRDeviceControllerStartupParamsInternal * params = nil;
    // We want the block to end up with just a pointer to the fabric table,
    // since we know our on-stack instance will outlive the block.
    FabricTable fabricTableInstance;
    FabricTable * fabricTable = &fabricTableInstance;
    dispatch_sync(_chipWorkQueue, ^{
        const FabricInfo * fabric = nullptr;
        BOOL ok = [self findMatchingFabric:*fabricTable params:startupParams fabric:&fabric];
        if (!ok) {
            MTR_LOG_ERROR("Can't start on new fabric: fabric matching failed");
            return;
        }

        if (fabric != nullptr) {
            MTR_LOG_ERROR("Can't start on new fabric that matches existing fabric");
            return;
        }

        params = [[MTRDeviceControllerStartupParamsInternal alloc] initForNewFabric:fabricTable
                                                                           keystore:_keystore
                                                                             params:startupParams];
    });

    if (params == nil) {
        [self controllerShuttingDown:controller];
        return nil;
    }

    BOOL ok = [controller startup:params];
    if (ok == NO) {
        return nil;
    }

    return [self maybeInitializeOTAProvider:controller];
}

- (MTRDeviceController * _Nullable)createController
{
    MTRDeviceController * controller = [[MTRDeviceController alloc] initWithFactory:self queue:_chipWorkQueue];
    if (controller == nil) {
        MTR_LOG_ERROR("Failed to init controller");
        return nil;
    }

    if ([_controllers count] == 0) {
        // Bringing up the first controller.  Start the event loop now.  If we
        // fail to bring it up, its cleanup will stop the event loop again.
        chip::DeviceLayer::PlatformMgrImpl().StartEventLoopTask();
    }

    // Add the controller to _controllers now, so if we fail partway through its
    // startup we will still do the right cleanups.
    [_controllers addObject:controller];

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
    CHIP_ERROR err = fabricTable.Init(
        { .storage = _persistentStorageDelegateBridge, .operationalKeystore = _keystore, .opCertStore = _opCertStore });
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
    VerifyOrReturnValue(_otaProviderDelegateBridge != nil, controller);
    VerifyOrReturnValue([_controllers count] == 1, controller);

    auto systemState = _controllerFactory->GetSystemState();
    CHIP_ERROR err = _otaProviderDelegateBridge->Init(systemState->SystemLayer(), systemState->ExchangeMgr());
    if (CHIP_NO_ERROR != err) {
        MTR_LOG_ERROR("Failed to init provider delegate bridge: %" CHIP_ERROR_FORMAT, err.Format());
        [controller shutdown];
        return nil;
    }

    return controller;
}

@end

@implementation MTRControllerFactory (InternalMethods)

- (void)controllerShuttingDown:(MTRDeviceController *)controller
{
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

    [_controllers removeObject:controller];

    if ([_controllers count] == 0) {
        if (_otaProviderDelegateBridge) {
            _otaProviderDelegateBridge->Shutdown();
        }

        // That was our last controller.  Stop the event loop before it
        // shuts down, because shutdown of the last controller will tear
        // down most of the world.
        DeviceLayer::PlatformMgrImpl().StopEventLoopTask();

        [controller shutDownCppController];
    } else {
        // Do the controller shutdown on the Matter work queue.
        dispatch_sync(_chipWorkQueue, ^{
            [controller shutDownCppController];
        });
    }

    [controller deinitFromFactory];
}

- (nullable MTRDeviceController *)runningControllerForFabricIndex:(chip::FabricIndex)fabricIndex
{
    for (MTRDeviceController * existing in _controllers) {
        if ([existing fabricIndex] == fabricIndex) {
            return existing;
        }
    }

    return nil;
}

- (MTRPersistentStorageDelegateBridge *)storageDelegateBridge
{
    return _persistentStorageDelegateBridge;
}

- (Credentials::GroupDataProvider *)groupData
{
    return _groupDataProvider;
}

@end

@implementation MTRControllerFactoryParams

- (instancetype)initWithStorage:(id<MTRStorage>)storage
{
    if (!(self = [super init])) {
        return nil;
    }

    _storage = storage;
    _otaProviderDelegate = nil;
    _paaCerts = nil;
    _cdCerts = nil;
    _port = nil;
    _startServer = NO;

    return self;
}

@end

@implementation MTRControllerFactoryParams (Deprecated)

- (id<MTRPersistentStorageDelegate>)storageDelegate
{
    // Cast is safe, because MTRPersistentStorageDelegate doesn't add
    // any selectors to MTRStorage, so anything implementing
    // MTRStorage also implements MTRPersistentStorageDelegate.
    return static_cast<id<MTRPersistentStorageDelegate>>(self.storage);
}

@end
