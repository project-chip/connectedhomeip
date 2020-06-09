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

extern "C" {
#include "chip-zcl/chip-zcl-buffer.h"
#include "chip-zcl/chip-zcl.h"
#include "gen/gen-command-id.h"
} // extern "C"

#import "CHIPDeviceController.h"
#import "CHIPError.h"
#import "CHIPLogging.h"

#include <controller/CHIPDeviceController.h>
#include <inet/IPAddress.h>
#include <system/SystemPacketBuffer.h>

static const char * const CHIP_WORK_QUEUE = "com.zigbee.chip.work";
static const char * const CHIP_SELECT_QUEUE = "com.zigbee.chip.select";

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

// queue used for all interactions with the cpp chip controller and for all chip internal events
// try to run small jobs in this queue
@property (atomic, readonly) dispatch_queue_t chipWorkQueue;
// queue used to call select on the system and inet layer fds., remove this with NW Framework.
// primarily used to not block the work queue
@property (atomic, readonly) dispatch_queue_t chipSelectQueue;
// queue used to signal callbacks to the application
@property (readwrite) dispatch_queue_t appCallbackQueue;
@property (readwrite) ControllerOnMessageBlock onMessageHandler;
@property (readwrite) ControllerOnErrorBlock onErrorHandler;
@property (readonly) chip::DeviceController::ChipDeviceController * cppController;

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
        _chipWorkQueue = dispatch_queue_create(CHIP_WORK_QUEUE, DISPATCH_QUEUE_SERIAL);
        if (!_chipWorkQueue) {
            return nil;
        }

        _chipSelectQueue = dispatch_queue_create(CHIP_SELECT_QUEUE, DISPATCH_QUEUE_SERIAL);
        if (!_chipSelectQueue) {
            return nil;
        }

        _cppController = new chip::DeviceController::ChipDeviceController();
        if (!_cppController) {
            CHIP_LOG_ERROR("Error: couldn't create c++ controller");
            return nil;
        }

        if (CHIP_NO_ERROR != _cppController->Init()) {
            CHIP_LOG_ERROR("Error: couldn't initialize c++ controller");
            delete _cppController;
            _cppController = NULL;
            return nil;
        }
    }

    return self;
}

static void onMessageReceived(chip::DeviceController::ChipDeviceController * deviceController, void * appReqState,
    chip::System::PacketBuffer * buffer, const chip::IPPacketInfo * packet_info)
{
    CHIPDeviceController * controller = (__bridge CHIPDeviceController *) appReqState;

    char src_addr[INET_ADDRSTRLEN];
    size_t data_len = buffer->DataLength();

    packet_info->SrcAddress.ToString(src_addr, sizeof(src_addr));
    NSString * ipAddress = [[NSString alloc] initWithUTF8String:src_addr];

    // convert to NSData and pass back to the application
    NSMutableData * dataBuffer = [[NSMutableData alloc] initWithBytes:buffer->Start() length:data_len];
    buffer = buffer->Next();

    while (buffer != NULL) {
        data_len = buffer->DataLength();
        [dataBuffer appendBytes:buffer->Start() length:data_len];
        buffer = buffer->Next();
    }

    [controller _dispatchAsyncMessageBlock:dataBuffer ipAddress:ipAddress port:packet_info->SrcPort];

    // ignore unused variable
    (void) deviceController;
    chip::System::PacketBuffer::Free(buffer);
}

static void onInternalError(chip::DeviceController::ChipDeviceController * deviceController, void * appReqState, CHIP_ERROR error,
    const chip::IPPacketInfo * pi)
{
    CHIPDeviceController * controller = (__bridge CHIPDeviceController *) appReqState;
    [controller _dispatchAsyncErrorBlock:[CHIPError errorForCHIPErrorCode:error]];
}

- (void)_dispatchAsyncErrorBlock:(NSError *)error
{
    CHIP_LOG_METHOD_ENTRY();
    // to avoid retaining "self"
    ControllerOnErrorBlock onErrorHandler = self.onErrorHandler;

    dispatch_async(_appCallbackQueue, ^() {
        onErrorHandler(error);
    });
}

- (void)_dispatchAsyncMessageBlock:(NSData *)data ipAddress:(NSString *)ipAddress port:(UInt16)port
{
    CHIP_LOG_METHOD_ENTRY();
    // to avoid retaining "self"
    ControllerOnMessageBlock onMessageHandler = self.onMessageHandler;

    dispatch_async(_appCallbackQueue, ^() {
        onMessageHandler(data, ipAddress, port);
    });
}

- (BOOL)connect:(NSString *)ipAddress error:(NSError * __autoreleasing *)error
{
    __block CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO maybe refactor
    // the work queue is being used for atomic access to chip's cpp controller
    // this could be done async but the error we care about is sync. However, I think this could be restructured such that
    // the request is fired async and that Block can then return an error to the caller. This function would then never error out.
    // the only drawback is that it complicates the api where the user must handle async errors on every function
    dispatch_sync(self.chipWorkQueue, ^() {
        chip::Inet::IPAddress addr;
        chip::Inet::IPAddress::FromString([ipAddress UTF8String], addr);
        err = self.cppController->ConnectDevice(0, addr, NULL, onMessageReceived, onInternalError, CHIP_PORT);
    });

    if (err != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error(%d): %@, connect failed", err, [CHIPError errorForCHIPErrorCode:err]);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:err];
        }
        return NO;
    }

    // Start the IO pump
    [self _serviceEvents];

    return YES;
}

- (AddressInfo *)getAddressInfo
{
    __block CHIP_ERROR err = CHIP_NO_ERROR;
    __block chip::IPAddress ipAddr;
    __block uint16_t port;

    dispatch_sync(self.chipWorkQueue, ^() {
        err = self.cppController->GetDeviceAddress(&ipAddr, &port);
    });

    if (err != CHIP_NO_ERROR) {
        return nil;
    }

    // ignore the unused port
    (void)port;
    // A buffer big enough to hold ipv4 and ipv6 addresses
    char ipAddrStr[64];
    ipAddr.ToString(ipAddrStr, sizeof(ipAddrStr));
    NSString * ipAddress = [NSString stringWithUTF8String:ipAddrStr];
    return [[AddressInfo alloc] initWithIP:ipAddress];
}

- (BOOL)sendMessage:(NSData *)message error:(NSError * __autoreleasing *)error
{
    __block CHIP_ERROR err = CHIP_NO_ERROR;

    dispatch_sync(self.chipWorkQueue, ^() {
        size_t messageLen = [message length];
        const void * messageChars = [message bytes];

        chip::System::PacketBuffer * buffer = chip::System::PacketBuffer::NewWithAvailableSize(messageLen);
        buffer->SetDataLength(messageLen);

        memcpy(buffer->Start(), messageChars, messageLen);
        err = self.cppController->SendMessage((__bridge void *) self, buffer);
    });

    if (err != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error(%d): %@, send failed", err, [CHIPError errorForCHIPErrorCode:err]);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:err];
        }
        return NO;
    }
    return YES;
}

- (BOOL)sendCHIPCommand:(ChipZclClusterId_t)cluster command:(ChipZclCommandId_t)command
{
    __block CHIP_ERROR err = CHIP_NO_ERROR;
    dispatch_sync(self.chipWorkQueue, ^() {
        // FIXME: This needs a better buffersizing setup!
        static const size_t bufferSize = 1024;
        chip::System::PacketBuffer * buffer = chip::System::PacketBuffer::NewWithAvailableSize(bufferSize);

        ChipZclBuffer_t * zcl_buffer = (ChipZclBuffer_t *) buffer;
        ChipZclCommandContext_t ctx = {
            1, // endpointId
            cluster, // clusterId
            true, // clusterSpecific
            false, // mfgSpecific
            0, // mfgCode
            command, // commandId
            ZCL_DIRECTION_CLIENT_TO_SERVER, // direction
            0, // payloadStartIndex
            nullptr, // request
            nullptr // response
        };
        chipZclEncodeZclHeader(zcl_buffer, &ctx);

        const size_t data_len = chipZclBufferDataLength(zcl_buffer);

        buffer->SetDataLength(data_len);

        err = self.cppController->SendMessage((__bridge void *) self, buffer);
    });
    if (err != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error(%d): %@, send failed", err, [CHIPError errorForCHIPErrorCode:err]);
        return NO;
    }
    return YES;
}

- (BOOL)disconnect:(NSError * __autoreleasing *)error
{
    __block CHIP_ERROR err = CHIP_NO_ERROR;

    dispatch_sync(self.chipWorkQueue, ^() {
        err = self.cppController->DisconnectDevice();
    });

    if (err != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error(%d): %@, disconnect failed", err, [CHIPError errorForCHIPErrorCode:err]);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:err];
        }
        return NO;
    }
    return YES;
}

- (BOOL)isConnected
{
    __block bool isConnected = false;

    // the work queue is being used for atomic access to chip's cpp controller
    dispatch_sync(self.chipWorkQueue, ^() {
        isConnected = self.cppController->IsConnected();
    });

    return isConnected ? YES : NO;
}

// TODO kill this with fire (NW might implicitly replace this?)
- (void)_serviceEvents
{
    dispatch_async(self.chipWorkQueue, ^() {
        __block fd_set readFDs, writeFDs, exceptFDs;
        struct timeval aSleepTime;
        int numFDs = 0;
        aSleepTime.tv_sec = 5;

        FD_ZERO(&readFDs);
        FD_ZERO(&writeFDs);
        FD_ZERO(&exceptFDs);

        chip::System::Layer * systemLayer = NULL;
        chip::Inet::InetLayer * inetLayer = NULL;
        // ask for the system and inet layers
        self.cppController->GetLayers(&systemLayer, &inetLayer);

        if (systemLayer != NULL && systemLayer->State() == chip::System::kLayerState_Initialized)
            systemLayer->PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);

        if (inetLayer != NULL && inetLayer->State == chip::Inet::InetLayer::kState_Initialized)
            inetLayer->PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);

        dispatch_async(self.chipSelectQueue, ^() {
            int selectRes = select(numFDs, &readFDs, &writeFDs, &exceptFDs, const_cast<struct timeval *>(&aSleepTime));

            dispatch_async(self.chipWorkQueue, ^() {
                if (!self.cppController->IsConnected()) {
                    // cancel the loop, it'll restart the next time a connection is established
                    return;
                }
                chip::System::Layer * systemLayer = NULL;
                chip::Inet::InetLayer * inetLayer = NULL;
                self.cppController->GetLayers(&systemLayer, &inetLayer);

                if (systemLayer != NULL && systemLayer->State() == chip::System::kLayerState_Initialized) {
                    systemLayer->HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
                }

                if (inetLayer != NULL && inetLayer->State == chip::Inet::InetLayer::kState_Initialized) {
                    inetLayer->HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
                }

                [self _serviceEvents];
            });
        });
    });
}

- (void)registerCallbacks:appCallbackQueue onMessage:(ControllerOnMessageBlock)onMessage onError:(ControllerOnErrorBlock)onError
{
    self.appCallbackQueue = appCallbackQueue;
    self.onMessageHandler = onMessage;
    self.onErrorHandler = onError;
}

@end

extern "C" {
// We have to have this empty callback, because the ZCL code links against it.
void chipZclPostAttributeChangeCallback(uint8_t endpoint, ChipZclClusterId clusterId, ChipZclAttributeId attributeId, uint8_t mask,
    uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
}
} // extern "C"
