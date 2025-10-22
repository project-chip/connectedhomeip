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

#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/TLVReader.h>
#include <memory>

namespace chip {
namespace Test {

// Helper class for testing clusters.
// Currently it has a generic ReadAttribute() and WriteAttribute().
// It can be extended in the future to support other cluster operations.
// The function template instantiations will succeed only for types that support the expected operations.
// For example, in the case of ReadAttribute(), the call to app::DataModel::Decode() for the given type T should be supported.
// Otherwise, that template instantiation will fail.
// This helper class can also be used with attributes that are lists or strings because it maintains the read/write memory.
// This is why ReadOperation and WriteOperation are data members.
//
// Example of usage:
//
// FixedLabelCluster fixedLabel(kRootEndpointId);
//
// ClusterTester tester(fixedLabel);
// uint32_t features{};
// ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);
//
// DataModel::DecodableList<Structs::LabelStruct::Type> labelList;
// ASSERT_EQ(tester.ReadAttribute(LabelList::Id, labelList), CHIP_NO_ERROR);
// auto it = labelList.begin();
// while (it.Next())
// {
//     ASSERT_GT(it.GetValue().label.size(), 0u);
// }
class ClusterTester
{
public:
    ClusterTester(app::ServerClusterInterface & cluster) : mCluster(cluster) {}

    template <typename T>
    CHIP_ERROR ReadAttribute(AttributeId attr, T & value)
    {
        const auto & paths = mCluster.GetPaths();
        // This must be a size-1 span
        VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);
        app::ConcreteAttributePath attributePath(paths[0].mEndpointId, paths[0].mClusterId, attr);
        mReadOperation = std::make_unique<app::Testing::ReadOperation>(attributePath);

        std::unique_ptr<app::AttributeValueEncoder> encoder = mReadOperation->StartEncoding();
        ReturnErrorOnFailure(mCluster.ReadAttribute(mReadOperation->GetRequest(), *encoder).GetUnderlyingError());
        ReturnErrorOnFailure(mReadOperation->FinishEncoding());

        std::vector<app::Testing::DecodedAttributeData> attributeData;
        ReturnErrorOnFailure(mReadOperation->GetEncodedIBs().Decode(attributeData));
        VerifyOrReturnError(attributeData.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

        return app::DataModel::Decode(attributeData[0].dataReader, value);
    }

    template <typename T>
    CHIP_ERROR WriteAttribute(AttributeId attr, const T & value)
    {
        const auto & paths = mCluster.GetPaths();
        // This must be a size-1 span
        VerifyOrReturnError(paths.size() == 1u, CHIP_ERROR_INCORRECT_STATE);
        app::ConcreteAttributePath attributePath(paths[0].mEndpointId, paths[0].mClusterId, attr);
        mWriteOperation                    = std::make_unique<app::Testing::WriteOperation>(attributePath);
        app::AttributeValueDecoder decoder = mWriteOperation->DecoderFor(value);
        return mCluster.WriteAttribute(mWriteOperation->GetRequest(), decoder).GetUnderlyingError();
    };

private:
    app::ServerClusterInterface & mCluster;
    std::unique_ptr<app::Testing::ReadOperation> mReadOperation;
    std::unique_ptr<app::Testing::WriteOperation> mWriteOperation;
};

} // namespace Test
} // namespace chip
