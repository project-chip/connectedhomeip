/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

#include <type_traits>

namespace chip {

template <template <typename> class Cond, typename Default, typename... Ts>
struct first_if_any_or_default;

template <template <typename> class Cond, typename Default>
struct first_if_any_or_default<Cond, Default>
{
    using type                  = Default;
    static constexpr bool found = false;
};

template <template <typename> class Cond, typename Default, typename T, typename... Ts>
struct first_if_any_or_default<Cond, Default, T, Ts...>
{
private:
    using next = first_if_any_or_default<Cond, Default, Ts...>;

public:
    using type                  = typename std::conditional<Cond<T>::value, T, typename next::type>::type;
    static constexpr bool found = Cond<T>::value || next::found;
};

template <class, template <class, class...> class>
struct is_instance : public std::false_type
{
};

template <class... Ts, template <class, class...> class U>
struct is_instance<U<Ts...>, U> : public std::true_type
{
};

} // namespace chip
