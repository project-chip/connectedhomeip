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

#include <lib/core/TLVReader.h>

namespace chip {
namespace Test {

// Helper function to read any attribute value of a given type
template <typename ClusterT, typename T>
CHIP_ERROR ReadClusterAttribute(ClusterT & cluster, const app::ConcreteDataAttributePath & path, T & value)
{
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
template <typename ClusterT, typename T>
CHIP_ERROR ReadClusterAttribute(ClusterT & cluster, AttributeId attr, T & val)
{
    const auto & paths = cluster.GetPaths();
    // This should be a size-1 span
    VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);
    return ReadClusterAttribute(cluster, chip::app::ConcreteAttributePath(paths[0].mEndpointId, paths[0].mClusterId, attr), val);
}

// Helper function to write any attribute value of a given type
template <typename ClusterT, typename T>
CHIP_ERROR WriteClusterAttribute(ClusterT & cluster, const app::ConcreteAttributePath & path, const T & value)
{
    app::Testing::WriteOperation writeOperation(path);
    app::AttributeValueDecoder decoder = writeOperation.DecoderFor(value);
    return cluster.WriteAttribute(writeOperation.GetRequest(), decoder).GetUnderlyingError();
}

} // namespace Test
} // namespace chip
