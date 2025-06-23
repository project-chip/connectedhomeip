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
protected:
    DecodableListBase() {}

    CHIP_ERROR ComputeSize(size_t & size, const TLV::TLVReader & reader) const
    {
        if (reader.GetContainerType() == TLV::kTLVType_NotSpecified)
        {
            size = 0;
            return CHIP_NO_ERROR;
        }

        return reader.CountRemainingInContainer(&size);
    }

    CHIP_ERROR DecodeInternal(TLV::TLVReader & mReader, TLV::TLVReader & reader)
    {
        VerifyOrReturnError(reader.GetType() == TLV::kTLVType_Array, CHIP_ERROR_SCHEMA_MISMATCH);
        TLV::TLVType type;
        ReturnErrorOnFailure(reader.EnterContainer(type));
        mReader.Init(reader);
        ReturnErrorOnFailure(reader.ExitContainer(type));
        return CHIP_NO_ERROR;
    }

protected:
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
            if (mReader.GetContainerType() == TLV::kTLVType_NotSpecified)
            {
                return false;
            }

            if (mStatus == CHIP_NO_ERROR)
            {
                mStatus = mReader.Next();
            }

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
struct DecodableHolder
{
    TLV::TLVReader mReader;

    inline void ClearReader() { mReader.Init(nullptr, 0); }

    inline void SetReader(const TLV::TLVReader & reader) { mReader.Init(reader); }
};

template <>
struct DecodableHolder<true>
{
    TLV::TLVReader mReader;
    Optional<FabricIndex> mFabricIndex;

    inline void ClearReader()
    {
        mReader.Init(nullptr, 0);
        mFabricIndex.ClearValue();
    }

    inline void SetReader(const TLV::TLVReader & reader)
    {
        mReader.Init(reader);
        mFabricIndex.ClearValue();
    }
};

template <bool IsFabricScoped>
class DecodableMaybeFabricScopedList : public DecodableListBase
{
public:
    static constexpr bool kIsFabricScoped = IsFabricScoped;

protected:
    class Iterator : public DecodableListBase::Iterator, protected FabricIndexIteratorMemberMixin<IsFabricScoped>
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
 * list contents; it can also be a wrapper for List<T> in the case where the list contents are available
 *
 * Typical use of a DecodableList looks like this:
 *
 *    CHIP_ERROR err = list.Iterate([&](auto & entry, bool &breakLoop) -> CHIP_ERROR {
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
    friend class Iterator;

public:
    using Super = detail::DecodableMaybeFabricScopedList<DataModel::IsFabricScoped<T>::value>;
    using Super::kIsFabricScoped;

    DecodableList() { ClearReader(); }

    template <class U, typename = std::enable_if_t<sizeof(U) == sizeof(T) && std::is_convertible<U *, T *>::value>>
    constexpr inline DecodableList(const Span<const U> & other) : readerOrSpan(other)
    {}

    template <class U, size_t N, typename = std::enable_if_t<sizeof(U) == sizeof(T) && std::is_convertible<U *, T *>::value>>
    constexpr inline DecodableList(const FixedSpan<const U, N> & other) : readerOrSpan(other)
    {}

    template <class U, size_t N, typename = std::enable_if_t<sizeof(U) == sizeof(T) && std::is_convertible<U *, T *>::value>>
    constexpr explicit DecodableList(U (&databuf)[N]) : readerOrSpan(databuf)
    {}

    DecodableList(typename List<const T>::pointer databuf, size_t datalen) : readerOrSpan(databuf, datalen) {}

    class Iterator : private Super::Iterator
    {
        using IteratorBase = typename Super::Iterator;

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
        inline Iterator(const DecodableList & list) : IteratorBase(list.reader(), list.readerOrSpan.mValue.mHolder.mFabricIndex)
        {}

        template <typename T0 = T, std::enable_if_t<!DataModel::IsFabricScoped<T0>::value, bool> = true>
        inline Iterator(const DecodableList & list) : IteratorBase(list.reader())
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

    constexpr DecodableList & operator=(const List<const T> & src)
    {
        readerOrSpan.mIsDecodable = false;
        readerOrSpan.mValue.mList = src;
        return *this;
    }

    template <size_t N>
    constexpr DecodableList & operator=(T (&databuf)[N])
    {
        readerOrSpan.mIsDecodable = false;
        readerOrSpan.mValue.mList = databuf;
        return (*this);
    }

    /** Use ComputeSize(size_t &) instead; backwards-compatibility. */
    inline CHIP_ERROR ComputeSize(size_t * size) const { return ComputeSize(*size); }

    /*
     * @brief
     *
     * Compute the size of the list. This can fail if the TLV is malformed. If
     * this succeeds, that does not guarantee that the individual items can be
     * successfully decoded; consumers should check Iterator::GetStatus() when
     * actually decoding them. If there is no list then the size is considered
     * to be zero.
     * @param size[out] the size of the list
     */
    inline CHIP_ERROR ComputeSize(size_t & size) const
    {
        if (readerOrSpan.mIsDecodable)
        {
            return Super::ComputeSize(size, reader());
        }
        else
        {
            size = readerOrSpan.mValue.mList.size();
            return CHIP_NO_ERROR;
        }
    }

    /*
     * @brief
     *
     * This call clears the TLV reader managed by this class, so it can be reused.
     */
    void ClearReader()
    {
        if (readerOrSpan.mIsDecodable)
        {
            readerOrSpan.mValue.mHolder.ClearReader();
        }
    }

    template <typename T0 = T, std::enable_if_t<DataModel::IsFabricScoped<T0>::value, bool> = true>
    void SetFabricIndex(FabricIndex fabricIndex)
    {
        if (readerOrSpan.mIsDecodable)
        {
            readerOrSpan.mValue.mHolder.mFabricIndex.SetValue(fabricIndex);
        }
    }

    /*
     * @brief
     *
     * This call stores a TLV reader positioned on the list this class is to manage.
     *
     * Specifically, the passed-in reader should be pointing into the list just after
     * having called `OpenContainer` on the list element.
     */
    void SetReader(const TLV::TLVReader & reader)
    {
        readerOrSpan.mIsDecodable = true;
        readerOrSpan.mValue.mHolder.SetReader(reader);
    }

    CHIP_ERROR Decode(TLV::TLVReader & reader)
    {
        readerOrSpan.mIsDecodable = true;
        return Super::DecodeInternal(this->reader(), reader);
    }

    // Iterates through all elements in the list, calling iterateFn on each element
    // The T element passed in to iterateFn has a guaranteed lifetime of the method call,
    // though if the implementation is a list, the element has a lifetiem beyond the call to iterateFn
    template <class F>
    __attribute__((always_inline)) inline CHIP_ERROR Iterate(F iterateFn) const
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

    inline typename List<const T>::pointer begin() const
    {
        VerifyOrDie(!readerOrSpan.mIsDecodable);
        return readerOrSpan.mValue.mList.begin();
    }

    inline typename List<const T>::pointer end() const
    {
        VerifyOrDie(!readerOrSpan.mIsDecodable);
        return readerOrSpan.mValue.mList.end();
    }

    inline typename List<const T>::reference operator[](size_t index) const
    {
        VerifyOrDie(!readerOrSpan.mIsDecodable);
        return readerOrSpan.mValue.mList[index];
    }

    inline size_t size() const
    {
        VerifyOrDie(!readerOrSpan.mIsDecodable);
        return readerOrSpan.mValue.mList.size();
    }

    inline void reduce_size(size_t new_size)
    {
        if (!readerOrSpan.mIsDecodable)
        {
            readerOrSpan.mValue.mList.reduce_size(new_size);
        }
    }

    template <typename T0 = T, std::enable_if_t<!DataModel::IsFabricScoped<T0>::value, bool> = true>
    inline CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
    {
        TLV::TLVType type;

        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, type));
        if (!readerOrSpan.mIsDecodable)
        {
            for (auto & item : readerOrSpan.mValue.mList)
            {
                ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), item));
            }
        }
        else
        {
            CHIP_ERROR result = Iterate([&](const auto & element, bool &) -> CHIP_ERROR {
                ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), element));
                return CHIP_NO_ERROR;
            });
            ReturnErrorOnFailure(result);
        }
        return writer.EndContainer(type);
    }

    template <typename T0 = T, std::enable_if_t<DataModel::IsFabricScoped<T0>::value, bool> = true>
    inline CHIP_ERROR EncodeForWrite(TLV::TLVWriter & writer, TLV::Tag tag) const
    {
        TLV::TLVType type;

        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, type));
        if (!readerOrSpan.mIsDecodable)
        {
            for (auto & item : readerOrSpan.mValue.mList)
            {
                ReturnErrorOnFailure(DataModel::EncodeForWrite(writer, TLV::AnonymousTag(), item));
            }
        }
        else
        {
            CHIP_ERROR result = Iterate([&](const auto & element, bool &) -> CHIP_ERROR {
                ReturnErrorOnFailure(DataModel::EncodeForWrite(writer, TLV::AnonymousTag(), element));
                return CHIP_NO_ERROR;
            });
            ReturnErrorOnFailure(result);
        }
        return writer.EndContainer(type);
    }

    template <typename T0 = T, std::enable_if_t<DataModel::IsFabricScoped<T0>::value, bool> = true>
    inline CHIP_ERROR EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex accessingFabricIndex) const
    {
        TLV::TLVType type;

        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, type));
        if (!readerOrSpan.mIsDecodable)
        {
            for (auto & item : readerOrSpan.mValue.mList)
            {
                ReturnErrorOnFailure(DataModel::EncodeForRead(writer, TLV::AnonymousTag(), accessingFabricIndex, item));
            }
        }
        else
        {
            CHIP_ERROR result = Iterate([&](const auto & element, bool &) -> CHIP_ERROR {
                ReturnErrorOnFailure(EncodeForRead(writer, TLV::AnonymousTag(), accessingFabricIndex, element));
                return CHIP_NO_ERROR;
            });
            ReturnErrorOnFailure(result);
        }
        return writer.EndContainer(type);
    }

private:
    inline const TLV::TLVReader & reader() const { return readerOrSpan.mValue.mHolder.mReader; }
    inline TLV::TLVReader & reader() { return readerOrSpan.mValue.mHolder.mReader; }

    // A container of bool + either a DataModel::List or a mReader for DecodableList
    class ValueHolder
    {
    public:
        inline ValueHolder() {}
        template <typename... Params>
        inline ValueHolder(Params &&... params) : mValue(std::forward<Params>(params)...)
        {}

        union Value
        {
            inline Value() {}
            template <typename... Params>
            inline Value(Params &&... params) : mList(std::forward<Params>(params)...)
            {}

            detail::DecodableHolder<DataModel::IsFabricScoped<T>::value> mHolder;
            DataModel::List<const T> mList;
        } mValue;
        bool mIsDecodable = false;
    };

    ValueHolder readerOrSpan;
};

} // namespace DataModel
} // namespace app
} // namespace chip
