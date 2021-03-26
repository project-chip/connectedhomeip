/*
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

#include "SetupPayloadHelper.h"
#include "ManualSetupPayloadGenerator.h"
#include "QRCodeSetupPayloadGenerator.h"
#include "SetupPayload.h"
#include <fstream>
#include <utility>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

using namespace chip;

namespace chip {

enum SetupPayloadKey
{
    SetupPayloadKey_Version,
    SetupPayloadKey_VendorID,
    SetupPayloadKey_ProductID,
    SetupPayloadKey_RequiresCustomFlowTrue,
    SetupPayloadKey_RendezVousInformation,
    SetupPayloadKey_Discriminator,
    SetupPayloadKey_SetupPINCode,
};

struct SetupPayloadParameter
{
    SetupPayloadKey key;
    std::string stringValue;
    uint64_t uintValue;
};

static CHIP_ERROR resolveSetupPayloadParameter(SetupPayloadParameter & parameter, const std::string & key,
                                               const std::string & value)
{
    bool isUnsignedInt   = true;
    bool shouldHaveValue = true;
    if (key == "version")
    {
        parameter.key = SetupPayloadKey_Version;
    }
    else if (key == "vendorID")
    {
        parameter.key = SetupPayloadKey_VendorID;
    }
    else if (key == "productID")
    {
        parameter.key = SetupPayloadKey_ProductID;
    }
    else if (key == "requiresCustomFlowTrue")
    {
        parameter.key   = SetupPayloadKey_RequiresCustomFlowTrue;
        shouldHaveValue = false;
    }
    else if (key == "rendezVousInformation")
    {
        parameter.key = SetupPayloadKey_RendezVousInformation;
    }
    else if (key == "discriminator")
    {
        parameter.key = SetupPayloadKey_Discriminator;
    }
    else if (key == "setUpPINCode")
    {
        parameter.key = SetupPayloadKey_SetupPINCode;
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (shouldHaveValue && value.length() > 0)
    {
        if (isUnsignedInt)
        {
            parameter.uintValue = stoul(value, nullptr, 10);
        }
        else
        {
            parameter.stringValue = value;
        }
    }
    else if (shouldHaveValue)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

static CHIP_ERROR addParameter(SetupPayload & setupPayload, const SetupPayloadParameter & parameter)
{
    switch (parameter.key)
    {
    case SetupPayloadKey_Version:
        ChipLogDetail(SetupPayload, "Loaded version: %u", (uint8_t) parameter.uintValue);
        setupPayload.version = static_cast<uint8_t>(parameter.uintValue);
        break;
    case SetupPayloadKey_VendorID:
        ChipLogDetail(SetupPayload, "Loaded vendorID: %u", (uint16_t) parameter.uintValue);
        setupPayload.vendorID = static_cast<uint16_t>(parameter.uintValue);
        break;
    case SetupPayloadKey_ProductID:
        ChipLogDetail(SetupPayload, "Loaded productID: %u", (uint16_t) parameter.uintValue);
        setupPayload.productID = static_cast<uint16_t>(parameter.uintValue);
        break;
    case SetupPayloadKey_RequiresCustomFlowTrue:
        ChipLogDetail(SetupPayload, "Requires custom flow was set to true");
        setupPayload.requiresCustomFlow = true;
        break;
    case SetupPayloadKey_RendezVousInformation:
        ChipLogDetail(SetupPayload, "Loaded rendezvousInfo: %u", (uint16_t) parameter.uintValue);
        setupPayload.rendezvousInformation = static_cast<RendezvousInformationFlags>(parameter.uintValue);
        break;
    case SetupPayloadKey_Discriminator:
        ChipLogDetail(SetupPayload, "Loaded discriminator: %u", (uint16_t) parameter.uintValue);
        setupPayload.discriminator = static_cast<uint16_t>(parameter.uintValue);
        break;
    case SetupPayloadKey_SetupPINCode:
        ChipLogDetail(SetupPayload, "Loaded setupPinCode: %lu", (unsigned long) parameter.uintValue);
        setupPayload.setUpPINCode = static_cast<uint32_t>(parameter.uintValue);
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR loadPayloadFromFile(SetupPayload & setupPayload, const std::string & filePath)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::ifstream fileStream(filePath);
    VerifyOrExit(!fileStream.fail(), err = CHIP_ERROR_INVALID_ARGUMENT);

    while (fileStream)
    {
        std::string key;
        std::string value;
        SetupPayloadParameter parameter;

        getline(fileStream, key, ' ');
        fileStream >> value;
        fileStream.ignore();

        if (key.length() == 0)
        {
            continue;
        }
        err = resolveSetupPayloadParameter(parameter, key, value);
        SuccessOrExit(err);

        err = addParameter(setupPayload, parameter);
        SuccessOrExit(err);
    }
exit:
    return err;
}

CHIP_ERROR generateQRCodeFromFilePath(std::string filePath, std::string & outCode)
{
    SetupPayload setupPayload;
    CHIP_ERROR err = loadPayloadFromFile(setupPayload, std::move(filePath));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    QRCodeSetupPayloadGenerator generator(setupPayload);
    err = generator.payloadBase41Representation(outCode);
    return err;
}

CHIP_ERROR generateManualCodeFromFilePath(std::string filePath, std::string & outCode)
{
    SetupPayload setupPayload;
    CHIP_ERROR err = loadPayloadFromFile(setupPayload, std::move(filePath));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    ManualSetupPayloadGenerator generator(setupPayload);
    err = generator.payloadDecimalStringRepresentation(outCode);
    return err;
}
} // namespace chip
