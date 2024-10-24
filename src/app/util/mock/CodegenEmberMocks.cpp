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
#include <app/reporting/reporting.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>

using chip::Protocols::InteractionModel::Status;

void MatterReportingAttributeChangeCallback(const chip::app::ConcreteAttributePath & aPath)
{
    // NOOP currently, exists to satisfy linker only
}

Status emAfReadOrWriteAttribute(const EmberAfAttributeSearchRecord * attRecord, const EmberAfAttributeMetadata ** metadata,
                                uint8_t * buffer, uint16_t readLength, bool write)
{
    if (!write)
    {
        memset(buffer, 0, readLength);
    }
    return Status::Success;
}

Status emAfWriteAttributeExternal(const chip::app::ConcreteAttributePath & path, const EmberAfWriteDataInput & input)
{
    return Status::Success;
}

Status emberAfWriteAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID, uint8_t * dataPtr,
                             EmberAfAttributeType dataType, chip::app::MarkAttributeDirty markDirty)
{
    return emAfWriteAttributeExternal(chip::app::ConcreteAttributePath(endpoint, cluster, attributeID),
                                      EmberAfWriteDataInput(dataPtr, dataType));
}

Status emberAfWriteAttribute(const chip::app::ConcreteAttributePath & path, const EmberAfWriteDataInput & input)
{
    return emAfWriteAttributeExternal(path, input);
}
