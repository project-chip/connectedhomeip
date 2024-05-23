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

#include <stdint.h>
#include <stdlib.h>

#include "FibonacciUtils.h"

namespace chip {

uint32_t GetFibonacciForIndex(uint32_t inIndex)
{
    uint32_t retval = 0;
    uint32_t tmp    = 0;
    uint32_t vals[2];
    uint32_t index = 0;

    vals[0] = 0;
    vals[1] = 1;

    if (inIndex < 2)
    {
        retval = vals[inIndex];
    }
    else
    {
        for (index = 2; index <= inIndex; index++)
        {
            tmp = vals[0] + vals[1];

            vals[0] = vals[1];
            vals[1] = tmp;
        }

        retval = tmp;
    }

    return retval;
}

} // namespace chip
