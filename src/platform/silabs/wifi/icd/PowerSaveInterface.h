
/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

/**
 * @brief Power Save Platform Interface for Wi-Fi platforms
 *
 */
class PowerSaveInterface
{
public:
    virtual ~PowerSaveInterface() = default;

    enum class PowerSaveConfiguration : uint8_t
    {
        kHighPerformance = 0,
        kDeepSleep       = 1,
        kConnectedSleep  = 2,
    };

    /**
     * @brief Configures the underlying platform to the requested power save mode.
     *
     * @param configuration PowerSaveConfiguration to configure the platform to
     * @param listenInterval Listen interval to configure the platform to
     *
     * @return CHIP_ERROR CHIP_NO_ERROR, if the configuration was successful
     *                    CHIP_ERROR_INTERNAL, if there was an error when configuring the power save mode
     *                    CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE, if the platform does not support the requested power save mode or if
     *                                                         it is a non-supported configuration
     */
    virtual CHIP_ERROR ConfigurePowerSave(PowerSaveConfiguration configuration, uint32_t listenInterval)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    /**
     * @brief Configures the broadcast filter.
     *
     * @param[in] enableBroadcastFilter Boolean to enable or disable the broadcast filter.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR, the counters were succesfully reset to 0.
     *                    CHIP_ERROR_INTERNAL, if there was an error when configuring the broadcast filter
     *                    CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE, if the platform does not support the requested power save mode
     *                                                         or if it is a non-supported configuration
     */
    virtual CHIP_ERROR ConfigureBroadcastFilter(bool enableBroadcastFilter) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }
};

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
