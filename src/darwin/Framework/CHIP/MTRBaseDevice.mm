/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#import "MTRAttributeCacheContainer_Internal.h"
#import "MTRAttributeTLVValueDecoder_Internal.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRCallbackBridgeBase_internal.h"
#import "MTRCluster.h"
#import "MTRError_Internal.h"
#import "MTREventTLVValueDecoder_Internal.h"
#import "MTRLogging.h"
#include "app/ConcreteAttributePath.h"
#include "app/ConcreteCommandPath.h"
#include "lib/core/CHIPError.h"
#include "lib/core/DataModelTypes.h"

#include <app/AttributePathParams.h>
#include <app/BufferedReadCallback.h>
#include <app/ClusterStateCache.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadClient.h>
#include <app/util/error-mapping.h>
#include <controller/ReadInteraction.h>
#include <controller/WriteInteraction.h>

#include <memory>

typedef void (^SubscriptionEstablishedHandler)(void);

using namespace chip;
using namespace chip::app;
using namespace chip::Protocols::InteractionModel;
using chip::Messaging::ExchangeManager;
using chip::Optional;
using chip::SessionHandle;

NSString * const MTRAttributePathKey = @"attributePath";
NSString * const MTRCommandPathKey = @"commandPath";
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

class MTRDataValueDictionaryCallbackBridge;

@interface MTRBaseDevice ()

@property (nonatomic, readonly, assign, nullable) chip::DeviceProxy * cppPASEDevice;
@property (nonatomic, readwrite) NSMutableDictionary * reportHandlerBridges;

- (chip::NodeId)deviceID;
@end

@interface MTRReadClientContainer : NSObject
@property (nonatomic, readwrite) app::ReadClient * readClientPtr;
@property (nonatomic, readwrite) app::AttributePathParams * pathParams;
@property (nonatomic, readwrite) uint64_t deviceId;
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

static void PurgeReadClientContainers(uint64_t deviceId, dispatch_queue_t queue, void (^_Nullable completion)(void))
{
    InitializeReadClientContainers();

    NSMutableArray<MTRReadClientContainer *> * listToDelete;
    NSNumber * key = [NSNumber numberWithUnsignedLongLong:deviceId];
    [readClientContainersLock lock];
    listToDelete = readClientContainers[key];
    [readClientContainers removeObjectForKey:key];
    [readClientContainersLock unlock];

    // Destroy read clients in the work queue
    dispatch_async(DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
        for (MTRReadClientContainer * container in listToDelete) {
            if (container.readClientPtr) {
                Platform::Delete(container.readClientPtr);
                container.readClientPtr = nullptr;
            }
            if (container.pathParams) {
                Platform::Delete(container.pathParams);
                container.pathParams = nullptr;
            }
        }
        [listToDelete removeAllObjects];
        if (completion) {
            dispatch_async(queue, completion);
        }
    });
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

#ifdef DEBUG
// This function is for unit testing only. This function closes all read clients.
static void CauseReadClientFailure(uint64_t deviceId, dispatch_queue_t queue, void (^_Nullable completion)(void))
{
    InitializeReadClientContainers();

    NSMutableArray<MTRReadClientContainer *> * listToFail;
    NSNumber * key = [NSNumber numberWithUnsignedLongLong:deviceId];
    [readClientContainersLock lock];
    listToFail = readClientContainers[key];
    [readClientContainers removeObjectForKey:key];
    [readClientContainersLock unlock];

    dispatch_async(DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
        for (MTRReadClientContainer * container in listToFail) {
            // Send auto resubscribe request again by read clients, which must fail.
            chip::app::ReadPrepareParams readParams;
            if (container.readClientPtr) {
                container.readClientPtr->SendAutoResubscribeRequest(std::move(readParams));
            }
        }
        if (completion) {
            dispatch_async(queue, completion);
        }
    });
}
#endif

@implementation MTRReadClientContainer
- (void)onDone
{
    if (_readClientPtr) {
        Platform::Delete(_readClientPtr);
        _readClientPtr = nullptr;
    }
    if (_pathParams) {
        Platform::Delete(_pathParams);
        _pathParams = nullptr;
    }
    PurgeCompletedReadClientContainers(_deviceId);
}

- (void)dealloc
{
    if (_readClientPtr) {
        Platform::Delete(_readClientPtr);
        _readClientPtr = nullptr;
    }
    if (_pathParams) {
        Platform::Delete(_pathParams);
        _pathParams = nullptr;
    }
}
@end

@implementation MTRBaseDevice

- (instancetype)initWithPASEDevice:(chip::DeviceProxy *)device controller:(MTRDeviceController *)controller
{
    if (self = [super init]) {
        chip::Optional<SessionHandle> session = device->GetSecureSession();
        if (!session.HasValue()) {
            MTR_LOG_ERROR("Failing to initialize MTRBaseDevice: no secure session");
            return nil;
        }

        if (!session.Value()->AsSecureSession()->IsPASESession()) {
            MTR_LOG_ERROR("Failing to initialize MTRBaseDevice: not a PASE session");
            return nil;
        }

        _cppPASEDevice = device;
        _nodeID = kUndefinedNodeId;
        _deviceController = controller;
    }
    return self;
}

- (instancetype)initWithNodeID:(chip::NodeId)nodeID controller:(MTRDeviceController *)controller
{
    if (self = [super init]) {
        _cppPASEDevice = nil;
        _nodeID = nodeID;
        _deviceController = controller;
    }
    return self;
}

- (chip::DeviceProxy * _Nullable)paseDevice
{
    return _cppPASEDevice;
}

- (chip::NodeId)deviceID
{
    if (_cppPASEDevice != nullptr) {
        return _cppPASEDevice->GetDeviceId();
    }

    return self.nodeID;
}

- (void)invalidateCASESession
{
    if (self.paseDevice) {
        return;
    }

    [self.deviceController invalidateCASESessionForNode:self.deviceID];
}

typedef void (^ReportCallback)(NSArray * _Nullable value, NSError * _Nullable error);
typedef void (^DataReportCallback)(NSArray * value);
typedef void (^ErrorCallback)(NSError * error);

namespace {

class SubscriptionCallback final : public ClusterStateCache::Callback {
public:
    SubscriptionCallback(dispatch_queue_t queue, DataReportCallback attributeReportCallback, DataReportCallback eventReportCallback,
        ErrorCallback errorCallback, SubscriptionEstablishedHandler _Nullable subscriptionEstablishedHandler)
        : mQueue(queue)
        , mAttributeReportCallback(attributeReportCallback)
        , mEventReportCallback(eventReportCallback)
        , mErrorCallback(errorCallback)
        , mSubscriptionEstablishedHandler(subscriptionEstablishedHandler)
        , mBufferedReadAdapter(*this)
    {
    }

    SubscriptionCallback(dispatch_queue_t queue, DataReportCallback attributeReportCallback, DataReportCallback eventReportCallback,
        ErrorCallback errorCallback, SubscriptionEstablishedHandler _Nullable subscriptionEstablishedHandler,
        void (^onDoneHandler)(void))
        : mQueue(queue)
        , mAttributeReportCallback(attributeReportCallback)
        , mEventReportCallback(eventReportCallback)
        , mErrorCallback(errorCallback)
        , mSubscriptionEstablishedHandler(subscriptionEstablishedHandler)
        , mBufferedReadAdapter(*this)
        , mOnDoneHandler(onDoneHandler)
    {
    }

    BufferedReadCallback & GetBufferedCallback() { return mBufferedReadAdapter; }

    // We need to exist to get a ReadClient, so can't take this as a constructor argument.
    void AdoptReadClient(std::unique_ptr<ReadClient> aReadClient) { mReadClient = std::move(aReadClient); }
    void AdoptAttributeCache(std::unique_ptr<ClusterStateCache> aAttributeCache) { mAttributeCache = std::move(aAttributeCache); }

private:
    void OnReportBegin() override;

    void OnReportEnd() override;

    void OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus) override;

    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;

    void OnError(CHIP_ERROR aError) override;

    void OnDone(ReadClient * aReadClient) override;

    void OnDeallocatePaths(ReadPrepareParams && aReadPrepareParams) override;

    void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override;

    void ReportData();
    void ReportError(CHIP_ERROR err);
    void ReportError(const StatusIB & status);
    void ReportError(NSError * _Nullable err);

private:
    dispatch_queue_t mQueue;
    DataReportCallback _Nullable mAttributeReportCallback = nil;
    DataReportCallback _Nullable mEventReportCallback = nil;
    // We set mErrorCallback to nil when queueing error reports, so we
    // make sure to only report one error.
    ErrorCallback _Nullable mErrorCallback = nil;
    SubscriptionEstablishedHandler _Nullable mSubscriptionEstablishedHandler;
    BufferedReadCallback mBufferedReadAdapter;
    NSMutableArray * _Nullable mAttributeReports = nil;
    NSMutableArray * _Nullable mEventReports = nil;

    // Our lifetime management is a little complicated.  On error we
    // attempt to delete the ReadClient, but asynchronously.  While
    // that's pending, someone else (e.g. an error it runs into) could
    // delete it too.  And if someone else does attempt to delete it, we want to
    // make sure we delete ourselves as well.
    //
    // To handle this, enforce the following rules:
    //
    // 1) We guarantee that mErrorCallback is only invoked with an error once.
    // 2) We ensure that we delete ourselves and the passed in ReadClient only from OnDone or a queued-up
    //    error callback, but not both, by tracking whether we have a queued-up
    //    deletion.
    std::unique_ptr<ReadClient> mReadClient;
    std::unique_ptr<ClusterStateCache> mAttributeCache;
    bool mHaveQueuedDeletion = false;
    void (^mOnDoneHandler)(void) = nil;
};

} // anonymous namespace

- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
                     params:(nullable MTRSubscribeParams *)params
             cacheContainer:(MTRAttributeCacheContainer * _Nullable)attributeCacheContainer
     attributeReportHandler:(nullable void (^)(NSArray * value))attributeReportHandler
         eventReportHandler:(nullable void (^)(NSArray * value))eventReportHandler
               errorHandler:(void (^)(NSError * error))errorHandler
    subscriptionEstablished:(nullable void (^)(void))subscriptionEstablishedHandler
{
    if (self.paseDevice != nil) {
        // We don't support subscriptions over PASE.
        dispatch_async(queue, ^{
            errorHandler([MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // Copy params before going async.
    params = [params copy];

    [self.deviceController getSessionForNode:self.nodeID
                           completionHandler:^(ExchangeManager * _Nullable exchangeManager, const Optional<SessionHandle> & session,
                               NSError * _Nullable error) {
                               if (error != nil) {
                                   dispatch_async(queue, ^{
                                       errorHandler(error);
                                   });
                                   return;
                               }

                               // Wildcard endpoint, cluster, attribute, event.
                               auto attributePath = std::make_unique<AttributePathParams>();
                               auto eventPath = std::make_unique<EventPathParams>();
                               ReadPrepareParams readParams(session.Value());
                               readParams.mMinIntervalFloorSeconds = minInterval;
                               readParams.mMaxIntervalCeilingSeconds = maxInterval;
                               readParams.mpAttributePathParamsList = attributePath.get();
                               readParams.mAttributePathParamsListSize = 1;
                               readParams.mpEventPathParamsList = eventPath.get();
                               readParams.mEventPathParamsListSize = 1;
                               readParams.mKeepSubscriptions = [params.keepPreviousSubscriptions boolValue];

                               std::unique_ptr<SubscriptionCallback> callback;
                               std::unique_ptr<ReadClient> readClient;
                               std::unique_ptr<ClusterStateCache> attributeCache;
                               if (attributeCacheContainer) {
                                   __weak MTRAttributeCacheContainer * weakPtr = attributeCacheContainer;
                                   callback = std::make_unique<SubscriptionCallback>(queue, attributeReportHandler,
                                       eventReportHandler, errorHandler, subscriptionEstablishedHandler, ^{
                                           MTRAttributeCacheContainer * container = weakPtr;
                                           if (container) {
                                               container.cppAttributeCache = nullptr;
                                           }
                                       });
                                   attributeCache = std::make_unique<ClusterStateCache>(*callback.get());
                                   readClient = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), exchangeManager,
                                       attributeCache->GetBufferedCallback(), ReadClient::InteractionType::Subscribe);
                               } else {
                                   callback = std::make_unique<SubscriptionCallback>(queue, attributeReportHandler,
                                       eventReportHandler, errorHandler, subscriptionEstablishedHandler);
                                   readClient = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), exchangeManager,
                                       callback->GetBufferedCallback(), ReadClient::InteractionType::Subscribe);
                               }

                               CHIP_ERROR err;
                               if (params != nil && params.autoResubscribe != nil && ![params.autoResubscribe boolValue]) {
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

                               if (attributeCacheContainer) {
                                   attributeCacheContainer.cppAttributeCache = attributeCache.get();
                                   // ClusterStateCache will be deleted when OnDone is called or an error is encountered as well.
                                   callback->AdoptAttributeCache(std::move(attributeCache));
                               }
                               // Callback and ReadClient will be deleted when OnDone is called or an error is
                               // encountered.
                               callback->AdoptReadClient(std::move(readClient));
                               callback.release();
                           }];
}

// Convert TLV data into data-value dictionary as described in MTRDeviceResponseHandler
id _Nullable MTRDecodeDataValueDictionaryFromCHIPTLV(chip::TLV::TLVReader * data)
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
        return [NSDictionary dictionaryWithObjectsAndKeys:MTRSignedIntegerValueType, MTRTypeKey, [NSNumber numberWithLongLong:val],
                             MTRValueKey, nil];
    }
    case chip::TLV::kTLVType_UnsignedInteger: {
        uint64_t val;
        CHIP_ERROR err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV unsigned integer decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:MTRUnsignedIntegerValueType, MTRTypeKey,
                             [NSNumber numberWithUnsignedLongLong:val], MTRValueKey, nil];
    }
    case chip::TLV::kTLVType_Boolean: {
        bool val;
        CHIP_ERROR err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV boolean decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary
            dictionaryWithObjectsAndKeys:MTRBooleanValueType, MTRTypeKey, [NSNumber numberWithBool:val], MTRValueKey, nil];
    }
    case chip::TLV::kTLVType_FloatingPointNumber: {
        // Try float first
        float floatValue;
        CHIP_ERROR err = data->Get(floatValue);
        if (err == CHIP_NO_ERROR) {
            return @ { MTRTypeKey : MTRFloatValueType, MTRValueKey : [NSNumber numberWithFloat:floatValue] };
        }
        double val;
        err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV floating point decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary
            dictionaryWithObjectsAndKeys:MTRDoubleValueType, MTRTypeKey, [NSNumber numberWithDouble:val], MTRValueKey, nil];
    }
    case chip::TLV::kTLVType_UTF8String: {
        uint32_t len = data->GetLength();
        const uint8_t * ptr;
        CHIP_ERROR err = data->GetDataPtr(ptr);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV UTF8String decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:MTRUTF8StringValueType, MTRTypeKey,
                             [[NSString alloc] initWithBytes:ptr length:len encoding:NSUTF8StringEncoding], MTRValueKey, nil];
    }
    case chip::TLV::kTLVType_ByteString: {
        uint32_t len = data->GetLength();
        const uint8_t * ptr;
        CHIP_ERROR err = data->GetDataPtr(ptr);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Error(%s): TLV ByteString decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:MTROctetStringValueType, MTRTypeKey,
                             [NSData dataWithBytes:ptr length:len], MTRValueKey, nil];
    }
    case chip::TLV::kTLVType_Null: {
        return [NSDictionary dictionaryWithObjectsAndKeys:MTRNullValueType, MTRTypeKey, nil];
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
                MTR_LOG_ERROR("Error when decoding TLV container");
                return nil;
            }
            NSMutableDictionary * arrayElement = [NSMutableDictionary dictionary];
            [arrayElement setObject:value forKey:MTRDataKey];
            if (dataTLVType == chip::TLV::kTLVType_Structure) {
                [arrayElement setObject:[NSNumber numberWithUnsignedLong:TagNumFromTag(tag)] forKey:MTRContextTagKey];
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
        return [NSDictionary dictionaryWithObjectsAndKeys:typeName, MTRTypeKey, array, MTRValueKey, nil];
    }
    default:
        MTR_LOG_ERROR("Error: Unsupported TLV type for conversion: %u", (unsigned) data->GetType());
        return nil;
    }
}

static CHIP_ERROR MTREncodeTLVFromDataValueDictionary(id object, chip::TLV::TLVWriter & writer, chip::TLV::Tag tag)
{
    if (![object isKindOfClass:[NSDictionary class]]) {
        MTR_LOG_ERROR("Error: Unsupported object to encode: %@", [object class]);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    NSString * typeName = ((NSDictionary *) object)[MTRTypeKey];
    id value = ((NSDictionary *) object)[MTRValueKey];
    if (!typeName) {
        MTR_LOG_ERROR("Error: Object to encode is corrupt");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if ([typeName isEqualToString:MTRSignedIntegerValueType]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt signed integer type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.Put(tag, [value longLongValue]);
    }
    if ([typeName isEqualToString:MTRUnsignedIntegerValueType]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt unsigned integer type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.Put(tag, [value unsignedLongLongValue]);
    }
    if ([typeName isEqualToString:MTRBooleanValueType]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt boolean type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.Put(tag, static_cast<bool>([value boolValue]));
    }
    if ([typeName isEqualToString:MTRFloatValueType]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt float type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.Put(tag, [value floatValue]);
    }
    if ([typeName isEqualToString:MTRDoubleValueType]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt double type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.Put(tag, [value doubleValue]);
    }
    if ([typeName isEqualToString:MTRNullValueType]) {
        return writer.PutNull(tag);
    }
    if ([typeName isEqualToString:MTRUTF8StringValueType]) {
        if (![value isKindOfClass:[NSString class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt UTF8 string type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.PutString(tag, [value cStringUsingEncoding:NSUTF8StringEncoding]);
    }
    if ([typeName isEqualToString:MTROctetStringValueType]) {
        if (![value isKindOfClass:[NSData class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt octet string type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.Put(tag, chip::ByteSpan(static_cast<const uint8_t *>([value bytes]), [value length]));
    }
    if ([typeName isEqualToString:MTRStructureValueType]) {
        if (![value isKindOfClass:[NSArray class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt structure type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, chip::TLV::kTLVType_Structure, outer));
        for (id element in value) {
            if (![element isKindOfClass:[NSDictionary class]]) {
                MTR_LOG_ERROR("Error: Structure element to encode has corrupt type: %@", [element class]);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            NSNumber * elementTag = element[MTRContextTagKey];
            id elementValue = element[MTRDataKey];
            if (!elementTag || !elementValue) {
                MTR_LOG_ERROR("Error: Structure element to encode has corrupt value: %@", element);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            ReturnErrorOnFailure(
                MTREncodeTLVFromDataValueDictionary(elementValue, writer, chip::TLV::ContextTag([elementTag unsignedCharValue])));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
    if ([typeName isEqualToString:MTRArrayValueType]) {
        if (![value isKindOfClass:[NSArray class]]) {
            MTR_LOG_ERROR("Error: Object to encode has corrupt array type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, chip::TLV::kTLVType_Array, outer));
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
            ReturnErrorOnFailure(MTREncodeTLVFromDataValueDictionary(elementValue, writer, chip::TLV::AnonymousTag()));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
    MTR_LOG_ERROR("Error: Unsupported type to encode: %@", typeName);
    return CHIP_ERROR_INVALID_ARGUMENT;
}

// Callback type to pass data value as an NSObject
typedef void (*MTRDataValueDictionaryCallback)(void * context, id value);
typedef void (*MTRErrorCallback)(void * context, CHIP_ERROR error);

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
        return MTREncodeTLVFromDataValueDictionary(decodedObj, writer, tag);
    }

    static constexpr bool kIsFabricScoped = false;

    static bool MustUseTimedInvoke() { return false; }

    id _Nullable GetDecodedObject() const { return decodedObj; }

private:
    id _Nullable decodedObj;
};

// Callback bridge for MTRDataValueDictionaryCallback
class MTRDataValueDictionaryCallbackBridge : public MTRCallbackBridge<MTRDataValueDictionaryCallback> {
public:
    MTRDataValueDictionaryCallbackBridge(dispatch_queue_t queue, MTRBaseDevice * device, MTRDeviceResponseHandler handler,
        MTRActionBlock action, bool keepAlive = false)
        : MTRCallbackBridge<MTRDataValueDictionaryCallback>(queue, device, handler, action, OnSuccessFn, keepAlive) {};

    static void OnSuccessFn(void * context, id value) { DispatchSuccess(context, value); }
};

template <typename DecodableAttributeType> class BufferedReadAttributeCallback final : public app::ReadClient::Callback {
public:
    using OnSuccessCallbackType
        = std::function<void(const app::ConcreteDataAttributePath & aPath, const DecodableAttributeType & aData)>;
    using OnErrorCallbackType = std::function<void(const app::ConcreteDataAttributePath * aPath, CHIP_ERROR aError)>;
    using OnDoneCallbackType = std::function<void(BufferedReadAttributeCallback * callback)>;
    using OnSubscriptionEstablishedCallbackType = std::function<void()>;

    BufferedReadAttributeCallback(ClusterId aClusterId, AttributeId aAttributeId, OnSuccessCallbackType aOnSuccess,
        OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone,
        OnSubscriptionEstablishedCallbackType aOnSubscriptionEstablished = nullptr)
        : mClusterId(aClusterId)
        , mAttributeId(aAttributeId)
        , mOnSuccess(aOnSuccess)
        , mOnError(aOnError)
        , mOnDone(aOnDone)
        , mOnSubscriptionEstablished(aOnSubscriptionEstablished)
        , mBufferedReadAdapter(*this)
    {
    }

    app::BufferedReadCallback & GetBufferedCallback() { return mBufferedReadAdapter; }

    void AdoptReadClient(Platform::UniquePtr<app::ReadClient> aReadClient) { mReadClient = std::move(aReadClient); }

private:
    void OnAttributeData(
        const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const app::StatusIB & aStatus) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        DecodableAttributeType value;

        //
        // We shouldn't be getting list item operations in the provided path since that should be handled by the buffered read
        // callback. If we do, that's a bug.
        //
        VerifyOrDie(!aPath.IsListItemOperation());

        VerifyOrExit(aStatus.IsSuccess(), err = aStatus.ToChipError());
        VerifyOrExit((aPath.mClusterId == mClusterId || mClusterId == kInvalidClusterId)
                && (aPath.mAttributeId == mAttributeId || mAttributeId == kInvalidAttributeId),
            err = CHIP_ERROR_SCHEMA_MISMATCH);
        VerifyOrExit(apData != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        SuccessOrExit(err = app::DataModel::Decode(*apData, value));

        mOnSuccess(aPath, value);

    exit:
        if (err != CHIP_NO_ERROR) {
            mOnError(&aPath, err);
        }
    }

    void OnError(CHIP_ERROR aError) override { mOnError(nullptr, aError); }

    void OnDone(ReadClient *) override { mOnDone(this); }

    void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override
    {
        if (mOnSubscriptionEstablished) {
            mOnSubscriptionEstablished();
        }
    }

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override {}

    ClusterId mClusterId;
    AttributeId mAttributeId;
    OnSuccessCallbackType mOnSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;
    OnSubscriptionEstablishedCallbackType mOnSubscriptionEstablished;
    app::BufferedReadCallback mBufferedReadAdapter;
    Platform::UniquePtr<app::ReadClient> mReadClient;
};

- (void)readAttributeWithEndpointId:(NSNumber *)endpointId
                          clusterId:(NSNumber *)clusterId
                        attributeId:(NSNumber *)attributeId
                             params:(MTRReadParams * _Nullable)params
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion
{
    endpointId = (endpointId == nil) ? nil : [endpointId copy];
    clusterId = (clusterId == nil) ? nil : [clusterId copy];
    attributeId = (attributeId == nil) ? nil : [attributeId copy];
    params = (params == nil) ? nil : [params copy];
    new MTRDataValueDictionaryCallbackBridge(clientQueue, self, completion,
        ^(ExchangeManager & exchangeManager, const SessionHandle & session, chip::Callback::Cancelable * success,
            chip::Callback::Cancelable * failure) {
            auto successFn = chip::Callback::Callback<MTRDataValueDictionaryCallback>::FromCancelable(success);
            auto failureFn = chip::Callback::Callback<MTRErrorCallback>::FromCancelable(failure);
            auto context = successFn->mContext;
            auto successCb = successFn->mCall;
            auto failureCb = failureFn->mCall;
            auto resultArray = [[NSMutableArray alloc] init];
            auto resultSuccess = [[NSMutableArray alloc] init];
            auto resultFailure = [[NSMutableArray alloc] init];
            auto onSuccessCb = [resultArray, resultSuccess](const app::ConcreteAttributePath & attribPath,
                                   const MTRDataValueDictionaryDecodableType & aData) {
                [resultArray addObject:@ {
                    MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:attribPath],
                    MTRDataKey : aData.GetDecodedObject()
                }];
                if ([resultSuccess count] == 0) {
                    [resultSuccess addObject:[NSNumber numberWithBool:YES]];
                }
            };

            auto onFailureCb = [resultArray, resultFailure](const app::ConcreteAttributePath * attribPath, CHIP_ERROR aError) {
                if (attribPath) {
                    [resultArray addObject:@ {
                        MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:*attribPath],
                        MTRErrorKey : [MTRError errorForCHIPErrorCode:aError]
                    }];
                } else if ([resultFailure count] == 0) {
                    [resultFailure addObject:[MTRError errorForCHIPErrorCode:aError]];
                }
            };

            app::AttributePathParams attributePath;
            if (endpointId) {
                attributePath.mEndpointId = static_cast<chip::EndpointId>([endpointId unsignedShortValue]);
            }
            if (clusterId) {
                attributePath.mClusterId = static_cast<chip::ClusterId>([clusterId unsignedLongValue]);
            }
            if (attributeId) {
                attributePath.mAttributeId = static_cast<chip::AttributeId>([attributeId unsignedLongValue]);
            }
            app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
            CHIP_ERROR err = CHIP_NO_ERROR;

            chip::app::ReadPrepareParams readParams(session);
            readParams.mpAttributePathParamsList = &attributePath;
            readParams.mAttributePathParamsListSize = 1;
            readParams.mIsFabricFiltered = params == nil || params.fabricFiltered == nil || [params.fabricFiltered boolValue];

            auto onDone = [resultArray, resultSuccess, resultFailure, context, successCb, failureCb](
                              BufferedReadAttributeCallback<MTRDataValueDictionaryDecodableType> * callback) {
                if ([resultFailure count] > 0 || [resultSuccess count] == 0) {
                    // Failure
                    if (failureCb) {
                        if ([resultFailure count] > 0) {
                            failureCb(context, [MTRError errorToCHIPErrorCode:resultFailure[0]]);
                        } else if ([resultArray count] > 0) {
                            failureCb(context, [MTRError errorToCHIPErrorCode:resultArray[0][MTRErrorKey]]);
                        } else {
                            failureCb(context, CHIP_ERROR_READ_FAILED);
                        }
                    }
                } else {
                    // Success
                    if (successCb) {
                        successCb(context, resultArray);
                    }
                }
                chip::Platform::Delete(callback);
            };

            auto callback = chip::Platform::MakeUnique<BufferedReadAttributeCallback<MTRDataValueDictionaryDecodableType>>(
                attributePath.mClusterId, attributePath.mAttributeId, onSuccessCb, onFailureCb, onDone, nullptr);
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
            return err;
        });
}

- (void)writeAttributeWithEndpointId:(NSNumber *)endpointId
                           clusterId:(NSNumber *)clusterId
                         attributeId:(NSNumber *)attributeId
                               value:(id)value
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
                         clientQueue:(dispatch_queue_t)clientQueue
                          completion:(MTRDeviceResponseHandler)completion
{
    new MTRDataValueDictionaryCallbackBridge(clientQueue, self, completion,
        ^(ExchangeManager & exchangeManager, const SessionHandle & session, chip::Callback::Cancelable * success,
            chip::Callback::Cancelable * failure) {
            auto successFn = chip::Callback::Callback<MTRDataValueDictionaryCallback>::FromCancelable(success);
            auto failureFn = chip::Callback::Callback<MTRErrorCallback>::FromCancelable(failure);
            auto context = successFn->mContext;
            auto successCb = successFn->mCall;
            auto failureCb = failureFn->mCall;
            auto resultArray = [[NSMutableArray alloc] init];
            auto resultSuccess = [[NSMutableArray alloc] init];
            auto resultFailure = [[NSMutableArray alloc] init];
            auto onSuccessCb = [resultArray, resultSuccess](const app::ConcreteAttributePath & attribPath) {
                [resultArray addObject:@ { MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:attribPath] }];
                if ([resultSuccess count] == 0) {
                    [resultSuccess addObject:[NSNumber numberWithBool:YES]];
                }
            };

            auto onFailureCb = [resultArray, resultFailure](const app::ConcreteAttributePath * attribPath, CHIP_ERROR aError) {
                if (attribPath) {
                    [resultArray addObject:@ {
                        MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:*attribPath],
                        MTRErrorKey : [MTRError errorForCHIPErrorCode:aError],
                    }];
                } else {
                    if ([resultFailure count] == 0) {
                        [resultFailure addObject:[MTRError errorForCHIPErrorCode:aError]];
                    }
                }
            };

            auto onDoneCb
                = [context, successCb, failureCb, resultArray, resultSuccess, resultFailure](app::WriteClient * pWriteClient) {
                      if ([resultFailure count] > 0 || [resultSuccess count] == 0) {
                          // Failure
                          if (failureCb) {
                              if ([resultFailure count] > 0) {
                                  failureCb(context, [MTRError errorToCHIPErrorCode:resultFailure[0]]);
                              } else if ([resultArray count] > 0) {
                                  failureCb(context, [MTRError errorToCHIPErrorCode:resultArray[0][MTRErrorKey]]);
                              } else {
                                  failureCb(context, CHIP_ERROR_WRITE_FAILED);
                              }
                          }
                      } else {
                          // Success
                          if (successCb) {
                              successCb(context, resultArray);
                          }
                      }
                  };

            return chip::Controller::WriteAttribute<MTRDataValueDictionaryDecodableType>(session,
                static_cast<chip::EndpointId>([endpointId unsignedShortValue]),
                static_cast<chip::ClusterId>([clusterId unsignedLongValue]),
                static_cast<chip::AttributeId>([attributeId unsignedLongValue]), MTRDataValueDictionaryDecodableType(value),
                onSuccessCb, onFailureCb, (timeoutMs == nil) ? NullOptional : Optional<uint16_t>([timeoutMs unsignedShortValue]),
                onDoneCb, NullOptional);
        });
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

    void OnError(const app::CommandSender * apCommandSender, CHIP_ERROR aError) override { mOnError(aError); }

    void OnDone(app::CommandSender * apCommandSender) override { mOnDone(apCommandSender); }

    OnSuccessCallbackType mOnSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;
    chip::ClusterId mClusterId;
    // Id of the command we send.
    chip::CommandId mCommandId;
};

void NSObjectCommandCallback::OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aCommandPath,
    const app::StatusIB & aStatus, TLV::TLVReader * aReader)
{
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

- (void)invokeCommandWithEndpointId:(NSNumber *)endpointId
                          clusterId:(NSNumber *)clusterId
                          commandId:(NSNumber *)commandId
                      commandFields:(id)commandFields
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion
{
    endpointId = (endpointId == nil) ? nil : [endpointId copy];
    clusterId = (clusterId == nil) ? nil : [clusterId copy];
    commandId = (commandId == nil) ? nil : [commandId copy];
    // TODO: This is not going to deep-copy the NSArray instances in
    // commandFields.  We need to do something smarter here.
    commandFields = (commandFields == nil) ? nil : [commandFields copy];
    timeoutMs = (timeoutMs == nil) ? nil : [timeoutMs copy];

    new MTRDataValueDictionaryCallbackBridge(clientQueue, self, completion,
        ^(ExchangeManager & exchangeManager, const SessionHandle & session, chip::Callback::Cancelable * success,
            chip::Callback::Cancelable * failure) {
            auto successFn = chip::Callback::Callback<MTRDataValueDictionaryCallback>::FromCancelable(success);
            auto failureFn = chip::Callback::Callback<MTRErrorCallback>::FromCancelable(failure);
            auto context = successFn->mContext;
            auto successCb = successFn->mCall;
            auto failureCb = failureFn->mCall;
            auto resultArray = [[NSMutableArray alloc] init];
            auto resultSuccess = [[NSMutableArray alloc] init];
            auto resultFailure = [[NSMutableArray alloc] init];
            auto onSuccessCb = [resultArray, resultSuccess](const app::ConcreteCommandPath & commandPath,
                                   const app::StatusIB & status, const MTRDataValueDictionaryDecodableType & responseData) {
                if (responseData.GetDecodedObject()) {
                    [resultArray addObject:@ {
                        MTRCommandPathKey : [[MTRCommandPath alloc] initWithPath:commandPath],
                        MTRDataKey : responseData.GetDecodedObject()
                    }];
                } else {
                    [resultArray addObject:@ { MTRCommandPathKey : [[MTRCommandPath alloc] initWithPath:commandPath] }];
                }
                if ([resultSuccess count] == 0) {
                    [resultSuccess addObject:[NSNumber numberWithBool:YES]];
                }
            };

            auto onFailureCb = [resultFailure](CHIP_ERROR aError) {
                if ([resultFailure count] == 0) {
                    [resultFailure addObject:[MTRError errorForCHIPErrorCode:aError]];
                }
            };

            app::CommandPathParams commandPath = { static_cast<chip::EndpointId>([endpointId unsignedShortValue]), 0,
                static_cast<chip::ClusterId>([clusterId unsignedLongValue]),
                static_cast<chip::CommandId>([commandId unsignedLongValue]), (app::CommandPathFlags::kEndpointIdValid) };

            auto decoder = chip::Platform::MakeUnique<NSObjectCommandCallback>(
                commandPath.mClusterId, commandPath.mCommandId, onSuccessCb, onFailureCb);
            VerifyOrReturnError(decoder != nullptr, CHIP_ERROR_NO_MEMORY);

            auto rawDecoderPtr = decoder.get();
            auto onDoneCb = [rawDecoderPtr, context, successCb, failureCb, resultArray, resultSuccess, resultFailure](
                                app::CommandSender * commandSender) {
                if ([resultFailure count] > 0 || [resultSuccess count] == 0) {
                    // Failure
                    if (failureCb) {
                        if ([resultFailure count] > 0) {
                            failureCb(context, [MTRError errorToCHIPErrorCode:resultFailure[0]]);
                        } else {
                            failureCb(context, CHIP_ERROR_WRITE_FAILED);
                        }
                    }
                } else {
                    // Success
                    if (successCb) {
                        successCb(context, resultArray);
                    }
                }
                chip::Platform::Delete(commandSender);
                chip::Platform::Delete(rawDecoderPtr);
            };

            decoder->SetOnDoneCallback(onDoneCb);

            bool isTimedRequest = (timeoutMs != nil);
            auto commandSender = chip::Platform::MakeUnique<app::CommandSender>(decoder.get(), &exchangeManager, isTimedRequest);
            VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_NO_MEMORY);

            ReturnErrorOnFailure(commandSender->AddRequestData(commandPath, MTRDataValueDictionaryDecodableType(commandFields),
                (timeoutMs == nil) ? NullOptional : Optional<uint16_t>([timeoutMs unsignedShortValue])));
            ReturnErrorOnFailure(commandSender->SendCommandRequest(session));

            decoder.release();
            commandSender.release();
            return CHIP_NO_ERROR;
        });
}

- (void)subscribeAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                               clusterId:(NSNumber * _Nullable)clusterId
                             attributeId:(NSNumber * _Nullable)attributeId
                             minInterval:(NSNumber *)minInterval
                             maxInterval:(NSNumber *)maxInterval
                                  params:(MTRSubscribeParams * _Nullable)params
                             clientQueue:(dispatch_queue_t)clientQueue
                           reportHandler:(MTRDeviceResponseHandler)reportHandler
                 subscriptionEstablished:(SubscriptionEstablishedHandler)subscriptionEstablishedHandler
{
    if (self.paseDevice != nil) {
        // We don't support subscriptions over PASE.
        dispatch_async(clientQueue, ^{
            reportHandler(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }

    // Copy params before going async.
    endpointId = (endpointId == nil) ? nil : [endpointId copy];
    clusterId = (clusterId == nil) ? nil : [clusterId copy];
    attributeId = (attributeId == nil) ? nil : [attributeId copy];
    minInterval = (minInterval == nil) ? nil : [minInterval copy];
    maxInterval = (maxInterval == nil) ? nil : [maxInterval copy];
    params = (params == nil) ? nil : [params copy];

    [self.deviceController
        getSessionForNode:self.nodeID
        completionHandler:^(
            ExchangeManager * _Nullable exchangeManager, const Optional<SessionHandle> & session, NSError * _Nullable error) {
            if (error != nil) {
                if (reportHandler) {
                    dispatch_async(clientQueue, ^{
                        reportHandler(nil, error);
                    });
                }
                return;
            }

            auto onReportCb = [clientQueue, reportHandler](
                                  const app::ConcreteAttributePath & attribPath, const MTRDataValueDictionaryDecodableType & data) {
                id valueObject = data.GetDecodedObject();
                app::ConcreteAttributePath pathCopy = attribPath;
                dispatch_async(clientQueue, ^{
                    reportHandler(
                        @[ @ { MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:pathCopy], MTRDataKey : valueObject } ],
                        nil);
                });
            };

            auto establishedOrFailed = chip::Platform::MakeShared<BOOL>(NO);
            auto onFailureCb = [establishedOrFailed, clientQueue, subscriptionEstablishedHandler, reportHandler](
                                   const app::ConcreteAttributePath * attribPath, CHIP_ERROR error) {
                if (!(*establishedOrFailed)) {
                    *establishedOrFailed = YES;
                    if (subscriptionEstablishedHandler) {
                        dispatch_async(clientQueue, subscriptionEstablishedHandler);
                    }
                }
                if (reportHandler) {
                    dispatch_async(clientQueue, ^{
                        reportHandler(nil, [MTRError errorForCHIPErrorCode:error]);
                    });
                }
            };

            auto onEstablishedCb = [establishedOrFailed, clientQueue, subscriptionEstablishedHandler]() {
                if (*establishedOrFailed) {
                    return;
                }
                *establishedOrFailed = YES;
                if (subscriptionEstablishedHandler) {
                    dispatch_async(clientQueue, subscriptionEstablishedHandler);
                }
            };

            MTRReadClientContainer * container = [[MTRReadClientContainer alloc] init];
            container.deviceId = [self deviceID];
            container.pathParams = Platform::New<app::AttributePathParams>();
            if (endpointId) {
                container.pathParams->mEndpointId = static_cast<chip::EndpointId>([endpointId unsignedShortValue]);
            }
            if (clusterId) {
                container.pathParams->mClusterId = static_cast<chip::ClusterId>([clusterId unsignedLongValue]);
            }
            if (attributeId) {
                container.pathParams->mAttributeId = static_cast<chip::AttributeId>([attributeId unsignedLongValue]);
            }

            app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
            CHIP_ERROR err = CHIP_NO_ERROR;

            chip::app::ReadPrepareParams readParams(session.Value());
            readParams.mpAttributePathParamsList = container.pathParams;
            readParams.mAttributePathParamsListSize = 1;
            readParams.mMinIntervalFloorSeconds = static_cast<uint16_t>([minInterval unsignedShortValue]);
            readParams.mMaxIntervalCeilingSeconds = static_cast<uint16_t>([maxInterval unsignedShortValue]);
            readParams.mIsFabricFiltered = (params == nil || params.fabricFiltered == nil || [params.fabricFiltered boolValue]);
            readParams.mKeepSubscriptions
                = (params != nil && params.keepPreviousSubscriptions != nil && [params.keepPreviousSubscriptions boolValue]);

            auto onDone = [container](BufferedReadAttributeCallback<MTRDataValueDictionaryDecodableType> * callback) {
                chip::Platform::Delete(callback);
                [container onDone];
            };

            auto callback = chip::Platform::MakeUnique<BufferedReadAttributeCallback<MTRDataValueDictionaryDecodableType>>(
                container.pathParams->mClusterId, container.pathParams->mAttributeId, onReportCb, onFailureCb, onDone,
                onEstablishedCb);

            auto readClient = Platform::New<app::ReadClient>(
                engine, exchangeManager, callback->GetBufferedCallback(), chip::app::ReadClient::InteractionType::Subscribe);

            err = readClient->SendAutoResubscribeRequest(std::move(readParams));

            if (err != CHIP_NO_ERROR) {
                if (reportHandler) {
                    dispatch_async(clientQueue, ^{
                        reportHandler(nil, [MTRError errorForCHIPErrorCode:err]);
                    });
                }
                Platform::Delete(readClient);
                return;
            }

            // Read clients will be purged when deregistered.
            container.readClientPtr = readClient;
            AddReadClientContainer(container.deviceId, container);
            callback.release();
        }];
}

- (void)deregisterReportHandlersWithClientQueue:(dispatch_queue_t)clientQueue completion:(void (^)(void))completion
{
    // This method must only be used for MTRDeviceOverXPC. However, for unit testing purpose, the method purges all read clients.
    MTR_LOG_DEBUG("Unexpected call to deregister report handlers");
    PurgeReadClientContainers([self deviceID], clientQueue, completion);
}

#ifdef DEBUG
// This method is for unit testing only
- (void)failSubscribers:(dispatch_queue_t)clientQueue completion:(void (^)(void))completion
{
    MTR_LOG_DEBUG("Causing failure in subscribers on purpose");
    CauseReadClientFailure([self deviceID], clientQueue, completion);
}
#endif

// The following method is for unit testing purpose only
+ (id)CHIPEncodeAndDecodeNSObject:(id)object
{
    MTRDataValueDictionaryDecodableType originalData(object);
    chip::TLV::TLVWriter writer;
    uint8_t buffer[1024];
    writer.Init(buffer, sizeof(buffer));

    CHIP_ERROR error = originalData.Encode(writer, chip::TLV::Tag(1));
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
    if (tag != chip::TLV::Tag(1)) {
        MTR_LOG_ERROR("Error: TLV reader did not read the tag correctly: %llu", tag.mVal);
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

@end

@implementation MTRAttributePath
- (instancetype)initWithPath:(const ConcreteDataAttributePath &)path
{
    if (self = [super init]) {
        _endpoint = @(path.mEndpointId);
        _cluster = @(path.mClusterId);
        _attribute = @(path.mAttributeId);
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRAttributePath> endpoint %u cluster %u attribute %u",
                     (uint16_t) _endpoint.unsignedShortValue, (uint32_t) _cluster.unsignedLongValue,
                     (uint32_t) _attribute.unsignedLongValue];
}

+ (instancetype)attributePathWithEndpointId:(NSNumber *)endpoint clusterId:(NSNumber *)clusterId attributeId:(NSNumber *)attributeId
{
    ConcreteDataAttributePath path(static_cast<chip::EndpointId>([endpoint unsignedShortValue]),
        static_cast<chip::ClusterId>([clusterId unsignedLongValue]),
        static_cast<chip::AttributeId>([attributeId unsignedLongValue]));

    return [[MTRAttributePath alloc] initWithPath:path];
}

- (BOOL)isEqualToAttributePath:(MTRAttributePath *)attributePath
{
    return [_endpoint isEqualToNumber:attributePath.endpoint] && [_cluster isEqualToNumber:attributePath.cluster] &&
        [_attribute isEqualToNumber:attributePath.attribute];
}

- (BOOL)isEqual:(id)object
{
    if (![object isKindOfClass:[self class]]) {
        return NO;
    }
    return [self isEqualToAttributePath:object];
}

- (NSUInteger)hash
{
    return _endpoint.unsignedShortValue ^ _cluster.unsignedLongValue ^ _attribute.unsignedLongValue;
}

- (id)copyWithZone:(NSZone *)zone
{
    return [MTRAttributePath attributePathWithEndpointId:_endpoint clusterId:_cluster attributeId:_attribute];
}
@end

@implementation MTREventPath
- (instancetype)initWithPath:(const ConcreteEventPath &)path
{
    if (self = [super init]) {
        _endpoint = @(path.mEndpointId);
        _cluster = @(path.mClusterId);
        _event = @(path.mEventId);
    }
    return self;
}

+ (instancetype)eventPathWithEndpointId:(NSNumber *)endpoint clusterId:(NSNumber *)clusterId eventId:(NSNumber *)eventId
{
    ConcreteEventPath path(static_cast<chip::EndpointId>([endpoint unsignedShortValue]),
        static_cast<chip::ClusterId>([clusterId unsignedLongValue]), static_cast<chip::EventId>([eventId unsignedLongValue]));

    return [[MTREventPath alloc] initWithPath:path];
}
@end

@implementation MTRCommandPath
- (instancetype)initWithPath:(const ConcreteCommandPath &)path
{
    if (self = [super init]) {
        _endpoint = @(path.mEndpointId);
        _cluster = @(path.mClusterId);
        _command = @(path.mCommandId);
    }
    return self;
}

+ (instancetype)commandPathWithEndpointId:(NSNumber *)endpoint clusterId:(NSNumber *)clusterId commandId:(NSNumber *)commandId
{
    ConcreteCommandPath path(static_cast<chip::EndpointId>([endpoint unsignedShortValue]),
        static_cast<chip::ClusterId>([clusterId unsignedLongValue]), static_cast<chip::CommandId>([commandId unsignedLongValue]));

    return [[MTRCommandPath alloc] initWithPath:path];
}
@end

@implementation MTRAttributeReport
- (instancetype)initWithPath:(const ConcreteDataAttributePath &)path value:(nullable id)value error:(nullable NSError *)error
{
    if (self = [super init]) {
        _path = [[MTRAttributePath alloc] initWithPath:path];
        _value = value;
        _error = error;
    }
    return self;
}
@end

@implementation MTREventReport
- (instancetype)initWithPath:(const ConcreteEventPath &)path
                 eventNumber:(NSNumber *)eventNumber
                    priority:(NSNumber *)priority
                   timestamp:(NSNumber *)timestamp
                       value:(nullable id)value
                       error:(nullable NSError *)error
{
    if (self = [super init]) {
        _path = [[MTREventPath alloc] initWithPath:path];
        _eventNumber = eventNumber;
        _priority = priority;
        _timestamp = timestamp;
        _value = value;
        _error = error;
    }
    return self;
}
@end

namespace {
void SubscriptionCallback::OnReportBegin()
{
    mAttributeReports = [NSMutableArray new];
    mEventReports = [NSMutableArray new];
}

// Reports attribute and event data if any exists
void SubscriptionCallback::ReportData()
{
    __block NSArray * attributeReports = mAttributeReports;
    mAttributeReports = nil;
    __block auto attributeCallback = mAttributeReportCallback;

    __block NSArray * eventReports = mEventReports;
    mEventReports = nil;
    __block auto eventCallback = mEventReportCallback;

    if (attributeCallback != nil && attributeReports.count) {
        dispatch_async(mQueue, ^{
            attributeCallback(attributeReports);
        });
    }
    if (eventCallback != nil && eventReports.count) {
        dispatch_async(mQueue, ^{
            eventCallback(eventReports);
        });
    }
}

void SubscriptionCallback::OnReportEnd() { ReportData(); }

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

    [mEventReports addObject:[[MTREventReport alloc] initWithPath:aEventHeader.mPath
                                                      eventNumber:@(aEventHeader.mEventNumber)
                                                         priority:@((uint8_t) aEventHeader.mPriorityLevel)
                                                        timestamp:@(aEventHeader.mTimestamp.mValue)
                                                            value:value
                                                            error:error]];
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

void SubscriptionCallback::OnError(CHIP_ERROR aError)
{
    // If OnError is called after OnReportBegin, we should report the collected data
    ReportData();
    ReportError([MTRError errorForCHIPErrorCode:aError]);
}

void SubscriptionCallback::OnDone(ReadClient *)
{
    if (mOnDoneHandler) {
        mOnDoneHandler();
        mOnDoneHandler = nil;
    }
    if (!mHaveQueuedDeletion) {
        delete this;
        return; // Make sure we touch nothing else.
    }
}

void SubscriptionCallback::OnDeallocatePaths(ReadPrepareParams && aReadPrepareParams)
{
    VerifyOrDie((aReadPrepareParams.mAttributePathParamsListSize == 0 && aReadPrepareParams.mpAttributePathParamsList == nullptr)
        || (aReadPrepareParams.mAttributePathParamsListSize == 1 && aReadPrepareParams.mpAttributePathParamsList != nullptr));
    if (aReadPrepareParams.mpAttributePathParamsList) {
        delete aReadPrepareParams.mpAttributePathParamsList;
    }

    VerifyOrDie((aReadPrepareParams.mDataVersionFilterListSize == 0 && aReadPrepareParams.mpDataVersionFilterList == nullptr)
        || (aReadPrepareParams.mDataVersionFilterListSize == 1 && aReadPrepareParams.mpDataVersionFilterList != nullptr));
    if (aReadPrepareParams.mpDataVersionFilterList != nullptr) {
        delete aReadPrepareParams.mpDataVersionFilterList;
    }

    VerifyOrDie((aReadPrepareParams.mEventPathParamsListSize == 0 && aReadPrepareParams.mpEventPathParamsList == nullptr)
        || (aReadPrepareParams.mEventPathParamsListSize == 1 && aReadPrepareParams.mpEventPathParamsList != nullptr));
    if (aReadPrepareParams.mpEventPathParamsList) {
        delete aReadPrepareParams.mpEventPathParamsList;
    }
}

void SubscriptionCallback::OnSubscriptionEstablished(SubscriptionId aSubscriptionId)
{
    if (mSubscriptionEstablishedHandler) {
        dispatch_async(mQueue, mSubscriptionEstablishedHandler);
    }
}

void SubscriptionCallback::ReportError(CHIP_ERROR err) { ReportError([MTRError errorForCHIPErrorCode:err]); }

void SubscriptionCallback::ReportError(const StatusIB & status) { ReportError([MTRError errorForIMStatus:status]); }

void SubscriptionCallback::ReportError(NSError * _Nullable err)
{
    if (!err) {
        // Very strange... Someone tried to create a MTRError for a success status?
        return;
    }

    if (mHaveQueuedDeletion) {
        // Already have an error report pending which will delete us.
        return;
    }

    __block ErrorCallback callback = mErrorCallback;
    __block auto * myself = this;
    mErrorCallback = nil;
    mAttributeReportCallback = nil;
    mEventReportCallback = nil;
    __auto_type onDoneHandler = mOnDoneHandler;
    mOnDoneHandler = nil;
    dispatch_async(mQueue, ^{
        callback(err);
        if (onDoneHandler) {
            onDoneHandler();
        }

        // Deletion of our ReadClient (and hence of ourselves, since the
        // ReadClient has a pointer to us) needs to happen on the Matter work
        // queue.
        dispatch_async(DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
            delete myself;
        });
    });

    mHaveQueuedDeletion = true;
}
} // anonymous namespace
