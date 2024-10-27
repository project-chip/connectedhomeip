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

constexpr uint32_t kDefaultExpectedValueInterval = 60000;

class WriteAttribute : public ModelCommand {
public:
    WriteAttribute()
        : ModelCommand("write-by-id")
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("attribute-value", &mAttributeValue);
        AddCommonByIdArguments();
    }

    WriteAttribute(chip::ClusterId clusterId)
        : ModelCommand("write-by-id")
        , mClusterId(clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("attribute-value", &mAttributeValue);
        AddCommonByIdArguments();
    }

    ~WriteAttribute() {}

    CHIP_ERROR SendCommand(MTRBaseDevice * _Nonnull device, chip::EndpointId endpointId) override
    {
        id value;
        ReturnErrorOnFailure(GetValue(&value));
        return WriteAttribute::SendCommand(device, endpointId, mClusterId, mAttributeId, value);
    }

    CHIP_ERROR SendCommand(MTRDevice * _Nonnull device, chip::EndpointId endpointId) override
    {
        id value;
        ReturnErrorOnFailure(GetValue(&value));
        return WriteAttribute::SendCommand(device, endpointId, mClusterId, mAttributeId, value);
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * _Nonnull device, chip::EndpointId endpointId, chip::ClusterId clusterId,
        chip::AttributeId attributeId, id _Nonnull value)
    {
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);

        __auto_type * endpoint = @(endpointId);
        __auto_type * cluster = @(mClusterId);
        __auto_type * attribute = @(mAttributeId);
        [device
            writeAttributeWithEndpointID:endpoint
                               clusterID:cluster
                             attributeID:attribute
                                   value:value
                       timedWriteTimeout:mTimedInteractionTimeoutMs.HasValue()
                           ? [NSNumber numberWithUnsignedShort:mTimedInteractionTimeoutMs.Value()]
                           : nil
                                   queue:callbackQueue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  if (error != nil) {
                                      LogNSError("Error writing attribute", error);
                                      RemoteDataModelLogger::LogAttributeErrorAsJSON(endpoint, cluster, attribute, error);
                                  }
                                  if (values) {
                                      for (id item in values) {
                                          NSLog(@"Response Item: %@", [item description]);
                                      }
                                      RemoteDataModelLogger::LogAttributeAsJSON(endpoint, cluster, attribute, values);
                                  }
                                  SetCommandExitStatus(error);
                              }];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendCommand(MTRDevice * _Nonnull device, chip::EndpointId endpointId, chip::ClusterId clusterId,
        chip::AttributeId attributeId, id _Nonnull value)
    {
        __auto_type * endpoint = @(endpointId);
        __auto_type * cluster = @(mClusterId);
        __auto_type * attribute = @(mAttributeId);
        __auto_type * expectedValueInterval = @(mExpectedValueInterval.ValueOr(kDefaultExpectedValueInterval));
        __auto_type * timedWriteTimeout = mTimedInteractionTimeoutMs.HasValue()
            ? [NSNumber numberWithUnsignedShort:mTimedInteractionTimeoutMs.Value()]
            : nil;

        [device writeAttributeWithEndpointID:endpoint
                                   clusterID:cluster
                                 attributeID:attribute
                                       value:value
                       expectedValueInterval:expectedValueInterval
                           timedWriteTimeout:timedWriteTimeout];

        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }

protected:
    WriteAttribute(const char * _Nonnull attributeName)
        : ModelCommand("write")
    {
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersion);
        // Subclasses are responsible for calling AddArguments.
    }

    void AddCommonByIdArguments()
    {
        AddArgument("use-mtr-device", 0, 1, &mUseMTRDevice,
            "Use MTRDevice instead of MTRBaseDevice to send this command. Default is false.");
        AddArgument("expectedValueInterval", 0, UINT32_MAX, &mExpectedValueInterval, "When the write is issued using an MTRDevice (via –use-mtr-device), specify the maximum interval (in milliseconds) during which reads of the attribute will return the expected value. The default is 60000 milliseconds (60 seconds).");
        AddArguments();
    }

    void AddArguments()
    {
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs,
            "If provided, do a timed write with the given timed interaction timeout. See \"7.6.10. Timed Interaction\" in the "
            "Matter specification.");
        ModelCommand::AddArguments();
    }

    chip::Optional<uint16_t> mTimedInteractionTimeoutMs;
    chip::Optional<uint32_t> mExpectedValueInterval;
    chip::Optional<uint32_t> mDataVersion;

private:
    CHIP_ERROR GetValue(id _Nonnull * _Nonnull outValue)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        chip::TLV::TLVWriter writer;
        chip::TLV::TLVReader reader;

        mData = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(sizeof(uint8_t), mDataMaxLen));
        VerifyOrExit(mData != nullptr, err = CHIP_ERROR_NO_MEMORY);

        writer.Init(mData, mDataMaxLen);

        err = mAttributeValue.Encode(writer, chip::TLV::AnonymousTag());
        SuccessOrExit(err);

        reader.Init(mData, writer.GetLengthWritten());
        err = reader.Next();
        SuccessOrExit(err);

        *outValue = NSObjectFromCHIPTLV(&reader);
        VerifyOrDo(nil != *outValue, err = CHIP_ERROR_INTERNAL);

    exit:
        if (nullptr != mData) {
            chip::Platform::MemoryFree(mData);
            mData = nullptr;
        }
        return err;
    }

    chip::ClusterId mClusterId;
    chip::AttributeId mAttributeId;
    CHIP_ERROR mError = CHIP_NO_ERROR;
    CustomArgument mAttributeValue;
    static constexpr uint32_t mDataMaxLen = 4096;
    uint8_t * _Nullable mData = nullptr;
};
