/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/support/Span.h>

namespace chip {
namespace TestCerts {

extern const ByteSpan gTestCert_X509ToChip_ErrorCases[50];

extern const ByteSpan gTestCert_ChipToX509_ErrorCases[43];

extern const ByteSpan gTestCert_ChipCertLoad_ErrorCases[40];

extern const ByteSpan gTestCert_ValidateChipRCAC_ErrorCases[84];

extern const ByteSpan gTestCert_GetCertType_ErrorCases[12];

extern const uint8_t sChipTest_NOC_Oversized_Cert_DER[744];
extern const uint8_t sChipTest_ICAC_Oversized_Cert_DER[678];
extern const uint8_t sChipTest_RCAC_Oversized_Cert_DER[871];
extern const uint8_t sChipTest_NOC_Cert_Version_V2_Cert_DER[519];
extern const uint8_t sChipTest_ICAC_Cert_Version_V2_Cert_DER[451];
extern const uint8_t sChipTest_RCAC_Cert_Version_V2_Cert_DER[416];
extern const uint8_t sChipTest_NOC_Sig_Algo_ECDSA_With_SHA1_Cert_DER[517];
extern const uint8_t sChipTest_ICAC_Sig_Algo_ECDSA_With_SHA1_Cert_DER[448];
extern const uint8_t sChipTest_RCAC_Sig_Algo_ECDSA_With_SHA1_Cert_DER[414];
extern const uint8_t sChipTest_NOC_Validity_Not_Before_Missing_Cert_DER[505];
extern const uint8_t sChipTest_ICAC_Validity_Not_Before_Missing_Cert_DER[439];
extern const uint8_t sChipTest_RCAC_Validity_Not_Before_Missing_Cert_DER[404];
extern const uint8_t sChipTest_NOC_Validity_Not_After_Missing_Cert_DER[506];
extern const uint8_t sChipTest_ICAC_Validity_Not_After_Missing_Cert_DER[439];
extern const uint8_t sChipTest_RCAC_Validity_Not_After_Missing_Cert_DER[403];
extern const uint8_t sChipTest_NOC_Validity_Wrong_Cert_DER[519];
extern const uint8_t sChipTest_ICAC_Validity_Wrong_Cert_DER[451];
extern const uint8_t sChipTest_RCAC_Validity_Wrong_Cert_DER[417];
extern const uint8_t sChipTest_NOC_Subject_NodeId_Invalid_Cert_DER[517];
extern const uint8_t sChipTest_NOC_Subject_FabricId_Invalid_Cert_DER[518];
extern const uint8_t sChipTest_ICAC_Subject_FabricId_Invalid_Cert_DER[451];
extern const uint8_t sChipTest_RCAC_Subject_FabricId_Invalid_Cert_DER[416];
extern const uint8_t sChipTest_NOC_Subject_CAT_Invalid_Cert_DER[544];
extern const uint8_t sChipTest_ICAC_Subject_CAT_Invalid_Cert_DER[477];
extern const uint8_t sChipTest_RCAC_Subject_CAT_Invalid_Cert_DER[470];
extern const uint8_t sChipTest_NOC_Sig_Curve_Secp256k1_Cert_DER[515];
extern const uint8_t sChipTest_ICAC_Sig_Curve_Secp256k1_Cert_DER[448];
extern const uint8_t sChipTest_RCAC_Sig_Curve_Secp256k1_Cert_DER[414];
extern const uint8_t sChipTest_NOC_Ext_Basic_Critical_Missing_Cert_DER[513];
extern const uint8_t sChipTest_ICAC_Ext_Basic_Critical_Missing_Cert_DER[447];
extern const uint8_t sChipTest_RCAC_Ext_Basic_Critical_Missing_Cert_DER[414];
extern const uint8_t sChipTest_NOC_Ext_Basic_Critical_Wrong_Cert_DER[513];
extern const uint8_t sChipTest_ICAC_Ext_Basic_Critical_Wrong_Cert_DER[448];
extern const uint8_t sChipTest_RCAC_Ext_Basic_Critical_Wrong_Cert_DER[414];
extern const uint8_t sChipTest_NOC_Ext_Basic_CA_Missing_Cert_DER[522];
extern const uint8_t sChipTest_ICAC_Ext_Basic_CA_Missing_Cert_DER[450];
extern const uint8_t sChipTest_RCAC_Ext_Basic_CA_Missing_Cert_DER[417];
extern const uint8_t sChipTest_NOC_Ext_Basic_PathLen_Presence_Wrong_Cert_DER[520];
extern const uint8_t sChipTest_NOC_Ext_KeyUsage_Critical_Missing_Cert_DER[512];
extern const uint8_t sChipTest_ICAC_Ext_KeyUsage_Critical_Missing_Cert_DER[448];
extern const uint8_t sChipTest_RCAC_Ext_KeyUsage_Critical_Missing_Cert_DER[414];
extern const uint8_t sChipTest_NOC_Ext_KeyUsage_Critical_Wrong_Cert_DER[513];
extern const uint8_t sChipTest_ICAC_Ext_KeyUsage_Critical_Wrong_Cert_DER[448];
extern const uint8_t sChipTest_RCAC_Ext_KeyUsage_Critical_Wrong_Cert_DER[414];
extern const uint8_t sChipTest_NOC_Ext_AKID_Length_Invalid_Cert_DER[516];
extern const uint8_t sChipTest_ICAC_Ext_AKID_Length_Invalid_Cert_DER[450];
extern const uint8_t sChipTest_RCAC_Ext_AKID_Length_Invalid_Cert_DER[415];
extern const uint8_t sChipTest_NOC_Ext_SKID_Length_Invalid_Cert_DER[515];
extern const uint8_t sChipTest_ICAC_Ext_SKID_Length_Invalid_Cert_DER[449];
extern const uint8_t sChipTest_RCAC_Ext_SKID_Length_Invalid_Cert_DER[414];
extern const uint8_t sChipTest_NOC_Oversized_Cert_CHIP[480];
extern const uint8_t sChipTest_ICAC_Oversized_Cert_CHIP[463];
extern const uint8_t sChipTest_RCAC_Oversized_Cert_CHIP[654];
extern const uint8_t sChipTest_NOC_Serial_Number_Missing_Cert_CHIP[268];
extern const uint8_t sChipTest_ICAC_Serial_Number_Missing_Cert_CHIP[251];
extern const uint8_t sChipTest_RCAC_Serial_Number_Missing_Cert_CHIP[241];
extern const uint8_t sChipTest_NOC_Sig_Algo_ECDSA_With_SHA1_Cert_CHIP[279];
extern const uint8_t sChipTest_ICAC_Sig_Algo_ECDSA_With_SHA1_Cert_CHIP[262];
extern const uint8_t sChipTest_RCAC_Sig_Algo_ECDSA_With_SHA1_Cert_CHIP[252];
extern const uint8_t sChipTest_NOC_Issuer_Missing_Cert_CHIP[256];
extern const uint8_t sChipTest_ICAC_Issuer_Missing_Cert_CHIP[249];
extern const uint8_t sChipTest_RCAC_Issuer_Missing_Cert_CHIP[239];
extern const uint8_t sChipTest_NOC_Validity_Not_Before_Missing_Cert_CHIP[273];
extern const uint8_t sChipTest_ICAC_Validity_Not_Before_Missing_Cert_CHIP[256];
extern const uint8_t sChipTest_RCAC_Validity_Not_Before_Missing_Cert_CHIP[246];
extern const uint8_t sChipTest_NOC_Validity_Not_After_Missing_Cert_CHIP[273];
extern const uint8_t sChipTest_ICAC_Validity_Not_After_Missing_Cert_CHIP[256];
extern const uint8_t sChipTest_RCAC_Validity_Not_After_Missing_Cert_CHIP[246];
extern const uint8_t sChipTest_NOC_Validity_Wrong_Cert_CHIP[279];
extern const uint8_t sChipTest_ICAC_Validity_Wrong_Cert_CHIP[262];
extern const uint8_t sChipTest_RCAC_Validity_Wrong_Cert_CHIP[252];
extern const uint8_t sChipTest_NOC_Subject_Missing_Cert_CHIP[256];
extern const uint8_t sChipTest_ICAC_Subject_Missing_Cert_CHIP[239];
extern const uint8_t sChipTest_RCAC_Subject_Missing_Cert_CHIP[239];
extern const uint8_t sChipTest_NOC_Subject_MatterId_Missing_Cert_CHIP[269];
extern const uint8_t sChipTest_ICAC_Subject_MatterId_Missing_Cert_CHIP[252];
extern const uint8_t sChipTest_RCAC_Subject_MatterId_Missing_Cert_CHIP[232];
extern const uint8_t sChipTest_NOC_Subject_NodeId_Invalid_Cert_CHIP[279];
extern const uint8_t sChipTest_NOC_Subject_MatterId_Twice_Cert_CHIP[289];
extern const uint8_t sChipTest_ICAC_Subject_MatterId_Twice_Cert_CHIP[272];
extern const uint8_t sChipTest_RCAC_Subject_MatterId_Twice_Cert_CHIP[272];
extern const uint8_t sChipTest_NOC_Subject_FabricId_Missing_Cert_CHIP[269];
extern const uint8_t sChipTest_NOC_Subject_FabricId_Invalid_Cert_CHIP[272];
extern const uint8_t sChipTest_ICAC_Subject_FabricId_Invalid_Cert_CHIP[255];
extern const uint8_t sChipTest_RCAC_Subject_FabricId_Invalid_Cert_CHIP[238];
extern const uint8_t sChipTest_NOC_Subject_FabricId_Twice_Cert_CHIP[289];
extern const uint8_t sChipTest_ICAC_Subject_FabricId_Twice_Cert_CHIP[272];
extern const uint8_t sChipTest_RCAC_Subject_FabricId_Twice_Cert_CHIP[272];
extern const uint8_t sChipTest_NOC_Subject_CAT_Invalid_Cert_CHIP[285];
extern const uint8_t sChipTest_ICAC_Subject_CAT_Invalid_Cert_CHIP[268];
extern const uint8_t sChipTest_RCAC_Subject_CAT_Invalid_Cert_CHIP[264];
extern const uint8_t sChipTest_NOC_Subject_CAT_Twice_Cert_CHIP[291];
extern const uint8_t sChipTest_ICAC_Subject_CAT_Twice_Cert_CHIP[274];
extern const uint8_t sChipTest_RCAC_Subject_CAT_Twice_Cert_CHIP[276];
extern const uint8_t sChipTest_NOC_Sig_Curve_Secp256k1_Cert_CHIP[279];
extern const uint8_t sChipTest_ICAC_Sig_Curve_Secp256k1_Cert_CHIP[262];
extern const uint8_t sChipTest_RCAC_Sig_Curve_Secp256k1_Cert_CHIP[252];
extern const uint8_t sChipTest_NOC_PublicKey_Wrong_Cert_CHIP[279];
extern const uint8_t sChipTest_ICAC_PublicKey_Wrong_Cert_CHIP[262];
extern const uint8_t sChipTest_RCAC_PublicKey_Wrong_Cert_CHIP[252];
extern const uint8_t sChipTest_NOC_Ext_Basic_Missing_Cert_CHIP[274];
extern const uint8_t sChipTest_ICAC_Ext_Basic_Missing_Cert_CHIP[257];
extern const uint8_t sChipTest_RCAC_Ext_Basic_Missing_Cert_CHIP[247];
extern const uint8_t sChipTest_NOC_Ext_Basic_CA_Missing_Cert_CHIP[277];
extern const uint8_t sChipTest_ICAC_Ext_Basic_CA_Missing_Cert_CHIP[260];
extern const uint8_t sChipTest_RCAC_Ext_Basic_CA_Missing_Cert_CHIP[250];
extern const uint8_t sChipTest_NOC_Ext_Basic_CA_Wrong_Cert_CHIP[279];
extern const uint8_t sChipTest_ICAC_Ext_Basic_CA_Wrong_Cert_CHIP[262];
extern const uint8_t sChipTest_RCAC_Ext_Basic_CA_Wrong_Cert_CHIP[252];
extern const uint8_t sChipTest_NOC_Ext_Basic_PathLen_Presence_Wrong_Cert_CHIP[279];
extern const uint8_t sChipTest_ICAC_Ext_Basic_PathLen_Presence_Wrong_Cert_CHIP[262];
extern const uint8_t sChipTest_RCAC_Ext_Basic_PathLen_Presence_Wrong_Cert_CHIP[252];
extern const uint8_t sChipTest_NOC_Ext_Basic_PathLen2_Cert_CHIP[279];
extern const uint8_t sChipTest_ICAC_Ext_Basic_PathLen2_Cert_CHIP[262];
extern const uint8_t sChipTest_RCAC_Ext_Basic_PathLen2_Cert_CHIP[252];
extern const uint8_t sChipTest_NOC_Ext_KeyUsage_Missing_Cert_CHIP[276];
extern const uint8_t sChipTest_ICAC_Ext_KeyUsage_Missing_Cert_CHIP[259];
extern const uint8_t sChipTest_RCAC_Ext_KeyUsage_Missing_Cert_CHIP[249];
extern const uint8_t sChipTest_NOC_Ext_KeyUsage_KeyCertSign_Wrong_Cert_CHIP[279];
extern const uint8_t sChipTest_ICAC_Ext_KeyUsage_KeyCertSign_Wrong_Cert_CHIP[262];
extern const uint8_t sChipTest_RCAC_Ext_KeyUsage_KeyCertSign_Wrong_Cert_CHIP[252];
extern const uint8_t sChipTest_NOC_Ext_AKID_Missing_Cert_CHIP[256];
extern const uint8_t sChipTest_ICAC_Ext_AKID_Missing_Cert_CHIP[239];
extern const uint8_t sChipTest_RCAC_Ext_AKID_Missing_Cert_CHIP[229];
extern const uint8_t sChipTest_NOC_Ext_AKID_Length_Invalid_Cert_CHIP[278];
extern const uint8_t sChipTest_ICAC_Ext_AKID_Length_Invalid_Cert_CHIP[261];
extern const uint8_t sChipTest_RCAC_Ext_AKID_Length_Invalid_Cert_CHIP[251];
extern const uint8_t sChipTest_NOC_Ext_SKID_Missing_Cert_CHIP[256];
extern const uint8_t sChipTest_ICAC_Ext_SKID_Missing_Cert_CHIP[239];
extern const uint8_t sChipTest_RCAC_Ext_SKID_Missing_Cert_CHIP[229];
extern const uint8_t sChipTest_NOC_Ext_SKID_Length_Invalid_Cert_CHIP[278];
extern const uint8_t sChipTest_ICAC_Ext_SKID_Length_Invalid_Cert_CHIP[261];
extern const uint8_t sChipTest_RCAC_Ext_SKID_Length_Invalid_Cert_CHIP[251];
extern const uint8_t sChipTest_NOC_Signature_Wrong_Cert_CHIP[279];
extern const uint8_t sChipTest_ICAC_Signature_Wrong_Cert_CHIP[262];
extern const uint8_t sChipTest_RCAC_Signature_Wrong_Cert_CHIP[252];

} // namespace TestCerts
} // namespace chip
