/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <algorithm>
#include <stdint.h>
#include <stdlib.h>

namespace chip {
namespace Sorting {

/**
 *
 * Impements the bubble sort algorithm to sort an array
 * of items of size 'n'.
 *
 * T should be swappable using std::swap (i.e implements Swappable).
 *
 * The provided comparison function SHALL have the following signature:
 *
 *      bool Compare(const T& a, const T& b);
 *
 * If a is deemed less than (i.e is ordered before) b, return true.
 * Else, return false.
 *
 * This is a stable sort.
 *
 * NOTE: This has a worst case time complexity of O(n^2). This
 * is well-suited for small arrays where the time trade-off is
 * acceptable compared to the lower flash cost for the simple sorting
 * implementation.
 *
 */
template <typename T, typename CompareFunc>
void BubbleSort(T * items, size_t n, CompareFunc f)
{
    for (size_t i = 0; i < (n - 1); i++)
    {
        for (size_t j = 0; j < (n - i - 1); j++)
        {
            const T & a = items[j + 1];
            const T & b = items[j];

            if (f(a, b))
            {
                std::swap(items[j], items[j + 1]);
            }
        }
    }
}

/**
 *
 * Impements the insertion sort algorithm to sort an array
 * of items of size 'n'.
 *
 * The provided comparison function SHALL have the following signature:
 *
 *      bool Compare(const T& a, const T& b);
 *
 * If a is deemed less than (i.e is ordered before) b, return true.
 * Else, return false.
 *
 * This is a stable sort.
 *
 */
template <typename T, typename CompareFunc>
void InsertionSort(T * items, size_t n, CompareFunc f)
{
    for (size_t i = 1; i < n; i++)
    {
        const T key = items[i];
        int j       = static_cast<int>(i) - 1;

        while (j >= 0 && f(key, items[j]))
        {
            items[j + 1] = items[j];
            j--;
        }
        items[j + 1] = key;
    }
}

} // namespace Sorting

} // namespace chip
