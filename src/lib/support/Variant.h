/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/core/CHIPCore.h>

#include <algorithm>
#include <new>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include <lib/core/InPlace.h>

namespace chip {

namespace VariantInternal {

template <std::size_t Index, typename... Ts>
struct VariantCurry;

template <std::size_t Index, typename T, typename... Ts>
struct VariantCurry<Index, T, Ts...>
{
    inline static void Destroy(std::size_t id, void * data)
    {
        if (id == Index)
            reinterpret_cast<T *>(data)->~T();
        else
            VariantCurry<Index + 1, Ts...>::Destroy(id, data);
    }

    inline static void Move(std::size_t that_t, void * that_v, void * this_v)
    {
        if (that_t == Index)
            new (this_v) T(std::move(*reinterpret_cast<T *>(that_v)));
        else
            VariantCurry<Index + 1, Ts...>::Move(that_t, that_v, this_v);
    }

    inline static void Copy(std::size_t that_t, const void * that_v, void * this_v)
    {
        if (that_t == Index)
        {
            new (this_v) T(*reinterpret_cast<const T *>(that_v));
        }
        else
        {
            VariantCurry<Index + 1, Ts...>::Copy(that_t, that_v, this_v);
        }
    }

    inline static bool Equal(std::size_t type_t, const void * that_v, const void * this_v)
    {
        if (type_t == Index)
        {
            return *reinterpret_cast<const T *>(this_v) == *reinterpret_cast<const T *>(that_v);
        }

        return VariantCurry<Index + 1, Ts...>::Equal(type_t, that_v, this_v);
    }
};

template <std::size_t Index>
struct VariantCurry<Index>
{
    inline static void Destroy(std::size_t id, void * data) {}
    inline static void Move(std::size_t that_t, void * that_v, void * this_v) {}
    inline static void Copy(std::size_t that_t, const void * that_v, void * this_v) {}
    inline static bool Equal(std::size_t type_t, const void * that_v, const void * this_v)
    {
        VerifyOrDie(false);
        return false;
    }
};

template <typename T, typename TupleType>
class TupleIndexOfType
{
private:
    template <std::size_t Index>
    static constexpr
        typename std::enable_if<std::is_same<T, typename std::tuple_element<Index, TupleType>::type>::value, std::size_t>::type
        calculate()
    {
        return Index;
    }

    template <std::size_t Index>
    static constexpr
        typename std::enable_if<!std::is_same<T, typename std::tuple_element<Index, TupleType>::type>::value, std::size_t>::type
        calculate()
    {
        return calculate<Index + 1>();
    }

public:
    static constexpr std::size_t value = calculate<0>();
};

} // namespace VariantInternal

/**
 * @brief
 *   Represents a type-safe union. An instance of Variant at any given time either holds a value of one of its
 *   alternative types, or no value.
 *
 *   Example:
 *     struct Type1 {};
 *
 *     struct Type2 {};
 *
 *     Variant<Type1, Type2> v;
 *     v.Set<Type1>(); // v contains Type1
 *     Type1 o1 = v.Get<Type1>();
 */
template <typename... Ts>
struct Variant
{
private:
    static constexpr std::size_t kDataSize    = std::max({ sizeof(Ts)... });
    static constexpr std::size_t kDataAlign   = std::max({ alignof(Ts)... });
    static constexpr std::size_t kInvalidType = SIZE_MAX;

    using Data  = typename std::aligned_storage<kDataSize, kDataAlign>::type;
    using Curry = VariantInternal::VariantCurry<0, Ts...>;

    std::size_t mTypeId;
    Data mData;

public:
    Variant() : mTypeId(kInvalidType) {}

    template <typename T, class... Args>
    constexpr explicit Variant(InPlaceTemplateType<T>, Args &&... args) :
        mTypeId(VariantInternal::TupleIndexOfType<T, std::tuple<Ts...>>::value)
    {
        new (&mData) T(std::forward<Args>(args)...);
    }

    Variant(const Variant<Ts...> & that) : mTypeId(that.mTypeId) { Curry::Copy(that.mTypeId, &that.mData, &mData); }

    Variant(Variant<Ts...> && that) : mTypeId(that.mTypeId)
    {
        Curry::Move(that.mTypeId, &that.mData, &mData);
        Curry::Destroy(that.mTypeId, &that.mData);
        that.mTypeId = kInvalidType;
    }

    Variant<Ts...> & operator=(const Variant<Ts...> & that)
    {
        Curry::Destroy(mTypeId, &mData);
        mTypeId = that.mTypeId;
        Curry::Copy(that.mTypeId, &that.mData, &mData);
        return *this;
    }

    Variant<Ts...> & operator=(Variant<Ts...> && that)
    {
        Curry::Destroy(mTypeId, &mData);
        mTypeId = that.mTypeId;
        Curry::Move(that.mTypeId, &that.mData, &mData);
        Curry::Destroy(that.mTypeId, &that.mData);
        that.mTypeId = kInvalidType;
        return *this;
    }

    bool operator==(const Variant & other) const
    {
        return GetType() == other.GetType() && (!Valid() || Curry::Equal(mTypeId, &other.mData, &mData));
    }

    template <typename T>
    bool Is() const
    {
        return (mTypeId == VariantInternal::TupleIndexOfType<T, std::tuple<Ts...>>::value);
    }

    std::size_t GetType() const { return mTypeId; }

    bool Valid() const { return (mTypeId != kInvalidType); }

    template <typename T, typename... Args>
    static Variant<Ts...> Create(Args &&... args)
    {
        return Variant<Ts...>(InPlaceTemplate<T>, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    void Set(Args &&... args)
    {
        Curry::Destroy(mTypeId, &mData);
        new (&mData) T(std::forward<Args>(args)...);
        mTypeId = VariantInternal::TupleIndexOfType<T, std::tuple<Ts...>>::value;
    }

    template <typename T>
    T & Get()
    {
        VerifyOrDie((mTypeId == VariantInternal::TupleIndexOfType<T, std::tuple<Ts...>>::value));
        return *reinterpret_cast<T *>(&mData);
    }

    template <typename T>
    const T & Get() const
    {
        VerifyOrDie((mTypeId == VariantInternal::TupleIndexOfType<T, std::tuple<Ts...>>::value));
        return *reinterpret_cast<const T *>(&mData);
    }

    // Ideally we would suppress this from within Optional.h, where this false positive is coming from. That said suppressing
    // here is okay since mTypeId would seemingly only be uninitialized when Variant is in a union.
    //
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage): Only in a false positive is mTypeId uninitialized.
    ~Variant() { Curry::Destroy(mTypeId, &mData); }
};

} // namespace chip
