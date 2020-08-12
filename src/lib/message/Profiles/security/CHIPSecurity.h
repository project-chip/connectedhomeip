/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
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
 *      This file defines base and common data types and interfaces
 *      for the CHIP Security profile.
 *
 */

#ifndef CHIPSECURITY_H_
#define CHIPSECURITY_H_

#include <core/CHIPVendorIdentifiers.hpp>
#include <support/ASN1.h>

/**
 *   @namespace chip::Profiles::Security
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for the
 *     CHIP Security profile.
 */

namespace chip {
namespace Profiles {
namespace Security {

// Message Types for CHIP Security Profile
//
enum
{
    // ---- PASE Protocol Messages ----
    kMsgType_PASEInitiatorStep1       = 1,
    kMsgType_PASEResponderStep1       = 2,
    kMsgType_PASEResponderStep2       = 3,
    kMsgType_PASEInitiatorStep2       = 4,
    kMsgType_PASEResponderKeyConfirm  = 5,
    kMsgType_PASEResponderReconfigure = 6,

    // ---- CASE Protocol Messages ----
    kMsgType_CASEBeginSessionRequest  = 10,
    kMsgType_CASEBeginSessionResponse = 11,
    kMsgType_CASEInitiatorKeyConfirm  = 12,
    kMsgType_CASEReconfigure          = 13,

    // ---- TAKE Protocol Messages ----
    kMsgType_TAKEIdentifyToken               = 20,
    kMsgType_TAKEIdentifyTokenResponse       = 21,
    kMsgType_TAKETokenReconfigure            = 22,
    kMsgType_TAKEAuthenticateToken           = 23,
    kMsgType_TAKEAuthenticateTokenResponse   = 24,
    kMsgType_TAKEReAuthenticateToken         = 25,
    kMsgType_TAKEReAuthenticateTokenResponse = 26,

    // ---- Key Extraction Protocol ----
    kMsgType_KeyExportRequest     = 30,
    kMsgType_KeyExportResponse    = 31,
    kMsgType_KeyExportReconfigure = 32,

    // ---- Certificate Provisioning Protocol ----
    kMsgType_GetCertificateRequest  = 40,
    kMsgType_GetCertificateResponse = 41,

    // ---- General Messages ----
    kMsgType_EndSession         = 100,
    kMsgType_KeyError           = 101,
    kMsgType_MsgCounterSyncResp = 102,
};

// CHIP Security Status Codes
//
enum
{
    kStatusCode_SessionAborted                 = 1,  // The sender has aborted the session establishment process.
    kStatusCode_PASESupportsOnlyConfig1        = 2,  // PASE supports only Config1.
    kStatusCode_UnsupportedEncryptionType      = 3,  // The requested encryption type is not supported.
    kStatusCode_InvalidKeyId                   = 4,  // An invalid key id was requested.
    kStatusCode_DuplicateKeyId                 = 5,  // The specified key id is already in use.
    kStatusCode_KeyConfirmationFailed          = 6,  // The derived session keys do not agree.
    kStatusCode_InternalError                  = 7,  // The sender encountered an internal error (e.g. no memory, etc...).
    kStatusCode_AuthenticationFailed           = 8,  // The sender rejected the authentication attempt.
    kStatusCode_UnsupportedCASEConfiguration   = 9,  // No common CASE configuration supported.
    kStatusCode_UnsupportedCertificate         = 10, // An unsupported certificate was offered.
    kStatusCode_NoCommonPASEConfigurations     = 11, // No common PASE configuration supported.
    kStatusCode_KeyNotFound                    = 12, // The specified key is not found.
    kStatusCode_WrongEncryptionType            = 13, // The specified encryption type is invalid.
    kStatusCode_UnknownKeyType                 = 14, // The specified key has unknown key type.
    kStatusCode_InvalidUseOfSessionKey         = 15, // The specified key is used incorrectly.
    kStatusCode_InternalKeyError               = 16, // The receiver of the CHIP message encountered key error.
    kStatusCode_NoCommonKeyExportConfiguration = 17, // No common key export protocol configuration supported.
    kStatusCode_UnauthorizedKeyExportRequest   = 18, // An unauthorized key export request.
    kStatusCode_NoNewOperationalCertRequired   = 19, // No new operational certificate required.
    kStatusCode_OperationalNodeIdInUse =
        20, // The specified operational node Id is already used by another CHIP node (indication of node id collision).
    kStatusCode_InvalidOperationalNodeId      = 21, // The specified operational node Id is invalid.
    kStatusCode_InvalidOperationalCertificate = 22, // The specified operational certificate is invalid.
};

// CHIP Key Error Message Size
//
enum
{
    kChipKeyErrorMessageSize = 9, // The size of the key error message.
};

// CHIP Message Counter Synchronization Response Message Size.
//
enum
{
    kChipMsgCounterSyncRespMsgSize = 4, // The size of the message counter synchronization response message.
};

// Data Element Tags for the CHIP Security Profile
//
enum
{
    // ---- Top-level Profile-Specific Tags ----
    kTag_ChipCertificate                = 1, // [ structure ] A CHIP certificate.
    kTag_EllipticCurvePrivateKey        = 2, // [ structure ] An elliptic curve private key.
    kTag_RSAPrivateKey                  = 3, // [ structure ] An RSA private key.
    kTag_ChipCertificateList            = 4, // [ array ] An array of CHIP certificates.
    kTag_ChipSignature                  = 5, // [ structure ] A CHIP signature object.
    kTag_ChipCertificateReference       = 6, // [ structure ] A CHIP certificate reference object.
    kTag_ChipCASECertificateInformation = 7, // [ structure ] A CHIP CASE certificate information object.
    kTag_ChipCASESignature              = 8, // [ structure ] An CHIP CASE signature object.
                                             //    Presently this has the same internal structure as an ECDSASignature.
    kTag_ChipAccessToken   = 9,              // [ structure ] A CHIP Access Token object
    kTag_GroupKeySignature = 10,             // [ structure ] A CHIP group Key signature object
    kTag_SerializedSession = 11,             // [ structure ] A serialized representation of CHIP session
                                             //    suitable for persisting.

    // ---- Context-specific Tags for ChipCertificate Structure ----
    kTag_SerialNumber            = 1,  // [ byte string ] Certificate serial number, in BER integer encoding.
    kTag_SignatureAlgorithm      = 2,  // [ unsigned int ] Enumerated value identifying the certificate signature algorithm.
    kTag_Issuer                  = 3,  // [ path ] The issuer distinguished name of the certificate.
    kTag_NotBefore               = 4,  // [ unsigned int ] Certificate validity period start (certificate date format).
    kTag_NotAfter                = 5,  // [ unsigned int ] Certificate validity period end (certificate date format).
    kTag_Subject                 = 6,  // [ path ] The subject distinguished name of the certificate.
    kTag_PublicKeyAlgorithm      = 7,  // [ unsigned int ] Identifies the algorithm with which the public key can be used.
    kTag_EllipticCurveIdentifier = 8,  // [ unsigned int ] For EC certs, identifies the elliptic curve used.
    kTag_RSAPublicKey            = 9,  // [ structure ] The RSA public key.
    kTag_EllipticCurvePublicKey  = 10, // [ byte string ] The elliptic curve public key, in X9.62 encoded format.
    kTag_RSASignature            = 11, // [ byte string ] The RSA signature for the certificate.
    kTag_ECDSASignature          = 12, // [ structure ] The ECDSA signature for the certificate.
                                       // Tags identifying certificate extensions (tag numbers 128 - 255)
    kCertificateExtensionTagsStart = 128,
    kTag_AuthorityKeyIdentifier    = 128, // [ structure ] Information about the public key used to sign the certificate.
    kTag_SubjectKeyIdentifier      = 129, // [ structure ] Information about the certificate's public key.
    kTag_KeyUsage                  = 130, // [ structure ] TODO: document me
    kTag_BasicConstraints          = 131, // [ structure ] TODO: document me
    kTag_ExtendedKeyUsage          = 132, // [ structure ] TODO: document me
    kCertificateExtensionTagsEnd   = 255,

    // ---- Context-specific Tags for RSAPublicKey Structure ----
    kTag_RSAPublicKey_Modulus        = 1, // [ byte string ] RSA public key modulus, in ASN.1 integer encoding.
    kTag_RSAPublicKey_PublicExponent = 2, // [ unsigned int ] RSA public key exponent.

    // ---- Context-specific Tags for ECDSASignature Structure ----
    kTag_ECDSASignature_r = 1, // [ byte string ] ECDSA r value, in ASN.1 integer encoding.
    kTag_ECDSASignature_s = 2, // [ byte string ] ECDSA s value, in ASN.1 integer encoding.

    // ---- Context-specific Tags for AuthorityKeyIdentifier Structure ----
    kTag_AuthorityKeyIdentifier_Critical =
        1, // [ boolean ] True if the AuthorityKeyIdentifier extension is critical. Otherwise absent.
    kTag_AuthorityKeyIdentifier_KeyIdentifier = 2, // [ byte string ] TODO: document me
    kTag_AuthorityKeyIdentifier_Issuer        = 3, // [ path ] TODO: document me
    kTag_AuthorityKeyIdentifier_SerialNumber  = 4, // [ byte string ] TODO: document me

    // ---- Context-specific Tags for SubjectKeyIdentifier Structure ----
    kTag_SubjectKeyIdentifier_Critical = 1, // [ boolean ] True if the SubjectKeyIdentifier extension is critical. Otherwise absent.
    kTag_SubjectKeyIdentifier_KeyIdentifier = 2, // [ byte string ] Unique identifier for certificate's public key, per RFC5280.

    // ---- Context-specific Tags for KeyUsage Structure ----
    kTag_KeyUsage_Critical = 1, // [ boolean ] True if the KeyUsage extension is critical. Otherwise absent.
    kTag_KeyUsage_KeyUsage = 2, // [ unsigned int ] Integer containing key usage bits, per to RFC5280.

    // ---- Context-specific Tags for BasicConstraints Structure ----
    kTag_BasicConstraints_Critical = 1, // [ boolean ] True if the BasicConstraints extension is critical. Otherwise absent.
    kTag_BasicConstraints_IsCA     = 2, // [ boolean ] True if the certificate can be used to verify certificate signatures.
    kTag_BasicConstraints_PathLenConstraint = 3, // [ unsigned int ] Maximum number of subordinate intermediate certificates.

    // ---- Context-specific Tags for ExtendedKeyUsage Structure ----
    kTag_ExtendedKeyUsage_Critical = 1, // [ boolean ] True if the ExtendedKeyUsage extension is critical. Otherwise absent.
    kTag_ExtendedKeyUsage_KeyPurposes =
        2, // [ array ] Array of enumerated values giving the purposes for which the public key can be used.

    // ---- Context-specific Tags for EllipticCurvePrivateKey Structure ----
    kTag_EllipticCurvePrivateKey_CurveIdentifier = 1, // [ unsigned int ] ChipCurveId identifying the elliptic curve.
    kTag_EllipticCurvePrivateKey_PrivateKey =
        2,                                      // [ byte string ] Private key encoded using the I2OSP algorithm defined in RFC3447.
    kTag_EllipticCurvePrivateKey_PublicKey = 3, // [ byte string ] The elliptic curve public key, in X9.62 encoded format.

    // ---- Context-specific Tags for RSAPrivateKey Structure ----
    // ... TBD ...

    // ---- Context-specific Tags for ChipSignature Structure ----
    kTag_ChipSignature_ECDSASignatureData = 1, // [ structure ] ECDSA signature data for the signed message.
    kTag_ChipSignature_RSASignatureData   = 2, // [ byte string ] RSA signature for the signed message.
                                               //   Per the schema, exactly one of ECDSASignature or RSASignature must be present.
    kTag_ChipSignature_SigningCertificateRef =
        3, // [ structure ] A CHIP certificate reference structure identifying the certificate
           //   used to generate the signature. If absent, the signature was generated by the
           //   first certificate in the RelatedCertificates list.
    kTag_ChipSignature_RelatedCertificates =
        4, // [ array ] Array of certificates needed to validate the signature.  May be omitted if
           //   validators are expected to have the necessary certificates for validation.
           //   At least one of SigningCertificateRef or RelatedCertificates must be present.
    kTag_ChipSignature_SignatureAlgorithm =
        5, // [ unsigned int ] Enumerated value identifying the signature algorithm.
           //   Legal values per the schema are: kOID_SigAlgo_ECDSAWithSHA1, kOID_SigAlgo_ECDSAWithSHA256,
           //     kOID_SigAlgo_SHA1WithRSAEncryption and kOID_SigAlgo_SHA256WithRSAEncryption.
           //   For backwards compatibility, this field should be omitted when the signature
           //     algorithm is ECDSAWithSHA1.
           //   When this field is included it must appear first within the ChipSignature structure.
           //   kOID_SigAlgo_SHA1WithRSAEncryption and kOID_SigAlgo_SHA256WithRSAEncryption are
           //     not presently supported in the code.

    // ---- Context-specific Tags for CHIP Certificate Reference Structure ----
    kTag_ChipCertificateRef_Subject = 1, // [ path ] The subject DN of the referenced certificate.
    kTag_ChipCertificateRef_PublicKeyId =
        2, // [ byte string ] Unique identifier for referenced certificate's public key, per RFC5280.

    // ---- Context-specific Tags for CHIP CASE Certificate Information Structure ----
    kTag_CASECertificateInfo_EntityCertificate =
        1, // [ structure ] A CHIP certificate object representing the authenticating entity.
    kTag_CASECertificateInfo_EntityCertificateRef =
        2, // [ structure ] A CHIP certificate reference object identifying the authenticating entity.
    kTag_CASECertificateInfo_RelatedCertificates = 3, // [ path ] A collection of certificates related to the authenticating entity.
    kTag_CASECertificateInfo_TrustAnchors =
        4, // [ path ] A collection of CHIP certificate reference identifying certificates trusted
           //   by the authenticating entity.

    // ---- Context-specific Tags for CHIP Access Token Structure ----
    kTag_AccessToken_Certificate = 1, // [ structure ] A CHIP certificate object representing the entity that is trusted to
                                      //   access a device or fabric.
    kTag_AccessToken_PrivateKey = 2,  // [ structure ] An EllipticCurvePrivateKey object containing the private key associated
                                      //   with the access token certificate.
    kTag_AccessToken_RelatedCertificates = 3, // [ array, optional ] An optional array of certificates related to the access token
                                              //   certificate that may be needed to validate it.

    kTag_GroupKeySignature_SignatureAlgorithm = 1, //  [ unsigned int ] Enumerated value identifying the certificate signature
                                                   //  algorithm.  Legal values are taken from the kOID_SigAlgo_* constant
                                                   //  namespace.  The only value currently supported is
                                                   //  kOID_SigAlgo_HMACWithSHA256.  When the tag is ommitted the signature
                                                   //  algorithm defaults to HMACWithSHA256
    kTag_GroupKeySignature_KeyId     = 2,          //  [ unsigned int ] CHIP KeyId to be used to generate and verify the signature
    kTag_GroupKeySignature_Signature = 3,          //  [ byte string ] Signature bytes themselves.

    // ---- Context-specific Tags for GetCertificateRequest Message Structure ----
    kTag_GetCertReqMsg_ReqType = 1, // [ unsigned int ] Identifies the certificate request type.
    kTag_GetCertReqMsg_Authorize_PairingToken =
        2, // [ byte string, optional ] Pairing token from the service. This token is used to authorize
           //   GetCertificateRequest message to the CA service.
    kTag_GetCertReqMsg_Authorize_PairingInitData =
        3, // [ byte string, optional ] Pairing initialization data from the service. This data is used
           //   to authorize GetCertificateRequest message to the CA service.
    kTag_GetCertReqMsg_OpDeviceCert = 4, // [ structure ] CHIP operational device certificate.
                                         //   This has the same internal structure as an kTag_ChipCertificate.
    kTag_GetCertReqMsg_OpRelatedCerts =
        5, // [ array, optional ] An optional array of CHIP operational intermediate CA certificates,
           //   which are needed to validate CHIP operational device certificate. May be omitted if
           //   validators are expected to have the necessary certificates for validation.
           //   Each element of this array has the same internal structure as an kTag_ChipCertificate.
    kTag_GetCertReqMsg_MfrAttest_ChipCert =
        6, // [ structure ] CHIP device certificate provisioned at the factory at manufacturing time.
           //   This has the same internal structure as an kTag_ChipCertificate.
    kTag_GetCertReqMsg_MfrAttest_ChipRelCerts =
        7, // [ array, optional ] An optional array of CHIP intermediate CA certificates, which are needed
           //   to validate included manufacturer-assigned CHIP device certificate. May be omitted if
           //   validators are expected to have the necessary certificates for validation.
           //   Each element of this array has the same internal structure as an kTag_ChipCertificate.
    kTag_GetCertReqMsg_MfrAttest_X509Cert =
        8, // [ byte string ] X509 device certificate provisioned at the factory at manufacturing time.
           //   This is an ASN1 encoded ECDSA or RSA certificate TLV-encoded as a data blob.
    kTag_GetCertReqMsg_MfrAttest_X509RelCerts =
        9, // [ array, optional ] An optional array of X509 intermediate CA certificates,
           //   which are needed to validate included manufacturer-assigned X509 device certificate.
           //   May be omitted if validators are expected to have the necessary certificates for validation.
           //   Each element of this array is a byte string, and each X509 certificate is TLV-encoded as a data blob.
    kTag_GetCertReqMsg_MfrAttest_HMACKeyId =
        10, // [ unsigned int ] Identifies the secret key that is used for the HMAC manufacturer attestation.
    kTag_GetCertReqMsg_MfrAttest_HMACMetaData =
        11, // [ byte string, optional ] Meta data associated with the HMAC manufacturer attestation.
    kTag_GetCertReqMsg_OpDeviceSigAlgo = 12,   // [ unsigned int ] Enumerated value identifying the operational signature algorithm.
                                               //   Legal values are taken from the kOID_SigAlgo_* constant namespace.
    kTag_GetCertReqMsg_OpDeviceSig_ECDSA = 13, // [ structure ] Operational device EC signature.
                                               //   This has the same internal structure as an kTag_ECDSASignature.
    kTag_GetCertReqMsg_MfrAttestSigAlgo =
        14, // [ unsigned int ] Enumerated value identifying the manufacturer attestation signature algorithm.
            //   Legal values are taken from the kOID_SigAlgo_* constant namespace.
    kTag_GetCertReqMsg_MfrAttestSig_ECDSA = 15, // [ structure ] Manufacturer attestation device EC signature.
                                                //   This has the same internal structure as an kTag_ECDSASignature.
    kTag_GetCertReqMsg_MfrAttestSig_RSA  = 16,  // [ byte string ] Manufacturer attestation device RSA signature.
    kTag_GetCertReqMsg_MfrAttestSig_HMAC = 17,  // [ byte string ] Manufacturer attestation device HMAC signature.

    // ---- Context-specific Tags for GetCertificateResponse Message Structure ----
    kTag_GetCertRespMsg_OpDeviceCert = 1, // [ structure ] Service assigned CHIP operational device certificate.
                                          //   This has the same internal structure as an kTag_ChipCertificate.
    kTag_GetCertRespMsg_OpRelatedCerts =
        2, // [ array, optional ] An optional array of CHIP certificates related to the
           //   operational device certificate, which are needed to validate certificate.
           //   Each element of this array has the same internal structure as an kTag_ChipCertificate.

    // ---- Context-specific Tags for CHIP representation of X.509 Distinguished Name Attributes ----
    //
    // The value used here must match *exactly* the OID enum values assigned to the corresponding object ids in the gen-oid-table.py
    // script.
    //
    // WARNING! Assign no values higher than 127.
    //
    kTag_DNAttrType_CommonName              = 1,  // [ UTF8 string ]
    kTag_DNAttrType_Surname                 = 2,  // [ UTF8 string ]
    kTag_DNAttrType_SerialNumber            = 3,  // [ UTF8 string ]
    kTag_DNAttrType_CountryName             = 4,  // [ UTF8 string ]
    kTag_DNAttrType_LocalityName            = 5,  // [ UTF8 string ]
    kTag_DNAttrType_StateOrProvinceName     = 6,  // [ UTF8 string ]
    kTag_DNAttrType_OrganizationName        = 7,  // [ UTF8 string ]
    kTag_DNAttrType_OrganizationalUnitName  = 8,  // [ UTF8 string ]
    kTag_DNAttrType_Title                   = 9,  // [ UTF8 string ]
    kTag_DNAttrType_Name                    = 10, // [ UTF8 string ]
    kTag_DNAttrType_GivenName               = 11, // [ UTF8 string ]
    kTag_DNAttrType_Initials                = 12, // [ UTF8 string ]
    kTag_DNAttrType_GenerationQualifier     = 13, // [ UTF8 string ]
    kTag_DNAttrType_DNQualifier             = 14, // [ UTF8 string ]
    kTag_DNAttrType_Pseudonym               = 15, // [ UTF8 string ]
    kTag_DNAttrType_DomainComponent         = 16, // [ UTF8 string ]
    kTag_DNAttrType_ChipDeviceId            = 17, // [ unsigned int ]
    kTag_DNAttrType_ChipServiceEndpointId   = 18, // [ unsigned int ]
    kTag_DNAttrType_ChipCAId                = 19, // [ unsigned int ]
    kTag_DNAttrType_ChipSoftwarePublisherId = 20, // [ unsigned int ]

    // ---- Context-specific Tags for Serialized Session structure ----
    kTag_SerializedSession_KeyId                   = 1,     // [ UNSIGNED INT, range 16bits ] Assigned session key id
    kTag_SerializedSession_PeerNodeId              = 2,     // [ UNSIGNED INT, range 64bits ] Node id of session peer
    kTag_SerializedSession_NextMessageId           = 3,     // [ UNSIGNED INT, range 32bits ] Next message id
    kTag_SerializedSession_MaxRcvdMessageId        = 4,     // [ UNSIGNED INT, range 32bits ] Max received message id
    kTag_SerializedSession_MessageRcvdFlags        = 5,     // [ UNSIGNED INT, range 64bits ] Message received flags
    kTag_SerializedSession_IsLocallyInitiated      = 6,     // [ BOOLEAN ] Is session locally initiated
    kTag_SerializedSession_IsShared                = 7,     // [ BOOLEAN ] Is session shared
    kTag_SerializedSession_SharedSessionAltNodeIds = 8,     // [ ARRAY OF UNSIGNED INT, range 64bits ] For a shared session,
                                                            //    list of alternate peer node ids.
    kTag_SerializedSession_CASE_PeerCertType = 9,           // [ UNSIGNED INT, range 8bits ] For CASE sessions, the type
                                                            //    of certificate presented by the peer.
    kTag_SerializedSession_EncryptionType        = 10,      // [ UNSIGNED INT, range 8bits ] Message encryption type
    kTag_SerializedSession_AES128CTRSHA1_DataKey = 11,      // [ BYTE STRING, len 16 ] For sessions supporting AES128CTRSHA1
                                                            //    message encryption, the data encryption key.
    kTag_SerializedSession_AES128CTRSHA1_IntegrityKey = 12, // [ BYTE STRING, len 20 ] For sessions supporting AES128CTRSHA1
                                                            //    message encryption, the data integrity key.
};

// CHIP-defined elliptic curve ids
//
// NOTE: The bottom bits of each curve id must match the enum value used in the curve's
// ASN1 OID (see ASN1OID.h).
enum
{
    kChipCurveId_NotSpecified = 0,

    kChipCurveId_secp160r1  = (kChipVendor_NestLabs << 16) | 0x0021,
    kChipCurveId_prime192v1 = (kChipVendor_NestLabs << 16) | 0x0015,
    kChipCurveId_secp224r1  = (kChipVendor_NestLabs << 16) | 0x0025,
    kChipCurveId_prime256v1 = (kChipVendor_NestLabs << 16) | 0x001B,

    kChipCurveId_VendorMask   = 0xFFFF0000,
    kChipCurveId_VendorShift  = 16,
    kChipCurveId_CurveNumMask = ASN1::kOID_Mask,
};

// Bit-field value represented set of defined elliptic curves.
enum
{
    kChipCurveSet_Mask = 0xFF,

    kChipCurveSet_secp160r1  = 0x01,
    kChipCurveSet_prime192v1 = 0x02,
    kChipCurveSet_secp224r1  = 0x04,
    kChipCurveSet_prime256v1 = 0x08,

    kChipCurveSet_All = (kChipCurveSet_secp160r1 | kChipCurveSet_prime192v1 | kChipCurveSet_secp224r1 | kChipCurveSet_prime256v1)
};

extern bool IsSupportedCurve(uint32_t curveId);

extern bool IsCurveInSet(uint32_t curveId, uint8_t curveSet);

extern ASN1::OID ChipCurveIdToOID(uint32_t weaveCurveId);

inline uint32_t OIDToChipCurveId(ASN1::OID curveOID)
{
    return (((uint32_t) kChipVendor_NestLabs) << kChipCurveId_VendorShift) | (kChipCurveId_CurveNumMask & curveOID);
}

} // namespace Security
} // namespace Profiles
} // namespace chip

#endif /* CHIPSECURITY_H_ */
