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

#include <app/WriteClient.h>

#include "ModelCommandBridge.h"

class WriteAttribute : public ModelCommand {
public:
    WriteAttribute()
        : ModelCommand("write-by-id")
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("attribute-value", &mAttributeValue);
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
        ModelCommand::AddArguments();
    }

    WriteAttribute(chip::ClusterId clusterId)
        : ModelCommand("write-by-id")
        , mClusterId(clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("attribute-value", &mAttributeValue);
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
        ModelCommand::AddArguments();
    }

    WriteAttribute(const char * _Nonnull attributeName)
        : ModelCommand("write")
    {
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
    }

    ~WriteAttribute() {}

    CHIP_ERROR SendCommand(CHIPDevice * _Nonnull device, chip::EndpointId endpointId) override
    {
        chip::TLV::TLVWriter writer;
        chip::TLV::TLVReader reader;
        NSDictionary * valueDict = @{@"type" : @"Structure", @"value" : @[]};

        mData = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(sizeof(uint8_t), mDataMaxLen));
        VerifyOrReturnError(mData != nullptr, CHIP_ERROR_NO_MEMORY);

        writer.Init(mData, mDataMaxLen);

        ReturnErrorOnFailure(mAttributeValue.Encode(writer, chip::TLV::AnonymousTag()));
        reader.Init(mData, writer.GetLengthWritten());
        ReturnErrorOnFailure(reader.Next());

        id value = NSObjectFromCHIPTLV(&reader);
        if (value != nil) {
            valueDict = (NSDictionary *) value;
        }

        return WriteAttribute::SendCommand(device, endpointId, mClusterId, mAttributeId, valueDict);
    }

    CHIP_ERROR SendCommand(CHIPDevice * _Nonnull device, chip::EndpointId endpointId, chip::ClusterId clusterId,
        chip::AttributeId attributeId, id _Nonnull value)
    {
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        [device
            writeAttributeWithEndpointId:[NSNumber numberWithUnsignedShort:endpointId]
                               clusterId:[NSNumber numberWithUnsignedInteger:clusterId]
                             attributeId:[NSNumber numberWithUnsignedInteger:attributeId]
                                   value:value
                       timedWriteTimeout:mTimedInteractionTimeoutMs.HasValue()
                           ? [NSNumber numberWithUnsignedShort:mTimedInteractionTimeoutMs.Value()]
                           : nil
                             clientQueue:callbackQueue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  CHIP_ERROR chipError = [CHIPError errorToCHIPErrorCode:error];
                                  if (chipError != CHIP_NO_ERROR) {
                                      ChipLogError(chipTool, "Error: %s", chip::ErrorStr(chipError));
                                  }
                                  if (values) {
                                      for (id item in values) {
                                          NSLog(@"Response Item: %@", [item description]);
                                      }
                                  }
                                  SetCommandExitStatus(chipError);
                              }];
        return CHIP_NO_ERROR;
    }

protected:
    chip::Optional<uint16_t> mTimedInteractionTimeoutMs;

private:
    chip::ClusterId mClusterId;
    chip::AttributeId mAttributeId;
    CHIP_ERROR mError = CHIP_NO_ERROR;
    CustomArgument mAttributeValue;
    static constexpr uint32_t mDataMaxLen = 4096;
    uint8_t * _Nullable mData = nullptr;
};
