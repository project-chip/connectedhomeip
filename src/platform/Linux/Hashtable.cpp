/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *          This is a just a wrapper around NSPR's parameterized dynamic
 *          has table.
 */

#include <nspr/plhash.h>
#include <string.h>

#include <platform/Linux/Hashtable.h>

#define NBITS_IN_UNSIGNED 32 /* (NBITS(unsigned int))*/
#define SEVENTY_FIVE_PERCENT ((int) (NBITS_IN_UNSIGNED * 0.75))
#define TWELVE_PERCENT ((int) (NBITS_IN_UNSIGNED * 0.125))
#define HIGH_BITS (~((unsigned int) (~0) >> TWELVE_PERCENT))

unsigned int hash_pjw(const unsigned char * ubuf, int buflength)
{
    unsigned int g, h0 = 0;
    int i = 0;

    for (i = 0; i < buflength; i++, ubuf++)
    {
        h0 = (h0 << TWELVE_PERCENT) + *ubuf;

        if ((g = h0 & HIGH_BITS))
            h0 = (h0 ^ (g >> SEVENTY_FIVE_PERCENT)) & ~HIGH_BITS;
    }

    return h0;
}

PLHashNumber hash_key(const void * key)
{
    return hash_pjw((const unsigned char *) key, strlen((char *) key));
}
