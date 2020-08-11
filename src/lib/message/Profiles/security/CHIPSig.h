/*
 *
 *    Copyright (c) 2020 Google LLC.
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
 *      This file defines interfaces for generating, verifying, and
 *      working with CHIP security signatures.
 *
 */

#ifndef CHIPSIG_H_
#define CHIPSIG_H_

#include <core/CHIPTLV.h>
#include <support/ASN1.h>
#include <Profiles/security/CHIPCert.h>


namespace chip {
namespace Profiles {
namespace Security {

enum {
    kGenerateChipSignatureFlag_None                                      = 0,
    kGenerateChipSignatureFlag_IncludeSigningCertSubjectDN               = 0x0001,
    kGenerateChipSignatureFlag_IncludeSigningCertKeyId                   = 0x0002,
    kGenerateChipSignatureFlag_IncludeRelatedCertificates                = 0x0004,
};


/**
 * Provides generic functionality for generating ChipSignatures.
 *
 * This is an abstract base class that can be used encode ChipSignature TLV structures.
 * This class provides the common functionality for encoding such signatures but delegates
 * to the subclass to compute and encode the signature data field.
 */
class ChipSignatureGeneratorBase
{
public:
    enum
    {
        kFlag_None                                          = 0,
        kFlag_IncludeSigningCertSubjectDN                   = 0x0001,
        kFlag_IncludeSigningCertKeyId                       = 0x0002,
        kFlag_IncludeRelatedCertificates                    = 0x0004,
    };

    ChipCertificateSet & CertSet;
    ChipCertificateData * SigningCert;
    OID SigAlgoOID;
    uint16_t Flags;

    CHIP_ERROR GenerateSignature(const uint8_t * msgHash, uint8_t msgHashLen, TLVWriter & writer);
    CHIP_ERROR GenerateSignature(const uint8_t * msgHash, uint8_t msgHashLen, uint8_t * sigBuf, uint16_t sigBufSize, uint16_t & sigLen);
    virtual CHIP_ERROR GenerateSignature(const uint8_t * msgHash, uint8_t msgHashLen, TLVWriter & writer, uint64_t tag);

protected:
    ChipSignatureGeneratorBase(ChipCertificateSet & certSet);

    virtual CHIP_ERROR GenerateSignatureData(const uint8_t * msgHash, uint8_t msgHashLen, TLVWriter & writer) = 0;
};

/**
 * Generates a ChipSignature using an in-memory private key.
 *
 * This is class can be used encode a ChipSignature TLV structure where the signature data field is computed
 * using a supplied private key.
 */
class ChipSignatureGenerator : public ChipSignatureGeneratorBase
{
public:
    const uint8_t * PrivKey;
    uint16_t PrivKeyLen;

    ChipSignatureGenerator(ChipCertificateSet & certSet, const uint8_t * privKey, uint16_t privKeyLen);

    CHIP_ERROR GenerateSignature(const uint8_t * msgHash, uint8_t msgHashLen, TLVWriter & writer, uint64_t tag) __OVERRIDE;

    using ChipSignatureGeneratorBase::GenerateSignature;

private:
    virtual CHIP_ERROR GenerateSignatureData(const uint8_t * msgHash, uint8_t msgHashLen, TLVWriter & writer) __OVERRIDE;
};


inline ChipSignatureGeneratorBase::ChipSignatureGeneratorBase(ChipCertificateSet & certSet)
: CertSet(certSet)
{
    SigningCert = certSet.LastCert();
    SigAlgoOID = chip::ASN1::kOID_SigAlgo_ECDSAWithSHA256;
    Flags = kFlag_IncludeRelatedCertificates;
}

inline ChipSignatureGenerator::ChipSignatureGenerator(ChipCertificateSet & certSet, const uint8_t * privKey, uint16_t privKeyLen)
: ChipSignatureGeneratorBase(certSet), PrivKey(privKey), PrivKeyLen(privKeyLen)
{
}


extern CHIP_ERROR VerifyChipSignature(const uint8_t *msgHash, uint8_t msgHashLen,
                                        const uint8_t *sig, uint16_t sigLen,
                                        ChipCertificateSet& certSet,
                                        ValidationContext& certValidContext);

extern CHIP_ERROR VerifyChipSignature(const uint8_t *msgHash, uint8_t msgHashLen,
                                        const uint8_t *sig, uint16_t sigLen, OID expectedSigAlgoOID,
                                        ChipCertificateSet& certSet,
                                        ValidationContext& certValidContext);

extern CHIP_ERROR GetChipSignatureAlgo(const uint8_t *sig, uint16_t sigLen, OID& sigAlgoOID);

extern CHIP_ERROR GenerateAndEncodeChipECDSASignature(TLVWriter& writer, uint64_t tag,
        const uint8_t * msgHash, uint8_t msgHashLen,
        const uint8_t * signingKey, uint16_t signingKeyLen);

extern CHIP_ERROR EncodeChipECDSASignature(TLVWriter& writer, EncodedECDSASignature& sig, uint64_t tag);
extern CHIP_ERROR DecodeChipECDSASignature(TLVReader& reader, EncodedECDSASignature& sig);
extern CHIP_ERROR DecodeCopyECDSASignature_DER(const uint8_t * sigBuf, uint8_t sigLen, EncodedECDSASignature& sig);
extern CHIP_ERROR ConvertECDSASignature_DERToChip(const uint8_t * sigBuf, uint8_t sigLen, TLVWriter& writer, uint64_t tag);
extern CHIP_ERROR InsertRelatedCertificatesIntoChipSignature(
                                        uint8_t *sigBuf, uint16_t sigLen, uint16_t sigBufLen,
                                        const uint8_t *relatedCerts, uint16_t relatedCertsLen,
                                        uint16_t& outSigLen);

} // namespace Security
} // namespace Profiles
} // namespace chip


#endif /* CHIPSIG_H_ */
