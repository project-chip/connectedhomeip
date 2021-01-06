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
#include <controller/CHIPDevice.h>

#import "CHIPDevice.h"
#import "CHIPDevice_Internal.h"
#import "CHIPError.h"
#import "CHIPLogging.h"

#include <system/SystemPacketBuffer.h>

@interface CHIPDevice ()

@property (nonatomic, readonly, strong, nonnull) NSRecursiveLock * lock;
@property (readonly) chip::Controller::Device * cppDevice;

@end

@implementation CHIPDevice

- (instancetype)init
{
    if (self = [super init]) {
        _lock = [[NSRecursiveLock alloc] init];
    }
    return self;
}

- (instancetype)initWithDevice:(chip::Controller::Device *)device
{
    if (self = [super init]) {
        _cppDevice = device;
    }
    return self;
}

- (chip::Controller::Device *)internalDevice
{
    return _cppDevice;
}

- (BOOL)sendMessage:(NSData *)message error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    [self.lock lock];
    size_t messageLen = [message length];
    const void * messageChars = [message bytes];

    chip::System::PacketBufferHandle buffer = chip::System::PacketBuffer::NewWithAvailableSize(messageLen);
    if (buffer.IsNull()) {
        err = CHIP_ERROR_NO_MEMORY;
    } else {
        buffer->SetDataLength(messageLen);

        if (buffer->DataLength() < messageLen) {
            err = CHIP_ERROR_NO_MEMORY;
        } else {
            memcpy(buffer->Start(), messageChars, messageLen);
            err = self.cppDevice->SendMessage(std::move(buffer));
        }
    }
    [self.lock unlock];

    if (err != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error(%d): %@, send failed", err, [CHIPError errorForCHIPErrorCode:err]);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:err];
        }
        return NO;
    }

    return YES;
}

- (BOOL)sendCHIPCommand:(chip::System::PacketBufferHandle (^)())encodeCommandBlock
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    [self.lock lock];
    chip::System::PacketBufferHandle buffer = encodeCommandBlock();
    if (buffer.IsNull()) {
        err = CHIP_ERROR_NO_MEMORY;
    } else {
        err = self.cppDevice->SendMessage(std::move(buffer));
    }
    [self.lock unlock];
    if (err != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error(%d): %@, send failed", err, [CHIPError errorForCHIPErrorCode:err]);
        return NO;
    }

    return YES;
}

- (BOOL)sendIdentifyCommandWithDuration:(NSTimeInterval)duration
{
    if (duration > UINT16_MAX) {
        duration = UINT16_MAX;
    }

    return [self sendCHIPCommand:^chip::System::PacketBufferHandle() {
        // Hardcode endpoint to 1 for now
        return encodeIdentifyClusterIdentifyCommand(1, duration);
    }];
}

- (BOOL)disconnect:(NSError * __autoreleasing *)error
{
    return YES;
}

- (BOOL)isActive
{
    bool isActive = false;

    [self.lock lock];
    isActive = self.cppDevice->IsActive();
    [self.lock unlock];

    return isActive ? YES : NO;
}

+ (BOOL)isDataModelCommand:(NSData * _Nonnull)message
{
    if (message.length == 0) {
        return NO;
    }

    UInt8 * bytes = (UInt8 *) message.bytes;
    return bytes[0] < 0x04 ? YES : NO;
}

+ (NSString *)commandToString:(NSData * _Nonnull)response
{
    if ([CHIPDevice isDataModelCommand:response] == NO) {
        return @"Response is not a CHIP command";
    }

    uint8_t * bytes = (uint8_t *) response.bytes;

    EmberApsFrame frame;
    if (extractApsFrame(bytes, (uint32_t) response.length, &frame) == 0) {
        return @"Response is not an APS frame";
    }

    uint8_t * message;
    uint16_t messageLen = extractMessage(bytes, response.length, &message);
    if (messageLen != 5) {
        // Not a Default Response command for sure.
        return @"Unexpected response length";
    }

    if (message[0] != 8) {
        // Unexpected control byte
        return [NSString stringWithFormat:@"Control byte value '0x%02x' is not expected", message[0]];
    }

    // message[1] is the sequence counter; just ignore it for now.

    if (message[2] != 0x0b) {
        // Not a Default Response command id
        return [NSString stringWithFormat:@"Command id '0x%02x' is not the Default Response command id (0x0b)", message[2]];
    }

    if (frame.clusterId != 0x06) {
        // Not On/Off cluster
        return [NSString stringWithFormat:@"Cluster id '0x%02x' is not the on/off cluster id (0x06)", frame.clusterId];
    }

    NSString * command;
    if (message[3] == 0) {
        command = @"off";
    } else if (message[3] == 1) {
        command = @"on";
    } else if (message[3] == 2) {
        command = @"toggle";
    } else {
        return [NSString stringWithFormat:@"Command '0x%02x' is unknown", message[3]];
    }

    NSString * status;
    if (message[4] == 0) {
        status = @"succeeded";
    } else {
        status = @"failed";
    }

    return [NSString stringWithFormat:@"Sending '%@' command %@", command, status];
}

@end
