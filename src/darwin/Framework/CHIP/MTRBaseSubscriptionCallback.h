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

#pragma once

#import "Foundation/Foundation.h"
#import "MTRBaseDevice.h"

#include <app/BufferedReadCallback.h>
#include <app/ClusterStateCache.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventHeader.h>
#include <app/MessageDef/StatusIB.h>
#include <app/ReadClient.h>
#include <app/ReadPrepareParams.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLV.h>

#include <memory>

/**
 * This file defines a base class for subscription callbacks used by
 * MTRBaseDevice and MTRDevice.  This base class handles everything except the
 * actual conversion from the incoming data to the desired data and the dispatch
 * of callbacks to the relevant client queues.  Its callbacks are called on the
 * Matter queue.  This allows MTRDevice and MTRBaseDevice to do any necessary
 * sync cleanup work before dispatching to the client callbacks on the client
 * queue.
 *
 * After onDoneHandler is invoked, this object will at some point delete itself
 * and destroy anything it owns (such as the ReadClient or the
 * ClusterStateCache).  Consumers should drop references to all the relevant
 * objects in that handler.  This deletion will happen on the Matter queue.
 *
 * The desired data is assumed to be NSObjects that can be stored in NSArray.
 */

NS_ASSUME_NONNULL_BEGIN

typedef void (^DataReportCallback)(NSArray * value);
typedef void (^ErrorCallback)(NSError * error);
typedef void (^SubscriptionEstablishedHandler)(void);
typedef void (^OnDoneHandler)(void);
typedef void (^UnsolicitedMessageFromPublisherHandler)(void);

class MTRBaseSubscriptionCallback : public chip::app::ClusterStateCache::Callback {
public:
    MTRBaseSubscriptionCallback(DataReportCallback attributeReportCallback, DataReportCallback eventReportCallback,
        ErrorCallback errorCallback, MTRDeviceResubscriptionScheduledHandler _Nullable resubscriptionCallback,
        SubscriptionEstablishedHandler _Nullable subscriptionEstablishedHandler, OnDoneHandler _Nullable onDoneHandler,
        UnsolicitedMessageFromPublisherHandler _Nullable unsolicitedMessageFromPublisherHandler = NULL)
        : mAttributeReportCallback(attributeReportCallback)
        , mEventReportCallback(eventReportCallback)
        , mErrorCallback(errorCallback)
        , mResubscriptionCallback(resubscriptionCallback)
        , mSubscriptionEstablishedHandler(subscriptionEstablishedHandler)
        , mBufferedReadAdapter(*this)
        , mOnDoneHandler(onDoneHandler)
        , mUnsolicitedMessageFromPublisherHandler(unsolicitedMessageFromPublisherHandler)
    {
    }

    virtual ~MTRBaseSubscriptionCallback()
    {
        // Ensure we release the ReadClient before we tear down anything else,
        // so it can call our OnDeallocatePaths properly.
        mReadClient = nullptr;
    }

    chip::app::BufferedReadCallback & GetBufferedCallback() { return mBufferedReadAdapter; }

    // We need to exist to get a ReadClient, so can't take this as a constructor argument.
    void AdoptReadClient(std::unique_ptr<chip::app::ReadClient> aReadClient) { mReadClient = std::move(aReadClient); }
    void AdoptClusterStateCache(std::unique_ptr<chip::app::ClusterStateCache> aClusterStateCache)
    {
        mClusterStateCache = std::move(aClusterStateCache);
    }

protected:
    // Report an error, which may be due to issues in our own internal state or
    // due to the OnError callback happening.
    //
    // aCancelSubscription should be false for the OnError case, since it will
    // be immediately followed by OnDone and we want to do the deletion there.
    void ReportError(CHIP_ERROR aError, bool aCancelSubscription = true);

private:
    void OnReportBegin() override;

    void OnReportEnd() override;

    // OnEventData and OnAttributeData must be implemented by subclasses.
    void OnEventData(const chip::app::EventHeader & aEventHeader, chip::TLV::TLVReader * apData,
        const chip::app::StatusIB * apStatus) override = 0;

    void OnAttributeData(const chip::app::ConcreteDataAttributePath & aPath, chip::TLV::TLVReader * apData,
        const chip::app::StatusIB & aStatus) override = 0;

    void OnError(CHIP_ERROR aError) override;

    void OnDone(chip::app::ReadClient * aReadClient) override;

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override;

    void OnSubscriptionEstablished(chip::SubscriptionId aSubscriptionId) override;

    CHIP_ERROR OnResubscriptionNeeded(chip::app::ReadClient * apReadClient, CHIP_ERROR aTerminationCause) override;

    void OnUnsolicitedMessageFromPublisher(chip::app::ReadClient * apReadClient) override;

    void ReportData();

protected:
    NSMutableArray * _Nullable mAttributeReports = nil;
    NSMutableArray * _Nullable mEventReports = nil;

private:
    DataReportCallback _Nullable mAttributeReportCallback = nil;
    DataReportCallback _Nullable mEventReportCallback = nil;
    // We set mErrorCallback to nil before calling the error callback, so we
    // make sure to only report one error.
    ErrorCallback _Nullable mErrorCallback = nil;
    MTRDeviceResubscriptionScheduledHandler _Nullable mResubscriptionCallback = nil;
    SubscriptionEstablishedHandler _Nullable mSubscriptionEstablishedHandler = nil;
    UnsolicitedMessageFromPublisherHandler _Nullable mUnsolicitedMessageFromPublisherHandler = nil;
    chip::app::BufferedReadCallback mBufferedReadAdapter;

    // Our lifetime management is a little complicated.  On errors that don't
    // originate with the ReadClient we attempt to delete ourselves (and hence
    // the ReadClient), but asynchronously, because the ReadClient API doesn't
    // allow sync deletion under callbacks other than OnDone.  While that's
    // pending, something else (e.g. an error it runs into) could end up calling
    // OnDone on us.  And generally if OnDone is called we want to delete
    // ourselves as well.
    //
    // To handle this, enforce the following rules:
    //
    // 1) We guarantee that mErrorCallback is only invoked with an error once.
    // 2) We guarantee that mOnDoneHandler is only invoked once, and always
    //    invoked before we delete ourselves.
    // 3) We ensure that we delete ourselves and the passed in ReadClient only
    //    from OnDone or from an error callback but not both, by tracking whether
    //    we have a queued-up deletion.
    std::unique_ptr<chip::app::ReadClient> mReadClient;
    std::unique_ptr<chip::app::ClusterStateCache> mClusterStateCache;
    bool mHaveQueuedDeletion = false;
    OnDoneHandler _Nullable mOnDoneHandler = nil;
};

NS_ASSUME_NONNULL_END
