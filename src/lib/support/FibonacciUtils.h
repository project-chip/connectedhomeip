/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file implements utility functions for deriving a fibonacci
 *      number from an index.
 *
 */

#pragma once

#include <cstdint>

namespace chip {

/**
 *  This function generates 32-bit Fibonacci number
 *  for a given 32 bit index. The index boundary is 47, then it would overflow.
 *
 * @return  32-bit unsigned fibonacci number.
 *
 */
extern uint32_t GetFibonacciForIndex(uint32_t index);

} // namespace chip
