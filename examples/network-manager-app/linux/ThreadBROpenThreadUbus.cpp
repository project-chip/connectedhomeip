/*
 *    Copyright (c) 2024-2025 Project CHIP Authors
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

#include "ThreadBROpenThreadUbus.h"

#include "UboxUtils.h"

#include <clusters/ThreadBorderRouterManagement/Attributes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/CHIPDeviceLayer.h>

#include <libubus.h>
#include <optional>

using namespace chip::ubus;
using namespace chip::app::Clusters::ThreadBorderRouterManagement::Attributes;

namespace chip {

static constexpr int kInvokeTimeout = 2000;

CHIP_ERROR OpenThreadUbusBorderRouterDelegate::Init(AttributeChangeCallback * attributeChangeCallback)
{
    mAttributeChangeCallback = attributeChangeCallback;

    // The cluster uses Init() for two opposite things: Startup() passes the
    // callback, and Shutdown() passes nullptr to detach it. The second runs on
    // the way down, by which point ApplicationShutdown() has already stopped
    // the ubus manager -- and registering a watch on a stopped manager is a
    // VerifyOrDie. Detaching is not a fresh initialisation, so stop here.
    VerifyOrReturnValue(attributeChangeCallback != nullptr, CHIP_NO_ERROR);

    mOtbr.SetResolvedCallback([](UbusWatch & watch, void * appState) {
        auto * self = static_cast<decltype(this)>(appState);
        ubus_invoke(&self->mUbusManager.Context(), watch.ObjectID(), "status", nullptr,
                    ([](ubus_request * req, int type, blob_attr * msg) {
                        static_cast<decltype(this)>(req->priv)->OnDataReceived(msg, false);
                    }),
                    self, kInvokeTimeout);
    });
    mOtbr.SetNotificationCallback([](UbusWatch & watch, void * appState, ubus_request_data * req, const char * notification,
                                     blob_attr * msg) { static_cast<decltype(this)>(appState)->OnDataReceived(msg, true); });
    mUbusManager.Register(mOtbr);

    return CHIP_NO_ERROR;
}

void OpenThreadUbusBorderRouterDelegate::GetBorderRouterName(MutableCharSpan & borderRouterName)
{
    CopyCharSpanToMutableCharSpanWithTruncation("OpenThread BorderRouter"_span, borderRouterName);
}

CHIP_ERROR OpenThreadUbusBorderRouterDelegate::GetBorderAgentId(MutableByteSpan & borderAgentId)
{
    return CopySpanToMutableSpan(mBorderAgentIDValid ? ByteSpan(mBorderAgentID) : ByteSpan(), borderAgentId);
}

uint16_t OpenThreadUbusBorderRouterDelegate::GetThreadVersion()
{
    return /* Thread 1.4.0 */ 5;
}

bool OpenThreadUbusBorderRouterDelegate::GetInterfaceEnabled()
{
    return !mActiveDataset.IsEmpty();
}

CHIP_ERROR OpenThreadUbusBorderRouterDelegate::GetDataset(Thread::OperationalDataset & dataset, DatasetType type)
{
    VerifyOrReturnError(type == DatasetType::kActive || type == DatasetType::kPending, CHIP_ERROR_INVALID_ARGUMENT);

    const Thread::OperationalDataset & source = (type == DatasetType::kPending) ? mPendingDataset : mActiveDataset;
    VerifyOrReturnError(!source.IsEmpty(), CHIP_ERROR_NOT_FOUND);
    dataset = source;
    return CHIP_NO_ERROR;
}

using ErrorField = BlobMsgField<uint16_t, CHIP_CTST("Error")>;

namespace {

// Owns the ubus_request of an in-flight provision invocation. otbr does not
// reply to provision until the device has attached, which can take tens of
// seconds, so the request has to outlive SetActiveDataset() and must not
// block the event loop the way ubus_invoke() would.
struct ProvisionRequest
{
    ubus_request req = {};
    OpenThreadUbusBorderRouterDelegate * delegate;
    uint32_t sequence = 0;
    uint16_t otError  = 0;
};

} // namespace

void OpenThreadUbusBorderRouterDelegate::SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                                                          ActivateDatasetCallback * callback)
{
    CHIP_ERROR err            = CHIP_ERROR_INTERNAL;
    ProvisionRequest * invoke = nullptr;
    VerifyOrExit(activeDataset.IsCommissioned(), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mActiveDataset.IsEmpty(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mActivateDatasetCallback == nullptr, err = CHIP_ERROR_BUSY);
    VerifyOrExit(mOtbr.Resolved() && mBorderAgentIDValid, err = CHIP_ERROR_NOT_CONNECTED);

    {
        BlobMsgBuf buf;
        buf.Add("dataset", activeDataset.AsByteSpan());
        ChipLogDetail(AppServer, "SetActiveDataset invoking on %d", mOtbr.ObjectID());
        invoke           = new ProvisionRequest;
        invoke->delegate = this;
        invoke->sequence = sequenceNum;
        VerifyOrExit(!ubus_invoke_async(&mUbusManager.Context(), mOtbr.ObjectID(), "provision", buf.head, &invoke->req),
                     err = CHIP_ERROR_INTERNAL);
    }

    invoke->req.priv    = invoke;
    invoke->req.data_cb = [](ubus_request * req, int type, blob_attr * msg) {
        ErrorField otError;
        VerifyOrReturn(BlobMsgParse(msg, otError));
        static_cast<ProvisionRequest *>(req->priv)->otError = otError.value_or(0);
    };
    invoke->req.complete_cb = [](ubus_request * req, int ret) {
        auto * self = static_cast<ProvisionRequest *>(req->priv);

        // provision replies once the dataset is committed and the join is
        // under way; completing the activation here keeps the Matter command
        // response well inside the controller's interaction timeout, which an
        // attach (>10s even for a lone border router becoming leader) would
        // overrun. The attach itself is reported through the
        // device_role_changed notification and the cluster's attributes.
        // A fail-safe revert may have detached this activation and a new
        // one may have started; complete only the activation this request
        // belongs to.
        if (auto * cb = self->delegate->mActivateDatasetCallback;
            cb != nullptr && self->sequence == self->delegate->mActivateDatasetSequence)
        {
            const bool failed                        = (ret != 0 || self->otError != 0);
            self->delegate->mActivateDatasetCallback = nullptr;
            if (failed)
            {
                self->delegate->mActiveDataset.Clear();
                self->delegate->mActivationPending = false;
                ChipLogError(AppServer, "provision failed: ubus %d, otError %u", ret, self->otError);
            }
            cb->OnActivateDatasetComplete(self->delegate->mActivateDatasetSequence, failed ? CHIP_ERROR_INTERNAL : CHIP_NO_ERROR);
        }
        delete self;
    };
    ubus_complete_request_async(&mUbusManager.Context(), &invoke->req);

    mActiveDataset           = activeDataset;
    mActivateDatasetCallback = callback;
    mActivateDatasetSequence = sequenceNum;
    mActivationPending       = true;
    return;

exit:
    delete invoke;
    callback->OnActivateDatasetComplete(sequenceNum, err);
}

CHIP_ERROR OpenThreadUbusBorderRouterDelegate::InvokeWithDataset(const char * method, const Thread::OperationalDataset & dataset)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    BlobMsgBuf buf;

    VerifyOrReturnError(mOtbr.Resolved(), CHIP_ERROR_NOT_CONNECTED);

    buf.Add("dataset", dataset.AsByteSpan());
    ChipLogDetail(AppServer, "%s invoking on %d", method, mOtbr.ObjectID());
    VerifyOrReturnError(!ubus_invoke(&mUbusManager.Context(), mOtbr.ObjectID(), method, buf.head,
                                     ([](ubus_request * req, int type, blob_attr * msg) {
                                         ErrorField otError;
                                         VerifyOrReturn(BlobMsgParse(msg, otError) && otError.value_or(0) == 0);
                                         *static_cast<CHIP_ERROR *>(req->priv) = CHIP_NO_ERROR;
                                     }),
                                     &err, kInvokeTimeout),
                        CHIP_ERROR_INTERNAL);

    return err;
}

CHIP_ERROR OpenThreadUbusBorderRouterDelegate::SetPendingDataset(const Thread::OperationalDataset & pendingDataset)
{
    // otbr schedules the migration; the switch happens when the dataset's delay
    // timer expires, and the pending_dataset_changed notification reports it.
    return InvokeWithDataset("set_pending", pendingDataset);
}

CHIP_ERROR OpenThreadUbusBorderRouterDelegate::RevertActiveDataset()
{
    // The fail-safe expiry handler calls this for every expired fail-safe,
    // whether or not it carried a dataset activation: a failed commissioning
    // attempt by an unrelated controller must not wipe the network. Only an
    // activation that has not been committed may be reverted.
    VerifyOrReturnError(mActivationPending, CHIP_NO_ERROR);
    mActivationPending = false;
    // The provision may still be in flight; without the callback its
    // completion is a no-op instead of completing the reverted activation
    // (and a new attempt meanwhile would read as Busy).
    mActivateDatasetCallback = nullptr;

    // SetActiveDataset is only accepted when no dataset is configured, so
    // reverting means returning to the unprovisioned state rather than
    // restoring a previous dataset.
    VerifyOrReturnError(mOtbr.Resolved(), CHIP_ERROR_NOT_CONNECTED);

    // deprovision detaches gracefully before erasing, so its reply can be
    // seconds away; fire the request asynchronously and let the otbr
    // notifications resync the cached state. Local state is cleared right
    // away: returning to unprovisioned is the outcome either way.
    mActiveDataset.Clear();
    mAttributeChangeCallback->ReportAttributeChanged(ActiveDatasetTimestamp::Id);

    auto * invoke    = new ProvisionRequest;
    invoke->delegate = this;
    if (ubus_invoke_async(&mUbusManager.Context(), mOtbr.ObjectID(), "deprovision", nullptr, &invoke->req))
    {
        delete invoke;
        return CHIP_ERROR_INTERNAL;
    }
    invoke->req.priv    = invoke;
    invoke->req.data_cb = [](ubus_request * req, int type, blob_attr * msg) {
        ErrorField otError;
        VerifyOrReturn(BlobMsgParse(msg, otError));
        static_cast<ProvisionRequest *>(req->priv)->otError = otError.value_or(0);
    };
    invoke->req.complete_cb = [](ubus_request * req, int ret) {
        auto * self = static_cast<ProvisionRequest *>(req->priv);
        if (ret != 0 || self->otError != 0)
        {
            ChipLogError(AppServer, "deprovision failed: ubus %d, otError %u", ret, self->otError);
        }
        delete self;
    };
    ubus_complete_request_async(&mUbusManager.Context(), &invoke->req);

    return CHIP_NO_ERROR;
}

void OpenThreadUbusBorderRouterDelegate::OnDataReceived(blob_attr * msg, bool notification)
{
    BlobMsgField<ByteSpan, CHIP_CTST("BorderAgentId")> borderAgentID;
    BlobMsgField<ByteSpan, CHIP_CTST("ActiveDataset")> activeDataset;
    BlobMsgField<ByteSpan, CHIP_CTST("PendingDataset")> pendingDataset;
    BlobMsgField<bool, CHIP_CTST("Attached")> attached;
    BlobMsgParse(msg, borderAgentID, attached, activeDataset, pendingDataset);

    if (!mBorderAgentIDValid && borderAgentID.has_value() && borderAgentID->size() == sizeof(mBorderAgentID))
    {
        ChipLogProgress(AppServer, "Received OTBR BorderAgentId");
        memcpy(mBorderAgentID, borderAgentID->data(), sizeof(mBorderAgentID));
        mBorderAgentIDValid = true;
    }

    if (activeDataset.has_value())
    {
        Thread::OperationalDatasetView dataset;
        if (dataset.Init(activeDataset.value()) == CHIP_NO_ERROR)
        {
            ChipLogProgress(AppServer, "Received OTBR ActiveDataset (size = %lu)",
                            static_cast<unsigned long>(activeDataset->size()));
            mActiveDataset = dataset;
            if (notification)
            {
                mAttributeChangeCallback->ReportAttributeChanged(ActiveDatasetTimestamp::Id);
            }
        }
    }

    if (pendingDataset.has_value())
    {
        bool updated = false;
        // An empty payload means a scheduled migration has completed, so the
        // dataset is cleared rather than left reporting a stale timestamp.
        // Only an actual change is reported; a snapshot repeating the known
        // dataset must not wake subscribers.
        if (pendingDataset->empty())
        {
            if (!mPendingDataset.IsEmpty())
            {
                mPendingDataset.Clear();
                updated = true;
            }
        }
        else if (!mPendingDataset.AsByteSpan().data_equal(pendingDataset.value()))
        {
            Thread::OperationalDatasetView dataset;
            if (dataset.Init(pendingDataset.value()) == CHIP_NO_ERROR)
            {
                mPendingDataset = dataset;
                updated         = true;
            }
        }
        if (notification && updated)
        {
            mAttributeChangeCallback->ReportAttributeChanged(PendingDatasetTimestamp::Id);
        }
    }

    if (attached.has_value())
    {
        ChipLogProgress(AppServer, "Received OTBR Attached = %d", attached.value());
        if (attached.value() && mActivateDatasetCallback)
        {
            auto * callback          = mActivateDatasetCallback;
            mActivateDatasetCallback = nullptr;
            callback->OnActivateDatasetComplete(mActivateDatasetSequence, CHIP_NO_ERROR);
        }
    }
}

} // namespace chip
