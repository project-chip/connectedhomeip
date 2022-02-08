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
#include <app/ReadClient.h>
#include <controller/CHIPDeviceController.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <list>
#include <utility>

namespace chip {
namespace Controller {

// Callback for success and failure cases of GetConnectedDevice().
struct GetConnectedDeviceCallback
{
    GetConnectedDeviceCallback(jobject wrapperCallback, jobject javaCallback);
    ~GetConnectedDeviceCallback();

    static void OnDeviceConnectedFn(void * context, OperationalDeviceProxy * device);
    static void OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error);

    Callback::Callback<OnDeviceConnected> mOnSuccess;
    Callback::Callback<OnDeviceConnectionFailure> mOnFailure;
    jobject mWrapperCallbackRef = nullptr;
    jobject mJavaCallbackRef    = nullptr;
};

struct ReportCallback : public app::ReadClient::Callback
{
    /** Subscription established callback can be nullptr. */
    ReportCallback(jobject wrapperCallback, jobject subscriptionEstablishedCallback, jobject reportCallback);
    ~ReportCallback();

    void OnReportBegin() override;

    void OnReportEnd() override;

    void OnAttributeData(const app::ConcreteDataAttributePath & aPath, DataVersion aVersion, TLV::TLVReader * apData,
                         const app::StatusIB & aStatus) override;

    void OnError(CHIP_ERROR aError) override;

    void OnDone() override;

    void OnSubscriptionEstablished(uint64_t aSubscriptionId) override;

    /** Report errors back to Java layer. attributePath may be nullptr for general errors. */
    void ReportError(jobject attributePath, CHIP_ERROR err);
    void ReportError(jobject attributePath, Protocols::InteractionModel::Status status);
    void ReportError(jobject attributePath, const char * message, ChipError::StorageType errorCode);

    CHIP_ERROR CreateChipAttributePath(const app::ConcreteDataAttributePath & aPath, jobject & outObj);

    app::ReadClient * mReadClient = nullptr;

    app::BufferedReadCallback mBufferedReadAdapter;
    jobject mWrapperCallbackRef                 = nullptr;
    jobject mSubscriptionEstablishedCallbackRef = nullptr;
    jobject mReportCallbackRef                  = nullptr;
    // List of pairs of Java ChipAttributePath and report value. Not using map because jobjects should not be keys.
    std::list<std::pair<jobject, jobject>> mReports;
};

} // namespace Controller
} // namespace chip
