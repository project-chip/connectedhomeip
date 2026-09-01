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
#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <credentials/CertificationDeclaration.h>
#include <credentials/CHIPCert.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/Span.h>

namespace chip {
namespace Controller {

class CertificateChainRequestTracker
{
public:
    static constexpr uint16_t kDefaultSegmentSize         = 600;
    static constexpr uint16_t kMaxCertificateDocumentSize = 10240;

    void Reset();

    CHIP_ERROR HandleResponse(ByteSpan certificate, const Optional<uint16_t> & totalDocumentSize,
                              const Optional<uint16_t> & nextSegmentId);

    bool IsComplete() const { return mHasResponse && !mNextSegmentId.HasValue(); }
    bool IsSegmentedTransfer() const { return mTotalDocumentSize.HasValue(); }
    bool HasPendingSegment() const { return mNextSegmentId.HasValue(); }
    Optional<uint16_t> NextSegmentId() const { return mNextSegmentId; }
    ByteSpan GetCertificate() const { return ByteSpan(mStorage.Get(), mCertificateSize); }

private:
    CHIP_ERROR HandleSingleResponse(ByteSpan certificate);
    CHIP_ERROR HandleSegmentedResponse(ByteSpan certificate, uint16_t totalDocumentSize, const Optional<uint16_t> & nextSegmentId);

    Platform::ScopedMemoryBufferWithSize<uint8_t> mStorage;
    size_t mCertificateSize = 0;
    size_t mWriteOffset     = 0;
    Optional<uint16_t> mTotalDocumentSize;
    Optional<uint16_t> mNextSegmentId;
    bool mHasResponse = false;
};

} // namespace Controller
} // namespace chip
