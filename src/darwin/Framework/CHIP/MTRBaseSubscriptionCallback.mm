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
#import "MTRLogging_Internal.h"

#include <lib/support/FibonacciUtils.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;

void MTRBaseSubscriptionCallback::OnReportBegin()
{
    mAttributeReports = [NSMutableArray new];
    mEventReports = [NSMutableArray new];
    if (mReportBeginHandler) {
        mReportBeginHandler();
    }
}

// Reports attribute and event data if any exists
void MTRBaseSubscriptionCallback::ReportData()
{
    // At data reporting time, nil out scheduled or currently running interimReportBlock
    if (mInterimReportBlock) {
        dispatch_block_cancel(mInterimReportBlock); // no-op when running from mInterimReportBlock
        mInterimReportBlock = nil;
    }

    __block NSArray * attributeReports = mAttributeReports;
    mAttributeReports = nil;
    auto attributeCallback = mAttributeReportCallback;

    __block NSArray * eventReports = mEventReports;
    mEventReports = nil;
    auto eventCallback = mEventReportCallback;

    if (attributeCallback != nil && attributeReports.count) {
        attributeCallback(attributeReports);
    }

    if (eventCallback != nil && eventReports.count) {
        eventCallback(eventReports);
    }
}

void MTRBaseSubscriptionCallback::QueueInterimReport()
{
    if (mInterimReportBlock) {
        return;
    }

    mInterimReportBlock = dispatch_block_create(DISPATCH_BLOCK_INHERIT_QOS_CLASS, ^{
        ReportData();
        // Allocate reports arrays to continue accumulation
        mAttributeReports = [NSMutableArray new];
        mEventReports = [NSMutableArray new];
    });

    dispatch_async(DeviceLayer::PlatformMgrImpl().GetWorkQueue(), mInterimReportBlock);
}

void MTRBaseSubscriptionCallback::OnReportEnd()
{
    ReportData();
    if (mReportEndHandler) {
        mReportEndHandler();
    }
}

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
        || (aReadPrepareParams.mDataVersionFilterListSize > 0 && aReadPrepareParams.mpDataVersionFilterList != nullptr));
    if (aReadPrepareParams.mpDataVersionFilterList != nullptr) {
        delete[] aReadPrepareParams.mpDataVersionFilterList;
    }

    VerifyOrDie((aReadPrepareParams.mEventPathParamsListSize == 0 && aReadPrepareParams.mpEventPathParamsList == nullptr)
        || (aReadPrepareParams.mEventPathParamsListSize == 1 && aReadPrepareParams.mpEventPathParamsList != nullptr));
    if (aReadPrepareParams.mpEventPathParamsList) {
        delete aReadPrepareParams.mpEventPathParamsList;
    }
}

void MTRBaseSubscriptionCallback::OnSubscriptionEstablished(SubscriptionId aSubscriptionId)
{
    // ReadClient resets it at ProcessSubscribeResponse after calling OnSubscriptionEstablished, so this is equivalent
    mResubscriptionNumRetries = 0;
    if (mSubscriptionEstablishedHandler) {
        auto subscriptionEstablishedHandler = mSubscriptionEstablishedHandler;
        subscriptionEstablishedHandler();
    }
}

uint32_t MTRBaseSubscriptionCallback::ComputeTimeTillNextSubscription()
{
    uint32_t maxWaitTimeInMsec = 0;
    uint32_t waitTimeInMsec = 0;
    uint32_t minWaitTimeInMsec = 0;

    if (mResubscriptionNumRetries <= CHIP_RESUBSCRIBE_MAX_FIBONACCI_STEP_INDEX) {
        maxWaitTimeInMsec = GetFibonacciForIndex(mResubscriptionNumRetries) * CHIP_RESUBSCRIBE_WAIT_TIME_MULTIPLIER_MS;
    } else {
        maxWaitTimeInMsec = CHIP_RESUBSCRIBE_MAX_RETRY_WAIT_INTERVAL_MS;
    }

    if (maxWaitTimeInMsec != 0) {
        minWaitTimeInMsec = (CHIP_RESUBSCRIBE_MIN_WAIT_TIME_INTERVAL_PERCENT_PER_STEP * maxWaitTimeInMsec) / 100;
        waitTimeInMsec = minWaitTimeInMsec + (Crypto::GetRandU32() % (maxWaitTimeInMsec - minWaitTimeInMsec));
    }

    return waitTimeInMsec;
}

CHIP_ERROR MTRBaseSubscriptionCallback::OnResubscriptionNeeded(ReadClient * apReadClient, CHIP_ERROR aTerminationCause)
{
    // No need to check ReadClient internal state is Idle because ReadClient only calls OnResubscriptionNeeded after calling ClearActiveSubscriptionState(), which sets the state to Idle.

    // This part is copied from ReadClient's DefaultResubscribePolicy:
    auto timeTillNextResubscription = ComputeTimeTillNextSubscription();
    ChipLogProgress(DataManagement,
        "Will try to resubscribe to %02x:" ChipLogFormatX64 " at retry index %" PRIu32 " after %" PRIu32
        "ms due to error %" CHIP_ERROR_FORMAT,
        apReadClient->GetFabricIndex(), ChipLogValueX64(apReadClient->GetPeerNodeId()), mResubscriptionNumRetries, timeTillNextResubscription,
        aTerminationCause.Format());
    ReturnErrorOnFailure(apReadClient->ScheduleResubscription(timeTillNextResubscription, NullOptional, aTerminationCause == CHIP_ERROR_TIMEOUT));

    // Not as good a place to increment as when resubscription timer fires, but as is, this should be as good, because OnResubscriptionNeeded is only called from ReadClient's Close() while Idle, and nothing should cause this to happen
    mResubscriptionNumRetries++;

    if (mResubscriptionCallback != nil) {
        auto callback = mResubscriptionCallback;
        auto error = [MTRError errorForCHIPErrorCode:aTerminationCause];
        auto delayMs = @(apReadClient->ComputeTimeTillNextSubscription());
        callback(error, delayMs);
    }
    return CHIP_NO_ERROR;
}

void MTRBaseSubscriptionCallback::OnUnsolicitedMessageFromPublisher(ReadClient *)
{
    if (mUnsolicitedMessageFromPublisherHandler) {
        auto unsolicitedMessageFromPublisherHandler = mUnsolicitedMessageFromPublisherHandler;
        unsolicitedMessageFromPublisherHandler();
    }
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

    __block auto * myself = this;

    auto errorCallback = mErrorCallback;
    mErrorCallback = nil;
    mAttributeReportCallback = nil;
    mEventReportCallback = nil;
    auto onDoneHandler = mOnDoneHandler;
    mOnDoneHandler = nil;

    errorCallback(err);
    if (onDoneHandler) {
        onDoneHandler();
    }

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
