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

#include <utility>

template <typename Function>
class Defer
{
public:
    Defer(Function && function) : mFunction(std::move(function)) {}
    ~Defer() { mFunction(); }

    Defer(Defer & that)              = default;
    Defer & operator=(Defer & that)  = default;
    Defer(Defer && that)             = default;
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
