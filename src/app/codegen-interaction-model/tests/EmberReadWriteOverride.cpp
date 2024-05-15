/*
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
#include "EmberReadWriteOverride.h"

#include <app/util/attribute-storage.h>

using chip::Protocols::InteractionModel::Status;

namespace {

constexpr size_t kMaxTesetIoSize = 128;

uint8_t gEmberIoBuffer[kMaxTesetIoSize];
size_t gEmberIoBufferFill;
Status gEmberStatusCode = Status::InvalidAction;

} // namespace

namespace Testing {

void SetEmberReadOutput(std::variant<chip::ByteSpan, Status> what)
{
    if (const chip::ByteSpan * span = std::get_if<chip::ByteSpan>(&what))
    {
        gEmberStatusCode = Status::Success;

        if (span->size() > sizeof(gEmberIoBufferFill))
        {
            ChipLogError(Test, "UNEXPECTED STATE: Too much data set for ember read output");
            gEmberStatusCode = Status::ResourceExhausted;

            return;
        }

        memcpy(gEmberIoBuffer, span->data(), span->size());
        gEmberIoBufferFill = span->size();
        return;
    }

    if (const Status * status = std::get_if<Status>(&what))
    {
        gEmberIoBufferFill = 0;
        gEmberStatusCode   = *status;
        return;
    }

    ChipLogError(Test, "UNEXPECTED STATE: invalid ember read output setting");
    gEmberStatusCode = Status::InvalidAction;
}

} // namespace Testing

/// TODO: this SHOULD be part of attribute-storage mocks and allow proper I/O control
///       with helpers for "ember encoding"
Status emAfReadOrWriteAttribute(const EmberAfAttributeSearchRecord * attRecord, const EmberAfAttributeMetadata ** metadata,
                                uint8_t * buffer, uint16_t readLength, bool write)
{
    if (gEmberStatusCode != Status::Success)
    {
        return gEmberStatusCode;
    }

    if (gEmberIoBufferFill > readLength)
    {
        ChipLogError(Test, "Internal TEST error: insufficient output buffer space.");
        return Status::ResourceExhausted;
    }

    memcpy(buffer, gEmberIoBuffer, gEmberIoBufferFill);
    return Status::Success;
}
