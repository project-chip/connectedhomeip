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

#include "QRCodeCommand.h"
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <setup_payload/SetupPayload.h>

using namespace ::chip;
using namespace ::chip::DeviceController;

CHIP_ERROR QRCodeCommand::Run(ChipDeviceController * dc, NodeId remoteId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SetupPayload payload;

    err = QRCodeSetupPayloadParser(mBase41Representation.c_str()).populatePayload(payload);

    std::vector<OptionalQRCodeInfo> data;
    data = payload.getAllOptionalVendorData();

    std::string outSerialNumber;
    payload.getSerialNumber(outSerialNumber);

    ChipLogProgress(SetupPayload, "\tRequiresCustomFlow :%u", payload.requiresCustomFlow);
    ChipLogProgress(SetupPayload, "\tVendorID : %u", payload.vendorID);
    ChipLogProgress(SetupPayload, "\tVersion : %u", payload.version);
    ChipLogProgress(SetupPayload, "\tProductID : %u", payload.productID);
    ChipLogProgress(SetupPayload, "\tDiscriminator : %u", payload.discriminator);
    ChipLogProgress(SetupPayload, "\tSetUpPINCode : %u", payload.setUpPINCode);
    ChipLogProgress(SetupPayload, "\tRendezvousInformation : %u", payload.rendezvousInformation);
    ChipLogProgress(SetupPayload, "\tSerialNumber : %u", outSerialNumber);

    SuccessOrExit(err);

exit:
    return err;
}
