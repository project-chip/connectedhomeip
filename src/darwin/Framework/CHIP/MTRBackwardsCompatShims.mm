/**
 *    Copyright (c) 2023 Project CHIP Authors
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

#import <Foundation/Foundation.h>

#import <Matter/MTRBackwardsCompatShims.h>
#import <Matter/MTRClusterConstants.h>

#import "MTRBaseClusters_Internal.h"
#import "MTRBaseDevice_Internal.h"

#import "MTRCallbackBridgeBase.h"
#import "MTRClusterStateCacheContainer_Internal.h"
#import "MTRCluster_Internal.h"
#import "MTRCommandPayloadsObjc.h"
#import "MTRDefines_Internal.h"
#import "MTRDevice_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRStructsObjc.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/ListLargeSystemExtensions.h>
#include <app/util/im-client-callbacks.h>
#include <controller/CHIPCluster.h>
#include <platform/CHIPDeviceLayer.h>

#include <type_traits>

using namespace chip::app::Clusters;
using chip::SessionHandle;
using chip::Messaging::ExchangeManager;

/**
 * This file defines implementations of manual backwards-compat shims of various sorts to handle
 * API changes that happened.
 */

namespace {

typedef void (*DefaultSuccessCallbackType)(void *);

class MTRDefaultSuccessCallbackBridge : public MTRCallbackBridge<DefaultSuccessCallback> {
public:
    MTRDefaultSuccessCallbackBridge(dispatch_queue_t queue, ResponseHandler handler, MTRActionBlock action)
        : MTRCallbackBridge<DefaultSuccessCallback>(queue, handler, action, OnSuccessFn) {};

    static void OnSuccessFn(void * context)
    {
        DispatchSuccess(context, nil);
    }
};

} // anonymous namespace

#pragma mark - Clusters that were removed wholesale: OnOffSwitchConfiguration

@implementation MTRBaseClusterOnOffSwitchConfiguration

- (void)readAttributeSwitchTypeWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeSwitchTypeWithParams:(MTRSubscribeParams * _Nonnull)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeSwitchTypeWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeSwitchActionsWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeSwitchActionsWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeSwitchActionsWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeSwitchActionsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeSwitchActionsWithParams:(MTRSubscribeParams * _Nonnull)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeSwitchActionsWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeGeneratedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeGeneratedCommandListWithParams:(MTRSubscribeParams * _Nonnull)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeGeneratedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcceptedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcceptedCommandListWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcceptedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAttributeListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAttributeListWithParams:(MTRSubscribeParams * _Nonnull)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAttributeListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeFeatureMapWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeFeatureMapWithParams:(MTRSubscribeParams * _Nonnull)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeFeatureMapWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeClusterRevisionWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeClusterRevisionWithParams:(MTRSubscribeParams * _Nonnull)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeClusterRevisionWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

@end

@implementation MTRBaseClusterOnOffSwitchConfiguration (Deprecated)

- (void)readAttributeSwitchTypeWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeSwitchTypeWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeSwitchTypeWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeSwitchTypeWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                            }];
}
+ (void)readAttributeSwitchTypeWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeSwitchTypeWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}

- (void)readAttributeSwitchActionsWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeSwitchActionsWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeSwitchActionsWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeSwitchActionsWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeSwitchActionsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeSwitchActionsWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeSwitchActionsWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeSwitchActionsWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}
+ (void)readAttributeSwitchActionsWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeSwitchActionsWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}

- (void)readAttributeGeneratedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeGeneratedCommandListWithCompletion:
              ^(NSArray * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSArray *>(value), error);
              }];
}
- (void)subscribeAttributeGeneratedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeGeneratedCommandListWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                      ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          reportHandler(static_cast<NSArray *>(value), error);
                                                                                                                                      }];
}
+ (void)readAttributeGeneratedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeGeneratedCommandListWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                         ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                             // Cast is safe because subclass does not add any selectors.
                                                                                                                                             completionHandler(static_cast<NSArray *>(value), error);
                                                                                                                                         }];
}

- (void)readAttributeAcceptedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcceptedCommandListWithCompletion:
              ^(NSArray * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSArray *>(value), error);
              }];
}
- (void)subscribeAttributeAcceptedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcceptedCommandListWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSArray *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeAcceptedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcceptedCommandListWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSArray *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeAttributeListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAttributeListWithCompletion:
              ^(NSArray * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSArray *>(value), error);
              }];
}
- (void)subscribeAttributeAttributeListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAttributeListWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                               ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   reportHandler(static_cast<NSArray *>(value), error);
                                                                                                                               }];
}
+ (void)readAttributeAttributeListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAttributeListWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                  ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      completionHandler(static_cast<NSArray *>(value), error);
                                                                                                                                  }];
}

- (void)readAttributeFeatureMapWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeFeatureMapWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeFeatureMapWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeFeatureMapWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                            }];
}
+ (void)readAttributeFeatureMapWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeFeatureMapWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}

- (void)readAttributeClusterRevisionWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeClusterRevisionWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeClusterRevisionWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeClusterRevisionWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}
+ (void)readAttributeClusterRevisionWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeClusterRevisionWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}

- (nullable instancetype)initWithDevice:(MTRBaseDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
{
    return [self initWithDevice:device endpointID:@(endpoint) queue:queue];
}

@end

@implementation MTRClusterOnOffSwitchConfiguration

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSwitchTypeWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeOnOffSwitchConfigurationID) attributeID:@(MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchTypeID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeSwitchActionsWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeOnOffSwitchConfigurationID) attributeID:@(MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchActionsID) params:params];
}

- (void)writeAttributeSwitchActionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeSwitchActionsWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeSwitchActionsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeOnOffSwitchConfigurationID) attributeID:@(MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchActionsID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeOnOffSwitchConfigurationID) attributeID:@(MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeOnOffSwitchConfigurationID) attributeID:@(MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeOnOffSwitchConfigurationID) attributeID:@(MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAttributeListID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeOnOffSwitchConfigurationID) attributeID:@(MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeFeatureMapID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeOnOffSwitchConfigurationID) attributeID:@(MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeClusterRevisionID) params:params];
}

@end

@implementation MTRClusterOnOffSwitchConfiguration (Deprecated)

- (instancetype)initWithDevice:(MTRDevice *)device endpoint:(uint16_t)endpoint queue:(dispatch_queue_t)queue
{
    return [self initWithDevice:device endpointID:@(endpoint) queue:queue];
}

@end

#pragma mark - Clusters that were removed wholesale: BinaryInputBasic

@implementation MTRBaseClusterBinaryInputBasic

- (void)readAttributeActiveTextWithCompletion:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeActiveTextWithValue:(NSString * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeActiveTextWithValue:(NSString * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeActiveTextWithValue:(NSString * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeActiveTextWithParams:(MTRSubscribeParams * _Nonnull)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeActiveTextWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDescriptionWithCompletion:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeDescriptionWithValue:(NSString * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeDescriptionWithValue:(NSString * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeDescriptionWithValue:(NSString * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDescriptionWithParams:(MTRSubscribeParams * _Nonnull)params
                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                  reportHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDescriptionWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeInactiveTextWithCompletion:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeInactiveTextWithValue:(NSString * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeInactiveTextWithValue:(NSString * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeInactiveTextWithValue:(NSString * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeInactiveTextWithParams:(MTRSubscribeParams * _Nonnull)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeInactiveTextWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeOutOfServiceWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeOutOfServiceWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeOutOfServiceWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeOutOfServiceWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeOutOfServiceWithParams:(MTRSubscribeParams * _Nonnull)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeOutOfServiceWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributePolarityWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributePolarityWithParams:(MTRSubscribeParams * _Nonnull)params
                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributePolarityWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributePresentValueWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributePresentValueWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributePresentValueWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributePresentValueWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributePresentValueWithParams:(MTRSubscribeParams * _Nonnull)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributePresentValueWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeReliabilityWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeReliabilityWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeReliabilityWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeReliabilityWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeReliabilityWithParams:(MTRSubscribeParams * _Nonnull)params
                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeReliabilityWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeStatusFlagsWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeStatusFlagsWithParams:(MTRSubscribeParams * _Nonnull)params
                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeStatusFlagsWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeApplicationTypeWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeApplicationTypeWithParams:(MTRSubscribeParams * _Nonnull)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeApplicationTypeWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeGeneratedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeGeneratedCommandListWithParams:(MTRSubscribeParams * _Nonnull)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeGeneratedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcceptedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcceptedCommandListWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcceptedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAttributeListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAttributeListWithParams:(MTRSubscribeParams * _Nonnull)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAttributeListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeFeatureMapWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeFeatureMapWithParams:(MTRSubscribeParams * _Nonnull)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeFeatureMapWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeClusterRevisionWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeClusterRevisionWithParams:(MTRSubscribeParams * _Nonnull)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeClusterRevisionWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

@end

@implementation MTRBaseClusterBinaryInputBasic (Deprecated)

- (void)readAttributeActiveTextWithCompletionHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActiveTextWithCompletion:
              ^(NSString * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSString *>(value), error);
              }];
}
- (void)writeAttributeActiveTextWithValue:(NSString * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeActiveTextWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeActiveTextWithValue:(NSString * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeActiveTextWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeActiveTextWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeActiveTextWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                            ^(NSString * _Nullable value, NSError * _Nullable error) {
                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                reportHandler(static_cast<NSString *>(value), error);
                                                                                                                            }];
}
+ (void)readAttributeActiveTextWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActiveTextWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                               ^(NSString * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   completionHandler(static_cast<NSString *>(value), error);
                                                                                                                               }];
}

- (void)readAttributeDescriptionWithCompletionHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDescriptionWithCompletion:
              ^(NSString * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSString *>(value), error);
              }];
}
- (void)writeAttributeDescriptionWithValue:(NSString * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeDescriptionWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeDescriptionWithValue:(NSString * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeDescriptionWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeDescriptionWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                              params:(MTRSubscribeParams * _Nullable)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                       reportHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDescriptionWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                             ^(NSString * _Nullable value, NSError * _Nullable error) {
                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                 reportHandler(static_cast<NSString *>(value), error);
                                                                                                                             }];
}
+ (void)readAttributeDescriptionWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDescriptionWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                ^(NSString * _Nullable value, NSError * _Nullable error) {
                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                    completionHandler(static_cast<NSString *>(value), error);
                                                                                                                                }];
}

- (void)readAttributeInactiveTextWithCompletionHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeInactiveTextWithCompletion:
              ^(NSString * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSString *>(value), error);
              }];
}
- (void)writeAttributeInactiveTextWithValue:(NSString * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeInactiveTextWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeInactiveTextWithValue:(NSString * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeInactiveTextWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeInactiveTextWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeInactiveTextWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                              ^(NSString * _Nullable value, NSError * _Nullable error) {
                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                  reportHandler(static_cast<NSString *>(value), error);
                                                                                                                              }];
}
+ (void)readAttributeInactiveTextWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSString * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeInactiveTextWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                 ^(NSString * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     completionHandler(static_cast<NSString *>(value), error);
                                                                                                                                 }];
}

- (void)readAttributeOutOfServiceWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeOutOfServiceWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeOutOfServiceWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeOutOfServiceWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeOutOfServiceWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeOutOfServiceWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeOutOfServiceWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeOutOfServiceWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                  reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                              }];
}
+ (void)readAttributeOutOfServiceWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeOutOfServiceWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}

- (void)readAttributePolarityWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePolarityWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributePolarityWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                           params:(MTRSubscribeParams * _Nullable)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributePolarityWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                          ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                              // Cast is safe because subclass does not add any selectors.
                                                                                                                              reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                          }];
}
+ (void)readAttributePolarityWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePolarityWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                 completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                             }];
}

- (void)readAttributePresentValueWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePresentValueWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributePresentValueWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributePresentValueWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributePresentValueWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributePresentValueWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributePresentValueWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributePresentValueWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                  reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                              }];
}
+ (void)readAttributePresentValueWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePresentValueWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}

- (void)readAttributeReliabilityWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeReliabilityWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeReliabilityWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeReliabilityWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeReliabilityWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeReliabilityWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeReliabilityWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                              params:(MTRSubscribeParams * _Nullable)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeReliabilityWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                 reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                             }];
}
+ (void)readAttributeReliabilityWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeReliabilityWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                    completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                }];
}

- (void)readAttributeStatusFlagsWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeStatusFlagsWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeStatusFlagsWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                              params:(MTRSubscribeParams * _Nullable)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeStatusFlagsWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                 reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                             }];
}
+ (void)readAttributeStatusFlagsWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeStatusFlagsWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                    completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                }];
}

- (void)readAttributeApplicationTypeWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeApplicationTypeWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeApplicationTypeWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeApplicationTypeWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}
+ (void)readAttributeApplicationTypeWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeApplicationTypeWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}

- (void)readAttributeGeneratedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeGeneratedCommandListWithCompletion:
              ^(NSArray * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSArray *>(value), error);
              }];
}
- (void)subscribeAttributeGeneratedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeGeneratedCommandListWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                      ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          reportHandler(static_cast<NSArray *>(value), error);
                                                                                                                                      }];
}
+ (void)readAttributeGeneratedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeGeneratedCommandListWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                         ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                             // Cast is safe because subclass does not add any selectors.
                                                                                                                                             completionHandler(static_cast<NSArray *>(value), error);
                                                                                                                                         }];
}

- (void)readAttributeAcceptedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcceptedCommandListWithCompletion:
              ^(NSArray * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSArray *>(value), error);
              }];
}
- (void)subscribeAttributeAcceptedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcceptedCommandListWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSArray *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeAcceptedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcceptedCommandListWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSArray *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeAttributeListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAttributeListWithCompletion:
              ^(NSArray * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSArray *>(value), error);
              }];
}
- (void)subscribeAttributeAttributeListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAttributeListWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                               ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   reportHandler(static_cast<NSArray *>(value), error);
                                                                                                                               }];
}
+ (void)readAttributeAttributeListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAttributeListWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                  ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      completionHandler(static_cast<NSArray *>(value), error);
                                                                                                                                  }];
}

- (void)readAttributeFeatureMapWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeFeatureMapWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeFeatureMapWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeFeatureMapWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                            }];
}
+ (void)readAttributeFeatureMapWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeFeatureMapWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}

- (void)readAttributeClusterRevisionWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeClusterRevisionWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeClusterRevisionWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeClusterRevisionWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}
+ (void)readAttributeClusterRevisionWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeClusterRevisionWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}

- (nullable instancetype)initWithDevice:(MTRBaseDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
{
    return [self initWithDevice:device endpointID:@(endpoint) queue:queue];
}

@end

@implementation MTRClusterBinaryInputBasic

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveTextWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeActiveTextID) params:params];
}

- (void)writeAttributeActiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeActiveTextWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeActiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeActiveTextID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDescriptionWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeDescriptionID) params:params];
}

- (void)writeAttributeDescriptionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeDescriptionWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeDescriptionWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeDescriptionID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInactiveTextWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeInactiveTextID) params:params];
}

- (void)writeAttributeInactiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeInactiveTextWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeInactiveTextWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeInactiveTextID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOutOfServiceWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeOutOfServiceID) params:params];
}

- (void)writeAttributeOutOfServiceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeOutOfServiceWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeOutOfServiceWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeOutOfServiceID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributePolarityWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributePolarityID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributePresentValueWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributePresentValueID) params:params];
}

- (void)writeAttributePresentValueWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributePresentValueWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributePresentValueWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributePresentValueID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReliabilityWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeReliabilityID) params:params];
}

- (void)writeAttributeReliabilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeReliabilityWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeReliabilityWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeReliabilityID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeStatusFlagsWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeStatusFlagsID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApplicationTypeWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeApplicationTypeID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeGeneratedCommandListID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeAcceptedCommandListID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeAttributeListID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeFeatureMapID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBinaryInputBasicID) attributeID:@(MTRAttributeIDTypeClusterBinaryInputBasicAttributeClusterRevisionID) params:params];
}

@end

@implementation MTRClusterBinaryInputBasic (Deprecated)

- (instancetype)initWithDevice:(MTRDevice *)device endpoint:(uint16_t)endpoint queue:(dispatch_queue_t)queue
{
    return [self initWithDevice:device endpointID:@(endpoint) queue:queue];
}

@end

#pragma mark - Clusters that were removed wholesale: BarrierControl

@implementation MTRBaseClusterBarrierControl

- (void)barrierControlGoToPercentWithParams:(MTRBarrierControlClusterBarrierControlGoToPercentParams *)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}
- (void)barrierControlStopWithCompletion:(MTRStatusCompletion)completion
{
    [self barrierControlStopWithParams:nil completion:completion];
}
- (void)barrierControlStopWithParams:(MTRBarrierControlClusterBarrierControlStopParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeBarrierMovingStateWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeBarrierMovingStateWithParams:(MTRSubscribeParams * _Nonnull)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeBarrierMovingStateWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeBarrierSafetyStatusWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeBarrierSafetyStatusWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeBarrierSafetyStatusWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeBarrierCapabilitiesWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeBarrierCapabilitiesWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeBarrierCapabilitiesWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeBarrierOpenEventsWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeBarrierOpenEventsWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeBarrierOpenEventsWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeBarrierOpenEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeBarrierOpenEventsWithParams:(MTRSubscribeParams * _Nonnull)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeBarrierOpenEventsWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeBarrierCloseEventsWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeBarrierCloseEventsWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeBarrierCloseEventsWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeBarrierCloseEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeBarrierCloseEventsWithParams:(MTRSubscribeParams * _Nonnull)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeBarrierCloseEventsWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeBarrierCommandOpenEventsWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeBarrierCommandOpenEventsWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeBarrierCommandOpenEventsWithParams:(MTRSubscribeParams * _Nonnull)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeBarrierCommandOpenEventsWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeBarrierCommandCloseEventsWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeBarrierCommandCloseEventsWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeBarrierCommandCloseEventsWithParams:(MTRSubscribeParams * _Nonnull)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeBarrierCommandCloseEventsWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeBarrierOpenPeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeBarrierOpenPeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeBarrierOpenPeriodWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeBarrierOpenPeriodWithParams:(MTRSubscribeParams * _Nonnull)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeBarrierOpenPeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeBarrierClosePeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeBarrierClosePeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeBarrierClosePeriodWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeBarrierClosePeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeBarrierClosePeriodWithParams:(MTRSubscribeParams * _Nonnull)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeBarrierClosePeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeBarrierPositionWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeBarrierPositionWithParams:(MTRSubscribeParams * _Nonnull)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeBarrierPositionWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeGeneratedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeGeneratedCommandListWithParams:(MTRSubscribeParams * _Nonnull)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeGeneratedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcceptedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcceptedCommandListWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcceptedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAttributeListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAttributeListWithParams:(MTRSubscribeParams * _Nonnull)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAttributeListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeFeatureMapWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeFeatureMapWithParams:(MTRSubscribeParams * _Nonnull)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeFeatureMapWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeClusterRevisionWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeClusterRevisionWithParams:(MTRSubscribeParams * _Nonnull)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeClusterRevisionWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

@end

@implementation MTRBaseClusterBarrierControl (Deprecated)

- (void)barrierControlGoToPercentWithParams:(MTRBarrierControlClusterBarrierControlGoToPercentParams *)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self barrierControlGoToPercentWithParams:params completion:
                                                         completionHandler];
}
- (void)barrierControlStopWithParams:(MTRBarrierControlClusterBarrierControlStopParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self barrierControlStopWithParams:params completion:
                                                  completionHandler];
}
- (void)barrierControlStopWithCompletionHandler:(MTRStatusCompletion)completionHandler
{
    [self barrierControlStopWithParams:nil completionHandler:completionHandler];
}

- (void)readAttributeBarrierMovingStateWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierMovingStateWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeBarrierMovingStateWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeBarrierMovingStateWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}
+ (void)readAttributeBarrierMovingStateWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierMovingStateWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}

- (void)readAttributeBarrierSafetyStatusWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierSafetyStatusWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeBarrierSafetyStatusWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeBarrierSafetyStatusWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeBarrierSafetyStatusWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierSafetyStatusWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeBarrierCapabilitiesWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierCapabilitiesWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeBarrierCapabilitiesWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeBarrierCapabilitiesWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeBarrierCapabilitiesWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierCapabilitiesWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeBarrierOpenEventsWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierOpenEventsWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeBarrierOpenEventsWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeBarrierOpenEventsWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeBarrierOpenEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeBarrierOpenEventsWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeBarrierOpenEventsWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeBarrierOpenEventsWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}
+ (void)readAttributeBarrierOpenEventsWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierOpenEventsWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}

- (void)readAttributeBarrierCloseEventsWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierCloseEventsWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeBarrierCloseEventsWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeBarrierCloseEventsWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeBarrierCloseEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeBarrierCloseEventsWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeBarrierCloseEventsWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeBarrierCloseEventsWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}
+ (void)readAttributeBarrierCloseEventsWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierCloseEventsWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}

- (void)readAttributeBarrierCommandOpenEventsWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierCommandOpenEventsWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeBarrierCommandOpenEventsWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeBarrierCommandOpenEventsWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeBarrierCommandOpenEventsWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                           params:(MTRSubscribeParams * _Nullable)params
                                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeBarrierCommandOpenEventsWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                          ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                              // Cast is safe because subclass does not add any selectors.
                                                                                                                                              reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                          }];
}
+ (void)readAttributeBarrierCommandOpenEventsWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierCommandOpenEventsWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                                 completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                             }];
}

- (void)readAttributeBarrierCommandCloseEventsWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierCommandCloseEventsWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeBarrierCommandCloseEventsWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeBarrierCommandCloseEventsWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeBarrierCommandCloseEventsWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                            params:(MTRSubscribeParams * _Nullable)params
                                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeBarrierCommandCloseEventsWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                           ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                               // Cast is safe because subclass does not add any selectors.
                                                                                                                                               reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                           }];
}
+ (void)readAttributeBarrierCommandCloseEventsWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierCommandCloseEventsWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                                  completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                              }];
}

- (void)readAttributeBarrierOpenPeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierOpenPeriodWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeBarrierOpenPeriodWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeBarrierOpenPeriodWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeBarrierOpenPeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeBarrierOpenPeriodWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}
+ (void)readAttributeBarrierOpenPeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierOpenPeriodWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}

- (void)readAttributeBarrierClosePeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierClosePeriodWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeBarrierClosePeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeBarrierClosePeriodWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeBarrierClosePeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeBarrierClosePeriodWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeBarrierClosePeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeBarrierClosePeriodWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}
+ (void)readAttributeBarrierClosePeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierClosePeriodWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}

- (void)readAttributeBarrierPositionWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierPositionWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeBarrierPositionWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeBarrierPositionWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}
+ (void)readAttributeBarrierPositionWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeBarrierPositionWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}

- (void)readAttributeGeneratedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeGeneratedCommandListWithCompletion:
              ^(NSArray * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSArray *>(value), error);
              }];
}
- (void)subscribeAttributeGeneratedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeGeneratedCommandListWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                      ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          reportHandler(static_cast<NSArray *>(value), error);
                                                                                                                                      }];
}
+ (void)readAttributeGeneratedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeGeneratedCommandListWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                         ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                             // Cast is safe because subclass does not add any selectors.
                                                                                                                                             completionHandler(static_cast<NSArray *>(value), error);
                                                                                                                                         }];
}

- (void)readAttributeAcceptedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcceptedCommandListWithCompletion:
              ^(NSArray * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSArray *>(value), error);
              }];
}
- (void)subscribeAttributeAcceptedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcceptedCommandListWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSArray *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeAcceptedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcceptedCommandListWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSArray *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeAttributeListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAttributeListWithCompletion:
              ^(NSArray * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSArray *>(value), error);
              }];
}
- (void)subscribeAttributeAttributeListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAttributeListWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                               ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   reportHandler(static_cast<NSArray *>(value), error);
                                                                                                                               }];
}
+ (void)readAttributeAttributeListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAttributeListWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                  ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      completionHandler(static_cast<NSArray *>(value), error);
                                                                                                                                  }];
}

- (void)readAttributeFeatureMapWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeFeatureMapWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeFeatureMapWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeFeatureMapWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                            }];
}
+ (void)readAttributeFeatureMapWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeFeatureMapWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}

- (void)readAttributeClusterRevisionWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeClusterRevisionWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeClusterRevisionWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeClusterRevisionWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}
+ (void)readAttributeClusterRevisionWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeClusterRevisionWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}

- (nullable instancetype)initWithDevice:(MTRBaseDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
{
    return [self initWithDevice:device endpointID:@(endpoint) queue:queue];
}

@end

@implementation MTRClusterBarrierControl

- (void)barrierControlGoToPercentWithParams:(MTRBarrierControlClusterBarrierControlGoToPercentParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)barrierControlStopWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues expectedValueInterval:(NSNumber *)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
{
    [self barrierControlStopWithParams:nil expectedValues:expectedValues expectedValueInterval:expectedValueIntervalMs completion:completion];
}
- (void)barrierControlStopWithParams:(MTRBarrierControlClusterBarrierControlStopParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierMovingStateWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierMovingStateID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierSafetyStatusWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierSafetyStatusID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierCapabilitiesWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCapabilitiesID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierOpenEventsWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenEventsID) params:params];
}

- (void)writeAttributeBarrierOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeBarrierOpenEventsWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeBarrierOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenEventsID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierCloseEventsWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCloseEventsID) params:params];
}

- (void)writeAttributeBarrierCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeBarrierCloseEventsWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeBarrierCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCloseEventsID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierCommandOpenEventsWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandOpenEventsID) params:params];
}

- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeBarrierCommandOpenEventsWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeBarrierCommandOpenEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandOpenEventsID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierCommandCloseEventsWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandCloseEventsID) params:params];
}

- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeBarrierCommandCloseEventsWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeBarrierCommandCloseEventsWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandCloseEventsID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierOpenPeriodWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenPeriodID) params:params];
}

- (void)writeAttributeBarrierOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeBarrierOpenPeriodWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeBarrierOpenPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenPeriodID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierClosePeriodWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierClosePeriodID) params:params];
}

- (void)writeAttributeBarrierClosePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeBarrierClosePeriodWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeBarrierClosePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierClosePeriodID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeBarrierPositionWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeBarrierPositionID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeGeneratedCommandListID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeAcceptedCommandListID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeAttributeListID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeFeatureMapID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeBarrierControlID) attributeID:@(MTRAttributeIDTypeClusterBarrierControlAttributeClusterRevisionID) params:params];
}

@end

@implementation MTRClusterBarrierControl (Deprecated)

- (instancetype)initWithDevice:(MTRDevice *)device endpoint:(uint16_t)endpoint queue:(dispatch_queue_t)queue
{
    return [self initWithDevice:device endpointID:@(endpoint) queue:queue];
}

- (void)barrierControlGoToPercentWithParams:(MTRBarrierControlClusterBarrierControlGoToPercentParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler
{
    [self barrierControlGoToPercentWithParams:params expectedValues:expectedDataValueDictionaries expectedValueInterval:expectedValueIntervalMs completion:
                                                                                                                                                    completionHandler];
}
- (void)barrierControlStopWithParams:(MTRBarrierControlClusterBarrierControlStopParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler
{
    [self barrierControlStopWithParams:params expectedValues:expectedDataValueDictionaries expectedValueInterval:expectedValueIntervalMs completion:
                                                                                                                                             completionHandler];
}
- (void)barrierControlStopWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler
{
    [self barrierControlStopWithParams:nil expectedValues:expectedValues expectedValueInterval:expectedValueIntervalMs completionHandler:completionHandler];
}
@end

@interface MTRBarrierControlClusterBarrierControlGoToPercentParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRBarrierControlClusterBarrierControlStopParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@implementation MTRBarrierControlClusterBarrierControlGoToPercentParams
- (instancetype)init
{
    if (self = [super init]) {

        _percentOpen = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRBarrierControlClusterBarrierControlGoToPercentParams alloc] init];

    other.percentOpen = self.percentOpen;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: percentOpen:%@; >", NSStringFromClass([self class]), _percentOpen];
    return descriptionString;
}

@end

@implementation MTRBarrierControlClusterBarrierControlGoToPercentParams (InternalMethods)

- (CHIP_ERROR)_encodeToTLVReader:(chip::System::PacketBufferTLVReader &)reader
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error
{
    chip::System::PacketBufferTLVReader reader;
    CHIP_ERROR err = [self _encodeToTLVReader:reader];
    if (err != CHIP_NO_ERROR) {
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:err];
        }
        return nil;
    }

    auto decodedObj = MTRDecodeDataValueDictionaryFromCHIPTLV(&reader);
    if (decodedObj == nil) {
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
        }
    }
    return decodedObj;
}
@end

@implementation MTRBarrierControlClusterBarrierControlStopParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRBarrierControlClusterBarrierControlStopParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRBarrierControlClusterBarrierControlStopParams (InternalMethods)

- (CHIP_ERROR)_encodeToTLVReader:(chip::System::PacketBufferTLVReader &)reader
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error
{
    chip::System::PacketBufferTLVReader reader;
    CHIP_ERROR err = [self _encodeToTLVReader:reader];
    if (err != CHIP_NO_ERROR) {
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:err];
        }
        return nil;
    }

    auto decodedObj = MTRDecodeDataValueDictionaryFromCHIPTLV(&reader);
    if (decodedObj == nil) {
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
        }
    }
    return decodedObj;
}
@end

#pragma mark - Clusters that were removed wholesale: ElectricalMeasurement

@implementation MTRBaseClusterElectricalMeasurement

- (void)getProfileInfoCommandWithCompletion:(MTRStatusCompletion)completion
{
    [self getProfileInfoCommandWithParams:nil completion:completion];
}
- (void)getProfileInfoCommandWithParams:(MTRElectricalMeasurementClusterGetProfileInfoCommandParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}
- (void)getMeasurementProfileCommandWithParams:(MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams *)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeMeasurementTypeWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeMeasurementTypeWithParams:(MTRSubscribeParams * _Nonnull)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeMeasurementTypeWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcVoltageWithParams:(MTRSubscribeParams * _Nonnull)params
                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcVoltageMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcVoltageMinWithParams:(MTRSubscribeParams * _Nonnull)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcVoltageMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcVoltageMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcVoltageMaxWithParams:(MTRSubscribeParams * _Nonnull)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcVoltageMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcCurrentMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcCurrentMinWithParams:(MTRSubscribeParams * _Nonnull)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcCurrentMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcCurrentMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcCurrentMaxWithParams:(MTRSubscribeParams * _Nonnull)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcCurrentMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcPowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcPowerWithParams:(MTRSubscribeParams * _Nonnull)params
                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcPowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcPowerMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcPowerMinWithParams:(MTRSubscribeParams * _Nonnull)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcPowerMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcPowerMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcPowerMaxWithParams:(MTRSubscribeParams * _Nonnull)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcPowerMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcVoltageMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcVoltageMultiplierWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcVoltageMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcVoltageDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcVoltageDivisorWithParams:(MTRSubscribeParams * _Nonnull)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcVoltageDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcCurrentMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcCurrentMultiplierWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcCurrentMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcCurrentDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcCurrentDivisorWithParams:(MTRSubscribeParams * _Nonnull)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcCurrentDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcPowerMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcPowerMultiplierWithParams:(MTRSubscribeParams * _Nonnull)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcPowerMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeDcPowerDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeDcPowerDivisorWithParams:(MTRSubscribeParams * _Nonnull)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeDcPowerDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcFrequencyWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcFrequencyWithParams:(MTRSubscribeParams * _Nonnull)params
                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcFrequencyWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcFrequencyMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcFrequencyMinWithParams:(MTRSubscribeParams * _Nonnull)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcFrequencyMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcFrequencyMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcFrequencyMaxWithParams:(MTRSubscribeParams * _Nonnull)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcFrequencyMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeNeutralCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeNeutralCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeNeutralCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeTotalActivePowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeTotalActivePowerWithParams:(MTRSubscribeParams * _Nonnull)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeTotalActivePowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeTotalReactivePowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeTotalReactivePowerWithParams:(MTRSubscribeParams * _Nonnull)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeTotalReactivePowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeTotalApparentPowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeTotalApparentPowerWithParams:(MTRSubscribeParams * _Nonnull)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeTotalApparentPowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeMeasured1stHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeMeasured1stHarmonicCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeMeasured1stHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeMeasured3rdHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeMeasured3rdHarmonicCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeMeasured3rdHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeMeasured5thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeMeasured5thHarmonicCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeMeasured5thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeMeasured7thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeMeasured7thHarmonicCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeMeasured7thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeMeasured9thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeMeasured9thHarmonicCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeMeasured9thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeMeasured11thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeMeasured11thHarmonicCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeMeasured11thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeMeasuredPhase1stHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeMeasuredPhase1stHarmonicCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeMeasuredPhase1stHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeMeasuredPhase3rdHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeMeasuredPhase3rdHarmonicCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeMeasuredPhase3rdHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeMeasuredPhase5thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeMeasuredPhase5thHarmonicCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeMeasuredPhase5thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeMeasuredPhase7thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeMeasuredPhase7thHarmonicCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeMeasuredPhase7thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeMeasuredPhase9thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeMeasuredPhase9thHarmonicCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeMeasuredPhase9thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeMeasuredPhase11thHarmonicCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeMeasuredPhase11thHarmonicCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeMeasuredPhase11thHarmonicCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcFrequencyMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcFrequencyMultiplierWithParams:(MTRSubscribeParams * _Nonnull)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcFrequencyMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcFrequencyDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcFrequencyDivisorWithParams:(MTRSubscribeParams * _Nonnull)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcFrequencyDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributePowerMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributePowerMultiplierWithParams:(MTRSubscribeParams * _Nonnull)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributePowerMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributePowerDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributePowerDivisorWithParams:(MTRSubscribeParams * _Nonnull)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributePowerDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeHarmonicCurrentMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeHarmonicCurrentMultiplierWithParams:(MTRSubscribeParams * _Nonnull)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeHarmonicCurrentMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributePhaseHarmonicCurrentMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributePhaseHarmonicCurrentMultiplierWithParams:(MTRSubscribeParams * _Nonnull)params
                                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributePhaseHarmonicCurrentMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeInstantaneousVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeInstantaneousVoltageWithParams:(MTRSubscribeParams * _Nonnull)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeInstantaneousVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeInstantaneousLineCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeInstantaneousLineCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeInstantaneousLineCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeInstantaneousActiveCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeInstantaneousActiveCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeInstantaneousActiveCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeInstantaneousReactiveCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeInstantaneousReactiveCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeInstantaneousReactiveCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeInstantaneousPowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeInstantaneousPowerWithParams:(MTRSubscribeParams * _Nonnull)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeInstantaneousPowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageWithParams:(MTRSubscribeParams * _Nonnull)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageMinWithParams:(MTRSubscribeParams * _Nonnull)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageMaxWithParams:(MTRSubscribeParams * _Nonnull)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsCurrentWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsCurrentWithParams:(MTRSubscribeParams * _Nonnull)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsCurrentWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsCurrentMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsCurrentMinWithParams:(MTRSubscribeParams * _Nonnull)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsCurrentMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsCurrentMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsCurrentMaxWithParams:(MTRSubscribeParams * _Nonnull)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsCurrentMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeActivePowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeActivePowerWithParams:(MTRSubscribeParams * _Nonnull)params
                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeActivePowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeActivePowerMinWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeActivePowerMinWithParams:(MTRSubscribeParams * _Nonnull)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeActivePowerMinWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeActivePowerMaxWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeActivePowerMaxWithParams:(MTRSubscribeParams * _Nonnull)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeActivePowerMaxWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeReactivePowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeReactivePowerWithParams:(MTRSubscribeParams * _Nonnull)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeReactivePowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeApparentPowerWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeApparentPowerWithParams:(MTRSubscribeParams * _Nonnull)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeApparentPowerWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributePowerFactorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributePowerFactorWithParams:(MTRSubscribeParams * _Nonnull)params
                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributePowerFactorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAverageRmsVoltageMeasurementPeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAverageRmsVoltageMeasurementPeriodWithParams:(MTRSubscribeParams * _Nonnull)params
                                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAverageRmsVoltageMeasurementPeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAverageRmsUnderVoltageCounterWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAverageRmsUnderVoltageCounterWithParams:(MTRSubscribeParams * _Nonnull)params
                                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAverageRmsUnderVoltageCounterWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsExtremeOverVoltagePeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsExtremeOverVoltagePeriodWithParams:(MTRSubscribeParams * _Nonnull)params
                                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsExtremeOverVoltagePeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsExtremeUnderVoltagePeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsExtremeUnderVoltagePeriodWithParams:(MTRSubscribeParams * _Nonnull)params
                                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsExtremeUnderVoltagePeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageSagPeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeRmsVoltageSagPeriodWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageSagPeriodWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageSagPeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageSwellPeriodWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeRmsVoltageSwellPeriodWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageSwellPeriodWithParams:(MTRSubscribeParams * _Nonnull)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageSwellPeriodWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcVoltageMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcVoltageMultiplierWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcVoltageMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcVoltageDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcVoltageDivisorWithParams:(MTRSubscribeParams * _Nonnull)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcVoltageDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcCurrentMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcCurrentMultiplierWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcCurrentMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcCurrentDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcCurrentDivisorWithParams:(MTRSubscribeParams * _Nonnull)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcCurrentDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcPowerMultiplierWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcPowerMultiplierWithParams:(MTRSubscribeParams * _Nonnull)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcPowerMultiplierWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcPowerDivisorWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcPowerDivisorWithParams:(MTRSubscribeParams * _Nonnull)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcPowerDivisorWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeOverloadAlarmsMaskWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeOverloadAlarmsMaskWithParams:(MTRSubscribeParams * _Nonnull)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeOverloadAlarmsMaskWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeVoltageOverloadWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeVoltageOverloadWithParams:(MTRSubscribeParams * _Nonnull)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeVoltageOverloadWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeCurrentOverloadWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeCurrentOverloadWithParams:(MTRSubscribeParams * _Nonnull)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeCurrentOverloadWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcOverloadAlarmsMaskWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value completion:(MTRStatusCompletion)completion
{
    [self writeAttributeAcOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull) value params:nil completion:completion];
}
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcOverloadAlarmsMaskWithParams:(MTRSubscribeParams * _Nonnull)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcOverloadAlarmsMaskWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcVoltageOverloadWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcVoltageOverloadWithParams:(MTRSubscribeParams * _Nonnull)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcVoltageOverloadWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcCurrentOverloadWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcCurrentOverloadWithParams:(MTRSubscribeParams * _Nonnull)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcCurrentOverloadWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcActivePowerOverloadWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcActivePowerOverloadWithParams:(MTRSubscribeParams * _Nonnull)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcActivePowerOverloadWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcReactivePowerOverloadWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcReactivePowerOverloadWithParams:(MTRSubscribeParams * _Nonnull)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcReactivePowerOverloadWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAverageRmsOverVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAverageRmsOverVoltageWithParams:(MTRSubscribeParams * _Nonnull)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAverageRmsOverVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAverageRmsUnderVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAverageRmsUnderVoltageWithParams:(MTRSubscribeParams * _Nonnull)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAverageRmsUnderVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsExtremeOverVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsExtremeOverVoltageWithParams:(MTRSubscribeParams * _Nonnull)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsExtremeOverVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsExtremeUnderVoltageWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsExtremeUnderVoltageWithParams:(MTRSubscribeParams * _Nonnull)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsExtremeUnderVoltageWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageSagWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageSagWithParams:(MTRSubscribeParams * _Nonnull)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageSagWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageSwellWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageSwellWithParams:(MTRSubscribeParams * _Nonnull)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageSwellWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeLineCurrentPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeLineCurrentPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeLineCurrentPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeActiveCurrentPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeActiveCurrentPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeActiveCurrentPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeReactiveCurrentPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeReactiveCurrentPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeReactiveCurrentPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltagePhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltagePhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltagePhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageMinPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageMinPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageMinPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageMaxPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageMaxPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageMaxPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsCurrentPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsCurrentPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsCurrentPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsCurrentMinPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsCurrentMinPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsCurrentMinPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsCurrentMaxPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsCurrentMaxPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsCurrentMaxPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeActivePowerPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeActivePowerPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeActivePowerPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeActivePowerMinPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeActivePowerMinPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeActivePowerMinPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeActivePowerMaxPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeActivePowerMaxPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeActivePowerMaxPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeReactivePowerPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeReactivePowerPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeReactivePowerPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeApparentPowerPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeApparentPowerPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeApparentPowerPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributePowerFactorPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributePowerFactorPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributePowerFactorPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAverageRmsOverVoltageCounterPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAverageRmsOverVoltageCounterPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAverageRmsOverVoltageCounterPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAverageRmsUnderVoltageCounterPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAverageRmsUnderVoltageCounterPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAverageRmsUnderVoltageCounterPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsExtremeOverVoltagePeriodPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsExtremeOverVoltagePeriodPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsExtremeOverVoltagePeriodPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsExtremeUnderVoltagePeriodPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageSagPeriodPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageSagPeriodPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageSagPeriodPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageSwellPeriodPhaseBWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageSwellPeriodPhaseBWithParams:(MTRSubscribeParams * _Nonnull)params
                                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageSwellPeriodPhaseBWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeLineCurrentPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeLineCurrentPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeLineCurrentPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeActiveCurrentPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeActiveCurrentPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeActiveCurrentPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeReactiveCurrentPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeReactiveCurrentPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeReactiveCurrentPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltagePhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltagePhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltagePhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageMinPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageMinPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageMinPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageMaxPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageMaxPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageMaxPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsCurrentPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsCurrentPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsCurrentPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsCurrentMinPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsCurrentMinPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsCurrentMinPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsCurrentMaxPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsCurrentMaxPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsCurrentMaxPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeActivePowerPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeActivePowerPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeActivePowerPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeActivePowerMinPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeActivePowerMinPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeActivePowerMinPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeActivePowerMaxPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeActivePowerMaxPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeActivePowerMaxPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeReactivePowerPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeReactivePowerPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeReactivePowerPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeApparentPowerPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeApparentPowerPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeApparentPowerPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributePowerFactorPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributePowerFactorPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributePowerFactorPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAverageRmsOverVoltageCounterPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAverageRmsOverVoltageCounterPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAverageRmsOverVoltageCounterPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAverageRmsUnderVoltageCounterPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAverageRmsUnderVoltageCounterPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAverageRmsUnderVoltageCounterPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsExtremeOverVoltagePeriodPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsExtremeOverVoltagePeriodPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsExtremeOverVoltagePeriodPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsExtremeUnderVoltagePeriodPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageSagPeriodPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageSagPeriodPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageSagPeriodPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeRmsVoltageSwellPeriodPhaseCWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeRmsVoltageSwellPeriodPhaseCWithParams:(MTRSubscribeParams * _Nonnull)params
                                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeRmsVoltageSwellPeriodPhaseCWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeGeneratedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeGeneratedCommandListWithParams:(MTRSubscribeParams * _Nonnull)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                           reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeGeneratedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAcceptedCommandListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAcceptedCommandListWithParams:(MTRSubscribeParams * _Nonnull)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                          reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAcceptedCommandListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeAttributeListWithCompletion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeAttributeListWithParams:(MTRSubscribeParams * _Nonnull)params
                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                    reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeAttributeListWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeFeatureMapWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeFeatureMapWithParams:(MTRSubscribeParams * _Nonnull)params
                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeFeatureMapWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)readAttributeClusterRevisionWithCompletion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(self.callbackQueue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)subscribeAttributeClusterRevisionWithParams:(MTRSubscribeParams * _Nonnull)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    dispatch_async(self.callbackQueue, ^{
        reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

+ (void)readAttributeClusterRevisionWithClusterStateCache:(MTRClusterStateCacheContainer *)clusterStateCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completion:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completion
{
    dispatch_async(queue, ^{
        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

@end

@implementation MTRBaseClusterElectricalMeasurement (Deprecated)

- (void)getProfileInfoCommandWithParams:(MTRElectricalMeasurementClusterGetProfileInfoCommandParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self getProfileInfoCommandWithParams:params completion:
                                                     completionHandler];
}
- (void)getProfileInfoCommandWithCompletionHandler:(MTRStatusCompletion)completionHandler
{
    [self getProfileInfoCommandWithParams:nil completionHandler:completionHandler];
}
- (void)getMeasurementProfileCommandWithParams:(MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams *)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self getMeasurementProfileCommandWithParams:params completion:
                                                            completionHandler];
}

- (void)readAttributeMeasurementTypeWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasurementTypeWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeMeasurementTypeWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeMeasurementTypeWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}
+ (void)readAttributeMeasurementTypeWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasurementTypeWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}

- (void)readAttributeDcVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcVoltageWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                            params:(MTRSubscribeParams * _Nullable)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcVoltageWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                           ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                               // Cast is safe because subclass does not add any selectors.
                                                                                                                               reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                           }];
}
+ (void)readAttributeDcVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcVoltageWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                  completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                              }];
}

- (void)readAttributeDcVoltageMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcVoltageMinWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcVoltageMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcVoltageMinWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                  reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                              }];
}
+ (void)readAttributeDcVoltageMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcVoltageMinWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}

- (void)readAttributeDcVoltageMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcVoltageMaxWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcVoltageMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcVoltageMaxWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                  reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                              }];
}
+ (void)readAttributeDcVoltageMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcVoltageMaxWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}

- (void)readAttributeDcCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                            params:(MTRSubscribeParams * _Nullable)params
                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                           ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                               // Cast is safe because subclass does not add any selectors.
                                                                                                                               reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                           }];
}
+ (void)readAttributeDcCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                  completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                              }];
}

- (void)readAttributeDcCurrentMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcCurrentMinWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcCurrentMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcCurrentMinWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                  reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                              }];
}
+ (void)readAttributeDcCurrentMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcCurrentMinWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}

- (void)readAttributeDcCurrentMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcCurrentMaxWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcCurrentMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcCurrentMaxWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                  reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                              }];
}
+ (void)readAttributeDcCurrentMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcCurrentMaxWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}

- (void)readAttributeDcPowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcPowerWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcPowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                          params:(MTRSubscribeParams * _Nullable)params
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcPowerWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                         ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                             // Cast is safe because subclass does not add any selectors.
                                                                                                                             reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                         }];
}
+ (void)readAttributeDcPowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcPowerWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                            }];
}

- (void)readAttributeDcPowerMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcPowerMinWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcPowerMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcPowerMinWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                            }];
}
+ (void)readAttributeDcPowerMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcPowerMinWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}

- (void)readAttributeDcPowerMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcPowerMaxWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcPowerMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcPowerMaxWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                            }];
}
+ (void)readAttributeDcPowerMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcPowerMaxWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}

- (void)readAttributeDcVoltageMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcVoltageMultiplierWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcVoltageMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcVoltageMultiplierWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeDcVoltageMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcVoltageMultiplierWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeDcVoltageDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcVoltageDivisorWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcVoltageDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcVoltageDivisorWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}
+ (void)readAttributeDcVoltageDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcVoltageDivisorWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}

- (void)readAttributeDcCurrentMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcCurrentMultiplierWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcCurrentMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcCurrentMultiplierWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeDcCurrentMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcCurrentMultiplierWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeDcCurrentDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcCurrentDivisorWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcCurrentDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcCurrentDivisorWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}
+ (void)readAttributeDcCurrentDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcCurrentDivisorWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}

- (void)readAttributeDcPowerMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcPowerMultiplierWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcPowerMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcPowerMultiplierWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}
+ (void)readAttributeDcPowerMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcPowerMultiplierWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}

- (void)readAttributeDcPowerDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcPowerDivisorWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeDcPowerDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeDcPowerDivisorWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                    reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                }];
}
+ (void)readAttributeDcPowerDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeDcPowerDivisorWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}

- (void)readAttributeAcFrequencyWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcFrequencyWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcFrequencyWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                              params:(MTRSubscribeParams * _Nullable)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcFrequencyWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                 reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                             }];
}
+ (void)readAttributeAcFrequencyWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcFrequencyWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                    completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                }];
}

- (void)readAttributeAcFrequencyMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcFrequencyMinWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcFrequencyMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcFrequencyMinWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                    reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                }];
}
+ (void)readAttributeAcFrequencyMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcFrequencyMinWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}

- (void)readAttributeAcFrequencyMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcFrequencyMaxWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcFrequencyMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcFrequencyMaxWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                    reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                }];
}
+ (void)readAttributeAcFrequencyMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcFrequencyMaxWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}

- (void)readAttributeNeutralCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeNeutralCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeNeutralCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeNeutralCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                    reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                }];
}
+ (void)readAttributeNeutralCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeNeutralCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}

- (void)readAttributeTotalActivePowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeTotalActivePowerWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeTotalActivePowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeTotalActivePowerWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}
+ (void)readAttributeTotalActivePowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeTotalActivePowerWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}

- (void)readAttributeTotalReactivePowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeTotalReactivePowerWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeTotalReactivePowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeTotalReactivePowerWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}
+ (void)readAttributeTotalReactivePowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeTotalReactivePowerWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}

- (void)readAttributeTotalApparentPowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeTotalApparentPowerWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeTotalApparentPowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeTotalApparentPowerWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}
+ (void)readAttributeTotalApparentPowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeTotalApparentPowerWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}

- (void)readAttributeMeasured1stHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasured1stHarmonicCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeMeasured1stHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                             params:(MTRSubscribeParams * _Nullable)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeMeasured1stHarmonicCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                            }];
}
+ (void)readAttributeMeasured1stHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasured1stHarmonicCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                               }];
}

- (void)readAttributeMeasured3rdHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasured3rdHarmonicCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeMeasured3rdHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                             params:(MTRSubscribeParams * _Nullable)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeMeasured3rdHarmonicCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                            }];
}
+ (void)readAttributeMeasured3rdHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasured3rdHarmonicCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                               }];
}

- (void)readAttributeMeasured5thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasured5thHarmonicCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeMeasured5thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                             params:(MTRSubscribeParams * _Nullable)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeMeasured5thHarmonicCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                            }];
}
+ (void)readAttributeMeasured5thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasured5thHarmonicCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                               }];
}

- (void)readAttributeMeasured7thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasured7thHarmonicCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeMeasured7thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                             params:(MTRSubscribeParams * _Nullable)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeMeasured7thHarmonicCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                            }];
}
+ (void)readAttributeMeasured7thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasured7thHarmonicCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                               }];
}

- (void)readAttributeMeasured9thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasured9thHarmonicCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeMeasured9thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                             params:(MTRSubscribeParams * _Nullable)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeMeasured9thHarmonicCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                            }];
}
+ (void)readAttributeMeasured9thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasured9thHarmonicCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                               }];
}

- (void)readAttributeMeasured11thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasured11thHarmonicCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeMeasured11thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                              params:(MTRSubscribeParams * _Nullable)params
                                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeMeasured11thHarmonicCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                                 reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                             }];
}
+ (void)readAttributeMeasured11thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasured11thHarmonicCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                                    completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                }];
}

- (void)readAttributeMeasuredPhase1stHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasuredPhase1stHarmonicCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeMeasuredPhase1stHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                  params:(MTRSubscribeParams * _Nullable)params
                                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeMeasuredPhase1stHarmonicCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                 }];
}
+ (void)readAttributeMeasuredPhase1stHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasuredPhase1stHarmonicCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                    }];
}

- (void)readAttributeMeasuredPhase3rdHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasuredPhase3rdHarmonicCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeMeasuredPhase3rdHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                  params:(MTRSubscribeParams * _Nullable)params
                                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeMeasuredPhase3rdHarmonicCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                 }];
}
+ (void)readAttributeMeasuredPhase3rdHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasuredPhase3rdHarmonicCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                    }];
}

- (void)readAttributeMeasuredPhase5thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasuredPhase5thHarmonicCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeMeasuredPhase5thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                  params:(MTRSubscribeParams * _Nullable)params
                                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeMeasuredPhase5thHarmonicCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                 }];
}
+ (void)readAttributeMeasuredPhase5thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasuredPhase5thHarmonicCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                    }];
}

- (void)readAttributeMeasuredPhase7thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasuredPhase7thHarmonicCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeMeasuredPhase7thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                  params:(MTRSubscribeParams * _Nullable)params
                                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeMeasuredPhase7thHarmonicCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                 }];
}
+ (void)readAttributeMeasuredPhase7thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasuredPhase7thHarmonicCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                    }];
}

- (void)readAttributeMeasuredPhase9thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasuredPhase9thHarmonicCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeMeasuredPhase9thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                  params:(MTRSubscribeParams * _Nullable)params
                                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeMeasuredPhase9thHarmonicCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                 }];
}
+ (void)readAttributeMeasuredPhase9thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasuredPhase9thHarmonicCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                    }];
}

- (void)readAttributeMeasuredPhase11thHarmonicCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasuredPhase11thHarmonicCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeMeasuredPhase11thHarmonicCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                   params:(MTRSubscribeParams * _Nullable)params
                                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeMeasuredPhase11thHarmonicCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                                      reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                  }];
}
+ (void)readAttributeMeasuredPhase11thHarmonicCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeMeasuredPhase11thHarmonicCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                                         completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                     }];
}

- (void)readAttributeAcFrequencyMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcFrequencyMultiplierWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcFrequencyMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcFrequencyMultiplierWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}
+ (void)readAttributeAcFrequencyMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcFrequencyMultiplierWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                          ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                              // Cast is safe because subclass does not add any selectors.
                                                                                                                                              completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                          }];
}

- (void)readAttributeAcFrequencyDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcFrequencyDivisorWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcFrequencyDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcFrequencyDivisorWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}
+ (void)readAttributeAcFrequencyDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcFrequencyDivisorWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}

- (void)readAttributePowerMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePowerMultiplierWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributePowerMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributePowerMultiplierWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}
+ (void)readAttributePowerMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePowerMultiplierWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}

- (void)readAttributePowerDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePowerDivisorWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributePowerDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                               params:(MTRSubscribeParams * _Nullable)params
                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributePowerDivisorWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                  reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                              }];
}
+ (void)readAttributePowerDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePowerDivisorWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}

- (void)readAttributeHarmonicCurrentMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeHarmonicCurrentMultiplierWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeHarmonicCurrentMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                            params:(MTRSubscribeParams * _Nullable)params
                                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeHarmonicCurrentMultiplierWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                           ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                               // Cast is safe because subclass does not add any selectors.
                                                                                                                                               reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                           }];
}
+ (void)readAttributeHarmonicCurrentMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeHarmonicCurrentMultiplierWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                                  completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                              }];
}

- (void)readAttributePhaseHarmonicCurrentMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePhaseHarmonicCurrentMultiplierWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributePhaseHarmonicCurrentMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                 params:(MTRSubscribeParams * _Nullable)params
                                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributePhaseHarmonicCurrentMultiplierWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                                    reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                }];
}
+ (void)readAttributePhaseHarmonicCurrentMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePhaseHarmonicCurrentMultiplierWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                                       completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                   }];
}

- (void)readAttributeInstantaneousVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeInstantaneousVoltageWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeInstantaneousVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeInstantaneousVoltageWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}
+ (void)readAttributeInstantaneousVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeInstantaneousVoltageWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                         ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                             // Cast is safe because subclass does not add any selectors.
                                                                                                                                             completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                         }];
}

- (void)readAttributeInstantaneousLineCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeInstantaneousLineCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeInstantaneousLineCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                           params:(MTRSubscribeParams * _Nullable)params
                                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeInstantaneousLineCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                          ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                              // Cast is safe because subclass does not add any selectors.
                                                                                                                                              reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                          }];
}
+ (void)readAttributeInstantaneousLineCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeInstantaneousLineCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                                 completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                             }];
}

- (void)readAttributeInstantaneousActiveCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeInstantaneousActiveCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeInstantaneousActiveCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                             params:(MTRSubscribeParams * _Nullable)params
                                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeInstantaneousActiveCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                            }];
}
+ (void)readAttributeInstantaneousActiveCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeInstantaneousActiveCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                               }];
}

- (void)readAttributeInstantaneousReactiveCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeInstantaneousReactiveCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeInstantaneousReactiveCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                               params:(MTRSubscribeParams * _Nullable)params
                                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeInstantaneousReactiveCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                                  reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                              }];
}
+ (void)readAttributeInstantaneousReactiveCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeInstantaneousReactiveCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                                     completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                 }];
}

- (void)readAttributeInstantaneousPowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeInstantaneousPowerWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeInstantaneousPowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeInstantaneousPowerWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}
+ (void)readAttributeInstantaneousPowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeInstantaneousPowerWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}

- (void)readAttributeRmsVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                            }];
}
+ (void)readAttributeRmsVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}

- (void)readAttributeRmsVoltageMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageMinWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltageMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageMinWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}
+ (void)readAttributeRmsVoltageMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageMinWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}

- (void)readAttributeRmsVoltageMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageMaxWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltageMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageMaxWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}
+ (void)readAttributeRmsVoltageMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageMaxWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}

- (void)readAttributeRmsCurrentWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsCurrentWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsCurrentWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                            }];
}
+ (void)readAttributeRmsCurrentWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}

- (void)readAttributeRmsCurrentMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentMinWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsCurrentMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsCurrentMinWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}
+ (void)readAttributeRmsCurrentMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentMinWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}

- (void)readAttributeRmsCurrentMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentMaxWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsCurrentMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsCurrentMaxWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}
+ (void)readAttributeRmsCurrentMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentMaxWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}

- (void)readAttributeActivePowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeActivePowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                              params:(MTRSubscribeParams * _Nullable)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeActivePowerWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                 reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                             }];
}
+ (void)readAttributeActivePowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                    completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                }];
}

- (void)readAttributeActivePowerMinWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerMinWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeActivePowerMinWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeActivePowerMinWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                    reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                }];
}
+ (void)readAttributeActivePowerMinWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerMinWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}

- (void)readAttributeActivePowerMaxWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerMaxWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeActivePowerMaxWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeActivePowerMaxWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                    reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                }];
}
+ (void)readAttributeActivePowerMaxWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerMaxWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}

- (void)readAttributeReactivePowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeReactivePowerWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeReactivePowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeReactivePowerWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}
+ (void)readAttributeReactivePowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeReactivePowerWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}

- (void)readAttributeApparentPowerWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeApparentPowerWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeApparentPowerWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeApparentPowerWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}
+ (void)readAttributeApparentPowerWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeApparentPowerWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}

- (void)readAttributePowerFactorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePowerFactorWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributePowerFactorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                              params:(MTRSubscribeParams * _Nullable)params
                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributePowerFactorWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                 reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                             }];
}
+ (void)readAttributePowerFactorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePowerFactorWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                    completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                }];
}

- (void)readAttributeAverageRmsVoltageMeasurementPeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsVoltageMeasurementPeriodWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeAverageRmsVoltageMeasurementPeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                     params:(MTRSubscribeParams * _Nullable)params
                                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAverageRmsVoltageMeasurementPeriodWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                                        reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                    }];
}
+ (void)readAttributeAverageRmsVoltageMeasurementPeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsVoltageMeasurementPeriodWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                                           completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                       }];
}

- (void)readAttributeAverageRmsUnderVoltageCounterWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsUnderVoltageCounterWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeAverageRmsUnderVoltageCounterWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeAverageRmsUnderVoltageCounterWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeAverageRmsUnderVoltageCounterWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                params:(MTRSubscribeParams * _Nullable)params
                                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAverageRmsUnderVoltageCounterWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                                   reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                               }];
}
+ (void)readAttributeAverageRmsUnderVoltageCounterWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsUnderVoltageCounterWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                                      completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                  }];
}

- (void)readAttributeRmsExtremeOverVoltagePeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeOverVoltagePeriodWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeRmsExtremeOverVoltagePeriodWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeRmsExtremeOverVoltagePeriodWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeRmsExtremeOverVoltagePeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                              params:(MTRSubscribeParams * _Nullable)params
                                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsExtremeOverVoltagePeriodWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                                 reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                             }];
}
+ (void)readAttributeRmsExtremeOverVoltagePeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeOverVoltagePeriodWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                                    completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                }];
}

- (void)readAttributeRmsExtremeUnderVoltagePeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeUnderVoltagePeriodWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeRmsExtremeUnderVoltagePeriodWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeRmsExtremeUnderVoltagePeriodWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeRmsExtremeUnderVoltagePeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                               params:(MTRSubscribeParams * _Nullable)params
                                              subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                        reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsExtremeUnderVoltagePeriodWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                                  reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                              }];
}
+ (void)readAttributeRmsExtremeUnderVoltagePeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeUnderVoltagePeriodWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                                     completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                 }];
}

- (void)readAttributeRmsVoltageSagPeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSagPeriodWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeRmsVoltageSagPeriodWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeRmsVoltageSagPeriodWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeRmsVoltageSagPeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageSagPeriodWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeRmsVoltageSagPeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSagPeriodWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeRmsVoltageSwellPeriodWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSwellPeriodWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeRmsVoltageSwellPeriodWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeRmsVoltageSwellPeriodWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeRmsVoltageSwellPeriodWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageSwellPeriodWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}
+ (void)readAttributeRmsVoltageSwellPeriodWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSwellPeriodWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                          ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                              // Cast is safe because subclass does not add any selectors.
                                                                                                                                              completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                          }];
}

- (void)readAttributeAcVoltageMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcVoltageMultiplierWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcVoltageMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcVoltageMultiplierWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeAcVoltageMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcVoltageMultiplierWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeAcVoltageDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcVoltageDivisorWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcVoltageDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcVoltageDivisorWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}
+ (void)readAttributeAcVoltageDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcVoltageDivisorWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}

- (void)readAttributeAcCurrentMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcCurrentMultiplierWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcCurrentMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcCurrentMultiplierWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeAcCurrentMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcCurrentMultiplierWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeAcCurrentDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcCurrentDivisorWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcCurrentDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcCurrentDivisorWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}
+ (void)readAttributeAcCurrentDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcCurrentDivisorWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}

- (void)readAttributeAcPowerMultiplierWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcPowerMultiplierWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcPowerMultiplierWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcPowerMultiplierWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}
+ (void)readAttributeAcPowerMultiplierWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcPowerMultiplierWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}

- (void)readAttributeAcPowerDivisorWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcPowerDivisorWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcPowerDivisorWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                 params:(MTRSubscribeParams * _Nullable)params
                                subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                          reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcPowerDivisorWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                    reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                }];
}
+ (void)readAttributeAcPowerDivisorWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcPowerDivisorWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}

- (void)readAttributeOverloadAlarmsMaskWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeOverloadAlarmsMaskWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeOverloadAlarmsMaskWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeOverloadAlarmsMaskWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeOverloadAlarmsMaskWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                     params:(MTRSubscribeParams * _Nullable)params
                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeOverloadAlarmsMaskWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}
+ (void)readAttributeOverloadAlarmsMaskWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeOverloadAlarmsMaskWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}

- (void)readAttributeVoltageOverloadWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeVoltageOverloadWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeVoltageOverloadWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeVoltageOverloadWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}
+ (void)readAttributeVoltageOverloadWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeVoltageOverloadWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}

- (void)readAttributeCurrentOverloadWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeCurrentOverloadWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeCurrentOverloadWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeCurrentOverloadWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}
+ (void)readAttributeCurrentOverloadWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeCurrentOverloadWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}

- (void)readAttributeAcOverloadAlarmsMaskWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcOverloadAlarmsMaskWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeAcOverloadAlarmsMaskWithValue:value params:nil completion:completionHandler];
}
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSNumber * _Nonnull)value params:(MTRWriteParams * _Nullable)params completionHandler:(MTRStatusCompletion)completionHandler
{
    [self writeAttributeAcOverloadAlarmsMaskWithValue:value params:params completion:completionHandler];
}
- (void)subscribeAttributeAcOverloadAlarmsMaskWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcOverloadAlarmsMaskWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}
+ (void)readAttributeAcOverloadAlarmsMaskWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcOverloadAlarmsMaskWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                         ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                             // Cast is safe because subclass does not add any selectors.
                                                                                                                                             completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                         }];
}

- (void)readAttributeAcVoltageOverloadWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcVoltageOverloadWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcVoltageOverloadWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcVoltageOverloadWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}
+ (void)readAttributeAcVoltageOverloadWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcVoltageOverloadWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}

- (void)readAttributeAcCurrentOverloadWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcCurrentOverloadWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcCurrentOverloadWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcCurrentOverloadWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}
+ (void)readAttributeAcCurrentOverloadWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcCurrentOverloadWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}

- (void)readAttributeAcActivePowerOverloadWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcActivePowerOverloadWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcActivePowerOverloadWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcActivePowerOverloadWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}
+ (void)readAttributeAcActivePowerOverloadWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcActivePowerOverloadWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                          ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                              // Cast is safe because subclass does not add any selectors.
                                                                                                                                              completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                          }];
}

- (void)readAttributeAcReactivePowerOverloadWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcReactivePowerOverloadWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAcReactivePowerOverloadWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                          params:(MTRSubscribeParams * _Nullable)params
                                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                   reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcReactivePowerOverloadWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                         ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                             // Cast is safe because subclass does not add any selectors.
                                                                                                                                             reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                         }];
}
+ (void)readAttributeAcReactivePowerOverloadWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcReactivePowerOverloadWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                                completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                            }];
}

- (void)readAttributeAverageRmsOverVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsOverVoltageWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAverageRmsOverVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAverageRmsOverVoltageWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}
+ (void)readAttributeAverageRmsOverVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsOverVoltageWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                          ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                              // Cast is safe because subclass does not add any selectors.
                                                                                                                                              completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                          }];
}

- (void)readAttributeAverageRmsUnderVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsUnderVoltageWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAverageRmsUnderVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                         params:(MTRSubscribeParams * _Nullable)params
                                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAverageRmsUnderVoltageWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}
+ (void)readAttributeAverageRmsUnderVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsUnderVoltageWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                           ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                               // Cast is safe because subclass does not add any selectors.
                                                                                                                                               completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                           }];
}

- (void)readAttributeRmsExtremeOverVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeOverVoltageWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsExtremeOverVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsExtremeOverVoltageWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}
+ (void)readAttributeRmsExtremeOverVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeOverVoltageWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                          ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                              // Cast is safe because subclass does not add any selectors.
                                                                                                                                              completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                          }];
}

- (void)readAttributeRmsExtremeUnderVoltageWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeUnderVoltageWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsExtremeUnderVoltageWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                         params:(MTRSubscribeParams * _Nullable)params
                                        subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                  reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsExtremeUnderVoltageWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}
+ (void)readAttributeRmsExtremeUnderVoltageWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeUnderVoltageWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                           ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                               // Cast is safe because subclass does not add any selectors.
                                                                                                                                               completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                           }];
}

- (void)readAttributeRmsVoltageSagWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSagWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltageSagWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageSagWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}
+ (void)readAttributeRmsVoltageSagWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSagWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}

- (void)readAttributeRmsVoltageSwellWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSwellWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltageSwellWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageSwellWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}
+ (void)readAttributeRmsVoltageSwellWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSwellWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}

- (void)readAttributeLineCurrentPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeLineCurrentPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeLineCurrentPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeLineCurrentPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}
+ (void)readAttributeLineCurrentPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeLineCurrentPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}

- (void)readAttributeActiveCurrentPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActiveCurrentPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeActiveCurrentPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeActiveCurrentPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeActiveCurrentPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActiveCurrentPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeReactiveCurrentPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeReactiveCurrentPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeReactiveCurrentPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeReactiveCurrentPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}
+ (void)readAttributeReactiveCurrentPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeReactiveCurrentPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                          ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                              // Cast is safe because subclass does not add any selectors.
                                                                                                                                              completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                          }];
}

- (void)readAttributeRmsVoltagePhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltagePhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltagePhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltagePhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}
+ (void)readAttributeRmsVoltagePhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltagePhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}

- (void)readAttributeRmsVoltageMinPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageMinPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltageMinPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageMinPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeRmsVoltageMinPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageMinPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeRmsVoltageMaxPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageMaxPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltageMaxPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageMaxPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeRmsVoltageMaxPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageMaxPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeRmsCurrentPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsCurrentPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsCurrentPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}
+ (void)readAttributeRmsCurrentPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}

- (void)readAttributeRmsCurrentMinPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentMinPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsCurrentMinPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsCurrentMinPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeRmsCurrentMinPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentMinPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeRmsCurrentMaxPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentMaxPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsCurrentMaxPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsCurrentMaxPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeRmsCurrentMaxPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentMaxPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeActivePowerPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeActivePowerPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeActivePowerPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}
+ (void)readAttributeActivePowerPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}

- (void)readAttributeActivePowerMinPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerMinPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeActivePowerMinPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeActivePowerMinPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}
+ (void)readAttributeActivePowerMinPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerMinPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                         ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                             // Cast is safe because subclass does not add any selectors.
                                                                                                                                             completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                         }];
}

- (void)readAttributeActivePowerMaxPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerMaxPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeActivePowerMaxPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeActivePowerMaxPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}
+ (void)readAttributeActivePowerMaxPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerMaxPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                         ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                             // Cast is safe because subclass does not add any selectors.
                                                                                                                                             completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                         }];
}

- (void)readAttributeReactivePowerPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeReactivePowerPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeReactivePowerPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeReactivePowerPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeReactivePowerPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeReactivePowerPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeApparentPowerPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeApparentPowerPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeApparentPowerPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeApparentPowerPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeApparentPowerPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeApparentPowerPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributePowerFactorPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePowerFactorPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributePowerFactorPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributePowerFactorPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}
+ (void)readAttributePowerFactorPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePowerFactorPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}

- (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                           params:(MTRSubscribeParams * _Nullable)params
                                                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                          ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                              // Cast is safe because subclass does not add any selectors.
                                                                                                                                                              reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                          }];
}
+ (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                                                 completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                             }];
}

- (void)readAttributeAverageRmsOverVoltageCounterPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsOverVoltageCounterPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAverageRmsOverVoltageCounterPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                     params:(MTRSubscribeParams * _Nullable)params
                                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAverageRmsOverVoltageCounterPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                                        reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                    }];
}
+ (void)readAttributeAverageRmsOverVoltageCounterPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsOverVoltageCounterPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                                           completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                       }];
}

- (void)readAttributeAverageRmsUnderVoltageCounterPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsUnderVoltageCounterPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAverageRmsUnderVoltageCounterPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                      params:(MTRSubscribeParams * _Nullable)params
                                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAverageRmsUnderVoltageCounterPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                     }];
}
+ (void)readAttributeAverageRmsUnderVoltageCounterPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsUnderVoltageCounterPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                        }];
}

- (void)readAttributeRmsExtremeOverVoltagePeriodPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeOverVoltagePeriodPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsExtremeOverVoltagePeriodPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                    params:(MTRSubscribeParams * _Nullable)params
                                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsExtremeOverVoltagePeriodPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                   }];
}
+ (void)readAttributeRmsExtremeOverVoltagePeriodPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeOverVoltagePeriodPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                      }];
}

- (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsExtremeUnderVoltagePeriodPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                     params:(MTRSubscribeParams * _Nullable)params
                                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsExtremeUnderVoltagePeriodPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                                        reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                    }];
}
+ (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                                           completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                       }];
}

- (void)readAttributeRmsVoltageSagPeriodPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSagPeriodPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltageSagPeriodPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                            params:(MTRSubscribeParams * _Nullable)params
                                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageSagPeriodPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                           ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                               // Cast is safe because subclass does not add any selectors.
                                                                                                                                               reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                           }];
}
+ (void)readAttributeRmsVoltageSagPeriodPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSagPeriodPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                                  completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                              }];
}

- (void)readAttributeRmsVoltageSwellPeriodPhaseBWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSwellPeriodPhaseBWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltageSwellPeriodPhaseBWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                              params:(MTRSubscribeParams * _Nullable)params
                                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageSwellPeriodPhaseBWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                                 reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                             }];
}
+ (void)readAttributeRmsVoltageSwellPeriodPhaseBWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSwellPeriodPhaseBWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                                    completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                }];
}

- (void)readAttributeLineCurrentPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeLineCurrentPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeLineCurrentPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeLineCurrentPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}
+ (void)readAttributeLineCurrentPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeLineCurrentPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}

- (void)readAttributeActiveCurrentPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActiveCurrentPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeActiveCurrentPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeActiveCurrentPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeActiveCurrentPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActiveCurrentPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeReactiveCurrentPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeReactiveCurrentPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeReactiveCurrentPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                        params:(MTRSubscribeParams * _Nullable)params
                                       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                 reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeReactiveCurrentPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                           reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                       }];
}
+ (void)readAttributeReactiveCurrentPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeReactiveCurrentPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                          ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                              // Cast is safe because subclass does not add any selectors.
                                                                                                                                              completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                          }];
}

- (void)readAttributeRmsVoltagePhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltagePhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltagePhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltagePhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}
+ (void)readAttributeRmsVoltagePhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltagePhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}

- (void)readAttributeRmsVoltageMinPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageMinPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltageMinPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageMinPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeRmsVoltageMinPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageMinPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeRmsVoltageMaxPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageMaxPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltageMaxPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageMaxPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeRmsVoltageMaxPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageMaxPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeRmsCurrentPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsCurrentPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                   params:(MTRSubscribeParams * _Nullable)params
                                  subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                            reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsCurrentPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                  ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                  }];
}
+ (void)readAttributeRmsCurrentPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}

- (void)readAttributeRmsCurrentMinPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentMinPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsCurrentMinPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsCurrentMinPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeRmsCurrentMinPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentMinPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeRmsCurrentMaxPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentMaxPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsCurrentMaxPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsCurrentMaxPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeRmsCurrentMaxPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsCurrentMaxPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeActivePowerPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeActivePowerPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeActivePowerPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}
+ (void)readAttributeActivePowerPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}

- (void)readAttributeActivePowerMinPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerMinPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeActivePowerMinPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeActivePowerMinPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}
+ (void)readAttributeActivePowerMinPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerMinPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                         ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                             // Cast is safe because subclass does not add any selectors.
                                                                                                                                             completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                         }];
}

- (void)readAttributeActivePowerMaxPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerMaxPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeActivePowerMaxPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeActivePowerMaxPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}
+ (void)readAttributeActivePowerMaxPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeActivePowerMaxPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                         ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                             // Cast is safe because subclass does not add any selectors.
                                                                                                                                             completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                         }];
}

- (void)readAttributeReactivePowerPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeReactivePowerPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeReactivePowerPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeReactivePowerPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeReactivePowerPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeReactivePowerPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeApparentPowerPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeApparentPowerPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeApparentPowerPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeApparentPowerPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeApparentPowerPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeApparentPowerPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributePowerFactorPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePowerFactorPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributePowerFactorPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                    params:(MTRSubscribeParams * _Nullable)params
                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributePowerFactorPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                   }];
}
+ (void)readAttributePowerFactorPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributePowerFactorPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                      }];
}

- (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                           params:(MTRSubscribeParams * _Nullable)params
                                                          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                                    reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                          ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                              // Cast is safe because subclass does not add any selectors.
                                                                                                                                                              reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                          }];
}
+ (void)readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                                                 completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                             }];
}

- (void)readAttributeAverageRmsOverVoltageCounterPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsOverVoltageCounterPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAverageRmsOverVoltageCounterPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                     params:(MTRSubscribeParams * _Nullable)params
                                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAverageRmsOverVoltageCounterPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                                        reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                    }];
}
+ (void)readAttributeAverageRmsOverVoltageCounterPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsOverVoltageCounterPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                                           completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                       }];
}

- (void)readAttributeAverageRmsUnderVoltageCounterPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsUnderVoltageCounterPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeAverageRmsUnderVoltageCounterPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                      params:(MTRSubscribeParams * _Nullable)params
                                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                               reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAverageRmsUnderVoltageCounterPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                     ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                                         reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                     }];
}
+ (void)readAttributeAverageRmsUnderVoltageCounterPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAverageRmsUnderVoltageCounterPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                        ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                                            completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                        }];
}

- (void)readAttributeRmsExtremeOverVoltagePeriodPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeOverVoltagePeriodPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsExtremeOverVoltagePeriodPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                    params:(MTRSubscribeParams * _Nullable)params
                                                   subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                             reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsExtremeOverVoltagePeriodPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                   ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                       // Cast is safe because subclass does not add any selectors.
                                                                                                                                                       reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                   }];
}
+ (void)readAttributeRmsExtremeOverVoltagePeriodPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeOverVoltagePeriodPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                      ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                                          completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                      }];
}

- (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsExtremeUnderVoltagePeriodPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                                     params:(MTRSubscribeParams * _Nullable)params
                                                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                              reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsExtremeUnderVoltagePeriodPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                                        reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                    }];
}
+ (void)readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                       ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                           // Cast is safe because subclass does not add any selectors.
                                                                                                                                                           completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                       }];
}

- (void)readAttributeRmsVoltageSagPeriodPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSagPeriodPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltageSagPeriodPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                            params:(MTRSubscribeParams * _Nullable)params
                                           subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                     reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageSagPeriodPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                           ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                               // Cast is safe because subclass does not add any selectors.
                                                                                                                                               reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                           }];
}
+ (void)readAttributeRmsVoltageSagPeriodPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSagPeriodPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                  // Cast is safe because subclass does not add any selectors.
                                                                                                                                                  completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                              }];
}

- (void)readAttributeRmsVoltageSwellPeriodPhaseCWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSwellPeriodPhaseCWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeRmsVoltageSwellPeriodPhaseCWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                              params:(MTRSubscribeParams * _Nullable)params
                                             subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                       reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeRmsVoltageSwellPeriodPhaseCWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                             ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                 // Cast is safe because subclass does not add any selectors.
                                                                                                                                                 reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                             }];
}
+ (void)readAttributeRmsVoltageSwellPeriodPhaseCWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeRmsVoltageSwellPeriodPhaseCWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                                ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                                    // Cast is safe because subclass does not add any selectors.
                                                                                                                                                    completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                                }];
}

- (void)readAttributeGeneratedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeGeneratedCommandListWithCompletion:
              ^(NSArray * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSArray *>(value), error);
              }];
}
- (void)subscribeAttributeGeneratedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                       params:(MTRSubscribeParams * _Nullable)params
                                      subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                                reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeGeneratedCommandListWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                      ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                          // Cast is safe because subclass does not add any selectors.
                                                                                                                                          reportHandler(static_cast<NSArray *>(value), error);
                                                                                                                                      }];
}
+ (void)readAttributeGeneratedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeGeneratedCommandListWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                         ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                             // Cast is safe because subclass does not add any selectors.
                                                                                                                                             completionHandler(static_cast<NSArray *>(value), error);
                                                                                                                                         }];
}

- (void)readAttributeAcceptedCommandListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcceptedCommandListWithCompletion:
              ^(NSArray * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSArray *>(value), error);
              }];
}
- (void)subscribeAttributeAcceptedCommandListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                      params:(MTRSubscribeParams * _Nullable)params
                                     subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                               reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAcceptedCommandListWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                     ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                         // Cast is safe because subclass does not add any selectors.
                                                                                                                                         reportHandler(static_cast<NSArray *>(value), error);
                                                                                                                                     }];
}
+ (void)readAttributeAcceptedCommandListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAcceptedCommandListWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                        ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                            // Cast is safe because subclass does not add any selectors.
                                                                                                                                            completionHandler(static_cast<NSArray *>(value), error);
                                                                                                                                        }];
}

- (void)readAttributeAttributeListWithCompletionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAttributeListWithCompletion:
              ^(NSArray * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSArray *>(value), error);
              }];
}
- (void)subscribeAttributeAttributeListWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                params:(MTRSubscribeParams * _Nullable)params
                               subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                         reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeAttributeListWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                               ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   reportHandler(static_cast<NSArray *>(value), error);
                                                                                                                               }];
}
+ (void)readAttributeAttributeListWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeAttributeListWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                  ^(NSArray * _Nullable value, NSError * _Nullable error) {
                                                                                                                                      // Cast is safe because subclass does not add any selectors.
                                                                                                                                      completionHandler(static_cast<NSArray *>(value), error);
                                                                                                                                  }];
}

- (void)readAttributeFeatureMapWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeFeatureMapWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeFeatureMapWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                             params:(MTRSubscribeParams * _Nullable)params
                            subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                      reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeFeatureMapWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                            ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                // Cast is safe because subclass does not add any selectors.
                                                                                                                                reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                            }];
}
+ (void)readAttributeFeatureMapWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeFeatureMapWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                               ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                   // Cast is safe because subclass does not add any selectors.
                                                                                                                                   completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                               }];
}

- (void)readAttributeClusterRevisionWithCompletionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeClusterRevisionWithCompletion:
              ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                  // Cast is safe because subclass does not add any selectors.
                  completionHandler(static_cast<NSNumber *>(value), error);
              }];
}
- (void)subscribeAttributeClusterRevisionWithMinInterval:(NSNumber * _Nonnull)minInterval maxInterval:(NSNumber * _Nonnull)maxInterval
                                                  params:(MTRSubscribeParams * _Nullable)params
                                 subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
                                           reportHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))reportHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeAttributeClusterRevisionWithParams:subscribeParams subscriptionEstablished:subscriptionEstablishedHandler reportHandler:
                                                                                                                                 ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                     // Cast is safe because subclass does not add any selectors.
                                                                                                                                     reportHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                 }];
}
+ (void)readAttributeClusterRevisionWithAttributeCache:(MTRAttributeCacheContainer *)attributeCacheContainer endpoint:(NSNumber *)endpoint queue:(dispatch_queue_t)queue completionHandler:(void (^)(NSNumber * _Nullable value, NSError * _Nullable error))completionHandler
{
    [self readAttributeClusterRevisionWithClusterStateCache:attributeCacheContainer.realContainer endpoint:endpoint queue:queue completion:
                                                                                                                                    ^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                                                                                        // Cast is safe because subclass does not add any selectors.
                                                                                                                                        completionHandler(static_cast<NSNumber *>(value), error);
                                                                                                                                    }];
}

- (nullable instancetype)initWithDevice:(MTRBaseDevice *)device
                               endpoint:(uint16_t)endpoint
                                  queue:(dispatch_queue_t)queue
{
    return [self initWithDevice:device endpointID:@(endpoint) queue:queue];
}

@end

@implementation MTRClusterElectricalMeasurement

- (void)getProfileInfoCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)expectedValues expectedValueInterval:(NSNumber *)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
{
    [self getProfileInfoCommandWithParams:nil expectedValues:expectedValues expectedValueInterval:expectedValueIntervalMs completion:completion];
}
- (void)getProfileInfoCommandWithParams:(MTRElectricalMeasurementClusterGetProfileInfoCommandParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (void)getMeasurementProfileCommandWithParams:(MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completion:(MTRStatusCompletion)completion
{
    dispatch_async(self.callbackQueue, ^{
        completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED]);
    });
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasurementTypeWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasurementTypeID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcVoltageWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcVoltageMinWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMinID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcVoltageMaxWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMaxID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcCurrentMinWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMinID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcCurrentMaxWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMaxID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcPowerWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcPowerMinWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMinID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcPowerMaxWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMaxID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcVoltageMultiplierWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMultiplierID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcVoltageDivisorWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageDivisorID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMultiplierID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcCurrentDivisorWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentDivisorID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcPowerMultiplierWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMultiplierID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeDcPowerDivisorWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerDivisorID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcFrequencyWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcFrequencyMinWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMinID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcFrequencyMaxWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMaxID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeNeutralCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeNeutralCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTotalActivePowerWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalActivePowerID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTotalReactivePowerWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalReactivePowerID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeTotalApparentPowerWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalApparentPowerID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasured1stHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasured3rdHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasured5thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasured7thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasured9thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasured11thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredPhase1stHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredPhase3rdHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredPhase5thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredPhase7thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredPhase9thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeMeasuredPhase11thHarmonicCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcFrequencyMultiplierWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMultiplierID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcFrequencyDivisorWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyDivisorID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerMultiplierWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerMultiplierID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerDivisorWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerDivisorID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeHarmonicCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributePhaseHarmonicCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstantaneousVoltageWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousVoltageID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstantaneousLineCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousLineCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstantaneousActiveCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstantaneousReactiveCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeInstantaneousPowerWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousPowerID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageMinWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageMaxWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentMinWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentMaxWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerMinWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerMaxWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReactivePowerWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApparentPowerWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerFactorWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsVoltageMeasurementPeriodWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID) params:params];
}

- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeAverageRmsVoltageMeasurementPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsUnderVoltageCounterWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID) params:params];
}

- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeAverageRmsUnderVoltageCounterWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeAverageRmsUnderVoltageCounterWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeOverVoltagePeriodWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID) params:params];
}

- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeRmsExtremeOverVoltagePeriodWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeRmsExtremeOverVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeUnderVoltagePeriodWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID) params:params];
}

- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeRmsExtremeUnderVoltagePeriodWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeRmsExtremeUnderVoltagePeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSagPeriodWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID) params:params];
}

- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeRmsVoltageSagPeriodWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeRmsVoltageSagPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSwellPeriodWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID) params:params];
}

- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeRmsVoltageSwellPeriodWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeRmsVoltageSwellPeriodWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcVoltageMultiplierWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageMultiplierID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcVoltageDivisorWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageDivisorID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcCurrentMultiplierWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentMultiplierID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcCurrentDivisorWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentDivisorID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcPowerMultiplierWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerMultiplierID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcPowerDivisorWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerDivisorID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeOverloadAlarmsMaskWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeOverloadAlarmsMaskID) params:params];
}

- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeOverloadAlarmsMaskWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeOverloadAlarmsMaskID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeVoltageOverloadWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeVoltageOverloadID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeCurrentOverloadWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeCurrentOverloadID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcOverloadAlarmsMaskWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID) params:params];
}

- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs
{
    [self writeAttributeAcOverloadAlarmsMaskWithValue:dataValueDictionary expectedValueInterval:expectedValueIntervalMs params:nil];
}
- (void)writeAttributeAcOverloadAlarmsMaskWithValue:(NSDictionary<NSString *, id> *)dataValueDictionary expectedValueInterval:(NSNumber *)expectedValueIntervalMs params:(MTRWriteParams * _Nullable)params
{
    NSNumber * timedWriteTimeout = params.timedWriteTimeout;

    [self.device writeAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID) value:dataValueDictionary expectedValueInterval:expectedValueIntervalMs timedWriteTimeout:timedWriteTimeout];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcVoltageOverloadWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageOverloadID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcCurrentOverloadWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentOverloadID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcActivePowerOverloadWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcActivePowerOverloadID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcReactivePowerOverloadWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcReactivePowerOverloadID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsOverVoltageWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsUnderVoltageWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeOverVoltageWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeUnderVoltageWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSagWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSwellWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLineCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReactiveCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltagePhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageMinPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageMaxPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentMinPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentMaxPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerMinPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerMaxPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReactivePowerPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApparentPowerPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerFactorPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsVoltageMeasurementPeriodPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsOverVoltageCounterPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsUnderVoltageCounterPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeOverVoltagePeriodPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeUnderVoltagePeriodPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSagPeriodPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSwellPeriodPhaseBWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeLineCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActiveCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReactiveCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltagePhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageMinPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageMaxPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentMinPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsCurrentMaxPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerMinPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeActivePowerMaxPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeReactivePowerPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeApparentPowerPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributePowerFactorPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsVoltageMeasurementPeriodPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsOverVoltageCounterPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAverageRmsUnderVoltageCounterPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeOverVoltagePeriodPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsExtremeUnderVoltagePeriodPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSagPeriodPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeRmsVoltageSwellPeriodPhaseCWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeGeneratedCommandListWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeGeneratedCommandListID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAcceptedCommandListWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcceptedCommandListID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeAttributeListWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeAttributeListID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeFeatureMapWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeFeatureMapID) params:params];
}

- (NSDictionary<NSString *, id> * _Nullable)readAttributeClusterRevisionWithParams:(MTRReadParams * _Nullable)params
{
    return [self.device readAttributeWithEndpointID:self.endpointID clusterID:@(MTRClusterIDTypeElectricalMeasurementID) attributeID:@(MTRAttributeIDTypeClusterElectricalMeasurementAttributeClusterRevisionID) params:params];
}

@end

@implementation MTRClusterElectricalMeasurement (Deprecated)

- (instancetype)initWithDevice:(MTRDevice *)device endpoint:(uint16_t)endpoint queue:(dispatch_queue_t)queue
{
    return [self initWithDevice:device endpointID:@(endpoint) queue:queue];
}

- (void)getProfileInfoCommandWithParams:(MTRElectricalMeasurementClusterGetProfileInfoCommandParams * _Nullable)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler
{
    [self getProfileInfoCommandWithParams:params expectedValues:expectedDataValueDictionaries expectedValueInterval:expectedValueIntervalMs completion:
                                                                                                                                                completionHandler];
}
- (void)getProfileInfoCommandWithExpectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler
{
    [self getProfileInfoCommandWithParams:nil expectedValues:expectedValues expectedValueInterval:expectedValueIntervalMs completionHandler:completionHandler];
}
- (void)getMeasurementProfileCommandWithParams:(MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams *)params expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedDataValueDictionaries expectedValueInterval:(NSNumber * _Nullable)expectedValueIntervalMs completionHandler:(MTRStatusCompletion)completionHandler
{
    [self getMeasurementProfileCommandWithParams:params expectedValues:expectedDataValueDictionaries expectedValueInterval:expectedValueIntervalMs completion:
                                                                                                                                                       completionHandler];
}
@end

@interface MTRElectricalMeasurementClusterGetProfileInfoCommandParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@implementation MTRElectricalMeasurementClusterGetProfileInfoResponseCommandParams
- (instancetype)init
{
    if (self = [super init]) {

        _profileCount = @(0);

        _profileIntervalPeriod = @(0);

        _maxNumberOfIntervals = @(0);

        _listOfAttributes = [NSArray array];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRElectricalMeasurementClusterGetProfileInfoResponseCommandParams alloc] init];

    other.profileCount = self.profileCount;
    other.profileIntervalPeriod = self.profileIntervalPeriod;
    other.maxNumberOfIntervals = self.maxNumberOfIntervals;
    other.listOfAttributes = self.listOfAttributes;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: profileCount:%@; profileIntervalPeriod:%@; maxNumberOfIntervals:%@; listOfAttributes:%@; >", NSStringFromClass([self class]), _profileCount, _profileIntervalPeriod, _maxNumberOfIntervals, _listOfAttributes];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED];
    }
    return nil;
}

@end

@implementation MTRElectricalMeasurementClusterGetProfileInfoCommandParams
- (instancetype)init
{
    if (self = [super init]) {
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRElectricalMeasurementClusterGetProfileInfoCommandParams alloc] init];

    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: >", NSStringFromClass([self class])];
    return descriptionString;
}

@end

@implementation MTRElectricalMeasurementClusterGetProfileInfoCommandParams (InternalMethods)

- (CHIP_ERROR)_encodeToTLVReader:(chip::System::PacketBufferTLVReader &)reader
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error
{
    chip::System::PacketBufferTLVReader reader;
    CHIP_ERROR err = [self _encodeToTLVReader:reader];
    if (err != CHIP_NO_ERROR) {
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:err];
        }
        return nil;
    }

    auto decodedObj = MTRDecodeDataValueDictionaryFromCHIPTLV(&reader);
    if (decodedObj == nil) {
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
        }
    }
    return decodedObj;
}
@end

@implementation MTRElectricalMeasurementClusterGetMeasurementProfileResponseCommandParams
- (instancetype)init
{
    if (self = [super init]) {

        _startTime = @(0);

        _status = @(0);

        _profileIntervalPeriod = @(0);

        _numberOfIntervalsDelivered = @(0);

        _attributeId = @(0);

        _intervals = [NSArray array];
        _timedInvokeTimeoutMs = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRElectricalMeasurementClusterGetMeasurementProfileResponseCommandParams alloc] init];

    other.startTime = self.startTime;
    other.status = self.status;
    other.profileIntervalPeriod = self.profileIntervalPeriod;
    other.numberOfIntervalsDelivered = self.numberOfIntervalsDelivered;
    other.attributeId = self.attributeId;
    other.intervals = self.intervals;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: startTime:%@; status:%@; profileIntervalPeriod:%@; numberOfIntervalsDelivered:%@; attributeId:%@; intervals:%@; >", NSStringFromClass([self class]), _startTime, _status, _profileIntervalPeriod, _numberOfIntervalsDelivered, _attributeId, _intervals];
    return descriptionString;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_IMPLEMENTED];
    }
    return nil;
}

@end

@implementation MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams
- (instancetype)init
{
    if (self = [super init]) {

        _attributeId = @(0);

        _startTime = @(0);

        _numberOfIntervals = @(0);
        _timedInvokeTimeoutMs = nil;
        _serverSideProcessingTimeout = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams alloc] init];

    other.attributeId = self.attributeId;
    other.startTime = self.startTime;
    other.numberOfIntervals = self.numberOfIntervals;
    other.timedInvokeTimeoutMs = self.timedInvokeTimeoutMs;
    other.serverSideProcessingTimeout = self.serverSideProcessingTimeout;

    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: attributeId:%@; startTime:%@; numberOfIntervals:%@; >", NSStringFromClass([self class]), _attributeId, _startTime, _numberOfIntervals];
    return descriptionString;
}

@end

@implementation MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams (InternalMethods)

- (CHIP_ERROR)_encodeToTLVReader:(chip::System::PacketBufferTLVReader &)reader
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error
{
    chip::System::PacketBufferTLVReader reader;
    CHIP_ERROR err = [self _encodeToTLVReader:reader];
    if (err != CHIP_NO_ERROR) {
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:err];
        }
        return nil;
    }

    auto decodedObj = MTRDecodeDataValueDictionaryFromCHIPTLV(&reader);
    if (decodedObj == nil) {
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
        }
    }
    return decodedObj;
}
@end

// Note: Add new shims that are not entire cluster removals above the "Clusters that were removed wholesale:" bits
