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

#include <app/AttributePathParams.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <app/util/ember-io-storage.h>
#include <lib/support/Span.h>

using chip::Protocols::InteractionModel::Status;

namespace {

constexpr size_t kMaxTestIoSize = 128;

uint8_t gEmberIoBuffer[kMaxTestIoSize];
size_t gEmberIoBufferFill;
Status gEmberStatusCode = Status::InvalidAction;

} // namespace

namespace chip {
namespace Test {

void SetEmberReadOutput(std::variant<chip::ByteSpan, Status> what)
{
    if (const chip::ByteSpan * span = std::get_if<chip::ByteSpan>(&what))
    {
        gEmberStatusCode = Status::Success;

        if (span->size() > sizeof(gEmberIoBuffer))
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

ByteSpan GetEmberBuffer()
{
    return ByteSpan(gEmberIoBuffer, gEmberIoBufferFill);
}

} // namespace Test
} // namespace chip

/// TODO: this SHOULD be part of attribute-storage mocks and allow proper I/O control
///       with helpers for "ember encoding"
Status emAfReadOrWriteAttribute(const EmberAfAttributeSearchRecord * attRecord, const EmberAfAttributeMetadata ** metadata,
                                uint8_t * buffer, uint16_t readLength, bool write)
{
    if (gEmberStatusCode != Status::Success)
    {
        return gEmberStatusCode;
    }

    if (write)
    {
        // copy over as much data as possible
        // NOTE: we do NOT use (*metadata)->size since it is unclear if our mocks set that correctly
        size_t len = std::min<size_t>(sizeof(gEmberIoBuffer), readLength);
        memcpy(gEmberIoBuffer, buffer, len);
        gEmberIoBufferFill = len;
    }
    else
    {
        VerifyOrDie(gEmberIoBufferFill <= readLength);
        memcpy(buffer, gEmberIoBuffer, gEmberIoBufferFill);
    }

    return Status::Success;
}

Status emAfWriteAttributeExternal(const chip::app::ConcreteAttributePath & path, const EmberAfWriteDataInput & input)
{
    if (gEmberStatusCode != Status::Success)
    {
        return gEmberStatusCode;
    }

    // ember here deduces the size of dataPtr. For testing however, we KNOW we read
    // out of the ember IO buffer, so we try to use that
    VerifyOrDie(input.dataPtr == chip::app::Compatibility::Internal::gEmberAttributeIOBufferSpan.data());

    // In theory this should do type validation and sizes. This is NOT done for testing.
    // copy over as much data as possible
    // NOTE: we do NOT use (*metadata)->size since it is unclear if our mocks set that correctly
    size_t len = std::min<size_t>(sizeof(gEmberIoBuffer), chip::app::Compatibility::Internal::gEmberAttributeIOBufferSpan.size());

    memcpy(gEmberIoBuffer, input.dataPtr, len);
    gEmberIoBufferFill = len;

    if (input.changeListener != nullptr)
    {
        input.changeListener->MarkDirty(chip::app::AttributePathParams(path.mEndpointId, path.mClusterId, path.mAttributeId));
    }

    return Status::Success;
}

Status emberAfWriteAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID, uint8_t * dataPtr,
                             EmberAfAttributeType dataType)
{
    return emAfWriteAttributeExternal(chip::app::ConcreteAttributePath(endpoint, cluster, attributeID),
                                      EmberAfWriteDataInput(dataPtr, dataType));
}

Status emberAfWriteAttribute(const chip::app::ConcreteAttributePath & path, const EmberAfWriteDataInput & input)
{
    return emAfWriteAttributeExternal(path, input);
}
