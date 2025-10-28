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

#include <lib/support/CompileTimeString.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/Span.h>
#include <lib/support/TypeTraits.h>

#include <memory>
#include <optional>

extern "C" {
#include <libubox/blobmsg.h>
}

namespace chip {
namespace ubus {

namespace detail {

// Exposes type id and blobmsg_{get,add}_* functions for a type
template <typename T>
struct BlobMsgType
{
    static_assert(!TemplatedTrueType<T>(), "Type is not supported by blob_msg");
    static constexpr enum blobmsg_type id = BLOBMSG_TYPE_UNSPEC;
    static int get(blob_attr * attr, T & value) { return -1; }
    static int add(blob_buf * buf, const char * name, T value) { return -1; }
};

// Specializations for types natively supported by blobmsg_get_* and blobmsg_add_*
// clang-format off
#define _CHIP_BLOBMSG_NATIVE_TYPE(_T, _id, _get, _add)                                                                             \
    template <> struct BlobMsgType<_T>                                                                                             \
    {                                                                                                                              \
        static constexpr enum blobmsg_type id = _id;                                                                               \
        static int get(blob_attr * attr, _T & value) { value = _get(attr); return 0; }                                             \
        static int add(blob_buf * buf, const char * name, _T value) { return _add(buf, name, value); }                             \
    }
_CHIP_BLOBMSG_NATIVE_TYPE(const char *, BLOBMSG_TYPE_STRING, blobmsg_get_string, blobmsg_add_string);
_CHIP_BLOBMSG_NATIVE_TYPE(bool,         BLOBMSG_TYPE_BOOL,   blobmsg_get_u8,     blobmsg_add_u8);
_CHIP_BLOBMSG_NATIVE_TYPE(double,       BLOBMSG_TYPE_DOUBLE, blobmsg_get_double, blobmsg_add_double);
_CHIP_BLOBMSG_NATIVE_TYPE(uint8_t,      BLOBMSG_TYPE_INT8,   blobmsg_get_u8,     blobmsg_add_u8);
_CHIP_BLOBMSG_NATIVE_TYPE(int8_t,       BLOBMSG_TYPE_INT8,   blobmsg_get_u8,     blobmsg_add_u8);
_CHIP_BLOBMSG_NATIVE_TYPE(uint16_t,     BLOBMSG_TYPE_INT16,  blobmsg_get_u16,    blobmsg_add_u16);
_CHIP_BLOBMSG_NATIVE_TYPE(int16_t,      BLOBMSG_TYPE_INT16,  blobmsg_get_u16,    blobmsg_add_u16);
_CHIP_BLOBMSG_NATIVE_TYPE(uint32_t,     BLOBMSG_TYPE_INT32,  blobmsg_get_u32,    blobmsg_add_u32);
_CHIP_BLOBMSG_NATIVE_TYPE(int32_t,      BLOBMSG_TYPE_INT32,  blobmsg_get_u32,    blobmsg_add_u32);
_CHIP_BLOBMSG_NATIVE_TYPE(uint64_t,     BLOBMSG_TYPE_INT64,  blobmsg_get_u64,    blobmsg_add_u64);
_CHIP_BLOBMSG_NATIVE_TYPE(int64_t,      BLOBMSG_TYPE_INT64,  blobmsg_get_u64,    blobmsg_add_u64);
#undef _CHIP_BLOBMSG_NATIVE_TYPE
// clang-format on

// Specialization for mapping ByteSpan to a hex-encoded BLOBMSG_TYPE_STRING
// Note that decoding is performed in-place, i.e. is destructive.
template <>
struct BlobMsgType<ByteSpan>
{
    static constexpr enum blobmsg_type id = BLOBMSG_TYPE_STRING;
    static int get(blob_attr * attr, ByteSpan & value);
    static int add(blob_buf * buf, const char * name, ByteSpan const & value);
};

// Non-optional value wrapper - similar to std::optional but always contains a value
template <typename T>
struct NonOptional
{
    T & value() { return mValue; }
    const T & value() const { return mValue; }

    operator T &() { return mValue; }
    operator const T &() const { return mValue; }

    NonOptional & operator=(const T & val)
    {
        mValue = val;
        return *this;
    }

    void reset() { mValue = T{}; }

private:
    T mValue{};
};

// Name-independent parts of BlobMsgField
template <typename T, bool Required>
struct BlobMsgFieldBase : public std::conditional_t<Required, NonOptional<T>, std::optional<T>>
{
    using Base = std::conditional_t<Required, NonOptional<T>, std::optional<T>>;
    using Impl = BlobMsgType<T>;
    using Base::Base;
    using Base::operator=;

    using ValueType = T;
    static constexpr bool required() { return Required; }

    bool Decode(blob_attr * attr)
    {
        if (!attr)
        {
            if constexpr (required())
            {
                return false;
            }
            else
            {
                this->reset();
                return true;
            }
        }

        T value;
        VerifyOrReturnValue(Impl::get(attr, value) == 0, false);
        Base::operator=(value);
        return true;
    }
};

} // namespace detail

// A field that can be read from or written in blob_msg format.
// The field type captures the field name as a compile-time string,
// so a matching blobmsg_policy can be generated automatically.
//
// By default fields are optional, i.e. derive from std::optional.
// Passing Required=true (or using BlobMsgRequiredField) creates makes
// the field non-optional. BlobMsgParse() will fail is any required
// fields are missing.
template <typename T, typename NameCTST, bool Required = false>
struct BlobMsgField final : public detail::BlobMsgFieldBase<T, Required>
{
    using Base = detail::BlobMsgFieldBase<T, Required>;
    using Base::Base;
    using Base::operator=;
    using typename Base::Impl;

    static constexpr const char * name() { return NameCTST::c_str(); }
    static constexpr blobmsg_policy policy() { return { .name = name(), .type = Impl::id }; }
};

// Alias for a required BlobMsgField
template <typename T, typename NameCTST>
using BlobMsgRequiredField = BlobMsgField<T, NameCTST, true>;

// Parses the specified fields using blobmsg_parse() and returns true on success.
// Fails (i.e. returns false) if blobmsg_parse() fails, any required field is not
// present, or decoding of any field fails. Note that fields that are present with
// the wrong type are ignored by blobmsg_parse() and not treated as an error.
//
// Note: This function accepts both plain and extended blob_attrs, i.e. works with
// both message payloads and nested tables.
template <typename... BlobMsgFields>
bool BlobMsgParse(blob_attr * attr, BlobMsgFields &... fields)
{
    static constexpr blobmsg_policy policy[] = { BlobMsgFields::policy()... };
    blob_attr * values[sizeof...(BlobMsgFields)];
    VerifyOrReturnValue(!blobmsg_parse_attr(policy, sizeof...(BlobMsgFields), values, attr), false);
    return [&]<size_t... Is>(std::index_sequence<Is...>) {
        return (fields.Decode(values[Is]) && ...);
    }(std::index_sequence_for<BlobMsgFields...>{});
}

// A RAII helper for automatically closing nested tables / arrays with BlobMsgBuf.
class BlobMsgCookie
{
public:
    BlobMsgCookie(blob_buf * buf, void * cookie) : mCookie(cookie, Deleter{ buf }) {}
    /* implicit */ operator bool() const { return mCookie.get() != nullptr; }

private:
    struct Deleter
    {
        blob_buf * mBuf;
        void operator()(void * cookie) { blob_nest_end(mBuf, cookie); }
    };

    std::unique_ptr<void, Deleter> mCookie;
};

// A thin wrapper around blob_buf with automatic resource management
struct BlobMsgBuf final : public blob_buf
{
    BlobMsgBuf() : blob_buf{} { Clear(); }
    ~BlobMsgBuf() { blob_buf_free(this); }

    // Returns true if the buffer has encounter an allocation error.
    // Once in an error state any subsequent add operations will have no effect.
    // Individual operations return true on success.
    bool HasError() const { return this->head == nullptr; }

    // Clears the buffer and resets the error state.
    bool Clear() { return Check(blob_buf_init(this, 0)); }

    ///// Encoding methods with explicit field names

    template <typename T>
    bool Add(const char * name, T value)
    {
        return !HasError() && Check(detail::BlobMsgType<T>::add(this, name, value));
    }

    bool AddFormat(const char * name, const char * format, ...) ENFORCE_FORMAT(3, 4);
    BlobMsgCookie AddArray(const char * name) { return AddNested(name, /* array = */ true); }
    BlobMsgCookie AddTable(const char * name) { return AddNested(name, /* array = */ false); }

    ///// Encoding methods based on BlobMsgField types or instances

    // Adds a value, with the name given by a BlogMsgField type, which must be an explicit template argument.
    // Example: buf.Add<ErrorField>(0);
    template <typename BlobMsgField> // explicit
    bool Add(typename BlobMsgField::ValueType value)
    {
        return Add(BlobMsgField::name(), value);
    }

    // Adds a BlogMsgField instance. If the field is optional and absent, nothing is encoded.
    template <typename BlobMsgField, typename = typename BlobMsgField::ValueType>
    bool Add(BlobMsgField const & field)
    {
        if constexpr (field.required())
        {
            return Add(field.name(), field.value());
        }
        else
        {
            return AddOptional(field.name(), field);
        }
    }

private:
    BlobMsgCookie AddNested(const char * name, bool array)
    {
        return BlobMsgCookie(this, !HasError() ? blobmsg_open_nested(this, name, array) : nullptr);
    }

    template <typename T>
    bool AddOptional(const char * name, std::optional<T> const & optional)
    {
        return !HasError() && (!optional.has_value() || Check(detail::BlobMsgType<T>::add(this, name, optional.value())));
    }

    bool Check(int status)
    {
        if (status != 0)
        {
            this->head = nullptr;
            return false;
        }
        return true;
    }
};

} // namespace ubus
} // namespace chip
