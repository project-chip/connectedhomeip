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

#include "Diagnostics.h"
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>

namespace chip {
namespace Tracing {
namespace Diagnostics {
class DiagnosticStorageImpl : public DiagnosticStorageInterface
{
public:
    static DiagnosticStorageImpl & GetInstance(uint8_t * buffer = nullptr, size_t bufferSize = 0);

    DiagnosticStorageImpl(const DiagnosticStorageImpl &)             = delete;
    DiagnosticStorageImpl & operator=(const DiagnosticStorageImpl &) = delete;

    CHIP_ERROR Store(DiagnosticEntry & diagnostic) override;

    CHIP_ERROR Retrieve(MutableByteSpan & payload) override;

    bool IsEmptyBuffer();

private:
    DiagnosticStorageImpl(uint8_t * buffer, size_t bufferSize);
    DiagnosticStorageImpl();
    ~DiagnosticStorageImpl();

    chip::TLV::TLVCircularBuffer mEndUserCircularBuffer;
};
} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
