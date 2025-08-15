/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/server-cluster/AttributeListBuilder.h>

#include <app/server-cluster/DefaultServerCluster.h>

namespace chip {
namespace app {

CHIP_ERROR AttributeListBuilder::Append(Span<const DataModel::AttributeEntry> mandatoryAttributes,
                                        Span<const DataModel::AttributeEntry> optionalAttributes,
                                        AttributeSet enabledOptionalAttributes)
{
    // determine how much data to append. This should only be called if generally we have something to append
    size_t append_size = mandatoryAttributes.size();
    for (const auto & entry : optionalAttributes)
    {
        if (enabledOptionalAttributes.IsSet(entry.attributeId))
        {
            append_size++;
        }
    }

    if (append_size > 0)
    {
        // NOTE: ReferenceExisting will APPEND data (and use heap) when some data already
        //       exists in the builder. This is why we ensure AppendCapacity for everything
        //       so that we do not perform extra allocations.
        ReturnErrorOnFailure(mBuilder.EnsureAppendCapacity(append_size + DefaultServerCluster::GlobalAttributes().size()));
        ReturnErrorOnFailure(mBuilder.ReferenceExisting(mandatoryAttributes));

        for (const auto & entry : optionalAttributes)
        {
            if (enabledOptionalAttributes.IsSet(entry.attributeId))
            {
                ReturnErrorOnFailure(mBuilder.Append(entry));
            }
        }
    }

    // NOTE: ReferenceExisting will APPEND data (and use heap) when some data already
    //       exists in the builder.
    return mBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes());
}

CHIP_ERROR AttributeListBuilder::Append(Span<const DataModel::AttributeEntry> mandatoryAttributes,
                                        Span<const OptionalAttributeEntry> optionalAttributes)
{
    // determine how much data to append. This should only be called if generally we have something to append
    size_t append_size = mandatoryAttributes.size();
    for (const auto & entry : optionalAttributes)
    {
        if (entry.enabled)
        {
            append_size++;
        }
    }

    if (append_size > 0)
    {
        // NOTE: ReferenceExisting will APPEND data (and use heap) when some data already
        //       exists in the builder. This is why we ensure AppendCapacity for everything
        //       so that we do not perform extra allocations.
        ReturnErrorOnFailure(mBuilder.EnsureAppendCapacity(append_size + DefaultServerCluster::GlobalAttributes().size()));
        ReturnErrorOnFailure(mBuilder.ReferenceExisting(mandatoryAttributes));

        for (const auto & entry : optionalAttributes)
        {
            if (entry.enabled)
            {
                ReturnErrorOnFailure(mBuilder.Append(entry.metadata));
            }
        }
    }

    // NOTE: ReferenceExisting will APPEND data (and use heap) when some data already
    //       exists in the builder.
    return mBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes());
}
} // namespace app
} // namespace chip
