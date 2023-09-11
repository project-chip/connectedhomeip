/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <tuple>

namespace chip {

template <typename T>
struct FunctionTraits
{
};

/**
 * Template struct for getting function return type and argument types.
 */
template <typename ReturnT, typename... ArgumentTypes>
struct FunctionTraits<ReturnT (*)(ArgumentTypes...)>
{
    using ReturnType = ReturnT;
    template <size_t i>
    using ArgType = std::tuple_element_t<i, std::tuple<ArgumentTypes...>>;
};

} // namespace chip
