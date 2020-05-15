//
//  SetupPayloadHelper.cpp
//  chiptool
//
//  Created by Shana Azria on 15/05/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#include "SetupPayloadHelper.hpp"
#include "QRCodeSetupPayloadGenerator.h"
#include "ManualSetupPayloadGenerator.h"
#include "SetupPayload.h"
#include <fstream>

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
    string stringValue;
    uint64_t uintValue;
};

static CHIP_ERROR resolveSetupPayloadParameter(SetupPayloadParameter & parameter, string key, string value)
{
    bool isUnsignedInt   = true;
    bool shouldHaveValue = true;
    if (key.compare("version") == 0)
    {
        parameter.key = SetupPayloadKey_Version;
    }
    else if (key.compare("vendorID") == 0)
    {
        parameter.key = SetupPayloadKey_VendorID;
    }
    else if (key.compare("productID") == 0)
    {
        parameter.key = SetupPayloadKey_ProductID;
    }
    else if (key.compare("requiresCustomFlowTrue") == 0)
    {
        parameter.key   = SetupPayloadKey_RequiresCustomFlowTrue;
        shouldHaveValue = false;
    }
    else if (key.compare("rendezVousInformation") == 0)
    {
        parameter.key = SetupPayloadKey_RendezVousInformation;
    }
    else if (key.compare("discriminator") == 0)
    {
        parameter.key = SetupPayloadKey_Discriminator;
    }
    else if (key.compare("setUpPINCode") == 0)
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
            try
            {
                parameter.uintValue = stoul(value, nullptr, 10);
            } catch (const std::invalid_argument & ia)
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
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

static CHIP_ERROR addParameter(SetupPayload & setupPayload, SetupPayloadParameter parameter)
{
    switch (parameter.key)
    {
    case SetupPayloadKey_Version:
        printf("Loaded version: %llu\n", parameter.uintValue);
        setupPayload.version = parameter.uintValue;
        break;
    case SetupPayloadKey_VendorID:
        printf("Loaded vendorID: %llu\n", parameter.uintValue);
        setupPayload.vendorID = parameter.uintValue;
        break;
    case SetupPayloadKey_ProductID:
        printf("Loaded productID: %llu\n", parameter.uintValue);
        setupPayload.productID = parameter.uintValue;
        break;
    case SetupPayloadKey_RequiresCustomFlowTrue:
        printf("Requires custom flow was set to true\n");
        setupPayload.requiresCustomFlow = true;
        break;
    case SetupPayloadKey_RendezVousInformation:
        printf("Loaded rendezvousInfo: %llu\n", parameter.uintValue);
        setupPayload.rendezvousInformation = parameter.uintValue;
        break;
    case SetupPayloadKey_Discriminator:
        printf("Loaded discriminator: %llu\n", parameter.uintValue);
        setupPayload.discriminator = parameter.uintValue;
        break;
    case SetupPayloadKey_SetupPINCode:
        printf("Loaded setupPinCode: %llu\n", parameter.uintValue);
        setupPayload.setUpPINCode = (uint32_t) parameter.uintValue;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR loadPayloadFromFile(SetupPayload & setupPayload, string filePath)
{
    ifstream fileStream(filePath);
    if (fileStream.fail())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    while (fileStream)
    {
        string key;
        string value;
        SetupPayloadParameter parameter;

        getline(fileStream, key, ' ');
        fileStream >> value;
        fileStream.ignore();

        if (key.length() == 0)
        {
            continue;
        }
        CHIP_ERROR err = resolveSetupPayloadParameter(parameter, key, value);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
        err = addParameter(setupPayload, parameter);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR generateQRCodeFromFilePath(string filePath, string & outCode)
{
    SetupPayload setupPayload;
    CHIP_ERROR err = loadPayloadFromFile(setupPayload, filePath);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    QRCodeSetupPayloadGenerator generator(setupPayload);
    outCode = generator.payloadBase41Representation();
    if (outCode.length() == 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR generateManualCodeFromFilePath(string filePath, string & outCode)
{
    SetupPayload setupPayload;
    CHIP_ERROR err = loadPayloadFromFile(setupPayload, filePath);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    ManualSetupPayloadGenerator generator(setupPayload);
    err = generator.payloadDecimalStringRepresentation(outCode);
    return err;
}
} // namespace chip
