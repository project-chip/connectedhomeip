/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#import "CHIPDevice_Internal.h"
#import "CHIPError_Internal.h"
#import "CHIPLogging.h"
#import <setup_payload/ManualSetupPayloadGenerator.h>
#import <setup_payload/SetupPayload.h>

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

- (BOOL)openPairingWindow:(NSUInteger)duration error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::SetupPayload setupPayload;

    if (duration > UINT16_MAX) {
        CHIP_LOG_ERROR("Error: Duration %tu is too large. Max value %d", duration, UINT16_MAX);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE];
        }
        return NO;
    }

    [self.lock lock];
    err = self.cppDevice->OpenPairingWindow(
        (uint16_t) duration, chip::Controller::Device::CommissioningWindowOption::kOriginalSetupCode, setupPayload);
    [self.lock unlock];

    if (err != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error(%s): Open Pairing Window failed", chip::ErrorStr(err));
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:err];
        }
        return NO;
    }

    return YES;
}

- (NSString *)openPairingWindowWithPIN:(NSUInteger)duration
                         discriminator:(NSUInteger)discriminator
                              setupPIN:(NSUInteger)setupPIN
                                 error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::SetupPayload setupPayload;

    if (duration > UINT16_MAX) {
        CHIP_LOG_ERROR("Error: Duration %tu is too large. Max value %d", duration, UINT16_MAX);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE];
        }
        return nil;
    }

    if (discriminator > 0xfff) {
        CHIP_LOG_ERROR("Error: Discriminator %tu is too large. Max value %d", discriminator, 0xfff);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE];
        }
        return nil;
    } else {
        setupPayload.discriminator = (uint16_t) discriminator;
    }

    setupPIN &= ((1 << chip::kSetupPINCodeFieldLengthInBits) - 1);
    setupPayload.setUpPINCode = (uint32_t) setupPIN;

    [self.lock lock];
    err = self.cppDevice->OpenPairingWindow(
        (uint16_t) duration, chip::Controller::Device::CommissioningWindowOption::kTokenWithProvidedPIN, setupPayload);
    [self.lock unlock];

    if (err != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error(%s): Open Pairing Window failed", chip::ErrorStr(err));
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:err];
        }
        return nil;
    }

    chip::ManualSetupPayloadGenerator generator(setupPayload);
    std::string outCode;

    if (generator.payloadDecimalStringRepresentation(outCode) == CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Setup code is %s", outCode.c_str());
    } else {
        CHIP_LOG_ERROR("Failed to get decimal setup code");
        return nil;
    }

    return [NSString stringWithCString:outCode.c_str() encoding:[NSString defaultCStringEncoding]];
}

- (BOOL)isActive
{
    bool isActive = false;

    [self.lock lock];
    isActive = self.cppDevice->IsActive();
    [self.lock unlock];

    return isActive ? YES : NO;
}
@end
