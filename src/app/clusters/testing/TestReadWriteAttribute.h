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
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/ServerClusterInterface.h>

#include <lib/core/TLVReader.h>

namespace chip {
namespace Test {

// Helper function to read any attribute value of a given type
template <typename T>
CHIP_ERROR ReadClusterAttribute(chip::app::ServerClusterInterface & cluster, const app::ConcreteDataAttributePath & path, T & value)
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);

    app::Testing::ReadOperation readOperation(path);
    std::unique_ptr<app::AttributeValueEncoder> encoder = readOperation.StartEncoding();
    ReturnErrorOnFailure(cluster.ReadAttribute(readOperation.GetRequest(), *encoder).GetUnderlyingError());
    ReturnErrorOnFailure(readOperation.FinishEncoding());

    std::vector<app::Testing::DecodedAttributeData> attributeData;
    ReturnErrorOnFailure(readOperation.GetEncodedIBs().Decode(attributeData));
    VerifyOrReturnError(attributeData.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

    return app::DataModel::Decode(attributeData[0].dataReader, value);
}

// Helper function to read any attribute value of a given type
template <typename T>
CHIP_ERROR ReadClusterAttribute(chip::app::ServerClusterInterface & cluster, AttributeId attr, T & value)
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);

    const auto & paths = cluster.GetPaths();
    // This should be a size-1 span
    VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);
    chip::app::ConcreteAttributePath attributePath(paths[0].mEndpointId, paths[0].mClusterId, attr);
    return ReadClusterAttribute(cluster, attributePath, value);
}

// Helper function to write any attribute value of a given type
template <typename T>
CHIP_ERROR WriteClusterAttribute(chip::app::ServerClusterInterface & cluster, const app::ConcreteAttributePath & path,
                                 const T & value)
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);

    app::Testing::WriteOperation writeOperation(path);
    app::AttributeValueDecoder decoder = writeOperation.DecoderFor(value);
    return cluster.WriteAttribute(writeOperation.GetRequest(), decoder).GetUnderlyingError();
}

// Helper function to write any attribute value of a given type
template <typename T>
CHIP_ERROR WriteClusterAttribute(chip::app::ServerClusterInterface & cluster, AttributeId attr, const T & value)
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);

    const auto & paths = cluster.GetPaths();
    // This should be a size-1 span
    VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);
    chip::app::ConcreteAttributePath attributePath(paths[0].mEndpointId, paths[0].mClusterId, attr);
    return WriteClusterAttribute(cluster, attributePath, value);
}

} // namespace Test
} // namespace chip
