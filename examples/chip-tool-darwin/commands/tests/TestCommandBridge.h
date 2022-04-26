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
#include <app/tests/suites/include/ConstraintsChecker.h>
#include <app/tests/suites/include/PICSChecker.h>
#include <app/tests/suites/include/ValueChecker.h>
#include <lib/support/UnitTestUtils.h>
#include <zap-generated/cluster/CHIPTestClustersObjc.h>

#import <CHIP/CHIPError.h>

constexpr uint16_t kTimeoutInSeconds = 90;

class TestCommandBridge : public CHIPCommandBridge, public ValueChecker, public ConstraintsChecker, public PICSChecker {
public:
    TestCommandBridge(const char * _Nonnull commandName)
        : CHIPCommandBridge(commandName)
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

    void Exit(std::string message, CHIP_ERROR err = CHIP_ERROR_INTERNAL) override
    {
        ChipLogError(chipTool, " ***** Test Failure: %s\n", message.c_str());
        SetCommandExitStatus(err);
    }

    /////////// GlobalCommands Interface /////////
    void Log(NSString * _Nonnull message)
    {
        NSLog(@"%@", message);
        NextTest();
    }

    void WaitForMs(unsigned int ms)
    {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, ms * NSEC_PER_MSEC), mCallbackQueue, ^{
            NextTest();
        });
    }

    void UserPrompt(NSString * _Nonnull message, NSString * _Nullable expectedValue = nil) { NextTest(); }

    void WaitForCommissionee(chip::NodeId nodeId)
    {
        CHIPDeviceController * controller = CurrentCommissioner();
        VerifyOrReturn(controller != nil, SetCommandExitStatus(CHIP_ERROR_INCORRECT_STATE));

        [controller getConnectedDevice:nodeId
                                 queue:mCallbackQueue
                     completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                         CHIP_ERROR err = [CHIPError errorToCHIPErrorCode:error];
                         VerifyOrReturn(CHIP_NO_ERROR == err, SetCommandExitStatus(err));

                         mConnectedDevice = device;
                         NextTest();
                     }];
    }

    CHIPDevice * _Nullable GetConnectedDevice(void) { return mConnectedDevice; }

protected:
    dispatch_queue_t _Nullable mCallbackQueue;
    CHIPDevice * _Nullable mConnectedDevice;

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

    bool CheckConstraintNotValue(
        const char * _Nonnull itemName, const NSString * _Nonnull current, const NSString * _Nonnull expected)
    {
        const chip::CharSpan currentValue([current UTF8String], [current lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        const chip::CharSpan expectedValue([expected UTF8String], [expected lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        return ConstraintsChecker::CheckConstraintNotValue(itemName, currentValue, expectedValue);
    }

    bool CheckConstraintNotValue(const char * _Nonnull itemName, const NSData * _Nonnull current, const NSData * _Nonnull expected)
    {
        const chip::ByteSpan currentValue(static_cast<const uint8_t *>([current bytes]), [current length]);
        const chip::ByteSpan expectedValue(static_cast<const uint8_t *>([expected bytes]), [expected length]);
        return ConstraintsChecker::CheckConstraintNotValue(itemName, currentValue, expectedValue);
    }

    bool CheckConstraintNotValue(const char * _Nonnull itemName, const NSNumber * _Nonnull current, NSNumber * _Nonnull expected)
    {
        if ([current isEqualToNumber:expected]) {
            Exit(std::string(itemName) + " got unexpected value: " + std::string([[current stringValue] UTF8String]));
            return false;
        }

        return true;
    }

    template <typename T>
    bool CheckConstraintNotValue(const char * _Nonnull itemName, const NSNumber * _Nonnull current, T expected)
    {
        return CheckConstraintNotValue(itemName, current, @(expected));
    }

    template <typename T> bool CheckConstraintNotValue(const char * _Nonnull itemName, NSError * _Nullable current, T expected)
    {
        NSNumber * currentValue = @([CHIPError errorToCHIPErrorCode:current].AsInteger());
        return CheckConstraintNotValue(itemName, currentValue, @(expected));
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
};
