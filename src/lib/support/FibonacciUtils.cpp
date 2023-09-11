/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
