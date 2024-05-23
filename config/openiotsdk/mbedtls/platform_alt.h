/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *      This file provides the Mbedtls platform aternative implementation.
 *      It contains custom setup() and teardown() functions.
 */

#ifndef MBEDTLS_PLATFORM_ALT
#define MBEDTLS_PLATFORM_ALT

/**
 * \brief The dummy platform context structure.
 */
typedef struct mbedtls_platform_context
{
    char dummy;
} mbedtls_platform_context;

#endif /* MBEDTLS_PLATFORM_ALT */
