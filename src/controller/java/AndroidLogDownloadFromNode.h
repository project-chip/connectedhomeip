/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include <app/OperationalSessionSetup.h>
#include <controller/CHIPDeviceController.h>
#include <lib/core/CHIPCallback.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#include "BdxDiagnosticLogsReceiver.h"

namespace chip {
namespace Controller {

/**
 * A helper class to download diagnostic log given some parameters.
 */
class AndroidLogDownloadFromNode
{
public:
    /*
     * @brief
     *   Try to look up the device attached to our controller with the given
     *   remote node id and ask it to download diagnostic log.
     *   If function returns an error, callback will never be be executed. Otherwise, callback will always be executed.
     *
     * @param[in] remoteNodeId The remote device Id
     * @param[in] intent Diagnostic log type
     * @param[in] timeout Download log timeout value. If this value is 0, controller does not handle timeouts.
     * @param[in] callback The callback to call when Log Download data is received and when an error occurs
     */
    static CHIP_ERROR LogDownloadFromNode(DeviceController * controller, NodeId remoteNodeId,
                                          app::Clusters::DiagnosticLogs::IntentEnum intent, uint16_t timeout,
                                          jobject jCallbackObject);

private:
    AndroidLogDownloadFromNode(DeviceController * controller, NodeId remoteNodeId, app::Clusters::DiagnosticLogs::IntentEnum intent,
                               uint16_t timeout, jobject javaCallback);

    ~AndroidLogDownloadFromNode() {}

    DeviceController * mController = nullptr;

    chip::Callback::Callback<OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
    chip::Callback::Callback<OnBdxTransfer> mOnBdxTransferCallback;
    chip::Callback::Callback<OnBdxTransferSuccess> mOnBdxTransferSuccessCallback;
    chip::Callback::Callback<OnBdxTransferFailure> mOnBdxTransferFailureCallback;

    chip::JniGlobalReference mJavaCallback;
    NodeId mRemoteNodeId                              = chip::kUndefinedNodeId;
    app::Clusters::DiagnosticLogs::IntentEnum mIntent = app::Clusters::DiagnosticLogs::IntentEnum::kUnknownEnumValue;
    uint16_t mTimeout                                 = 0;

    char mFileDesignatorBuffer[bdx::DiagnosticLogs::kMaxFileDesignatorLen];
    MutableCharSpan mFileDesignator = MutableCharSpan(mFileDesignatorBuffer, bdx::DiagnosticLogs::kMaxFileDesignatorLen);

    BdxDiagnosticLogsReceiver * mBdxReceiver = nullptr;

    CHIP_ERROR GetConnectedDevice();
    CHIP_ERROR SendRetrieveLogsRequest(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    void OnTransferCallback(FabricIndex fabricIndex, NodeId remoteNodeId, const chip::ByteSpan & data,
                            CHIP_ERROR * errInfoOnFailure);
    static void FinishLogDownloadFromNode(void * context, CHIP_ERROR err);

    static void OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    static void OnResponseRetrieveLogs(void * context,
                                       const app::Clusters::DiagnosticLogs::Commands::RetrieveLogsResponse::DecodableType & data);
    static void OnCommandFailure(void * context, CHIP_ERROR err);

    static void OnBdxTransferCallback(void * context, FabricIndex fabricIndex, NodeId remoteNodeId, const chip::ByteSpan & data,
                                      CHIP_ERROR * errInfoOnFailure);
    static void OnBdxTransferSuccessCallback(void * context, FabricIndex fabricIndex, NodeId remoteNodeId);
    static void OnBdxTransferFailureCallback(void * context, FabricIndex fabricIndex, NodeId remoteNodeId, CHIP_ERROR status);
};

} // namespace Controller
} // namespace chip
