/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file provides functions for safely type casting some basic
 *      datatypes (e.g. uint8_t * <-> unsigned char *)
 *
 *
 */

#pragma once

#include <limits.h>
#include <nlassert.h>
#include <stdint.h>

namespace chip {

namespace Uint8 {

/**
 * Safely typecast a pointer to unsigned char to uint8_t
 *
 * @param[in]  in  pointer to unsigned char
 *
 * @return The typecast value
 */
inline uint8_t * from_uchar(unsigned char * in)
{
    nlSTATIC_ASSERT_PRINT(CHAR_BIT == 8, "Can't type cast unsigned char array as uint8_t array");
#ifdef __cplusplus
    return reinterpret_cast<uint8_t *>(in);
#else
    return (uint8_t *) in;
#endif
}

/**
 * Safely typecast a pointer to const unsigned char to const uint8_t
 *
 * @param[in]  in  pointer to const unsigned char
 *
 * @return The typecast value
 */
inline const uint8_t * from_const_uchar(const unsigned char * in)
{
    nlSTATIC_ASSERT_PRINT(CHAR_BIT == 8, "Can't type cast unsigned char array as uint8_t array");
#ifdef __cplusplus
    return reinterpret_cast<const uint8_t *>(in);
#else
    return (const uint8_t *) in;
#endif
}

/**
 * Safely typecast a pointer to char to uint8_t
 *
 * @param[in]  in  pointer to char
 *
 * @return The typecast value
 */
inline uint8_t * from_char(char * in)
{
    nlSTATIC_ASSERT_PRINT(CHAR_BIT == 8, "Can't type cast char array as uint8_t array");
#ifdef __cplusplus
    return reinterpret_cast<uint8_t *>(in);
#else
    return (uint8_t *) in;
#endif
}

/**
 * Safely typecast a pointer to const char to const uint8_t
 *
 * @param[in]  in  pointer to const char
 *
 * @return The typecast value
 */
inline const uint8_t * from_const_char(const char * in)
{
    nlSTATIC_ASSERT_PRINT(CHAR_BIT == 8, "Can't type cast char array as uint8_t array");
#ifdef __cplusplus
    return reinterpret_cast<const uint8_t *>(in);
#else
    return (const uint8_t *) in;
#endif
}

/**
 * Safely typecast a pointer to uint8_t to unsigned char
 *
 * @param[in]  in  pointer to uint8_t
 *
 * @return The typecast value
 */
inline unsigned char * to_uchar(uint8_t * in)
{
    nlSTATIC_ASSERT_PRINT(CHAR_BIT == 8, "Can't type cast uint8_t array to unsigned char array");
#ifdef __cplusplus
    return reinterpret_cast<unsigned char *>(in);
#else
    return (unsigned char *) in;
#endif
}

/**
 * Safely typecast a pointer to const uint8_t to const unsigned char
 *
 * @param[in]  in  pointer to const uint8_t
 *
 * @return The typecast value
 */
inline const unsigned char * to_const_uchar(const uint8_t * in)
{
    nlSTATIC_ASSERT_PRINT(CHAR_BIT == 8, "Can't type cast uint8_t array to unsigned char array");
#ifdef __cplusplus
    return reinterpret_cast<const unsigned char *>(in);
#else
    return (const unsigned char *) in;
#endif
}

/**
 * Safely typecast a pointer to uint8_t to char
 *
 * @param[in]  in  pointer to uint8_t
 *
 * @return The typecast value
 */
inline char * to_char(uint8_t * in)
{
    nlSTATIC_ASSERT_PRINT(CHAR_BIT == 8, "Can't type cast uint8_t array to char array");
#ifdef __cplusplus
    return reinterpret_cast<char *>(in);
#else
    return (char *) in;
#endif
}

/**
 * Safely typecast a pointer to const uint8_t to char
 *
 * @param[in]  in  pointer to const uint8_t
 *
 * @return The typecast value
 */
inline const char * to_const_char(const uint8_t * in)
{
    nlSTATIC_ASSERT_PRINT(CHAR_BIT == 8, "Can't type cast uint8_t array to char array");
#ifdef __cplusplus
    return reinterpret_cast<const char *>(in);
#else
    return (const char *) in;
#endif
}

} // namespace Uint8
} // namespace chip
