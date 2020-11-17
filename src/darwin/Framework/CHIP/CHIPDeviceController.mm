/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#import <Foundation/Foundation.h>

#include <app/chip-zcl-zpro-codec.h>

#import "CHIPDeviceController.h"
#import "CHIPDevicePairingDelegateBridge.h"
#import "CHIPDeviceStatusDelegateBridge.h"
#import "CHIPDevice_Internal.h"
#import "CHIPError.h"
#import "CHIPLogging.h"
#import "CHIPPersistentStorageDelegateBridge.h"

#include <controller/CHIPDeviceController.h>
#include <inet/IPAddress.h>
#include <support/CHIPMem.h>
#include <system/SystemPacketBuffer.h>

static const char * const CHIP_SELECT_QUEUE = "com.zigbee.chip.select";

constexpr chip::NodeId kLocalDeviceId = chip::kTestControllerNodeId;

@implementation AddressInfo
- (instancetype)initWithIP:(NSString *)ip
{
    if (self = [super init]) {
        _ip = ip;
    }
    return self;
}
@end

@interface CHIPDeviceController ()

@property (nonatomic, readonly, strong, nonnull) NSRecursiveLock * lock;

// queue used to call select on the system and inet layer fds., remove this with NW Framework.
// primarily used to not block the work queue
@property (atomic, readonly) dispatch_queue_t chipSelectQueue;
/**
 *  The Controller delegate.
 *  Note: Getter is not thread safe.
 */
@property (readonly, weak, nonatomic) id<CHIPDeviceControllerDelegate> delegate;

/**
 * The delegate queue where delegate callbacks will run
 */
@property (readonly, nonatomic) dispatch_queue_t delegateQueue;
@property (readonly) chip::Controller::DeviceCommissioner * cppController;
@property (readonly) CHIPDevicePairingDelegateBridge * pairingDelegateBridge;
@property (readonly) CHIPDeviceStatusDelegateBridge * deviceStatusDelegateBridge;
@property (readonly) CHIPPersistentStorageDelegateBridge * persistentStorageDelegateBridge;

@end

@implementation CHIPDeviceController

+ (CHIPDeviceController *)sharedController
{
    static CHIPDeviceController * controller = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // initialize the device controller
        controller = [[CHIPDeviceController alloc] init];
    });
    return controller;
}

- (instancetype)init
{
    if (self = [super init]) {

        _lock = [[NSRecursiveLock alloc] init];

        _chipSelectQueue = dispatch_queue_create(CHIP_SELECT_QUEUE, DISPATCH_QUEUE_SERIAL);
        if (!_chipSelectQueue) {
            return nil;
        }

        if (CHIP_NO_ERROR != chip::Platform::MemoryInit()) {
            CHIP_LOG_ERROR("Error: Failed in memory init");
            return nil;
        }

        _deviceStatusDelegateBridge = new CHIPDeviceStatusDelegateBridge();
        if (!_deviceStatusDelegateBridge) {
            CHIP_LOG_ERROR("Error: couldn't create device status delegate bridge");
            return nil;
        }
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.zigbee.chip.controller.callback", DISPATCH_QUEUE_SERIAL);
        _deviceStatusDelegateBridge->setDelegate(self, callbackQueue);

        _cppController = new chip::Controller::DeviceCommissioner();
        if (!_cppController) {
            CHIP_LOG_ERROR("Error: couldn't create c++ controller");
            return nil;
        }

        _pairingDelegateBridge = new CHIPDevicePairingDelegateBridge();
        if (!_pairingDelegateBridge) {
            CHIP_LOG_ERROR("Error: couldn't create pairing delegate");
            delete _cppController;
            _cppController = NULL;
            return nil;
        }

        _persistentStorageDelegateBridge = new CHIPPersistentStorageDelegateBridge();
        if (!_persistentStorageDelegateBridge) {
            CHIP_LOG_ERROR("Error: couldn't create persistent storage delegate");
            delete _cppController;
            _cppController = NULL;
            delete _pairingDelegateBridge;
            _pairingDelegateBridge = NULL;
            return nil;
        }

        if (CHIP_NO_ERROR != _cppController->Init(kLocalDeviceId, _persistentStorageDelegateBridge, _pairingDelegateBridge)) {
            CHIP_LOG_ERROR("Error: couldn't initialize c++ controller");
            delete _cppController;
            _cppController = NULL;
            delete _pairingDelegateBridge;
            _pairingDelegateBridge = NULL;
            delete _persistentStorageDelegateBridge;
            _persistentStorageDelegateBridge = NULL;
            return nil;
        }

        // Start the IO pump
        dispatch_async(_chipSelectQueue, ^() {
            self.cppController->ServiceEvents();
        });
    }
    return self;
}

// MARK: CHIPDeviceStatusDelegate
- (void)onMessageReceived:(NSData *)message
{
    CHIP_LOG_METHOD_ENTRY();

    id<CHIPDeviceControllerDelegate> strongDelegate = [self delegate];
    if (strongDelegate && [self delegateQueue]) {
        dispatch_async(self.delegateQueue, ^{
            [strongDelegate deviceControllerOnMessage:message];
        });
    }
}

- (BOOL)pairDevice:(uint64_t)deviceID
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    [self.lock lock];

    chip::RendezvousParameters params = chip::RendezvousParameters().SetSetupPINCode(setupPINCode).SetDiscriminator(discriminator);
    err = self.cppController->PairDevice(deviceID, params);
    [self.lock unlock];

    if (err != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error(%d): %@, failed in pairing the device", err, [CHIPError errorForCHIPErrorCode:err]);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:err];
        }
        return NO;
    }

    return YES;
}

- (BOOL)unpairDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    [self.lock lock];

    err = self.cppController->UnpairDevice(deviceID);
    [self.lock unlock];

    if (err != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error(%d): %@, failed in unpairing the device", err, [CHIPError errorForCHIPErrorCode:err]);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:err];
        }
        return NO;
    }

    return YES;
}

- (BOOL)stopDevicePairing:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    [self.lock lock];
    err = self.cppController->StopPairing(deviceID);
    [self.lock unlock];

    if (err != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error(%d): %@, failed in stopping the pairing process", err, [CHIPError errorForCHIPErrorCode:err]);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:err];
        }
        return NO;
    }

    return YES;
}

- (CHIPDevice *)getPairedDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Controller::Device * cppDevice = nil;

    [self.lock lock];
    err = self.cppController->GetDevice(deviceID, &cppDevice);
    [self.lock unlock];

    if (err != CHIP_NO_ERROR || !cppDevice) {
        CHIP_LOG_ERROR("Error(%d): %@, failed in getting device instance", err, [CHIPError errorForCHIPErrorCode:err]);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:err];
        }
        return nil;
    }

    cppDevice->SetDelegate(_deviceStatusDelegateBridge);

    return [[CHIPDevice alloc] initWithDevice:cppDevice];
}

- (BOOL)disconnect:(NSError * __autoreleasing *)error
{
    return YES;
}

- (void)setDelegate:(id<CHIPDeviceControllerDelegate>)delegate queue:(dispatch_queue_t)queue
{
    [self.lock lock];
    if (delegate && queue) {
        self->_delegate = delegate;
        self->_delegateQueue = queue;
    } else {
        self->_delegate = nil;
        self->_delegateQueue = NULL;
    }
    [self.lock unlock];
}

- (void)setPairingDelegate:(id<CHIPDevicePairingDelegate>)delegate queue:(dispatch_queue_t)queue
{
    [self.lock lock];
    _pairingDelegateBridge->setDelegate(delegate, queue);
    [self.lock unlock];
}

- (void)setPersistentStorageDelegate:(id<CHIPPersistentStorageDelegate>)delegate queue:(dispatch_queue_t)queue
{
    [self.lock lock];
    _persistentStorageDelegateBridge->setFrameworkDelegate(delegate, queue);
    [self.lock unlock];
}

@end
