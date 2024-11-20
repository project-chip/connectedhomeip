/*
 *   Copyright (c) 2022-2023 Project CHIP Authors
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
        AddCommonByIdArguments();
    }

    ReadAttribute(chip::ClusterId clusterId)
        : ModelCommand("read-by-id")
        , mClusterId(clusterId)
    {
        AddArgument("attribute-id", 0, UINT32_MAX, &mAttributeId);
        AddArgument("fabric-filtered", 0, 1, &mFabricFiltered);
        AddCommonByIdArguments();
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
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        MTRReadParams * params = [[MTRReadParams alloc] init];
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }

        __auto_type * endpoint = @(endpointId);
        __auto_type * cluster = @(mClusterId);
        __auto_type * attribute = @(mAttributeId);
        [device
            readAttributesWithEndpointID:endpoint
                               clusterID:cluster
                             attributeID:attribute
                                  params:params
                                   queue:callbackQueue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  if (error != nil) {
                                      LogNSError("Error reading attribute", error);
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

    CHIP_ERROR SendCommand(MTRDevice * _Nonnull device, chip::EndpointId endpointId) override
    {
        MTRReadParams * params = [[MTRReadParams alloc] init];
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }

        __auto_type * endpoint = @(endpointId);
        __auto_type * cluster = @(mClusterId);
        __auto_type * attribute = @(mAttributeId);
        __auto_type values = [device readAttributeWithEndpointID:endpoint
                                                       clusterID:cluster
                                                     attributeID:attribute
                                                          params:params];

        NSError * error = nil;
        if (nil == values) {
            __auto_type * userInfo = @ { @"reason" : @"No value available." };
            error = [NSError errorWithDomain:@"Error" code:0 userInfo:userInfo];
            LogNSError("Error reading attribute", error);
            RemoteDataModelLogger::LogAttributeErrorAsJSON(endpoint, cluster, attribute, error);
        } else {
            NSLog(@"cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u: %@", mClusterId, mAttributeId, endpointId, values);
            RemoteDataModelLogger::LogAttributeAsJSON(endpoint, cluster, attribute, values);
        }

        SetCommandExitStatus(error);
        return CHIP_NO_ERROR;
    }

protected:
    void AddCommonByIdArguments()
    {
        AddArgument("use-mtr-device", 0, 1, &mUseMTRDevice,
            "Use MTRDevice instead of MTRBaseDevice to send this command. Default is false.");
        ModelCommand::AddArguments();
    }

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
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);

        MTRSubscribeParams * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }

        __auto_type * endpoint = @(endpointId);
        __auto_type * cluster = @(mClusterId);
        __auto_type * attribute = @(mAttributeId);
        [device subscribeToAttributesWithEndpointID:endpoint
            clusterID:cluster
            attributeID:attribute
            params:params
            queue:callbackQueue
            reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                if (error != nil) {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(endpoint, cluster, attribute, error);
                }
                if (values) {
                    for (id item in values) {
                        NSLog(@"Response Item: %@", [item description]);
                    }
                    RemoteDataModelLogger::LogAttributeAsJSON(endpoint, cluster, attribute, values);
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
        AddCommonArguments();
    }

    SubscribeEvent(chip::ClusterId clusterId, bool isClusterAny = false)
        : ModelCommand("subscribe-event-by-id")
        , mClusterId(clusterId)
    {
        if (isClusterAny == true) {
            AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        }
        AddArgument("event-id", 0, UINT32_MAX, &mEventId);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        AddArgument("is-urgent", 0, 1, &mIsUrgent);
        AddCommonArguments();
    }

    void AddCommonArguments()
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
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);

        MTRSubscribeParams * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mEventNumber.HasValue()) {
            params.minEventNumber = [NSNumber numberWithUnsignedLongLong:mEventNumber.Value()];
        }
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mIsUrgent.HasValue()) {
            params.reportEventsUrgently = mIsUrgent.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }

        if (strcmp(GetName(), "subscribe-event-by-id") == 0) {
            [device subscribeToEventsWithEndpointID:(endpointId == chip::kInvalidEndpointId)
                        ? nil
                        : [NSNumber numberWithUnsignedShort:endpointId]
                clusterID:(mClusterId == chip::kInvalidClusterId) ? nil : [NSNumber numberWithUnsignedInteger:mClusterId]
                eventID:(mEventId == chip::kInvalidEventId) ? nil : [NSNumber numberWithUnsignedInteger:mEventId]
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
        } else {
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
        }

        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }

protected:
    chip::Optional<bool> mKeepSubscriptions;
    chip::Optional<bool> mAutoResubscribe;
    chip::Optional<chip::EventNumber> mEventNumber;
    chip::Optional<bool> mIsUrgent;
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
    chip::EventId mEventId;
};

class ReadEvent : public ModelCommand {
public:
    ReadEvent()
        : ModelCommand("read-event-by-id")
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("event-id", 0, UINT32_MAX, &mEventId);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ModelCommand::AddArguments();
    }

    ReadEvent(chip::ClusterId clusterId)
        : ModelCommand("read-event-by-id")
        , mClusterId(clusterId)
    {
        AddArgument("event-id", 0, UINT32_MAX, &mEventId);
        AddArgument("event-min", 0, UINT64_MAX, &mEventNumber);
        ModelCommand::AddArguments();
    }

    ~ReadEvent() {}

    CHIP_ERROR SendCommand(MTRBaseDevice * _Nonnull device, chip::EndpointId endpointId) override
    {
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        MTRReadParams * params = [[MTRReadParams alloc] init];
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mEventNumber.HasValue()) {
            params.minEventNumber = [NSNumber numberWithUnsignedLongLong:mEventNumber.Value()];
        }

        [device
            readEventsWithEndpointID:(endpointId == chip::kInvalidEndpointId) ? nil : [NSNumber numberWithUnsignedShort:endpointId]
                           clusterID:(mClusterId == chip::kInvalidClusterId) ? nil : [NSNumber numberWithUnsignedInteger:mClusterId]
                             eventID:(mEventId == chip::kInvalidEventId) ? nil : [NSNumber numberWithUnsignedInteger:mEventId]
                              params:params
                               queue:callbackQueue
                          completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                              if (error != nil) {
                                  LogNSError("Error reading event", error);
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
    chip::Optional<chip::EventNumber> mEventNumber;

private:
    chip::ClusterId mClusterId;
    chip::AttributeId mEventId;
};
