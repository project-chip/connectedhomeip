/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "BdxTransferProxyDiagnosticLog.h"

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPMemString.h>
#include <platform/LockTracker.h>

namespace chip {
namespace bdx {

CHIP_ERROR BDXTransferProxyDiagnosticLog::Init(TransferSession * transferSession)
{
    VerifyOrReturnError(nullptr != transferSession, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(nullptr == mTransfer, CHIP_ERROR_INCORRECT_STATE);

    uint16_t fileDesignatorLength = 0;
    auto fileDesignator           = transferSession->GetFileDesignator(fileDesignatorLength);

    VerifyOrReturnError(fileDesignatorLength <= MATTER_ARRAY_SIZE(mFileDesignator), CHIP_ERROR_INVALID_STRING_LENGTH);

    mTransfer          = transferSession;
    mFileDesignatorLen = static_cast<uint8_t>(fileDesignatorLength);
    memcpy(mFileDesignator, fileDesignator, fileDesignatorLength);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BDXTransferProxyDiagnosticLog::Accept()
{
    ReturnErrorOnFailure(EnsureState());
    VerifyOrReturnError(nullptr != mTransfer, CHIP_ERROR_INCORRECT_STATE);

    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode  = TransferControlFlags::kSenderDrive;
    acceptData.MaxBlockSize = mTransfer->GetTransferBlockSize();
    acceptData.StartOffset  = mTransfer->GetStartOffset();
    acceptData.Length       = mTransfer->GetTransferLength();

    return mTransfer->AcceptTransfer(acceptData);
}

CHIP_ERROR BDXTransferProxyDiagnosticLog::Reject(CHIP_ERROR error)
{
    ReturnErrorOnFailure(EnsureState());
    VerifyOrReturnError(nullptr != mTransfer, CHIP_ERROR_INCORRECT_STATE);

    auto statusCode = GetBdxStatusCodeFromChipError(error);
    return mTransfer->AbortTransfer(statusCode);
}

CHIP_ERROR BDXTransferProxyDiagnosticLog::Continue()
{
    ReturnErrorOnFailure(EnsureState());
    VerifyOrReturnError(nullptr != mTransfer, CHIP_ERROR_INCORRECT_STATE);

    return mTransfer->PrepareBlockAck();
}

void BDXTransferProxyDiagnosticLog::Reset()
{
    ReturnOnFailure(EnsureState());
    VerifyOrReturn(nullptr != mTransfer);

    memset(mFileDesignator, 0, sizeof(mFileDesignator));
    mFileDesignatorLen = 0;
    mTransfer          = nullptr;
    mFabricIndex       = kUndefinedFabricIndex;
    mPeerNodeId        = kUndefinedNodeId;
}

CHIP_ERROR BDXTransferProxyDiagnosticLog::EnsureState() const
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(kUndefinedFabricIndex != mFabricIndex, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(kUndefinedNodeId != mPeerNodeId, CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

} // namespace bdx
} // namespace chip
