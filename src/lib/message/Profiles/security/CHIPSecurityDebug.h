/*
 *
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file declares utility functions for outputting
 *      information related to CHIP security.
 *
 *      @note These function symbols are only available when
 *            CHIP_CONFIG_ENABLE_SECURITY_DEBUG_FUNCS has been
 *            asserted.
 *
 */

#ifndef CHIPSECURITYDEBUG_H_
#define CHIPSECURITYDEBUG_H_

#include <stdint.h>
#include <stdio.h>

#include <core/CHIPCore.h>
#include <Profiles/security/CHIPSecurity.h>
#include <Profiles/security/CHIPCert.h>

namespace chip {
namespace Profiles {
namespace Security {

#if CHIP_CONFIG_ENABLE_SECURITY_DEBUG_FUNCS

extern void PrintCert(FILE *out, const ChipCertificateData& cert, const ChipCertificateSet  *certSet, uint16_t indent = 0, bool verbose = false);
extern void PrintCertValidationResults(FILE *out, const ChipCertificateSet& certSet, const ValidationContext& validContext, uint16_t indent = 0);
extern CHIP_ERROR PrintChipDN(FILE * out, TLVReader & reader);
extern void PrintChipDN(FILE *out, const ChipDN& dn);
extern void PrintPackedTime(FILE *out, uint32_t t);
extern void PrintPackedDate(FILE *out, uint16_t t);
extern const char *DescribeChipCertId(OID attrOID, uint64_t weaveCertId);
extern CHIP_ERROR PrintCertArray(FILE * out, TLVReader & reader, uint16_t indent);
extern CHIP_ERROR PrintECDSASignature(FILE * out, TLVReader & reader, uint16_t indent);
extern CHIP_ERROR PrintCertReference(FILE * out, TLVReader & reader, uint16_t indent);
extern CHIP_ERROR PrintChipSignature(FILE * out, TLVReader & reader, uint16_t indent);

#endif // CHIP_CONFIG_ENABLE_SECURITY_DEBUG_FUNCS

} // namespace Security
} // namespace Profiles
} // namespace chip

#endif /* CHIPSECURITYDEBUG_H_ */
