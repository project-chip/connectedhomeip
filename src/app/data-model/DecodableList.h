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
#include <app/data-model/List.h>
#include <functional>
#include <lib/core/TLV.h>

namespace chip {
namespace app {
namespace DataModel {

namespace detail {

/*
 * Base class of DecodableList to minimize template usage
 */
class DecodableListBase
{
public:
    /*
     * @brief
     *
     * This call stores a TLV reader positioned on the list this class is to manage.
     *
     * Specifically, the passed-in reader should be pointing into the list just after
     * having called `OpenContainer` on the list element.
     */
    void SetReader(const TLV::TLVReader & reader) { mReader.Init(reader); }

    /*
     * @brief
     *
     * This call clears the TLV reader managed by this class, so it can be reused.
     */
    void ClearReader() { mReader.Init(nullptr, 0); }

    /** Use ComputeSize(size_t &) instead; backwards-compatibility. */
    inline CHIP_ERROR ComputeSize(size_t * size) const { return ComputeSize(*size); }

    /*
     * Compute the size of the list. This can fail if the TLV is malformed. If
     * this succeeds, that does not guarantee that the individual items can be
     * successfully decoded; consumers should check Iterator::GetStatus() when
     * actually decoding them. If there is no list then the size is considered
     * to be zero.
     */
    CHIP_ERROR ComputeSize(size_t & size) const
    {
        if (mReader.GetContainerType() == TLV::kTLVType_NotSpecified)
        {
            size = 0;
            return CHIP_NO_ERROR;
        }

        return mReader.CountRemainingInContainer(&size);
    }

    CHIP_ERROR Decode(TLV::TLVReader & reader)
    {
        VerifyOrReturnError(reader.GetType() == TLV::kTLVType_Array, CHIP_ERROR_SCHEMA_MISMATCH);
        TLV::TLVType type;
        ReturnErrorOnFailure(reader.EnterContainer(type));
        SetReader(reader);
        ReturnErrorOnFailure(reader.ExitContainer(type));
        return CHIP_NO_ERROR;
    }

protected:
    DecodableListBase() { ClearReader(); }

    class Iterator
    {
    public:
        Iterator(const TLV::TLVReader & reader)
        {
            mStatus = CHIP_NO_ERROR;
            mReader.Init(reader);
        }

        bool Next()
        {
            if (mStatus != CHIP_NO_ERROR || mReader.GetContainerType() == TLV::kTLVType_NotSpecified)
            {
                return false;
            }

            mStatus = mReader.Next();

            return (mStatus == CHIP_NO_ERROR);
        }

        /*
         * Returns the result of all previous operations on this iterator.
         *
         * Notably, if the end-of-list was encountered in a previous call to Next,
         * the status returned shall be CHIP_NO_ERROR.
         */
        CHIP_ERROR GetStatus() const
        {
            if (mStatus == CHIP_END_OF_TLV)
            {
                return CHIP_NO_ERROR;
            }

            return mStatus;
        }

    protected:
        CHIP_ERROR mStatus;
        TLV::TLVReader mReader;
    };

    TLV::TLVReader mReader;
};

template <bool IsFabricScoped>
class FabricIndexListMemberMixin
{
};

template <>
class FabricIndexListMemberMixin<true>
{
public:
    void SetFabricIndex(FabricIndex fabricIndex) { mFabricIndex.SetValue(fabricIndex); }

protected:
    Optional<FabricIndex> mFabricIndex;
};

template <bool IsFabricScoped>
class FabricIndexIteratorMemberMixin
{
};

template <>
class FabricIndexIteratorMemberMixin<true>
{
public:
    FabricIndexIteratorMemberMixin(const Optional<FabricIndex> & fabricindex) : mFabricIndex(fabricindex) {}

protected:
    const Optional<FabricIndex> mFabricIndex;
};

template <bool IsFabricScoped>
class DecodableMaybeFabricScopedList : public DecodableListBase, public FabricIndexListMemberMixin<IsFabricScoped>
{
public:
    static constexpr bool kIsFabricScoped = IsFabricScoped;

protected:
    class Iterator : protected DecodableListBase::Iterator, protected FabricIndexIteratorMemberMixin<IsFabricScoped>
    {
    public:
        template <bool IsActuallyFabricScoped = IsFabricScoped, std::enable_if_t<IsActuallyFabricScoped, bool> = true>
        Iterator(const TLV::TLVReader & reader, const Optional<FabricIndex> & fabricIndex) :
            DecodableListBase::Iterator(reader), FabricIndexIteratorMemberMixin<IsFabricScoped>(fabricIndex)
        {}

        template <bool IsActuallyFabricScoped = IsFabricScoped, std::enable_if_t<!IsActuallyFabricScoped, bool> = true>
        Iterator(const TLV::TLVReader & reader) : DecodableListBase::Iterator(reader)
        {}
    };
};

} // namespace detail

/*
 * @brief
 *
 * This class provides an iteratable decoder of list items within TLV payloads
 * such that no memory has to be provided ahead of time to store the entirety of the decoded
 * list contents.
 *
 * Typical use of a DecodableList looks like this:
 *
 *    CHIP_ERROR err = list.for_each([&](auto & entry, bool &breakLoop) -> CHIP_ERROR {
 *        // Do whatever with entry
 *        // Returning something other than CHIP_NO_ERROR will abort iteration
 *        // Setting breakLoop = true will also abort iteration
 *    });
 *    // If err is failure, it is the non-success status of the lambda, or decoding failed somewhere along the way.  Some valid
 *    // entries may have been processed already.
 *
 */
template <typename T>
class DecodableList : public detail::DecodableMaybeFabricScopedList<DataModel::IsFabricScoped<T>::value>
{
public:
    using Super = detail::DecodableMaybeFabricScopedList<DataModel::IsFabricScoped<T>::value>;
    using Super::kIsFabricScoped;

    DecodableList() {}

    class Iterator : private detail::DecodableMaybeFabricScopedList<DataModel::IsFabricScoped<T>::value>::Iterator
    {
        using IteratorBase = typename detail::DecodableMaybeFabricScopedList<DataModel::IsFabricScoped<T>::value>::Iterator;

    public:
        using IteratorBase::GetStatus;

        /*
         * Initialize the iterator with a reference to a reader.
         *
         * This reader should be pointing into the list just after
         * having called `OpenContainer` on the list element, or should
         * have a `kTLVType_NotSpecified` container type if there is
         * no list.
         */
        template <typename T0 = T, std::enable_if_t<DataModel::IsFabricScoped<T0>::value, bool> = true>
        inline Iterator(const DecodableList & list) : IteratorBase(list.mReader, list.mFabricIndex)
        {}

        template <typename T0 = T, std::enable_if_t<!DataModel::IsFabricScoped<T0>::value, bool> = true>
        inline Iterator(const DecodableList & list) : IteratorBase(list.mReader)
        {}

        /*
         * Increments the iterator to point to the next list element
         * if a valid one exists, and decodes the list element into
         * the internal value storage.
         *
         * If an element does exist and was successfully decoded, this
         * shall return true.
         *
         * Otherwise, if the end of list is reached, or there was no list,
         * this call shall return false.
         *
         * If an error was encountered at any point during the iteration or decode,
         * this shall return false as well. The caller is expected to invoke GetStatus()
         * to retrieve the status of the operation.
         */
        template <typename T0 = T, std::enable_if_t<!DataModel::IsFabricScoped<T0>::value, bool> = true>
        bool Next()
        {
            return IteratorBase::Next() && DecodeValue();
        }

        template <typename T0 = T, std::enable_if_t<DataModel::IsFabricScoped<T0>::value, bool> = true>
        bool Next()
        {
            bool hasNext = IteratorBase::Next() && DecodeValue();

            if (hasNext && this->mFabricIndex.HasValue())
            {
                mValue.SetFabricIndex(this->mFabricIndex.Value());
            }

            return hasNext;
        }

        /*
         * Retrieves a reference to the decoded value, if one
         * was decoded on a previous call to Next().
         */
        const T & GetValue() const { return mValue; }

    private:
        bool DecodeValue()
        {
            if (this->mStatus == CHIP_NO_ERROR)
            {
                //
                // Re-construct mValue to reset its state back to cluster object defaults.
                // This is especially important when decoding successive list elements
                // that do not contain all of the fields for a given struct because
                // they are marked optional/fabric-sensitive. Without this re-construction,
                // data from previous decode attempts will continue to linger and give
                // an incorrect view of the state as seen from a client.
                //
                mValue        = T();
                this->mStatus = DataModel::Decode(this->mReader, mValue);
            }

            return (this->mStatus == CHIP_NO_ERROR);
        }

        T mValue;
    };

    /** Use for_each(BinaryFunc) instead; backwards-compatibility. */
    Iterator begin() const { return Iterator(*this); }

    /**
     * @brief Iterates through all elements in the list, calling iterateFn on each element.
     * The T element passed in to iterateFn has a guaranteed lifetime of the method call
     * @tparam BinaryFunc a function of type std::function<CHIP_ERROR(T & entry, bool & breakLoop)>; template arg for GCC inlining
     * efficiency for lambdas
     * @param iterateFn the function to call on each element. if this function returns an error result or sets breakLoop=true,
     * iteration stops and for_each returns that same error result.
     */
    template <class BinaryFunc>
    __attribute__((always_inline)) inline CHIP_ERROR for_each(BinaryFunc iterateFn) const
    {
        Iterator iter(*this);
        while (iter.Next())
        {
            bool breakLoop = false;
            ReturnErrorOnFailure(iterateFn(iter.GetValue(), breakLoop));
            if (breakLoop)
            {
                break;
            }
        }
        return iter.GetStatus();
    }
};

template <typename X, std::enable_if_t<!DataModel::IsFabricScoped<X>::value, bool> = true>
inline CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, DecodableList<X> list)
{
    TLV::TLVType type;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, type));
    CHIP_ERROR result = list.for_each([&writer](const auto & element, bool &) -> CHIP_ERROR {
        ReturnErrorOnFailure(Encode(writer, TLV::AnonymousTag(), element));
        return CHIP_NO_ERROR;
    });
    ReturnErrorOnFailure(result);
    return writer.EndContainer(type);
}

template <typename X, std::enable_if_t<DataModel::IsFabricScoped<X>::value, bool> = true>
inline CHIP_ERROR EncodeForWrite(TLV::TLVWriter & writer, TLV::Tag tag, DecodableList<X> list)
{
    TLV::TLVType type;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, type));
    CHIP_ERROR result = list.for_each([&writer](const auto & element, bool &) -> CHIP_ERROR {
        ReturnErrorOnFailure(EncodeForWrite(writer, TLV::AnonymousTag(), element));
        return CHIP_NO_ERROR;
    });
    ReturnErrorOnFailure(result);
    return writer.EndContainer(type);
}

template <typename X, std::enable_if_t<DataModel::IsFabricScoped<X>::value, bool> = true>
inline CHIP_ERROR EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex accessingFabricIndex, DecodableList<X> list)
{
    TLV::TLVType type;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, type));
    CHIP_ERROR result = list.for_each([&writer](const auto & element, bool &) -> CHIP_ERROR {
        ReturnErrorOnFailure(EncodeForRead(writer, TLV::AnonymousTag(), element));
        return CHIP_NO_ERROR;
    });
    ReturnErrorOnFailure(result);
    return writer.EndContainer(type);
}

} // namespace DataModel
} // namespace app
} // namespace chip
