

#include <iostream>
#include <thread>

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app/server/Server.h>
#include <core/CHIPError.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <support/CHIPMem.h>
#include <support/RandUtils.h>

#include "Options.h"

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

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
    uint16_t vendorId;
    uint16_t productId;
    std::string result;

    err = ConfigurationMgr().GetSetupPinCode(setUpPINCode);
    SuccessOrExit(err);

    err = ConfigurationMgr().GetSetupDiscriminator(setUpDiscriminator);
    SuccessOrExit(err);

    err = ConfigurationMgr().GetVendorId(vendorId);
    SuccessOrExit(err);

    err = ConfigurationMgr().GetProductId(productId);
    SuccessOrExit(err);

    payload.version       = 0;
    payload.vendorID      = vendorId;
    payload.productID     = productId;
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

int ChipLinuxAppInit(int argc, char ** argv)
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

#if defined(PW_RPC_ENABLED)
    chip::rpc::Init();
    std::cerr << "PW_RPC initialized." << std::endl;
#endif // defined(PW_RPC_ENABLED)

    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(nullptr); // Use default device name (CHIP-XXXX)

#if CONFIG_NETWORK_LAYER_BLE
    chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(LinuxDeviceOptions::GetInstance().mBleDevice, false);
#endif

    chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true);

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    if (LinuxDeviceOptions::GetInstance().mWiFi)
    {
        chip::DeviceLayer::ConnectivityMgrImpl().StartWiFiManagement();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

#if CHIP_ENABLE_OPENTHREAD
    if (LinuxDeviceOptions::GetInstance().mThread)
    {
        SuccessOrExit(err = chip::DeviceLayer::ThreadStackMgrImpl().InitThreadStack());
        std::cerr << "Thread initialized." << std::endl;
    }
#endif // CHIP_ENABLE_OPENTHREAD

exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cerr << "Failed to run Linux Lighting App: " << ErrorStr(err) << std::endl;
        // End the program with non zero error code to indicate a error.
        return 1;
    }
    return 0;
}

void ChipLinuxAppMainLoop()
{
    // Init ZCL Data Model and CHIP App Server
    InitServer();

    chip::DeviceLayer::PlatformMgr().RunEventLoop();
}
