/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/data-model/FabricScoped.h>
#include <lib/core/TLV.h>

namespace chip {
namespace app {
namespace DataModel {

/*
 * Dedicated type for list<T> that is at its base, just a Span.
 *
 * Motivated by the need to create distinction between Lists that use Spans
 * vs. other data model types that use Spans (like octetstr). These have different
 * encodings. Consequently, there needs to be an actual C++ type distinction to ensure
 * correct specialization of the Encode/Decode methods.
 *
 */
template <typename T>
struct List : public Span<T>
{
    //
    // The following 'using' statements are needed to make visible
    // all constructors of the base class within this derived class,
    // as well as introduce functions in the base class into the
    // derived class.
    //
    // This is needed to make it seamless to initialize and interact with
    // List<T> instances as though they were just Spans.
    //
    using Span<T>::Span;

    // Inherited copy constructors are _not_ imported by the using statement
    // above, though, so we need to implement that ourselves.  This is templated
    // on the span's type to allow us to init a List<const Foo> from Span<Foo>.
    // Span's constructor handles the checks on the types for us.
    template <class U>
    constexpr List(const Span<U> & other) : Span<T>(other)
    {}

    template <size_t N>
    constexpr List & operator=(T (&databuf)[N])
    {
        Span<T>::operator=(databuf);
        return (*this);
    }

    //
    // A list is deemed fabric scoped if the type of its elements is as well.
    //
    static constexpr bool kIsFabricScoped = DataModel::IsFabricScoped<T>::value;
};

template <typename X>
inline CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, List<X> list)
{
    TLV::TLVType type;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, type));
    for (auto & item : list)
    {
        ReturnErrorOnFailure(Encode(writer, TLV::AnonymousTag(), item));
    }
    ReturnErrorOnFailure(writer.EndContainer(type));

    return CHIP_NO_ERROR;
}

template <typename X, std::enable_if_t<DataModel::IsFabricScoped<X>::value, bool> = true>
inline CHIP_ERROR EncodeForWrite(TLV::TLVWriter & writer, TLV::Tag tag, List<X> list)
{
    TLV::TLVType type;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, type));
    for (auto & item : list)
    {
        ReturnErrorOnFailure(EncodeForWrite(writer, TLV::AnonymousTag(), item));
    }
    ReturnErrorOnFailure(writer.EndContainer(type));

    return CHIP_NO_ERROR;
}

template <typename X, std::enable_if_t<DataModel::IsFabricScoped<X>::value, bool> = true>
inline CHIP_ERROR EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex accessingFabricIndex, List<X> list)
{
    TLV::TLVType type;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, type));
    for (auto & item : list)
    {
        ReturnErrorOnFailure(EncodeForRead(writer, TLV::AnonymousTag(), accessingFabricIndex, item));
    }
    ReturnErrorOnFailure(writer.EndContainer(type));

    return CHIP_NO_ERROR;
}

} // namespace DataModel
} // namespace app
} // namespace chip
