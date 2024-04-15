/*
 *    Copyright (c) 2021-2024 Project CHIP Authors
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

#include <app/MessageDef/AttributeReportIBs.h>
#include <app/data-model/Encode.h>
#include <app/data-model/FabricScoped.h>
#include <app/data-model/List.h> // So we can encode lists
#include <lib/core/CHIPError.h>

#include <type_traits>

namespace chip {
namespace app {

/**
 * The AttributeReportBuilder is a helper class for filling a single report in AttributeReportIBs.
 *
 * Possible usage of AttributeReportBuilder might be:
 *
 * AttributeReportBuilder builder;
 * ReturnErrorOnFailure(builder.PrepareAttribute(...));
 * ReturnErrorOnFailure(builder.Encode(...));
 * ReturnErrorOnFailure(builder.FinishAttribute());
 */
class AttributeReportBuilder
{
public:
    /**
     * PrepareAttribute encodes the "header" part of an attribute report including the path and data version.
     * Path will be encoded according to section 10.5.4.3.1 in the spec.
     * Note: Only append is supported currently (encode a null list index), other operations won't encode a list index in the
     * attribute path field.
     * TODO: Add support for encoding a single element in the list (path with a valid list index).
     */
    CHIP_ERROR PrepareAttribute(AttributeReportIBs::Builder & aAttributeReportIBs, const ConcreteDataAttributePath & aPath,
                                DataVersion aDataVersion);

    /**
     * FinishAttribute encodes the "footer" part of an attribute report (it closes the containers opened in PrepareAttribute)
     */
    CHIP_ERROR FinishAttribute(AttributeReportIBs::Builder & aAttributeReportIBs);

    /**
     * EncodeValue encodes the value field of the report, it should be called exactly once.
     */
    template <typename T, std::enable_if_t<!DataModel::IsFabricScoped<T>::value, bool> = true, typename... Ts>
    CHIP_ERROR EncodeValue(AttributeReportIBs::Builder & aAttributeReportIBs, TLV::Tag tag, T && item, Ts &&... aArgs)
    {
        return DataModel::Encode(*(aAttributeReportIBs.GetAttributeReport().GetAttributeData().GetWriter()), tag, item,
                                 std::forward<Ts>(aArgs)...);
    }

    template <typename T, std::enable_if_t<DataModel::IsFabricScoped<T>::value, bool> = true, typename... Ts>
    CHIP_ERROR EncodeValue(AttributeReportIBs::Builder & aAttributeReportIBs, TLV::Tag tag, FabricIndex accessingFabricIndex,
                           T && item, Ts &&... aArgs)
    {
        return DataModel::EncodeForRead(*(aAttributeReportIBs.GetAttributeReport().GetAttributeData().GetWriter()), tag,
                                        accessingFabricIndex, item, std::forward<Ts>(aArgs)...);
    }
};

} // namespace app
} // namespace chip
