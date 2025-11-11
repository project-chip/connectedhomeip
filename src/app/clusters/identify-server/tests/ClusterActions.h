/*
 *
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

#pragma once

#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/reporting/ReportScheduler.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <cassert>
#include <type_traits>

// Helper function to write a value to an attribute.
template <typename T>
CHIP_ERROR WriteAttribute(chip::app::DefaultServerCluster & cluster, chip::AttributeId attributeId, const T & value)
{
    chip::Span<const chip::app::ConcreteClusterPath> paths = cluster.GetPaths();
    assert(paths.size() == 1); // DefaultServerCluster guarantees a single path
    const chip::app::ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, attributeId };

    chip::app::Testing::WriteOperation writeOperation(path);
    chip::app::AttributeValueDecoder decoder = writeOperation.DecoderFor(value);
    return cluster.WriteAttribute(writeOperation.GetRequest(), decoder).GetUnderlyingError();
}

// Helper function to invoke a command and return the result.
template <typename T>
std::optional<chip::app::DataModel::ActionReturnStatus> InvokeCommand(chip::app::DefaultServerCluster & cluster,
                                                                      chip::CommandId commandId, const T & data)
{
    chip::Span<const chip::app::ConcreteClusterPath> paths = cluster.GetPaths();
    assert(paths.size() == 1); // DefaultServerCluster guarantees a single path
    const chip::app::DataModel::InvokeRequest request = { .path = { paths[0].mEndpointId, paths[0].mClusterId, commandId } };

    uint8_t buffer[128];
    chip::TLV::TLVWriter tlvWriter;
    tlvWriter.Init(buffer);
    ReturnErrorOnFailure(data.Encode(tlvWriter, chip::TLV::AnonymousTag()));

    chip::TLV::TLVReader tlvReader;
    tlvReader.Init(buffer, tlvWriter.GetLengthWritten());
    ReturnErrorOnFailure(tlvReader.Next());

    return cluster.InvokeCommand(request, tlvReader, nullptr);
}

// Helper function to read a numeric attribute and decode its value.
template <typename T>
CHIP_ERROR ReadNumericAttribute(chip::app::DefaultServerCluster & cluster, chip::AttributeId attributeId, T & value)
{
    // For complex types (string, lists), Decode() might reference the local variable `attributeData`
    // after this function returns, so we only support numeric attributes.
    static_assert(std::is_arithmetic<T>::value, "This helper only supports numeric attribute types.");

    chip::Span<const chip::app::ConcreteClusterPath> paths = cluster.GetPaths();
    assert(paths.size() == 1);
    const chip::app::ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, attributeId };

    chip::app::Testing::ReadOperation readOperation(path);
    std::unique_ptr<chip::app::AttributeValueEncoder> encoder = readOperation.StartEncoding();
    ReturnErrorOnFailure(cluster.ReadAttribute(readOperation.GetRequest(), *encoder).GetUnderlyingError());
    ReturnErrorOnFailure(readOperation.FinishEncoding());

    std::vector<chip::app::Testing::DecodedAttributeData> attributeData;
    ReturnErrorOnFailure(readOperation.GetEncodedIBs().Decode(attributeData));
    VerifyOrReturnError(attributeData.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

    return chip::app::DataModel::Decode(attributeData[0].dataReader, value);
}
