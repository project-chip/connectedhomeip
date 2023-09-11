/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <functional>

namespace chip {

/**
 * Apply the functor sequentially to the previous value and a subsequent element of the container.
 *
 * This provides a similar functionality to std::acumulate, but can be used in constexpr contexts.
 */
template <class Container, class Functor, class ValueType = typename Container::value_type>
constexpr ValueType Fold(const Container & container, ValueType initial, Functor functor)
{
    for (const auto & element : container)
    {
        initial = functor(initial, element);
    }

    return initial;
}

/**
 * Sum all elements of the container using "+" operator.
 */
template <class Container, class ValueType = typename Container::value_type>
constexpr ValueType Sum(const Container & container)
{
    return Fold(container, ValueType{}, std::plus<ValueType>{});
}

} // namespace chip
