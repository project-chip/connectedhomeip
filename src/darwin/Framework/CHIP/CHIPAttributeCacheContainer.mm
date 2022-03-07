/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#import "CHIPAttributeCacheContainer_Internal.h"
#import "CHIPDeviceControllerOverXPC+AttributeCache.h"
#import "CHIPDevice_Internal.h"
#import "CHIPError.h"
#import "CHIPLogging.h"

#include <app/InteractionModelEngine.h>
#include <lib/support/ErrorStr.h>
#include <platform/PlatformManager.h>

using namespace chip;

void ContainerAttributeCacheCallback::OnDone()
{
    dispatch_async(DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
        CHIPAttributeCacheContainer * container = attributeCacheContainer;
        if (container) {
            CHIP_LOG_ERROR("Attribute cache read client done for device %llu", container.deviceId);
            if (container.cppReadClient) {
                delete container.cppReadClient;
                container.cppReadClient = nullptr;
            }
            if (container.cppAttributeCache) {
                delete container.cppAttributeCache;
                container.cppAttributeCache = nullptr;
            }
        } else {
            CHIP_LOG_ERROR("Attribute cache read client done for a released cache container");
        }
    });
}

@implementation CHIPAttributeCacheContainer

- (instancetype)init
{
    if ([super init]) {
        _cppAttributeCache = nullptr;
        _cppReadClient = nullptr;
        _attributeCacheCallback = new ContainerAttributeCacheCallback;
        if (_attributeCacheCallback) {
            _attributeCacheCallback->SetContainer(self);
        }
    }
    return self;
}

- (void)dealloc
{
    if (_cppReadClient) {
        delete _cppReadClient;
    }
    if (_cppAttributeCache) {
        delete _cppAttributeCache;
    }
    if (_attributeCacheCallback) {
        delete _attributeCacheCallback;
    }
}

- (void)subscribeWithDeviceController:(CHIPDeviceController *)deviceController
                             deviceId:(uint64_t)deviceId
                          clientQueue:clientQueue
                           completion:(void (^)(NSError * _Nullable error))completion
{
    __auto_type workQueue = DeviceLayer::PlatformMgrImpl().GetWorkQueue();
    __auto_type completionHandler = ^(NSError * _Nullable error) {
        dispatch_async(clientQueue, ^{
            completion(error);
        });
    };
    if ([deviceController isKindOfClass:[CHIPDeviceControllerOverXPC class]]) {
        self.deviceId = deviceId;
        CHIPDeviceControllerOverXPC * xpcDeviceController = (CHIPDeviceControllerOverXPC *) deviceController;
        self.xpcDeviceController = xpcDeviceController;
        [xpcDeviceController subscribeAttributeCacheWithNodeId:deviceId completion:completionHandler];
        return;
    }
    [deviceController
        getConnectedDevice:deviceId
                     queue:workQueue
         completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
             if (error) {
                 CHIP_LOG_ERROR("Error: Failed to get connected device (%llu) for attribute cache: %@", deviceId, error);
                 completionHandler(error);
                 return;
             }
             if (self.cppReadClient) {
                 delete self.cppReadClient;
                 self.cppReadClient = nullptr;
             }
             if (self.cppAttributeCache) {
                 delete self.cppAttributeCache;
             }
             self.cppAttributeCache = new app::AttributeCache(*self.attributeCacheCallback);
             if (!self.cppAttributeCache) {
                 CHIP_LOG_ERROR("Error: Failed to allocate attribute cache for device %llu", deviceId);
                 completionHandler([NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
                 return;
             }

             __auto_type engine = app::InteractionModelEngine::GetInstance();
             __auto_type readClient = new app::ReadClient(engine, [device internalDevice]->GetExchangeManager(),
                 self.cppAttributeCache->GetBufferedCallback(), app::ReadClient::InteractionType::Subscribe);
             if (!readClient) {
                 CHIP_LOG_ERROR("Error: Failed to allocate attribute cache read client for device %llu", deviceId);
                 completionHandler([NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
                 return;
             }
             self.deviceId = deviceId;
             app::ReadPrepareParams readParams([device internalDevice]->GetSecureSession().Value());
             static app::AttributePathParams attributePath(kInvalidEndpointId, kInvalidClusterId, kInvalidAttributeId);
             readParams.mpAttributePathParamsList = &attributePath;
             readParams.mAttributePathParamsListSize = 1;
             readParams.mMaxIntervalCeilingSeconds = 43200;
             __auto_type err = readClient->SendAutoResubscribeRequest(std::move(readParams));
             if (err != CHIP_NO_ERROR) {
                 CHIP_LOG_ERROR("Error: attribute cache subscription failed for device %llu: %s", deviceId, ErrorStr(err));
                 completionHandler([NSError errorWithDomain:CHIPErrorDomain code:err.AsInteger() userInfo:nil]);
                 return;
             }
             self.cppReadClient = readClient;
             CHIP_LOG_DEBUG("Attribute cache subscription succeeded for device %llu", deviceId);
             completionHandler(nil);
         }];
}

static CHIP_ERROR AppendAttibuteValueToArray(
    const chip::app::ConcreteAttributePath & path, chip::app::AttributeCache * cache, NSMutableArray * array)
{
    chip::TLV::TLVReader reader;
    CHIP_ERROR err = cache->Get(path, reader);
    if (err == CHIP_NO_ERROR) {
        id obj = NSObjectFromCHIPTLV(&reader);
        if (obj) {
            [array addObject:@{
                @"endpointId" : [NSNumber numberWithUnsignedShort:path.mEndpointId],
                @"clusterId" : [NSNumber numberWithUnsignedLong:path.mClusterId],
                @"attributeId" : [NSNumber numberWithUnsignedLong:path.mAttributeId],
                @"status" : @0,
                @"data" : obj
            }];
            return CHIP_NO_ERROR;
        }
        CHIP_LOG_ERROR("Error: Cached value could not be converted to generic NSObject");
        [array addObject:@ {
            @"endpointId" : [NSNumber numberWithUnsignedShort:path.mEndpointId],
            @"clusterId" : [NSNumber numberWithUnsignedLong:path.mClusterId],
            @"attributeId" : [NSNumber numberWithUnsignedLong:path.mAttributeId],
            @"status" : [NSNumber numberWithInteger:CHIP_ERROR_DECODE_FAILED.AsInteger()]
        }];
        return CHIP_ERROR_DECODE_FAILED;
    }
    CHIP_LOG_ERROR("Error: Failed to read from attribute cache: %s", err.AsString());
    [array addObject:@ {
        @"endpointId" : [NSNumber numberWithUnsignedShort:path.mEndpointId],
        @"clusterId" : [NSNumber numberWithUnsignedLong:path.mClusterId],
        @"attributeId" : [NSNumber numberWithUnsignedLong:path.mAttributeId],
        @"status" : [NSNumber numberWithInteger:err.AsInteger()]
    }];
    return err;
}

- (void)readAttributeWithEndpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                        attributeId:(NSUInteger)attributeId
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(void (^)(NSArray<NSDictionary<NSString *, id> *> * _Nullable values,
                                        NSError * _Nullable error))completion
{
    __auto_type completionHandler = ^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
        dispatch_async(clientQueue, ^{
            completion(values, error);
        });
    };

    if (self.xpcDeviceController) {
        CHIPDeviceControllerOverXPC * strongController = self.xpcDeviceController;
        if (strongController) {
            [strongController
                readAttributeCacheWithNodeId:self.deviceId
                                  endpointId:endpointId
                                   clusterId:clusterId
                                 attributeId:attributeId
                                  completion:(void (^)(id _Nullable values, NSError * _Nullable error)) completionHandler];
        } else {
            CHIP_LOG_ERROR("Reading attribute cache failed when associated device controller is deleted");
            completionHandler(nil, [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
        }
        return;
    }
    dispatch_async(DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
        if (endpointId == chip::kInvalidEndpointId && clusterId == chip::kInvalidClusterId) {
            CHIP_LOG_ERROR("Error: currently read from attribute cache does not support wildcards for both endpoint and cluster");
            completionHandler(nil, [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeInvalidArgument userInfo:nil]);
            return;
        }

        if (!self.cppAttributeCache) {
            CHIP_LOG_ERROR("Error: No attribute cache available to read from");
            completionHandler(nil, [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
            return;
        }

        NSMutableArray * result = [[NSMutableArray alloc] init];
        CHIP_ERROR err = CHIP_NO_ERROR;
        if (endpointId == chip::kInvalidEndpointId) {
            err = self.cppAttributeCache->ForEachAttribute(
                static_cast<chip::ClusterId>(clusterId), [&](const app::ConcreteAttributePath & path) {
                    if (static_cast<chip::AttributeId>(attributeId) == chip::kInvalidAttributeId
                        || static_cast<chip::AttributeId>(attributeId) == path.mAttributeId) {
                        (void) AppendAttibuteValueToArray(path, self.cppAttributeCache, result);
                    }
                    return CHIP_NO_ERROR;
                });
        } else if (clusterId == chip::kInvalidClusterId) {
            err = self.cppAttributeCache->ForEachCluster(static_cast<chip::EndpointId>(endpointId), [&](chip::ClusterId clusterId) {
                (void) self.cppAttributeCache->ForEachAttribute(
                    static_cast<chip::EndpointId>(endpointId), clusterId, [&](const app::ConcreteAttributePath & path) {
                        if (static_cast<chip::AttributeId>(attributeId) == chip::kInvalidAttributeId
                            || static_cast<chip::AttributeId>(attributeId) == path.mAttributeId) {
                            (void) AppendAttibuteValueToArray(path, self.cppAttributeCache, result);
                        }
                        return CHIP_NO_ERROR;
                    });
                return CHIP_NO_ERROR;
            });
        } else if (attributeId == chip::kInvalidAttributeId) {
            err = self.cppAttributeCache->ForEachAttribute(static_cast<chip::EndpointId>(endpointId),
                static_cast<chip::ClusterId>(clusterId), [&](const app::ConcreteAttributePath & path) {
                    (void) AppendAttibuteValueToArray(path, self.cppAttributeCache, result);
                    return CHIP_NO_ERROR;
                });
        } else {
            app::ConcreteAttributePath path;
            path.mEndpointId = static_cast<chip::EndpointId>(endpointId);
            path.mClusterId = static_cast<chip::ClusterId>(clusterId);
            path.mAttributeId = static_cast<chip::AttributeId>(attributeId);
            err = AppendAttibuteValueToArray(path, self.cppAttributeCache, result);
        }
        if (err == CHIP_NO_ERROR) {
            completionHandler(result, nil);
        } else {
            completionHandler(nil, [NSError errorWithDomain:CHIPErrorDomain code:err.AsInteger() userInfo:nil]);
        }
    });
}

@end
