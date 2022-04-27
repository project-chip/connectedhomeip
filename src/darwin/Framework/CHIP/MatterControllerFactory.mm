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

#import "MatterControllerFactory.h"
#import "MatterControllerFactory_Internal.h"

#import "CHIPAttestationTrustStoreBridge.h"
#import "CHIPControllerAccessControl.h"
#import "CHIPDeviceController.h"
#import "CHIPDeviceController_Internal.h"
#import "CHIPLogging.h"
#import "CHIPP256KeypairBridge.h"
#import "CHIPPersistentStorageDelegateBridge.h"

#include <controller/CHIPDeviceControllerFactory.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::Controller;

static NSString * const kErrorMemoryInit = @"Init Memory failure";
static NSString * const kErrorPersistentStorageInit = @"Init failure while creating a persistent storage delegate";
static NSString * const kErrorAttestationTrustStoreInit = @"Init failure while creating the attestation trust store";
static NSString * const kInfoFactoryShutdown = @"Shutting down the Matter controller factory";
static NSString * const kErrorGroupProviderInit = @"Init failure while initializing group data provider";
static NSString * const kErrorControllersInit = @"Init controllers array failure";
static NSString * const kErrorControllerFactoryInit = @"Init failure while initializing controller factory";

@interface MatterControllerFactory ()

@property (atomic, readonly) dispatch_queue_t chipWorkQueue;
@property (readonly) DeviceControllerFactory * controllerFactory;
@property (readonly) CHIPPersistentStorageDelegateBridge * persistentStorageDelegateBridge;
@property (readonly) CHIPAttestationTrustStoreBridge * attestationTrustStoreBridge;
// We use TestPersistentStorageDelegate just to get an in-memory store to back
// our group data provider impl.  We initialize this store correctly on every
// controller startup, so don't need to actually persist it.
@property (readonly) chip::TestPersistentStorageDelegate * groupStorageDelegate;
@property (readonly) chip::Credentials::GroupDataProviderImpl * groupDataProvider;
@property (readonly) NSMutableArray<CHIPDeviceController *> * controllers;

@end

@implementation MatterControllerFactory

+ (instancetype)sharedInstance
{
    static MatterControllerFactory * factory = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // initialize the factory.
        factory = [[MatterControllerFactory alloc] init];
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
    CHIP_ERROR errorCode = Platform::MemoryInit();
    if ([self checkForInitError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorMemoryInit]) {
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
    errorCode = _groupDataProvider->Init();
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

    CHIP_LOG_ERROR("Error: %@", logMsg);

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

    Platform::MemoryShutdown();
}

- (void)cleanupStartupObjects
{
    if (_attestationTrustStoreBridge) {
        delete _attestationTrustStoreBridge;
        _attestationTrustStoreBridge = nullptr;
    }

    if (_persistentStorageDelegateBridge) {
        delete _persistentStorageDelegateBridge;
        _persistentStorageDelegateBridge = nullptr;
    }
}

- (BOOL)startup:(MatterControllerFactoryParams *)startupParams
{
    if ([self isRunning]) {
        CHIP_LOG_DEBUG("Ignoring duplicate call to startup, Matter controller factory already started...");
        return YES;
    }

    DeviceLayer::PlatformMgrImpl().StartEventLoopTask();

    dispatch_sync(_chipWorkQueue, ^{
        if ([self isRunning]) {
            return;
        }

        [CHIPControllerAccessControl init];

        _persistentStorageDelegateBridge = new CHIPPersistentStorageDelegateBridge(startupParams.storageDelegate);
        if (_persistentStorageDelegateBridge == nil) {
            CHIP_LOG_ERROR("Error: %@", kErrorPersistentStorageInit);
            return;
        }

        // Initialize device attestation verifier
        if (startupParams.paaCerts) {
            _attestationTrustStoreBridge = new CHIPAttestationTrustStoreBridge(startupParams.paaCerts);
            if (_attestationTrustStoreBridge == nullptr) {
                CHIP_LOG_ERROR("Error: %@", kErrorAttestationTrustStoreInit);
                return;
            }
            chip::Credentials::SetDeviceAttestationVerifier(chip::Credentials::GetDefaultDACVerifier(_attestationTrustStoreBridge));
        } else {
            // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
            const chip::Credentials::AttestationTrustStore * testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
            chip::Credentials::SetDeviceAttestationVerifier(chip::Credentials::GetDefaultDACVerifier(testingRootStore));
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
        CHIP_ERROR errorCode = _controllerFactory->Init(params);
        if (errorCode != CHIP_NO_ERROR) {
            CHIP_LOG_ERROR("Error: %@", kErrorControllerFactoryInit);
            return;
        }

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

    CHIP_LOG_DEBUG("%@", kInfoFactoryShutdown);
    _controllerFactory->Shutdown();

    [self cleanupStartupObjects];

    // NOTE: we do not call cleanupInitObjects because we can be restarted, and
    // that does not re-create the objects that we create inside init.
    // Maybe we should be creating them in startup?

    _isRunning = NO;
}

- (CHIPDeviceController * _Nullable)startControllerOnExistingFabric:(CHIPDeviceControllerStartupParams *)startupParams
{
    if (![self isRunning]) {
        CHIP_LOG_ERROR("Trying to start controller while Matter controller factory is not running");
        return nil;
    }

    // Create the controller, so we start the event loop, since we plan to do our fabric table operations there.
    auto * controller = [self createController];
    if (controller == nil) {
        return nil;
    }

    __block BOOL okToStart = NO;
    dispatch_sync(_chipWorkQueue, ^{
        FabricTable fabricTable;
        if (fabricTable.Init(_persistentStorageDelegateBridge) != CHIP_NO_ERROR) {
            CHIP_LOG_ERROR("Can't start on existing fabric: fabric table init failed");
            return;
        }

        CHIPP256KeypairBridge keypairBridge;
        if (keypairBridge.Init(startupParams.rootCAKeypair) != CHIP_NO_ERROR) {
            return;
        }
        auto * fabric
            = fabricTable.FindFabric(Credentials::P256PublicKeySpan(keypairBridge.Pubkey().ConstBytes()), startupParams.fabricId);

        if (!fabric) {
            CHIP_LOG_ERROR("Can't start on existing fabric: fabric not found");
            return;
        }

        for (CHIPDeviceController * existing in _controllers) {
            BOOL isRunning = YES; // assume the worst
            if ([existing isRunningOnFabric:fabric isRunning:&isRunning] != CHIP_NO_ERROR) {
                CHIP_LOG_ERROR("Can't tell what fabric a controller is running on.  Not safe to start.");
                return;
            }

            if (isRunning) {
                CHIP_LOG_ERROR("Can't start on existing fabric: another controller is running on it");
                return;
            }
        }

        okToStart = YES;
    });

    if (startupParams.rootCAKeypair == nil) {
        // TODO: This block needs for nil keypair needs to go away.
        //
        // We don't have to a public key to identify this fabric, so
        // okToStart got set to false, just assume that it's OK to start
        // the controller.  But only if we have no running controllers
        // already, so we don't stomp on other controllers.
        //
        // Our controller is already in _controllers.
        if ([_controllers count] == 1) {
            okToStart = YES;
        } else {
            CHIP_LOG_ERROR("No root key, an a controller is already running.  Blocking second controller");
        }
    }

    if (okToStart == NO) {
        [self controllerShuttingDown:controller];
        return nil;
    }

    // TODO: Pass in the existing NOC and whatnot, as needed.
    BOOL ok = [controller startup:startupParams];
    if (ok == NO) {
        return nil;
    }

    return controller;
}

- (CHIPDeviceController * _Nullable)startControllerOnNewFabric:(CHIPDeviceControllerStartupParams *)startupParams
{
    if (![self isRunning]) {
        CHIP_LOG_ERROR("Trying to start controller while Matter controller factory is not running");
        return nil;
    }

    if (startupParams.rootCAKeypair == nil) {
        // TODO: This block needs for nil keypair needs to go away.
        //
        // Disallow starting on a "new fabric" if there is no indication
        // of what the new fabric should be.
        return nil;
    }

    // Create the controller, so we start the event loop, since we plan to do our fabric table operations there.
    auto * controller = [self createController];
    if (controller == nil) {
        return nil;
    }

    __block BOOL okToStart = NO;
    dispatch_sync(_chipWorkQueue, ^{
        FabricTable fabricTable;
        if (fabricTable.Init(_persistentStorageDelegateBridge) != CHIP_NO_ERROR) {
            CHIP_LOG_ERROR("Can't start on new fabric: storage can't read fabric table");
            return;
        }

        CHIPP256KeypairBridge keypairBridge;
        if (keypairBridge.Init(startupParams.rootCAKeypair) != CHIP_NO_ERROR) {
            CHIP_LOG_ERROR("Can't start controller without a usable public key");
            return;
        }
        auto * fabric
            = fabricTable.FindFabric(Credentials::P256PublicKeySpan(keypairBridge.Pubkey().ConstBytes()), startupParams.fabricId);
        if (fabric) {
            CHIP_LOG_ERROR("Can't start on new fabric that matches existing fabric");
            return;
        }

        okToStart = YES;
    });

    if (okToStart == NO) {
        [self controllerShuttingDown:controller];
        return nil;
    }

    BOOL ok = [controller startup:startupParams];
    if (ok == NO) {
        return nil;
    }

    return controller;
}

- (CHIPDeviceController * _Nullable)createController
{
    CHIPDeviceController * controller = [[CHIPDeviceController alloc] initWithFactory:self queue:_chipWorkQueue];
    if (controller == nil) {
        CHIP_LOG_ERROR("Failed to init controller");
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

@end

@implementation MatterControllerFactory (InternalMethods)

- (void)controllerShuttingDown:(CHIPDeviceController *)controller
{
    if (![_controllers containsObject:controller]) {
        CHIP_LOG_ERROR("Controller we don't know about shutting down");
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
        // That was our last controller.  Stop the event loop before it
        // shuts down, because shutdown of the last controller will tear
        // down most of the world.
        DeviceLayer::PlatformMgrImpl().StopEventLoopTask();
    }
}

- (CHIPPersistentStorageDelegateBridge *)storageDelegateBridge
{
    return _persistentStorageDelegateBridge;
}

- (Credentials::GroupDataProvider *)groupData
{
    return _groupDataProvider;
}

@end

@implementation MatterControllerFactoryParams

- (instancetype)initWithStorage:(id<CHIPPersistentStorageDelegate>)storageDelegate
{
    if (!(self = [super init])) {
        return nil;
    }

    _storageDelegate = storageDelegate;
    _paaCerts = nil;
    _port = nil;
    _startServer = NO;

    return self;
}

@end
