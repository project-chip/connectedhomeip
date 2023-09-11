/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Base-64 utility functions.
 *
 */

#pragma once

#include <stdint.h>

namespace chip {

typedef char (*Base64ValToCharFunct)(uint8_t val);
typedef uint8_t (*Base64CharToValFunct)(uint8_t c);

// Encode an array of bytes to a base64 string.
//
// Returns length of generated string.
// Output will contain padding characters ('=') as necessary to make length a multiple of 4 characters.
// Output DOES NOT include a null terminator.
// Output buffer must be at least (inLen + 2) / 3 * 4 bytes long.
// Input and output buffers CANNOT overlap.
//
extern uint16_t Base64Encode(const uint8_t * in, uint16_t inLen, char * out);
extern uint16_t Base64URLEncode(const uint8_t * in, uint16_t inLen, char * out);
extern uint16_t Base64Encode(const uint8_t * in, uint16_t inLen, char * out, Base64ValToCharFunct valToCharFunct);

// Decode a base64 string to bytes.
//
// Returns length of decoded data, or UINT16_MAX if input could not be decoded.
// Input MAY contain padding characters ('=') but only at the end of the input string.
// Output buffer must be at least inLen * 3 / 4 bytes long, however the actual output
//   may be shorter than this due to padding.
// Supports decode in place by setting out pointer equal to in.
//
extern uint16_t Base64Decode(const char * in, uint16_t inLen, uint8_t * out);
extern uint16_t Base64URLDecode(const char * in, uint16_t inLen, uint8_t * out);
extern uint16_t Base64Decode(const char * in, uint16_t inLen, uint8_t * out, Base64CharToValFunct charToValFunct);

// Encode/decode functions that take/return 32-bit lengths.
//
// Similar to the above functions, except Base64Decode32() returns UINT32_MAX if the input cannot be decoded.
//
extern uint32_t Base64Encode32(const uint8_t * in, uint32_t inLen, char * out);
extern uint32_t Base64Encode32(const uint8_t * in, uint32_t inLen, char * out, Base64ValToCharFunct valToCharFunct);
extern uint32_t Base64Decode32(const char * in, uint32_t inLen, uint8_t * out);
extern uint32_t Base64Decode32(const char * in, uint32_t inLen, uint8_t * out, Base64CharToValFunct charToValFunct);

/** Computes the base-64 encoded length for a given input length.
 *
 * The computed length includes room for padding characters.
 *
 * NOTE: The supplied argument must be an integer type.
 */
#define BASE64_ENCODED_LEN(LEN) ((((LEN) + 2) / 3) * 4)

/** Computes the maximum possible decoded length for a given base-64 string input length.
 *
 * NOTE: The actual decoded length may be smaller than this due to padding.
 */
#define BASE64_MAX_DECODED_LEN(LEN) ((LEN) *3 / 4)

} // namespace chip
