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
 *          Provides an generic implementation of SoftwareUpdateManager features
 *          for use on platforms that support BDX.
 */

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <profiles/bulk-data-transfer/Development/BulkDataTransfer.h>

namespace chip {
namespace DeviceLayer {

class SoftwareUpdateManagerImpl;

namespace Internal {

/**
 * Provides a generic implementation of SoftwareUpdateManager features for
 * use on platforms that support BDX.
 *
 * This class is intended to be inherited (directly or indirectly) by the SoftwareUpdateManagerImpl
 * class, which also appears as the template's ImplClass parameter.
 *
 */

template <class ImplClass>
class GenericSoftwareUpdateManagerImpl_BDX
{
    using BDXTransfer   = ::chip::Profiles::BulkDataTransfer::BDXTransfer;
    using BDXNode       = ::chip::Profiles::BulkDataTransfer::BdxNode;
    using ReceiveAccept = ::chip::Profiles::BulkDataTransfer::ReceiveAccept;

protected:
    // ===== Members for use by the implementation subclass.

    CHIP_ERROR DoInit(void);
    CHIP_ERROR StartImageDownload(char * aURI, uint64_t aStartOffset);
    CHIP_ERROR GetUpdateSchemeList(::chip::Profiles::SoftwareUpdate::UpdateSchemeList * aUpdateSchemeList);
    void AbortDownload(void);

private:
    // ===== Private members reserved for use by this class only.

    CHIP_ERROR PrepareBinding(void);
    CHIP_ERROR StartDownload(void);
    void ResetState(void);

    static void BlockReceiveHandler(BDXTransfer * aXfer, uint64_t alength, uint8_t * aDataBlock, bool aIsLastBlock);
    static void ErrorHandler(BDXTransfer * aXfer, CHIP_ERROR anErrorCode);
    static CHIP_ERROR ReceiveAcceptHandler(BDXTransfer * aXfer, ReceiveAccept * aEeceiveAcceptMsg);
    static void ReceiveRejectHandler(BDXTransfer * aXfer, chip::StatusReport * aReport);
    static void XferErrorHandler(BDXTransfer * aXfer, ::chip::StatusReport * aXferError);
    static void XferDoneHandler(BDXTransfer * aXfer);

    static void HandleBindingEvent(void * apAppState, ::chip::Binding::EventType aEvent,
                                   const ::chip::Binding::InEventParam & aInParam, ::chip::Binding::OutEventParam & aOutParam);

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }

    ::chip::Binding * mBinding;
    char * mURI;

    BDXNode mBDXClient;
    BDXTransfer * mBDXTransfer;

    uint64_t mStartOffset;
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericSoftwareUpdateManagerImpl_BDX<SoftwareUpdateManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
