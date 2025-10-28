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

// Detection traits for Encode() and EncodeForWrite()
template <typename T, typename = void>
struct HasEncodeForWrite : std::false_type {};

template <typename T>
struct HasEncodeForWrite<
    T,
    std::void_t<decltype(std::declval<T>().EncodeForWrite(std::declval<chip::TLV::TLVWriter &>(),
                                                          chip::TLV::AnonymousTag()))>
> : std::true_type {};

template <typename T, typename = void>
struct HasGenericEncode : std::false_type {};

template <typename T>
struct HasGenericEncode<
    T,
    std::void_t<decltype(std::declval<T>().Encode(std::declval<chip::TLV::TLVWriter &>(),
                                                 chip::TLV::AnonymousTag()))>
> : std::true_type {};


template <typename T>
CHIP_ERROR WriteClusterAttribute(app::ServerClusterInterface & cluster,
                                 const app::ConcreteAttributePath & path,
                                 const T & value)
{
    // Simple scalar / enum path
    if constexpr (std::is_integral_v<T> || std::is_enum_v<T>)
    {
        app::Testing::WriteOperation writeOperation(path);
        app::AttributeValueDecoder decoder = writeOperation.DecoderFor(value);
        return cluster.WriteAttribute(writeOperation.GetRequest(), decoder).GetUnderlyingError();
    }
    else
    {
        // TLV buffer path for structs / ZAP structs
        app::Testing::WriteOperation writeOperation(path);
        writeOperation.SetSubjectDescriptor(chip::app::Testing::kAdminSubjectDescriptor);

        uint8_t buffer[1024];
        chip::TLV::TLVWriter writer;
        writer.Init(buffer);

        if constexpr (HasGenericEncode<T>::value)
        {
            ReturnErrorOnFailure(value.Encode(writer, chip::TLV::AnonymousTag()));
        }
        else if constexpr (HasEncodeForWrite<T>::value)
        {
            ReturnErrorOnFailure(value.EncodeForWrite(writer, chip::TLV::AnonymousTag()));
        }
        else
        {
            static_assert(!sizeof(T), "Type not supported by WriteClusterAttribute");
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        chip::TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        ReturnErrorOnFailure(reader.Next());

        app::AttributeValueDecoder decoder(reader, *writeOperation.GetRequest().subjectDescriptor);
        return cluster.WriteAttribute(writeOperation.GetRequest(), decoder).GetUnderlyingError();
    }
}
template <typename ElementT>
CHIP_ERROR WriteClusterAttribute(app::ServerClusterInterface & cluster,
                                 const app::ConcreteAttributePath & path,
                                 const chip::app::DataModel::List<ElementT> & list)
{
    app::Testing::WriteOperation writeOperation(path);
    writeOperation.SetSubjectDescriptor(chip::app::Testing::kAdminSubjectDescriptor);

    uint8_t buffer[1024];
    chip::TLV::TLVWriter writer;
    writer.Init(buffer);

    chip::TLV::TLVType outer;
    ReturnErrorOnFailure(writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Array, outer));

    for (const auto & item : list)
    {
        if constexpr (std::is_integral_v<ElementT> || std::is_enum_v<ElementT>)
        {
            ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::AnonymousTag(), item));
        }
        else if constexpr (HasGenericEncode<ElementT>::value)
        {
            ReturnErrorOnFailure(item.Encode(writer, chip::TLV::AnonymousTag()));
        }
        else if constexpr (HasEncodeForWrite<ElementT>::value)
        {
            ReturnErrorOnFailure(item.EncodeForWrite(writer, chip::TLV::AnonymousTag()));
        }
        else
        {
            static_assert(!sizeof(ElementT), "Element type not supported in DataModel::List");
        }
    }

    ReturnErrorOnFailure(writer.EndContainer(outer));

    chip::TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ReturnErrorOnFailure(reader.Next());

    app::AttributeValueDecoder decoder(reader, *writeOperation.GetRequest().subjectDescriptor);
    return cluster.WriteAttribute(writeOperation.GetRequest(), decoder).GetUnderlyingError();
}
} // namespace Test
} // namespace chip
