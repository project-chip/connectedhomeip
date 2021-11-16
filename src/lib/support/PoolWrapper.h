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

    virtual U * CreateObject(ConstructorArguments &&... args)              = 0;
    virtual void ReleaseObject(U * element)                                = 0;
    virtual void ResetObject(U * element, ConstructorArguments &&... args) = 0;

    template <typename Function>
    bool ForEachActiveObject(Function && function)
    {
        auto proxy = [&](U * target) -> bool { return function(target); };
        return ForEachActiveObjectInner(
            &proxy, [](void * context, U * target) -> bool { return (*static_cast<decltype(proxy) *>(context))(target); });
    }

protected:
    using Lambda                                                         = bool (*)(void *, U *);
    virtual bool ForEachActiveObjectInner(void * context, Lambda lambda) = 0;
};

template <class T, size_t N, typename Interface>
class PoolProxy;

template <class T, size_t N, typename U, typename... ConstructorArguments>
class PoolProxy<T, N, std::tuple<U, ConstructorArguments...>> : public PoolInterface<U, ConstructorArguments...>
{
public:
    static_assert(std::is_base_of<U, T>::value, "Interface type is not derived from Pool type");

    PoolProxy() {}
    virtual ~PoolProxy() override {}

    virtual U * CreateObject(ConstructorArguments &&... args) override
    {
        return Impl().CreateObject(std::forward<ConstructorArguments>(args)...);
    }

    virtual void ReleaseObject(U * element) override { Impl().ReleaseObject(static_cast<T *>(element)); }

    virtual void ResetObject(U * element, ConstructorArguments &&... args) override
    {
        return Impl().ResetObject(static_cast<T *>(element), std::forward<ConstructorArguments>(args)...);
    }

protected:
    virtual bool ForEachActiveObjectInner(void * context,
                                          typename PoolInterface<U, ConstructorArguments...>::Lambda lambda) override
    {
        return Impl().ForEachActiveObject([&](T * target) { return lambda(context, static_cast<U *>(target)); });
    }

    virtual BitMapObjectPool<T, N> & Impl() = 0;
};

/*
 * @brief
 *   Define a implementation of a pool which derive and expose PoolInterface's.
 *
 *  @tparam T          a subclass of element to be allocated.
 *  @tparam N          a positive integer max number of elements the pool provides.
 *  @tparam Interfaces a list of parameters which defines PoolInterface's. each interface is defined by a
 *                     std::tuple<U, ConstructorArguments...>. The PoolImpl is derived from every
 *                     PoolInterface<U, ConstructorArguments...>, the PoolImpl can be converted to the interface type
 *                     and passed around
 */
template <class T, size_t N, typename... Interfaces>
class PoolImpl : public PoolProxy<T, N, Interfaces>...
{
public:
    PoolImpl() {}
    virtual ~PoolImpl() override {}

protected:
    virtual BitMapObjectPool<T, N> & Impl() override { return mImpl; }

private:
    BitMapObjectPool<T, N> mImpl;
};

} // namespace chip
