/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/scenes-server/SceneHandlerImpl.h>

namespace chip {
namespace scenes {

CHIP_ERROR
DefaultSceneHandlerImpl::EncodeAttributeValueList(const List<AttributeValuePairType> & aVlist, MutableByteSpan & serializedBytes)
{
    TLV::TLVWriter writer;
    writer.Init(serializedBytes);
    ReturnErrorOnFailure(app::DataModel::Encode(writer, TLV::AnonymousTag(), aVlist));
    serializedBytes.reduce_size(writer.GetLengthWritten());

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultSceneHandlerImpl::DecodeAttributeValueList(const ByteSpan & serializedBytes,
                                                             DecodableList<AttributeValuePairDecodableType> & aVlist)
{
    TLV::TLVReader reader;

    reader.Init(serializedBytes);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    ReturnErrorOnFailure(aVlist.Decode(reader));

    return CHIP_NO_ERROR;
}

CHIP_ERROR
DefaultSceneHandlerImpl::SerializeAdd(EndpointId endpoint, const ExtensionFieldSetDecodableType & extensionFieldSet,
                                      MutableByteSpan & serializedBytes)
{
    AttributeValuePairType aVPairs[kMaxAvPair];

    size_t pairTotal = 0;
    // Verify size of list
    ReturnErrorOnFailure(extensionFieldSet.attributeValueList.ComputeSize(&pairTotal));
    VerifyOrReturnError(pairTotal <= ArraySize(aVPairs), CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t pairCount  = 0;
    auto pair_iterator = extensionFieldSet.attributeValueList.begin();
    while (pair_iterator.Next())
    {
        aVPairs[pairCount] = pair_iterator.GetValue();
        pairCount++;
    }
    ReturnErrorOnFailure(pair_iterator.GetStatus());
    List<AttributeValuePairType> attributeValueList(aVPairs, pairCount);

    return EncodeAttributeValueList(attributeValueList, serializedBytes);
}

CHIP_ERROR DefaultSceneHandlerImpl::Deserialize(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                                                ExtensionFieldSetType & extensionFieldSet)
{
    DecodableList<AttributeValuePairDecodableType> attributeValueList;

    ReturnErrorOnFailure(DecodeAttributeValueList(serializedBytes, attributeValueList));

    // Verify size of list
    size_t pairTotal = 0;
    ReturnErrorOnFailure(attributeValueList.ComputeSize(&pairTotal));
    VerifyOrReturnError(pairTotal <= ArraySize(mAVPairs), CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t pairCount  = 0;
    auto pair_iterator = attributeValueList.begin();
    while (pair_iterator.Next())
    {
        mAVPairs[pairCount] = pair_iterator.GetValue();
        pairCount++;
    };
    ReturnErrorOnFailure(pair_iterator.GetStatus());

    extensionFieldSet.clusterID          = cluster;
    extensionFieldSet.attributeValueList = mAVPairs;
    extensionFieldSet.attributeValueList.reduce_size(pairCount);

    return CHIP_NO_ERROR;
}

} // namespace scenes
} // namespace chip
