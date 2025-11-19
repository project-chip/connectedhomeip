/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/data-model-provider/OperationTypes.h>
#include <app/data-model-provider/tests/TestConstants.h>
#include <app/data-model/Encode.h>
#include <app/data-model/FabricScoped.h>
#include <app/data-model/List.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLVReader.h>
#include <type_traits>

namespace chip {
namespace app {
namespace Testing {

/// Contains support for setting up a WriteAttributeRequest and underlying data.
///
/// It wraps the boilerplate to obtain a AttributeValueDecoder that can be passed in
/// to DataModel::Provider calls.
///
/// Usage:
///
///    WriteOperation operation(1 /* endpoint */, 2 /* cluster */, 3 /* attribute */);
///    test.SetSubjectDescriptor(kAdminSubjectDescriptor);  // optional set access
///
///    AttributeValueDecoder decoder = test.DecoderFor<uint32_t>(0x1234);
///
///    // decoder is usable at this point
///    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);
class WriteOperation
{
public:
    WriteOperation(const ConcreteDataAttributePath & path)
    {
        mRequest.path              = path;
        mRequest.subjectDescriptor = &kDenySubjectDescriptor;
    }

    WriteOperation(EndpointId endpoint, ClusterId cluster, AttributeId attribute) :
        WriteOperation(ConcreteAttributePath(endpoint, cluster, attribute))
    {}

    WriteOperation & SetSubjectDescriptor(const chip::Access::SubjectDescriptor & descriptor)
    {
        mRequest.subjectDescriptor = &descriptor;
        return *this;
    }

    WriteOperation & SetDataVersion(Optional<DataVersion> version)
    {
        mRequest.path.mDataVersion = version;
        return *this;
    }

    WriteOperation & SetWriteFlags(const BitFlags<DataModel::WriteFlags> & flags)
    {
        mRequest.writeFlags = flags;
        return *this;
    }

    WriteOperation & SetOperationFlags(const BitFlags<DataModel::OperationFlags> & flags)
    {
        mRequest.operationFlags = flags;
        return *this;
    }

    WriteOperation & SetPathExpanded(bool value)
    {
        mRequest.path.mExpanded = value;
        return *this;
    }

    const DataModel::WriteAttributeRequest & GetRequest() const { return mRequest; }

    // Helper to encode a value, using EncodeForWrite for fabric-scoped types
    template <typename T>
    CHIP_ERROR EncodeValue(TLV::TLVWriter & writer, TLV::Tag tag, const T & value)
    {
        if constexpr (chip::app::DataModel::IsFabricScoped<T>::value)
        {
            return chip::app::DataModel::EncodeForWrite(writer, tag, value);
        }
        else
        {
            return chip::app::DataModel::Encode(writer, tag, value);
        }
    }

    // Specialization for List types - check if list elements are fabric-scoped
    template <typename T>
    CHIP_ERROR EncodeValue(TLV::TLVWriter & writer, TLV::Tag tag, const chip::app::DataModel::List<T> & value)
    {
        // Check if the element type (removing const) is fabric-scoped
        using ElementType = std::remove_const_t<T>;
        if constexpr (chip::app::DataModel::IsFabricScoped<ElementType>::value)
        {
            return chip::app::DataModel::EncodeForWrite(writer, tag, value);
        }
        else
        {
            return chip::app::DataModel::Encode(writer, tag, value);
        }
    }

    template <typename T>
    TLV::TLVReader ReadEncodedValue(const T & value)
    {
        TLV::TLVWriter writer;
        writer.Init(mTLVBuffer);

        // Encoding is within a structure:
        //   - BEGIN_STRUCT
        //     - 1: .....
        //   - END_STRUCT
        TLV::TLVType outerContainerType;
        SuccessOrDie(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
        SuccessOrDie(EncodeValue(writer, TLV::ContextTag(1), value));
        SuccessOrDie(writer.EndContainer(outerContainerType));
        SuccessOrDie(writer.Finalize());

        TLV::TLVReader reader;
        reader.Init(mTLVBuffer);

        SuccessOrDie(reader.Next());
        SuccessOrDie(reader.EnterContainer(outerContainerType));
        SuccessOrDie(reader.Next());

        return reader;
    }

    template <class T>
    AttributeValueDecoder DecoderFor(const T & value)
    {
        mTLVReader = ReadEncodedValue(value);
        if (mRequest.subjectDescriptor == nullptr)
        {
            AttributeValueDecoder(mTLVReader, kDenySubjectDescriptor);
        }
        return AttributeValueDecoder(mTLVReader, *mRequest.subjectDescriptor);
    }

private:
    constexpr static size_t kMaxTLVBufferSize = 1024;

    DataModel::WriteAttributeRequest mRequest;

    // where data is being written
    uint8_t mTLVBuffer[kMaxTLVBufferSize] = { 0 };

    // tlv reader used for the returned AttributeValueDecoder (since attributeValueDecoder uses references)
    TLV::TLVReader mTLVReader;
};

} // namespace Testing
} // namespace app
} // namespace chip
