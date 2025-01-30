/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "DCLClient.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

#include "HTTPSRequest.h"
#include "JsonSchemaMacros.h"

namespace {
constexpr const char * kDefaultDCLHostName            = "on.dcl.csa-iot.org";
constexpr const char * kErrorSchemaValidation         = "Model schema validation failed for response content: ";
constexpr const char * kErrorVendorIdIsZero           = "Invalid argument: Vendor ID should not be 0";
constexpr const char * kErrorProductIdIsZero          = "Invalid argument: Product ID should not be 0";
constexpr const char * kErrorOrdinalValueTooLarge     = "Ordinal value exceeds the maximum allowable bits: ";
constexpr const char * kRequestModelVendorProductPath = "/dcl/model/models/%u/%u";
constexpr uint8_t kRequestPathBufferSize              = 64;
constexpr uint16_t kTermsAndConditionSchemaVersion    = 1;
} // namespace

namespace chip {
namespace tool {
namespace dcl {

namespace {
CHIP_ERROR ValidateModelSchema(const Json::Value & json)
{
    CHECK_REQUIRED_TYPE(json, model, Object)
    auto model = json["model"];

    CHECK_REQUIRED_TYPE(model, commissioningCustomFlow, UInt);

    // The "enhancedSetupFlowOptions" field is theoretically required by the schema.
    // However, the current DCL implementation does not include it.
    // To handle this gracefully, we inject the field and set its value to 0 if it is missing.
    if (!model.isMember("enhancedSetupFlowOptions"))
    {
        model["enhancedSetupFlowOptions"] = 0;
    }

    CHECK_REQUIRED_TYPE(model, enhancedSetupFlowOptions, UInt)

    // Check if enhancedSetupFlowOptions has bit 0 set.
    // Bit 0 indicates that enhanced setup flow is enabled.
    auto enhancedSetupFlowOptions = model["enhancedSetupFlowOptions"];
    VerifyOrReturnError((enhancedSetupFlowOptions.asUInt() & 0x01) != 0, CHIP_NO_ERROR);

    // List of required keys in the "model" object if enhancedSetupFlowOptions has bit 0 set.
    CHECK_REQUIRED_TYPE(model, enhancedSetupFlowTCUrl, String)
    CHECK_REQUIRED_TYPE(model, enhancedSetupFlowTCDigest, String)
    CHECK_REQUIRED_TYPE(model, enhancedSetupFlowTCFileSize, UInt)
    CHECK_REQUIRED_TYPE(model, enhancedSetupFlowTCRevision, UInt)
    CHECK_REQUIRED_TYPE(model, enhancedSetupFlowMaintenanceUrl, String)

    return CHIP_NO_ERROR;
}

CHIP_ERROR ValidateModelCustomFlow(const Json::Value & json, CommissioningFlow payloadCommissioningFlow)
{
    auto model = json["model"];
    CHECK_REQUIRED_VALUE(model, commissioningCustomFlow, to_underlying(payloadCommissioningFlow))
    return CHIP_NO_ERROR;
}

CHIP_ERROR ValidateTCLanguageEntries(const Json::Value & languageEntries)
{
    for (Json::Value::const_iterator it = languageEntries.begin(); it != languageEntries.end(); it++)
    {
        const Json::Value & languageArray = *it;

        CHECK_TYPE(languageArray, languageArray, Array);

        for (Json::ArrayIndex i = 0; i < languageArray.size(); i++)
        {
            const Json::Value & term = languageArray[i];
            CHECK_REQUIRED_TYPE(term, title, String);
            CHECK_REQUIRED_TYPE(term, text, String);
            CHECK_REQUIRED_TYPE(term, required, Bool);
            CHECK_REQUIRED_TYPE(term, ordinal, UInt);

            auto ordinal = term["ordinal"].asUInt();
            VerifyOrReturnError(ordinal < 16, CHIP_ERROR_INVALID_ARGUMENT,
                                ChipLogError(chipTool, "%s%u", kErrorOrdinalValueTooLarge, ordinal));
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ValidateTCCountryEntries(const Json::Value & countryEntries)
{
    for (Json::Value::const_iterator it = countryEntries.begin(); it != countryEntries.end(); it++)
    {
        const Json::Value & countryEntry = *it;

        CHECK_REQUIRED_TYPE(countryEntry, defaultLanguage, String);
        CHECK_REQUIRED_TYPE(countryEntry, languageEntries, Object);

        ReturnErrorOnFailure(ValidateTCLanguageEntries(countryEntry["languageEntries"]));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ValidateTermsAndConditionsSchema(const Json::Value & tc, unsigned int expectedEnhancedSetupFlowTCRevision)
{
    CHECK_REQUIRED_VALUE(tc, schemaVersion, kTermsAndConditionSchemaVersion)
    CHECK_REQUIRED_TYPE(tc, esfRevision, UInt)
    CHECK_REQUIRED_TYPE(tc, defaultCountry, String)
    CHECK_REQUIRED_TYPE(tc, countryEntries, Object)
    CHECK_REQUIRED_VALUE(tc, esfRevision, expectedEnhancedSetupFlowTCRevision)
    return ValidateTCCountryEntries(tc["countryEntries"]);
}

CHIP_ERROR RequestTermsAndConditions(const Json::Value & json, Json::Value & tc)
{
    auto & model = json["model"];
    if ((model["enhancedSetupFlowOptions"].asUInt() & 0x01) == 0)
    {
        ChipLogProgress(chipTool,
                        "Enhanced setup flow is not enabled for this model (bit 0 of enhancedSetupFlowOptions is not set). No "
                        "Terms and Conditions are required for this configuration.");
        tc = Json::nullValue;
        return CHIP_NO_ERROR;
    }

    auto & enhancedSetupFlowTCUrl      = model["enhancedSetupFlowTCUrl"];
    auto & enhancedSetupFlowTCFileSize = model["enhancedSetupFlowTCFileSize"];
    auto & enhancedSetupFlowTCDigest   = model["enhancedSetupFlowTCDigest"];
    auto & enhancedSetupFlowTCRevision = model["enhancedSetupFlowTCRevision"];

    auto * tcUrl                = enhancedSetupFlowTCUrl.asCString();
    const auto optionalFileSize = MakeOptional(static_cast<uint32_t>(enhancedSetupFlowTCFileSize.asUInt()));
    const auto optionalDigest   = MakeOptional(enhancedSetupFlowTCDigest.asCString());
    ReturnErrorOnFailure(https::Request(tcUrl, tc, optionalFileSize, optionalDigest));
    ReturnErrorOnFailure(ValidateTermsAndConditionsSchema(tc, enhancedSetupFlowTCRevision.asUInt()));

    return CHIP_NO_ERROR;
}

} // namespace

DCLClient::DCLClient(Optional<const char *> hostname, Optional<uint16_t> port)
{
    mHostName = hostname.ValueOr(kDefaultDCLHostName);
    mPort     = port.ValueOr(0);
}

CHIP_ERROR DCLClient::Model(const char * onboardingPayload, Json::Value & outModel)
{
    SetupPayload payload;
    bool isQRCode = strncmp(onboardingPayload, kQRCodePrefix, strlen(kQRCodePrefix)) == 0;
    if (isQRCode)
    {
        ReturnErrorOnFailure(QRCodeSetupPayloadParser(onboardingPayload).populatePayload(payload));
        VerifyOrReturnError(payload.isValidQRCodePayload(), CHIP_ERROR_INVALID_ARGUMENT);
    }
    else
    {
        ReturnErrorOnFailure(ManualSetupPayloadParser(onboardingPayload).populatePayload(payload));
        VerifyOrReturnError(payload.isValidManualCode(), CHIP_ERROR_INVALID_ARGUMENT);
    }

    auto vendorId  = static_cast<VendorId>(payload.vendorID);
    auto productId = payload.productID;

    // If both vendorId and productId are zero, return a null model without error.
    if (vendorId == 0 && productId == 0)
    {
        ChipLogProgress(chipTool, "Vendor ID and Product ID not found in the provided payload. DCL lookup will not be used.");
        outModel = Json::nullValue;
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(Model(vendorId, productId, outModel));

    auto commissioningFlow = payload.commissioningFlow;
    CHIP_ERROR error       = ValidateModelCustomFlow(outModel, commissioningFlow);
    VerifyOrReturnError(CHIP_NO_ERROR == error, error,
                        ChipLogError(chipTool, "%s%s", kErrorSchemaValidation, outModel.toStyledString().c_str()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR DCLClient::Model(const chip::VendorId vendorId, const uint16_t productId, Json::Value & outModel)
{
    VerifyOrReturnError(0 != vendorId, CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(chipTool, "%s", kErrorVendorIdIsZero));
    VerifyOrReturnError(0 != productId, CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(chipTool, "%s", kErrorProductIdIsZero));

    char path[kRequestPathBufferSize];
    VerifyOrReturnError(snprintf(path, sizeof(path), kRequestModelVendorProductPath, to_underlying(vendorId), productId) >= 0,
                        CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(https::Request(mHostName, mPort, path, outModel));

    CHIP_ERROR error = ValidateModelSchema(outModel);
    VerifyOrReturnError(CHIP_NO_ERROR == error, error,
                        ChipLogError(chipTool, "%s%s", kErrorSchemaValidation, outModel.toStyledString().c_str()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR DCLClient::TermsAndConditions(const char * onboardingPayload, Json::Value & outTc)
{
    Json::Value json;
    ReturnErrorOnFailure(Model(onboardingPayload, json));
    VerifyOrReturnError(Json::nullValue != json.type(), CHIP_NO_ERROR, outTc = Json::nullValue);
    ReturnErrorOnFailure(RequestTermsAndConditions(json, outTc));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DCLClient::TermsAndConditions(const chip::VendorId vendorId, const uint16_t productId, Json::Value & outTc)
{
    Json::Value json;
    ReturnErrorOnFailure(Model(vendorId, productId, json));
    VerifyOrReturnError(Json::nullValue != json.type(), CHIP_NO_ERROR, outTc = Json::nullValue);
    ReturnErrorOnFailure(RequestTermsAndConditions(json, outTc));
    return CHIP_NO_ERROR;
}

} // namespace dcl
} // namespace tool
} // namespace chip
