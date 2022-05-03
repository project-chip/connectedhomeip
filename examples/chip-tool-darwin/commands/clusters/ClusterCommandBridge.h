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

#import <CHIP/CHIP.h>
#import <CHIP/CHIPDevice_Internal.h>
#include <lib/support/UnitTestUtils.h>

#include "ModelCommandBridge.h"

class ClusterCommand : public ModelCommand {
public:
    ClusterCommand()
        : ModelCommand("command-by-id")
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("command-id", 0, UINT32_MAX, &mCommandId);
        AddArgument("payload", &mPayload);
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
        AddArgument("repeat-count", 1, UINT16_MAX, &mRepeatCount);
        AddArgument("repeat-delay-ms", 0, UINT16_MAX, &mRepeatDelayInMs);
        ModelCommand::AddArguments();
    }

    ClusterCommand(chip::ClusterId clusterId)
        : ModelCommand("command-by-id")
        , mClusterId(clusterId)
    {
        AddArgument("command-id", 0, UINT32_MAX, &mCommandId);
        AddArgument("payload", &mPayload);
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
        AddArgument("repeat-count", 1, UINT16_MAX, &mRepeatCount);
        AddArgument("repeat-delay-ms", 0, UINT16_MAX, &mRepeatDelayInMs);
        ModelCommand::AddArguments();
    }

    ClusterCommand(const char * _Nonnull commandName)
        : ModelCommand(commandName)
    {
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
        AddArgument("repeat-count", 1, UINT16_MAX, &mRepeatCount);
        AddArgument("repeat-delay-ms", 0, UINT16_MAX, &mRepeatDelayInMs);
    }

    ~ClusterCommand() {}

    CHIP_ERROR SendCommand(CHIPDevice * _Nonnull device, chip::EndpointId endpointId) override
    {
        chip::TLV::TLVWriter writer;
        chip::TLV::TLVReader reader;
        NSDictionary * commandFieldDict = @{@"type" : @"Structure", @"value" : @[]};

        mData = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(sizeof(uint8_t), mDataMaxLen));
        VerifyOrReturnError(mData != nullptr, CHIP_ERROR_NO_MEMORY);

        writer.Init(mData, mDataMaxLen);

        ReturnErrorOnFailure(mPayload.Encode(writer, chip::TLV::AnonymousTag()));
        reader.Init(mData, writer.GetLengthWritten());
        ReturnErrorOnFailure(reader.Next());

        id commandFields = NSObjectFromCHIPTLV(&reader);
        if (commandFields != nil) {
            commandFieldDict = (NSDictionary *) commandFields;
        }

        return ClusterCommand::SendCommand(device, endpointId, mClusterId, mCommandId, commandFieldDict);
    }

    CHIP_ERROR SendCommand(CHIPDevice * _Nonnull device, chip::EndpointId endpointId, chip::ClusterId clusterId,
        chip::CommandId commandId, id _Nonnull commandFields)
    {
        uint16_t repeatCount = mRepeatCount.ValueOr(1);
        uint16_t __block responsesNeeded = repeatCount;
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);

        while (repeatCount--) {
            [device invokeCommandWithEndpointId:[NSNumber numberWithUnsignedShort:endpointId]
                                      clusterId:[NSNumber numberWithUnsignedInteger:clusterId]
                                      commandId:[NSNumber numberWithUnsignedInteger:commandId]
                                  commandFields:commandFields
                             timedInvokeTimeout:mTimedInteractionTimeoutMs.HasValue()
                                 ? [NSNumber numberWithUnsignedShort:mTimedInteractionTimeoutMs.Value()]
                                 : nil
                                    clientQueue:callbackQueue
                                     completion:^(
                                         NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                         CHIP_ERROR err = CHIP_NO_ERROR;
                                         responsesNeeded--;
                                         if (error) {
                                             NSLog(@"Error: %@", error);
                                         }
                                         err = [CHIPError errorToCHIPErrorCode:error];
                                         if (responsesNeeded == 0) {
                                             SetCommandExitStatus(err);
                                         }
                                     }];

            if (mRepeatDelayInMs.HasValue()) {
                chip::test_utils::SleepMillis(mRepeatDelayInMs.Value());
            }
        }
        return CHIP_NO_ERROR;
    }

protected:
    chip::Optional<uint16_t> mTimedInteractionTimeoutMs;
    chip::Optional<uint16_t> mRepeatCount;
    chip::Optional<uint16_t> mRepeatDelayInMs;

private:
    chip::ClusterId mClusterId;
    chip::CommandId mCommandId;

    CHIP_ERROR mError = CHIP_NO_ERROR;
    CustomArgument mPayload;
    static constexpr uint32_t mDataMaxLen = 4096;
    uint8_t * _Nullable mData = nullptr;
};
