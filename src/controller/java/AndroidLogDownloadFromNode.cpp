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

#include "AndroidLogDownloadFromNode.h"

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <protocols/bdx/BdxUri.h>

using namespace chip::app::Clusters;

namespace chip {
namespace Controller {

// Max Length is below 8
CharSpan toIntentCharSpan(DiagnosticLogs::IntentEnum intent)
{
    switch (intent)
    {
    case DiagnosticLogs::IntentEnum::kEndUserSupport:
        return CharSpan::fromCharString("EndUser");
    case DiagnosticLogs::IntentEnum::kNetworkDiag:
        return CharSpan::fromCharString("Network");
    case DiagnosticLogs::IntentEnum::kCrashLogs:
        return CharSpan::fromCharString("Crash");
    default:
        return CharSpan();
    }
}

AndroidLogDownloadFromNode::AndroidLogDownloadFromNode(chip::Controller::DeviceController * controller, NodeId remoteNodeId,
                                                       DiagnosticLogs::IntentEnum intent, uint16_t timeout,
                                                       jobject jCallbackObject) :
    mController(controller),
    mOnDeviceConnectedCallback(&OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(&OnDeviceConnectionFailureFn, this),
    mOnBdxTransferCallback(&OnBdxTransferCallback, this), mOnBdxTransferSuccessCallback(&OnBdxTransferSuccessCallback, this),
    mOnBdxTransferFailureCallback(&OnBdxTransferFailureCallback, this)
{
    mRemoteNodeId = remoteNodeId;
    mIntent       = intent;
    mTimeout      = timeout;

    if (mJavaCallback.Init(jCallbackObject) != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Fail to init mJavaObjectRef");
        return;
    }
}

CHIP_ERROR AndroidLogDownloadFromNode::LogDownloadFromNode(DeviceController * controller, NodeId remoteNodeId,
                                                           DiagnosticLogs::IntentEnum intent, uint16_t timeout, jobject jcallback)
{
    VerifyOrReturnValue(controller != nullptr && jcallback != nullptr && remoteNodeId != kUndefinedNodeId,
                        CHIP_ERROR_INVALID_ARGUMENT);

    auto * logDownload = new AndroidLogDownloadFromNode(controller, remoteNodeId, intent, timeout, jcallback);
    VerifyOrReturnValue(logDownload != nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = logDownload->GetConnectedDevice();

    if (err != CHIP_NO_ERROR)
    {
        delete logDownload;
        logDownload = nullptr;
    }
    // Else will clean up when the callback is called.
    return err;
}

CHIP_ERROR AndroidLogDownloadFromNode::GetConnectedDevice()
{
    return mController->GetConnectedDevice(mRemoteNodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}

CHIP_ERROR AndroidLogDownloadFromNode::SendRetrieveLogsRequest(Messaging::ExchangeManager & exchangeMgr,
                                                               const SessionHandle & sessionHandle)
{
    DiagnosticLogs::Commands::RetrieveLogsRequest::Type request;
    request.intent            = mIntent;
    request.requestedProtocol = DiagnosticLogs::TransferProtocolEnum::kBdx;

    CHIP_ERROR err = chip::bdx::MakeURI(mRemoteNodeId, toIntentCharSpan(mIntent), mFileDesignator);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Make BDX URI failure : %" CHIP_ERROR_FORMAT, err.Format());
        FinishLogDownloadFromNode(static_cast<void *>(this), err);
    }

    mBdxReceiver =
        new BdxDiagnosticLogsReceiver(&mOnBdxTransferCallback, &mOnBdxTransferSuccessCallback, &mOnBdxTransferFailureCallback,
                                      mController->GetFabricIndex(), mRemoteNodeId, mFileDesignator);
    VerifyOrReturnValue(mBdxReceiver != nullptr, CHIP_ERROR_NO_MEMORY);

    auto systemState = DeviceControllerFactory::GetInstance().GetSystemState();
    systemState->BDXTransferServer()->SetDelegate(mBdxReceiver);

    if (mTimeout > 0)
    {
        mBdxReceiver->StartBDXTransferTimeout(mTimeout);
    }

    request.transferFileDesignator = MakeOptional(mFileDesignator);
    ClusterBase cluster(exchangeMgr, sessionHandle, 0);

    return cluster.InvokeCommand(request, this, OnResponseRetrieveLogs, OnCommandFailure);
}

void AndroidLogDownloadFromNode::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                                     const SessionHandle & sessionHandle)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    auto * self    = static_cast<AndroidLogDownloadFromNode *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    err = self->SendRetrieveLogsRequest(exchangeMgr, sessionHandle);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Log Download failure : %" CHIP_ERROR_FORMAT, err.Format());
        FinishLogDownloadFromNode(context, err);
    }
}

void AndroidLogDownloadFromNode::OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnDeviceConnectionFailureFn: %" CHIP_ERROR_FORMAT, err.Format());
    FinishLogDownloadFromNode(context, err);
}

void AndroidLogDownloadFromNode::OnResponseRetrieveLogs(void * context,
                                                        const DiagnosticLogs::Commands::RetrieveLogsResponse::DecodableType & data)
{
    auto * self = static_cast<AndroidLogDownloadFromNode *>(context);
    VerifyOrReturn(self != nullptr,
                   ChipLogProgress(Controller, "Success Read Current Fabric index callback with null context. Ignoring"));

    using namespace chip::app::Clusters::DiagnosticLogs;
    if (data.status == StatusEnum::kSuccess)
    {
        ChipLogProgress(Controller, "Success. Will receive log from BDX protocol.");
    }
    else if (data.status == StatusEnum::kExhausted)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        self->OnTransferCallback(self->mController->GetFabricIndex(), self->mRemoteNodeId, data.logContent, &err);
        FinishLogDownloadFromNode(context, err);
    }
    else if (data.status == StatusEnum::kNoLogs)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        self->OnTransferCallback(self->mController->GetFabricIndex(), self->mRemoteNodeId, ByteSpan(), &err);
        FinishLogDownloadFromNode(context, err);
    }
    else if (data.status == StatusEnum::kBusy)
    {
        FinishLogDownloadFromNode(context, CHIP_ERROR_BUSY);
    }
    else if (data.status == StatusEnum::kDenied)
    {
        FinishLogDownloadFromNode(context, CHIP_ERROR_ACCESS_DENIED);
    }
    else
    {
        FinishLogDownloadFromNode(context, CHIP_ERROR_INVALID_DATA_LIST);
    }
}

void AndroidLogDownloadFromNode::OnCommandFailure(void * context, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnCommandFailure %" CHIP_ERROR_FORMAT, err.Format());

    auto * self = static_cast<AndroidLogDownloadFromNode *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Send command failure callback with null context. Ignoring"));

    FinishLogDownloadFromNode(context, err);
}

void AndroidLogDownloadFromNode::FinishLogDownloadFromNode(void * context, CHIP_ERROR err)
{
    auto * self = static_cast<AndroidLogDownloadFromNode *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Finish Log Download with null context. Ignoring"));

    if (self->mBdxReceiver != nullptr)
    {
        if (self->mTimeout > 0 && err != CHIP_ERROR_TIMEOUT)
        {
            self->mBdxReceiver->CancelBDXTransferTimeout();
        }
        delete self->mBdxReceiver;
        self->mBdxReceiver = nullptr;
    }

    CHIP_ERROR jniErr = CHIP_NO_ERROR;
    JNIEnv * env      = JniReferences::GetInstance().GetEnvForCurrentThread();
    JniLocalReferenceScope scope(env);

    jobject jCallback   = self->mJavaCallback.ObjectRef();
    jint jFabricIndex   = static_cast<jint>(self->mController->GetFabricIndex());
    jlong jremoteNodeId = static_cast<jlong>(self->mRemoteNodeId);

    VerifyOrExit(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));

    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Log Download succeeded.");
        jmethodID onSuccessMethod;
        // Java method signature : boolean onSuccess(int fabricIndex, long nodeId)
        jniErr = JniReferences::GetInstance().FindMethod(env, jCallback, "onSuccess", "(IJ)V", &onSuccessMethod);

        VerifyOrExit(jniErr == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onSuccess method"));

        env->CallVoidMethod(jCallback, onSuccessMethod, jFabricIndex, jremoteNodeId);
    }
    else
    {
        ChipLogError(Controller, "Log Download Failed : %" CHIP_ERROR_FORMAT, err.Format());

        jmethodID onErrorMethod;
        // Java method signature : void onError(int fabricIndex, long nodeId, long errorCode)
        jniErr = JniReferences::GetInstance().FindMethod(env, jCallback, "onError", "(IJJ)V", &onErrorMethod);
        VerifyOrExit(jniErr == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onError method"));

        env->CallVoidMethod(jCallback, onErrorMethod, jFabricIndex, jremoteNodeId, static_cast<jlong>(err.AsInteger()));
    }

exit:
    // Finish this function, this object will be deleted.
    delete self;
}

void AndroidLogDownloadFromNode::OnBdxTransferCallback(void * context, FabricIndex fabricIndex, NodeId remoteNodeId,
                                                       const chip::ByteSpan & data, CHIP_ERROR * errInfoOnFailure)
{
    auto * self = static_cast<AndroidLogDownloadFromNode *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Send command failure callback with null context. Ignoring"));

    self->OnTransferCallback(fabricIndex, remoteNodeId, data, errInfoOnFailure);
}

void AndroidLogDownloadFromNode::OnTransferCallback(FabricIndex fabricIndex, NodeId remoteNodeId, const chip::ByteSpan & data,
                                                    CHIP_ERROR * errInfoOnFailure)
{
    VerifyOrReturn(mJavaCallback.HasValidObjectRef(), ChipLogError(Controller, "mJavaCallback is invalid"));

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));

    JniLocalReferenceScope scope(env);

    jmethodID onTransferDataMethod;
    // Java method signature : boolean onTransferData(int fabricIndex, long nodeId, byte[] data)
    *errInfoOnFailure =
        JniReferences::GetInstance().FindMethod(env, mJavaCallback.ObjectRef(), "onTransferData", "(IJ[B)Z", &onTransferDataMethod);
    VerifyOrReturn(*errInfoOnFailure == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onTransferData method"));
    chip::ByteArray dataByteArray(env, data);

    jboolean ret = env->CallBooleanMethod(mJavaCallback.ObjectRef(), onTransferDataMethod, static_cast<jint>(fabricIndex),
                                          static_cast<jlong>(remoteNodeId), dataByteArray.jniValue());

    if (ret != JNI_TRUE)
    {
        ChipLogError(Controller, "Transfer will be rejected.");
        *errInfoOnFailure = CHIP_ERROR_INTERNAL;
    }
}

void AndroidLogDownloadFromNode::OnBdxTransferSuccessCallback(void * context, FabricIndex fabricIndex, NodeId remoteNodeId)
{
    ChipLogProgress(Controller, "OnBdxTransferSuccessCallback");

    auto * self = static_cast<AndroidLogDownloadFromNode *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Send command failure callback with null context. Ignoring"));

    FinishLogDownloadFromNode(context, CHIP_NO_ERROR);
}

void AndroidLogDownloadFromNode::OnBdxTransferFailureCallback(void * context, FabricIndex fabricIndex, NodeId remoteNodeId,
                                                              CHIP_ERROR status)
{
    ChipLogProgress(Controller, "OnBdxTransferFailureCallback: %" CHIP_ERROR_FORMAT, status.Format());

    auto * self = static_cast<AndroidLogDownloadFromNode *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Send command failure callback with null context. Ignoring"));

    FinishLogDownloadFromNode(context, status);
}
} // namespace Controller
} // namespace chip
