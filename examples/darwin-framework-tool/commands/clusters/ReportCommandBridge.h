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

#include "ModelCommandBridge.h"

class ReadAttribute : public ModelCommand {
public:
    ReadAttribute()
        : ModelCommand("read-by-id")
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        ModelCommand::AddArguments();
    }

    ReadAttribute(chip::ClusterId clusterId)
        : ModelCommand("read-by-id")
        , mClusterId(clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        ModelCommand::AddArguments();
    }

    ReadAttribute(const char * _Nonnull attributeName)
        : ModelCommand("read")
    {
        AddArgument("attr-name", attributeName);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        ModelCommand::AddArguments();
    }

    ~ReadAttribute() {}

    CHIP_ERROR SendCommand(MTRBaseDevice * _Nonnull device, chip::EndpointId endpointId) override
    {
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        MTRReadParams * params = [[MTRReadParams alloc] init];
        params.fabricFiltered = mFabricFiltered.HasValue() ? [NSNumber numberWithBool:mFabricFiltered.Value()] : nil;
        [device
            readAttributeWithEndpointId:[NSNumber numberWithUnsignedShort:endpointId]
                              clusterId:[NSNumber numberWithUnsignedInteger:mClusterId]
                            attributeId:[NSNumber numberWithUnsignedInteger:mAttributeId]
                                 params:params
                            clientQueue:callbackQueue
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                 if (error != nil) {
                                     LogNSError("Error reading attribute", error);
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
    chip::Optional<bool> mFabricFiltered;

private:
    chip::ClusterId mClusterId;
    chip::AttributeId mAttributeId;
};

class SubscribeAttribute : public ModelCommand {
public:
    SubscribeAttribute()
        : ModelCommand("subscribe-by-id")
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        AddArgument("wait", 0, 1, &mWait);
        ModelCommand::AddArguments();
    }

    SubscribeAttribute(chip::ClusterId clusterId)
        : ModelCommand("subscribe-by-id")
        , mClusterId(clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        AddArgument("wait", 0, 1, &mWait);
        ModelCommand::AddArguments();
    }

    SubscribeAttribute(const char * _Nonnull attributeName)
        : ModelCommand("subscribe")
    {
        AddArgument("attr-name", attributeName);
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        AddArgument("wait", 0, 1, &mWait);
        ModelCommand::AddArguments();
    }

    ~SubscribeAttribute() {}

    CHIP_ERROR SendCommand(MTRBaseDevice * _Nonnull device, chip::EndpointId endpointId) override
    {
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        MTRSubscribeParams * params = [[MTRSubscribeParams alloc] init];
        params.keepPreviousSubscriptions
            = mKeepSubscriptions.HasValue() ? [NSNumber numberWithBool:mKeepSubscriptions.Value()] : nil;
        [device subscribeAttributeWithEndpointId:[NSNumber numberWithUnsignedShort:endpointId]
            clusterId:[NSNumber numberWithUnsignedInteger:mClusterId]
            attributeId:[NSNumber numberWithUnsignedInteger:mAttributeId]
            minInterval:[NSNumber numberWithUnsignedInteger:mMinInterval]
            maxInterval:[NSNumber numberWithUnsignedInteger:mMaxInterval]
            params:params
            clientQueue:callbackQueue
            reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                if (values) {
                    for (id item in values) {
                        NSLog(@"Response Item: %@", [item description]);
                    }
                }
                if (error || !mWait) {
                    SetCommandExitStatus(error);
                }
            }
            subscriptionEstablished:^() {
                mSubscriptionEstablished = YES;
            }];

        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return chip::System::Clock::Seconds16(mWait ? UINT16_MAX : 10);
    }

protected:
    chip::Optional<bool> mKeepSubscriptions;
    chip::Optional<bool> mFabricFiltered;
    bool mSubscriptionEstablished = NO;
    uint16_t mMinInterval;
    uint16_t mMaxInterval;
    bool mWait;

    void Shutdown() override
    {
        mSubscriptionEstablished = NO;
        ModelCommand::Shutdown();
    }

    bool DeferInteractiveCleanup() override { return mSubscriptionEstablished; }

private:
    chip::ClusterId mClusterId;
    chip::AttributeId mAttributeId;
};

class SubscribeEvent : public ModelCommand {
public:
    SubscribeEvent()
        : ModelCommand("subscribe-all-events")
    {
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        AddArgument("wait", 0, 1, &mWait);
        ModelCommand::AddArguments();
    }

    ~SubscribeEvent() {}

    CHIP_ERROR SendCommand(MTRBaseDevice * _Nonnull device, chip::EndpointId endpointId) override
    {
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);

        MTRSubscribeParams * params = [[MTRSubscribeParams alloc] init];
        params.keepPreviousSubscriptions
            = mKeepSubscriptions.HasValue() ? [NSNumber numberWithBool:mKeepSubscriptions.Value()] : nil;
        [device subscribeWithQueue:callbackQueue
            minInterval:mMinInterval
            maxInterval:mMaxInterval
            params:params
            cacheContainer:nil
            attributeReportHandler:^(NSArray * value) {
                if (!mWait) {
                    SetCommandExitStatus(CHIP_NO_ERROR);
                }
            }
            eventReportHandler:^(NSArray * value) {
                for (id item in value) {
                    NSLog(@"Response Item: %@", [item description]);
                }
                if (!mWait) {
                    SetCommandExitStatus(CHIP_NO_ERROR);
                }
            }
            errorHandler:^(NSError * error) {
                if (error && !mWait) {
                    SetCommandExitStatus(error);
                }
            }
            subscriptionEstablished:^() {
                mSubscriptionEstablished = YES;
            }];

        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return chip::System::Clock::Seconds16(mWait ? UINT16_MAX : 10);
    }

protected:
    chip::Optional<bool> mKeepSubscriptions;
    chip::Optional<chip::EventNumber> mEventNumber;
    bool mSubscriptionEstablished = NO;
    uint16_t mMinInterval;
    uint16_t mMaxInterval;
    bool mWait;
};
