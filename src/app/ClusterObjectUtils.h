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

#include <lib/core/CHIPSafeCasts.h>
#include <iterator>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace ClusterObjectUtils {

/*
 * @brief
 * Set of overloaded encode methods that based on the type of cluster element passed in,
 * appropriately encodes them to TLV.
 */
template <typename X, typename std::enable_if_t<std::is_integral<X>::value, int> = 0>
CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, X x)
{
    return writer.Put(tag, x);
}

template <typename X, typename std::enable_if_t<std::is_enum<X>::value, int> = 0>
CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, X x)
{
    return writer.Put(tag, x);
}

CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, ByteSpan x)
{
    return writer.Put(tag, x);
}

CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, const Span<const char> x)
{
    return writer.PutString(tag, x.data(), static_cast<uint32_t>(x.size()));
}

template <typename X>
CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, const Span<X> & x)
{
    TLV::TLVType type;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, type));
    for (auto & item : x)
    {
        ReturnErrorOnFailure(Encode(writer, TLV::AnonymousTag, item));
    }
    ReturnErrorOnFailure(writer.EndContainer(type));

    return CHIP_NO_ERROR;
}

/*
 * @brief
 *
 * This specific variant that encodes cluster objects (like structs, commands, events) to TLV
 * depends on the presence of an Encode method on the object to present. The signature of that method
 * is as follows:
 *
 * CHIP_ERROR <Object>::Encode(TLVWriter &writer, uint64_t tag);
 *
 *
 */
template <typename X, typename std::enable_if_t<std::is_class<X>::value &&
                      std::is_same<decltype(&X::Encode), CHIP_ERROR (X::*)(TLV::TLVWriter &, uint64_t)>::value, X> * = nullptr>
CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, X & x)
{
    return x.Encode(writer, tag);
}

//
// Decode
//
template <typename X, typename std::enable_if_t<std::is_integral<X>::value, int> = 0>
CHIP_ERROR Decode(TLV::TLVReader & reader, X & x)
{
    return reader.Get(x);
}

//
// @brief
//
// Decodes an octet string that is expected at the positioned reader.
//
// The passed in ByteSpan is ignored and updated to point directly into
// the buffer backing the reader.
//
CHIP_ERROR Decode(TLV::TLVReader & reader, ByteSpan & x)
{
    VerifyOrReturnError(reader.GetType() == TLV::kTLVType_ByteString, CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
    return reader.Get(x);
}

//
// @brief
//
// Decodes a UTF-8 string that is expected at the positioned reader.
//
// The passed in char Span is ignored and updated to point directly into
// the buffer backing the reader.
//
CHIP_ERROR Decode(TLV::TLVReader & reader, Span<const char> & x)
{
    ByteSpan bs;

    VerifyOrReturnError(reader.GetType() == TLV::kTLVType_UTF8String, CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
    ReturnErrorOnFailure(reader.Get(bs));
    x = Span<const char>(Uint8::to_const_char(bs.data()), (size_t) bs.size());
    return CHIP_NO_ERROR;
}

/*
 * @brief
 *
 * This specific variant that decodes cluster objects (like structs, commands, events) from TLV
 * depends on the presence of a Decode method on the object to present. The signature of that method
 * is as follows:
 *
 * CHIP_ERROR <Object>::Decode(TLVReader &reader);
 *
 */
template <typename X, typename std::enable_if_t<std::is_class<X>::value, int> = 0>
CHIP_ERROR Decode(TLV::TLVReader & reader, X & x)
{
    return x.Decode(reader);
}

/*
 * @brief
 *
 * This class provides an iteratable decoder of list items within TLV payloads
 * such that no memory has to be provided ahead of time to store the entirety of the decoded
 * list contents.
 *
 * Specifically, this provides a read-only input iterator that overloads the '->' operator to dynamically
 * decode the list item at a given position in the stored TLVReader and return it to the caller.
 *
 */
template <typename T>
class IteratableList
{
public:
    /*
     * @brief
     *
     * This calls stores a TLV reader positioned on the list this class is to manage.
     *
     * Specifically, the passed-in TLV reader should be pointing into the list just after
     * having called `OpenContainer` on the list element.
     */
    void SetReader(const TLV::TLVReader & reader) { mReader = reader; }

    size_t size()
    {
        CHIP_ERROR err;
        TLV::TLVReader reader;
        reader.Init(mReader);
        size_t count = 0;

        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            count++;
        }

        return count;
    }

    class Iterator
    {
    public:
        /*
         * @brief
         *
         * Since neither the iterator constructor nor the operator overload permit sending errors back,
         * encapsulate a tuple of the value and error that is then returned to the caller when decoding
         */
        struct ValuePair
        {
            bool HasError() { return (status != CHIP_NO_ERROR); }
            CHIP_ERROR status;
            T value;
        };

        using iterator_category = std::input_iterator_tag;
        using value_type        = ValuePair;
        using pointer           = ValuePair *;
        using reference         = ValuePair &;

        /*
         * @brief
         *
         * Constructor
         *
         * reader       Pointer to a TLV reader whoose contents are copied into an internal
         *              reader instance that is then used to track the iterator position
         *              within the backing buffer.
         *
         * signature    A unique signature that associated it with a given IteratableList instance
         *              This permits actually comparing two iterators trivially to ensure they're
         *              both logically iterating over the same list.
         *
         * isEnd        A flag that indicates if the iterator is pointing at the end of the TLV list.
         *              This is preferable to actually iterating over the list till the end.
         */
        Iterator(TLV::TLVReader * reader, uintptr_t signature, bool isEnd = false)
        {
            mSignature    = signature;
            mIsEnd        = isEnd;
            mValue.status = CHIP_NO_ERROR;
            mDecoded      = false;

            if (!isEnd && reader)
            {
                mReader.Init(*reader);
                mValue.status = mReader.Next();
            }
        }

        friend bool operator==(const Iterator & a, const Iterator & b)
        {
            return ((a.mSignature == b.mSignature) && ((a.mIsEnd == b.mIsEnd)));
        }

        friend bool operator!=(const Iterator & a, const Iterator & b)
        {
            return !((a.mSignature == b.mSignature) && ((a.mIsEnd == b.mIsEnd)));
        }

        /*
         * @brief
         *
         * De-reference operator overload that returns a fully decoded list item
         * at the current iterator position.
         *
         * Before accessing the value within the pair, the status should *always* be
         * checked first.
         *
         * This can be called multiple times safely at the same iterator position.
         *
         */
        ValuePair * operator->()
        {
            if (mValue.status == CHIP_NO_ERROR && !mDecoded)
            {
                mValue.status = Decode(mReader, mValue.value);
                mDecoded      = true;
            }

            return &mValue;
        }

        /*
         * @brief
         *
         * Increments the iterator one item forward in the current list.
         *
         * The caller should check the status of the operation by using the
         * -> operator above to accesss the status within the ValuePair.
         *
         */
        Iterator & operator++()
        {
            if (mValue.status != CHIP_NO_ERROR)
            {
                return *this;
            }

            mValue.status = mReader.Next();

            if (mValue.status != CHIP_NO_ERROR)
            {
                if (mValue.status == CHIP_END_OF_TLV)
                {
                    mIsEnd        = true;
                    mValue.status = CHIP_NO_ERROR;
                }
            }
            else if (mValue.status == CHIP_NO_ERROR)
            {
                //
                // clear our tracker variable so that the value at this new position
                // can be decoded in a subsequence call to the -> operator overload above.
                //
                mDecoded = false;
            }

            return *this;
        }

    private:
        ValuePair mValue;
        bool mDecoded;
        uintptr_t mSignature;
        TLV::TLVReader mReader;
        bool mIsEnd;
    };

    TLV::TLVReader & GetReader() { return mReader; }

    Iterator begin() { return Iterator(&mReader, (uintptr_t) this, false); }
    Iterator end() { return Iterator(NULL, (uintptr_t) this, true); }

private:
    TLV::TLVReader mReader;
};

template <typename X>
CHIP_ERROR Decode(TLV::TLVReader & reader, IteratableList<X> & x)
{
    TLV::TLVType type;

    ReturnErrorOnFailure(reader.EnterContainer(type));
    x.SetReader(reader);
    ReturnErrorOnFailure(reader.ExitContainer(type));

    return CHIP_NO_ERROR;
}

} // namespace ClusterObjectUtils
} // namespace app
} // namespace chip
