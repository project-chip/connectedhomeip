/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

#include "AttributePathIB.h"
#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>
#include <app/data-model/Encode.h>
#include <app/data-model/Nullable.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributePathIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("AttributePathIB =");
    PRETTY_PRINT("{");

    // make a copy of the Path reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (!TLV::IsContextTag(reader.GetTag()))
        {
            continue;
        }
        uint32_t tagNum = TLV::TagNumFromTag(reader.GetTag());
        switch (tagNum)
        {
        case to_underlying(Tag::kEnableTagCompression):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kEnableTagCompression))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kEnableTagCompression));
#if CHIP_DETAIL_LOGGING
            {
                bool enableTagCompression;
                ReturnErrorOnFailure(reader.Get(enableTagCompression));
                PRETTY_PRINT("\tenableTagCompression = %s, ", enableTagCompression ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kNode):
            // check if this tag has appeared before

            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kNode))), err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kNode));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                NodeId node;
                reader.Get(node);
                PRETTY_PRINT("\tNode = 0x%" PRIx64 ",", node);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kEndpoint):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kEndpoint))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kEndpoint));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                EndpointId endpoint;
                reader.Get(endpoint);
                PRETTY_PRINT("\tEndpoint = 0x%x,", endpoint);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kCluster):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kCluster))), err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kCluster));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                ClusterId cluster;
                ReturnErrorOnFailure(reader.Get(cluster));
                PRETTY_PRINT("\tCluster = 0x%" PRIx32 ",", cluster);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kAttribute):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kAttribute))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kAttribute));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                AttributeId attribute;
                ReturnErrorOnFailure(reader.Get(attribute));
                PRETTY_PRINT("\tAttribute = " ChipLogFormatMEI ",", ChipLogValueMEI(attribute));
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kListIndex):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kListIndex))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kListIndex));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType() || TLV::kTLVType_Null == reader.GetType(),
                                CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            // We have checked the element is either uint or null
            if (TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint16_t listIndex;
                ReturnErrorOnFailure(reader.Get(listIndex));
                PRETTY_PRINT("\tListIndex = 0x%x,", listIndex);
            }
            else
            {
                PRETTY_PRINT("\tListIndex = Null,");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("}");
    PRETTY_PRINT("\t");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        if ((tagPresenceMask & (1 << to_underlying(Tag::kAttribute))) == 0 &&
            (tagPresenceMask & (1 << to_underlying(Tag::kListIndex))) != 0)
        {
            err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
        }
        else
        {
            err = CHIP_NO_ERROR;
        }
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR AttributePathIB::Parser::GetEnableTagCompression(bool * const apEnableTagCompression) const
{
    return GetSimpleValue(to_underlying(Tag::kEnableTagCompression), TLV::kTLVType_Boolean, apEnableTagCompression);
}

CHIP_ERROR AttributePathIB::Parser::GetNode(NodeId * const apNode) const
{
    return GetUnsignedInteger(to_underlying(Tag::kNode), apNode);
}

CHIP_ERROR AttributePathIB::Parser::GetEndpoint(EndpointId * const apEndpoint) const
{
    return GetUnsignedInteger(to_underlying(Tag::kEndpoint), apEndpoint);
}

CHIP_ERROR AttributePathIB::Parser::GetCluster(ClusterId * const apCluster) const
{
    return GetUnsignedInteger(to_underlying(Tag::kCluster), apCluster);
}

CHIP_ERROR AttributePathIB::Parser::GetAttribute(AttributeId * const apAttribute) const
{
    return GetUnsignedInteger(to_underlying(Tag::kAttribute), apAttribute);
}

CHIP_ERROR AttributePathIB::Parser::GetListIndex(ListIndex * const apListIndex) const
{
    return GetUnsignedInteger(to_underlying(Tag::kListIndex), apListIndex);
}

CHIP_ERROR AttributePathIB::Parser::GetListIndex(DataModel::Nullable<ListIndex> * const apListIndex) const
{
    return GetNullableUnsignedInteger(to_underlying(Tag::kListIndex), apListIndex);
}

CHIP_ERROR AttributePathIB::Parser::GetListIndex(ConcreteDataAttributePath & aAttributePath) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DataModel::Nullable<ListIndex> listIndex;
    err = GetListIndex(&(listIndex));
    if (err == CHIP_NO_ERROR)
    {
        if (listIndex.IsNull())
        {
            aAttributePath.mListOp = ConcreteDataAttributePath::ListOperation::AppendItem;
        }
        else
        {
            // TODO: Add ListOperation::ReplaceItem support. (Attribute path with valid list index)
            err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
        }
    }
    else if (CHIP_END_OF_TLV == err)
    {
        // We do not have the context for the actual data type here. We always set the list operation to not list and the users
        // should interpret it as ReplaceAll when the attribute type is a list.
        aAttributePath.mListOp = ConcreteDataAttributePath::ListOperation::NotList;
        err                    = CHIP_NO_ERROR;
    }
    return err;
}

CHIP_ERROR AttributePathIB::Parser::ParsePath(AttributePathParams & aAttribute) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = GetEndpoint(&(aAttribute.mEndpointId));
    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(!aAttribute.HasWildcardEndpointId(), CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    else if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(InvalidAction));

    err = GetCluster(&aAttribute.mClusterId);
    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(IsValidClusterId(aAttribute.mClusterId), CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    else if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(InvalidAction));

    err = GetAttribute(&aAttribute.mAttributeId);
    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(IsValidAttributeId(aAttribute.mAttributeId), CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    else if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(InvalidAction));

    // A wildcard cluster requires that the attribute path either be
    // wildcard or a global attribute.
    VerifyOrReturnError(!aAttribute.HasWildcardClusterId() || aAttribute.HasWildcardAttributeId() ||
                            IsGlobalAttribute(aAttribute.mAttributeId),
                        CHIP_IM_GLOBAL_STATUS(InvalidAction));

    err = GetListIndex(&aAttribute.mListIndex);
    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(!aAttribute.HasWildcardAttributeId() && !aAttribute.HasWildcardListIndex(),
                            CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    else if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(InvalidAction));
    return CHIP_NO_ERROR;
}

AttributePathIB::Builder & AttributePathIB::Builder::EnableTagCompression(const bool aEnableTagCompression)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(to_underlying(Tag::kEnableTagCompression)), aEnableTagCompression);
    }
    return *this;
}

AttributePathIB::Builder & AttributePathIB::Builder::Node(const NodeId aNode)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kNode)), aNode);
    }
    return *this;
}

AttributePathIB::Builder & AttributePathIB::Builder::Endpoint(const EndpointId aEndpoint)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kEndpoint)), aEndpoint);
    }
    return *this;
}

AttributePathIB::Builder & AttributePathIB::Builder::Cluster(const ClusterId aCluster)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kCluster)), aCluster);
    }
    return *this;
}

AttributePathIB::Builder & AttributePathIB::Builder::Attribute(const AttributeId aAttribute)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kAttribute)), aAttribute);
    }
    return *this;
}

AttributePathIB::Builder & AttributePathIB::Builder::ListIndex(const DataModel::Nullable<chip::ListIndex> & aListIndex)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = DataModel::Encode(*mpWriter, TLV::ContextTag(to_underlying(Tag::kListIndex)), aListIndex);
    }
    return *this;
}

AttributePathIB::Builder & AttributePathIB::Builder::ListIndex(const chip::ListIndex aListIndex)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kListIndex)), aListIndex);
    }
    return *this;
}

AttributePathIB::Builder & AttributePathIB::Builder::EndOfAttributePathIB()
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR AttributePathIB::Builder::Encode(const AttributePathParams & aAttributePathParams)
{
    if (!(aAttributePathParams.HasWildcardEndpointId()))
    {
        Endpoint(aAttributePathParams.mEndpointId);
    }

    if (!(aAttributePathParams.HasWildcardClusterId()))
    {
        Cluster(aAttributePathParams.mClusterId);
    }

    if (!(aAttributePathParams.HasWildcardAttributeId()))
    {
        Attribute(aAttributePathParams.mAttributeId);
    }

    if (!(aAttributePathParams.HasWildcardListIndex()))
    {
        ListIndex(aAttributePathParams.mListIndex);
    }

    EndOfAttributePathIB();
    return GetError();
}

CHIP_ERROR AttributePathIB::Builder::Encode(const ConcreteDataAttributePath & aAttributePath)
{
    Endpoint(aAttributePath.mEndpointId);
    Cluster(aAttributePath.mClusterId);
    Attribute(aAttributePath.mAttributeId);

    if (!aAttributePath.IsListOperation() || aAttributePath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
    {
        /* noop */
    }
    else if (aAttributePath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        ListIndex(DataModel::NullNullable);
    }
    else
    {
        // TODO: Add ListOperation::ReplaceItem support. (Attribute path with valid list index)
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    EndOfAttributePathIB();
    return GetError();
}

} // namespace app
} // namespace chip
