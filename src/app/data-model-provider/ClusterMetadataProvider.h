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
#include <type_traits>

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace DataModel {

// This template will be specialized for each cluster type, in generated code.
template <class TypeInfo, ClusterId cluster>
struct ClusterMetadataProvider
{
    static_assert(!std::is_same_v<TypeInfo, TypeInfo>,
                  "Metadata provider for this TypeInfo and Cluster is not implemented, try importing "
                  "<clusters/<cluster>/MetadataProvider.h>");
};

} // namespace DataModel
} // namespace app
} // namespace chip
