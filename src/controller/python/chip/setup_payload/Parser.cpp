/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <support/CodeUtils.h>

#include <string>

using namespace chip;

namespace {

using AttributeVisitor       = void (*)(const char * attrName, const char * attrValue);
using VendorAttributeVisitor = void (*)(uint8_t tag, const char * attrValue);

void YieldSetupPayloadAttributes(const SetupPayload & payload, AttributeVisitor attrVisitor,
                                 VendorAttributeVisitor vendorAttrVisitor)
{
    attrVisitor("Version", std::to_string(payload.version).c_str());
    attrVisitor("VendorID", std::to_string(payload.vendorID).c_str());
    attrVisitor("ProductID", std::to_string(payload.productID).c_str());
    attrVisitor("CommissioningFlow", std::to_string(static_cast<uint8_t>(payload.commissioningFlow)).c_str());
    attrVisitor("RendezvousInformation", std::to_string(payload.rendezvousInformation.Raw()).c_str());
    attrVisitor("Discriminator", std::to_string(payload.discriminator).c_str());
    attrVisitor("SetUpPINCode", std::to_string(payload.setUpPINCode).c_str());

    std::string serialNumber;

    if (payload.getSerialNumber(serialNumber) == CHIP_NO_ERROR)
        attrVisitor("SerialNumber", serialNumber.c_str());

    for (const OptionalQRCodeInfo & info : payload.getAllOptionalVendorData())
    {
        if (info.type == optionalQRCodeInfoTypeString)
            vendorAttrVisitor(info.tag, info.data.c_str());

        if (info.type == optionalQRCodeInfoTypeInt32)
            vendorAttrVisitor(info.tag, std::to_string(info.int32).c_str());
    }
}

} // namespace

extern "C" CHIP_ERROR pychip_SetupPayload_ParseQrCode(const char * qrCode, AttributeVisitor attrVisitor,
                                                      VendorAttributeVisitor vendorAttrVisitor)
{
    SetupPayload payload;
    ReturnErrorOnFailure(QRCodeSetupPayloadParser(qrCode).populatePayload(payload));

    YieldSetupPayloadAttributes(payload, attrVisitor, vendorAttrVisitor);
    return CHIP_NO_ERROR;
}

extern "C" CHIP_ERROR pychip_SetupPayload_ParseManualPairingCode(const char * manualPairingCode, AttributeVisitor attrVisitor,
                                                                 VendorAttributeVisitor vendorAttrVisitor)
{
    SetupPayload payload;
    ReturnErrorOnFailure(ManualSetupPayloadParser(manualPairingCode).populatePayload(payload));

    YieldSetupPayloadAttributes(payload, attrVisitor, vendorAttrVisitor);
    return CHIP_NO_ERROR;
}
