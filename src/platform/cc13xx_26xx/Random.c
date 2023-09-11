/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2020 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

/* Basic implementation of pseudo-random number generation to override the
 * default newlib-nano implementation.
 */

static unsigned long int next = 1;

/* Return next random integer */
int rand()
{
    next = next * 1103515245L + 12345;
    return (unsigned int) (next / 65536L) % 32768L;
}

/* Set seed for random generator */
void srand(unsigned int seed)
{
    next = seed;
}
