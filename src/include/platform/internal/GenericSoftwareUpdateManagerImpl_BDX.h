/*
 *
 *    Copyright (c) 2019 Google LLC.
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

/**
 *    @file
 *          Provides an generic implementation of SoftwareUpdateManager features
 *          for use on platforms that support BDX.
 */

#ifndef GENERIC_SOFTWARE_UPDATE_MANAGER_IMPL_BDX_H
#define GENERIC_SOFTWARE_UPDATE_MANAGER_IMPL_BDX_H

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/Profiles/bulk-data-transfer/Development/BulkDataTransfer.h>

namespace nl {
namespace Weave {
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

template<class ImplClass>
class GenericSoftwareUpdateManagerImpl_BDX
{
    using BDXTransfer = ::nl::Weave::Profiles::BulkDataTransfer::BDXTransfer;
    using BDXNode = ::nl::Weave::Profiles::BulkDataTransfer::BdxNode;
    using ReceiveAccept = ::nl::Weave::Profiles::BulkDataTransfer::ReceiveAccept;

protected:
    // ===== Members for use by the implementation subclass.

    WEAVE_ERROR DoInit(void);
    WEAVE_ERROR StartImageDownload(char *aURI, uint64_t aStartOffset);
    WEAVE_ERROR GetUpdateSchemeList(::nl::Weave::Profiles::SoftwareUpdate::UpdateSchemeList * aUpdateSchemeList);
    void AbortDownload(void);

private:
    // ===== Private members reserved for use by this class only.

    WEAVE_ERROR PrepareBinding(void);
    WEAVE_ERROR StartDownload(void);
    void ResetState(void);

    static void BlockReceiveHandler(BDXTransfer * aXfer, uint64_t alength, uint8_t * aDataBlock,
                                    bool aIsLastBlock);
    static void ErrorHandler(BDXTransfer * aXfer, WEAVE_ERROR anErrorCode);
    static WEAVE_ERROR ReceiveAcceptHandler(BDXTransfer * aXfer, ReceiveAccept * aEeceiveAcceptMsg);
    static void ReceiveRejectHandler(BDXTransfer * aXfer, nl::Weave::StatusReport * aReport);
    static void XferErrorHandler(BDXTransfer * aXfer, ::nl::Weave::StatusReport * aXferError);
    static void XferDoneHandler(BDXTransfer * aXfer);

    static void HandleBindingEvent(void * apAppState, ::nl::Weave::Binding::EventType aEvent,
                                   const ::nl::Weave::Binding::InEventParam & aInParam,
                                   ::nl::Weave::Binding::OutEventParam & aOutParam);

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }

    ::nl::Weave::Binding * mBinding;
    char * mURI;

    BDXNode mBDXClient;
    BDXTransfer * mBDXTransfer;

    uint64_t mStartOffset;
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericSoftwareUpdateManagerImpl_BDX<SoftwareUpdateManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // GENERIC_SOFTWARE_UPDATE_MANAGER_IMPL_BDX_H
