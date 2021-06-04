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

#include <core/CHIPCore.h>

#include <algorithm>
#include <cassert>
#include <new>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace chip {

namespace Internal {

template <typename... Ts>
struct VariantCurry;

template <typename T, typename... Ts>
struct VariantCurry<T, Ts...>
{
    inline static void Destroy(std::size_t id, void * data)
    {
        if (id == T::VariantId)
            reinterpret_cast<T *>(data)->~T();
        else
            VariantCurry<Ts...>::Destroy(id, data);
    }

    inline static void Move(std::size_t that_t, void * that_v, void * this_v)
    {
        if (that_t == T::VariantId)
            new (this_v) T(std::move(*reinterpret_cast<T *>(that_v)));
        else
            VariantCurry<Ts...>::Move(that_t, that_v, this_v);
    }

    inline static void Copy(std::size_t that_t, const void * that_v, void * this_v)
    {
        if (that_t == T::VariantId)
            new (this_v) T(*reinterpret_cast<const T *>(that_v));
        else
            VariantCurry<Ts...>::Copy(that_t, that_v, this_v);
    }
};

template <>
struct VariantCurry<>
{
    inline static void Destroy(std::size_t id, void * data) {}
    inline static void Move(std::size_t that_t, void * that_v, void * this_v) {}
    inline static void Copy(std::size_t that_t, const void * that_v, void * this_v) {}
};

} // namespace Internal

/**
 * @brief
 *   Represents a type-safe union. An instance of Variant at any given time either holds a value of one of its
 *   alternative types, or no value. Each type must define a unique value of a static field named VariantId.
 *
 *   Example:
 *     struct Type1
 *     {
 *         static constexpr const std::size_t VariantId = 1;
 *     };
 *
 *     struct Type2
 *     {
 *         static constexpr const std::size_t VariantId = 2;
 *     };
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
    using Curry = Internal::VariantCurry<Ts...>;

    std::size_t mTypeId;
    Data mData;

public:
    Variant() : mTypeId(kInvalidType) {}

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

    template <typename T>
    bool Is() const
    {
        return (mTypeId == T::VariantId);
    }

    std::size_t GetType() const { return mTypeId; }

    bool Valid() const { return (mTypeId != kInvalidType); }

    template <typename T, typename... Args>
    void Set(Args &&... args)
    {
        Curry::Destroy(mTypeId, &mData);
        new (&mData) T(std::forward<Args>(args)...);
        mTypeId = T::VariantId;
    }

    template <typename T>
    T & Get()
    {
        VerifyOrDie(mTypeId == T::VariantId);
        return *reinterpret_cast<T *>(&mData);
    }

    template <typename T>
    const T & Get() const
    {
        VerifyOrDie(mTypeId == T::VariantId);
        return *reinterpret_cast<const T *>(&mData);
    }

    ~Variant() { Curry::Destroy(mTypeId, &mData); }
};

} // namespace chip
