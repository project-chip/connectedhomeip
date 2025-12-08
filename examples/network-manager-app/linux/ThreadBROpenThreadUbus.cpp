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
    CopyCharSpanToMutableCharSpan("OpenThread BorderRouter"_span, borderRouterName);
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
    VerifyOrReturnError(type == DatasetType::kActive, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!mActiveDataset.IsEmpty(), CHIP_ERROR_NOT_FOUND);
    dataset = mActiveDataset;
    return CHIP_NO_ERROR;
}

using ErrorField = BlobMsgField<uint16_t, CHIP_CTST("Error")>;

void OpenThreadUbusBorderRouterDelegate::SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                                                          ActivateDatasetCallback * callback)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    VerifyOrExit(activeDataset.IsCommissioned(), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mActiveDataset.IsEmpty(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mActivateDatasetCallback == nullptr, err = CHIP_ERROR_BUSY);
    VerifyOrExit(mOtbr.Resolved() && mBorderAgentIDValid, err = CHIP_ERROR_NOT_CONNECTED);

    {
        BlobMsgBuf buf;
        buf.Add("dataset", activeDataset.AsByteSpan());
        ChipLogDetail(AppServer, "SetActiveDataset invoking on %d", mOtbr.ObjectID());
        VerifyOrExit(!ubus_invoke(&mUbusManager.Context(), mOtbr.ObjectID(), "provision", buf.head,
                                  ([](ubus_request * req, int type, blob_attr * msg) {
                                      ErrorField otError;
                                      VerifyOrReturn(BlobMsgParse(msg, otError) && otError.value_or(0) == 0);
                                      *static_cast<decltype(err) *>(req->priv) = CHIP_NO_ERROR;
                                  }),
                                  &err, kInvokeTimeout),
                     err = CHIP_ERROR_INTERNAL);
    }

    mActiveDataset           = activeDataset;
    mActivateDatasetCallback = callback;
    mActivateDatasetSequence = sequenceNum;
    return;

exit:
    callback->OnActivateDatasetComplete(sequenceNum, err);
}

void OpenThreadUbusBorderRouterDelegate::OnDataReceived(blob_attr * msg, bool notification)
{
    BlobMsgField<ByteSpan, CHIP_CTST("BorderAgentId")> borderAgentID;
    BlobMsgField<ByteSpan, CHIP_CTST("ActiveDataset")> activeDataset;
    BlobMsgField<bool, CHIP_CTST("Attached")> attached;
    BlobMsgParse(msg, borderAgentID, attached, activeDataset);

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
