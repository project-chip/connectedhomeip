/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#include "ParseCommand.h"
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/SetupPayload.h>
#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <support/verhoeff/Verhoeff.h>

using namespace ::chip;
using namespace ::chip::DeviceController;

CHIP_ERROR ParseCommand::Run(ChipDeviceController * dc, NodeId remoteId)
{    
    std::string codeString(mCode);
    SetupPayload payload;
    
    CHIP_ERROR err = CHIP_NO_ERROR;
    err = Parse(codeString, payload);

    SuccessOrExit(err);
    
    Print(payload);
exit:
    return err;
}

void ParseCommand::Print(chip::SetupPayload payload){
    std::string serialNumber;
    std::vector<OptionalQRCodeInfo> optionalVendorData;

    payload.getSerialNumber(serialNumber);
    optionalVendorData = payload.getAllOptionalVendorData();

    ChipLogProgress(SetupPayload, "RequiresCustomFlow: %u", payload.requiresCustomFlow);
    ChipLogProgress(SetupPayload, "VendorID: %u", payload.vendorID);
    ChipLogProgress(SetupPayload, "Version: %u", payload.version);
    ChipLogProgress(SetupPayload, "ProductID: %u", payload.productID);
    ChipLogProgress(SetupPayload, "Discriminator: %u", payload.discriminator);
    ChipLogProgress(SetupPayload, "SetUpPINCode: %u", payload.setUpPINCode);
    ChipLogProgress(SetupPayload, "RendezvousInformation: %u", payload.rendezvousInformation);
    ChipLogProgress(SetupPayload, "SerialNumber: %s", serialNumber.c_str());
    
    ChipLogProgress(SetupPayload, "OptionalVendorData Size: %u", optionalVendorData.size());

    for (const OptionalQRCodeInfo & info : optionalVendorData)
    {        
        if (info.type == optionalQRCodeInfoTypeString)
        {
            ChipLogProgress(SetupPayload, "OptionalQRCodeInfo: tag=%u,string value=%s",info.tag, info.data.c_str());
        }
        else if (info.type == optionalQRCodeInfoTypeInt32)
        {
            ChipLogProgress(SetupPayload, "OptionalQRCodeInfo: tag=%u,int value=%u",info.tag, info.int32);
        }
    }
}

CHIP_ERROR ParseCommand::Parse(std::string codeString, chip::SetupPayload &payload){
    
    CHIP_ERROR err = CHIP_NO_ERROR;
    
    if(IsQRCode(codeString)) {
        ChipLogDetail(SetupPayload, "Parsing Base41Representation: %s", codeString.c_str());
        return QRCodeSetupPayloadParser(codeString).populatePayload(payload);
    }
   
   ChipLogDetail(SetupPayload, "Parsing decimalRepresentation: %s", codeString.c_str());
   return ManualSetupPayloadParser(codeString).populatePayload(payload);
}

bool ParseCommand::IsQRCode(std::string codeString){
    return codeString.rfind("CH:")==0;
}