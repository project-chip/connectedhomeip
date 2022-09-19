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
        if (mFabricFiltered.HasValue()) {
            params.fabricFiltered = mFabricFiltered.Value();
        }
        [device readAttributePathWithEndpointID:[NSNumber numberWithUnsignedShort:endpointId]
                                      clusterID:[NSNumber numberWithUnsignedInteger:mClusterId]
                                    attributeID:[NSNumber numberWithUnsignedInteger:mAttributeId]
                                         params:params
                                          queue:callbackQueue
                                     completion:^(
                                         NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
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
        AddCommonArguments();
    }

    SubscribeAttribute(chip::ClusterId clusterId)
        : ModelCommand("subscribe-by-id")
        , mClusterId(clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddCommonArguments();
    }

    SubscribeAttribute(const char * _Nonnull attributeName)
        : ModelCommand("subscribe")
    {
        AddArgument("attr-name", attributeName);
        AddCommonArguments();
    }

    void AddCommonArguments()
    {
        AddArgument("min-interval", 0, UINT16_MAX, &mMinInterval);
        AddArgument("max-interval", 0, UINT16_MAX, &mMaxInterval);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddArgument("keepSubscriptions", 0, 1, &mKeepSubscriptions);
        AddArgument("autoResubscribe", 0, 1, &mAutoResubscribe);
        ModelCommand::AddArguments();
    }

    ~SubscribeAttribute() {}

    CHIP_ERROR SendCommand(MTRBaseDevice * _Nonnull device, chip::EndpointId endpointId) override
    {
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);

        MTRSubscribeParams * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mFabricFiltered.HasValue()) {
            params.fabricFiltered = mFabricFiltered.Value();
        }
        if (mKeepSubscriptions.HasValue()) {
            params.keepPreviousSubscriptions = mKeepSubscriptions.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.autoResubscribe = mAutoResubscribe.Value();
        }

        [device subscribeAttributePathWithEndpointID:[NSNumber numberWithUnsignedShort:endpointId]
            clusterID:[NSNumber numberWithUnsignedInteger:mClusterId]
            attributeID:[NSNumber numberWithUnsignedInteger:mAttributeId]
            params:params
            queue:callbackQueue
            reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                if (values) {
                    for (id item in values) {
                        NSLog(@"Response Item: %@", [item description]);
                    }
                }
                SetCommandExitStatus(error);
            }
            subscriptionEstablished:^() {
                mSubscriptionEstablished = YES;
            }];

        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }

protected:
    chip::Optional<bool> mKeepSubscriptions;
    chip::Optional<bool> mAutoResubscribe;
    chip::Optional<bool> mFabricFiltered;
    bool mSubscriptionEstablished = NO;
    uint16_t mMinInterval;
    uint16_t mMaxInterval;

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
        AddArgument("autoResubscribe", 0, 1, &mAutoResubscribe);
        ModelCommand::AddArguments();
    }

    ~SubscribeEvent() {}

    CHIP_ERROR SendCommand(MTRBaseDevice * _Nonnull device, chip::EndpointId endpointId) override
    {
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);

        MTRSubscribeParams * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.keepPreviousSubscriptions = mKeepSubscriptions.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.autoResubscribe = mAutoResubscribe.Value();
        }

        [device subscribeWithQueue:callbackQueue
            params:params
            clusterStateCacheContainer:nil
            attributeReportHandler:^(NSArray * value) {
                SetCommandExitStatus(CHIP_NO_ERROR);
            }
            eventReportHandler:^(NSArray * value) {
                for (id item in value) {
                    NSLog(@"Response Item: %@", [item description]);
                }
                SetCommandExitStatus(CHIP_NO_ERROR);
            }
            errorHandler:^(NSError * error) {
                SetCommandExitStatus(error);
            }
            subscriptionEstablished:^() {
                mSubscriptionEstablished = YES;
            }
            resubscriptionScheduled:^(NSError * error, NSNumber * resubscriptionDelay) {
                NSLog(@"Subscription dropped with error %@.  Resubscription in %@ms", error, resubscriptionDelay);
            }];

        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }

protected:
    chip::Optional<bool> mKeepSubscriptions;
    chip::Optional<bool> mAutoResubscribe;
    chip::Optional<chip::EventNumber> mEventNumber;
    bool mSubscriptionEstablished = NO;
    uint16_t mMinInterval;
    uint16_t mMaxInterval;
};
