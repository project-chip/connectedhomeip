/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <app/data-model/DecodableList.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/data-model/List.h>
#include <app/data-model/NullObject.h>
#include <app-common/zap-generated/ids/Attributes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Globals {
namespace Attributes {

namespace GeneratedCommandList {
struct TypeInfo
{
    using Type             = chip::app::DataModel::List<const chip::CommandId>;
    using DecodableType    = chip::app::DataModel::DecodableList<chip::CommandId>;
    using DecodableArgType = const chip::app::DataModel::DecodableList<chip::CommandId> &;

    static constexpr AttributeId GetAttributeId() { return Attributes::GeneratedCommandList::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
struct TypeInfo
{
    using Type             = chip::app::DataModel::List<const chip::CommandId>;
    using DecodableType    = chip::app::DataModel::DecodableList<chip::CommandId>;
    using DecodableArgType = const chip::app::DataModel::DecodableList<chip::CommandId> &;

    static constexpr AttributeId GetAttributeId() { return Attributes::AcceptedCommandList::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace AcceptedCommandList

namespace AttributeList {
struct TypeInfo
{
    using Type             = chip::app::DataModel::List<const chip::AttributeId>;
    using DecodableType    = chip::app::DataModel::DecodableList<chip::AttributeId>;
    using DecodableArgType = const chip::app::DataModel::DecodableList<chip::AttributeId> &;

    static constexpr AttributeId GetAttributeId() { return AttributeList::Id ; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace AttributeList

namespace FeatureMap {
struct TypeInfo
{
    using Type             = uint32_t;
    using DecodableType    = uint32_t;
    using DecodableArgType = uint32_t;

    static constexpr AttributeId GetAttributeId() { return Attributes::FeatureMap::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace FeatureMap

namespace ClusterRevision {
struct TypeInfo
{
    using Type             = uint16_t;
    using DecodableType    = uint16_t;
    using DecodableArgType = uint16_t;

    static constexpr AttributeId GetAttributeId() { return Attributes::ClusterRevision::Id; }
    static constexpr bool MustUseTimedWrite() { return false; }
};
} // namespace ClusterRevision

}
}
} // namespace DataModel
} // namespace app
} // namespace chip
