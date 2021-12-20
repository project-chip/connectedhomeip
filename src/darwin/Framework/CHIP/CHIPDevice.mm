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
    void SetReadClient(ReadClient * aReadClient) { mReadClient = aReadClient; }

private:
    void OnReportBegin(const ReadClient * apReadClient) override;

    void OnReportEnd(const ReadClient * apReadClient) override;

    void OnAttributeData(const ReadClient * apReadClient, const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
        const StatusIB & aStatus) override;

    void OnError(const ReadClient * apReadClient, CHIP_ERROR aError) override;

    void OnDone(ReadClient * apReadClient) override;

    void OnSubscriptionEstablished(const ReadClient * apReadClient) override;

    void ReportError(CHIP_ERROR err);
    void ReportError(EmberAfStatus status);
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
    ReadClient * mReadClient = nullptr;
    bool mHaveQueuedDeletion = false;
};

} // anonymous namespace

- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
              reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
    subscriptionEstablished:(SubscriptionEstablishedHandler _Nullable)subscriptionEstablishedHandler
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

    auto callback = new SubscriptionCallback(queue, reportHandler, subscriptionEstablishedHandler);
    ReadClient * readClient = new ReadClient(InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
        callback->GetBufferedCallback(), ReadClient::InteractionType::Subscribe);

    CHIP_ERROR err = readClient->SendRequest(params);
    if (err != CHIP_NO_ERROR) {
        dispatch_async(queue, ^{
            reportHandler(nil, [CHIPError errorForCHIPErrorCode:err]);
        });

        delete readClient;
        delete callback;
        return;
    }

    // Callback and ReadClient will be deleted when OnDone is called or an error is
    // encountered.
    callback->SetReadClient(readClient);
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
void SubscriptionCallback::OnReportBegin(const ReadClient * apReadClient) { mReports = [NSMutableArray new]; }

void SubscriptionCallback::OnReportEnd(const ReadClient * apReadClient)
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
    const ReadClient * apReadClient, const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus)
{
    if (aPath.IsListItemOperation()) {
        ReportError(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    if (aStatus.mStatus != Status::Success) {
        ReportError(ToEmberAfStatus(aStatus.mStatus));
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

void SubscriptionCallback::OnError(const ReadClient * apReadClient, CHIP_ERROR aError)
{
    ReportError([CHIPError errorForCHIPErrorCode:aError]);
}

void SubscriptionCallback::OnDone(ReadClient * apReadClient)
{
    if (!mHaveQueuedDeletion) {
        delete this;
        delete apReadClient;
        return; // Make sure we touch nothing else.
    }
}

void SubscriptionCallback::OnSubscriptionEstablished(const ReadClient * apReadClient)
{
    if (mSubscriptionEstablishedHandler) {
        dispatch_async(mQueue, mSubscriptionEstablishedHandler);
        // Don't need it anymore.
        mSubscriptionEstablishedHandler = nil;
    }
}

void SubscriptionCallback::ReportError(CHIP_ERROR err) { ReportError([CHIPError errorForCHIPErrorCode:err]); }

void SubscriptionCallback::ReportError(EmberAfStatus status) { ReportError([CHIPError errorForZCLErrorCode:status]); }

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

        delete mReadClient;
        delete myself;
    });

    mHaveQueuedDeletion = true;
}
} // anonymous namespace
