/*
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

#include <tuple>

#include <lib/support/Pool.h>

namespace chip {

/// Provides an interface over a pool implementation which doesn't expose the size and the actual type of the pool.
template <typename U, typename... ConstructorArguments>
class PoolInterface
{
public:
    // For convenient use in PoolImpl
    using Interface = std::tuple<U, ConstructorArguments...>;

    virtual ~PoolInterface() {}

    virtual U * CreateObject(ConstructorArguments... args) = 0;
    virtual void ReleaseObject(U * element)                = 0;
    virtual void ReleaseAll()                              = 0;

    template <typename Function>
    Loop ForEachActiveObject(Function && function)
    {
        static_assert(std::is_same<Loop, decltype(function(std::declval<U *>()))>::value,
                      "The function must take T* and return Loop");
        auto proxy = [&](U * target) -> Loop { return function(target); };
        return ForEachActiveObjectInner(
            &proxy, [](void * context, U * target) -> Loop { return (*static_cast<decltype(proxy) *>(context))(target); });
    }
    template <typename Function>
    Loop ForEachActiveObject(Function && function) const
    {
        static_assert(std::is_same<Loop, decltype(function(std::declval<U *>()))>::value,
                      "The function must take const T* and return Loop");
        auto proxy = [&](const U * target) -> Loop { return function(target); };
        return ForEachActiveObjectInner(
            &proxy, [](void * context, const U * target) -> Loop { return (*static_cast<decltype(proxy) *>(context))(target); });
    }

protected:
    using Lambda                                                                    = Loop (*)(void *, U *);
    using LambdaConst                                                               = Loop (*)(void *, const U *);
    virtual Loop ForEachActiveObjectInner(void * context, Lambda lambda)            = 0;
    virtual Loop ForEachActiveObjectInner(void * context, LambdaConst lambda) const = 0;
};

template <class T, size_t N, ObjectPoolMem M, typename Interface>
class PoolProxy;

template <class T, size_t N, ObjectPoolMem M, typename U, typename... ConstructorArguments>
class PoolProxy<T, N, M, std::tuple<U, ConstructorArguments...>> : public PoolInterface<U, ConstructorArguments...>
{
public:
    static_assert(std::is_base_of<U, T>::value, "Interface type is not derived from Pool type");

    PoolProxy() {}
    ~PoolProxy() override {}

    U * CreateObject(ConstructorArguments... args) override { return Impl().CreateObject(std::move(args)...); }

    void ReleaseObject(U * element) override { Impl().ReleaseObject(static_cast<T *>(element)); }

    void ReleaseAll() override { Impl().ReleaseAll(); }

protected:
    Loop ForEachActiveObjectInner(void * context, typename PoolInterface<U, ConstructorArguments...>::Lambda lambda) override
    {
        return Impl().ForEachActiveObject([&](T * target) { return lambda(context, static_cast<U *>(target)); });
    }
    Loop ForEachActiveObjectInner(void * context,
                                  typename PoolInterface<U, ConstructorArguments...>::LambdaConst lambda) const override
    {
        return Impl().ForEachActiveObject([&](const T * target) { return lambda(context, static_cast<const U *>(target)); });
    }

    virtual ObjectPool<T, N, M> & Impl()             = 0;
    virtual const ObjectPool<T, N, M> & Impl() const = 0;
};

/*
 * @brief
 *   Define a implementation of a pool which derive and expose PoolInterface's.
 *
 *  @tparam T          a subclass of element to be allocated.
 *  @tparam N          a positive integer max number of elements the pool provides.
 *  @tparam M          an ObjectPoolMem constant selecting static vs heap allocation.
 *  @tparam Interfaces a list of parameters which defines PoolInterface's. each interface is defined by a
 *                     std::tuple<U, ConstructorArguments...>. The PoolImpl is derived from every
 *                     PoolInterface<U, ConstructorArguments...>, the PoolImpl can be converted to the interface type
 *                     and passed around
 */
template <class T, size_t N, ObjectPoolMem M, typename... Interfaces>
class PoolImpl : public PoolProxy<T, N, M, Interfaces>...
{
public:
    PoolImpl() {}
    ~PoolImpl() override {}

protected:
    ObjectPool<T, N, M> & Impl() override { return mImpl; }
    const ObjectPool<T, N, M> & Impl() const override { return mImpl; }

private:
    ObjectPool<T, N, M> mImpl;
};

} // namespace chip
