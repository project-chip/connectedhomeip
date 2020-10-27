/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <platform/Linux/BLEManagerImpl.h>

#include "gen/attribute-id.h"
#include "gen/cluster-id.h"
#include "gen/znet-bookkeeping.h"
#include <app/chip-zcl-zpro-codec.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <core/CHIPError.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <support/RandUtils.h>

#include "LightingManager.h"
#include "Options.h"
#include "Server.h"

#include <cassert>
#include <iostream>

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

constexpr uint32_t kDefaultSetupPinCode = 12345678; // TODO: Should be a macro in CHIPProjectConfig.h like other example apps.
constexpr int kExampleVenderID          = 0xabcd;

extern "C" {
void emberAfPostAttributeChangeCallback(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
    if (clusterId != ZCL_ON_OFF_CLUSTER_ID)
    {
        ChipLogProgress(Zcl, "Unknown cluster ID: %d", clusterId);
        return;
    }

    if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
    {
        ChipLogProgress(Zcl, "Unknown attribute ID: %d", attributeId);
        return;
    }

    if (*value)
    {
        LightingMgr().InitiateAction(LightingManager::ON_ACTION);
    }
    else
    {
        LightingMgr().InitiateAction(LightingManager::OFF_ACTION);
    }
}

/** @brief On/off Cluster Server Post Init
 *
 * Following resolution of the On/Off state at startup for this endpoint,
 * perform any additional initialization needed; e.g., synchronize hardware
 * state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginOnOffClusterServerPostInitCallback(uint8_t endpoint)
{
    // TODO: implement any additional On/off Cluster Server post init actions
}
}

namespace {
void EventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished)
    {
        ChipLogProgress(DeviceLayer, "Receive kCHIPoBLEConnectionEstablished");
    }
}

CHIP_ERROR PrintQRCodeContent()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // If we do not have a discriminator, generate one
    chip::SetupPayload payload;
    uint32_t setUpPINCode;
    uint16_t setUpDiscriminator;
    std::string result;

    err = ConfigurationMgr().GetSetupPinCode(setUpPINCode);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        setUpPINCode = kDefaultSetupPinCode;
        err          = ConfigurationMgr().StoreSetupPinCode(setUpPINCode);
    }
    SuccessOrExit(err);

    err = ConfigurationMgr().GetSetupDiscriminator(setUpDiscriminator);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        setUpDiscriminator = GetRandU16() & 0xFFF;
        err                = ConfigurationMgr().StoreSetupDiscriminator(setUpDiscriminator);
    }
    SuccessOrExit(err);

    payload.version       = 1;
    payload.vendorID      = kExampleVenderID;
    payload.productID     = 1;
    payload.setUpPINCode  = setUpPINCode;
    payload.discriminator = setUpDiscriminator;

    // Wrap it so SuccessOrExit can work
    {
        chip::QRCodeSetupPayloadGenerator generator(payload);
        err = generator.payloadBase41Representation(result);
        SuccessOrExit(err);
    }

    std::cout << "SetupPINCode: [" << setUpPINCode << "]" << std::endl;
    // There might be whitespace in setup QRCode, add brackets to make it clearer.
    std::cout << "SetupQRCode:  [" << result << "]" << std::endl;

exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cerr << "Failed to generate QR Code: " << ErrorStr(err) << std::endl;
    }
    return err;
}
} // namespace

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    err = ParseArguments(argc, argv);
    SuccessOrExit(err);

    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    err = PrintQRCodeContent();
    SuccessOrExit(err);

    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(nullptr); // Use default device name (CHIP-XXXX)

    chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(LinuxDeviceOptions::GetInstance().mBleDevice, false);

    chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(
        chip::DeviceLayer::ConnectivityManager::kCHIPoBLEServiceMode_Enabled);

    LightingMgr().Init();

    // Init ZCL Data Model and CHIP App Server
    InitServer();

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cerr << "Failed to run Linux Lighting App: " << ErrorStr(err) << std::endl;
        // End the program with non zero error code to indicate a error.
        return 1;
    }
    return 0;
}
