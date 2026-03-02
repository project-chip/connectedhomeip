/*
 *    Copyright (c) 2026 Project CHIP Authors
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
#pragma once

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/temperature-control-server/supported-temperature-levels-manager.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/TemperatureControl/Attributes.h>
#include <clusters/TemperatureControl/Metadata.h>

namespace chip::app::Clusters {

class TemperatureControlCluster : public DefaultServerCluster
{
public:
    struct StartupConfiguration
    {
        int16_t temperatureSetpoint{};
        int16_t minTemperature{};
        int16_t maxTemperature{};
        int16_t step{};
        uint8_t selectedTemperatureLevel{};
        TemperatureControl::SupportedTemperatureLevelsIteratorDelegate * delegate{ nullptr };
    };

    TemperatureControlCluster(EndpointId endpointId, const BitFlags<TemperatureControl::Feature> features,
                              const StartupConfiguration & config);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR SetTemperatureSetpoint(int16_t temperatureSetpoint);
    int16_t GetTemperatureSetpoint() const { return mTemperatureSetpoint; }

    int16_t GetMinTemperature() const { return mMinTemperature; }
    int16_t GetMaxTemperature() const { return mMaxTemperature; }

protected:
    const BitFlags<TemperatureControl::Feature> mFeatures;
    TemperatureControl::SupportedTemperatureLevelsIteratorDelegate * mDelegate{};
    int16_t mTemperatureSetpoint{};
    int16_t mMinTemperature{};
    int16_t mMaxTemperature{};
    int16_t mStep{};
    uint8_t mSelectedTemperatureLevel{};
    chip::app::DataModel::List<const chip::CharSpan> mSupportedTemperatureLevels;

private:
    std::optional<DataModel::ActionReturnStatus>
    HandleSetTemperature(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                         const TemperatureControl::Commands::SetTemperature::DecodableType & commandData);
};

} // namespace chip::app::Clusters
