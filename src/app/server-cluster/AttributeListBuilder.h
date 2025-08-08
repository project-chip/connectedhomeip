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
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/OptionalAttributes.h>
#include <lib/core/CHIPError.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace chip {
namespace app {

/// Provides a centralized implementation of the very common operation of
/// appending a list of attributes to a ReadOnlyBufferBuilder.
///
/// The intent is that the more complex logic of `Append` to be shared across
/// cluster implementations, so that flash size is kept small.
///
/// Append handles both mandatory and optional attributes and also handles
/// the required auto-add of `GlobalAttributes`.
class AttributeListBuilder
{
public:
    AttributeListBuilder(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) : mBuilder(builder) {}

    /// Constructs a list of cluster attributes, typically for responding to a
    /// `ServerClusterInterface::Attributes` call.
    ///
    /// It allows for one calkl that will add to the buffer all of:
    ///   - mandatoryAttributes (all of them)
    ///   - optionalAttributes IF AND ONLY IF enabledOptionalAttributes is set for them
    ///   - GlobalAttributes()  (all of them)
    CHIP_ERROR Append(Span<const DataModel::AttributeEntry> mandatoryAttributes,
                      Span<const DataModel::AttributeEntry> optionalAttributes, const AttributeSet & enabledOptionalAttributes);

private:
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> & mBuilder;
};

} // namespace app
} // namespace chip
