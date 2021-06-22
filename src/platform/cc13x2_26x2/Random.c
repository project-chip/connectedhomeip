/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
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
