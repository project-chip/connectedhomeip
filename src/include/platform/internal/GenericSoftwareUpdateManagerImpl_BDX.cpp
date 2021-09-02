/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

/**
 *    @file
 *          Contains non-inline method definitions for the
 *          GenericSoftwareUpdateManagerImpl<> template.
 */

#ifndef GENERIC_SOFTWARE_UPDATE_MANAGER_IMPL_BDX_CPP
#define GENERIC_SOFTWARE_UPDATE_MANAGER_IMPL_BDX_CPP

#if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

#include <lib/core/CHIPCore.h>
#include <platform/PlatformManager.h>
#include <platform/SoftwareUpdateManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericSoftwareUpdateManagerImpl_BDX.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace ::chip::TLV;
using namespace ::chip::Profiles;
using namespace ::chip::Profiles::Common;
using namespace ::chip::Profiles::BulkDataTransfer;

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericSoftwareUpdateManagerImpl_BDX<SoftwareUpdateManagerImpl>;

template <class ImplClass>
CHIP_ERROR GenericSoftwareUpdateManagerImpl_BDX<ImplClass>::DoInit(void)
{
    CHIP_ERROR err;

    mBinding     = NULL;
    mURI         = NULL;
    mBDXTransfer = NULL;
    mStartOffset = 0;

    err = mBDXClient.Init(&ExchangeMgr);

    return err;
}

template <class ImplClass>
CHIP_ERROR GenericSoftwareUpdateManagerImpl_BDX<ImplClass>::StartImageDownload(char * aURI, uint64_t aStartOffset)
{
    CHIP_ERROR err;

    VerifyOrExit(aURI != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    mURI         = aURI;
    mStartOffset = aStartOffset;

    mBinding = ExchangeMgr.NewBinding(HandleBindingEvent, NULL);
    VerifyOrExit(mBinding != NULL, err = CHIP_ERROR_NO_MEMORY);

    err = mBinding->BeginConfiguration()
              .Target_ServiceEndpoint(CHIP_DEVICE_CONFIG_FILE_DOWNLOAD_ENDPOINT_ID)
              .Transport_UDP_WRM()
              .Exchange_ResponseTimeoutMsec(CHIP_DEVICE_CONFIG_FILE_DOWNLOAD_RESPOSNE_TIMEOUT)
              .Security_SharedCASESession()
              .PrepareBinding();

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericSoftwareUpdateManagerImpl_BDX<ImplClass>::StartDownload(void)
{
    CHIP_ERROR err;

    ReferencedString uri;
    uri.init((uint16_t) strlen(mURI), mURI);

    BDXHandlers handlers = {
        NULL, // SendAcceptHandler
        ReceiveAcceptHandler,
        ReceiveRejectHandler,
        NULL, // GetBlockHandler
        BlockReceiveHandler,
        XferErrorHandler,
        XferDoneHandler,
        ErrorHandler,
    };

    VerifyOrExit(mBDXTransfer == NULL, err = CHIP_ERROR_INCORRECT_STATE);

    err = mBDXClient.NewTransfer(mBinding, handlers, uri, this, mBDXTransfer);
    SuccessOrExit(err);

    // Release our reference to the binding, as it's no longer needed.
    mBinding->Release();
    mBinding = NULL;

    /*
     * This implementation only supports downloading a software image from an offset
     * provided by the application till the end of file. The 0 value in mLength field
     * below indicates that expected length of the transfer is unknown by the initiator
     * at this point and hence the remainder of the file starting from the offset
     * mentioned above is expected to be downloaded in the transfer.
     */
    mBDXTransfer->mMaxBlockSize = CHIP_DEVICE_CONFIG_SWU_BDX_BLOCK_SIZE;
    mBDXTransfer->mStartOffset  = mStartOffset;
    mBDXTransfer->mLength       = 0;

    err = mBDXClient.InitBdxReceive(*mBDXTransfer, true, false, false, NULL);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ResetState();
    }
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericSoftwareUpdateManagerImpl_BDX<ImplClass>::ReceiveAcceptHandler(BDXTransfer * aXfer,
                                                                                 ReceiveAccept * aReceiveAcceptMsg)
{
    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericSoftwareUpdateManagerImpl_BDX<ImplClass>::ReceiveRejectHandler(BDXTransfer * aXfer, StatusReport * aReport)
{
    GenericSoftwareUpdateManagerImpl_BDX<ImplClass> * self = &SoftwareUpdateMgrImpl();

    // Release all resources.
    self->ResetState();

    // If the BDX transfer was rejected by the server with a status report containing status code
    // kStatus_LengthMismatch, it specifically means that the start offset requested by the application
    // in the BDX request is greater than or equal to the length of the file being downloaded. In the context
    // of this implementation, it means that file download is complete since the end of file has already been
    // reached.
    //
    if (aReport->mProfileId == kChipProfile_BDX && aReport->mStatusCode == kStatus_LengthMismatch)
    {
        self->Impl()->DownloadComplete();
    }
    else
    {
        self->Impl()->SoftwareUpdateFailed(CHIP_ERROR_STATUS_REPORT_RECEIVED, aReport);
    }
}

template <class ImplClass>
void GenericSoftwareUpdateManagerImpl_BDX<ImplClass>::BlockReceiveHandler(BDXTransfer * xfr, uint64_t aLength, uint8_t * aDataBlock,
                                                                          bool aIsLastBlock)
{
    CHIP_ERROR err;
    GenericSoftwareUpdateManagerImpl_BDX<ImplClass> * self = &SoftwareUpdateMgrImpl();

    err = self->Impl()->StoreImageBlock(aLength, aDataBlock);
    if (err == CHIP_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED)
    {
        return;
    }
    else if (err != CHIP_NO_ERROR)
    {
        self->ResetState();
        self->Impl()->SoftwareUpdateFailed(err, NULL);
    }
}

template <class ImplClass>
void GenericSoftwareUpdateManagerImpl_BDX<ImplClass>::XferErrorHandler(BDXTransfer * aXfer, StatusReport * aReport)
{
    GenericSoftwareUpdateManagerImpl_BDX<ImplClass> * self = &SoftwareUpdateMgrImpl();

    self->ResetState();
    self->Impl()->SoftwareUpdateFailed(CHIP_ERROR_STATUS_REPORT_RECEIVED, aReport);
}

template <class ImplClass>
void GenericSoftwareUpdateManagerImpl_BDX<ImplClass>::XferDoneHandler(BDXTransfer * aXfer)
{
    GenericSoftwareUpdateManagerImpl_BDX<ImplClass> * self = &SoftwareUpdateMgrImpl();

    self->ResetState();
    self->Impl()->DownloadComplete();
}

template <class ImplClass>
void GenericSoftwareUpdateManagerImpl_BDX<ImplClass>::ErrorHandler(BDXTransfer * aXfer, CHIP_ERROR aErrorCode)
{
    GenericSoftwareUpdateManagerImpl_BDX<ImplClass> * self = &SoftwareUpdateMgrImpl();

    self->ResetState();
    self->Impl()->SoftwareUpdateFailed(aErrorCode, NULL);
}

template <class ImplClass>
void GenericSoftwareUpdateManagerImpl_BDX<ImplClass>::HandleBindingEvent(void * appState, ::chip::Binding::EventType aEvent,
                                                                         const ::chip::Binding::InEventParam & aInParam,
                                                                         ::chip::Binding::OutEventParam & aOutParam)
{
    CHIP_ERROR err                                         = CHIP_NO_ERROR;
    StatusReport * statusReport                            = NULL;
    GenericSoftwareUpdateManagerImpl_BDX<ImplClass> * self = &SoftwareUpdateMgrImpl();

    switch (aEvent)
    {
    case chip::Binding::kEvent_PrepareFailed:
        ChipLogProgress(
            DeviceLayer, "Failed to prepare Software Update BDX binding: %s",
            (aInParam.PrepareFailed.Reason == CHIP_ERROR_STATUS_REPORT_RECEIVED)
                ? StatusReportStr(aInParam.PrepareFailed.StatusReport->mProfileId, aInParam.PrepareFailed.StatusReport->mStatusCode)
                : ErrorStr(aInParam.PrepareFailed.Reason));
        statusReport = aInParam.PrepareFailed.StatusReport;
        err          = aInParam.PrepareFailed.Reason;
        break;

    case chip::Binding::kEvent_BindingFailed:
        ChipLogProgress(DeviceLayer, "Software Update BDX binding failed: %s", ErrorStr(aInParam.BindingFailed.Reason));
        err = aInParam.PrepareFailed.Reason;
        break;

    case chip::Binding::kEvent_BindingReady:
        ChipLogProgress(DeviceLayer, "Software Update BDX binding ready");
        err = self->StartDownload();
        break;

    default:
        chip::Binding::DefaultEventHandler(appState, aEvent, aInParam, aOutParam);
    }

    if (err != CHIP_NO_ERROR)
    {
        self->ResetState();
        self->Impl()->SoftwareUpdateFailed(err, statusReport);
    }
}

template <class ImplClass>
void GenericSoftwareUpdateManagerImpl_BDX<ImplClass>::AbortDownload(void)
{
    ResetState();
}

template <class ImplClass>
void GenericSoftwareUpdateManagerImpl_BDX<ImplClass>::ResetState(void)
{
    if (mBinding != NULL)
    {
        mBinding->Release();
        mBinding = NULL;
    }
    mURI = NULL;
    if (mBDXTransfer)
    {
        mBDXTransfer->Shutdown();
        mBDXTransfer = NULL;
    }
    mStartOffset = 0;
}

template <class ImplClass>
CHIP_ERROR GenericSoftwareUpdateManagerImpl_BDX<ImplClass>::GetUpdateSchemeList(
    ::chip::Profiles::SoftwareUpdate::UpdateSchemeList * aUpdateSchemeList)
{
    uint8_t supportedSchemes[] = { Profiles::SoftwareUpdate::kUpdateScheme_BDX };
    aUpdateSchemeList->init(ArraySize(supportedSchemes), supportedSchemes);

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
#endif // GENERIC_SOFTWARE_UPDATE_MANAGER_IMPL_BDX_CPP
