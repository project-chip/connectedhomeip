/**
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
 *      This file provides declarations required by the CHIP ZCL codec.
 *      An implementation of the codec needs to implement the functions
 *      declared in this header.
 *
 */
#ifndef CHIP_ZCL_CODEC
#define CHIP_ZCL_CODEC

/**
 * Base types for the codec. This is a smaller subset than the actual ZCL types, and the
 * generated layer for a specific code is responsible for mapping ZCL types onto these
 * base types.
 */
enum
{
    CHIP_ZCL_CODEC_TYPE_BOOLEAN,

    CHIP_ZCL_CODEC_TYPE_INTEGER,
    CHIP_ZCL_CODEC_TYPE_UNSIGNED_INTEGER,

    CHIP_ZCL_CODEC_TYPE_BINARY,
    CHIP_ZCL_CODEC_TYPE_FIXED_LENGTH_BINARY,

    CHIP_ZCL_CODEC_TYPE_STRING,
    CHIP_ZCL_CODEC_TYPE_MAX_LENGTH_STRING,

    // These are used for struct-based encoding/decoding from/to an
    // EmberZclStringType_t substructure.
    CHIP_ZCL_CODEC_TYPE_UINT8_LENGTH_STRING,
    CHIP_ZCL_CODEC_TYPE_UINT16_LENGTH_STRING,

    // Additional types for non-struct internal use.
    //
    // These allow direct encoding/decoding from/to a buffer containing a
    // length-prefixed binary or text string.
    CHIP_ZCL_CODEC_TYPE_UINT8_LENGTH_PREFIXED_BINARY,
    CHIP_ZCL_CODEC_TYPE_UINT16_LENGTH_PREFIXED_BINARY,
    CHIP_ZCL_CODEC_TYPE_UINT8_LENGTH_PREFIXED_STRING,
    CHIP_ZCL_CODEC_TYPE_UINT16_LENGTH_PREFIXED_STRING,

    // Used for NULL and other weird cases.
    CHIP_ZCL_CODEC_TYPE_MISC,

    // Markers that are not really value types.
    CHIP_ZCL_CODEC_START_MARKER,
    CHIP_ZCL_CODEC_ARRAY_MARKER
};

/**
 * @brief Start a decoding session.
 */

#endif // CHIP_ZCL_CODEC