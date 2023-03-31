/*
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
#pragma once

#include <app/BufferedReadCallback.h>
#include <app/CommandSender.h>
#include <app/ReadClient.h>
#include <app/WriteClient.h>
#include <controller/CHIPDeviceController.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <list>
#include <utility>

namespace chip {
namespace Controller {

CHIP_ERROR CreateChipAttributePath(const app::ConcreteDataAttributePath & aPath, jobject & outObj);

// Callback for success and failure cases of GetConnectedDevice().
struct GetConnectedDeviceCallback
{
    GetConnectedDeviceCallback(jobject wrapperCallback, jobject javaCallback);
    ~GetConnectedDeviceCallback();

    static void OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    Callback::Callback<OnDeviceConnected> mOnSuccess;
    Callback::Callback<OnDeviceConnectionFailure> mOnFailure;
    jobject mWrapperCallbackRef = nullptr;
    jobject mJavaCallbackRef    = nullptr;
};

struct ReportCallback : public app::ClusterStateCache::Callback
{
    /** Subscription established callback can be nullptr. */
    ReportCallback(jobject wrapperCallback, jobject subscriptionEstablishedCallback, jobject reportCallback,
                   jobject resubscriptionAttemptCallback);
    ~ReportCallback();

    void OnReportBegin() override;

    void OnReportEnd() override;

    void OnAttributeData(const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                         const app::StatusIB & aStatus) override;
    void OnEventData(const app::EventHeader & aEventHeader, TLV::TLVReader * apData, const app::StatusIB * apStatus) override;

    void OnError(CHIP_ERROR aError) override;

    void OnDone(app::ReadClient *) override;

    void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override;

    CHIP_ERROR OnResubscriptionNeeded(app::ReadClient * apReadClient, CHIP_ERROR aTerminationCause) override;

    void OnDeallocatePaths(app::ReadPrepareParams && aReadPrepareParams) override;

    /** Report errors back to Java layer. attributePath may be nullptr for general errors. */
    void ReportError(jobject attributePath, jobject eventPath, CHIP_ERROR err);
    void ReportError(jobject attributePath, jobject eventPath, Protocols::InteractionModel::Status status);
    void ReportError(jobject attributePath, jobject eventPath, const char * message, ChipError::StorageType errorCode);

    CHIP_ERROR CreateChipEventPath(const app::ConcreteEventPath & aPath, jobject & outObj);

    void UpdateClusterDataVersion();

    app::ReadClient * mReadClient = nullptr;

    app::ClusterStateCache mClusterCacheAdapter;
    jobject mWrapperCallbackRef                 = nullptr;
    jobject mSubscriptionEstablishedCallbackRef = nullptr;
    jobject mResubscriptionAttemptCallbackRef   = nullptr;
    jobject mReportCallbackRef                  = nullptr;
    // NodeState Java object that will be returned to the application.
    jobject mNodeStateObj = nullptr;
    jclass mNodeStateCls  = nullptr;
};

struct ReportEventCallback : public app::ReadClient::Callback
{
    /** Subscription established callback can be nullptr. */
    ReportEventCallback(jobject wrapperCallback, jobject subscriptionEstablishedCallback, jobject reportCallback,
                        jobject resubscriptionAttemptCallback);
    ~ReportEventCallback();

    void OnReportBegin() override;

    void OnReportEnd() override;

    void OnEventData(const app::EventHeader & aEventHeader, TLV::TLVReader * apData, const app::StatusIB * apStatus) override;

    void OnError(CHIP_ERROR aError) override;

    void OnDone(app::ReadClient *) override;

    void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override;

    CHIP_ERROR OnResubscriptionNeeded(app::ReadClient * apReadClient, CHIP_ERROR aTerminationCause) override;

    /** Report errors back to Java layer. attributePath may be nullptr for general errors. */
    void ReportError(jobject eventPath, CHIP_ERROR err);
    void ReportError(jobject eventPath, Protocols::InteractionModel::Status status);
    void ReportError(jobject eventPath, const char * message, ChipError::StorageType errorCode);

    CHIP_ERROR CreateChipEventPath(const app::ConcreteEventPath & aPath, jobject & outObj);

    app::ReadClient * mReadClient = nullptr;

    app::BufferedReadCallback mBufferedReadAdapter;
    jobject mWrapperCallbackRef                 = nullptr;
    jobject mSubscriptionEstablishedCallbackRef = nullptr;
    jobject mResubscriptionAttemptCallbackRef   = nullptr;
    jobject mReportCallbackRef                  = nullptr;
    // NodeState Java object that will be returned to the application.
    jobject mNodeStateObj = nullptr;
    jclass mNodeStateCls  = nullptr;
};

struct WriteAttributesCallback : public app::WriteClient::Callback
{
    WriteAttributesCallback(jobject wrapperCallback, jobject javaCallback);
    ~WriteAttributesCallback();
    app::WriteClient::Callback * GetChunkedWriteCallback() { return &mChunkedWriteCallback; }

    void OnResponse(const app::WriteClient * apWriteClient, const app::ConcreteDataAttributePath & aPath,
                    app::StatusIB aStatus) override;
    /** Report errors back to Java layer. attributePath may be nullptr for general errors. */
    void OnError(const app::WriteClient * apWriteClient, CHIP_ERROR aProtocolError) override;

    void OnDone(app::WriteClient * apWriteClient) override;

    void ReportError(jobject attributePath, CHIP_ERROR err);
    void ReportError(jobject attributePath, Protocols::InteractionModel::Status status);
    void ReportError(jobject attributePath, const char * message, ChipError::StorageType errorCode);

    app::WriteClient * mWriteClient = nullptr;
    app::ChunkedWriteCallback mChunkedWriteCallback;
    jobject mWrapperCallbackRef = nullptr;
    jobject mJavaCallbackRef    = nullptr;
};

struct InvokeCallback : public app::CommandSender::Callback
{
    InvokeCallback(jobject wrapperCallback, jobject javaCallback);
    ~InvokeCallback();

    void OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aPath, const app::StatusIB & aStatusIB,
                    TLV::TLVReader * apData) override;
    /** Report errors back to Java layer. attributePath may be nullptr for general errors. */
    void OnError(const app::CommandSender * apCommandSender, CHIP_ERROR aError) override;

    void OnDone(app::CommandSender * apCommandSender) override;

    CHIP_ERROR CreateInvokeElement(const app::ConcreteCommandPath & aPath, TLV::TLVReader * apData, jobject & outObj);
    void ReportError(CHIP_ERROR err);
    void ReportError(Protocols::InteractionModel::Status status);
    void ReportError(const char * message, ChipError::StorageType errorCode);

    app::CommandSender * mCommandSender = nullptr;
    jobject mWrapperCallbackRef         = nullptr;
    jobject mJavaCallbackRef            = nullptr;
};

} // namespace Controller
} // namespace chip
