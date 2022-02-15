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
#import "CHIPDevice_Internal.h"
#import "CHIPError_Internal.h"
#import "CHIPLogging.h"
#include "lib/core/CHIPError.h"

#include <app/AttributePathParams.h>
#include <app/BufferedReadCallback.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadClient.h>
#include <app/util/error-mapping.h>

typedef void (^SubscriptionEstablishedHandler)(void);

using namespace chip;
using namespace chip::app;
using namespace chip::Protocols::InteractionModel;

@interface CHIPDevice ()

@property (nonatomic, readonly, strong, nonnull) NSRecursiveLock * lock;
@property (readonly) chip::DeviceProxy * cppDevice;

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
