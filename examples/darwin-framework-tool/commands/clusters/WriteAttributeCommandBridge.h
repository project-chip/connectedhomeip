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

#import "MTRError_Utils.h"
#import <Matter/Matter.h>

class WriteAttribute : public ModelCommand {
public:
    WriteAttribute()
        : ModelCommand("write-by-id")
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("attribute-value", &mAttributeValue);
        AddArguments();
    }

    WriteAttribute(chip::ClusterId clusterId)
        : ModelCommand("write-by-id")
        , mClusterId(clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("attribute-value", &mAttributeValue);
        AddArguments();
    }

    ~WriteAttribute() {}

    CHIP_ERROR SendCommand(MTRBaseDevice * _Nonnull device, chip::EndpointId endpointId) override
    {
        chip::TLV::TLVWriter writer;
        chip::TLV::TLVReader reader;

        mData = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(sizeof(uint8_t), mDataMaxLen));
        VerifyOrReturnError(mData != nullptr, CHIP_ERROR_NO_MEMORY);

        writer.Init(mData, mDataMaxLen);

        ReturnErrorOnFailure(mAttributeValue.Encode(writer, chip::TLV::AnonymousTag()));
        reader.Init(mData, writer.GetLengthWritten());
        ReturnErrorOnFailure(reader.Next());

        id value = NSObjectFromCHIPTLV(&reader);
        if (value == nil) {
            return CHIP_ERROR_INTERNAL;
        }

        return WriteAttribute::SendCommand(device, endpointId, mClusterId, mAttributeId, value);
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * _Nonnull device, chip::EndpointId endpointId, chip::ClusterId clusterId,
        chip::AttributeId attributeId, id _Nonnull value)
    {
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        [device
            writeAttributeWithEndpointID:[NSNumber numberWithUnsignedShort:endpointId]
                               clusterID:[NSNumber numberWithUnsignedInteger:clusterId]
                             attributeID:[NSNumber numberWithUnsignedInteger:attributeId]
                                   value:value
                       timedWriteTimeout:mTimedInteractionTimeoutMs.HasValue()
                           ? [NSNumber numberWithUnsignedShort:mTimedInteractionTimeoutMs.Value()]
                           : nil
                                   queue:callbackQueue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  if (error != nil) {
                                      LogNSError("Error writing attribute", error);
                                  }
                                  if (values) {
                                      for (id item in values) {
                                          NSLog(@"Response Item: %@", [item description]);
                                      }
                                  }
                                  SetCommandExitStatus(error);
                              }];
        return CHIP_NO_ERROR;
    }

protected:
    WriteAttribute(const char * _Nonnull attributeName)
        : ModelCommand("write")
    {
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        // Subclasses are responsible for calling AddArguments.
    }

    void AddArguments()
    {
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs,
            "If provided, do a timed write with the given timed interaction timeout.");
        ModelCommand::AddArguments();
    }

    chip::Optional<uint16_t> mTimedInteractionTimeoutMs;
    chip::Optional<uint32_t> mDataVersion;

private:
    chip::ClusterId mClusterId;
    chip::AttributeId mAttributeId;
    CHIP_ERROR mError = CHIP_NO_ERROR;
    CustomArgument mAttributeValue;
    static constexpr uint32_t mDataMaxLen = 4096;
    uint8_t * _Nullable mData = nullptr;
};
