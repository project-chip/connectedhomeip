/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <controller/CertificateChainRequestTracker.h>

#include <lib/support/SafeInt.h>

namespace chip {
namespace Controller {

void CertificateChainRequestTracker::Reset()
{
    mStorage.Free();
    mCertificateSize = 0;
    mWriteOffset     = 0;
    mTotalDocumentSize.ClearValue();
    mNextSegmentId.ClearValue();
    mHasResponse = false;
}

CHIP_ERROR CertificateChainRequestTracker::HandleResponse(ByteSpan certificate, const Optional<uint16_t> & totalDocumentSize,
                                                          const Optional<uint16_t> & nextSegmentId)
{
    if (totalDocumentSize.HasValue())
    {
        return HandleSegmentedResponse(certificate, totalDocumentSize.Value(), nextSegmentId);
    }

    VerifyOrReturnError(!nextSegmentId.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
    return HandleSingleResponse(certificate);
}

CHIP_ERROR CertificateChainRequestTracker::HandleSingleResponse(ByteSpan certificate)
{
    VerifyOrReturnError(!mHasResponse, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(certificate.size() <= kMaxCertificateDocumentSize, CHIP_ERROR_MESSAGE_TOO_LONG);

    VerifyOrReturnError(mStorage.Alloc(certificate.size()), CHIP_ERROR_NO_MEMORY);

    MutableByteSpan output(mStorage.Get(), certificate.size());
    ReturnErrorOnFailure(CopySpanToMutableSpan(certificate, output));

    mCertificateSize = certificate.size();
    mWriteOffset     = certificate.size();
    mHasResponse     = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CertificateChainRequestTracker::HandleSegmentedResponse(ByteSpan certificate, uint16_t totalDocumentSize,
                                                                   const Optional<uint16_t> & nextSegmentId)
{
    VerifyOrReturnError(totalDocumentSize <= kMaxCertificateDocumentSize, CHIP_ERROR_MESSAGE_TOO_LONG);
    VerifyOrReturnError(certificate.size() <= totalDocumentSize, CHIP_ERROR_INVALID_ARGUMENT);

    if (!mHasResponse)
    {
        VerifyOrReturnError(mStorage.Alloc(totalDocumentSize), CHIP_ERROR_NO_MEMORY);
        mTotalDocumentSize.SetValue(totalDocumentSize);
        mHasResponse = true;
    }
    else
    {
        VerifyOrReturnError(mTotalDocumentSize.HasValue(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mTotalDocumentSize.Value() == totalDocumentSize, CHIP_ERROR_INVALID_ARGUMENT);
    }

    VerifyOrReturnError(CanCastTo<uint16_t>(mWriteOffset + certificate.size()), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mWriteOffset + certificate.size() <= totalDocumentSize, CHIP_ERROR_INVALID_ARGUMENT);

    MutableByteSpan remaining(mStorage.Get() + mWriteOffset, totalDocumentSize - mWriteOffset);
    ReturnErrorOnFailure(CopySpanToMutableSpan(certificate, remaining));
    mWriteOffset += certificate.size();

    if (nextSegmentId.HasValue())
    {
        // Make sure next segment id is the next expected segment id, which should be the current segment id + 1. If nextSegmentId
        // is not provided, it means this is the last segment.
        if (mNextSegmentId.HasValue())
        {
            VerifyOrReturnError(nextSegmentId.Value() == mNextSegmentId.Value() + 1, CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            VerifyOrReturnError(nextSegmentId.Value() == 1, CHIP_ERROR_INVALID_ARGUMENT);
        }
        mNextSegmentId.SetValue(nextSegmentId.Value());
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(mWriteOffset == totalDocumentSize, CHIP_ERROR_INVALID_ARGUMENT);
    mCertificateSize = totalDocumentSize;
    mNextSegmentId.ClearValue();
    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
