/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#pragma once

#include <credentials/DeviceAttestationVendorReserved.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {
namespace Credentials {

// As per specifications section 11.22.5.1. Constant RESP_MAX
constexpr size_t kMaxRspLen = 900;

// CSRNonce and AttestationNonce need to be this size
constexpr size_t kExpectedAttestationNonceSize = 32;

/**
 *  @brief Take the attestation elements buffer and return each component separately.
 *         All output data stays valid while attestationElements buffer is valid.
 *
 *  @param[in]    attestationElements ByteSpan containing source of Attestation Elements data.
 *  @param[out]   certificationDeclaration Valid Certification Declaration data.
 *  @param[out]   attestationNonce Attestation Nonce - 32 octets required.
 *  @param[out]   timestamp Timestamp data in epoch time format.
 *  @param[out]   firmwareInfo ByteSpan containing Firmware Information data if present within attestationElements.
 *                             Empty ByteSpan if not present in attestationElements.
 *  @param[out]   vendorReserved  Placeholder to for client to examine vendorReserved elements later
 */
CHIP_ERROR DeconstructAttestationElements(const ByteSpan & attestationElements, ByteSpan & certificationDeclaration,
                                          ByteSpan & attestationNonce, uint32_t & timestamp, ByteSpan & firmwareInfo,
                                          DeviceAttestationVendorReservedDeconstructor & vendorReserved);

/**
 *  @brief Take each component separately and form the Attestation Elements buffer.
 *
 *  @param[in]  certificationDeclaration Valid Certification Declaration data.
 *  @param[in]  attestationNonce Attestation Nonce - 32 octets required.
 *  @param[in]  timestamp Timestamp data in epoch time format.
 *  @param[in]  firmwareInfo Optional Firmware Information data - Can be empty.
 *  @param[in]  vendorReserved    Prefilled-in vendor reserved elements to be put into DA elements.
 *  @param[out] attestationElements Buffer used to write all AttestationElements data, formed with all the data fields above.
 *                                  Provided buffer needs to be capable to handle all data fields + tags.
 */
CHIP_ERROR ConstructAttestationElements(const ByteSpan & certificationDeclaration, const ByteSpan & attestationNonce,
                                        uint32_t timestamp, const ByteSpan & firmwareInfo,
                                        DeviceAttestationVendorReservedConstructor & vendorReserved,
                                        MutableByteSpan & attestationElements);

/***
 *  @brief Count the number of VendorReservedElements in a DeviceAttestation blob
 *
 *  @param[in]   attestationElements ByeSpan conitaining source of Attestation Elements data
 *  @param[out]   numElements Count of vendor reserved elements in the DeviceAttestation
 *  @returns CHIP_NO_ERROR on success
 */
CHIP_ERROR CountVendorReservedElementsInDA(const ByteSpan & attestationElements, size_t & numElements);

/**
 *  @brief Take each component separately and form the CSRElements buffer.
 *
 *  @param[in]  csr Certificate Signing Request body
 *  @param[in]  csrNonce CSR Nonce - 32 octets required.
 *  @param[in]  vendor_reserved1 Optional vendor_reserved1 blob, can be empty to omit
 *  @param[in]  vendor_reserved2 Optional vendor_reserved2 blob, can be empty to omit
 *  @param[in]  vendor_reserved3 Optional vendor_reserved3 blob, can be empty to omit
 *  @param[out] nocsrElements Buffer used to write all nocsrElements data, formed with all the data fields above.
 *                            Provided buffer needs to be capable to handle all data fields + tags.
 */
CHIP_ERROR ConstructNOCSRElements(const ByteSpan & csr, const ByteSpan & csrNonce, const ByteSpan & vendor_reserved1,
                                  const ByteSpan & vendor_reserved2, const ByteSpan & vendor_reserved3,
                                  MutableByteSpan & nocsrElements);

/**
 *  @brief Take the NOCSR elements buffer and return each component seperately.
 *         All output data stays valid while nocsrElements buffer is valid.
 *
 *  @param[in]  nocsrElements ByteSpan containg source of NOCSR Elements data
 *  @param[out] csr Certificate Signing Request Body
 *  @param[out] csrNonce CSR Nonce
 *  @param[out] vendor_reserved1 Optional vendor_reserved1 blob, empty if omitted
 *  @param[out] vendor_reserved2 Optional vendor_reserved2 blob, empty if omitted
 *  @param[out] vendor_reserved3 Optional vendor_reserved3 blob, empty if omitted
 */
CHIP_ERROR DeconstructNOCSRElements(const ByteSpan & nocsrElements, ByteSpan & csr, ByteSpan & csrNonce,
                                    ByteSpan & vendor_reserved1, ByteSpan & vendor_reserved2, ByteSpan & vendor_reserved3);

} // namespace Credentials
} // namespace chip
