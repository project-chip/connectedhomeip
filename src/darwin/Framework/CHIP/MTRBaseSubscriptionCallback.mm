/**
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

#import "MTRBaseSubscriptionCallback.h"
#import "MTRError_Internal.h"

#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;

void MTRBaseSubscriptionCallback::OnReportBegin()
{
    mAttributeReports = [NSMutableArray new];
    mEventReports = [NSMutableArray new];
}

// Reports attribute and event data if any exists
void MTRBaseSubscriptionCallback::ReportData()
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

void MTRBaseSubscriptionCallback::OnReportEnd() { ReportData(); }

void MTRBaseSubscriptionCallback::OnError(CHIP_ERROR aError)
{
    // If OnError is called after OnReportBegin, we should report the collected data
    ReportData();
    ReportError(aError, /* aCancelSubscription = */ false);
}

void MTRBaseSubscriptionCallback::OnDone(ReadClient *)
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

void MTRBaseSubscriptionCallback::OnDeallocatePaths(ReadPrepareParams && aReadPrepareParams)
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

void MTRBaseSubscriptionCallback::OnSubscriptionEstablished(SubscriptionId aSubscriptionId)
{
    if (mSubscriptionEstablishedHandler) {
        dispatch_async(mQueue, mSubscriptionEstablishedHandler);
    }
}

CHIP_ERROR MTRBaseSubscriptionCallback::OnResubscriptionNeeded(ReadClient * apReadClient, CHIP_ERROR aTerminationCause)
{
    CHIP_ERROR err = ClusterStateCache::Callback::OnResubscriptionNeeded(apReadClient, aTerminationCause);
    ReturnErrorOnFailure(err);

    if (mResubscriptionCallback != nil) {
        auto callback = mResubscriptionCallback;
        auto error = [MTRError errorForCHIPErrorCode:aTerminationCause];
        auto delayMs = @(apReadClient->ComputeTimeTillNextSubscription());
        dispatch_async(mQueue, ^{
            callback(error, delayMs);
        });
    }
    return CHIP_NO_ERROR;
}

void MTRBaseSubscriptionCallback::ReportError(CHIP_ERROR aError, bool aCancelSubscription)
{
    auto * err = [MTRError errorForCHIPErrorCode:aError];
    if (!err) {
        // Very strange... Someone tried to report a success status as an error?
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
    });

    if (aCancelSubscription) {
        // We can't synchronously delete ourselves, because we're inside one of
        // the ReadClient callbacks and we need to outlive the callback's
        // execution.  Queue an async deletion on the Matter queue (where we are
        // running already).
        //
        // If we now get OnDone, we will ignore that, since we have the deletion
        // posted already, but that's OK even during shutdown: since we are
        // queueing the deletion now, it will be processed before the Matter queue
        // gets paused, which is fairly early in the shutdown process.
        mHaveQueuedDeletion = true;
        dispatch_async(DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
            delete myself;
        });
    }
}
