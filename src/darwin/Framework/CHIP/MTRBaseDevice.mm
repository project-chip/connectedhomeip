/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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
#import <Matter/Matter.h>

#import "MTRAttributeTLVValueDecoder_Internal.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRBaseSubscriptionCallback.h"
#import "MTRCallbackBridgeBase.h"
#import "MTRCluster.h"
#import "MTRClusterStateCacheContainer_Internal.h"
#import "MTRCluster_Internal.h"
#import "MTRDeviceDataValidation.h"
#import "MTRDevice_Internal.h"
#import "MTRError_Internal.h"
#import "MTREventTLVValueDecoder_Internal.h"
#import "MTRFramework.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRSetupPayload_Internal.h"
#import "MTRUtilities.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"
#import "zap-generated/MTRCommandPayloads_Internal.h"

#import "app/ConcreteAttributePath.h"
#import "app/ConcreteCommandPath.h"
#import "app/ConcreteEventPath.h"
#import "app/StatusResponse.h"
#import "lib/core/CHIPError.h"
#import "lib/core/DataModelTypes.h"

#import <app/AttributePathParams.h>
#import <app/BufferedReadCallback.h>
#import <app/ClusterStateCache.h>
#import <app/InteractionModelEngine.h>
#import <app/ReadClient.h>
#import <app/data-model/List.h>
#import <controller/CommissioningWindowOpener.h>
#import <controller/ReadInteraction.h>
#import <controller/WriteInteraction.h>
#import <crypto/CHIPCryptoPAL.h>
#import <setup_payload/SetupPayload.h>
#import <system/SystemClock.h>

#import <memory>

using namespace chip;
using namespace chip::app;
using namespace chip::Protocols::InteractionModel;
using chip::Optional;
using chip::SessionHandle;
using chip::Messaging::ExchangeManager;
using namespace chip::Tracing::DarwinFramework;

NSString * const MTRAttributePathKey = @"attributePath";
NSString * const MTRCommandPathKey = @"commandPath";
NSString * const MTREventPathKey = @"eventPath";
NSString * const MTRDataKey = @"data";
NSString * const MTRErrorKey = @"error";
NSString * const MTRTypeKey = @"type";
NSString * const MTRValueKey = @"value";
NSString * const MTRContextTagKey = @"contextTag";
NSString * const MTRSignedIntegerValueType = @"SignedInteger";
NSString * const MTRUnsignedIntegerValueType = @"UnsignedInteger";
NSString * const MTRBooleanValueType = @"Boolean";
NSString * const MTRUTF8StringValueType = @"UTF8String";
NSString * const MTROctetStringValueType = @"OctetString";
NSString * const MTRFloatValueType = @"Float";
NSString * const MTRDoubleValueType = @"Double";
NSString * const MTRNullValueType = @"Null";
NSString * const MTRStructureValueType = @"Structure";
NSString * const MTRArrayValueType = @"Array";
NSString * const MTREventNumberKey = @"eventNumber";
NSString * const MTREventPriorityKey = @"eventPriority";
NSString * const MTREventTimeTypeKey = @"eventTimeType";
NSString * const MTREventSystemUpTimeKey = @"eventSystemUpTime";
NSString * const MTREventTimestampDateKey = @"eventTimestampDate";
NSString * const MTREventIsHistoricalKey = @"eventIsHistorical";

class MTRDataValueDictionaryCallbackBridge;
class MTRDataValueDictionaryDecodableType;
template <typename DecodableValueType>
class BufferedReadClientCallback;

@interface MTRReadClientContainer : NSObject
@property (nonatomic, readwrite) app::ReadClient * readClientPtr;
@property (nonatomic, readwrite) BufferedReadClientCallback<MTRDataValueDictionaryDecodableType> * callback;
@property (nonatomic, readwrite) app::AttributePathParams * pathParams;
@property (nonatomic, readwrite) app::EventPathParams * eventPathParams;
@property (nonatomic, readwrite) uint64_t deviceID;

- (void)cleanup;
- (void)onDone;
@end

static NSMutableDictionary<NSNumber *, NSMutableArray<MTRReadClientContainer *> *> * readClientContainers;
static NSLock * readClientContainersLock;

static void InitializeReadClientContainers()
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        readClientContainers = [NSMutableDictionary dictionary];
        readClientContainersLock = [[NSLock alloc] init];
    });
}

static void AddReadClientContainer(uint64_t deviceId, MTRReadClientContainer * container)
{
    InitializeReadClientContainers();

    NSNumber * key = [NSNumber numberWithUnsignedLongLong:deviceId];
    [readClientContainersLock lock];
    if (!readClientContainers[key]) {
        readClientContainers[key] = [NSMutableArray array];
    }
    [readClientContainers[key] addObject:container];
    [readClientContainersLock unlock];
}

static void ReinstateReadClientList(NSMutableArray<MTRReadClientContainer *> * readClientList, NSNumber * key,
    dispatch_queue_t queue, dispatch_block_t _Nullable completion)
{
    [readClientContainersLock lock];
    auto existingList = readClientContainers[key];
    if (existingList) {
        [existingList addObjectsFromArray:readClientList];
    } else {
        readClientContainers[key] = readClientList;
    }
    [readClientContainersLock unlock];
    if (completion) {
        dispatch_async(queue, completion);
    }
}

static void PurgeReadClientContainers(
    MTRDeviceController * controller, uint64_t deviceId, dispatch_queue_t queue, void (^_Nullable completion)(void))
{
    InitializeReadClientContainers();

    NSMutableArray<MTRReadClientContainer *> * listToDelete;
    NSNumber * key = [NSNumber numberWithUnsignedLongLong:deviceId];
    [readClientContainersLock lock];
    listToDelete = readClientContainers[key];
    [readClientContainers removeObjectForKey:key];
    [readClientContainersLock unlock];

    // Destroy read clients in the work queue
    [controller
        asyncDispatchToMatterQueue:^() {
            for (MTRReadClientContainer * container in listToDelete) {
                [container cleanup];
            }
            [listToDelete removeAllObjects];
            if (completion) {
                dispatch_async(queue, completion);
            }
        }
        errorHandler:^(NSError * error) {
            // Can't delete things. Just put them back, and hope we
            // can delete them later.
            ReinstateReadClientList(listToDelete, key, queue, completion);
        }];
}

static void PurgeCompletedReadClientContainers(uint64_t deviceId)
{
    InitializeReadClientContainers();

    NSNumber * key = [NSNumber numberWithUnsignedLongLong:deviceId];
    [readClientContainersLock lock];
    NSMutableArray<MTRReadClientContainer *> * array = readClientContainers[key];
    NSUInteger i = 0;
    while (i < [array count]) {
        if (array[i].readClientPtr == nullptr) {
            [array removeObjectAtIndex:i];
            continue;
        }
        i++;
    }
    [readClientContainersLock unlock];
}

static bool CheckMemberOfType(NSDictionary<NSString *, id> * responseValue, NSString * memberName, Class expectedClass,
    NSString * errorMessage, NSError * __autoreleasing * error);
static void LogStringAndReturnError(NSString * errorStr, CHIP_ERROR errorCode, NSError * __autoreleasing * error);
static void LogStringAndReturnError(NSString * errorStr, MTRErrorCode errorCode, NSError * __autoreleasing * error);

@implementation MTRReadClientContainer
- (void)cleanup
{
    if (_readClientPtr) {
        Platform::Delete(_readClientPtr);
        _readClientPtr = nullptr;
    }
    if (_pathParams) {
        static_assert(std::is_trivially_destructible<AttributePathParams>::value, "AttributePathParams destructors won't get run");
        Platform::MemoryFree(_pathParams);
        _pathParams = nullptr;
    }
    if (_eventPathParams) {
        static_assert(std::is_trivially_destructible<EventPathParams>::value, "EventPathParams destructors won't get run");
        Platform::MemoryFree(_eventPathParams);
        _eventPathParams = nullptr;
    }
    if (_callback) {
        Platform::Delete(_callback);
        _callback = nullptr;
    }
}

- (void)onDone
{
    [self cleanup];

    PurgeCompletedReadClientContainers(_deviceID);
}

@end

@implementation MTRBaseDevice

- (instancetype)initWithPASEDevice:(chip::DeviceProxy *)device controller:(MTRDeviceController *)controller
{
    if (self = [super init]) {
        _isPASEDevice = YES;
        _nodeID = device->GetDeviceId();
        _deviceController = controller;
    }
    return self;
}

- (instancetype)initWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller
{
    if (self = [super init]) {
        _isPASEDevice = NO;
        _nodeID = nodeID.unsignedLongLongValue;
        _deviceController = controller;
    }
    return self;
}

+ (MTRBaseDevice *)deviceWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller
{
    // Indirect through the controller to give it a chance to create an
    // MTRBaseDeviceOverXPC instead of just an MTRBaseDevice.
    return [controller baseDeviceForNodeID:nodeID];
}

- (nullable MTRDeviceController_Concrete *)concreteController
{
    auto * controller = self.deviceController;
    if ([controller isKindOfClass:MTRDeviceController_Concrete.class]) {
        return static_cast<MTRDeviceController_Concrete *>(controller);
    }

    return nil;
}

- (MTRTransportType)sessionTransportType
{
    auto * concreteController = self.concreteController;
    if (concreteController == nil) {
        MTR_LOG_ERROR("Unable to determine session transport type for MTRBaseDevice created with an XPC controller");
        return MTRTransportTypeUndefined;
    }
    return [concreteController sessionTransportTypeForDevice:self];
}

- (void)invalidateCASESession
{
    if (self.isPASEDevice) {
        return;
    }

    auto * concreteController = self.concreteController;
    if (concreteController == nil) {
        // Nothing we can do here.
        MTR_LOG_ERROR("Unable invalidate CASE session for MTRBaseDevice created with an XPC controller");
        return;
    }

    [concreteController invalidateCASESessionForNode:@(self.nodeID)];
}

namespace {

class SubscriptionCallback final : public MTRBaseSubscriptionCallback {
public:
    SubscriptionCallback(DataReportCallback attributeReportCallback, DataReportCallback eventReportCallback,
        ErrorCallback errorCallback, MTRDeviceResubscriptionScheduledHandler _Nullable resubscriptionScheduledHandler,
        MTRSubscriptionEstablishedHandler _Nullable subscriptionEstablishedHandler, OnDoneHandler _Nullable onDoneHandler)
        : MTRBaseSubscriptionCallback(attributeReportCallback, eventReportCallback, errorCallback, resubscriptionScheduledHandler,
            subscriptionEstablishedHandler, onDoneHandler)
    {
    }

    void OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus) override;

    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;
};

} // anonymous namespace

- (void)subscribeWithQueue:(dispatch_queue_t)queue
                        params:(MTRSubscribeParams *)params
    clusterStateCacheContainer:(MTRClusterStateCacheContainer * _Nullable)clusterStateCacheContainer
        attributeReportHandler:(MTRDeviceReportHandler _Nullable)attributeReportHandler
            eventReportHandler:(MTRDeviceReportHandler _Nullable)eventReportHandler
                  errorHandler:(void (^)(NSError * error))errorHandler
       subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
       resubscriptionScheduled:(MTRDeviceResubscriptionScheduledHandler _Nullable)resubscriptionScheduled
{
    if (self.isPASEDevice) {
        // We don't support subscriptions over PASE.
        dispatch_async(queue, ^{
            errorHandler([MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    auto * concreteController = self.concreteController;
    if (concreteController == nil) {
        // No subscriptions (or really any MTRBaseDevice use) with XPC controllers.
        MTR_LOG_ERROR("Unable to create subscription for MTRBaseDevice created with an XPC controller");
        dispatch_async(queue, ^{
            errorHandler([MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // Copy params before going async.
    params = [params copy];

    [concreteController getSessionForNode:self.nodeID
                               completion:^(ExchangeManager * _Nullable exchangeManager, const Optional<SessionHandle> & session,
                                   NSError * _Nullable error, NSNumber * _Nullable retryDelay) {
                                   if (error != nil) {
                                       dispatch_async(queue, ^{
                                           errorHandler(error);
                                       });
                                       return;
                                   }

                                   // Wildcard endpoint, cluster, attribute, event.
                                   auto attributePath = std::make_unique<AttributePathParams>();
                                   auto eventPath = std::make_unique<EventPathParams>();
                                   eventPath->mIsUrgentEvent = params.reportEventsUrgently;
                                   ReadPrepareParams readParams(session.Value());
                                   [params toReadPrepareParams:readParams];
                                   readParams.mpAttributePathParamsList = attributePath.get();
                                   readParams.mAttributePathParamsListSize = 1;
                                   readParams.mpEventPathParamsList = eventPath.get();
                                   readParams.mEventPathParamsListSize = 1;

                                   std::unique_ptr<ClusterStateCache> clusterStateCache;
                                   ReadClient::Callback * callbackForReadClient = nullptr;
                                   OnDoneHandler onDoneHandler = nil;

                                   if (clusterStateCacheContainer) {
                                       __weak MTRClusterStateCacheContainer * weakPtr = clusterStateCacheContainer;
                                       onDoneHandler = ^{
                                           // This, like all manipulation of cppClusterStateCache, needs to run on the Matter
                                           // queue.
                                           MTRClusterStateCacheContainer * container = weakPtr;
                                           if (container) {
                                               container.cppClusterStateCache = nullptr;
                                               container.baseDevice = nil;
                                           }
                                       };
                                   }

                                   auto callback = std::make_unique<SubscriptionCallback>(
                                       ^(NSArray * value) {
                                           dispatch_async(queue, ^{
                                               if (attributeReportHandler != nil) {
                                                   attributeReportHandler(value);
                                               }
                                           });
                                       },
                                       ^(NSArray * value) {
                                           dispatch_async(queue, ^{
                                               if (eventReportHandler != nil) {
                                                   eventReportHandler(value);
                                               }
                                           });
                                       },
                                       ^(NSError * error) {
                                           dispatch_async(queue, ^{
                                               errorHandler(error);
                                           });
                                       },
                                       ^(NSError * error, NSNumber * resubscriptionDelay) {
                                           dispatch_async(queue, ^{
                                               if (resubscriptionScheduled != nil) {
                                                   resubscriptionScheduled(error, resubscriptionDelay);
                                               }
                                           });
                                       },
                                       ^(void) {
                                           dispatch_async(queue, ^{
                                               if (subscriptionEstablished != nil) {
                                                   subscriptionEstablished();
                                               }
                                           });
                                       },
                                       onDoneHandler);

                                   if (clusterStateCacheContainer) {
                                       clusterStateCache = std::make_unique<ClusterStateCache>(*callback.get());
                                       callbackForReadClient = &clusterStateCache->GetBufferedCallback();
                                   } else {
                                       callbackForReadClient = &callback->GetBufferedCallback();
                                   }

                                   auto readClient = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(),
                                       exchangeManager, *callbackForReadClient, ReadClient::InteractionType::Subscribe);

                                   CHIP_ERROR err;
                                   if (!params.resubscribeAutomatically) {
                                       err = readClient->SendRequest(readParams);
                                   } else {
                                       // SendAutoResubscribeRequest cleans up the params, even on failure.
                                       attributePath.release();
                                       eventPath.release();
                                       err = readClient->SendAutoResubscribeRequest(std::move(readParams));
                                   }

                                   if (err != CHIP_NO_ERROR) {
                                       dispatch_async(queue, ^{
                                           errorHandler([MTRError errorForCHIPErrorCode:err]);
                                       });

                                       return;
                                   }

                                   if (clusterStateCacheContainer) {
                                       clusterStateCacheContainer.cppClusterStateCache = clusterStateCache.get();
                                       // ClusterStateCache will be deleted when OnDone is called.
                                       callback->AdoptClusterStateCache(std::move(clusterStateCache));
                                       clusterStateCacheContainer.baseDevice = self;
                                   }
                                   // Callback and ReadClient will be deleted when OnDone is called.
                                   callback->AdoptReadClient(std::move(readClient));
                                   callback.release();
                               }];
}

static NSDictionary<NSString *, id> * _MakeDataValueDictionary(NSString * type, id _Nullable value, NSNumber * _Nullable dataVersion)
{
    if (value && dataVersion) {
        return @ { MTRTypeKey : type, MTRValueKey : value, MTRDataVersionKey : dataVersion };
    } else if (value) {
        return @ { MTRTypeKey : type, MTRValueKey : value };
    } else if (dataVersion) {
        return @ { MTRTypeKey : type, MTRDataVersionKey : dataVersion };
    } else {
        return @ { MTRTypeKey : type };
    }
}

// Convert TLV data into data-value dictionary as described in MTRDeviceResponseHandler
NSDictionary<NSString *, id> * _Nullable MTRDecodeDataValueDictionaryFromCHIPTLV(chip::TLV::TLVReader * data, NSNumber * dataVersion)
{
    chip::TLV::TLVType dataTLVType = data->GetType();
    switch (dataTLVType) {
    case chip::TLV::kTLVType_SignedInteger: {
        int64_t val;
        CHIP_ERROR err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV signed integer decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return _MakeDataValueDictionary(MTRSignedIntegerValueType, @(val), dataVersion);
    }
    case chip::TLV::kTLVType_UnsignedInteger: {
        uint64_t val;
        CHIP_ERROR err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV unsigned integer decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return _MakeDataValueDictionary(MTRUnsignedIntegerValueType, @(val), dataVersion);
    }
    case chip::TLV::kTLVType_Boolean: {
        bool val;
        CHIP_ERROR err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV boolean decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return _MakeDataValueDictionary(MTRBooleanValueType, @(val), dataVersion);
    }
    case chip::TLV::kTLVType_FloatingPointNumber: {
        // Try float first
        float floatValue;
        CHIP_ERROR err = data->Get(floatValue);
        if (err == CHIP_NO_ERROR) {
            return _MakeDataValueDictionary(MTRFloatValueType, @(floatValue), dataVersion);
        }
        double val;
        err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV floating point decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return _MakeDataValueDictionary(MTRDoubleValueType, @(val), dataVersion);
    }
    case chip::TLV::kTLVType_UTF8String: {
        CharSpan stringValue;
        CHIP_ERROR err = data->Get(stringValue);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV UTF8String decoding failed", chip::ErrorStr(err));
            return nil;
        }
        NSString * stringObj = AsString(stringValue);
        if (stringObj == nil) {
            MTR_LOG_ERROR("Error(%s): TLV UTF8String value is not actually UTF-8", err.AsString());
            return nil;
        }
        return _MakeDataValueDictionary(MTRUTF8StringValueType, stringObj, dataVersion);
    }
    case chip::TLV::kTLVType_ByteString: {
        ByteSpan bytesValue;
        CHIP_ERROR err = data->Get(bytesValue);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV ByteString decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return _MakeDataValueDictionary(MTROctetStringValueType, AsData(bytesValue), dataVersion);
    }
    case chip::TLV::kTLVType_Null: {
        return _MakeDataValueDictionary(MTRNullValueType, nil, dataVersion);
    }
    case chip::TLV::kTLVType_Structure:
    case chip::TLV::kTLVType_Array: {
        NSString * typeName;
        switch (dataTLVType) {
        case chip::TLV::kTLVType_Structure:
            typeName = MTRStructureValueType;
            break;
        case chip::TLV::kTLVType_Array:
            typeName = MTRArrayValueType;
            break;
        default:
            typeName = @"Unsupported";
            break;
        }
        chip::TLV::TLVType tlvType;
        CHIP_ERROR err = data->EnterContainer(tlvType);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV container entering failed", chip::ErrorStr(err));
            return nil;
        }
        NSMutableArray * array = [[NSMutableArray alloc] init];
        while ((err = data->Next()) == CHIP_NO_ERROR) {
            chip::TLV::Tag tag = data->GetTag();
            id value = MTRDecodeDataValueDictionaryFromCHIPTLV(data);
            if (value == nullptr) {
                MTR_LOG_ERROR("Error when decoding TLV container of type %s", typeName.UTF8String);
                return nil;
            }
            NSMutableDictionary * arrayElement = [NSMutableDictionary dictionary];
            [arrayElement setObject:value forKey:MTRDataKey];
            if (dataTLVType == chip::TLV::kTLVType_Structure) {
                uint64_t tagNum;
                if (IsContextTag(tag)) {
                    tagNum = TagNumFromTag(tag);
                } else if (IsProfileTag(tag)) {
                    uint64_t profile = ProfileIdFromTag(tag);
                    tagNum = (profile << kProfileIdShift) | TagNumFromTag(tag);
                } else {
                    MTR_LOG_ERROR("Skipping unknown tag type when decoding TLV structure.");
                    continue;
                }
                [arrayElement setObject:[NSNumber numberWithUnsignedLongLong:tagNum] forKey:MTRContextTagKey];
            }
            [array addObject:arrayElement];
        }
        if (err != CHIP_END_OF_TLV) {
            MTR_LOG_ERROR("Error(%s): TLV container decoding failed", chip::ErrorStr(err));
            return nil;
        }
        err = data->ExitContainer(tlvType);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV container exiting failed", chip::ErrorStr(err));
            return nil;
        }
        return _MakeDataValueDictionary(typeName, array, dataVersion);
    }
    default:
        MTR_LOG_ERROR("Error: Unsupported TLV type for conversion: %u", static_cast<unsigned>(data->GetType()));
        return nil;
    }
}

// writer is allowed to be null to just validate the incoming object without
// actually encoding.
static CHIP_ERROR MTREncodeTLVFromDataValueDictionaryInternal(id object, chip::TLV::TLVWriter * writer, chip::TLV::Tag tag)
{
    if (![object isKindOfClass:[NSDictionary class]]) {
        MTR_LOG_ERROR("Error: Unsupported object to encode: %@", [object class]);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    NSString * typeName = ((NSDictionary *) object)[MTRTypeKey];
    id value = ((NSDictionary *) object)[MTRValueKey];
    if (![typeName isKindOfClass:[NSString class]]) {
        MTR_LOG_ERROR("Error: Object to encode has no MTRTypeKey: %@", object);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if ([typeName isEqualToString:MTRSignedIntegerValueType]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt signed integer type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer ? writer->Put(tag, [value longLongValue]) : CHIP_NO_ERROR;
    }
    if ([typeName isEqualToString:MTRUnsignedIntegerValueType]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt unsigned integer type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer ? writer->Put(tag, [value unsignedLongLongValue]) : CHIP_NO_ERROR;
    }
    if ([typeName isEqualToString:MTRBooleanValueType]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt boolean type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer ? writer->Put(tag, static_cast<bool>([value boolValue])) : CHIP_NO_ERROR;
    }
    if ([typeName isEqualToString:MTRFloatValueType]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt float type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer ? writer->Put(tag, [value floatValue]) : CHIP_NO_ERROR;
    }
    if ([typeName isEqualToString:MTRDoubleValueType]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt double type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer ? writer->Put(tag, [value doubleValue]) : CHIP_NO_ERROR;
    }
    if ([typeName isEqualToString:MTRNullValueType]) {
        return writer ? writer->PutNull(tag) : CHIP_NO_ERROR;
    }
    if ([typeName isEqualToString:MTRUTF8StringValueType]) {
        if (![value isKindOfClass:[NSString class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt UTF8 string type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer ? writer->PutString(tag, AsCharSpan(value)) : CHIP_NO_ERROR;
    }
    if ([typeName isEqualToString:MTROctetStringValueType]) {
        if (![value isKindOfClass:[NSData class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt octet string type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer ? writer->Put(tag, AsByteSpan(value)) : CHIP_NO_ERROR;
    }
    if ([typeName isEqualToString:MTRStructureValueType]) {
        if (![value isKindOfClass:[NSArray class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt structure type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        TLV::TLVType outer = TLV::kTLVType_NotSpecified;
        if (writer) {
            ReturnErrorOnFailure(writer->StartContainer(tag, chip::TLV::kTLVType_Structure, outer));
        }
        for (id element in value) {
            if (![element isKindOfClass:[NSDictionary class]]) {
                MTR_LOG_ERROR("Error: Structure element to encode has corrupt type: %@", [element class]);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            id elementTag = element[MTRContextTagKey];
            id elementValue = element[MTRDataKey];
            if (!elementTag || !elementValue) {
                MTR_LOG_ERROR("Error: Structure element to encode has corrupt value: %@", element);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            if (![elementTag isKindOfClass:NSNumber.class]) {
                MTR_LOG_ERROR("Error: Structure element to encode has corrupt tag type: %@", [elementTag class]);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            // Our tag might actually be a profile tag.
            uint64_t tagValue = [elementTag unsignedLongLongValue];
            TLV::Tag tag;
            if (tagValue > UINT8_MAX) {
                tag = TLV::ProfileTag(tagValue >> kProfileIdShift,
                    (tagValue & ((1ull << kProfileIdShift) - 1)));
            } else {
                tag = TLV::ContextTag(static_cast<uint8_t>(tagValue));
            }
            ReturnErrorOnFailure(
                MTREncodeTLVFromDataValueDictionaryInternal(elementValue, writer, tag));
        }
        if (writer) {
            ReturnErrorOnFailure(writer->EndContainer(outer));
        }
        return CHIP_NO_ERROR;
    }
    if ([typeName isEqualToString:MTRArrayValueType]) {
        if (![value isKindOfClass:[NSArray class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt array type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        TLV::TLVType outer = TLV::kTLVType_NotSpecified;
        if (writer) {
            ReturnErrorOnFailure(writer->StartContainer(tag, chip::TLV::kTLVType_Array, outer));
        }
        for (id element in value) {
            if (![element isKindOfClass:[NSDictionary class]]) {
                MTR_LOG_ERROR("Error: Array element to encode has corrupt type: %@", [element class]);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            id elementValue = element[MTRDataKey];
            if (!elementValue) {
                MTR_LOG_ERROR("Error: Array element to encode has corrupt value: %@", element);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            ReturnErrorOnFailure(MTREncodeTLVFromDataValueDictionaryInternal(elementValue, writer, chip::TLV::AnonymousTag()));
        }
        if (writer) {
            ReturnErrorOnFailure(writer->EndContainer(outer));
        }
        return CHIP_NO_ERROR;
    }
    MTR_LOG_ERROR("Error: Unsupported type to encode: %@", typeName);
    return CHIP_ERROR_INVALID_ARGUMENT;
}

static CHIP_ERROR MTREncodeTLVFromDataValueDictionary(id object, chip::TLV::TLVWriter * writer, chip::TLV::Tag tag)
{
    CHIP_ERROR err = MTREncodeTLVFromDataValueDictionaryInternal(object, writer, tag);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Failed to encode to TLV: %@", object);
    }
    return err;
}

NSData * _Nullable MTREncodeTLVFromDataValueDictionary(NSDictionary<NSString *, id> * value, NSError * __autoreleasing * error)
{
    // A single data item cannot be bigger than a packet, so just use 1200 bytes
    // as the max size of our buffer.  This assumes that lists will not be
    // passed as-is to this method but will get chunked, with each list item
    // passed to this method separately.
    uint8_t buffer[1200];
    TLV::TLVWriter writer;
    writer.Init(buffer);

    CHIP_ERROR err = MTREncodeTLVFromDataValueDictionary(value, &writer, TLV::AnonymousTag());
    if (err != CHIP_NO_ERROR) {
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:err];
        }
        return nil;
    }

    return AsData(ByteSpan(buffer, writer.GetLengthWritten()));
}

BOOL MTRDataValueDictionaryIsWellFormed(MTRDeviceDataValueDictionary value)
{
    return MTREncodeTLVFromDataValueDictionary(value, nullptr, TLV::AnonymousTag()) == CHIP_NO_ERROR;
}

// Callback type to pass data value as an NSObject
typedef void (*MTRDataValueDictionaryCallback)(void * context, id value);

// Rename to be generic for decode and encode
class MTRDataValueDictionaryDecodableType {
public:
    MTRDataValueDictionaryDecodableType()
        : decodedObj(nil)
    {
    }
    MTRDataValueDictionaryDecodableType(id obj)
        : decodedObj(obj)
    {
    }

    CHIP_ERROR Decode(chip::TLV::TLVReader & data)
    {
        decodedObj = MTRDecodeDataValueDictionaryFromCHIPTLV(&data);
        if (decodedObj == nil) {
            MTR_LOG_ERROR("Error: Failed to get value from TLV data for attribute reading response");
        }
        return (decodedObj) ? CHIP_NO_ERROR : CHIP_ERROR_DECODE_FAILED;
    }

    CHIP_ERROR Encode(chip::TLV::TLVWriter & writer, chip::TLV::Tag tag) const
    {
        return MTREncodeTLVFromDataValueDictionary(decodedObj, &writer, tag);
    }

    static constexpr bool kIsFabricScoped = false;

    static bool MustUseTimedInvoke() { return false; }

    NSDictionary<NSString *, id> * _Nullable GetDecodedObject() const { return decodedObj; }

private:
    NSDictionary<NSString *, id> * _Nullable decodedObj;
};

// Callback bridge for MTRDataValueDictionaryCallback
class MTRDataValueDictionaryCallbackBridge : public MTRCallbackBridge<MTRDataValueDictionaryCallback> {
public:
    MTRDataValueDictionaryCallbackBridge(dispatch_queue_t queue, MTRDeviceResponseHandler handler, MTRActionBlock action)
        : MTRCallbackBridge<MTRDataValueDictionaryCallback>(queue, handler, action, OnSuccessFn) {};

    static void OnSuccessFn(void * context, id value) { DispatchSuccess(context, value); }
};

template <typename DecodableValueType>
class BufferedReadClientCallback final : public app::ReadClient::Callback {
public:
    using OnSuccessAttributeCallbackType
        = std::function<void(const ConcreteDataAttributePath & aPath, const DecodableValueType & aData)>;
    using OnSuccessEventCallbackType = std::function<void(const EventHeader & aEventHeader, const DecodableValueType & aData)>;
    using OnErrorCallbackType = std::function<void(
        const app::ConcreteAttributePath * attributePath, const app::ConcreteEventPath * eventPath, CHIP_ERROR aError)>;
    using OnDoneCallbackType = std::function<void(BufferedReadClientCallback * callback)>;
    using OnSubscriptionEstablishedCallbackType = std::function<void()>;
    using OnDeviceResubscriptionScheduledCallbackType = std::function<void(NSError * error, NSNumber * resubscriptionDelay)>;

    BufferedReadClientCallback(app::AttributePathParams * aAttributePathParamsList, size_t aAttributePathParamsSize,
        app::EventPathParams * aEventPathParamsList, size_t aEventPathParamsSize,
        OnSuccessAttributeCallbackType aOnAttributeSuccess, OnSuccessEventCallbackType aOnEventSuccess,
        OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone,
        OnSubscriptionEstablishedCallbackType aOnSubscriptionEstablished = nullptr,
        OnDeviceResubscriptionScheduledCallbackType aOnDeviceResubscriptionScheduled = nullptr)
        : mAttributePathParamsList(aAttributePathParamsList)
        , mAttributePathParamsSize(aAttributePathParamsSize)
        , mEventPathParamsList(aEventPathParamsList)
        , mEventPathParamsSize(aEventPathParamsSize)
        , mOnAttributeSuccess(aOnAttributeSuccess)
        , mOnEventSuccess(aOnEventSuccess)
        , mOnError(aOnError)
        , mOnDone(aOnDone)
        , mOnSubscriptionEstablished(aOnSubscriptionEstablished)
        , mOnDeviceResubscriptionScheduled(aOnDeviceResubscriptionScheduled)
        , mBufferedReadAdapter(*this)
    {
    }

    ~BufferedReadClientCallback()
    {
        // Ensure we release the ReadClient before we tear down anything else,
        // so it can call our OnDeallocatePaths properly.
        mReadClient = nullptr;
    }

    app::BufferedReadCallback & GetBufferedCallback() { return mBufferedReadAdapter; }

    void AdoptReadClient(Platform::UniquePtr<app::ReadClient> aReadClient) { mReadClient = std::move(aReadClient); }

private:
    void OnAttributeData(
        const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const app::StatusIB & aStatus) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        DecodableValueType value;

        VerifyOrExit(mOnAttributeSuccess != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrExit(mAttributePathParamsList != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        //
        // We shouldn't be getting list item operations in the provided path since that should be handled by the buffered read
        // callback. If we do, that's a bug.
        //
        VerifyOrDie(!aPath.IsListItemOperation());

        VerifyOrExit(
            std::find_if(mAttributePathParamsList, mAttributePathParamsList + mAttributePathParamsSize,
                [aPath](app::AttributePathParams & pathParam) -> bool { return pathParam.IsAttributePathSupersetOf(aPath); })
                != mAttributePathParamsList + mAttributePathParamsSize,
            err = CHIP_ERROR_SCHEMA_MISMATCH);

        VerifyOrExit(aStatus.IsSuccess(), err = aStatus.ToChipError());

        VerifyOrExit(apData != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        SuccessOrExit(err = app::DataModel::Decode(*apData, value));

        mOnAttributeSuccess(aPath, value);

    exit:
        if (err != CHIP_NO_ERROR) {
            mOnError(&aPath, nullptr, err);
        }
    }

    void OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        DecodableValueType value;

        VerifyOrExit(mOnEventSuccess != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrExit(mEventPathParamsList != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrExit(std::find_if(mEventPathParamsList, mEventPathParamsList + mEventPathParamsSize,
                         [aEventHeader](app::EventPathParams & pathParam) -> bool {
                             return pathParam.IsEventPathSupersetOf(aEventHeader.mPath);
                         })
                != mEventPathParamsList + mEventPathParamsSize,
            err = CHIP_ERROR_SCHEMA_MISMATCH);

        VerifyOrExit(apStatus == nullptr, err = apStatus->ToChipError());

        VerifyOrExit(apData != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        SuccessOrExit(err = app::DataModel::Decode(*apData, value));

        mOnEventSuccess(aEventHeader, value);

    exit:
        if (err != CHIP_NO_ERROR) {
            mOnError(nullptr, &aEventHeader.mPath, err);
        }
    }

    void OnError(CHIP_ERROR aError) override { mOnError(nullptr, nullptr, aError); }

    void OnDone(ReadClient *) override { mOnDone(this); }

    void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override
    {
        if (mOnSubscriptionEstablished) {
            mOnSubscriptionEstablished();
        }
    }

    CHIP_ERROR OnResubscriptionNeeded(ReadClient * apReadClient, CHIP_ERROR aTerminationCause) override
    {
        CHIP_ERROR err = ReadClient::Callback::OnResubscriptionNeeded(apReadClient, aTerminationCause);
        ReturnErrorOnFailure(err);

        if (mOnDeviceResubscriptionScheduled != nullptr) {
            auto callback = mOnDeviceResubscriptionScheduled;
            auto error = [MTRError errorForCHIPErrorCode:aTerminationCause];
            auto delayMs = @(apReadClient->ComputeTimeTillNextSubscription());
            callback(error, delayMs);
        }
        return CHIP_NO_ERROR;
    }

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override {}

    OnSuccessAttributeCallbackType mOnAttributeSuccess;
    OnSuccessEventCallbackType mOnEventSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;
    OnSubscriptionEstablishedCallbackType mOnSubscriptionEstablished;
    OnDeviceResubscriptionScheduledCallbackType mOnDeviceResubscriptionScheduled;
    app::BufferedReadCallback mBufferedReadAdapter;
    Platform::UniquePtr<app::ReadClient> mReadClient;
    app::AttributePathParams * mAttributePathParamsList;
    app::EventPathParams * mEventPathParamsList;
    size_t mAttributePathParamsSize;
    size_t mEventPathParamsSize;
};

- (void)readAttributesWithEndpointID:(NSNumber * _Nullable)endpointID
                           clusterID:(NSNumber * _Nullable)clusterID
                         attributeID:(NSNumber * _Nullable)attributeID
                              params:(MTRReadParams * _Nullable)params
                               queue:(dispatch_queue_t)queue
                          completion:(MTRDeviceResponseHandler)completion
{
    NSArray<MTRAttributeRequestPath *> * attributePaths = [NSArray
        arrayWithObject:[MTRAttributeRequestPath requestPathWithEndpointID:endpointID clusterID:clusterID attributeID:attributeID]];
    [self readAttributePaths:attributePaths eventPaths:nil params:params queue:queue completion:completion];
}

- (void)_readKnownAttributeWithEndpointID:(NSNumber *)endpointID
                                clusterID:(NSNumber *)clusterID
                              attributeID:(NSNumber *)attributeID
                                   params:(MTRReadParams * _Nullable)params
                                    queue:(dispatch_queue_t)queue
                               completion:(void (^)(id _Nullable value, NSError * _Nullable error))completion
{
    auto * attributePath = [MTRAttributePath attributePathWithEndpointID:endpointID clusterID:clusterID attributeID:attributeID];

    auto innerCompletion = ^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
        if (error != nil) {
            completion(nil, error);
            return;
        }

        // Preserving the old behavior: we don't fail on multiple reports, but
        // just report the first one.
        if (values.count == 0) {
            completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:nil]);
            return;
        }

        NSDictionary<NSString *, id> * value = values[0];
        NSError * initError;
        auto * report = [[MTRAttributeReport alloc] initWithResponseValue:value error:&initError];
        if (initError != nil) {
            completion(nil, initError);
            return;
        }

        if (![report.path isEqual:attributePath]) {
            // For some reason the server returned data for the wrong
            // attribute, even though it happened to decode to our type.
            completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:nil]);
            return;
        }

        completion(report.value, report.error);
    };

    [self readAttributesWithEndpointID:endpointID
                             clusterID:clusterID
                           attributeID:attributeID
                                params:params
                                 queue:queue
                            completion:innerCompletion];
}

- (void)readAttributePaths:(NSArray<MTRAttributeRequestPath *> * _Nullable)attributePaths
                eventPaths:(NSArray<MTREventRequestPath *> * _Nullable)eventPaths
                    params:(MTRReadParams * _Nullable)params
                     queue:(dispatch_queue_t)queue
                completion:(MTRDeviceResponseHandler)completion
{
    MTR_LOG("%@ readAttributePaths: %@, eventPaths: %@", self, attributePaths, eventPaths);

    [self readAttributePaths:attributePaths eventPaths:eventPaths params:params includeDataVersion:NO queue:queue completion:completion];
}

- (void)readAttributePaths:(NSArray<MTRAttributeRequestPath *> * _Nullable)attributePaths
                eventPaths:(NSArray<MTREventRequestPath *> * _Nullable)eventPaths
                    params:(MTRReadParams * _Nullable)params
        includeDataVersion:(BOOL)includeDataVersion
                     queue:(dispatch_queue_t)queue
                completion:(MTRDeviceResponseHandler)completion
{
    // NOTE: Do not log the read here.  This is called ether from
    // readAttributePaths:eventPaths:params:queue:completion: or MTRDevice, both
    // of which already log, and we want to be able to tell the two codepaths apart.
    if ((attributePaths == nil || [attributePaths count] == 0) && (eventPaths == nil || [eventPaths count] == 0)) {
        // No paths, just return an empty array.
        dispatch_async(queue, ^{
            completion(@[], nil);
        });
        return;
    }

    NSArray<MTRAttributeRequestPath *> * attributes = nil;
    if (attributePaths != nil) {
        attributes = [[NSArray alloc] initWithArray:attributePaths copyItems:YES];
    }

    NSArray<MTREventRequestPath *> * events = nil;
    if (eventPaths != nil) {
        events = [[NSArray alloc] initWithArray:eventPaths copyItems:YES];
    }
    params = (params == nil) ? nil : [params copy];
    auto * bridge = new MTRDataValueDictionaryCallbackBridge(queue, completion,
        ^(ExchangeManager & exchangeManager, const SessionHandle & session, MTRDataValueDictionaryCallback successCb,
            MTRErrorCallback failureCb, MTRCallbackBridgeBase * bridge) {
            // interactionStatus tracks whether the whole read interaction has failed.
            //
            // Make sure interactionStatus survives even if this block scope is destroyed.
            auto interactionStatus = std::make_shared<CHIP_ERROR>(CHIP_NO_ERROR);

            auto resultArray = [[NSMutableArray alloc] init];
            auto onAttributeSuccessCb
                = [resultArray, includeDataVersion](const ConcreteDataAttributePath & aAttributePath, const MTRDataValueDictionaryDecodableType & aData) {
                      // TODO: move this logic into MTRDataValueDictionaryDecodableType
                      if (includeDataVersion && aAttributePath.mDataVersion.HasValue()) {
                          NSDictionary * dataValue = aData.GetDecodedObject();
                          [resultArray addObject:@{
                              MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:aAttributePath],
                              MTRDataKey : _MakeDataValueDictionary(dataValue[MTRTypeKey], dataValue[MTRValueKey], @(aAttributePath.mDataVersion.Value()))
                          }];
                      } else {
                          [resultArray addObject:@ {
                              MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:aAttributePath],
                              MTRDataKey : aData.GetDecodedObject()
                          }];
                      }
                  };

            auto onEventSuccessCb
                = [resultArray](const EventHeader & aEventHeader, const MTRDataValueDictionaryDecodableType & aData) {
                      [resultArray addObject:[MTRBaseDevice eventReportForHeader:aEventHeader andData:aData.GetDecodedObject()]];
                  };

            auto onFailureCb = [resultArray, interactionStatus](const app::ConcreteAttributePath * aAttributePath,
                                   const app::ConcreteEventPath * aEventPath, CHIP_ERROR aError) {
                if (aAttributePath != nullptr) {
                    [resultArray addObject:@ {
                        MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:*aAttributePath],
                        MTRErrorKey : [MTRError errorForCHIPErrorCode:aError]
                    }];
                } else if (aEventPath != nullptr) {
                    [resultArray addObject:@ {
                        MTREventPathKey : [[MTREventPath alloc] initWithPath:*aEventPath],
                        MTRErrorKey : [MTRError errorForCHIPErrorCode:aError]
                    }];
                } else {
                    // This will only happen once per read interaction, and
                    // after that there will be no more calls to onFailureCb or
                    // onSuccessCb.
                    *interactionStatus = aError;
                }
            };

            Platform::ScopedMemoryBuffer<AttributePathParams> attributePathParamsList;
            Platform::ScopedMemoryBuffer<EventPathParams> eventPathParamsList;

            if (attributes != nil) {
                size_t count = 0;
                VerifyOrReturnError(attributePathParamsList.Calloc([attributes count]), CHIP_ERROR_NO_MEMORY);
                for (MTRAttributeRequestPath * attribute in attributes) {
                    [attribute convertToAttributePathParams:attributePathParamsList[count++]];
                }
            }

            if (events != nil) {
                size_t count = 0;
                VerifyOrReturnError(eventPathParamsList.Calloc([events count]), CHIP_ERROR_NO_MEMORY);
                for (MTREventRequestPath * event in events) {
                    [event convertToEventPathParams:eventPathParamsList[count++]];
                }
            }

            app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
            CHIP_ERROR err = CHIP_NO_ERROR;

            chip::app::ReadPrepareParams readParams(session);
            [params toReadPrepareParams:readParams];
            readParams.mpAttributePathParamsList = attributePathParamsList.Get();
            readParams.mAttributePathParamsListSize = [attributes count];
            readParams.mpEventPathParamsList = eventPathParamsList.Get();
            readParams.mEventPathParamsListSize = [events count];

            AttributePathParams * attributePathParamsListToFree = attributePathParamsList.Get();
            EventPathParams * eventPathParamsListToFree = eventPathParamsList.Get();

            auto onDone
                = [resultArray, interactionStatus, bridge, successCb, failureCb, attributePathParamsListToFree,
                      eventPathParamsListToFree](BufferedReadClientCallback<MTRDataValueDictionaryDecodableType> * callback) {
                      if (*interactionStatus != CHIP_NO_ERROR) {
                          // Failure
                          failureCb(bridge, *interactionStatus);
                      } else {
                          // Success
                          successCb(bridge, resultArray);
                      }
                      if (attributePathParamsListToFree != nullptr) {
                          Platform::MemoryFree(attributePathParamsListToFree);
                      }
                      if (eventPathParamsListToFree != nullptr) {
                          Platform::MemoryFree(eventPathParamsListToFree);
                      }
                      chip::Platform::Delete(callback);
                  };

            auto callback = chip::Platform::MakeUnique<BufferedReadClientCallback<MTRDataValueDictionaryDecodableType>>(
                attributePathParamsList.Get(), readParams.mAttributePathParamsListSize, eventPathParamsList.Get(),
                readParams.mEventPathParamsListSize, onAttributeSuccessCb, onEventSuccessCb, onFailureCb, onDone, nullptr);
            VerifyOrReturnError(callback != nullptr, CHIP_ERROR_NO_MEMORY);

            auto readClient = chip::Platform::MakeUnique<app::ReadClient>(
                engine, &exchangeManager, callback->GetBufferedCallback(), chip::app::ReadClient::InteractionType::Read);
            VerifyOrReturnError(readClient != nullptr, CHIP_ERROR_NO_MEMORY);

            err = readClient->SendRequest(readParams);

            if (err != CHIP_NO_ERROR) {
                return err;
            }

            //
            // At this point, we'll get a callback through the OnDone callback above regardless of success or failure
            // of the read operation to permit us to free up the callback object. So, release ownership of the callback
            // object now to prevent it from being reclaimed at the end of this scoped block.
            //
            callback->AdoptReadClient(std::move(readClient));
            callback.release();
            IgnoreUnusedVariable(attributePathParamsList.Release());
            IgnoreUnusedVariable(eventPathParamsList.Release());
            return err;
        });
    std::move(*bridge).DispatchAction(self);
}

- (void)writeAttributeWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                         attributeID:(NSNumber *)attributeID
                               value:(id)value
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
                               queue:(dispatch_queue_t)queue
                          completion:(MTRDeviceResponseHandler)completion
{
    MTR_LOG("%@ write %@ 0x%llx 0x%llx: %@", self, endpointID, clusterID.unsignedLongLongValue, attributeID.unsignedLongLongValue, value);

    [self _writeAttributeWithEndpointID:endpointID clusterID:clusterID attributeID:attributeID value:value timedWriteTimeout:timeoutMs queue:queue completion:completion];
}

- (void)_writeAttributeWithEndpointID:(NSNumber *)endpointID
                            clusterID:(NSNumber *)clusterID
                          attributeID:(NSNumber *)attributeID
                                value:(id)value
                    timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
                                queue:(dispatch_queue_t)queue
                           completion:(MTRDeviceResponseHandler)completion
{
    auto * bridge = new MTRDataValueDictionaryCallbackBridge(queue, completion,
        ^(ExchangeManager & exchangeManager, const SessionHandle & session, MTRDataValueDictionaryCallback successCb,
            MTRErrorCallback failureCb, MTRCallbackBridgeBase * bridge) {
            // Controller::WriteAttribute guarantees that there will be exactly one call to either the success callback or the
            // failure callback, for a non-group session.
            auto onSuccessCb = [successCb, bridge](const app::ConcreteAttributePath & attribPath) {
                auto resultArray = @[ @ { MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:attribPath] } ];
                successCb(bridge, resultArray);
            };

            auto onFailureCb = [failureCb, bridge](
                                   const app::ConcreteAttributePath * attribPath, CHIP_ERROR aError) { failureCb(bridge, aError); };

            // To handle list chunking properly, we have to convert lists into
            // DataModel::List here, because that's special-cased in
            // WriteClient.
            if (![value isKindOfClass:NSDictionary.class]) {
                MTR_LOG_ERROR("Error: Unsupported object to write as attribute value: %@", value);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            NSDictionary<NSString *, id> * dataValue = value;
            NSString * typeName = dataValue[MTRTypeKey];
            if (![typeName isKindOfClass:NSString.class]) {
                MTR_LOG_ERROR("Error: Object to encode is corrupt: %@", dataValue);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            if (![typeName isEqualToString:MTRArrayValueType]) {
                return chip::Controller::WriteAttribute<MTRDataValueDictionaryDecodableType>(session,
                    static_cast<chip::EndpointId>([endpointID unsignedShortValue]),
                    static_cast<chip::ClusterId>([clusterID unsignedLongValue]),
                    static_cast<chip::AttributeId>([attributeID unsignedLongValue]), MTRDataValueDictionaryDecodableType(value),
                    onSuccessCb, onFailureCb, (timeoutMs == nil) ? NullOptional : Optional<uint16_t>([timeoutMs unsignedShortValue]));
            }

            // Now we are dealing with a list.
            NSArray<NSDictionary<NSString *, id> *> * arrayValue = value[MTRValueKey];
            if (![arrayValue isKindOfClass:NSArray.class]) {
                MTR_LOG_ERROR("Error: Object to encode claims to be a list but isn't: %@", arrayValue);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            std::vector<MTRDataValueDictionaryDecodableType> encodableVector;
            encodableVector.reserve(arrayValue.count);

            for (NSDictionary<NSString *, id> * arrayItem in arrayValue) {
                if (![arrayItem isKindOfClass:NSDictionary.class]) {
                    MTR_LOG_ERROR("Error: Can't encode corrupt list: %@", arrayValue);
                    return CHIP_ERROR_INVALID_ARGUMENT;
                }

                encodableVector.push_back(MTRDataValueDictionaryDecodableType(arrayItem[MTRDataKey]));
            }

            DataModel::List<MTRDataValueDictionaryDecodableType> encodableList(encodableVector.data(), encodableVector.size());
            return chip::Controller::WriteAttribute<DataModel::List<MTRDataValueDictionaryDecodableType>>(session,
                static_cast<chip::EndpointId>([endpointID unsignedShortValue]),
                static_cast<chip::ClusterId>([clusterID unsignedLongValue]),
                static_cast<chip::AttributeId>([attributeID unsignedLongValue]), encodableList,
                onSuccessCb, onFailureCb, (timeoutMs == nil) ? NullOptional : Optional<uint16_t>([timeoutMs unsignedShortValue]));
        });
    std::move(*bridge).DispatchAction(self);
}

class NSObjectCommandCallback final : public app::CommandSender::Callback {
public:
    using OnSuccessCallbackType
        = std::function<void(const app::ConcreteCommandPath &, const app::StatusIB &, const MTRDataValueDictionaryDecodableType &)>;
    using OnErrorCallbackType = std::function<void(CHIP_ERROR aError)>;
    using OnDoneCallbackType = std::function<void(app::CommandSender * commandSender)>;

    /*
     * Constructor that takes in success, failure and onDone callbacks.
     *
     * The latter can be provided later through the SetOnDoneCallback below in cases where the
     * TypedCommandCallback object needs to be created first before it can be passed in as a closure
     * into a hypothetical OnDoneCallback function.
     */
    NSObjectCommandCallback(chip::ClusterId clusterId, chip::CommandId commandId, OnSuccessCallbackType aOnSuccess,
        OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone = {})
        : mOnSuccess(aOnSuccess)
        , mOnError(aOnError)
        , mOnDone(aOnDone)
        , mClusterId(clusterId)
        , mCommandId(commandId)
    {
    }

    void SetOnDoneCallback(OnDoneCallbackType callback) { mOnDone = callback; }

private:
    void OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aCommandPath,
        const app::StatusIB & aStatus, TLV::TLVReader * aReader) override;

    void OnError(const app::CommandSender * apCommandSender, CHIP_ERROR aError) override
    {
        if (mCalledCallback) {
            return;
        }
        mCalledCallback = true;

        mOnError(aError);
    }

    void OnDone(app::CommandSender * apCommandSender) override
    {
        if (!mCalledCallback) {
            // This can happen if the server sends a response with an empty
            // InvokeResponses list.  Since we are not sending wildcard command
            // paths, that's not a valid response and we should treat it as an
            // error.  Use the error we would have gotten if we in fact expected
            // a nonempty list.
            OnError(apCommandSender, CHIP_END_OF_TLV);
        }

        mOnDone(apCommandSender);
    }

    OnSuccessCallbackType mOnSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;
    chip::ClusterId mClusterId;
    // Id of the command we send.
    chip::CommandId mCommandId;
    bool mCalledCallback = false;
};

void NSObjectCommandCallback::OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aCommandPath,
    const app::StatusIB & aStatus, TLV::TLVReader * aReader)
{
    if (mCalledCallback) {
        return;
    }
    mCalledCallback = true;

    MTRDataValueDictionaryDecodableType response;
    CHIP_ERROR err = CHIP_NO_ERROR;

    //
    // Validate that the data response we received matches what we expect in terms of its cluster and command IDs.
    //
    VerifyOrExit(aCommandPath.mClusterId == mClusterId, err = CHIP_ERROR_SCHEMA_MISMATCH);

    // If aReader is null, we got a status response and the command id in the
    // path should match our command id.  If aReader is not null, we got a data
    // response, which will have its own command id, which we don't know.
    VerifyOrExit(aCommandPath.mCommandId == mCommandId || aReader != nullptr, err = CHIP_ERROR_SCHEMA_MISMATCH);

    if (aReader != nullptr) {
        err = app::DataModel::Decode(*aReader, response);
        SuccessOrExit(err);
    }

    mOnSuccess(aCommandPath, aStatus, response);

exit:
    if (err != CHIP_NO_ERROR) {
        mOnError(err);
    }
}

- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(id)commandFields
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                              queue:(dispatch_queue_t)queue
                         completion:(MTRDeviceResponseHandler)completion
{
    // We don't have a way to communicate a non-default invoke timeout
    // here for now.
    // TODO: https://github.com/project-chip/connectedhomeip/issues/24563
    [self _invokeCommandWithEndpointID:endpointID
                             clusterID:clusterID
                             commandID:commandID
                         commandFields:commandFields
                    timedInvokeTimeout:timeoutMs
           serverSideProcessingTimeout:nil
                               logCall:YES
                                 queue:queue
                            completion:completion];
}

- (void)_invokeCommandWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                           commandID:(NSNumber *)commandID
                       commandFields:(id)commandFields
                  timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
         serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
                             logCall:(BOOL)logCall
                               queue:(dispatch_queue_t)queue
                          completion:(MTRDeviceResponseHandler)completion
{
    endpointID = (endpointID == nil) ? nil : [endpointID copy];
    clusterID = (clusterID == nil) ? nil : [clusterID copy];
    commandID = (commandID == nil) ? nil : [commandID copy];
    // TODO: This is not going to deep-copy the NSArray instances in
    // commandFields.  We need to do something smarter here.
    commandFields = (commandFields == nil) ? nil : [commandFields copy];

    serverSideProcessingTimeout = [serverSideProcessingTimeout copy];
    if (serverSideProcessingTimeout != nil) {
        serverSideProcessingTimeout = MTRClampedNumber(serverSideProcessingTimeout, @(0), @(UINT16_MAX));
    }

    timeoutMs = [timeoutMs copy];
    if (timeoutMs != nil) {
        timeoutMs = MTRClampedNumber(timeoutMs, @(1), @(UINT16_MAX));
    }

    if (logCall) {
        MTR_LOG("%@ invoke %@ 0x%llx (%@) 0x%llx (%@): %@", self, endpointID,
            clusterID.unsignedLongLongValue, MTRClusterNameForID(static_cast<MTRClusterIDType>(clusterID.unsignedLongLongValue)),
            commandID.unsignedLongLongValue, MTRRequestCommandNameForID(static_cast<MTRClusterIDType>(clusterID.unsignedLongLongValue), static_cast<MTRCommandIDType>(commandID.unsignedLongLongValue)),
            commandFields);
    }

    auto * bridge = new MTRDataValueDictionaryCallbackBridge(queue, completion,
        ^(ExchangeManager & exchangeManager, const SessionHandle & session, MTRDataValueDictionaryCallback successCb,
            MTRErrorCallback failureCb, MTRCallbackBridgeBase * bridge) {
            NSData * attestationChallenge;
            if ([clusterID isEqualToNumber:@(MTRClusterIDTypeOperationalCredentialsID)] &&
                [commandID isEqualToNumber:@(MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationRequestID)] && session->IsSecureSession()) {
                // An AttestationResponse command needs to have an attestationChallenge
                // to make sense of the results.  If we are doing an
                // AttestationRequest, store the challenge now.
                attestationChallenge = AsData(session->AsSecureSession()->GetCryptoContext().GetAttestationChallenge());
            }
            // NSObjectCommandCallback guarantees that there will be exactly one call to either the success callback or the failure
            // callback.
            auto onSuccessCb = [successCb, bridge, attestationChallenge](const app::ConcreteCommandPath & commandPath, const app::StatusIB & status,
                                   const MTRDataValueDictionaryDecodableType & responseData) {
                auto resultArray = [[NSMutableArray alloc] init];
                if (responseData.GetDecodedObject()) {
                    auto response = responseData.GetDecodedObject();
                    if (attestationChallenge != nil) {
                        // Add the attestationChallenge to our data.
                        NSArray<NSDictionary<NSString *, id> *> * value = response[MTRValueKey];
                        NSMutableArray<NSDictionary<NSString *, id> *> * newValue = [[NSMutableArray alloc] initWithCapacity:(value.count + 1)];
                        [newValue addObjectsFromArray:value];
                        [newValue addObject:@{
                            MTRContextTagKey : @(kAttestationChallengeTagValue),
                            MTRDataKey : @ {
                                MTRTypeKey : MTROctetStringValueType,
                                MTRValueKey : attestationChallenge,
                            },
                        }];
                        auto * newResponse = [NSMutableDictionary dictionaryWithCapacity:(response.count + 1)];
                        [newResponse addEntriesFromDictionary:response];
                        newResponse[MTRValueKey] = newValue;
                        response = newResponse;
                    }
                    [resultArray addObject:@ {
                        MTRCommandPathKey : [[MTRCommandPath alloc] initWithPath:commandPath],
                        MTRDataKey : response,
                    }];
                } else {
                    [resultArray addObject:@ { MTRCommandPathKey : [[MTRCommandPath alloc] initWithPath:commandPath] }];
                }
                successCb(bridge, resultArray);
            };

            auto onFailureCb = [failureCb, bridge](CHIP_ERROR aError) { failureCb(bridge, aError); };

            app::CommandPathParams commandPath = { static_cast<chip::EndpointId>([endpointID unsignedShortValue]), 0,
                static_cast<chip::ClusterId>([clusterID unsignedLongValue]),
                static_cast<chip::CommandId>([commandID unsignedLongValue]), (app::CommandPathFlags::kEndpointIdValid) };

            auto decoder = chip::Platform::MakeUnique<NSObjectCommandCallback>(
                commandPath.mClusterId, commandPath.mCommandId, onSuccessCb, onFailureCb);
            VerifyOrReturnError(decoder != nullptr, CHIP_ERROR_NO_MEMORY);

            auto rawDecoderPtr = decoder.get();
            auto onDoneCb = [rawDecoderPtr](app::CommandSender * commandSender) {
                chip::Platform::Delete(commandSender);
                chip::Platform::Delete(rawDecoderPtr);
            };

            decoder->SetOnDoneCallback(onDoneCb);

            bool isTimedRequest = (timeoutMs != nil);
            auto commandSender = chip::Platform::MakeUnique<app::CommandSender>(decoder.get(), &exchangeManager, isTimedRequest);
            VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_NO_MEMORY);

            ReturnErrorOnFailure(commandSender->AddRequestData(commandPath, MTRDataValueDictionaryDecodableType(commandFields),
                (timeoutMs == nil) ? NullOptional : Optional<uint16_t>([timeoutMs unsignedShortValue])));

            Optional<System::Clock::Timeout> invokeTimeout;
            if (serverSideProcessingTimeout != nil) {
                // Clamp to a number of seconds that will not overflow 32-bit
                // int when converted to ms.
                auto serverTimeoutInSeconds = System::Clock::Seconds16(serverSideProcessingTimeout.unsignedShortValue);
                invokeTimeout.SetValue(session->ComputeRoundTripTimeout(serverTimeoutInSeconds));
            }
            ReturnErrorOnFailure(commandSender->SendCommandRequest(session, invokeTimeout));

            decoder.release();
            commandSender.release();
            return CHIP_NO_ERROR;
        });
    std::move(*bridge).DispatchAction(self);
}

- (void)_invokeKnownCommandWithEndpointID:(NSNumber *)endpointID
                                clusterID:(NSNumber *)clusterID
                                commandID:(NSNumber *)commandID
                           commandPayload:(id)commandPayload
                       timedInvokeTimeout:(NSNumber * _Nullable)timeout
              serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
                            responseClass:(Class _Nullable)responseClass
                                    queue:(dispatch_queue_t)queue
                               completion:(void (^)(id _Nullable response, NSError * _Nullable error))completion
{
    NSError * encodingError;
    auto * commandFields = [commandPayload _encodeAsDataValue:&encodingError];
    if (commandFields == nil) {
        dispatch_async(queue, ^{
            completion(nil, encodingError);
        });
        return;
    }

    auto responseHandler = ^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
        id _Nullable response = nil;
        if (error == nil) {
            if (values.count != 1) {
                error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:nil];
            } else if (responseClass != nil) {
                response = [[responseClass alloc] initWithResponseValue:values[0] error:&error];
            }
        }
        completion(response, error);
    };

    [self _invokeCommandWithEndpointID:endpointID
                             clusterID:clusterID
                             commandID:commandID
                         commandFields:commandFields
                    timedInvokeTimeout:timeout
           serverSideProcessingTimeout:serverSideProcessingTimeout
                               logCall:YES
                                 queue:queue
                            completion:responseHandler];
}

- (void)subscribeToAttributesWithEndpointID:(NSNumber * _Nullable)endpointID
                                  clusterID:(NSNumber * _Nullable)clusterID
                                attributeID:(NSNumber * _Nullable)attributeID
                                     params:(MTRSubscribeParams * _Nullable)params
                                      queue:(dispatch_queue_t)queue
                              reportHandler:(MTRDeviceResponseHandler)reportHandler
                    subscriptionEstablished:(MTRSubscriptionEstablishedHandler)subscriptionEstablished
{
    NSArray<MTRAttributeRequestPath *> * attributePaths = [NSArray
        arrayWithObject:[MTRAttributeRequestPath requestPathWithEndpointID:endpointID clusterID:clusterID attributeID:attributeID]];
    [self subscribeToAttributePaths:attributePaths
                         eventPaths:nil
                             params:params
                              queue:queue
                      reportHandler:reportHandler
            subscriptionEstablished:subscriptionEstablished
            resubscriptionScheduled:nil];
}

- (void)_subscribeToKnownAttributeWithEndpointID:(NSNumber *)endpointID
                                       clusterID:(NSNumber *)clusterID
                                     attributeID:(NSNumber *)attributeID
                                          params:(MTRSubscribeParams *)params
                                           queue:(dispatch_queue_t)queue
                                   reportHandler:(void (^)(id _Nullable value, NSError * _Nullable error))reportHandler
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
{
    auto * attributePath = [MTRAttributePath attributePathWithEndpointID:endpointID clusterID:clusterID attributeID:attributeID];

    auto innerReportHandler = ^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
        if (error != nil) {
            reportHandler(nil, error);
            return;
        }

        for (NSDictionary<NSString *, id> * value in values) {
            NSError * initError;
            auto * report = [[MTRAttributeReport alloc] initWithResponseValue:value error:&initError];
            if (initError != nil) {
                reportHandler(nil, initError);
                continue;
            }

            if (![report.path isEqual:attributePath]) {
                // For some reason the server returned data for the wrong
                // attribute, even though it happened to decode to our type.
                reportHandler(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeSchemaMismatch userInfo:nil]);
                continue;
            }

            reportHandler(report.value, report.error);
        }
    };

    [self subscribeToAttributesWithEndpointID:endpointID
                                    clusterID:clusterID
                                  attributeID:attributeID
                                       params:params
                                        queue:queue
                                reportHandler:innerReportHandler
                      subscriptionEstablished:subscriptionEstablished];
}

- (void)subscribeToAttributePaths:(NSArray<MTRAttributeRequestPath *> * _Nullable)attributePaths
                       eventPaths:(NSArray<MTREventRequestPath *> * _Nullable)eventPaths
                           params:(MTRSubscribeParams * _Nullable)params
                            queue:(dispatch_queue_t)queue
                    reportHandler:(MTRDeviceResponseHandler)reportHandler
          subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished
          resubscriptionScheduled:(MTRDeviceResubscriptionScheduledHandler _Nullable)resubscriptionScheduled
{
    if ((attributePaths == nil || [attributePaths count] == 0) && (eventPaths == nil || [eventPaths count] == 0)) {
        // Per spec a server would respond InvalidAction to this, so just go
        // ahead and do that.
        dispatch_async(queue, ^{
            reportHandler(nil, [MTRError errorForIMStatus:StatusIB(Status::InvalidAction)]);
        });
        return;
    }

    if (self.isPASEDevice) {
        // We don't support subscriptions over PASE.
        dispatch_async(queue, ^{
            reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    auto * concreteController = self.concreteController;
    if (concreteController == nil) {
        // No subscriptions (or really any MTRBaseDevice use) with XPC controllers.
        MTR_LOG_ERROR("Unable to create subscription for MTRBaseDevice created with an XPC controller");
        dispatch_async(queue, ^{
            reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // Copy params before going async.
    NSArray<MTRAttributeRequestPath *> * attributes = nil;
    if (attributePaths != nil) {
        attributes = [[NSArray alloc] initWithArray:attributePaths copyItems:YES];
    }

    NSArray<MTREventRequestPath *> * events = nil;
    if (eventPaths != nil) {
        events = [[NSArray alloc] initWithArray:eventPaths copyItems:YES];
    }

    params = (params == nil) ? nil : [params copy];

    [concreteController
        getSessionForNode:self.nodeID
               completion:^(ExchangeManager * _Nullable exchangeManager, const Optional<SessionHandle> & session,
                   NSError * _Nullable error, NSNumber * _Nullable retryDelay) {
                   if (error != nil) {
                       dispatch_async(queue, ^{
                           reportHandler(nil, error);
                       });
                       return;
                   }

                   auto onAttributeReportCb = [queue, reportHandler](const ConcreteAttributePath & attributePath,
                                                  const MTRDataValueDictionaryDecodableType & data) {
                       id valueObject = data.GetDecodedObject();
                       ConcreteAttributePath pathCopy(attributePath);
                       dispatch_async(queue, ^{
                           reportHandler(@[ @ {
                               MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:pathCopy],
                               MTRDataKey : valueObject
                           } ],
                               nil);
                       });
                   };

                   auto onEventReportCb = [queue, reportHandler](
                                              const EventHeader & eventHeader, const MTRDataValueDictionaryDecodableType & data) {
                       NSDictionary * report = [MTRBaseDevice eventReportForHeader:eventHeader andData:data.GetDecodedObject()];
                       dispatch_async(queue, ^{
                           reportHandler(@[ report ], nil);
                       });
                   };

                   auto onFailureCb = [queue, reportHandler](const app::ConcreteAttributePath * attributePath,
                                          const app::ConcreteEventPath * eventPath, CHIP_ERROR error) {
                       if (attributePath != nullptr) {
                           ConcreteAttributePath pathCopy(*attributePath);
                           dispatch_async(queue, ^{
                               reportHandler(@[ @ {
                                   MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:pathCopy],
                                   MTRErrorKey : [MTRError errorForCHIPErrorCode:error]
                               } ],
                                   nil);
                           });
                       } else if (eventPath != nullptr) {
                           ConcreteEventPath pathCopy(*eventPath);
                           dispatch_async(queue, ^{
                               reportHandler(@[ @ {
                                   MTREventPathKey : [[MTREventPath alloc] initWithPath:pathCopy],
                                   MTRErrorKey : [MTRError errorForCHIPErrorCode:error]
                               } ],
                                   nil);
                           });
                       } else {
                           dispatch_async(queue, ^{
                               reportHandler(nil, [MTRError errorForCHIPErrorCode:error]);
                           });
                       }
                   };

                   auto onEstablishedCb = [queue, subscriptionEstablished]() {
                       if (subscriptionEstablished) {
                           dispatch_async(queue, subscriptionEstablished);
                       }
                   };

                   auto onResubscriptionScheduledCb
                       = [queue, resubscriptionScheduled](NSError * error, NSNumber * resubscriptionDelay) {
                             if (resubscriptionScheduled) {
                                 dispatch_async(queue, ^{
                                     resubscriptionScheduled(error, resubscriptionDelay);
                                 });
                             }
                         };

                   MTRReadClientContainer * container = [[MTRReadClientContainer alloc] init];
                   container.deviceID = self.nodeID;

                   size_t attributePathSize = 0;
                   if (attributes != nil) {
                       container.pathParams = static_cast<AttributePathParams *>(
                           Platform::MemoryCalloc([attributes count], sizeof(AttributePathParams)));
                       if (container.pathParams == nullptr) {
                           dispatch_async(queue, ^{
                               reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NO_MEMORY]);
                           });
                           [container cleanup];
                           return;
                       }
                       for (MTRAttributeRequestPath * attribute in attributes) {
                           [attribute convertToAttributePathParams:container.pathParams[attributePathSize++]];
                       }
                   }
                   size_t eventPathSize = 0;
                   if (events != nil) {
                       container.eventPathParams
                           = static_cast<EventPathParams *>(Platform::MemoryCalloc([events count], sizeof(EventPathParams)));
                       if (container.eventPathParams == nullptr) {
                           dispatch_async(queue, ^{
                               reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NO_MEMORY]);
                           });
                           [container cleanup];
                           return;
                       }
                       for (MTREventRequestPath * event in events) {
                           [event convertToEventPathParams:container.eventPathParams[eventPathSize++]];
                       }
                   }

                   app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
                   CHIP_ERROR err = CHIP_NO_ERROR;

                   chip::app::ReadPrepareParams readParams(session.Value());
                   [params toReadPrepareParams:readParams];
                   readParams.mpAttributePathParamsList = container.pathParams;
                   readParams.mAttributePathParamsListSize = attributePathSize;
                   readParams.mpEventPathParamsList = container.eventPathParams;
                   readParams.mEventPathParamsListSize = eventPathSize;

                   auto onDone = [container](BufferedReadClientCallback<MTRDataValueDictionaryDecodableType> * callback) {
                       [container onDone];
                   };

                   auto callback = chip::Platform::MakeUnique<BufferedReadClientCallback<MTRDataValueDictionaryDecodableType>>(
                       container.pathParams, attributePathSize, container.eventPathParams, eventPathSize, onAttributeReportCb,
                       onEventReportCb, onFailureCb, onDone, onEstablishedCb, onResubscriptionScheduledCb);

                   auto readClient = Platform::New<app::ReadClient>(
                       engine, exchangeManager, callback->GetBufferedCallback(), chip::app::ReadClient::InteractionType::Subscribe);

                   container.readClientPtr = readClient;
                   container.callback = callback.release();

                   if (!params.resubscribeAutomatically) {
                       err = readClient->SendRequest(readParams);
                   } else {
                       err = readClient->SendAutoResubscribeRequest(std::move(readParams));
                   }

                   if (err != CHIP_NO_ERROR) {
                       dispatch_async(queue, ^{
                           reportHandler(nil, [MTRError errorForCHIPErrorCode:err]);
                       });
                       [container cleanup];
                       return;
                   }

                   // Read clients will be purged when deregistered.
                   AddReadClientContainer(container.deviceID, container);
               }];
}

- (void)deregisterReportHandlersWithQueue:(dispatch_queue_t)queue completion:(dispatch_block_t)completion
{
    // This method must only be used for MTRDeviceOverXPC. However, for unit testing purpose, the method purges all read clients.
    MTR_LOG_DEBUG("Unexpected call to deregister report handlers");
    PurgeReadClientContainers(self.deviceController, self.nodeID, queue, completion);
}

namespace {
class OpenCommissioningWindowHelper {
    typedef void (^ResultCallback)(CHIP_ERROR status, const SetupPayload &);

public:
    static CHIP_ERROR OpenCommissioningWindow(Controller::DeviceController * controller, NodeId nodeID,
        System::Clock::Seconds16 timeout, uint16_t discriminator, const Optional<uint32_t> & setupPIN, ResultCallback callback);

private:
    OpenCommissioningWindowHelper(Controller::DeviceController * controller, ResultCallback callback);

    static void OnOpenCommissioningWindowResponse(void * context, NodeId deviceId, CHIP_ERROR status, chip::SetupPayload payload);

    Controller::CommissioningWindowOpener mOpener;
    Callback::Callback<Controller::OnOpenCommissioningWindow> mOnOpenCommissioningWindowCallback;
    ResultCallback mResultCallback;
};

OpenCommissioningWindowHelper::OpenCommissioningWindowHelper(Controller::DeviceController * controller, ResultCallback callback)
    : mOpener(controller)
    , mOnOpenCommissioningWindowCallback(OnOpenCommissioningWindowResponse, this)
    , mResultCallback(callback)
{
}

CHIP_ERROR OpenCommissioningWindowHelper::OpenCommissioningWindow(Controller::DeviceController * controller, NodeId nodeID,
    System::Clock::Seconds16 timeout, uint16_t discriminator, const Optional<uint32_t> & setupPIN, ResultCallback callback)
{
    auto * self = new OpenCommissioningWindowHelper(controller, callback);
    SetupPayload unused;
    CHIP_ERROR err = self->mOpener.OpenCommissioningWindow(nodeID, timeout, Crypto::kSpake2p_Min_PBKDF_Iterations, discriminator,
        setupPIN, NullOptional, &self->mOnOpenCommissioningWindowCallback, unused);
    if (err != CHIP_NO_ERROR) {
        delete self;
    }
    // Else will clean up when the callback is called.
    return err;
}

void OpenCommissioningWindowHelper::OnOpenCommissioningWindowResponse(
    void * context, NodeId deviceId, CHIP_ERROR status, chip::SetupPayload payload)
{
    auto * self = static_cast<OpenCommissioningWindowHelper *>(context);
    self->mResultCallback(status, payload);
    delete self;
}

#pragma mark - Utility for time conversion
NSTimeInterval MTRTimeIntervalForEventTimestampValue(uint64_t timeValue)
{
    // Note: The event timestamp value as written in the spec is in microseconds, but the released 1.0 SDK implemented it in
    // milliseconds. The following issue was filed to address the inconsistency:
    //    https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/6236
    // For consistency with the released behavior, calculations here will be done in milliseconds.

    // First convert the event timestamp value (in milliseconds) to NSTimeInterval - to minimize potential loss of precision
    // of uint64 => NSTimeInterval (double), convert whole seconds and remainder separately and then combine
    uint64_t eventTimestampValueSeconds = timeValue / chip::kMillisecondsPerSecond;
    uint64_t eventTimestampValueRemainderMilliseconds = timeValue % chip::kMillisecondsPerSecond;
    NSTimeInterval eventTimestampValueRemainder
        = NSTimeInterval(eventTimestampValueRemainderMilliseconds) / static_cast<double>(chip::kMillisecondsPerSecond);
    NSTimeInterval eventTimestampValue = eventTimestampValueSeconds + eventTimestampValueRemainder;

    return eventTimestampValue;
}

#pragma mark - Utility for event priority conversion
BOOL MTRPriorityLevelIsValid(chip::app::PriorityLevel priorityLevel)
{
    return (priorityLevel >= chip::app::PriorityLevel::Debug) && (priorityLevel <= chip::app::PriorityLevel::Critical);
}

MTREventPriority MTREventPriorityForValidPriorityLevel(chip::app::PriorityLevel priorityLevel)
{
    switch (priorityLevel) {
    case chip::app::PriorityLevel::Debug:
        return MTREventPriorityDebug;
    case chip::app::PriorityLevel::Info:
        return MTREventPriorityInfo;
    default:
        return MTREventPriorityCritical;
    }
}

} // anonymous namespace

- (void)_openCommissioningWindowWithSetupPasscode:(nullable NSNumber *)setupPasscode
                                    discriminator:(NSNumber *)discriminator
                                         duration:(NSNumber *)duration
                                            queue:(dispatch_queue_t)queue
                                       completion:(MTRDeviceOpenCommissioningWindowHandler)completion
{
    MATTER_LOG_METRIC_BEGIN(kMetricOpenPairingWindow);

    if (self.isPASEDevice) {
        MTR_LOG_ERROR("Can't open a commissioning window over PASE");
        dispatch_async(queue, ^{
            MATTER_LOG_METRIC_END(kMetricOpenPairingWindow, CHIP_ERROR_INCORRECT_STATE);
            completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    auto * concreteController = self.concreteController;
    if (concreteController == nil) {
        MTR_LOG_ERROR("Can't open a commissioning window via MTRBaseDevice created with an XPC controller");
        dispatch_async(queue, ^{
            MATTER_LOG_METRIC_END(kMetricOpenPairingWindow, CHIP_ERROR_INCORRECT_STATE);
            completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    unsigned long long durationVal = [duration unsignedLongLongValue];
    if (!CanCastTo<uint16_t>(durationVal)) {
        MTR_LOG_ERROR("Error: Duration %llu is too large.", durationVal);
        dispatch_async(queue, ^{
            MATTER_LOG_METRIC_END(kMetricOpenPairingWindow, CHIP_ERROR_INVALID_INTEGER_VALUE);
            completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE]);
        });
        return;
    }

    unsigned long long discriminatorVal = [discriminator unsignedLongLongValue];

    if (discriminatorVal > 0xFFF) {
        MTR_LOG_ERROR("Error: Discriminator %llu is too large. Max value %d", discriminatorVal, 0xFFF);
        dispatch_async(queue, ^{
            MATTER_LOG_METRIC_END(kMetricOpenPairingWindow, CHIP_ERROR_INVALID_INTEGER_VALUE);
            completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE]);
        });
        return;
    }

    Optional<uint32_t> passcode;
    if (setupPasscode != nil) {
        unsigned long long passcodeVal = [setupPasscode unsignedLongLongValue];
        if (!CanCastTo<uint32_t>(passcodeVal) || !SetupPayload::IsValidSetupPIN(static_cast<uint32_t>(passcodeVal))) {
            MTR_LOG_ERROR("Error: Setup passcode %llu is not valid", passcodeVal);
            dispatch_async(queue, ^{
                MATTER_LOG_METRIC_END(kMetricOpenPairingWindow, CHIP_ERROR_INVALID_INTEGER_VALUE);
                completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE]);
            });
            return;
        }
        passcode.Emplace(static_cast<uint32_t>(passcodeVal));
    }

    [concreteController
        asyncGetCommissionerOnMatterQueue:^(Controller::DeviceCommissioner * commissioner) {
            auto resultCallback = ^(CHIP_ERROR status, const SetupPayload & payload) {
                if (status != CHIP_NO_ERROR) {
                    dispatch_async(queue, ^{
                        MATTER_LOG_METRIC_END(kMetricOpenPairingWindow, status);
                        completion(nil, [MTRError errorForCHIPErrorCode:status]);
                    });
                    return;
                }
                auto * payloadObj = [[MTRSetupPayload alloc] initWithSetupPayload:payload];
                if (payloadObj == nil) {
                    dispatch_async(queue, ^{
                        MATTER_LOG_METRIC_END(kMetricOpenPairingWindow, CHIP_ERROR_NO_MEMORY);
                        completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_NO_MEMORY]);
                    });
                    return;
                }

                dispatch_async(queue, ^{
                    MATTER_LOG_METRIC_END(kMetricOpenPairingWindow, CHIP_NO_ERROR);
                    completion(payloadObj, nil);
                });
            };

            SetupPayload setupPayload;
            auto errorCode = OpenCommissioningWindowHelper::OpenCommissioningWindow(commissioner, self.nodeID,
                chip::System::Clock::Seconds16(static_cast<uint16_t>(durationVal)), static_cast<uint16_t>(discriminatorVal),
                passcode, resultCallback);

            if (errorCode != CHIP_NO_ERROR) {
                dispatch_async(queue, ^{
                    MATTER_LOG_METRIC_END(kMetricOpenPairingWindow, errorCode);
                    completion(nil, [MTRError errorForCHIPErrorCode:errorCode]);
                });
                return;
            }

            // resultCallback will handle things now.
        }
        errorHandler:^(NSError * error) {
            dispatch_async(queue, ^{
                MATTER_LOG_METRIC_END(kMetricOpenPairingWindow, [MTRError errorToCHIPErrorCode:error]);
                completion(nil, error);
            });
        }];
}

- (void)openCommissioningWindowWithSetupPasscode:(NSNumber *)setupPasscode
                                   discriminator:(NSNumber *)discriminator
                                        duration:(NSNumber *)duration
                                           queue:(dispatch_queue_t)queue
                                      completion:(MTRDeviceOpenCommissioningWindowHandler)completion
{
    [self _openCommissioningWindowWithSetupPasscode:setupPasscode
                                      discriminator:discriminator
                                           duration:duration
                                              queue:queue
                                         completion:completion];
}

- (void)openCommissioningWindowWithDiscriminator:(NSNumber *)discriminator
                                        duration:(NSNumber *)duration
                                           queue:(dispatch_queue_t)queue
                                      completion:(MTRDeviceOpenCommissioningWindowHandler)completion
{
    [self _openCommissioningWindowWithSetupPasscode:nil
                                      discriminator:discriminator
                                           duration:duration
                                              queue:queue
                                         completion:completion];
}

#ifdef DEBUG
// The following method is for unit testing purpose only
+ (id)CHIPEncodeAndDecodeNSObject:(id)object
{
    MTRDataValueDictionaryDecodableType originalData(object);
    chip::TLV::TLVWriter writer;
    uint8_t buffer[1024];
    writer.Init(buffer, sizeof(buffer));

    CHIP_ERROR error = originalData.Encode(writer, chip::TLV::CommonTag(1));
    if (error != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Error: Data encoding failed: %s", error.AsString());
        return nil;
    }

    error = writer.Finalize();
    if (error != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Error: TLV writer finalizing failed: %s", error.AsString());
        return nil;
    }
    chip::TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    error = reader.Next();
    if (error != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Error: TLV reader failed to fetch next element: %s", error.AsString());
        return nil;
    }
    __auto_type tag = reader.GetTag();
    if (tag != chip::TLV::CommonTag(1)) {
        MTR_LOG_ERROR("Error: TLV reader did not read the tag correctly: %x.%u", chip::TLV::ProfileIdFromTag(tag),
            chip::TLV::TagNumFromTag(tag));
        return nil;
    }
    MTRDataValueDictionaryDecodableType decodedData;
    error = decodedData.Decode(reader);
    if (error != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Error: Data decoding failed: %s", error.AsString());
        return nil;
    }
    return decodedData.GetDecodedObject();
}
#endif

- (void)readEventsWithEndpointID:(NSNumber * _Nullable)endpointID
                       clusterID:(NSNumber * _Nullable)clusterID
                         eventID:(NSNumber * _Nullable)eventID
                          params:(MTRReadParams * _Nullable)params
                           queue:(dispatch_queue_t)queue
                      completion:(MTRDeviceResponseHandler)completion
{
    NSArray<MTREventRequestPath *> * eventPaths = [NSArray arrayWithObject:[MTREventRequestPath requestPathWithEndpointID:endpointID
                                                                                                                clusterID:clusterID
                                                                                                                  eventID:eventID]];
    [self readAttributePaths:nil eventPaths:eventPaths params:params queue:queue completion:completion];
}

- (void)subscribeToEventsWithEndpointID:(NSNumber * _Nullable)endpointID
                              clusterID:(NSNumber * _Nullable)clusterID
                                eventID:(NSNumber * _Nullable)eventID
                                 params:(MTRSubscribeParams * _Nullable)params
                                  queue:(dispatch_queue_t)queue
                          reportHandler:(MTRDeviceResponseHandler)reportHandler
                subscriptionEstablished:(MTRSubscriptionEstablishedHandler)subscriptionEstablished
{
    NSArray<MTREventRequestPath *> * eventPaths = [NSArray arrayWithObject:[MTREventRequestPath requestPathWithEndpointID:endpointID
                                                                                                                clusterID:clusterID
                                                                                                                  eventID:eventID]];
    [self subscribeToAttributePaths:nil
                         eventPaths:eventPaths
                             params:params
                              queue:queue
                      reportHandler:reportHandler
            subscriptionEstablished:subscriptionEstablished
            resubscriptionScheduled:nil];
}

+ (NSDictionary *)eventReportForHeader:(const chip::app::EventHeader &)header andData:(id _Nullable)data
{
    MTREventPath * eventPath = [[MTREventPath alloc] initWithPath:header.mPath];
    if (data == nil) {
        MTR_LOG_ERROR("%@ could not decode event data", eventPath);
        return @{ MTREventPathKey : eventPath, MTRErrorKey : [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT] };
    }

    // Construct the right type, and key/value depending on the type
    NSNumber * eventTimeType;
    NSString * timestampKey;
    id timestampValue;
    if (header.mTimestamp.mType == Timestamp::Type::kSystem) {
        eventTimeType = @(MTREventTimeTypeSystemUpTime);
        timestampKey = MTREventSystemUpTimeKey;
        timestampValue = @(MTRTimeIntervalForEventTimestampValue(header.mTimestamp.mValue));
    } else if (header.mTimestamp.mType == Timestamp::Type::kEpoch) {
        eventTimeType = @(MTREventTimeTypeTimestampDate);
        timestampKey = MTREventTimestampDateKey;
        timestampValue = [NSDate dateWithTimeIntervalSince1970:MTRTimeIntervalForEventTimestampValue(header.mTimestamp.mValue)];
    } else {
        MTR_LOG_ERROR("%@ Unsupported event timestamp type %u - ignoring", eventPath, (unsigned int) header.mTimestamp.mType);
        return @{ MTREventPathKey : eventPath, MTRErrorKey : [MTRError errorForCHIPErrorCode:CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE] };
    }

    if (!MTRPriorityLevelIsValid(header.mPriorityLevel)) {
        MTR_LOG_ERROR("%@ Unsupported event priority %u - ignoring", eventPath, (unsigned int) header.mPriorityLevel);
        return @{ MTREventPathKey : eventPath, MTRErrorKey : [MTRError errorForCHIPErrorCode:CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE] };
    }

    return @{
        MTREventPathKey : eventPath,
        MTRDataKey : data,
        MTREventNumberKey : @(header.mEventNumber),
        MTREventPriorityKey : @(MTREventPriorityForValidPriorityLevel(header.mPriorityLevel)),
        MTREventTimeTypeKey : eventTimeType,
        timestampKey : timestampValue
    };
}

+ (System::PacketBufferHandle)_responseDataForCommand:(NSDictionary<NSString *, id> *)responseValue
                                            clusterID:(chip::ClusterId)clusterID
                                            commandID:(chip::CommandId)commandID
                                                error:(NSError * __autoreleasing *)error
{
    if (!CheckMemberOfType(responseValue, MTRCommandPathKey, [MTRCommandPath class],
            @"response-value command path is not an MTRCommandPath.", error)) {
        return System::PacketBufferHandle();
    }

    MTRCommandPath * path = responseValue[MTRCommandPathKey];

    if (![path.cluster isEqualToNumber:@(clusterID)]) {
        LogStringAndReturnError([NSString stringWithFormat:@"Expected cluster id %@ but got %@", path.cluster, @(clusterID)],
            MTRErrorCodeSchemaMismatch, error);
        return System::PacketBufferHandle();
    }

    if (![path.command isEqualToNumber:@(commandID)]) {
        LogStringAndReturnError([NSString stringWithFormat:@"Expected command id %@ but got %@", path.command, @(commandID)],
            MTRErrorCodeSchemaMismatch, error);
        return System::PacketBufferHandle();
    }

    if (!CheckMemberOfType(
            responseValue, MTRDataKey, [NSDictionary class], @"response-value data is not a data-value dictionary.", error)) {
        return System::PacketBufferHandle();
    }

    NSDictionary * data = responseValue[MTRDataKey];

    auto buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSizeWithoutReserve, 0);
    if (buffer.IsNull()) {
        LogStringAndReturnError(@"Unable to allocate encoding buffer", CHIP_ERROR_NO_MEMORY, error);
        return System::PacketBufferHandle();
    }

    System::PacketBufferTLVWriter writer;
    // Commands never need chained buffers, since they cannot be chunked.
    writer.Init(std::move(buffer), /* useChainedBuffers = */ false);

    CHIP_ERROR errorCode = MTREncodeTLVFromDataValueDictionary(data, &writer, TLV::AnonymousTag());
    if (errorCode != CHIP_NO_ERROR) {
        LogStringAndReturnError(@"Unable to encode data-value to TLV", errorCode, error);
        return System::PacketBufferHandle();
    }

    errorCode = writer.Finalize(&buffer);
    if (errorCode != CHIP_NO_ERROR) {
        LogStringAndReturnError(@"Unable to encode data-value to TLV", errorCode, error);
        return System::PacketBufferHandle();
    }

    return buffer;
}

- (void)downloadLogOfType:(MTRDiagnosticLogType)type
                  timeout:(NSTimeInterval)timeout
                    queue:(dispatch_queue_t)queue
               completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
{
    auto * concreteController = self.concreteController;
    if (concreteController == nil) {
        MTR_LOG_ERROR("Can't download logs via MTRBaseDevice created with an XPC controller");
        dispatch_async(queue, ^{
            completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    [concreteController downloadLogFromNodeWithID:@(_nodeID)
                                             type:type
                                          timeout:timeout
                                            queue:queue
                                       completion:completion];
}

- (NSString *)description
{
    return [NSString
        stringWithFormat:@"<%@: %p, node: %016llX-%016llX (%llu)>", NSStringFromClass(self.class), self, _deviceController.compressedFabricID.unsignedLongLongValue, _nodeID, _nodeID];
}

@end

@implementation MTRBaseDevice (Deprecated)

- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
                     params:(MTRSubscribeParams * _Nullable)params
             cacheContainer:(MTRAttributeCacheContainer * _Nullable)attributeCacheContainer
     attributeReportHandler:(MTRDeviceReportHandler _Nullable)attributeReportHandler
         eventReportHandler:(MTRDeviceReportHandler _Nullable)eventReportHandler
               errorHandler:(MTRDeviceErrorHandler)errorHandler
    subscriptionEstablished:(dispatch_block_t _Nullable)subscriptionEstablishedHandler
    resubscriptionScheduled:(MTRDeviceResubscriptionScheduledHandler _Nullable)resubscriptionScheduledHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:@(minInterval) maxInterval:@(maxInterval)];
    } else {
        subscribeParams.minInterval = @(minInterval);
        subscribeParams.maxInterval = @(maxInterval);
    }
    [self subscribeWithQueue:queue
                            params:subscribeParams
        clusterStateCacheContainer:attributeCacheContainer.realContainer
            attributeReportHandler:attributeReportHandler
                eventReportHandler:eventReportHandler
                      errorHandler:errorHandler
           subscriptionEstablished:subscriptionEstablishedHandler
           resubscriptionScheduled:resubscriptionScheduledHandler];
}

- (void)readAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                          clusterId:(NSNumber * _Nullable)clusterId
                        attributeId:(NSNumber * _Nullable)attributeId
                             params:(MTRReadParams * _Nullable)params
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion
{
    [self readAttributesWithEndpointID:endpointId
                             clusterID:clusterId
                           attributeID:attributeId
                                params:params
                                 queue:clientQueue
                            completion:completion];
}

- (void)writeAttributeWithEndpointId:(NSNumber *)endpointId
                           clusterId:(NSNumber *)clusterId
                         attributeId:(NSNumber *)attributeId
                               value:(id)value
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
                         clientQueue:(dispatch_queue_t)clientQueue
                          completion:(MTRDeviceResponseHandler)completion
{
    [self writeAttributeWithEndpointID:endpointId
                             clusterID:clusterId
                           attributeID:attributeId
                                 value:value
                     timedWriteTimeout:timeoutMs
                                 queue:clientQueue
                            completion:completion];
}

- (void)invokeCommandWithEndpointId:(NSNumber *)endpointId
                          clusterId:(NSNumber *)clusterId
                          commandId:(NSNumber *)commandId
                      commandFields:(id)commandFields
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion
{
    [self invokeCommandWithEndpointID:endpointId
                            clusterID:clusterId
                            commandID:commandId
                        commandFields:commandFields
                   timedInvokeTimeout:timeoutMs
                                queue:clientQueue
                           completion:completion];
}

- (void)subscribeAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                               clusterId:(NSNumber * _Nullable)clusterId
                             attributeId:(NSNumber * _Nullable)attributeId
                             minInterval:(NSNumber *)minInterval
                             maxInterval:(NSNumber *)maxInterval
                                  params:(MTRSubscribeParams * _Nullable)params
                             clientQueue:(dispatch_queue_t)clientQueue
                           reportHandler:(MTRDeviceResponseHandler)reportHandler
                 subscriptionEstablished:(dispatch_block_t _Nullable)subscriptionEstablishedHandler
{
    MTRSubscribeParams * _Nullable subscribeParams = [params copy];
    if (subscribeParams == nil) {
        subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscribeParams.minInterval = minInterval;
        subscribeParams.maxInterval = maxInterval;
    }
    [self subscribeToAttributesWithEndpointID:endpointId
                                    clusterID:clusterId
                                  attributeID:attributeId
                                       params:subscribeParams
                                        queue:clientQueue
                                reportHandler:reportHandler
                      subscriptionEstablished:subscriptionEstablishedHandler];
}

- (void)deregisterReportHandlersWithClientQueue:(dispatch_queue_t)queue completion:(dispatch_block_t)completion
{
    [self deregisterReportHandlersWithQueue:queue completion:completion];
}

@end

static NSString * FormatPossiblyWildcardEndpoint(NSNumber * _Nullable possiblyWildcardEndpoint)
{
    if (possiblyWildcardEndpoint == nil) {
        return @"wildcard";
    }

    return [NSString stringWithFormat:@"%u", possiblyWildcardEndpoint.unsignedShortValue];
}

static NSString * FormatPossiblyWildcardCluster(NSNumber * _Nullable possiblyWildcardCluster)
{
    if (possiblyWildcardCluster == nil) {
        return @"wildcard";
    }

    return [NSString stringWithFormat:@"0x%llx (%llu, %@)",
                     possiblyWildcardCluster.unsignedLongLongValue,
                     possiblyWildcardCluster.unsignedLongLongValue,
                     MTRClusterNameForID(static_cast<MTRClusterIDType>(possiblyWildcardCluster.unsignedLongLongValue))];
}

static NSString * FormatPossiblyWildcardClusterElement(NSNumber * _Nullable possiblyWildcardCluster, NSNumber * _Nullable possiblyWildcardElement, NSString * (^nameGetter)(MTRClusterIDType clusterID, NSNumber * elementID))
{
    if (possiblyWildcardElement == nil) {
        return @"wildcard";
    }

    if (possiblyWildcardCluster == nil) {
        // We can't get a useful name for this, so just return the numeric
        // value.
        return [NSString stringWithFormat:@"0x%llx (%llu)", possiblyWildcardElement.unsignedLongLongValue, possiblyWildcardElement.unsignedLongLongValue];
    }

    return [NSString stringWithFormat:@"0x%llx (%llu, %@)",
                     possiblyWildcardElement.unsignedLongLongValue,
                     possiblyWildcardElement.unsignedLongLongValue,
                     nameGetter(static_cast<MTRClusterIDType>(possiblyWildcardCluster.unsignedLongLongValue), possiblyWildcardElement)];
}

@implementation MTRAttributeRequestPath
- (instancetype)initWithEndpointID:(NSNumber * _Nullable)endpointID
                         clusterID:(NSNumber * _Nullable)clusterID
                       attributeID:(NSNumber * _Nullable)attributeID
{
    _endpoint = [endpointID copy];
    _cluster = [clusterID copy];
    _attribute = [attributeID copy];
    return self;
}

- (NSString *)description
{
    NSString * endpointStr = FormatPossiblyWildcardEndpoint(_endpoint);
    NSString * clusterStr = FormatPossiblyWildcardCluster(_cluster);
    NSString * attributeStr = FormatPossiblyWildcardClusterElement(_cluster, _attribute, ^(MTRClusterIDType clusterID, NSNumber * attributeID) {
        return MTRAttributeNameForID(clusterID, static_cast<MTRAttributeIDType>(attributeID.unsignedLongLongValue));
    });
    return [NSString stringWithFormat:@"<MTRAttributeRequestPath endpoint %@ cluster %@ attribute %@>", endpointStr, clusterStr, attributeStr];
}

+ (MTRAttributeRequestPath *)requestPathWithEndpointID:(NSNumber * _Nullable)endpointID
                                             clusterID:(NSNumber * _Nullable)clusterID
                                           attributeID:(NSNumber * _Nullable)attributeID
{

    return [[MTRAttributeRequestPath alloc] initWithEndpointID:endpointID clusterID:clusterID attributeID:attributeID];
}

- (BOOL)isEqualToAttributeRequestPath:(MTRAttributeRequestPath *)path
{
    return MTREqualObjects(_endpoint, path.endpoint)
        && MTREqualObjects(_cluster, path.cluster)
        && MTREqualObjects(_attribute, path.attribute);
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }
    return [self isEqualToAttributeRequestPath:object];
}

- (NSUInteger)hash
{
    return _endpoint.unsignedShortValue ^ _cluster.unsignedLongValue ^ _attribute.unsignedLongValue;
}

- (id)copyWithZone:(NSZone *)zone
{
    return [MTRAttributeRequestPath requestPathWithEndpointID:_endpoint clusterID:_cluster attributeID:_attribute];
}

- (void)convertToAttributePathParams:(chip::app::AttributePathParams &)params
{
    if (_endpoint != nil) {
        params.mEndpointId = static_cast<chip::EndpointId>(_endpoint.unsignedShortValue);
    } else {
        params.SetWildcardEndpointId();
    }

    if (_cluster != nil) {
        params.mClusterId = static_cast<chip::ClusterId>(_cluster.unsignedLongValue);
    } else {
        params.SetWildcardClusterId();
    }

    if (_attribute != nil) {
        params.mAttributeId = static_cast<chip::AttributeId>(_attribute.unsignedLongValue);
    } else {
        params.SetWildcardAttributeId();
    }
}

static NSString * const sEndpointIDKey = @"endpointIDKey";
static NSString * const sClusterIDKey = @"clusterIDKey";
static NSString * const sAttributeIDKey = @"attributeIDKey";

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (nullable instancetype)initWithCoder:(NSCoder *)decoder
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _endpoint = [decoder decodeObjectOfClass:[NSNumber class] forKey:sEndpointIDKey];
    if (_endpoint && ![_endpoint isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTRAttributeRequestPath decoded %@ for endpoint, not NSNumber.", _attribute);
        return nil;
    }

    _cluster = [decoder decodeObjectOfClass:[NSNumber class] forKey:sClusterIDKey];
    if (_cluster && ![_cluster isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTRAttributeRequestPath decoded %@ for cluster, not NSNumber.", _attribute);
        return nil;
    }

    _attribute = [decoder decodeObjectOfClass:[NSNumber class] forKey:sAttributeIDKey];
    if (_attribute && ![_attribute isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTRAttributeRequestPath decoded %@ for attribute, not NSNumber.", _attribute);
        return nil;
    }

    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    if (_endpoint) {
        [coder encodeObject:_endpoint forKey:sEndpointIDKey];
    }
    if (_cluster) {
        [coder encodeObject:_cluster forKey:sClusterIDKey];
    }
    if (_attribute) {
        [coder encodeObject:_attribute forKey:sAttributeIDKey];
    }
}

@end

@implementation MTREventRequestPath
- (instancetype)initWithEndpointID:(NSNumber * _Nullable)endpointID
                         clusterID:(NSNumber * _Nullable)clusterID
                           eventID:(NSNumber * _Nullable)eventID
{
    _endpoint = [endpointID copy];
    _cluster = [clusterID copy];
    _event = [eventID copy];
    return self;
}

- (NSString *)description
{
    NSString * endpointStr = FormatPossiblyWildcardEndpoint(_endpoint);
    NSString * clusterStr = FormatPossiblyWildcardCluster(_cluster);
    NSString * eventStr = FormatPossiblyWildcardClusterElement(_cluster, _event, ^(MTRClusterIDType clusterID, NSNumber * eventID) {
        return MTREventNameForID(clusterID, static_cast<MTREventIDType>(eventID.unsignedLongLongValue));
    });
    return [NSString stringWithFormat:@"<MTREventRequestPath endpoint %@ cluster %@ event %@>", endpointStr, clusterStr, eventStr];
}

+ (MTREventRequestPath *)requestPathWithEndpointID:(NSNumber * _Nullable)endpointID
                                         clusterID:(NSNumber * _Nullable)clusterID
                                           eventID:(NSNumber * _Nullable)eventID
{

    return [[MTREventRequestPath alloc] initWithEndpointID:endpointID clusterID:clusterID eventID:eventID];
}

- (BOOL)isEqualToEventRequestPath:(MTREventRequestPath *)path
{
    return MTREqualObjects(_endpoint, path.endpoint)
        && MTREqualObjects(_cluster, path.cluster)
        && MTREqualObjects(_event, path.event);
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }
    return [self isEqualToEventRequestPath:object];
}

- (NSUInteger)hash
{
    return _endpoint.unsignedShortValue ^ _cluster.unsignedLongValue ^ _event.unsignedLongValue;
}

- (id)copyWithZone:(NSZone *)zone
{
    return [MTREventRequestPath requestPathWithEndpointID:_endpoint clusterID:_cluster eventID:_event];
}

- (void)convertToEventPathParams:(chip::app::EventPathParams &)params
{
    if (_endpoint != nil) {
        params.mEndpointId = static_cast<chip::EndpointId>(_endpoint.unsignedShortValue);
    } else {
        params.SetWildcardEndpointId();
    }

    if (_cluster != nil) {
        params.mClusterId = static_cast<chip::ClusterId>(_cluster.unsignedLongValue);
    } else {
        params.SetWildcardClusterId();
    }

    if (_event != nil) {
        params.mEventId = static_cast<chip::EventId>(_event.unsignedLongValue);
    } else {
        params.SetWildcardEventId();
    }
}
static NSString * const sEventEndpointIDKey = @"endpointIDKey";
static NSString * const sEventClusterIDKey = @"clusterIDKey";
static NSString * const sEventAttributeIDKey = @"attributeIDKey";

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (nullable instancetype)initWithCoder:(NSCoder *)decoder
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _endpoint = [decoder decodeObjectOfClass:[NSNumber class] forKey:sEventEndpointIDKey];
    if (_endpoint && ![_endpoint isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTREventRequestPath decoded %@ for endpoint, not NSNumber.", _endpoint);
        return nil;
    }

    _cluster = [decoder decodeObjectOfClass:[NSNumber class] forKey:sEventClusterIDKey];
    if (_cluster && ![_cluster isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTREventRequestPath decoded %@ for cluster, not NSNumber.", _cluster);
        return nil;
    }

    _event = [decoder decodeObjectOfClass:[NSNumber class] forKey:sEventAttributeIDKey];
    if (_event && ![_event isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTREventRequestPath decoded %@ for event, not NSNumber.", _event);
        return nil;
    }

    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    if (_endpoint) {
        [coder encodeObject:_endpoint forKey:sEventEndpointIDKey];
    }
    if (_cluster) {
        [coder encodeObject:_cluster forKey:sEventClusterIDKey];
    }
    if (_event) {
        [coder encodeObject:_event forKey:sEventAttributeIDKey];
    }
}

@end

@implementation MTRClusterPath
- (instancetype)initWithPath:(const ConcreteClusterPath &)path
{
    if (self = [super init]) {
        _endpoint = @(path.mEndpointId);
        _cluster = @(path.mClusterId);
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRClusterPath endpoint %u cluster 0x%llx (%llu, %@)>", _endpoint.unsignedShortValue,
                     _cluster.unsignedLongLongValue, _cluster.unsignedLongLongValue,
                     MTRClusterNameForID(static_cast<MTRClusterIDType>(_cluster.unsignedLongLongValue))];
}

+ (MTRClusterPath *)clusterPathWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID
{
    ConcreteClusterPath path(static_cast<chip::EndpointId>([endpointID unsignedShortValue]),
        static_cast<chip::ClusterId>([clusterID unsignedLongValue]));

    return [[MTRClusterPath alloc] initWithPath:path];
}

- (BOOL)isEqualToClusterPath:(MTRClusterPath *)clusterPath
{
    return MTREqualObjects(_endpoint, clusterPath.endpoint)
        && MTREqualObjects(_cluster, clusterPath.cluster);
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }
    return [self isEqualToClusterPath:object];
}

- (NSUInteger)hash
{
    return _endpoint.unsignedShortValue ^ _cluster.unsignedLongValue;
}

- (id)copyWithZone:(NSZone *)zone
{
    return [MTRClusterPath clusterPathWithEndpointID:_endpoint clusterID:_cluster];
}

static NSString * const sEndpointKey = @"endpointKey";
static NSString * const sClusterKey = @"clusterKey";

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (nullable instancetype)initWithCoder:(NSCoder *)decoder
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _endpoint = [decoder decodeObjectOfClass:[NSNumber class] forKey:sEndpointKey];
    if (_endpoint && ![_endpoint isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTRClusterPath decoded %@ for endpoint, not NSNumber.", _endpoint);
        return nil;
    }

    _cluster = [decoder decodeObjectOfClass:[NSNumber class] forKey:sClusterKey];
    if (_cluster && ![_cluster isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTRClusterPath decoded %@ for cluster, not NSNumber.", _cluster);
        return nil;
    }

    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_endpoint forKey:sEndpointKey];
    [coder encodeObject:_cluster forKey:sClusterKey];
}

@end

@implementation MTRAttributePath
- (instancetype)initWithPath:(const ConcreteDataAttributePath &)path
{
    if (self = [super initWithPath:path]) {
        _attribute = @(path.mAttributeId);
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRAttributePath endpoint %u cluster 0x%llx (%llu, %@) 0x%llx (%llu, %@)>",
                     self.endpoint.unsignedShortValue,
                     self.cluster.unsignedLongLongValue, self.cluster.unsignedLongLongValue, MTRClusterNameForID(static_cast<MTRClusterIDType>(self.cluster.unsignedLongLongValue)),
                     _attribute.unsignedLongLongValue, _attribute.unsignedLongLongValue,
                     MTRAttributeNameForID(static_cast<MTRClusterIDType>(self.cluster.unsignedLongLongValue), static_cast<MTRAttributeIDType>(_attribute.unsignedLongLongValue))];
}

+ (MTRAttributePath *)attributePathWithEndpointID:(NSNumber *)endpointID
                                        clusterID:(NSNumber *)clusterID
                                      attributeID:(NSNumber *)attributeID
{
    ConcreteDataAttributePath path(static_cast<chip::EndpointId>([endpointID unsignedShortValue]),
        static_cast<chip::ClusterId>([clusterID unsignedLongValue]),
        static_cast<chip::AttributeId>([attributeID unsignedLongValue]));

    return [[MTRAttributePath alloc] initWithPath:path];
}

- (BOOL)isEqualToAttributePath:(MTRAttributePath *)attributePath
{
    return [self isEqualToClusterPath:attributePath] && MTREqualObjects(_attribute, attributePath.attribute);
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }
    return [self isEqualToAttributePath:object];
}

- (NSUInteger)hash
{
    return self.endpoint.unsignedShortValue ^ self.cluster.unsignedLongValue ^ _attribute.unsignedLongValue;
}

- (id)copyWithZone:(NSZone *)zone
{
    return [MTRAttributePath attributePathWithEndpointID:self.endpoint clusterID:self.cluster attributeID:_attribute];
}

- (ConcreteAttributePath)_asConcretePath
{
    return ConcreteAttributePath([self.endpoint unsignedShortValue], static_cast<ClusterId>([self.cluster unsignedLongValue]),
        static_cast<AttributeId>([self.attribute unsignedLongValue]));
}

static NSString * const sAttributeKey = @"attributeKey";

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (nullable instancetype)initWithCoder:(NSCoder *)decoder
{
    self = [super initWithCoder:decoder];
    if (self == nil) {
        return nil;
    }

    _attribute = [decoder decodeObjectOfClass:[NSNumber class] forKey:sAttributeKey];
    if (_attribute && ![_attribute isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTRAttributePath decoded %@ for attribute, not NSNumber.", _attribute);
        return nil;
    }

    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];
    [coder encodeObject:_attribute forKey:sAttributeKey];
}

@end

@implementation MTRAttributePath (Deprecated)
+ (instancetype)attributePathWithEndpointId:(NSNumber *)endpointId
                                  clusterId:(NSNumber *)clusterId
                                attributeId:(NSNumber *)attributeId
{
    return [self attributePathWithEndpointID:endpointId clusterID:clusterId attributeID:attributeId];
}
@end

@implementation MTREventPath
- (instancetype)initWithPath:(const ConcreteEventPath &)path
{
    if (self = [super initWithPath:path]) {
        _event = @(path.mEventId);
    }
    return self;
}

- (NSString *)description
{
    return
        [NSString stringWithFormat:@"<MTREventPath endpoint %u cluster 0x%llx (%llu, %@) event 0x%llx (%llu, %@)>",
                  self.endpoint.unsignedShortValue,
                  self.cluster.unsignedLongLongValue, self.cluster.unsignedLongLongValue, MTRClusterNameForID(static_cast<MTRClusterIDType>(self.cluster.unsignedLongLongValue)),
                  _event.unsignedLongLongValue, _event.unsignedLongLongValue,
                  MTREventNameForID(static_cast<MTRClusterIDType>(self.cluster.unsignedLongLongValue), static_cast<MTREventIDType>(_event.unsignedLongLongValue))];
}

+ (MTREventPath *)eventPathWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID eventID:(NSNumber *)eventID
{
    ConcreteEventPath path(static_cast<chip::EndpointId>([endpointID unsignedShortValue]),
        static_cast<chip::ClusterId>([clusterID unsignedLongValue]), static_cast<chip::EventId>([eventID unsignedLongValue]));

    return [[MTREventPath alloc] initWithPath:path];
}

- (BOOL)isEqualToEventPath:(MTREventPath *)eventPath
{
    return [self isEqualToClusterPath:eventPath] && MTREqualObjects(_event, eventPath.event);
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }
    return [self isEqualToEventPath:object];
}

- (NSUInteger)hash
{
    return self.endpoint.unsignedShortValue ^ self.cluster.unsignedLongValue ^ _event.unsignedLongValue;
}

- (id)copyWithZone:(NSZone *)zone
{
    return [MTREventPath eventPathWithEndpointID:self.endpoint clusterID:self.cluster eventID:_event];
}

- (ConcreteEventPath)_asConcretePath
{
    return ConcreteEventPath([self.endpoint unsignedShortValue], static_cast<ClusterId>([self.cluster unsignedLongValue]),
        static_cast<EventId>([self.event unsignedLongValue]));
}

static NSString * const sEventKey = @"eventKey";

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (nullable instancetype)initWithCoder:(NSCoder *)decoder
{
    self = [super initWithCoder:decoder];
    if (self == nil) {
        return nil;
    }

    _event = [decoder decodeObjectOfClass:[NSNumber class] forKey:sEventKey];
    if (_event && ![_event isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTREventPath decoded %@ for event, not NSNumber.", _event);
        return nil;
    }

    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];
    [coder encodeObject:_event forKey:sEventKey];
}
@end

@implementation MTREventPath (Deprecated)
+ (instancetype)eventPathWithEndpointId:(NSNumber *)endpointId clusterId:(NSNumber *)clusterId eventId:(NSNumber *)eventId
{
    return [self eventPathWithEndpointID:endpointId clusterID:clusterId eventID:eventId];
}
@end

@implementation MTRCommandPath
- (instancetype)initWithPath:(const ConcreteCommandPath &)path
{
    if (self = [super initWithPath:path]) {
        _command = @(path.mCommandId);
    }
    return self;
}

- (NSString *)description
{
    return
        [NSString stringWithFormat:@"<MTRCommandPath endpoint %u cluster 0x%llx (%llu) command 0x%llx (%llu)>", self.endpoint.unsignedShortValue,
                  self.cluster.unsignedLongLongValue, self.cluster.unsignedLongLongValue, _command.unsignedLongLongValue,
                  _command.unsignedLongLongValue];
}

+ (MTRCommandPath *)commandPathWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID commandID:(NSNumber *)commandID
{
    ConcreteCommandPath path(static_cast<chip::EndpointId>([endpointID unsignedShortValue]),
        static_cast<chip::ClusterId>([clusterID unsignedLongValue]), static_cast<chip::CommandId>([commandID unsignedLongValue]));

    return [[MTRCommandPath alloc] initWithPath:path];
}

- (BOOL)isEqualToCommandPath:(MTRCommandPath *)commandPath
{
    return [self isEqualToClusterPath:commandPath] && MTREqualObjects(_command, commandPath.command);
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }
    return [self isEqualToCommandPath:object];
}

- (NSUInteger)hash
{
    return self.endpoint.unsignedShortValue ^ self.cluster.unsignedLongValue ^ _command.unsignedLongValue;
}

- (id)copyWithZone:(NSZone *)zone
{
    return [MTRCommandPath commandPathWithEndpointID:self.endpoint clusterID:self.cluster commandID:_command];
}

static NSString * const sCommandKey = @"commandKey";

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (nullable instancetype)initWithCoder:(NSCoder *)decoder
{
    self = [super initWithCoder:decoder];
    if (self == nil) {
        return nil;
    }

    _command = [decoder decodeObjectOfClass:[NSNumber class] forKey:sCommandKey];
    if (_command && ![_command isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTRCommandPath decoded %@ for command, not NSNumber.", _command);
        return nil;
    }

    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];
    [coder encodeObject:_command forKey:sCommandKey];
}
@end

@implementation MTRCommandPath (Deprecated)
+ (instancetype)commandPathWithEndpointId:(NSNumber *)endpointId clusterId:(NSNumber *)clusterId commandId:(NSNumber *)commandId
{
    return [self commandPathWithEndpointID:endpointId clusterID:clusterId commandID:commandId];
}
@end

static void LogStringAndReturnError(NSString * errorStr, MTRErrorCode errorCode, NSError * __autoreleasing * error)
{
    MTR_LOG_ERROR("%s", errorStr.UTF8String);
    if (!error) {
        return;
    }

    NSDictionary * userInfo = @ { NSLocalizedFailureReasonErrorKey : NSLocalizedString(errorStr, nil) };
    *error = [NSError errorWithDomain:MTRErrorDomain code:errorCode userInfo:userInfo];
}

static void LogStringAndReturnError(NSString * errorStr, CHIP_ERROR errorCode, NSError * __autoreleasing * error)
{
    MTR_LOG_ERROR("%s: %s", errorStr.UTF8String, errorCode.AsString());
    if (!error) {
        return;
    }

    *error = [MTRError errorForCHIPErrorCode:errorCode];
}

static bool CheckMemberOfType(NSDictionary<NSString *, id> * responseValue, NSString * memberName, Class expectedClass,
    NSString * errorMessage, NSError * __autoreleasing * error)
{
    id _Nullable value = responseValue[memberName];
    if (value == nil) {
        LogStringAndReturnError([NSString stringWithFormat:@"%s is null when not expected to be", memberName.UTF8String],
            MTRErrorCodeInvalidArgument, error);
        return false;
    }

    if (![value isKindOfClass:expectedClass]) {
        LogStringAndReturnError(errorMessage, MTRErrorCodeInvalidArgument, error);
        return false;
    }

    return true;
}

// Allocates a buffer, encodes the data-value as TLV, and points the TLV::Reader
// to the data.  Returns false if any of that fails, in which case error gets
// set.
//
// Data model decoding requires a contiguous buffer (because lists walk all the
// data multiple times and TLVPacketBufferBackingStore doesn't have a way to
// checkpoint and restore its state), but we can encode into chained packet
// buffers and then decide whether we need a contiguous realloc.
static bool EncodeDataValueToTLV(System::PacketBufferHandle & buffer, Platform::ScopedMemoryBuffer<uint8_t> & flatBuffer,
    NSDictionary * data, TLV::TLVReader & reader, NSError * __autoreleasing * error)
{
    buffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSizeWithoutReserve, 0);
    if (buffer.IsNull()) {
        LogStringAndReturnError(@"Unable to allocate encoding buffer", CHIP_ERROR_NO_MEMORY, error);
        return false;
    }

    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(buffer), /* useChainedBuffers = */ true);

    CHIP_ERROR errorCode = MTREncodeTLVFromDataValueDictionary(data, &writer, TLV::AnonymousTag());
    if (errorCode != CHIP_NO_ERROR) {
        LogStringAndReturnError(@"Unable to encode data-value to TLV", errorCode, error);
        return false;
    }

    errorCode = writer.Finalize(&buffer);
    if (errorCode != CHIP_NO_ERROR) {
        LogStringAndReturnError(@"Unable to encode data-value to TLV", errorCode, error);
        return false;
    }

    if (buffer->HasChainedBuffer()) {
        // We need to reallocate into a single contiguous buffer.
        size_t remainingData = buffer->TotalLength();
        if (!flatBuffer.Calloc(remainingData)) {
            LogStringAndReturnError(@"Unable to allocate decoding buffer", CHIP_ERROR_NO_MEMORY, error);
            return false;
        }
        size_t copiedData = 0;
        while (!buffer.IsNull()) {
            if (buffer->DataLength() > remainingData) {
                // Should never happen, but let's be extra careful about buffer
                // overruns.
                LogStringAndReturnError(@"Encoding buffer size is bigger than it claimed", CHIP_ERROR_INCORRECT_STATE, error);
                return false;
            }

            memcpy(flatBuffer.Get() + copiedData, buffer->Start(), buffer->DataLength());
            copiedData += buffer->DataLength();
            remainingData -= buffer->DataLength();
            buffer.Advance();
        }
        if (remainingData != 0) {
            LogStringAndReturnError(
                @"Did not copy all data from Encoding buffer for some reason", CHIP_ERROR_INCORRECT_STATE, error);
            return false;
        }
        reader.Init(flatBuffer.Get(), copiedData);
    } else {
        reader.Init(buffer->Start(), buffer->DataLength());
    }

    errorCode = reader.Next(TLV::AnonymousTag());
    if (errorCode != CHIP_NO_ERROR) {
        LogStringAndReturnError(@"data-value TLV encoding did not create a TLV element", errorCode, error);
        return false;
    }

    return true;
}

@implementation MTRAttributeReport
+ (void)initialize
{
    // One of our init methods ends up doing Platform::MemoryAlloc.
    MTRFrameworkInit();
}

- (instancetype)initWithPath:(const ConcreteDataAttributePath &)path value:(id _Nullable)value error:(NSError * _Nullable)error
{
    if (self = [super init]) {
        _path = [[MTRAttributePath alloc] initWithPath:path];
        _value = value;
        _error = error;
    }
    return self;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    // In theory, the types of all the things in the dictionary will be correct
    // if our consumer passes in an actual response-value dictionary, but
    // double-check just to be sure
    if (!CheckMemberOfType(responseValue, MTRAttributePathKey, [MTRAttributePath class],
            @"response-value attribute path is not an MTRAttributePath.", error)) {
        return nil;
    }
    MTRAttributePath * path = responseValue[MTRAttributePathKey];

    id _Nullable value = responseValue[MTRErrorKey];
    if (value != nil) {
        if (!CheckMemberOfType(responseValue, MTRErrorKey, [NSError class], @"response-value error is not an NSError.", error)) {
            return nil;
        }

        _path = path;
        _value = nil;
        _error = value;
        return self;
    }

    if (!CheckMemberOfType(
            responseValue, MTRDataKey, [NSDictionary class], @"response-value data is not a data-value dictionary.", error)) {
        return nil;
    }
    NSDictionary * data = responseValue[MTRDataKey];

    // Encode the data to TLV and then decode from that, to reuse existing code.
    System::PacketBufferHandle buffer;
    Platform::ScopedMemoryBuffer<uint8_t> flatBuffer;
    TLV::TLVReader reader;
    if (!EncodeDataValueToTLV(buffer, flatBuffer, data, reader, error)) {
        return nil;
    }

    auto attributePath = [path _asConcretePath];

    CHIP_ERROR errorCode = CHIP_ERROR_INTERNAL;
    id decodedValue = MTRDecodeAttributeValue(attributePath, reader, &errorCode);
    if (errorCode == CHIP_NO_ERROR) {
        _path = path;
        _value = decodedValue;
        _error = nil;
        return self;
    }

    if (errorCode == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB) {
        LogStringAndReturnError(@"No known schema for decoding attribute value.", MTRErrorCodeUnknownSchema, error);
        return nil;
    }

    // Treat all other errors as schema errors.
    LogStringAndReturnError(@"Attribute decoding failed schema check.", MTRErrorCodeSchemaMismatch, error);
    return nil;
}

- (id)copyWithZone:(NSZone *)zone
{
    return [[MTRAttributeReport alloc] initWithPath:[self.path _asConcretePath] value:self.value error:self.error];
}

@end

@implementation MTREventReport {
    NSNumber * _timestampValue;
}

+ (void)initialize
{
    // One of our init methods ends up doing Platform::MemoryAlloc.
    MTRFrameworkInit();
}

- (instancetype)initWithPath:(const chip::app::ConcreteEventPath &)path
                 eventNumber:(NSNumber *)eventNumber
                    priority:(PriorityLevel)priority
                   timestamp:(const Timestamp &)timestamp
                       value:(id)value
{
    if (self = [super init]) {
        _path = [[MTREventPath alloc] initWithPath:path];
        _eventNumber = eventNumber;
        if (!MTRPriorityLevelIsValid(priority)) {
            return nil;
        }
        _priority = @(MTREventPriorityForValidPriorityLevel(priority));
        _timestampValue = @(timestamp.mValue);
        if (timestamp.IsSystem()) {
            _eventTimeType = MTREventTimeTypeSystemUpTime;
            _systemUpTime = MTRTimeIntervalForEventTimestampValue(timestamp.mValue);
        } else if (timestamp.IsEpoch()) {
            _eventTimeType = MTREventTimeTypeTimestampDate;
            _timestampDate = [NSDate dateWithTimeIntervalSince1970:MTRTimeIntervalForEventTimestampValue(timestamp.mValue)];
        } else {
            return nil;
        }
        _value = value;
        _error = nil;
    }
    return self;
}

- (instancetype)initWithPath:(const chip::app::ConcreteEventPath &)path error:(NSError *)error
{
    if (self = [super init]) {
        _path = [[MTREventPath alloc] initWithPath:path];
        // Use some sort of initialized values for our members, even though
        // those values are meaningless in this case.
        _eventNumber = @(0);
        _priority = @(MTREventPriorityDebug);
        _eventTimeType = MTREventTimeTypeSystemUpTime;
        _systemUpTime = 0;
        _timestampDate = nil;
        _value = nil;
        _error = error;
    }
    return self;
}

- (nullable instancetype)initWithResponseValue:(NSDictionary<NSString *, id> *)responseValue
                                         error:(NSError * __autoreleasing *)error
{
    if (!(self = [super init])) {
        return nil;
    }

    // In theory, the types of all the things in the dictionary will be correct
    // if our consumer passes in an actual response-value dictionary, but
    // double-check just to be sure
    if (!CheckMemberOfType(
            responseValue, MTREventPathKey, [MTREventPath class], @"response-value event path is not an MTREventPath.", error)) {
        return nil;
    }
    MTREventPath * path = responseValue[MTREventPathKey];

    id _Nullable value = responseValue[MTRErrorKey];
    if (value != nil) {
        if (!CheckMemberOfType(responseValue, MTRErrorKey, [NSError class], @"response-value error is not an NSError.", error)) {
            return nil;
        }

        return [self initWithPath:[path _asConcretePath] error:value];
    }

    if (!CheckMemberOfType(
            responseValue, MTRDataKey, [NSDictionary class], @"response-value data is not a data-value dictionary.", error)) {
        return nil;
    }
    NSDictionary * data = responseValue[MTRDataKey];

    // Encode the data to TLV and then decode from that, to reuse existing code.
    System::PacketBufferHandle buffer;
    Platform::ScopedMemoryBuffer<uint8_t> flatBuffer;
    TLV::TLVReader reader;
    if (!EncodeDataValueToTLV(buffer, flatBuffer, data, reader, error)) {
        return nil;
    }
    auto eventPath = [path _asConcretePath];

    CHIP_ERROR errorCode = CHIP_ERROR_INTERNAL;
    id decodedValue = MTRDecodeEventPayload(eventPath, reader, &errorCode);
    if (errorCode == CHIP_NO_ERROR) {
        // Validate our other members.
        if (!CheckMemberOfType(
                responseValue, MTREventNumberKey, [NSNumber class], @"response-value event number is not an NSNumber", error)) {
            return nil;
        }
        _eventNumber = responseValue[MTREventNumberKey];

        if (!CheckMemberOfType(
                responseValue, MTREventPriorityKey, [NSNumber class], @"response-value event priority is not an NSNumber", error)) {
            return nil;
        }
        _priority = responseValue[MTREventPriorityKey];

        if (!CheckMemberOfType(responseValue, MTREventTimeTypeKey, [NSNumber class],
                @"response-value event time type is not an NSNumber", error)) {
            return nil;
        }
        NSNumber * wrappedTimeType = responseValue[MTREventTimeTypeKey];
        if (wrappedTimeType.unsignedIntegerValue == MTREventTimeTypeSystemUpTime) {
            if (!CheckMemberOfType(responseValue, MTREventSystemUpTimeKey, [NSNumber class],
                    @"response-value event system uptime time is not an NSNumber", error)) {
                return nil;
            }
            NSNumber * wrappedSystemTime = responseValue[MTREventSystemUpTimeKey];
            _systemUpTime = wrappedSystemTime.doubleValue;
        } else if (wrappedTimeType.unsignedIntegerValue == MTREventTimeTypeTimestampDate) {
            if (!CheckMemberOfType(responseValue, MTREventTimestampDateKey, [NSDate class],
                    @"response-value event timestampe is not an NSDate", error)) {
                return nil;
            }
            _timestampDate = responseValue[MTREventTimestampDateKey];
        } else {
            LogStringAndReturnError([NSString stringWithFormat:@"Invalid event time type: %lu", wrappedTimeType.unsignedLongValue],
                MTRErrorCodeInvalidArgument, error);
            return nil;
        }
        _eventTimeType = static_cast<MTREventTimeType>(wrappedTimeType.unsignedIntegerValue);

        _path = path;
        _value = decodedValue;
        _error = nil;
        return self;
    }

    if (errorCode == CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB) {
        LogStringAndReturnError(@"No known schema for decoding event payload.", MTRErrorCodeUnknownSchema, error);
        return nil;
    }

    // Treat all other errors as schema errors.
    LogStringAndReturnError(@"Event payload decoding failed schema check.", MTRErrorCodeSchemaMismatch, error);
    return nil;
}
@end

@implementation MTREventReport (Deprecated)
- (NSNumber *)timestamp
{
    return _timestampValue;
}
@end

namespace {
void SubscriptionCallback::OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus)
{
    id _Nullable value = nil;
    NSError * _Nullable error = nil;
    if (apStatus != nullptr) {
        error = [MTRError errorForIMStatus:*apStatus];
    } else if (apData == nullptr) {
        error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
    } else {
        CHIP_ERROR err;
        value = MTRDecodeEventPayload(aEventHeader.mPath, *apData, &err);
        if (err == CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB) {
            // We don't know this event; just skip it.
            return;
        }

        if (err != CHIP_NO_ERROR) {
            value = nil;
            error = [MTRError errorForCHIPErrorCode:err];
        }
    }

    if (mEventReports == nil) {
        // Never got a OnReportBegin?  Not much to do other than tear things down.
        ReportError(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    if (error != nil) {
        [mEventReports addObject:[[MTREventReport alloc] initWithPath:aEventHeader.mPath error:error]];
    } else {
        [mEventReports addObject:[[MTREventReport alloc] initWithPath:aEventHeader.mPath
                                                          eventNumber:@(aEventHeader.mEventNumber)
                                                             priority:aEventHeader.mPriorityLevel
                                                            timestamp:aEventHeader.mTimestamp
                                                                value:value]];
    }
}

void SubscriptionCallback::OnAttributeData(
    const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus)
{
    if (aPath.IsListItemOperation()) {
        ReportError(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    id _Nullable value = nil;
    NSError * _Nullable error = nil;
    if (aStatus.mStatus != Status::Success) {
        error = [MTRError errorForIMStatus:aStatus];
    } else if (apData == nullptr) {
        error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
    } else {
        CHIP_ERROR err;
        value = MTRDecodeAttributeValue(aPath, *apData, &err);
        if (err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB) {
            // We don't know this attribute; just skip it.
            return;
        }

        if (err != CHIP_NO_ERROR) {
            value = nil;
            error = [MTRError errorForCHIPErrorCode:err];
        }
    }

    if (mAttributeReports == nil) {
        // Never got a OnReportBegin?  Not much to do other than tear things down.
        ReportError(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    [mAttributeReports addObject:[[MTRAttributeReport alloc] initWithPath:aPath value:value error:error]];
}

} // anonymous namespace
