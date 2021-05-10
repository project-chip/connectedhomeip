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

#include <type_traits>
#include <typeinfo>
#include <utility>

namespace chip {

namespace { // Anonymous namespace to hide VariantHelper

template <typename... Ts>
struct VariantHelper;

template <typename T, typename... Ts>
struct VariantHelper<T, Ts...>
{
    inline static void Destroy(std::size_t id, void * mData)
    {
        if (id == T::VariantId)
            reinterpret_cast<T *>(mData)->~T();
        else
            VariantHelper<Ts...>::Destroy(id, mData);
    }

    inline static void Move(std::size_t that_t, void * that_v, void * this_v)
    {
        if (that_t == T::VariantId)
            new (this_v) T(std::move(*reinterpret_cast<T *>(that_v)));
        else
            VariantHelper<Ts...>::Move(that_t, that_v, this_v);
    }

    inline static void Copy(std::size_t that_t, const void * that_v, void * this_v)
    {
        if (that_t == T::VariantId)
            new (this_v) T(*reinterpret_cast<const T *>(that_v));
        else
            VariantHelper<Ts...>::Copy(that_t, that_v, this_v);
    }
};

template <>
struct VariantHelper<>
{
    inline static void Destroy(std::size_t id, void * mData) {}
    inline static void Move(std::size_t that_t, void * that_v, void * this_v) {}
    inline static void Copy(std::size_t that_t, const void * that_v, void * this_v) {}
};

} // namespace

template <typename... Ts>
struct Variant
{
private:
    static constexpr std::size_t kDataSize    = std::max(sizeof(Ts)...);
    static constexpr std::size_t kDataAlign   = std::max(alignof(Ts)...);
    static constexpr std::size_t kInvalidType = SIZE_MAX;

    using Data   = typename std::aligned_storage<kDataSize, kDataAlign>::type;
    using Helper = VariantHelper<Ts...>;

    std::size_t mTypeId;
    Data mData;

public:
    Variant() : mTypeId(kInvalidType) {}

    Variant(const Variant<Ts...> & that) : mTypeId(that.mTypeId) { Helper::Copy(that.mTypeId, &that.mData, &mData); }

    Variant(Variant<Ts...> && that) : mTypeId(that.mTypeId) { Helper::Move(that.mTypeId, &that.mData, &mData); }

    // Serves as both the move and the copy asignment operator.
    Variant<Ts...> & operator=(Variant<Ts...> that)
    {
        std::swap(mTypeId, that.mTypeId);
        std::swap(mData, that.mData);

        return *this;
    }

    template <typename T>
    bool Is()
    {
        return (mTypeId == T::VariantId);
    }

    bool Valid() { return (mTypeId != kInvalidType); }

    template <typename T, typename... Args>
    void Set(Args &&... args)
    {
        Helper::Destroy(mTypeId, &mData);
        new (&mData) T(std::forward<Args>(args)...);
        mTypeId = T::VariantId;
    }

    template <typename T>
    T & Get()
    {
        // It is a dynamic_cast-like behaviour
        if (mTypeId == T::VariantId)
            return *reinterpret_cast<T *>(&mData);
        else
            assert(false);
    }

    template <typename T>
    const T & Get() const
    {
        // It is a dynamic_cast-like behaviour
        if (mTypeId == T::VariantId)
            return *reinterpret_cast<const T *>(&mData);
        else
            assert(false);
    }

    ~Variant() { Helper::Destroy(mTypeId, &mData); }
};

} // namespace chip
