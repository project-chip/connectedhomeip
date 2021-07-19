/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
 *      This file contains functions for working with ASN1 errors.
 */

#include <stdlib.h>

#include <asn1/ASN1.h>
#include <support/ErrorStr.h>

namespace chip {
namespace ASN1 {

/**
 * Given an ASN1 error, returns a human-readable NULL-terminated C string
 * describing the error.
 *
 * @param[in] buf                   Buffer into which the error string will be placed.
 * @param[in] bufSize               Size of the supplied buffer in bytes.
 * @param[in] err                   The error to be described.
 *
 * @return true                     If a description string was written into the supplied buffer.
 * @return false                    If the supplied error was not an ASN1 error.
 *
 */
bool FormatASN1Error(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    const char * desc = nullptr;

    if (!ChipError::IsPart(ChipError::SdkPart::kASN1, err))
    {
        return false;
    }

#if !CHIP_CONFIG_SHORT_ERROR_STR
    switch (ChipError::AsInteger(err))
    {
    case ChipError::AsInteger(ASN1_END):
        desc = "End of input";
        break;
    case ChipError::AsInteger(ASN1_ERROR_UNDERRUN):
        desc = "Reader underrun";
        break;
    case ChipError::AsInteger(ASN1_ERROR_OVERFLOW):
        desc = "Writer overflow";
        break;
    case ChipError::AsInteger(ASN1_ERROR_INVALID_STATE):
        desc = "Invalid state";
        break;
    case ChipError::AsInteger(ASN1_ERROR_MAX_DEPTH_EXCEEDED):
        desc = "Max depth exceeded";
        break;
    case ChipError::AsInteger(ASN1_ERROR_INVALID_ENCODING):
        desc = "Invalid encoding";
        break;
    case ChipError::AsInteger(ASN1_ERROR_UNSUPPORTED_ENCODING):
        desc = "Unsupported encoding";
        break;
    case ChipError::AsInteger(ASN1_ERROR_TAG_OVERFLOW):
        desc = "Tag overflow";
        break;
    case ChipError::AsInteger(ASN1_ERROR_LENGTH_OVERFLOW):
        desc = "Length overflow";
        break;
    case ChipError::AsInteger(ASN1_ERROR_VALUE_OVERFLOW):
        desc = "Value overflow";
        break;
    case ChipError::AsInteger(ASN1_ERROR_UNKNOWN_OBJECT_ID):
        desc = "Unknown object id";
        break;
    }
#endif // !CHIP_CONFIG_SHORT_ERROR_STR

    FormatError(buf, bufSize, "ASN1", err, desc);

    return true;
}

} // namespace ASN1
} // namespace chip
