/*
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AppConfig.h>
#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/Provider.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {

/**
 * List of attribute ids of attributes that appear on every cluster and have
 * values that are always produced via code, hence do not appear in attribute
 * metadata to save space.  These _must_ appear in order.
 */
constexpr AttributeId GlobalAttributesNotInMetadata[] = {
    Clusters::Globals::Attributes::GeneratedCommandList::Id,
    Clusters::Globals::Attributes::AcceptedCommandList::Id,
    Clusters::Globals::Attributes::AttributeList::Id,
};

static_assert(ArrayIsSorted(GlobalAttributesNotInMetadata), "Array of global attribute ids must be sorted");

bool IsSupportedGlobalAttributeNotInMetadata(AttributeId attributeId);

/**
 * Reads a `IsSupportedGlobalAttributeNotInMetadata` attribute into `encoder`.
 *
 * Preconditions:
 *   - `path` MUST be a valid cluster path inside `provider` and its mAttributeID
 *     MUST be `IsSupportedGlobalAttributeNotInMetadata`
 */
DataModel::ActionReturnStatus ReadGlobalAttributeFromMetadata(DataModel::Provider * provider, const ConcreteAttributePath & path,
                                                              AttributeValueEncoder & encoder);

} // namespace app
} // namespace chip
