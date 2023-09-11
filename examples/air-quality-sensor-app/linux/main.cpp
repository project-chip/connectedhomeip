/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AirQualitySensorAppAttrUpdateDelegate.h"
#include <AppMain.h>

#include <app/util/af.h>
#include <platform/CHIPDeviceBuildConfig.h>

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
#include <imgui_ui/ui.h>
#include <imgui_ui/windows/qrcode.h>
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
constexpr const char kChipEventFifoPathPrefix[] = "/tmp/chip_air_quality_fifo_";
NamedPipeCommands sChipNamedPipeCommands;
AirQualitySensorAppAttrUpdateDelegate sAirQualitySensorAppCommandDelegate;
} // namespace

void ApplicationInit() {}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

    std::string path = kChipEventFifoPathPrefix + std::to_string(getpid());

    if (sChipNamedPipeCommands.Start(path, &sAirQualitySensorAppCommandDelegate) != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        sChipNamedPipeCommands.Stop();
    }

#if defined(CHIP_IMGUI_ENABLED) && CHIP_IMGUI_ENABLED
    example::Ui::ImguiUi ui;
    ui.AddWindow(std::make_unique<example::Ui::Windows::QRCode>());
    ChipLinuxAppMainLoop(&ui);
#else
    ChipLinuxAppMainLoop();
#endif

    return 0;
}
