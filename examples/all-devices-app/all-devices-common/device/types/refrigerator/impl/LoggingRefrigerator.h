/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <device/types/refrigerator/Refrigerator.h>
#include <device/types/temperature-controlled-cabinet/impl/LoggingTemperatureControlledCabinetPart.h>

#include <lib/support/CodeUtils.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace chip::app {

class LoggingRefrigerator : public Refrigerator
{
public:
    struct Config
    {
        // Safe food storage defaults representing a typical domestic refrigerator:
        // - 4°C is the globally recommended safe temperature to prevent food spoilage.
        // - 1°C to 7°C represents the typical safe operating boundaries.
        // - 0.1°C steps provide precise temperature adjustment.
        // Note: Temperature values are represented in 0.01°C steps.
        static constexpr TemperatureControlledCabinetPart::Config DefaultCabinetConfig()
        {
            return {
                .temperatureSetpoint = 400, // 4.00 °C
                .minTemperature      = 100, // 1.00 °C
                .maxTemperature      = 700, // 7.00 °C
                .step                = 10,  // 0.10 °C
            };
        }

        TemperatureControlledCabinetPart::Config cabinetConfig = DefaultCabinetConfig();
        uint8_t cabinetCount                                   = 1;
        /// Semantic tags applied to the refrigerator root endpoint descriptor (e.g. to
        /// disambiguate variants under wildcard allocation). The tags are copied, so the
        /// storage backing this span does not need to outlive the constructor call.
        Span<const EndpointComposition::SemanticTag> tagList = {};
    };

    explicit LoggingRefrigerator(TimerDelegate & timerDelegate);
    LoggingRefrigerator(TimerDelegate & timerDelegate, Config config);
    ~LoggingRefrigerator() override = default;

    size_t GetCabinetCount() const { return mCabinets.size(); }
    LoggingTemperatureControlledCabinetPart & GetCabinet(size_t index)
    {
        VerifyOrDie(index < mCabinets.size());
        return *mCabinets[index];
    }

protected:
    CHIP_ERROR RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider) override;
    void UnregisterParts(CodeDrivenDataModelProvider & provider) override;

private:
    // Owned copy of Config::tagList: mTagList (consumed by the DescriptorCluster, which keeps
    // only a view) points into it, so it must never be resized after the constructor.
    std::vector<EndpointComposition::SemanticTag> mOwnedTags;
    // Cabinet names are owned here because the parts keep only a const char pointer to them.
    std::vector<std::string> mCabinetNames;
    std::vector<std::unique_ptr<LoggingTemperatureControlledCabinetPart>> mCabinets;
};

} // namespace chip::app
