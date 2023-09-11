/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <app/icd/ICDManager.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace app {

/**
 * @brief ICDEventManager class is responsible of processing Platform Events that affect an ICD's behaviour
 * The class registers an Event Handler with the Platform Manager and dispatches the processing to the ICDManager class.
 */
class ICDEventManager
{

public:
    ICDEventManager() = default;

    /**
     * @brief Initialisation function of the ICDEventManager.
     *        Init function MUST be called before using the object
     */
    CHIP_ERROR Init(ICDManager * icdManager);
    CHIP_ERROR Shutdown();

private:
    /**
     * @brief Event Handler callback given to the PlatformManager
     *        Function dispatchs the event to the ICDManager member
     */
    static void ICDEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    static uint8_t expectedMsgCount;
    static uint8_t awaitingAckCount;
    ICDManager * mICDManager;
};

} // namespace app
} // namespace chip
