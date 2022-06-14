/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include "../common/CHIPCommandBridge.h"
#include <app/tests/suites/commands/delay/DelayCommands.h>
#include <app/tests/suites/commands/log/LogCommands.h>
#include <app/tests/suites/commands/system/SystemCommands.h>
#include <app/tests/suites/include/ConstraintsChecker.h>
#include <app/tests/suites/include/PICSChecker.h>
#include <app/tests/suites/include/ValueChecker.h>
#include <lib/support/UnitTestUtils.h>
#include <map>
#include <string>
#include <zap-generated/cluster/CHIPTestClustersObjc.h>

#import <CHIP/CHIP.h>
#import <CHIP/CHIPError_Internal.h>

class TestCommandBridge;

NS_ASSUME_NONNULL_BEGIN

@interface TestPairingDelegate : NSObject <CHIPDevicePairingDelegate>
@property TestCommandBridge * commandBridge;
@property chip::NodeId deviceId;
@property BOOL active; // Whether to pass on notifications to the commandBridge

- (void)onStatusUpdate:(CHIPPairingStatus)status;
- (void)onPairingComplete:(NSError * _Nullable)error;
- (void)onPairingDeleted:(NSError * _Nullable)error;
- (void)onCommissioningComplete:(NSError * _Nullable)error;

- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithTestCommandBridge:(TestCommandBridge *)commandBridge;
@end

NS_ASSUME_NONNULL_END

constexpr uint16_t kTimeoutInSeconds = 90;

class TestCommandBridge : public CHIPCommandBridge,
                          public ValueChecker,
                          public ConstraintsChecker,
                          public PICSChecker,
                          public DelayCommands,
                          public LogCommands,
                          public SystemCommands {
public:
    TestCommandBridge(const char * _Nonnull commandName)
        : CHIPCommandBridge(commandName)
        , mPairingDelegate([[TestPairingDelegate alloc] initWithTestCommandBridge:this])
    {
        AddArgument("delayInMs", 0, UINT64_MAX, &mDelayInMs);
        AddArgument("PICS", &mPICSFilePath);
    }

    ~TestCommandBridge() {};

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override
    {
        if (mPICSFilePath.HasValue()) {
            PICS.SetValue(PICSBooleanReader::Read(mPICSFilePath.Value()));
        }

        mCallbackQueue = dispatch_queue_create("com.chip-tool.command", DISPATCH_QUEUE_SERIAL);

        NextTest();
        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(kTimeoutInSeconds); }

    virtual void NextTest() = 0;

    // Support for tests that asynchronously come up with a status of some
    // sort.  Subclasses are expected to compare the provided status to the
    // expected status for the test.
    virtual void OnStatusUpdate(const chip::app::StatusIB & status) = 0;

    void Exit(std::string message, CHIP_ERROR err = CHIP_ERROR_INTERNAL) override
    {
        ChipLogError(chipTool, " ***** Test Failure: %s\n", message.c_str());
        SetCommandExitStatus(err);
    }

    /////////// DelayCommands Interface /////////
    void OnWaitForMs() override
    {
        dispatch_async(mCallbackQueue, ^{
            NextTest();
        });
    }

    CHIP_ERROR WaitForCommissionee(const char * _Nullable identity,
        const chip::app::Clusters::DelayCommands::Commands::WaitForCommissionee::Type & value) override
    {
        CHIPDeviceController * controller = GetCommissioner(identity);
        VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);

        SetIdentity(identity);

        // Disconnect our existing device; otherwise getConnectedDevice will
        // just hand it right back to us without establishing a new CASE
        // session.
        if (GetDevice(identity) != nil) {
            auto device = [GetDevice(identity) internalDevice];
            if (device != nullptr) {
                device->Disconnect();
            }
            mConnectedDevices[identity] = nil;
        }

        [controller getConnectedDevice:value.nodeId
                                 queue:mCallbackQueue
                     completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                         if (error != nil) {
                             SetCommandExitStatus(error);
                             return;
                         }

                         mConnectedDevices[identity] = device;
                         NextTest();
                     }];
        return CHIP_NO_ERROR;
    }

    /////////// CommissionerCommands-like Interface /////////
    CHIP_ERROR PairWithCode(
        const char * _Nullable identity, const chip::app::Clusters::CommissionerCommands::Commands::PairWithCode::Type & value)
    {
        CHIPDeviceController * controller = GetCommissioner(identity);
        VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);

        SetIdentity(identity);

        [controller setPairingDelegate:mPairingDelegate queue:mCallbackQueue];
        [mPairingDelegate setDeviceId:value.nodeId];
        [mPairingDelegate setActive:YES];

        NSString * payloadStr = [[NSString alloc] initWithBytes:value.payload.data()
                                                         length:value.payload.size()
                                                       encoding:NSUTF8StringEncoding];
        NSError * err;
        BOOL ok = [controller pairDevice:value.nodeId onboardingPayload:payloadStr error:&err];
        if (ok == YES) {
            return CHIP_NO_ERROR;
        }

        return [CHIPError errorToCHIPErrorCode:err];
    }

    /////////// SystemCommands Interface /////////
    CHIP_ERROR ContinueOnChipMainThread(CHIP_ERROR err) override
    {
        if (CHIP_NO_ERROR == err) {
            dispatch_async(mCallbackQueue, ^{
                NextTest();
            });
        } else {
            Exit(chip::ErrorStr(err), err);
        }
        return CHIP_NO_ERROR;
    }

    CHIPDevice * _Nullable GetDevice(const char * _Nullable identity) { return mConnectedDevices[identity]; }

    // PairingDeleted and PairingComplete need to be public so our pairing
    // delegate can call them.
    void PairingDeleted()
    {
        // This should not happen!
        Exit("Unexpected deletion of pairing");
    }

    void PairingComplete(chip::NodeId nodeId)
    {
        CHIPDeviceController * controller = CurrentCommissioner();
        VerifyOrReturn(controller != nil, Exit("No current commissioner"));

        NSError * commissionError = nil;
        [controller commissionDevice:nodeId commissioningParams:[[CHIPCommissioningParameters alloc] init] error:&commissionError];
        CHIP_ERROR err = [CHIPError errorToCHIPErrorCode:commissionError];
        if (err != CHIP_NO_ERROR) {
            Exit("Failed to kick off commissioning", err);
            return;
        }
    }

protected:
    dispatch_queue_t _Nullable mCallbackQueue;

    void Wait()
    {
        if (mDelayInMs.HasValue()) {
            chip::test_utils::SleepMillis(mDelayInMs.Value());
        }
    };

    chip::Optional<uint64_t> mDelayInMs;
    chip::Optional<char *> mPICSFilePath;
    chip::Optional<chip::EndpointId> mEndpointId;
    chip::Optional<uint16_t> mTimeout;

    bool CheckConstraintStartsWith(
        const char * _Nonnull itemName, const NSString * _Nonnull current, const char * _Nonnull expected)
    {
        const chip::CharSpan value([current UTF8String], [current lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        return ConstraintsChecker::CheckConstraintStartsWith(itemName, value, expected);
    }

    bool CheckConstraintEndsWith(const char * _Nonnull itemName, const NSString * _Nonnull current, const char * _Nonnull expected)
    {
        const chip::CharSpan value([current UTF8String], [current lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        return ConstraintsChecker::CheckConstraintEndsWith(itemName, value, expected);
    }

    bool CheckConstraintIsUpperCase(const char * _Nonnull itemName, const NSString * _Nonnull current, bool expectUpperCase)
    {
        const chip::CharSpan value([current UTF8String], [current lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        return ConstraintsChecker::CheckConstraintIsUpperCase(itemName, value, expectUpperCase);
    }

    bool CheckConstraintIsLowerCase(const char * _Nonnull itemName, const NSString * _Nonnull current, bool expectLowerCase)
    {
        const chip::CharSpan value([current UTF8String], [current lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        return ConstraintsChecker::CheckConstraintIsLowerCase(itemName, value, expectLowerCase);
    }

    bool CheckConstraintIsHexString(const char * _Nonnull itemName, const NSString * _Nonnull current, bool expectHexString)
    {
        const chip::CharSpan value([current UTF8String], [current lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        return ConstraintsChecker::CheckConstraintIsHexString(itemName, value, expectHexString);
    }

    template <typename T> bool CheckConstraintContains(const char * _Nonnull itemName, const NSArray * _Nonnull current, T expected)
    {
        for (id currentElement in current) {
            if ([currentElement isEqualToNumber:@(expected)]) {
                return true;
            }
        }

        Exit(std::string(itemName) + " expect the value " + std::to_string(expected) + " but the list does not contains it.");
        return false;
    }

    template <typename T> bool CheckConstraintExcludes(const char * _Nonnull itemName, const NSArray * _Nonnull current, T expected)
    {
        for (id currentElement in current) {
            if ([currentElement isEqualToNumber:@(expected)]) {
                Exit(std::string(itemName) + " does not expect the value " + std::to_string(expected)
                    + " but the list contains it.");
                return false;
            }
        }

        return true;
    }

    bool CheckConstraintNotValue(
        const char * _Nonnull itemName, const NSString * _Nullable current, const NSString * _Nullable expected)
    {
        if (current == nil && expected == nil) {
            Exit(std::string(itemName) + " got unexpected value. Both values are nil.");
            return false;
        }
        if ((current == nil) != (expected == nil)) {
            return true;
        }
        const chip::CharSpan currentValue([current UTF8String], [current lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        const chip::CharSpan expectedValue([expected UTF8String], [expected lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        return ConstraintsChecker::CheckConstraintNotValue(itemName, currentValue, expectedValue);
    }

    bool CheckConstraintNotValue(
        const char * _Nonnull itemName, const NSData * _Nullable current, const NSData * _Nullable expected)
    {
        if (current == nil && expected == nil) {
            Exit(std::string(itemName) + " got unexpected value. Both values are nil.");
            return false;
        }
        if ((current == nil) != (expected == nil)) {
            return true;
        }
        const chip::ByteSpan currentValue(static_cast<const uint8_t *>([current bytes]), [current length]);
        const chip::ByteSpan expectedValue(static_cast<const uint8_t *>([expected bytes]), [expected length]);
        return ConstraintsChecker::CheckConstraintNotValue(itemName, currentValue, expectedValue);
    }

    bool CheckConstraintNotValue(const char * _Nonnull itemName, const NSNumber * _Nullable current, NSNumber * _Nullable expected)
    {
        if (current == nil && expected == nil) {
            Exit(std::string(itemName) + " got unexpected value. Both values are nil.");
            return false;
        }
        if ((current == nil) != (expected == nil)) {
            return true;
        }
        if ([current isEqualToNumber:expected]) {
            Exit(std::string(itemName) + " got unexpected value: " + std::string([[current stringValue] UTF8String]));
            return false;
        }

        return true;
    }

    template <typename T>
    bool CheckConstraintNotValue(const char * _Nonnull itemName, const NSNumber * _Nullable current, T expected)
    {
        return CheckConstraintNotValue(itemName, current, @(expected));
    }

    template <typename T> bool CheckConstraintNotValue(const char * _Nonnull itemName, NSError * _Nullable current, T expected)
    {
        NSNumber * currentValue = @([CHIPError errorToCHIPErrorCode:current].AsInteger());
        return CheckConstraintNotValue(itemName, currentValue, @(expected));
    }

    using ConstraintsChecker::CheckConstraintMinValue;

    // Used when the minValue is a saved variable, since ConstraintsChecker does
    // not expect Core Foundation types.
    template <typename T, std::enable_if_t<std::is_signed<T>::value, int> = 0>
    bool CheckConstraintMinValue(const char * _Nonnull itemName, T current, const NSNumber * _Nonnull expected)
    {
        return ConstraintsChecker::CheckConstraintMinValue(itemName, current, [expected longLongValue]);
    }

    template <typename T, std::enable_if_t<!std::is_signed<T>::value, int> = 0>
    bool CheckConstraintMinValue(const char * _Nonnull itemName, T current, const NSNumber * _Nonnull expected)
    {
        return ConstraintsChecker::CheckConstraintMinValue(itemName, current, [expected unsignedLongLongValue]);
    }

    using ConstraintsChecker::CheckConstraintMaxValue;

    // Used when the maxValue is a saved variable, since ConstraintsChecker does
    // not expect Core Foundation types.
    template <typename T, std::enable_if_t<std::is_signed<T>::value, int> = 0>
    bool CheckConstraintMaxValue(const char * _Nonnull itemName, T current, const NSNumber * _Nonnull expected)
    {
        return ConstraintsChecker::CheckConstraintMaxValue(itemName, current, [expected longLongValue]);
    }

    template <typename T, std::enable_if_t<!std::is_signed<T>::value, int> = 0>
    bool CheckConstraintMaxValue(const char * _Nonnull itemName, T current, const NSNumber * _Nonnull expected)
    {
        return ConstraintsChecker::CheckConstraintMaxValue(itemName, current, [expected unsignedLongLongValue]);
    }

    bool CheckConstraintHasValue(const char * _Nonnull itemName, id _Nullable current, bool shouldHaveValue)
    {
        if (shouldHaveValue && (current == nil)) {
            Exit(std::string(itemName) + " expected to have a value but doesn't");
            return false;
        }

        if (!shouldHaveValue && (current != nil)) {
            Exit(std::string(itemName) + " not expected to have a value but does");
            return false;
        }

        return true;
    }

    bool CheckValueAsString(const char * _Nonnull itemName, const id _Nonnull current, const NSString * _Nonnull expected)
    {
        NSString * data = current;
        const chip::CharSpan currentValue([data UTF8String], [data lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        const chip::CharSpan expectedValue([expected UTF8String], [expected lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        return ValueChecker::CheckValueAsString(itemName, currentValue, expectedValue);
    }

    bool CheckValueAsString(const char * _Nonnull itemName, const id _Nonnull current, const NSData * _Nonnull expected)
    {
        NSData * data = current;
        const chip::ByteSpan currentValue(static_cast<const uint8_t *>([data bytes]), [data length]);
        const chip::ByteSpan expectedValue(static_cast<const uint8_t *>([expected bytes]), [expected length]);
        return ValueChecker::CheckValueAsString(itemName, currentValue, expectedValue);
    }

    bool CheckValue(const char * _Nonnull itemName, NSNumber * _Nonnull current, NSNumber * _Nonnull expected)
    {
        if (![current isEqualToNumber:expected]) {
            Exit(std::string(itemName) + " value mismatch: expected " + std::string([[expected stringValue] UTF8String])
                + " but got " + std::string([[current stringValue] UTF8String]));
            return false;
        }

        return true;
    }

    bool CheckValue(const char * _Nonnull itemName, id _Nonnull current, NSNumber * _Nonnull expected)
    {
        NSNumber * currentValue = current;
        return CheckValue(itemName, currentValue, expected);
    }

    template <typename T> bool CheckValue(const char * _Nonnull itemName, NSNumber * _Nonnull current, T expected)
    {
        return CheckValue(itemName, current, @(expected));
    }

    template <typename T> bool CheckValue(const char * _Nonnull itemName, id _Nonnull current, T expected)
    {
        NSNumber * currentValue = current;
        return CheckValue(itemName, currentValue, @(expected));
    }

    template <typename T> bool CheckValue(const char * _Nonnull itemName, NSError * _Nullable current, T expected)
    {

        NSNumber * currentValue = @(current.code);
        return CheckValue(itemName, currentValue, @(expected));
    }

    template <typename T, typename U> bool CheckValue(const char * _Nonnull itemName, T current, U expected)
    {

        return ValueChecker::CheckValue(itemName, current, expected);
    }

    bool CheckValueNonNull(const char * _Nonnull itemName, id _Nullable current)
    {
        if (current != nil) {
            return true;
        }

        Exit(std::string(itemName) + " expected to not be null but is");
        return false;
    }

    bool CheckValueNull(const char * _Nonnull itemName, id _Nullable current)
    {
        if (current == nil) {
            return true;
        }

        Exit(std::string(itemName) + " expected to be null but isn't");
        return false;
    }

private:
    TestPairingDelegate * _Nonnull mPairingDelegate;

    // Set of our connected devices, keyed by identity.
    std::map<std::string, CHIPDevice *> mConnectedDevices;
};

NS_ASSUME_NONNULL_BEGIN

@implementation TestPairingDelegate
- (void)onStatusUpdate:(CHIPPairingStatus)status
{
    if (_active) {
        if (status == kSecurePairingSuccess) {
            NSLog(@"Secure pairing success");
        } else if (status == kSecurePairingFailed) {
            _active = NO;
            NSLog(@"Secure pairing failed");
            _commandBridge->OnStatusUpdate(chip::app::StatusIB(chip::Protocols::InteractionModel::Status::Failure));
        }
    }
}

- (void)onPairingComplete:(NSError * _Nullable)error
{
    if (_active) {
        if (error != nil) {
            _active = NO;
            NSLog(@"Pairing complete with error");
            CHIP_ERROR err = [CHIPError errorToCHIPErrorCode:error];
            _commandBridge->OnStatusUpdate([self convertToStatusIB:err]);
        } else {
            _commandBridge->PairingComplete(_deviceId);
        }
    }
}

- (void)onPairingDeleted:(NSError * _Nullable)error
{
    if (_active) {
        _commandBridge->PairingDeleted();
    }
}

- (void)onCommissioningComplete:(NSError * _Nullable)error
{
    if (_active) {
        _active = NO;
        CHIP_ERROR err = [CHIPError errorToCHIPErrorCode:error];
        _commandBridge->OnStatusUpdate([self convertToStatusIB:err]);
    }
}

- (chip::app::StatusIB)convertToStatusIB:(CHIP_ERROR)err
{
    using chip::app::StatusIB;
    using namespace chip;
    using namespace chip::Protocols::InteractionModel;
    using namespace chip::app::Clusters::OperationalCredentials;

    if (CHIP_ERROR_INVALID_PUBLIC_KEY == err) {
        return StatusIB(Status::Failure, to_underlying(OperationalCertStatus::kInvalidPublicKey));
    }
    if (CHIP_ERROR_WRONG_NODE_ID == err) {
        return StatusIB(Status::Failure, to_underlying(OperationalCertStatus::kInvalidNodeOpId));
    }
    if (CHIP_ERROR_UNSUPPORTED_CERT_FORMAT == err) {
        return StatusIB(Status::Failure, to_underlying(OperationalCertStatus::kInvalidNOC));
    }
    if (CHIP_ERROR_FABRIC_EXISTS == err) {
        return StatusIB(Status::Failure, to_underlying(OperationalCertStatus::kFabricConflict));
    }
    if (CHIP_ERROR_INVALID_FABRIC_INDEX == err) {
        return StatusIB(Status::Failure, to_underlying(OperationalCertStatus::kInvalidFabricIndex));
    }

    return StatusIB(err);
}

- (instancetype)initWithTestCommandBridge:(TestCommandBridge *)commandBridge
{
    if (!(self = [super init])) {
        return nil;
    }

    _commandBridge = commandBridge;
    _active = NO;
    return self;
}
@end

NS_ASSUME_NONNULL_END
