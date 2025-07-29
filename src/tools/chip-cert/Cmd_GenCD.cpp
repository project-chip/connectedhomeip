/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *      This file implements the command handler for the 'chip-cert' tool
 *      that generates a CHIP Certification Declaration.
 *
 */

#include "chip-cert.h"

#include <credentials/CertificationDeclaration.h>

#include <string>

namespace {

using namespace chip;
using namespace chip::ASN1;
using namespace chip::ArgParser;
using namespace chip::Credentials;
using namespace chip::Crypto;
using namespace chip::TLV;

#define CMD_NAME "chip-cert gen-cd"

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg);

// clang-format off
OptionDef gCmdOptionDefs[] =
{
    { "key",                   kArgumentRequired, 'K' },
    { "cert",                  kArgumentRequired, 'C' },
    { "out",                   kArgumentRequired, 'O' },
    { "format-version",        kArgumentRequired, 'f' },
    { "vendor-id",             kArgumentRequired, 'V' },
    { "product-id",            kArgumentRequired, 'p' },
    { "device-type-id",        kArgumentRequired, 'd' },
    { "certificate-id",        kArgumentRequired, 'c' },
    { "security-level",        kArgumentRequired, 'l' },
    { "security-info",         kArgumentRequired, 'i' },
    { "version-number",        kArgumentRequired, 'n' },
    { "certification-type",    kArgumentRequired, 't' },
    { "dac-origin-vendor-id",  kArgumentRequired, 'o' },
    { "dac-origin-product-id", kArgumentRequired, 'r' },
    { "authorized-paa-cert",   kArgumentRequired, 'a' },
#if CHIP_CONFIG_INTERNAL_FLAG_GENERATE_DA_TEST_CASES
    { "ignore-error",          kNoArgument,       'I' },
    { "error-type",            kArgumentRequired, 'E' },
#endif
    { }
};

const char * const gCmdOptionHelp =
    "   -K, --key <file/str>\n"
    "\n"
    "       File or string containing private key to be used to sign the Certification Declaration.\n"
    "\n"
    "   -C, --cert <file/str>\n"
    "\n"
    "       File or string containing certificate associated with the private key that is used\n"
    "       to sign the Certification Declaration. The Subject Key Identifier in the\n"
    "       certificate will be included in the signed Certification Declaration message.\n"
    "\n"
    "   -O, --out <file/stdout>\n"
    "\n"
    "       File to contain the signed Certification Declaration message.\n"
    "       If specified '-' then output is written to stdout.\n"
    "\n"
    "   -f, --format-version <int>\n"
    "\n"
    "       Format Version.\n"
    "\n"
    "   -V, --vendor-id <hex-digits>\n"
    "\n"
    "       Vendor Id (VID) in hex.\n"
    "\n"
    "   -p, --product-id <hex-digits>\n"
    "\n"
    "       Product Id (PID) in hex. Maximum 100 PID values can be specified.\n"
    "       Each PID value should have it's own -p or --product-id option selector.\n"
    "\n"
    "   -d, --device-type-id <hex-digits>\n"
    "\n"
    "       Device Type Id in hex.\n"
    "\n"
    "   -c, --certificate-id <string>\n"
    "\n"
    "       Certificate Id encoded as UTF8 string.\n"
    "\n"
    "   -l, --security-level <hex-digits>\n"
    "\n"
    "       Security Level in hex.\n"
    "\n"
    "   -i, --security-info <hex-digits>\n"
    "\n"
    "       Security Information in hex.\n"
    "\n"
    "   -n, --version-number <hex-digits>\n"
    "\n"
    "       Version Number in hex.\n"
    "\n"
    "   -t, --certification-type <int>\n"
    "\n"
    "       Certification Type. Valid values are:\n"
    "           0 - Development and Test (default)\n"
    "           1 - Provisional\n"
    "           2 - Official\n"
    "\n"
    "   -o, --dac-origin-vendor-id <hex-digits>\n"
    "\n"
    "       DAC Origin Vendor Id in hex.\n"
    "\n"
    "   -r, --dac-origin-product-id <hex-digits>\n"
    "\n"
    "       DAC Origin Product Id in hex.\n"
    "\n"
    "   -a, --authorized-paa-cert <file/str>\n"
    "\n"
    "       File or string containing PAA certificate authorized to sign PAI which signs the DAC\n"
    "       for a product carrying this CD. This field is optional and if present, only specified\n"
    "       PAAs will be authorized to sign device's PAI for the lifetime of the generated CD.\n"
    "       Maximum 10 authorized PAA certificates can be specified.\n"
    "       Each PAA should have its own -a (--authorized-paa-cert) option selector.\n"
    "       The certificate can be in DER or PEM Form.\n"
    "       Note that only the Subject Key Identifier (SKID) value will be extracted\n"
    "       from the PAA certificate and put into CD Structure.\n"
    "\n"
#if CHIP_CONFIG_INTERNAL_FLAG_GENERATE_DA_TEST_CASES
    "   -I, --ignore-error\n"
    "\n"
    "       Ignore some input parameters error.\n"
    "       WARNING: This option makes it possible to circumvent Certification Declaration\n"
    "       structure/parameter requirement. This is required for negative testing of the attestation flow.\n"
    "       Because of this it SHOULD NEVER BE ENABLED IN PRODUCTION BUILDS.\n"
    "\n"
    "   -E, --error-type <error-type>\n"
    "\n"
    "       When specified injects specific error into the structure of generated Certification Declaration.\n"
    "       Note that 'ignore-error' option MUST be specified for this error injection to take effect.\n"
    "       Supported error types that can be injected are:\n"
    "           no-error                           - No error to inject.\n"
    "           format-version-missing             - The CD TLV structure won't have format version field.\n"
    "           format-version-wrong               - Format version will be set to 2 instead of required 1.\n"
    "           vid-missing                        - The CD TLV structure won't have vedor_id field.\n"
    "           vid-mismatch                       - The vendor_id field will have value that doesn't match the VID in DAC.\n"
    "           pid-array-missing                  - The CD TLV structure won't have product_id_array field.\n"
    "           pid-array-count0                   - The product_id_array will be empty.\n"
    "           pid-array-count01-valid            - The product_id_array field will have one valid PID value.\n"
    "           pid-array-count01-mismatch         - The product_id_array field will have one PID value that doesn't match PID in DAC.\n"
    "           pid-array-count10-valid            - The product_id_array field will have 10 PID values one of which is valid matches PID in DAC.\n"
    "           pid-array-count10-mismatch         - The product_id_array field will have 10 PID values none of which matches the PID in DAC.\n"
    "           pid-array-count100-valid           - The product_id_array field will have 100 PID values one of which is valid matches PID in DAC.\n"
    "           pid-array-count100-mismatch        - The product_id_array field will have 100 PID values none of which matches the PID in DAC.\n"
    "           device-type-id-missing             - The CD TLV structure won't have device_type_id field.\n"
    "           device-type-id-mismatch            - device_type_id field won't match the value in the DCL entries associated with the VID and PID.\n"
    "           cert-id-missing                    - The CD TLV structure won't have certificate_id field.\n"
    "           cert-id-mismatch                   - The certificate_id field will contain value NOT allocated by the CSA.\n"
    "           cert-id-len-wrong                  - The certificate_id field will be truncated to have invalid length.\n"
    "           security-level-missing             - The CD TLV structure won't have security_level field.\n"
    "           security-level-wrong               - The security_level field will be set to invalid value (different from 0).\n"
    "           security-info-missing              - The CD TLV structure won't have security_information field.\n"
    "           security-info-wrong                - The security_information field will be set to invalid value (different from 0).\n"
    "           version-number-missing             - The CD TLV structure won't have version_number field.\n"
    "           version-number-wrong               - The version_number field will contain value NOT assigned by the CSA.\n"
    "           cert-type-missing                  - The CD TLV structure won't have certification_type field.\n"
    "           cert-type-wrong                    - The certification_type field will contain invalue value.\n"
    "           dac-origin-vid-present             - The CD TLV structure will include optional dac_origin_vid field.\n"
    "           dac-origin-pid-present             - The CD TLV structure will include optional dac_origin_pid field.\n"
    "           dac-origin-vid-pid-present         - The CD TLV structure will include optional dac_origin_vid and dac_origin_pid fields.\n"
    "           dac-origin-vid-mismatch            - The optional dac_origin_vid field will be present and won't match the VID in DAC.\n"
    "           dac-origin-pid-mismatch            - The optional dac_origin_pid field will be present and won't match the PID in DAC.\n"
    "           authorized-paa-list-count0         - The authorized_paa_list will be empty TLV list.\n"
    "           authorized-paa-list-count1-valid   - The authorized_paa_list will have one valid value.\n"
    "           authorized-paa-list-count2-valid   - The authorized_paa_list will have two elements one of which is valid.\n"
    "           authorized-paa-list-count3-invalid - The authorized_paa_list will have three elements none of which is valid.\n"
    "           authorized-paa-list-count10-valid  - The authorized_paa_list will have ten elements one of which is valid.\n"
    "           authorized-paa-list-count10-invalid- The authorized_paa_list will have ten elements none of which is valid.\n"
    "           signer-info-v2                     - Signer Info version will be set to v2 instead of required v3.\n"
    "           signer-info-digest-algo            - Use Signer Info SHA1 digest algorithm instead of required SHA256.\n"
    "           signer-info-skid                   - Inject error into SKID of a Signer Info signing certificate.\n"
    "           cms-v2                             - CMS version will be set to v2 instead of required v3.\n"
    "           cms-digest-algo                    - Use SHA1 digest algorithm instead of required SHA256.\n"
    "           cms-sig-algo                       - Use ecdsa-with-SHA1 signature algorithm instead of required ecdsa-with-SHA256.\n"
    "                                                required secp256r1 (aka prime256v1).\n"
    "           cms-econtent-type                  - CMS eContentType is set to Microsoft Authenticode [MSAC] ( OID = { 1.3.6.1.4.1.311.2.1.4 } )\n"
    "                                                instead of required pkcs7_data.\n"
    "           cms-sig                            - Inject error into CMS signature.\n"
    "\n"
#endif
    ;

OptionSet gCmdOptions =
{
    HandleOption,
    gCmdOptionDefs,
    "COMMAND OPTIONS",
    gCmdOptionHelp
};

HelpOptions gHelpOptions(
    CMD_NAME,
    "Usage: " CMD_NAME " [ <options...> ]\n",
    CHIP_VERSION_STRING "\n" COPYRIGHT_STRING,
    "Generate CD CMS Signed Message"
);

OptionSet *gCmdOptionSets[] =
{
    &gCmdOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

/** Certification Declaration Error and Configuration Flags
 *
 * By default all methods (if none of the class setters were used) return valid
 * certification declaration configuration parameter as described in the spec.
 * These parameters can be modified to inject errors into cd structure.
 */
class CDStructConfig
{
public:
    void EnableErrorTestCase() { mEnabled = true; }
    void SetFormatVersionMissing() { mFlags.Set(CDConfigFlags::kFormatVersionMissing); }
    void SetFormatVersionWrong() { mFlags.Set(CDConfigFlags::kFormatVersionWrong); }
    void SetVIDMissing() { mFlags.Set(CDConfigFlags::kVIDMissing); }
    void SetVIDWrong() { mFlags.Set(CDConfigFlags::kVIDWrong); }
    void SetPIDArrayMissing() { mFlags.Set(CDConfigFlags::kPIDArrayMissing); }
    void SetPIDArrayWrong() { mFlags.Set(CDConfigFlags::kPIDArrayWrong); }
    void SetPIDArrayCount(uint8_t pidArrayCount) { mPIDArrayCount = pidArrayCount; }
    void SetDeviceTypeIdMissing() { mFlags.Set(CDConfigFlags::kDeviceTypeIdMissing); }
    void SetDeviceTypeIdWrong() { mFlags.Set(CDConfigFlags::kDeviceTypeIdWrong); }
    void SetCertIdMissing() { mFlags.Set(CDConfigFlags::kCertIdMissing); }
    void SetCertIdWrong() { mFlags.Set(CDConfigFlags::kCertIdWrong); }
    void SetCertIdLenWrong() { mFlags.Set(CDConfigFlags::kCertIdLenWrong); }
    void SetSecurityLevelMissing() { mFlags.Set(CDConfigFlags::kSecurityLevelMissing); }
    void SetSecurityLevelWrong() { mFlags.Set(CDConfigFlags::kSecurityLevelWrong); }
    void SetSecurityInfoMissing() { mFlags.Set(CDConfigFlags::kSecurityInfoMissing); }
    void SetSecurityInfoWrong() { mFlags.Set(CDConfigFlags::kSecurityInfoWrong); }
    void SetVersionNumberMissing() { mFlags.Set(CDConfigFlags::kVersionNumberMissing); }
    void SetVersionNumberWrong() { mFlags.Set(CDConfigFlags::kVersionNumberWrong); }
    void SetCertTypeMissing() { mFlags.Set(CDConfigFlags::kCertTypeMissing); }
    void SetCertTypeWrong() { mFlags.Set(CDConfigFlags::kCertTypeWrong); }
    void SetDACOriginVIDWrong() { mFlags.Set(CDConfigFlags::kDACOriginVID); }
    void SetDACOriginPIDWrong() { mFlags.Set(CDConfigFlags::kDACOriginPID); }
    void SetDACOriginVIDPresent() { mFlags.Set(CDConfigFlags::kDACOriginVIDPresent); }
    void SetDACOriginPIDPresent() { mFlags.Set(CDConfigFlags::kDACOriginPIDPresent); }
    void SetAuthPAAListPresent() { mFlags.Set(CDConfigFlags::kAuthPAAListPresent); }
    void SetAuthPAAListWrong() { mFlags.Set(CDConfigFlags::kAuthPAAListWrong); }
    void SetAuthPAAListCount(uint8_t authPAAListCount) { mAuthPAAListCount = authPAAListCount; }
    void SetSignerInfoVersionWrong() { mFlags.Set(CDConfigFlags::kSignerInfoVersion); }
    void SetSignerInfoDigestAlgoWrong() { mFlags.Set(CDConfigFlags::kSignerInfoDigestAlgo); }
    void SetSignerInfoSKIDWrong() { mFlags.Set(CDConfigFlags::kSignerInfoSKID); }
    void SetCMSVersionWrong() { mFlags.Set(CDConfigFlags::kCMSVersion); }
    void SetCMSDigestAlgoWrong() { mFlags.Set(CDConfigFlags::kCMSDigestAlgo); }
    void SetCMSSigAlgoWrong() { mFlags.Set(CDConfigFlags::kCMSSigAlgo); }
    void SetCMSEContentTypeWrong() { mFlags.Set(CDConfigFlags::kCMSEContentType); }
    void SetCMSSignatureWrong() { mFlags.Set(CDConfigFlags::kCMSSignature); }

    bool IsErrorTestCaseEnabled() { return mEnabled; }
    bool IsFormatVersionPresent() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kFormatVersionMissing)); }
    uint8_t GetFormatVersion() { return (mEnabled && mFlags.Has(CDConfigFlags::kFormatVersionWrong)) ? 2 : 1; }
    bool IsVIDPresent() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kVIDMissing)); }
    bool IsVIDCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kVIDWrong)); }
    bool IsPIDArrayPresent() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kPIDArrayMissing)); }
    bool IsPIDArrayCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kPIDArrayWrong)); }
    uint8_t GetPIDArrayCount() const { return mPIDArrayCount; }
    bool IsDeviceTypeIdPresent() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kDeviceTypeIdMissing)); }
    bool IsDeviceTypeIdCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kDeviceTypeIdWrong)); }
    bool IsCertIdPresent() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kCertIdMissing)); }
    bool IsCertIdCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kCertIdWrong)); }
    bool IsCertIdLenCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kCertIdLenWrong)); }
    bool IsSecurityLevelPresent() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kSecurityLevelMissing)); }
    bool IsSecurityLevelCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kSecurityLevelWrong)); }
    bool IsSecurityInfoPresent() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kSecurityInfoMissing)); }
    bool IsSecurityInfoCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kSecurityInfoWrong)); }
    bool IsVersionNumberPresent() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kVersionNumberMissing)); }
    bool IsVersionNumberCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kVersionNumberWrong)); }
    bool IsCertTypePresent() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kCertTypeMissing)); }
    bool IsCertTypeCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kCertTypeWrong)); }
    bool IsDACOriginVIDCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kDACOriginVID)); }
    bool IsDACOriginPIDCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kDACOriginPID)); }
    bool IsDACOriginVIDPresent() { return (mEnabled && mFlags.Has(CDConfigFlags::kDACOriginVIDPresent)); }
    bool IsDACOriginPIDPresent() { return (mEnabled && mFlags.Has(CDConfigFlags::kDACOriginPIDPresent)); }
    bool IsAuthPAAListPresent() { return (mFlags.Has(CDConfigFlags::kAuthPAAListPresent)); }
    bool IsAuthPAAListCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kAuthPAAListWrong)); }
    uint8_t GetAuthPAAListCount() const { return mAuthPAAListCount; }
    bool IsSignerInfoVersionCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kSignerInfoVersion)); }
    bool IsSignerInfoDigestAlgoCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kSignerInfoDigestAlgo)); }
    bool IsSignerInfoSKIDCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kSignerInfoSKID)); }
    bool IsCMSVersionCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kCMSVersion)); }
    bool IsCMSDigestAlgoCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kCMSDigestAlgo)); }
    bool IsCMSSigAlgoCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kCMSSigAlgo)); }
    bool IsCMSEContentTypeCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kCMSEContentType)); }
    bool IsCMSSignatureCorrect() { return (!mEnabled || !mFlags.Has(CDConfigFlags::kCMSSignature)); }

private:
    enum class CDConfigFlags : uint64_t
    {
        kFormatVersionMissing = 0x0000000000000001,
        kFormatVersionWrong   = 0x0000000000000002,
        kVIDMissing           = 0x0000000000000004,
        kVIDWrong             = 0x0000000000000008,
        kPIDArrayMissing      = 0x0000000000000010,
        kPIDArrayWrong        = 0x0000000000000020,
        kDeviceTypeIdMissing  = 0x0000000000000040,
        kDeviceTypeIdWrong    = 0x0000000000000080,
        kCertIdMissing        = 0x0000000000000100,
        kCertIdWrong          = 0x0000000000000200,
        kCertIdLenWrong       = 0x0000000000000400,
        kSecurityLevelMissing = 0x0000000000000800,
        kSecurityLevelWrong   = 0x0000000000001000,
        kSecurityInfoMissing  = 0x0000000000002000,
        kSecurityInfoWrong    = 0x0000000000004000,
        kVersionNumberMissing = 0x0000000000008000,
        kVersionNumberWrong   = 0x0000000000010000,
        kCertTypeMissing      = 0x0000000000020000,
        kCertTypeWrong        = 0x0000000000040000,
        kDACOriginVID         = 0x0000000000080000,
        kDACOriginPID         = 0x0000000000100000,
        kDACOriginVIDPresent  = 0x0000000000200000,
        kDACOriginPIDPresent  = 0x0000000000400000,
        kAuthPAAListPresent   = 0x0000000000800000,
        kAuthPAAListWrong     = 0x0000000001000000,
        kSignerInfoVersion    = 0x0000000002000000,
        kSignerInfoDigestAlgo = 0x0000000004000000,
        kSignerInfoSKID       = 0x0000000008000000,
        kCMSVersion           = 0x0000000010000000,
        kCMSDigestAlgo        = 0x0000000020000000,
        kCMSSigAlgo           = 0x0000000040000000,
        kCMSEContentType      = 0x0000000080000000,
        kCMSSignature         = 0x0000000100000000,
    };

    bool mEnabled = false;
    chip::BitFlags<CDConfigFlags> mFlags;
    uint8_t mPIDArrayCount    = 1;
    uint8_t mAuthPAAListCount = 1;
};

CertificationElements gCertElements;
const char * gCertFileNameOrStr = nullptr;
const char * gKeyFileNameOrStr  = nullptr;
const char * gSignedCDFileName  = nullptr;
CDStructConfig gCDConfig;

bool ExtractSKIDFromX509Cert(X509 * cert, ByteSpan & skid)
{
    const ASN1_OCTET_STRING * skidString = X509_get0_subject_key_id(cert);
    VerifyOrReturnError(skidString != nullptr, false);
    VerifyOrReturnError(skidString->length == kKeyIdentifierLength, false);
    VerifyOrReturnError(CanCastTo<size_t>(skidString->length), false);
    skid = ByteSpan(skidString->data, static_cast<size_t>(skidString->length));
    return true;
};

bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'C':
        gCertFileNameOrStr = arg;
        break;
    case 'K':
        gKeyFileNameOrStr = arg;
        break;
    case 'O':
        gSignedCDFileName = arg;
        break;
    case 'f':
        if (!ParseInt(arg, gCertElements.FormatVersion, 16))
        {
            PrintArgError("%s: Invalid value specified for Format Version: %s\n", progName, arg);
            return false;
        }
        break;
    case 'V':
        if (!ParseInt(arg, gCertElements.VendorId, 16) || gCertElements.VendorId == 0)
        {
            PrintArgError("%s: Invalid value specified for Vendor Id: %s\n", progName, arg);
            return false;
        }
        break;
    case 'p':
        if (gCertElements.ProductIdsCount == MATTER_ARRAY_SIZE(gCertElements.ProductIds))
        {
            PrintArgError("%s: Too many Product Ids are specified: %s\n", progName, arg);
            return false;
        }
        if (!ParseInt(arg, gCertElements.ProductIds[gCertElements.ProductIdsCount], 16) ||
            gCertElements.ProductIds[gCertElements.ProductIdsCount] == 0)
        {
            PrintArgError("%s: Invalid value specified for Product Id: %s\n", progName, arg);
            return false;
        }
        gCertElements.ProductIdsCount++;
        break;
    case 'd':
        if (!ParseInt(arg, gCertElements.DeviceTypeId, 16))
        {
            PrintArgError("%s: Invalid value specified for Device Type Id: %s\n", progName, arg);
            return false;
        }
        break;
    case 'c':
        if (strlen(arg) != kCertificateIdLength)
        {
            PrintArgError("%s: Invalid value specified for Certificate Id: %s\n", progName, arg);
            return false;
        }
        memcpy(gCertElements.CertificateId, arg, strlen(arg));
        gCertElements.CertificateId[kCertificateIdLength] = '\0';
        break;
    case 'l':
        if (!ParseInt(arg, gCertElements.SecurityLevel, 16))
        {
            PrintArgError("%s: Invalid value specified for Security Level: %s\n", progName, arg);
            return false;
        }
        break;
    case 'i':
        if (!ParseInt(arg, gCertElements.SecurityInformation, 16))
        {
            PrintArgError("%s: Invalid value specified for Security Information: %s\n", progName, arg);
            return false;
        }
        break;
    case 'n':
        if (!ParseInt(arg, gCertElements.VersionNumber, 16))
        {
            PrintArgError("%s: Invalid value specified for Version Number: %s\n", progName, arg);
            return false;
        }
        break;
    case 't':
        if (!ParseInt(arg, gCertElements.CertificationType) || gCertElements.CertificationType > 2)
        {
            PrintArgError("%s: Invalid value specified for Certification Type: %s\n", progName, arg);
            return false;
        }
        break;
    case 'o':
        if (!ParseInt(arg, gCertElements.DACOriginVendorId, 16) || gCertElements.DACOriginVendorId == 0)
        {
            PrintArgError("%s: Invalid value specified for DAC Origin Vendor Id: %s\n", progName, arg);
            return false;
        }
        gCertElements.DACOriginVIDandPIDPresent = true;
        break;
    case 'r':
        if (!ParseInt(arg, gCertElements.DACOriginProductId, 16) || gCertElements.DACOriginProductId == 0)
        {
            PrintArgError("%s: Invalid value specified for DAC Origin Product Id: %s\n", progName, arg);
            return false;
        }
        gCertElements.DACOriginVIDandPIDPresent = true;
        break;
    case 'a':
        if (gCertElements.AuthorizedPAAListCount >= MATTER_ARRAY_SIZE(gCertElements.AuthorizedPAAList))
        {
            PrintArgError("%s: Too many Authorized PAA Certificates are specified: %s\n", progName, arg);
            return false;
        }
        {
            const char * fileNameOrStr = arg;
            std::unique_ptr<X509, void (*)(X509 *)> cert(nullptr, &X509_free);
            VerifyOrReturnError(ReadCert(fileNameOrStr, cert), false);

            ByteSpan skid;
            VerifyOrReturnError(ExtractSKIDFromX509Cert(cert.get(), skid), false);
            memcpy(gCertElements.AuthorizedPAAList[gCertElements.AuthorizedPAAListCount++], skid.data(), skid.size());
        }
        break;
#if CHIP_CONFIG_INTERNAL_FLAG_GENERATE_DA_TEST_CASES
    case 'I':
        gCDConfig.EnableErrorTestCase();
        break;
    case 'E':
        if (strcmp(arg, "format-version-missing") == 0)
        {
            gCDConfig.SetFormatVersionMissing();
        }
        else if (strcmp(arg, "format-version-wrong") == 0)
        {
            gCDConfig.SetFormatVersionWrong();
        }
        else if (strcmp(arg, "vid-missing") == 0)
        {
            gCDConfig.SetVIDMissing();
        }
        else if (strcmp(arg, "vid-mismatch") == 0)
        {
            gCDConfig.SetVIDWrong();
        }
        else if (strcmp(arg, "pid-array-missing") == 0)
        {
            gCDConfig.SetPIDArrayMissing();
        }
        else if (strcmp(arg, "pid-array-count0") == 0)
        {
            gCDConfig.SetPIDArrayCount(0);
        }
        else if (strcmp(arg, "pid-array-count01-valid") == 0)
        {
            gCDConfig.SetPIDArrayCount(1);
        }
        else if (strcmp(arg, "pid-array-count01-mismatch") == 0)
        {
            gCDConfig.SetPIDArrayCount(1);
            gCDConfig.SetPIDArrayWrong();
        }
        else if (strcmp(arg, "pid-array-count10-valid") == 0)
        {
            gCDConfig.SetPIDArrayCount(10);
        }
        else if (strcmp(arg, "pid-array-count10-mismatch") == 0)
        {
            gCDConfig.SetPIDArrayCount(10);
            gCDConfig.SetPIDArrayWrong();
        }
        else if (strcmp(arg, "pid-array-count100-valid") == 0)
        {
            gCDConfig.SetPIDArrayCount(100);
        }
        else if (strcmp(arg, "pid-array-count100-mismatch") == 0)
        {
            gCDConfig.SetPIDArrayCount(100);
            gCDConfig.SetPIDArrayWrong();
        }
        else if (strcmp(arg, "device-type-id-missing") == 0)
        {
            gCDConfig.SetDeviceTypeIdMissing();
        }
        else if (strcmp(arg, "device-type-id-mismatch") == 0)
        {
            gCDConfig.SetDeviceTypeIdWrong();
        }
        else if (strcmp(arg, "cert-id-missing") == 0)
        {
            gCDConfig.SetCertIdMissing();
        }
        else if (strcmp(arg, "cert-id-mismatch") == 0)
        {
            gCDConfig.SetCertIdWrong();
        }
        else if (strcmp(arg, "cert-id-len-wrong") == 0)
        {
            gCDConfig.SetCertIdLenWrong();
        }
        else if (strcmp(arg, "security-level-missing") == 0)
        {
            gCDConfig.SetSecurityLevelMissing();
        }
        else if (strcmp(arg, "security-level-wrong") == 0)
        {
            gCDConfig.SetSecurityLevelWrong();
        }
        else if (strcmp(arg, "security-info-missing") == 0)
        {
            gCDConfig.SetSecurityInfoMissing();
        }
        else if (strcmp(arg, "security-info-wrong") == 0)
        {
            gCDConfig.SetSecurityInfoWrong();
        }
        else if (strcmp(arg, "version-number-missing") == 0)
        {
            gCDConfig.SetVersionNumberMissing();
        }
        else if (strcmp(arg, "version-number-wrong") == 0)
        {
            gCDConfig.SetVersionNumberWrong();
        }
        else if (strcmp(arg, "cert-type-missing") == 0)
        {
            gCDConfig.SetCertTypeMissing();
        }
        else if (strcmp(arg, "cert-type-wrong") == 0)
        {
            gCDConfig.SetCertTypeWrong();
        }
        else if (strcmp(arg, "dac-origin-vid-present") == 0)
        {
            gCDConfig.SetDACOriginVIDPresent();
        }
        else if (strcmp(arg, "dac-origin-pid-present") == 0)
        {
            gCDConfig.SetDACOriginPIDPresent();
        }
        else if (strcmp(arg, "dac-origin-vid-pid-present") == 0)
        {
            gCDConfig.SetDACOriginVIDPresent();
            gCDConfig.SetDACOriginPIDPresent();
        }
        else if (strcmp(arg, "dac-origin-vid-mismatch") == 0)
        {
            gCDConfig.SetDACOriginVIDPresent();
            gCDConfig.SetDACOriginPIDPresent();
            gCDConfig.SetDACOriginVIDWrong();
        }
        else if (strcmp(arg, "dac-origin-pid-mismatch") == 0)
        {
            gCDConfig.SetDACOriginVIDPresent();
            gCDConfig.SetDACOriginPIDPresent();
            gCDConfig.SetDACOriginPIDWrong();
        }
        else if (strcmp(arg, "different-origin") == 0)
        {
            gCDConfig.SetDACOriginVIDPresent();
            gCDConfig.SetDACOriginPIDPresent();
        }
        else if (strcmp(arg, "authorized-paa-list-count0") == 0)
        {
            gCDConfig.SetAuthPAAListPresent();
            gCDConfig.SetAuthPAAListCount(0);
        }
        else if (strcmp(arg, "authorized-paa-list-count1-valid") == 0)
        {
            gCDConfig.SetAuthPAAListPresent();
            gCDConfig.SetAuthPAAListCount(1);
        }
        else if (strcmp(arg, "authorized-paa-list-count2-valid") == 0)
        {
            gCDConfig.SetAuthPAAListPresent();
            gCDConfig.SetAuthPAAListCount(2);
        }
        else if (strcmp(arg, "authorized-paa-list-count3-invalid") == 0)
        {
            gCDConfig.SetAuthPAAListCount(3);
            gCDConfig.SetAuthPAAListPresent();
            gCDConfig.SetAuthPAAListWrong();
        }
        else if (strcmp(arg, "authorized-paa-list-count10-valid") == 0)
        {
            gCDConfig.SetAuthPAAListPresent();
            gCDConfig.SetAuthPAAListCount(10);
        }
        else if (strcmp(arg, "authorized-paa-list-count10-invalid") == 0)
        {
            gCDConfig.SetAuthPAAListCount(10);
            gCDConfig.SetAuthPAAListPresent();
            gCDConfig.SetAuthPAAListWrong();
        }
        else if (strcmp(arg, "signer-info-v2") == 0)
        {
            gCDConfig.SetSignerInfoVersionWrong();
        }
        else if (strcmp(arg, "signer-info-digest-algo") == 0)
        {
            gCDConfig.SetSignerInfoDigestAlgoWrong();
        }
        else if (strcmp(arg, "signer-info-skid") == 0)
        {
            gCDConfig.SetSignerInfoSKIDWrong();
        }
        else if (strcmp(arg, "cms-v2") == 0)
        {
            gCDConfig.SetCMSVersionWrong();
        }
        else if (strcmp(arg, "cms-digest-algo") == 0)
        {
            gCDConfig.SetCMSDigestAlgoWrong();
        }
        else if (strcmp(arg, "cms-sig-algo") == 0)
        {
            gCDConfig.SetCMSSigAlgoWrong();
        }
        else if (strcmp(arg, "cms-econtent-type") == 0)
        {
            gCDConfig.SetCMSEContentTypeWrong();
        }
        else if (strcmp(arg, "cms-sig") == 0)
        {
            gCDConfig.SetCMSSignatureWrong();
        }
        else if (strcmp(arg, "no-error") != 0)
        {
            PrintArgError("%s: Invalid value specified for the error type: %s\n", progName, arg);
            return false;
        }
        break;
#endif
    default:
        PrintArgError("%s: Unhandled option: %s\n", progName, name);
        return false;
    }

    return true;
}

static constexpr uint8_t sOID_ContentType_PKCS7Data[] = { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x01 };
static constexpr uint8_t sOID_ContentType_MSAC[]      = { 0x06, 0x0A, 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x02, 0x01, 0x04 };
static constexpr uint8_t sOID_ContentType_PKCS7SignedData[] = { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x02 };
static constexpr uint8_t sOID_DigestAlgo_SHA256[]           = { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01 };
static constexpr uint8_t sOID_DigestAlgo_SHA1[]             = { 0x2B, 0x0E, 0x03, 0x02, 0x1A };
static constexpr uint8_t sOID_SigAlgo_ECDSAWithSHA1[]       = { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x01 };

/** Certification Declaration Element TLV Tags
 */
enum
{
    kTag_FormatVersion       = 0,  /**< [ unsigned int ] Format version. */
    kTag_VendorId            = 1,  /**< [ unsigned int ] Vedor identifier. */
    kTag_ProductIdArray      = 2,  /**< [ array ] Product identifiers (each is unsigned int). */
    kTag_DeviceTypeId        = 3,  /**< [ unsigned int ] Device Type identifier. */
    kTag_CertificateId       = 4,  /**< [ UTF-8 string, length 19 ] Certificate identifier. */
    kTag_SecurityLevel       = 5,  /**< [ unsigned int ] Security level. */
    kTag_SecurityInformation = 6,  /**< [ unsigned int ] Security information. */
    kTag_VersionNumber       = 7,  /**< [ unsigned int ] Version number. */
    kTag_CertificationType   = 8,  /**< [ unsigned int ] Certification Type. */
    kTag_DACOriginVendorId   = 9,  /**< [ unsigned int, optional ] DAC origin vendor identifier. */
    kTag_DACOriginProductId  = 10, /**< [ unsigned int, optional ] DAC origin product identifier. */
    kTag_AuthorizedPAAList   = 11, /**< [ array, optional ] Authorized PAA List. */
};

CHIP_ERROR EncodeCertificationElements_Ignore_Error(const CertificationElements & certElements,
                                                    MutableByteSpan & encodedCertElements, CDStructConfig & cdConfig)
{
    TLVWriter writer;
    TLVType outerContainer1, outerContainer2;

    writer.Init(encodedCertElements);

    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainer1));

    if (cdConfig.IsFormatVersionPresent())
    {
        ReturnErrorOnFailure(writer.Put(ContextTag(kTag_FormatVersion), cdConfig.GetFormatVersion()));
    }
    if (cdConfig.IsVIDPresent())
    {
        if (cdConfig.IsVIDCorrect())
        {
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_VendorId), certElements.VendorId));
        }
        else
        {
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_VendorId), static_cast<uint16_t>(certElements.VendorId ^ UINT16_MAX)));
        }
    }
    if (cdConfig.IsPIDArrayPresent())
    {
        ReturnErrorOnFailure(writer.StartContainer(ContextTag(kTag_ProductIdArray), kTLVType_Array, outerContainer2));
        uint16_t pid =
            cdConfig.IsPIDArrayCorrect() ? certElements.ProductIds[0] : static_cast<uint16_t>(certElements.ProductIds[0] + 1);
        for (uint8_t i = 0; i < cdConfig.GetPIDArrayCount(); i++)
        {
            ReturnErrorOnFailure(writer.Put(AnonymousTag(), static_cast<uint16_t>(pid++)));
        }
        ReturnErrorOnFailure(writer.EndContainer(outerContainer2));
    }
    if (cdConfig.IsDeviceTypeIdPresent())
    {
        if (cdConfig.IsDeviceTypeIdCorrect())
        {
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_DeviceTypeId), certElements.DeviceTypeId));
        }
        else
        {
            ReturnErrorOnFailure(
                writer.Put(ContextTag(kTag_DeviceTypeId), static_cast<uint32_t>(certElements.DeviceTypeId ^ UINT32_MAX)));
        }
    }
    if (cdConfig.IsCertIdPresent())
    {
        if (cdConfig.IsCertIdCorrect() && cdConfig.IsCertIdLenCorrect())
        {
            ReturnErrorOnFailure(writer.PutString(ContextTag(kTag_CertificateId), certElements.CertificateId));
        }
        else if (!cdConfig.IsCertIdCorrect())
        {
            ReturnErrorOnFailure(writer.PutString(ContextTag(kTag_CertificateId), "INV20141ZB330001-24"));
        }
        else
        {
            std::string cert_id(certElements.CertificateId);
            cert_id += "1234";
            ReturnErrorOnFailure(writer.PutString(ContextTag(kTag_CertificateId), cert_id.c_str()));
        }
    }
    if (cdConfig.IsSecurityLevelPresent())
    {
        if (cdConfig.IsSecurityLevelCorrect())
        {
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_SecurityLevel), certElements.SecurityLevel));
        }
        else
        {
            ReturnErrorOnFailure(
                writer.Put(ContextTag(kTag_SecurityLevel), static_cast<uint8_t>(certElements.SecurityLevel ^ UINT8_MAX)));
        }
    }
    if (cdConfig.IsSecurityInfoPresent())
    {
        if (cdConfig.IsSecurityInfoCorrect())
        {
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_SecurityInformation), certElements.SecurityInformation));
        }
        else
        {
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_SecurityInformation),
                                            static_cast<uint16_t>(certElements.SecurityInformation ^ UINT16_MAX)));
        }
    }
    if (cdConfig.IsVersionNumberPresent())
    {
        if (cdConfig.IsVersionNumberCorrect())
        {
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_VersionNumber), certElements.VersionNumber));
        }
        else
        {
            ReturnErrorOnFailure(
                writer.Put(ContextTag(kTag_VersionNumber), static_cast<uint16_t>(certElements.VersionNumber ^ UINT16_MAX)));
        }
    }
    if (cdConfig.IsCertTypePresent())
    {
        if (cdConfig.IsCertTypeCorrect())
        {
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_CertificationType), certElements.CertificationType));
        }
        else
        {
            ReturnErrorOnFailure(
                writer.Put(ContextTag(kTag_CertificationType), static_cast<uint8_t>(certElements.CertificationType ^ UINT8_MAX)));
        }
    }
    if (cdConfig.IsDACOriginVIDPresent())
    {
        if (cdConfig.IsDACOriginVIDCorrect() && certElements.DACOriginVIDandPIDPresent)
        {
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_DACOriginVendorId), certElements.DACOriginVendorId));
        }
        else
        {
            uint16_t wrong_dac_origin_vid = 0x8008;
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_DACOriginVendorId), wrong_dac_origin_vid));
        }
    }
    if (cdConfig.IsDACOriginPIDPresent())
    {
        if (cdConfig.IsDACOriginPIDCorrect() && certElements.DACOriginVIDandPIDPresent)
        {
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_DACOriginProductId), certElements.DACOriginProductId));
        }
        else
        {
            uint16_t wrong_dac_origin_pid = 0xFF00;
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_DACOriginProductId), wrong_dac_origin_pid));
        }
    }
    if (cdConfig.IsAuthPAAListPresent())
    {
        ReturnErrorOnFailure(writer.StartContainer(ContextTag(kTag_AuthorizedPAAList), kTLVType_Array, outerContainer2));
        uint8_t wrong_kid[kKeyIdentifierLength] = { 0xF4, 0x44, 0xCA, 0xBB, 0xC5, 0x01, 0x65, 0x77, 0xAA, 0x8B,
                                                    0x44, 0xFF, 0xB9, 0x0F, 0xCC, 0xA1, 0x40, 0xFE, 0x66, 0x20 };
        for (uint8_t i = 0; i < cdConfig.GetAuthPAAListCount(); i++)
        {
            if (cdConfig.IsAuthPAAListCorrect() && (i < certElements.AuthorizedPAAListCount))
            {
                ReturnErrorOnFailure(writer.Put(AnonymousTag(), ByteSpan(certElements.AuthorizedPAAList[i])));
            }
            else
            {
                wrong_kid[(i % kKeyIdentifierLength)] ^= 0xFF;
                ReturnErrorOnFailure(writer.Put(AnonymousTag(), ByteSpan(wrong_kid)));
            }
        }
        ReturnErrorOnFailure(writer.EndContainer(outerContainer2));
    }

    ReturnErrorOnFailure(writer.EndContainer(outerContainer1));

    ReturnErrorOnFailure(writer.Finalize());

    encodedCertElements.reduce_size(writer.GetLengthWritten());

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeEncapsulatedContent_Ignor_Error(const ByteSpan & cdContent, ASN1Writer & writer, CDStructConfig & cdConfig)
{
    /**
     * EncapsulatedContentInfo ::= SEQUENCE {
     *   eContentType OBJECT IDENTIFIER pkcs7-data (1.2.840.113549.1.7.1),
     *   eContent [0] EXPLICIT OCTET STRING cd_content }
     */
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_SEQUENCE
    {
        // eContentType OBJECT IDENTIFIER pkcs7-data (1.2.840.113549.1.7.1)
        if (cdConfig.IsCMSEContentTypeCorrect())
        {
            ReturnErrorOnFailure(writer.PutObjectId(sOID_ContentType_PKCS7Data, sizeof(sOID_ContentType_PKCS7Data)));
        }
        else
        {
            ReturnErrorOnFailure(writer.PutObjectId(sOID_ContentType_MSAC, sizeof(sOID_ContentType_MSAC)));
        }

        // eContent [0] EXPLICIT OCTET STRING cd_content
        ASN1_START_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
        {
            // OCTET STRING cd_content
            ReturnErrorOnFailure(writer.PutOctetString(cdContent.data(), static_cast<uint16_t>(cdContent.size())));
        }
        ASN1_END_CONSTRUCTED;
    }
    ASN1_END_SEQUENCE;

exit:
    return err;
}

CHIP_ERROR EncodeSignerInfo_Ignor_Error(const ByteSpan & signerKeyId, const P256ECDSASignature & signature, ASN1Writer & writer,
                                        CDStructConfig & cdConfig)
{
    /**
     * SignerInfo ::= SEQUENCE {
     *   version INTEGER ( v3(3) ),
     *   subjectKeyIdentifier OCTET STRING,
     *   digestAlgorithm OBJECT IDENTIFIER sha256 (2.16.840.1.101.3.4.2.1),
     *   signatureAlgorithm OBJECT IDENTIFIER ecdsa-with-SHA256 (1.2.840.10045.4.3.2),
     *   signature OCTET STRING }
     */
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1_START_SET
    {
        ASN1_START_SEQUENCE
        {
            // version INTEGER ( v3(3) )
            ASN1_ENCODE_INTEGER(cdConfig.IsSignerInfoVersionCorrect() ? 3 : 2);

            // subjectKeyIdentifier OCTET STRING
            if (cdConfig.IsSignerInfoSKIDCorrect())
            {
                ReturnErrorOnFailure(writer.PutOctetString(kASN1TagClass_ContextSpecific, 0, signerKeyId.data(),
                                                           static_cast<uint16_t>(signerKeyId.size())));
            }
            else
            {
                uint8_t wrong_skid[kKeyIdentifierLength];
                memcpy(wrong_skid, signerKeyId.data(), signerKeyId.size());
                wrong_skid[7] ^= 0xFF;
                ReturnErrorOnFailure(writer.PutOctetString(kASN1TagClass_ContextSpecific, 0, wrong_skid, sizeof(wrong_skid)));
            }

            // digestAlgorithm OBJECT IDENTIFIER sha256 (2.16.840.1.101.3.4.2.1)
            ASN1_START_SEQUENCE
            {
                if (cdConfig.IsSignerInfoDigestAlgoCorrect())
                {
                    ReturnErrorOnFailure(writer.PutObjectId(sOID_DigestAlgo_SHA256, sizeof(sOID_DigestAlgo_SHA256)));
                }
                else
                {
                    ReturnErrorOnFailure(writer.PutObjectId(sOID_DigestAlgo_SHA1, sizeof(sOID_DigestAlgo_SHA1)));
                }
            }
            ASN1_END_SEQUENCE;

            // signatureAlgorithm OBJECT IDENTIFIER ecdsa-with-SHA256 (1.2.840.10045.4.3.2)
            ASN1_START_SEQUENCE
            {
                if (cdConfig.IsCMSSigAlgoCorrect())
                {
                    ASN1_ENCODE_OBJECT_ID(kOID_SigAlgo_ECDSAWithSHA256);
                }
                else
                {
                    ReturnErrorOnFailure(writer.PutObjectId(sOID_SigAlgo_ECDSAWithSHA1, sizeof(sOID_SigAlgo_ECDSAWithSHA1)));
                }
            }
            ASN1_END_SEQUENCE;

            uint8_t asn1SignatureBuf[kMax_ECDSA_Signature_Length_Der];
            MutableByteSpan asn1Signature(asn1SignatureBuf);
            ReturnErrorOnFailure(ConvertECDSASignatureRawToDER(P256ECDSASignatureSpan(signature.ConstBytes()), asn1Signature));

            if (!cdConfig.IsCMSSignatureCorrect())
            {
                asn1SignatureBuf[10] ^= 0xFF;
            }

            // signature OCTET STRING
            ReturnErrorOnFailure(writer.PutOctetString(asn1Signature.data(), static_cast<uint16_t>(asn1Signature.size())));
        }
        ASN1_END_SEQUENCE;
    }
    ASN1_END_SET;

exit:
    return err;
}

CHIP_ERROR CMS_Sign_Ignore_Error(const ByteSpan & cdContent, const ByteSpan & signerKeyId, Crypto::P256Keypair & signerKeypair,
                                 MutableByteSpan & signedMessage, CDStructConfig & cdConfig)
{
    /**
     * CertificationDeclaration ::= SEQUENCE {
     *   version INTEGER ( v3(3) ),
     *   digestAlgorithm OBJECT IDENTIFIER sha256 (2.16.840.1.101.3.4.2.1),
     *   encapContentInfo EncapsulatedContentInfo,
     *   signerInfo SignerInfo }
     */
    CHIP_ERROR err = CHIP_NO_ERROR;
    ASN1Writer writer;
    uint32_t size = static_cast<uint32_t>(std::min(static_cast<size_t>(UINT32_MAX), signedMessage.size()));

    writer.Init(signedMessage.data(), size);

    ASN1_START_SEQUENCE
    {
        // OID identifies the CMS signed-data content type
        ReturnErrorOnFailure(writer.PutObjectId(sOID_ContentType_PKCS7SignedData, sizeof(sOID_ContentType_PKCS7SignedData)));

        ASN1_START_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
        {
            ASN1_START_SEQUENCE
            {
                // version INTEGER ( v3(3) )
                ASN1_ENCODE_INTEGER(cdConfig.IsCMSVersionCorrect() ? 3 : 2);

                // digestAlgorithm OBJECT IDENTIFIER sha256 (2.16.840.1.101.3.4.2.1)
                ASN1_START_SET
                {
                    ASN1_START_SEQUENCE
                    {
                        if (cdConfig.IsCMSDigestAlgoCorrect())
                        {
                            ReturnErrorOnFailure(writer.PutObjectId(sOID_DigestAlgo_SHA256, sizeof(sOID_DigestAlgo_SHA256)));
                        }
                        else
                        {
                            ReturnErrorOnFailure(writer.PutObjectId(sOID_DigestAlgo_SHA1, sizeof(sOID_DigestAlgo_SHA1)));
                        }
                    }
                    ASN1_END_SEQUENCE;
                }
                ASN1_END_SET;

                // encapContentInfo EncapsulatedContentInfo
                ReturnErrorOnFailure(EncodeEncapsulatedContent_Ignor_Error(cdContent, writer, cdConfig));

                Crypto::P256ECDSASignature signature;
                ReturnErrorOnFailure(signerKeypair.ECDSA_sign_msg(cdContent.data(), cdContent.size(), signature));

                // signerInfo SignerInfo
                ReturnErrorOnFailure(EncodeSignerInfo_Ignor_Error(signerKeyId, signature, writer, cdConfig));
            }
            ASN1_END_SEQUENCE;
        }
        ASN1_END_CONSTRUCTED;
    }
    ASN1_END_SEQUENCE;

    signedMessage.reduce_size(writer.GetLengthWritten());

exit:
    return err;
}

} // namespace

bool Cmd_GenCD(int argc, char * argv[])
{
    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        return true;
    }

    VerifyOrReturnError(ParseArgs(CMD_NAME, argc, argv, gCmdOptionSets), false);

    if (gCDConfig.IsErrorTestCaseEnabled())
    {
        fprintf(stderr,
                "WARNING gen-cd: The ignor-error option is set. This option makes it possible to generate invalid certification "
                "declaration.\n");
    }

    if (gKeyFileNameOrStr == nullptr)
    {
        fprintf(stderr, "Please specify the signing private key using the --key option.\n");
        return false;
    }

    if (gCertFileNameOrStr == nullptr)
    {
        fprintf(stderr, "Please specify the signing certificate using the --cert option.\n");
        return false;
    }

    if (gSignedCDFileName == nullptr)
    {
        fprintf(stderr, "Please specify the file name for the signed Certification Declaration using the --out option.\n");
        return false;
    }

    if (strcmp(gSignedCDFileName, "-") != 0 && access(gSignedCDFileName, R_OK) == 0)
    {
        fprintf(stderr,
                "Output signed CD file already exists (%s)\n"
                "To replace the file, please remove it and re-run the command.\n",
                gSignedCDFileName);
        return false;
    }

    if (!gCDConfig.IsErrorTestCaseEnabled())
    {
        if (gCertElements.VendorId == 0 || gCertElements.ProductIdsCount == 0 || strlen(gCertElements.CertificateId) == 0 ||
            gCertElements.VersionNumber == 0)
        {
            fprintf(stderr, "Please specify all mandatory CD elements.\n");
            return false;
        }

        if (gCertElements.DACOriginVIDandPIDPresent &&
            (gCertElements.DACOriginVendorId == 0 || gCertElements.DACOriginProductId == 0))
        {
            fprintf(stderr, "The DAC Origin Vendor Id and Product Id SHALL be specified together.\n");
            return false;
        }
    }

    {
        std::unique_ptr<X509, void (*)(X509 *)> cert(nullptr, &X509_free);
        std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> key(EVP_PKEY_new(), &EVP_PKEY_free);

        VerifyOrReturnError(ReadCert(gCertFileNameOrStr, cert), false);
        VerifyOrReturnError(ReadKey(gKeyFileNameOrStr, key), false);

        // Extract the subject key id from the X509 certificate.
        ByteSpan signerKeyId;
        VerifyOrReturnError(ExtractSKIDFromX509Cert(cert.get(), signerKeyId), false);

        // Initialize P256Keypair from EVP_PKEY.
        P256Keypair keypair;
        {
            P256SerializedKeypair serializedKeypair;
            VerifyOrReturnError(SerializeKeyPair(key.get(), serializedKeypair), false);
            VerifyOrReturnError(keypair.Deserialize(serializedKeypair) == CHIP_NO_ERROR, false);
        }

        // Encode CD TLV content.
        uint8_t encodedCDBuf[kCertificationElements_TLVEncodedMaxLength];
        MutableByteSpan encodedCD(encodedCDBuf);
        if (gCDConfig.IsErrorTestCaseEnabled())
        {
            VerifyOrReturnError(EncodeCertificationElements_Ignore_Error(gCertElements, encodedCD, gCDConfig) == CHIP_NO_ERROR,
                                false);
        }
        else
        {
            VerifyOrReturnError(EncodeCertificationElements(gCertElements, encodedCD) == CHIP_NO_ERROR, false);
        }

        // Sign CD.
        uint8_t signedMessageBuf[kMaxCMSSignedCDMessage];
        MutableByteSpan signedMessage(signedMessageBuf);
        if (gCDConfig.IsErrorTestCaseEnabled())
        {
            VerifyOrReturnError(CMS_Sign_Ignore_Error(encodedCD, signerKeyId, keypair, signedMessage, gCDConfig) == CHIP_NO_ERROR,
                                false);
        }
        else
        {
            VerifyOrReturnError(CMS_Sign(encodedCD, signerKeyId, keypair, signedMessage) == CHIP_NO_ERROR, false);
        }

        // Write to file.
        VerifyOrReturnError(WriteDataIntoFile(gSignedCDFileName, signedMessage.data(), static_cast<uint32_t>(signedMessage.size()),
                                              kDataFormat_Raw),
                            false);
    }
    return true;
}
