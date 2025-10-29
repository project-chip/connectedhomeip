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
 *      Base-64 utility functions.
 *
 */

#pragma once

#include <lib/support/Span.h>

#include <stdint.h>

namespace chip {

typedef char (*Base64ValToCharFunct)(uint8_t val);
typedef uint8_t (*Base64CharToValFunct)(uint8_t c);

extern uint16_t Base64Encode(const uint8_t * in, uint16_t inLen, char * out);
extern uint16_t Base64Encode(const uint8_t * in, uint16_t inLen, char * out, Base64ValToCharFunct valToCharFunct);
extern uint16_t Base64URLEncode(const uint8_t * in, uint16_t inLen, char * out);
extern uint16_t Base64Decode(const char * in, uint16_t inLen, uint8_t * out);
extern uint16_t Base64Decode(const char * in, uint16_t inLen, uint8_t * out, Base64CharToValFunct charToValFunct);
extern uint16_t Base64URLDecode(const char * in, uint16_t inLen, uint8_t * out);

extern uint32_t Base64Encode32(const uint8_t * in, uint32_t inLen, char * out);
extern uint32_t Base64Encode32(const uint8_t * in, uint32_t inLen, char * out, Base64ValToCharFunct valToCharFunct);
extern uint32_t Base64Decode32(const char * in, uint32_t inLen, uint8_t * out);
extern uint32_t Base64Decode32(const char * in, uint32_t inLen, uint8_t * out, Base64CharToValFunct charToValFunct);

extern CHIP_ERROR Base64Encode(const ByteSpan & input, MutableCharSpan & output);
extern CHIP_ERROR Base64URLEncode(const ByteSpan & input, MutableCharSpan & output);
extern CHIP_ERROR Base64Decode(const CharSpan & input, MutableByteSpan & output);
extern CHIP_ERROR Base64URLDecode(const CharSpan & input, MutableByteSpan & output);

#define BASE64_ENCODED_LEN(LEN) ((((LEN) + 2) / 3) * 4)

#define BASE64_MAX_DECODED_LEN(LEN) ((LEN) * 3 / 4)

} // namespace chip
