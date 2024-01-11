/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "MatterError.h"

#import <Foundation/Foundation.h>

#ifndef MCCryptoUtils_h
#define MCCryptoUtils_h

@interface MCCryptoUtils : NSObject

/**
 * @brief Convert an ASN.1 DER signature (per X9.62) as used by TLS libraries to SEC1 raw format
 *
 * Errors are:
 *   - CHIP_ERROR_INVALID_ARGUMENT on any argument being invalid (e.g. nullptr), wrong sizes,
 *     wrong or unsupported format,
 *   - CHIP_ERROR_BUFFER_TOO_SMALL on running out of space at runtime.
 *   - CHIP_ERROR_INTERNAL on any unexpected processing error.
 *
 * @param[in] feLengthBytes Field Element length in bytes (e.g. 32 for P256 curve)
 * @param[in] asn1Signature ASN.1 DER signature input
 * @param[out] outRawSignature Raw signature of <r,s> concatenated format output buffer. Size must be at
 * least >= `2 * fe_length_bytes`. On success, the outRawSignature buffer will be re-assigned
 * to have the correct size (2 * feLengthBytes).
 * @return Returns an MatterError on error, MATTER_NO_ERROR otherwise
 */
+ (MatterError * _Nonnull)ecdsaAsn1SignatureToRawWithFeLengthBytes:(NSUInteger)feLengthBytes asn1Signature:(CFDataRef _Nonnull)asn1Signature outRawSignature:(NSData * _Nonnull * _Nonnull)outRawSignature;

@end

#endif /* MCCryptoUtils_h */
