/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <utility>

template <typename Function>
class Defer
{
public:
    Defer(Function && function) : mFunction(std::move(function)) {}
    ~Defer() { mFunction(); }

    Defer(Defer & that) = default;
    Defer & operator=(Defer & that) = default;
    Defer(Defer && that)            = default;
    Defer & operator=(Defer && that) = default;

private:
    Function mFunction;
};

/**
 * @brief
 *   Defers the execution of a function until the surrounding function returns.
 *   Create a Defer object holding a lambda:
 *
 *     auto deferred = MakeDefer([]() { -- do cleanup -- });
 *
 *   The lambda will be called as soon as the object goes out of scope.
 */
template <typename Function>
Defer<Function> MakeDefer(Function && function)
{
    return Defer<Function>(std::forward<Function>(function));
}
