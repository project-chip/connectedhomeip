/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/support/Span.h>

namespace chip {
namespace Testing {

// Compare two attribute entry lists as sets and ensures that the content is identical
bool EqualAttributeSets(Span<const app::DataModel::AttributeEntry> a, Span<const app::DataModel::AttributeEntry> b);

// Compare two accepted command entry lists as sets ensure that the content is identical
bool EqualAcceptedCommandSets(Span<const app::DataModel::AcceptedCommandEntry> a,
                              Span<const app::DataModel::AcceptedCommandEntry> b);

// Compare two lists of generated command Ids as sets and ensure the content is identical
bool EqualGeneratedCommandSets(Span<const CommandId> a,
                               Span<const CommandId> b);

} // namespace Testing
} // namespace chip
