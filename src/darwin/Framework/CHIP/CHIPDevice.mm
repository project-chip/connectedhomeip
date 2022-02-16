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

#import "CHIPAttributeTLVValueDecoder_Internal.h"
#import "CHIPCallbackBridgeBase_internal.h"
#import "CHIPDevice_Internal.h"
#import "CHIPError_Internal.h"
#import "CHIPLogging.h"
#include "lib/core/CHIPError.h"

#include <app/AttributePathParams.h>
#include <app/BufferedReadCallback.h>
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

NSString * const kCHIPTypeKey = @"type";
NSString * const kCHIPValueKey = @"value";
NSString * const kCHIPTagKey = @"tag";
NSString * const kCHIPSignedIntegerValueTypeKey = @"SignedInteger";
NSString * const kCHIPUnsignedIntegerValueTypeKey = @"UnsignedInteger";
NSString * const kCHIPBooleanValueTypeKey = @"Boolean";
NSString * const kCHIPUTF8StringValueTypeKey = @"UTF8String";
NSString * const kCHIPOctetStringValueTypeKey = @"OctetString";
NSString * const kCHIPFloatValueTypeKey = @"Float";
NSString * const kCHIPDoubleValueTypeKey = @"Double";
NSString * const kCHIPNullValueTypeKey = @"Null";
NSString * const kCHIPStructureValueTypeKey = @"Structure";
NSString * const kCHIPArrayValueTypeKey = @"Array";
NSString * const kCHIPListValueTypeKey = @"List";
NSString * const kCHIPEndpointIdKey = @"endpointId";
NSString * const kCHIPClusterIdKey = @"clusterId";
NSString * const kCHIPAttributeIdKey = @"attributeId";
NSString * const kCHIPCommandIdKey = @"commandId";
NSString * const kCHIPDataKey = @"data";
NSString * const kCHIPStatusKey = @"status";

class NSObjectAttributeCallbackBridge;

@interface CHIPDevice ()

@property (nonatomic, readonly, strong, nonnull) NSRecursiveLock * lock;
@property (readonly) chip::DeviceProxy * cppDevice;
@property (nonatomic, readwrite) NSMutableDictionary * reportHandlerBridges;

@end

@interface CHIPAttributePath ()
- (instancetype)initWithPath:(const ConcreteDataAttributePath &)path;
@end

@interface CHIPAttributeReport ()
- (instancetype)initWithPath:(const ConcreteDataAttributePath &)path value:(nullable id)value;
@end

@implementation CHIPDevice

- (instancetype)init
{
    if (self = [super init]) {
        _lock = [[NSRecursiveLock alloc] init];
    }
    return self;
}

- (instancetype)initWithDevice:(chip::DeviceProxy *)device
{
    if (self = [super init]) {
        _cppDevice = device;
    }
    return self;
}

- (chip::DeviceProxy *)internalDevice
{
    return _cppDevice;
}

typedef void (^ReportCallback)(NSArray * _Nullable value, NSError * _Nullable error);

namespace {

class SubscriptionCallback final : public ReadClient::Callback {
public:
    SubscriptionCallback(dispatch_queue_t queue, ReportCallback reportCallback,
        SubscriptionEstablishedHandler _Nullable subscriptionEstablishedHandler)
        : mQueue(queue)
        , mReportCallback(reportCallback)
        , mSubscriptionEstablishedHandler(subscriptionEstablishedHandler)
        , mBufferedReadAdapter(*this)
    {
    }

    BufferedReadCallback & GetBufferedCallback() { return mBufferedReadAdapter; }

    // We need to exist to get a ReadClient, so can't take this as a constructor argument.
    void AdoptReadClient(std::unique_ptr<ReadClient> aReadClient) { mReadClient = std::move(aReadClient); }

private:
    void OnReportBegin() override;

    void OnReportEnd() override;

    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;

    void OnError(CHIP_ERROR aError) override;

    void OnDone() override;

    void OnSubscriptionEstablished(uint64_t aSubscriptionId) override;

    void ReportError(CHIP_ERROR err);
    void ReportError(const StatusIB & status);
    void ReportError(NSError * _Nullable err);

private:
    dispatch_queue_t mQueue;
    // We set mReportCallback to nil when queueing error reports, so we
    // make sure to only report one error.
    ReportCallback _Nullable mReportCallback = nil;
    SubscriptionEstablishedHandler _Nullable mSubscriptionEstablishedHandler;
    BufferedReadCallback mBufferedReadAdapter;
    NSMutableArray * _Nullable mReports = nil;

    // Our lifetime management is a little complicated.  On error we
    // attempt to delete the ReadClient, but asynchronously.  While
    // that's pending, someone else (e.g. an error it runs into) could
    // delete it too.  And if someone else does attempt to delete it, we want to
    // make sure we delete ourselves as well.
    //
    // To handle this, enforce the following rules:
    //
    // 1) We guarantee that mReportCallback is only invoked with an error once.
    // 2) We ensure that we delete ourselves and the passed in ReadClient only from OnDone or a queued-up
    //    error callback, but not both, by tracking whether we have a queued-up
    //    deletion.
    std::unique_ptr<ReadClient> mReadClient;
    bool mHaveQueuedDeletion = false;
};

} // anonymous namespace

- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
              reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
    subscriptionEstablished:(nullable void (^)(void))subscriptionEstablishedHandler
{
    DeviceProxy * device = [self internalDevice];
    if (!device) {
        dispatch_async(queue, ^{
            reportHandler(nil, [CHIPError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
        });
        return;
    }
    AttributePathParams attributePath; // Wildcard endpoint, cluster, attribute.
    ReadPrepareParams params(device->GetSecureSession().Value());
    params.mMinIntervalFloorSeconds = minInterval;
    params.mMaxIntervalCeilingSeconds = maxInterval;
    params.mpAttributePathParamsList = &attributePath;
    params.mAttributePathParamsListSize = 1;

    auto callback = std::make_unique<SubscriptionCallback>(queue, reportHandler, subscriptionEstablishedHandler);
    auto readClient = std::make_unique<ReadClient>(InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
        callback->GetBufferedCallback(), ReadClient::InteractionType::Subscribe);

    CHIP_ERROR err = readClient->SendRequest(params);
    if (err != CHIP_NO_ERROR) {
        dispatch_async(queue, ^{
            reportHandler(nil, [CHIPError errorForCHIPErrorCode:err]);
        });

        return;
    }

    // Callback and ReadClient will be deleted when OnDone is called or an error is
    // encountered.
    callback->AdoptReadClient(std::move(readClient));
    callback.release();
}

// Convert TLV data into NSObject
static id ObjectFromTLV(chip::TLV::TLVReader * data)
{
    chip::TLV::TLVType dataTLVType = data->GetType();
    switch (dataTLVType) {
    case chip::TLV::kTLVType_SignedInteger: {
        int64_t val;
        CHIP_ERROR err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            CHIP_LOG_ERROR("Error(%s): TLV signed integer decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:kCHIPSignedIntegerValueTypeKey, kCHIPTypeKey,
                             [NSNumber numberWithLongLong:val], kCHIPValueKey, nil];
    }
    case chip::TLV::kTLVType_UnsignedInteger: {
        uint64_t val;
        CHIP_ERROR err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            CHIP_LOG_ERROR("Error(%s): TLV unsigned integer decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:kCHIPUnsignedIntegerValueTypeKey, kCHIPTypeKey,
                             [NSNumber numberWithUnsignedLongLong:val], kCHIPValueKey, nil];
    }
    case chip::TLV::kTLVType_Boolean: {
        bool val;
        CHIP_ERROR err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            CHIP_LOG_ERROR("Error(%s): TLV boolean decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary
            dictionaryWithObjectsAndKeys:kCHIPBooleanValueTypeKey, kCHIPTypeKey, [NSNumber numberWithBool:val], kCHIPValueKey, nil];
    }
    case chip::TLV::kTLVType_FloatingPointNumber: {
        double val;
        CHIP_ERROR err = data->Get(val);
        if (err != CHIP_NO_ERROR) {
            CHIP_LOG_ERROR("Error(%s): TLV floating point decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:kCHIPDoubleValueTypeKey, kCHIPTypeKey, [NSNumber numberWithDouble:val],
                             kCHIPValueKey, nil];
    }
    case chip::TLV::kTLVType_UTF8String: {
        const uint8_t * ptr;
        CHIP_ERROR err = data->GetDataPtr(ptr);
        if (err != CHIP_NO_ERROR) {
            CHIP_LOG_ERROR("Error(%s): TLV UTF8String decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:kCHIPUTF8StringValueTypeKey, kCHIPTypeKey,
                             [NSString stringWithUTF8String:(const char *) ptr], kCHIPValueKey, nil];
    }
    case chip::TLV::kTLVType_ByteString: {
        uint32_t len = data->GetLength();
        const uint8_t * ptr;
        CHIP_ERROR err = data->GetDataPtr(ptr);
        if (err != CHIP_NO_ERROR) {
            CHIP_LOG_ERROR("Error(%s): TLV ByteString decoding failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:kCHIPOctetStringValueTypeKey, kCHIPTypeKey,
                             [NSData dataWithBytes:ptr length:len], kCHIPValueKey, nil];
    }
    case chip::TLV::kTLVType_Null: {
        return [NSDictionary dictionaryWithObjectsAndKeys:kCHIPNullValueTypeKey, kCHIPTypeKey, nil];
    }
    case chip::TLV::kTLVType_Structure:
    case chip::TLV::kTLVType_Array:
    case chip::TLV::kTLVType_List: {
        NSString * typeName;
        switch (dataTLVType) {
        case chip::TLV::kTLVType_Structure:
            typeName = kCHIPStructureValueTypeKey;
            break;
        case chip::TLV::kTLVType_Array:
            typeName = kCHIPArrayValueTypeKey;
            break;
        case chip::TLV::kTLVType_List:
            typeName = kCHIPListValueTypeKey;
            break;
        default:
            typeName = @"Unsupported";
            break;
        }
        chip::TLV::TLVType tlvType;
        CHIP_ERROR err = data->EnterContainer(tlvType);
        if (err != CHIP_NO_ERROR) {
            CHIP_LOG_ERROR("Error(%s): TLV container entering failed", chip::ErrorStr(err));
            return nil;
        }
        NSMutableArray * array = [[NSMutableArray alloc] init];
        while ((err = data->Next()) == CHIP_NO_ERROR) {
            chip::TLV::Tag tag = data->GetTag();
            id value = ObjectFromTLV(data);
            if (value == nullptr) {
                CHIP_LOG_ERROR("Error when decoding TLV container");
                return nil;
            }
            [array addObject:[NSDictionary dictionaryWithObjectsAndKeys:value, kCHIPValueKey,
                                           [NSNumber numberWithUnsignedLongLong:(unsigned long long) tag.mVal], kCHIPTagKey, nil]];
        }
        if (err != CHIP_END_OF_TLV) {
            CHIP_LOG_ERROR("Error(%s): TLV container decoding failed", chip::ErrorStr(err));
            return nil;
        }
        err = data->ExitContainer(tlvType);
        if (err != CHIP_NO_ERROR) {
            CHIP_LOG_ERROR("Error(%s): TLV container exiting failed", chip::ErrorStr(err));
            return nil;
        }
        return [NSDictionary dictionaryWithObjectsAndKeys:typeName, kCHIPTypeKey, array, kCHIPValueKey, nil];
    }
    default:
        CHIP_LOG_ERROR("Error: Unsupported TLV type for conversion: %u", (unsigned) data->GetType());
        return nil;
    }
}

static CHIP_ERROR EncodeTLVFromObject(id object, chip::TLV::TLVWriter & writer, chip::TLV::Tag tag)
{
    if (![object isKindOfClass:[NSDictionary class]]) {
        CHIP_LOG_ERROR("Error: Unsupported object to encode: %@", [object class]);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    NSString * typeName = ((NSDictionary *) object)[kCHIPTypeKey];
    id value = ((NSDictionary *) object)[kCHIPValueKey];
    if (!typeName) {
        CHIP_LOG_ERROR("Error: Object to encode is corrupt");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if ([typeName isEqualToString:kCHIPSignedIntegerValueTypeKey]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            CHIP_LOG_ERROR("Error: Object to encode has corrupt signed integer type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.Put(tag, [value intValue]);
    }
    if ([typeName isEqualToString:kCHIPUnsignedIntegerValueTypeKey]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            CHIP_LOG_ERROR("Error: Object to encode has corrupt unsigned integer type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.Put(tag, [value unsignedIntValue]);
    }
    if ([typeName isEqualToString:kCHIPBooleanValueTypeKey]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            CHIP_LOG_ERROR("Error: Object to encode has corrupt boolean type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.Put(tag, [value unsignedIntValue] ? true : false);
    }
    if ([typeName isEqualToString:kCHIPFloatValueTypeKey]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            CHIP_LOG_ERROR("Error: Object to encode has corrupt float type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.Put(tag, [value floatValue]);
    }
    if ([typeName isEqualToString:kCHIPDoubleValueTypeKey]) {
        if (![value isKindOfClass:[NSNumber class]]) {
            CHIP_LOG_ERROR("Error: Object to encode has corrupt double type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.Put(tag, [value doubleValue]);
    }
    if ([typeName isEqualToString:kCHIPNullValueTypeKey]) {
        return writer.PutNull(tag);
    }
    if ([typeName isEqualToString:kCHIPUTF8StringValueTypeKey]) {
        if (![value isKindOfClass:[NSString class]]) {
            CHIP_LOG_ERROR("Error: Object to encode has corrupt UTF8 string type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.PutString(tag, [value cStringUsingEncoding:NSUTF8StringEncoding]);
    }
    if ([typeName isEqualToString:kCHIPOctetStringValueTypeKey]) {
        if (![value isKindOfClass:[NSData class]]) {
            CHIP_LOG_ERROR("Error: Object to encode has corrupt octet string type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return writer.Put(tag, chip::ByteSpan(static_cast<const uint8_t *>([value bytes]), [value length]));
    }
    if ([typeName isEqualToString:kCHIPStructureValueTypeKey]) {
        if (![value isKindOfClass:[NSArray class]]) {
            CHIP_LOG_ERROR("Error: Object to encode has corrupt structure type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, chip::TLV::kTLVType_Structure, outer));
        for (id element in value) {
            if (![element isKindOfClass:[NSDictionary class]]) {
                CHIP_LOG_ERROR("Error: Structure element to encode has corrupt type: %@", [element class]);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            NSNumber * elementTag = element[kCHIPTagKey];
            id elementValue = element[kCHIPValueKey];
            if (!elementTag || !elementValue) {
                CHIP_LOG_ERROR("Error: Structure element to encode has corrupt value: %@", element);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            ReturnErrorOnFailure(EncodeTLVFromObject(elementValue, writer, chip::TLV::ContextTag([elementTag unsignedCharValue])));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
    if ([typeName isEqualToString:kCHIPArrayValueTypeKey]) {
        if (![value isKindOfClass:[NSArray class]]) {
            CHIP_LOG_ERROR("Error: Object to encode has corrupt array type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, chip::TLV::kTLVType_Array, outer));
        for (id element in value) {
            if (![element isKindOfClass:[NSDictionary class]]) {
                CHIP_LOG_ERROR("Error: Array element to encode has corrupt type: %@", [element class]);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            id elementValue = element[kCHIPValueKey];
            if (!elementValue) {
                CHIP_LOG_ERROR("Error: Array element to encode has corrupt value: %@", element);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            ReturnErrorOnFailure(EncodeTLVFromObject(elementValue, writer, chip::TLV::AnonymousTag()));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
    if ([typeName isEqualToString:kCHIPListValueTypeKey]) {
        if (![value isKindOfClass:[NSArray class]]) {
            CHIP_LOG_ERROR("Error: Object to encode has corrupt list type: %@", [value class]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, chip::TLV::kTLVType_List, outer));
        for (id element in value) {
            if (![element isKindOfClass:[NSDictionary class]]) {
                CHIP_LOG_ERROR("Error: List element to encode has corrupt type: %@", [element class]);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            NSNumber * elementTag = element[kCHIPTagKey];
            id elementValue = element[kCHIPValueKey];
            if (!elementValue) {
                CHIP_LOG_ERROR("Error: Array element to encode has corrupt value: %@", element);
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            ReturnErrorOnFailure(EncodeTLVFromObject(elementValue, writer,
                elementTag ? chip::TLV::ContextTag((uint8_t)[elementTag unsignedCharValue]) : chip::TLV::AnonymousTag()));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
    CHIP_LOG_ERROR("Error: Unsupported type to encode: %@", typeName);
    return CHIP_ERROR_INVALID_ARGUMENT;
}

// Callback type to pass attribute value as an NSObject
typedef void (*NSObjectAttributeCallback)(void * context, id value);
typedef void (*CHIPErrorCallback)(void * context, CHIP_ERROR error);

// Rename to be generic for decode and encode
class NSObjectData {
public:
    NSObjectData()
        : decodedObj(nil)
    {
    }
    NSObjectData(id obj)
        : decodedObj(obj)
    {
    }

    CHIP_ERROR Decode(chip::TLV::TLVReader & data)
    {
        decodedObj = ObjectFromTLV(&data);
        if (decodedObj == nil) {
            CHIP_LOG_ERROR("Error: Failed to get value from TLV data for attribute reading response");
        }
        return (decodedObj) ? CHIP_NO_ERROR : CHIP_ERROR_DECODE_FAILED;
    }

    CHIP_ERROR Encode(chip::TLV::TLVWriter & writer, chip::TLV::Tag tag) const
    {
        return EncodeTLVFromObject(decodedObj, writer, tag);
    }

    static constexpr bool kIsFabricScoped = false;

    static bool MustUseTimedInvoke() { return false; }

    id _Nullable GetDecodedObject() const { return decodedObj; }

private:
    id _Nullable decodedObj;
};

// Callback bridge for NSObjectAttributeCallback
class NSObjectAttributeCallbackBridge : public CHIPCallbackBridge<NSObjectAttributeCallback> {
public:
    NSObjectAttributeCallbackBridge(
        dispatch_queue_t queue, CHIPDeviceResponseHandler handler, CHIPActionBlock action, bool keepAlive = false)
        : CHIPCallbackBridge<NSObjectAttributeCallback>(queue, handler, action, OnSuccessFn, keepAlive) {};

    static void OnSuccessFn(void * context, id value) { DispatchSuccess(context, value); }
};

// Subscribe bridge for NSObjectAttributeCallback
class NSObjectAttributeCallbackSubscribeBridge : public NSObjectAttributeCallbackBridge {
public:
    NSObjectAttributeCallbackSubscribeBridge(dispatch_queue_t queue,
        void (^handler)(NSDictionary<NSString *, id> * _Nullable value, NSError * _Nullable error), CHIPActionBlock action,
        SubscriptionEstablishedHandler establishedHandler)
        : NSObjectAttributeCallbackBridge(queue, (CHIPDeviceResponseHandler) handler, action, true)
        , mQueue(queue)
        , mEstablishedHandler(establishedHandler) {};

    static void OnSubscriptionEstablished(void * context)
    {
        auto * self = static_cast<NSObjectAttributeCallbackSubscribeBridge *>(context);
        if (!self->mQueue) {
            return;
        }

        if (self->mEstablishedHandler != nil) {
            dispatch_async(self->mQueue, self->mEstablishedHandler);
            // On failure, mEstablishedHandler will be cleaned up by our destructor,
            // but we can clean it up earlier on successful subscription
            // establishment.
            self->mEstablishedHandler = nil;
        }
    }

private:
    dispatch_queue_t mQueue;
    SubscriptionEstablishedHandler mEstablishedHandler;
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

    void OnDone() override { mOnDone(this); }

    void OnSubscriptionEstablished(uint64_t aSubscriptionId) override
    {
        if (mOnSubscriptionEstablished) {
            mOnSubscriptionEstablished();
        }
    }

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override
    {
        VerifyOrDie(
            aReadPrepareParams.mAttributePathParamsListSize == 1 && aReadPrepareParams.mpAttributePathParamsList != nullptr);
        chip::Platform::Delete<app::AttributePathParams>(aReadPrepareParams.mpAttributePathParamsList);

        VerifyOrDie(aReadPrepareParams.mDataVersionFilterListSize == 1 && aReadPrepareParams.mpDataVersionFilterList != nullptr);
        chip::Platform::Delete<app::DataVersionFilter>(aReadPrepareParams.mpDataVersionFilterList);
    }

    ClusterId mClusterId;
    AttributeId mAttributeId;
    OnSuccessCallbackType mOnSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;
    OnSubscriptionEstablishedCallbackType mOnSubscriptionEstablished;
    app::BufferedReadCallback mBufferedReadAdapter;
    Platform::UniquePtr<app::ReadClient> mReadClient;
};

- (void)readAttributeWithEndpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                        attributeId:(NSUInteger)attributeId
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(CHIPDeviceResponseHandler)completion
{
    new NSObjectAttributeCallbackBridge(
        clientQueue, completion, ^(chip::Callback::Cancelable * success, chip::Callback::Cancelable * failure) {
            auto successFn = chip::Callback::Callback<NSObjectAttributeCallback>::FromCancelable(success);
            auto failureFn = chip::Callback::Callback<CHIPErrorCallback>::FromCancelable(failure);
            auto context = successFn->mContext;
            auto successCb = successFn->mCall;
            auto failureCb = failureFn->mCall;
            auto resultArray = [[NSMutableArray alloc] init];
            auto resultSuccess = [[NSMutableArray alloc] init];
            auto resultFailure = [[NSMutableArray alloc] init];
            auto onSuccessCb = [resultArray, resultSuccess](
                                   const app::ConcreteAttributePath & attribPath, const NSObjectData & aData) {
                [resultArray
                    addObject:[[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:attribPath.mEndpointId],
                                                    kCHIPEndpointIdKey, [NSNumber numberWithUnsignedInt:attribPath.mClusterId],
                                                    kCHIPClusterIdKey, [NSNumber numberWithUnsignedInt:attribPath.mAttributeId],
                                                    kCHIPAttributeIdKey, aData.GetDecodedObject(), kCHIPDataKey,
                                                    [NSNumber numberWithUnsignedInt:0], kCHIPStatusKey, nil]];
                if ([resultSuccess count] == 0) {
                    [resultSuccess addObject:[NSNumber numberWithBool:YES]];
                }
            };

            auto onFailureCb = [resultArray, resultFailure](const app::ConcreteAttributePath * attribPath, CHIP_ERROR aError) {
                if (attribPath) {
                    [resultArray addObject:[[NSDictionary alloc]
                                               initWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:attribPath->mEndpointId],
                                               kCHIPEndpointIdKey, [NSNumber numberWithUnsignedInt:attribPath->mClusterId],
                                               kCHIPClusterIdKey, [NSNumber numberWithUnsignedInt:attribPath->mAttributeId],
                                               kCHIPAttributeIdKey, [NSNumber numberWithUnsignedInteger:aError.AsInteger()],
                                               kCHIPStatusKey, nil]];
                } else if ([resultFailure count] == 0) {
                    [resultFailure addObject:[NSNumber numberWithUnsignedInteger:aError.AsInteger()]];
                }
            };

            auto chipEndpointId = static_cast<chip::EndpointId>(endpointId);
            auto chipClusterId = static_cast<chip::ClusterId>(clusterId);
            auto chipAttributeId = static_cast<chip::AttributeId>(attributeId);

            app::AttributePathParams attributePath(chipEndpointId, chipClusterId, chipAttributeId);
            app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
            CHIP_ERROR err = CHIP_NO_ERROR;

            chip::app::ReadPrepareParams readParams([self internalDevice]->GetSecureSession().Value());
            readParams.mpAttributePathParamsList = &attributePath;
            readParams.mAttributePathParamsListSize = 1;

            auto onDone = [resultArray, resultSuccess, resultFailure, context, successCb, failureCb](
                              BufferedReadAttributeCallback<NSObjectData> * callback) {
                if ([resultFailure count] > 0 || [resultSuccess count] == 0) {
                    // Failure
                    if (failureCb) {
                        if ([resultFailure count] > 0) {
                            failureCb(
                                context, CHIP_ERROR(static_cast<CHIP_ERROR::StorageType>([resultFailure[0] unsignedIntegerValue])));
                        } else if ([resultArray count] > 0) {
                            failureCb(context,
                                CHIP_ERROR(
                                    static_cast<CHIP_ERROR::StorageType>([resultArray[0][kCHIPStatusKey] unsignedIntegerValue])));
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

            auto callback = chip::Platform::MakeUnique<BufferedReadAttributeCallback<NSObjectData>>(
                chipClusterId, chipAttributeId, onSuccessCb, onFailureCb, onDone, nullptr);
            VerifyOrReturnError(callback != nullptr, CHIP_ERROR_NO_MEMORY);

            auto readClient = chip::Platform::MakeUnique<app::ReadClient>(engine, [self internalDevice]->GetExchangeManager(),
                callback -> GetBufferedCallback(), chip::app::ReadClient::InteractionType::Read);
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

- (void)writeAttributeWithEndpointId:(NSUInteger)endpointId
                           clusterId:(NSUInteger)clusterId
                         attributeId:(NSUInteger)attributeId
                               value:(id)value
                         clientQueue:(dispatch_queue_t)clientQueue
                          completion:(CHIPDeviceResponseHandler)completion
{
    new NSObjectAttributeCallbackBridge(
        clientQueue, completion, ^(chip::Callback::Cancelable * success, chip::Callback::Cancelable * failure) {
            auto successFn = chip::Callback::Callback<NSObjectAttributeCallback>::FromCancelable(success);
            auto failureFn = chip::Callback::Callback<CHIPErrorCallback>::FromCancelable(failure);
            auto context = successFn->mContext;
            auto successCb = successFn->mCall;
            auto failureCb = failureFn->mCall;
            auto resultArray = [[NSMutableArray alloc] init];
            auto resultSuccess = [[NSMutableArray alloc] init];
            auto resultFailure = [[NSMutableArray alloc] init];
            auto onSuccessCb = [resultArray, resultSuccess](const app::ConcreteAttributePath & commandPath) {
                [resultArray
                    addObject:[[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:commandPath.mEndpointId],
                                                    kCHIPEndpointIdKey, [NSNumber numberWithUnsignedInt:commandPath.mClusterId],
                                                    kCHIPClusterIdKey, [NSNumber numberWithUnsignedInt:commandPath.mAttributeId],
                                                    kCHIPAttributeIdKey, [NSNumber numberWithUnsignedInt:0], kCHIPStatusKey, nil]];
                if ([resultSuccess count] == 0) {
                    [resultSuccess addObject:[NSNumber numberWithBool:YES]];
                }
            };

            auto onFailureCb = [resultArray, resultFailure](const app::ConcreteAttributePath * commandPath, CHIP_ERROR aError) {
                if (commandPath) {
                    [resultArray addObject:[[NSDictionary alloc]
                                               initWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:commandPath->mEndpointId],
                                               kCHIPEndpointIdKey, [NSNumber numberWithUnsignedInt:commandPath->mClusterId],
                                               kCHIPClusterIdKey, [NSNumber numberWithUnsignedInt:commandPath->mAttributeId],
                                               kCHIPAttributeIdKey, [NSNumber numberWithUnsignedInteger:aError.AsInteger()],
                                               kCHIPStatusKey, nil]];
                } else {
                    if ([resultFailure count] == 0) {
                        [resultFailure addObject:[NSNumber numberWithUnsignedInteger:aError.AsInteger()]];
                    }
                }
            };

            auto onDoneCb = [context, successCb, failureCb, resultArray, resultSuccess, resultFailure](
                                app::WriteClient * pWriteClient) {
                if ([resultFailure count] > 0 || [resultSuccess count] == 0) {
                    // Failure
                    if (failureCb) {
                        if ([resultFailure count] > 0) {
                            failureCb(
                                context, CHIP_ERROR(static_cast<CHIP_ERROR::StorageType>([resultFailure[0] unsignedIntegerValue])));
                        } else if ([resultArray count] > 0) {
                            failureCb(context,
                                CHIP_ERROR(
                                    static_cast<CHIP_ERROR::StorageType>([resultArray[0][kCHIPStatusKey] unsignedIntegerValue])));
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

            return chip::Controller::WriteAttribute<NSObjectData>([self internalDevice]->GetSecureSession().Value(),
                static_cast<chip::EndpointId>(endpointId), static_cast<chip::ClusterId>(clusterId),
                static_cast<chip::AttributeId>(attributeId), NSObjectData(value), onSuccessCb, onFailureCb, NullOptional, onDoneCb,
                NullOptional);
        });
}

class NSObjectCommandCallback final : public app::CommandSender::Callback {
public:
    using OnSuccessCallbackType
        = std::function<void(const app::ConcreteCommandPath &, const app::StatusIB &, const NSObjectData &)>;
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
    chip::CommandId mCommandId;
};

void NSObjectCommandCallback::OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aCommandPath,
    const app::StatusIB & aStatus, TLV::TLVReader * aReader)
{
    NSObjectData response;
    CHIP_ERROR err = CHIP_NO_ERROR;

    //
    // Validate that the data response we received matches what we expect in terms of its cluster and command IDs.
    //
    VerifyOrExit(aCommandPath.mClusterId == mClusterId && aCommandPath.mCommandId == mCommandId, err = CHIP_ERROR_SCHEMA_MISMATCH);

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

- (void)invokeCommandWithEndpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                          commandId:(NSUInteger)commandId
                      commandFields:(id)commandFields
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(CHIPDeviceResponseHandler)completion
{
    new NSObjectAttributeCallbackBridge(
        clientQueue, completion, ^(chip::Callback::Cancelable * success, chip::Callback::Cancelable * failure) {
            auto successFn = chip::Callback::Callback<NSObjectAttributeCallback>::FromCancelable(success);
            auto failureFn = chip::Callback::Callback<CHIPErrorCallback>::FromCancelable(failure);
            auto context = successFn->mContext;
            auto successCb = successFn->mCall;
            auto failureCb = failureFn->mCall;
            auto resultArray = [[NSMutableArray alloc] init];
            auto resultSuccess = [[NSMutableArray alloc] init];
            auto resultFailure = [[NSMutableArray alloc] init];
            auto onSuccessCb = [resultArray, resultSuccess](const app::ConcreteCommandPath & commandPath,
                                   const app::StatusIB & status, const NSObjectData & responseData) {
                if (responseData.GetDecodedObject()) {
                    [resultArray addObject:[[NSDictionary alloc]
                                               initWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:commandPath.mEndpointId],
                                               kCHIPEndpointIdKey, [NSNumber numberWithUnsignedInt:commandPath.mClusterId],
                                               kCHIPClusterIdKey, [NSNumber numberWithUnsignedInt:commandPath.mCommandId],
                                               kCHIPCommandIdKey, [NSNumber numberWithUnsignedInt:0], kCHIPStatusKey,
                                               responseData.GetDecodedObject(), kCHIPDataKey, nil]];
                } else {
                    [resultArray addObject:[[NSDictionary alloc]
                                               initWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:commandPath.mEndpointId],
                                               kCHIPEndpointIdKey, [NSNumber numberWithUnsignedInt:commandPath.mClusterId],
                                               kCHIPClusterIdKey, [NSNumber numberWithUnsignedInt:commandPath.mCommandId],
                                               kCHIPCommandIdKey, [NSNumber numberWithUnsignedInt:0], kCHIPStatusKey, nil]];
                }
                if ([resultSuccess count] == 0) {
                    [resultSuccess addObject:[NSNumber numberWithBool:YES]];
                }
            };

            auto onFailureCb = [resultFailure](CHIP_ERROR aError) {
                if ([resultFailure count] == 0) {
                    [resultFailure addObject:[NSNumber numberWithUnsignedInteger:aError.AsInteger()]];
                }
            };

            app::CommandPathParams commandPath = { (chip::EndpointId) endpointId, 0, (chip::ClusterId) clusterId,
                (chip::CommandId) commandId, (app::CommandPathFlags::kEndpointIdValid) };

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
                            failureCb(
                                context, CHIP_ERROR(static_cast<CHIP_ERROR::StorageType>([resultFailure[0] unsignedIntegerValue])));
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

            auto commandSender
                = chip::Platform::MakeUnique<app::CommandSender>(decoder.get(), [self internalDevice]->GetExchangeManager(), false);
            VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_NO_MEMORY);

            ReturnErrorOnFailure(commandSender->AddRequestData(commandPath, NSObjectData(commandFields), chip::NullOptional));
            ReturnErrorOnFailure(commandSender->SendCommandRequest([self internalDevice]->GetSecureSession().Value()));

            decoder.release();
            commandSender.release();
            return CHIP_NO_ERROR;
        });
}

- (void)subscribeAttributeWithEndpointId:(NSUInteger)endpointId
                               clusterId:(NSUInteger)clusterId
                             attributeId:(NSUInteger)attributeId
                             minInterval:(NSUInteger)minInterval
                             maxInterval:(NSUInteger)maxInterval
                             clientQueue:(dispatch_queue_t)clientQueue
                           reportHandler:(void (^)(NSDictionary<NSString *, id> * _Nullable value,
                                             NSError * _Nullable error))reportHandler
                 subscriptionEstablished:(SubscriptionEstablishedHandler)subscriptionEstablishedHandler
{
    new NSObjectAttributeCallbackSubscribeBridge(
        clientQueue, (void (^)(id _Nullable, NSError * _Nullable)) reportHandler,
        ^(chip::Callback::Cancelable * success, chip::Callback::Cancelable * failure) {
            auto successFn = chip::Callback::Callback<NSObjectAttributeCallback>::FromCancelable(success);
            auto failureFn = chip::Callback::Callback<CHIPErrorCallback>::FromCancelable(failure);
            auto context = successFn->mContext;
            auto successCb = successFn->mCall;
            auto failureCb = failureFn->mCall;
            auto onReportCb = [context, successCb](const app::ConcreteAttributePath & attribPath, const NSObjectData & data) {
                if (successCb != nullptr) {
                    successCb(context,
                        [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:attribPath.mEndpointId],
                                              kCHIPEndpointIdKey, [NSNumber numberWithUnsignedInt:attribPath.mClusterId],
                                              kCHIPClusterIdKey, [NSNumber numberWithUnsignedInt:attribPath.mAttributeId],
                                              kCHIPAttributeIdKey, data.GetDecodedObject(), kCHIPValueKey, nil]);
                }
            };

            auto establishedOrFailed = std::make_shared<BOOL>(NO);
            auto onFailureCb
                = [context, failureCb, establishedOrFailed](const app::ConcreteAttributePath * attribPath, CHIP_ERROR error) {
                      if (!(*establishedOrFailed)) {
                          *establishedOrFailed = YES;
                          NSObjectAttributeCallbackSubscribeBridge::OnSubscriptionEstablished(context);
                      }
                      if (failureCb != nullptr) {
                          failureCb(context, error);
                      }
                  };

            auto onEstablishedCb = [context, establishedOrFailed]() {
                if (*establishedOrFailed) {
                    return;
                }
                *establishedOrFailed = YES;
                NSObjectAttributeCallbackSubscribeBridge::OnSubscriptionEstablished(context);
            };

            auto chipEndpointId = static_cast<chip::EndpointId>(endpointId);
            auto chipClusterId = static_cast<chip::ClusterId>(clusterId);
            auto chipAttributeId = static_cast<chip::AttributeId>(attributeId);

            app::AttributePathParams attributePath(chipEndpointId, chipClusterId, chipAttributeId);
            app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
            CHIP_ERROR err = CHIP_NO_ERROR;

            chip::app::ReadPrepareParams readParams([self internalDevice]->GetSecureSession().Value());
            readParams.mpAttributePathParamsList = &attributePath;
            readParams.mAttributePathParamsListSize = 1;

            auto onDone = [](BufferedReadAttributeCallback<NSObjectData> * callback) { chip::Platform::Delete(callback); };

            auto callback = chip::Platform::MakeUnique<BufferedReadAttributeCallback<NSObjectData>>(
                chipClusterId, chipAttributeId, onReportCb, onFailureCb, onDone, onEstablishedCb);
            VerifyOrReturnError(callback != nullptr, CHIP_ERROR_NO_MEMORY);

            auto readClient = chip::Platform::MakeUnique<app::ReadClient>(engine, [self internalDevice]->GetExchangeManager(),
                callback -> GetBufferedCallback(), chip::app::ReadClient::InteractionType::Subscribe);
            VerifyOrReturnError(readClient != nullptr, CHIP_ERROR_NO_MEMORY);

            err = readClient->SendAutoResubscribeRequest(std::move(readParams));

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
        },
        subscriptionEstablishedHandler);
}

- (void)deregisterReportHandlersWithClientQueue:(dispatch_queue_t)clientQueue completion:(void (^)(void))completion
{
    // Do nothing for a local instance.
    CHIP_LOG_ERROR("Unexpected call to deregister report handlers");
    dispatch_async(clientQueue, completion);
}

// The following method is for unit testing purpose only
+ (id)CHIPEncodeAndDecodeNSObject:(id)object
{
    NSObjectData originalData(object);
    chip::TLV::TLVWriter writer;
    uint8_t buffer[1024];
    writer.Init(buffer, sizeof(buffer));

    CHIP_ERROR error = originalData.Encode(writer, chip::TLV::Tag(1));
    if (error != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error: Data encoding failed: %s", error.AsString());
        return nil;
    }

    error = writer.Finalize();
    if (error != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error: TLV writer finalizing failed: %s", error.AsString());
        return nil;
    }
    chip::TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    error = reader.Next();
    if (error != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error: TLV reader failed to fetch next element: %s", error.AsString());
        return nil;
    }
    __auto_type tag = reader.GetTag();
    if (tag != chip::TLV::Tag(1)) {
        CHIP_LOG_ERROR("Error: TLV reader did not read the tag correctly: %llu", tag.mVal);
        return nil;
    }
    NSObjectData decodedData;
    error = decodedData.Decode(reader);
    if (error != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error: Data decoding failed: %s", error.AsString());
        return nil;
    }
    return decodedData.GetDecodedObject();
}

@end

@implementation CHIPAttributePath
- (instancetype)initWithPath:(const ConcreteDataAttributePath &)path
{
    if (self = [super init]) {
        _endpoint = @(path.mEndpointId);
        _cluster = @(path.mClusterId);
        _attribute = @(path.mAttributeId);
    }
    return self;
}
@end

@implementation CHIPAttributeReport
- (instancetype)initWithPath:(const ConcreteDataAttributePath &)path value:(nullable id)value
{
    if (self = [super init]) {
        _path = [[CHIPAttributePath alloc] initWithPath:path];
        _value = value;
    }
    return self;
}
@end

namespace {
void SubscriptionCallback::OnReportBegin() { mReports = [NSMutableArray new]; }

void SubscriptionCallback::OnReportEnd()
{
    __block NSArray * reports = mReports;
    mReports = nil;
    if (mReportCallback) {
        dispatch_async(mQueue, ^{
            mReportCallback(reports, nil);
        });
    }
    // Else we have a pending error already.
}

void SubscriptionCallback::OnAttributeData(
    const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus)
{
    if (aPath.IsListItemOperation()) {
        ReportError(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    if (aStatus.mStatus != Status::Success) {
        ReportError(aStatus);
        return;
    }

    if (apData == nullptr) {
        ReportError(CHIP_ERROR_INVALID_ARGUMENT);
        return;
    }

    CHIP_ERROR err;
    id _Nullable value = CHIPDecodeAttributeValue(aPath, *apData, &err);
    if (err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH) {
        // We don't know this attribute; just skip it.
        return;
    }

    if (err != CHIP_NO_ERROR) {
        ReportError(err);
        return;
    }

    if (mReports == nil) {
        // Never got a OnReportBegin?
        ReportError(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    [mReports addObject:[[CHIPAttributeReport alloc] initWithPath:aPath value:value]];
}

void SubscriptionCallback::OnError(CHIP_ERROR aError) { ReportError([CHIPError errorForCHIPErrorCode:aError]); }

void SubscriptionCallback::OnDone()
{
    if (!mHaveQueuedDeletion) {
        delete this;
        return; // Make sure we touch nothing else.
    }
}

void SubscriptionCallback::OnSubscriptionEstablished(uint64_t aSubscriptionId)
{
    if (mSubscriptionEstablishedHandler) {
        dispatch_async(mQueue, mSubscriptionEstablishedHandler);
        // Don't need it anymore.
        mSubscriptionEstablishedHandler = nil;
    }
}

void SubscriptionCallback::ReportError(CHIP_ERROR err) { ReportError([CHIPError errorForCHIPErrorCode:err]); }

void SubscriptionCallback::ReportError(const StatusIB & status) { ReportError([CHIPError errorForIMStatus:status]); }

void SubscriptionCallback::ReportError(NSError * _Nullable err)
{
    if (!err) {
        // Very strange... Someone tried to create a CHIPError for a success status?
        return;
    }

    if (mHaveQueuedDeletion) {
        // Already have an error report pending which will delete us.
        return;
    }

    __block ReportCallback callback = mReportCallback;
    __block auto * myself = this;
    mReportCallback = nil;
    dispatch_async(mQueue, ^{
        callback(nil, err);

        delete myself;
    });

    mHaveQueuedDeletion = true;
}
} // anonymous namespace
