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
#include <lib/support/JniTypeWrappers.h>
#include <list>
#include <unordered_map>
#include <utility>

namespace chip {
namespace Controller {

CHIP_ERROR CreateChipAttributePath(JNIEnv * env, const app::ConcreteDataAttributePath & aPath, jobject & outObj);

// Callback for success and failure cases of GetConnectedDevice().
struct GetConnectedDeviceCallback
{
    GetConnectedDeviceCallback(
        jobject wrapperCallback, jobject javaCallback,
        const char * callbackClassSignature = "chip/devicecontroller/GetConnectedDeviceCallbackJni$GetConnectedDeviceCallback");
    ~GetConnectedDeviceCallback();

    static void OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const OperationalSessionSetup::ConnnectionFailureInfo & failureInfo);

    Callback::Callback<OnDeviceConnected> mOnSuccess;
    Callback::Callback<OperationalSessionSetup::OnSetupFailure> mOnFailure;
    JniGlobalReference mWrapperCallbackRef;
    JniGlobalReference mJavaCallbackRef;

private:
    const char * mCallbackClassSignature = nullptr;
};

struct ReportCallback : public app::ClusterStateCache::Callback
{
    /** Subscription established callback can be nullptr. */
    ReportCallback(jobject wrapperCallback, jobject subscriptionEstablishedCallback, jobject resubscriptionAttemptCallback,
                   const char * nodeStateClassSignature);
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
    void ReportError(const app::ConcreteAttributePath * attributePath, const app::ConcreteEventPath * eventPath, CHIP_ERROR err);
    void ReportError(const app::ConcreteAttributePath * attributePath, const app::ConcreteEventPath * eventPath,
                     Protocols::InteractionModel::Status status);
    void ReportError(const app::ConcreteAttributePath * attributePath, const app::ConcreteEventPath * eventPath,
                     const char * message, ChipError::StorageType errorCode);

    void UpdateClusterDataVersion();

    app::ReadClient * mReadClient = nullptr;

    app::ClusterStateCache mClusterCacheAdapter;
    JniGlobalReference mWrapperCallbackRef;
    JniGlobalReference mSubscriptionEstablishedCallbackRef;
    JniGlobalReference mResubscriptionAttemptCallbackRef;

    const char * mNodeStateClassSignature;
};

struct WriteAttributesCallback : public app::WriteClient::Callback
{
    WriteAttributesCallback(jobject wrapperCallback);
    ~WriteAttributesCallback();
    app::WriteClient::Callback * GetChunkedWriteCallback() { return &mChunkedWriteCallback; }

    void OnResponse(const app::WriteClient * apWriteClient, const app::ConcreteDataAttributePath & aPath,
                    app::StatusIB aStatus) override;
    /** Report errors back to Java layer. attributePath may be nullptr for general errors. */
    void OnError(const app::WriteClient * apWriteClient, CHIP_ERROR aProtocolError) override;

    void OnDone(app::WriteClient * apWriteClient) override;

    void ReportError(const app::ConcreteAttributePath * attributePath, CHIP_ERROR err);
    void ReportError(const app::ConcreteAttributePath * attributePath, Protocols::InteractionModel::Status status);
    void ReportError(const app::ConcreteAttributePath * attributePath, const char * message, ChipError::StorageType errorCode);

    app::WriteClient * mWriteClient = nullptr;
    app::ChunkedWriteCallback mChunkedWriteCallback;
    JniGlobalReference mWrapperCallbackRef;
};

struct InvokeCallback : public app::CommandSender::Callback
{
    InvokeCallback(jobject wrapperCallback);
    ~InvokeCallback();

    void OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aPath, const app::StatusIB & aStatusIB,
                    TLV::TLVReader * apData) override;
    /** Report errors back to Java layer. attributePath may be nullptr for general errors. */
    void OnError(const app::CommandSender * apCommandSender, CHIP_ERROR aError) override;

    void OnDone(app::CommandSender * apCommandSender) override;

    void ReportError(CHIP_ERROR err);
    void ReportError(Protocols::InteractionModel::Status status);
    void ReportError(const char * message, ChipError::StorageType errorCode);

    app::CommandSender * mCommandSender = nullptr;
    JniGlobalReference mWrapperCallbackRef;
};

struct ExtendableInvokeCallback : public app::CommandSender::ExtendableCallback
{
    ExtendableInvokeCallback(jobject wrapperCallback);
    ~ExtendableInvokeCallback();

    void OnResponse(app::CommandSender * commandSender, const app::CommandSender::ResponseData & aResponseData) override;
    void OnNoResponse(app::CommandSender * commandSender, const app::CommandSender::NoResponseData & aNoResponseData) override;
    void OnError(const app::CommandSender * apCommandSender, const app::CommandSender::ErrorData & aErrorData) override;
    void OnDone(app::CommandSender * apCommandSender) override;

    app::CommandSender * mCommandSender = nullptr;
    JniGlobalReference mWrapperCallbackRef;
};

jlong newConnectedDeviceCallback(JNIEnv * env, jobject self, jobject callback);
void deleteConnectedDeviceCallback(JNIEnv * env, jobject self, jlong callbackHandle);
jlong newReportCallback(JNIEnv * env, jobject self, jobject subscriptionEstablishedCallbackJava,
                        jobject resubscriptionAttemptCallbackJava, const char * nodeStateClassSignature);
void deleteReportCallback(JNIEnv * env, jobject self, jlong callbackHandle);
jlong newWriteAttributesCallback(JNIEnv * env, jobject self);
void deleteWriteAttributesCallback(JNIEnv * env, jobject self, jlong callbackHandle);
jlong newInvokeCallback(JNIEnv * env, jobject self);
void deleteInvokeCallback(JNIEnv * env, jobject self, jlong callbackHandle);
jlong newExtendableInvokeCallback(JNIEnv * env, jobject self);
void deleteExtendableInvokeCallback(JNIEnv * env, jobject self, jlong callbackHandle);

} // namespace Controller
} // namespace chip
