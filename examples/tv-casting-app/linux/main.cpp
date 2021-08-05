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

#include <controller/CHIPCommissionableNodeController.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/SetupPayload.h>
#include <support/CHIPMem.h>

using namespace chip;

#define TV_DEVICE_TYPE 35

CHIP_ERROR RunTimedEventLoop(uint32_t aDelayMilliseconds)
{
    System::Timer * timer = nullptr;
    if (DeviceLayer::SystemLayer.NewTimer(timer) == CHIP_NO_ERROR)
    {
        timer->Start(
            aDelayMilliseconds,
            [](System::Layer *, void *, CHIP_ERROR err) {
                DeviceLayer::PlatformMgr().StopEventLoopTask();
                DeviceLayer::PlatformMgr().Shutdown();
            },
            nullptr);
    }
    else
    {
        return CHIP_ERROR_INTERNAL;
    }
    DeviceLayer::PlatformMgr().RunEventLoop();
    return CHIP_NO_ERROR;
}

void LogOnboardingPayload(chip::SetupPayload payload)
{
    ChipLogProgress(Zcl, "Onboarding Payload::");
    ChipLogProgress(Zcl, "PIN: %u", payload.setUpPINCode);
    ChipLogProgress(Zcl, "Discriminator: %u", payload.discriminator);
    ChipLogProgress(Zcl, "Vendor ID: %u", payload.vendorID);
    ChipLogProgress(Zcl, "Product ID: %u", payload.productID);
}

int main(int argc, char * argv[])
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    int commissionerCount = 0, selectedCommissioner = CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES;
    chip::Controller::CommissionableNodeController mCommissionableNodeController;
    Mdns::DiscoveryFilter filter(Mdns::DiscoveryFilterType::kDeviceType, TV_DEVICE_TYPE);
    chip::SetupPayload payload;
    SuccessOrExit(chip::Platform::MemoryInit());
    SuccessOrExit(chip::DeviceLayer::PlatformMgr().InitChipStack());

    // Discover commissioner TVs
    SuccessOrExit(mCommissionableNodeController.DiscoverCommissioners(filter));
    SuccessOrExit(RunTimedEventLoop(2 * 1000));

    // Display discovered commissioner TVs
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        const Mdns::DiscoveredNodeData * commissioner = mCommissionableNodeController.GetDiscoveredCommissioner(i);
        if (commissioner != nullptr)
        {
            ChipLogProgress(Zcl, "Discovered Commisioner #%d", ++commissionerCount);
            commissioner->Log();
        }
    }
    ChipLogProgress(Zcl, "%d commissioner(s) discovered", commissionerCount);

    // Accept user selection for commissioner TV
    VerifyOrExit(commissionerCount > 0, err = CHIP_NO_ERROR);
    ChipLogProgress(Zcl, "Choose a commissioner TV (by number# above) to request commissioning from: ");
    scanf("%d", &selectedCommissioner);

    // Initiate commissionning with selected commissioner TV
    SuccessOrExit(chip::DeviceLayer::PlatformMgr().InitChipStack());
    SuccessOrExit(mCommissionableNodeController.RequestCommissioning(selectedCommissioner));
    SuccessOrExit(mCommissionableNodeController.GetOnboardingPayload(payload));
    LogOnboardingPayload(payload);
    SuccessOrExit(RunTimedEventLoop(3 * 60 * 1000));

    // TBD: Implement casting to TV

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to run TV Casting App:: %s", ErrorStr(err));
        // End the program with non zero error code to indicate an error.
        return 1;
    }
    return 0;
}
