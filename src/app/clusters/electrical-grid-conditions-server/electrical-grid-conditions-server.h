/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <lib/core/CHIPError.h>

// Add equality operator for ElectricalGridConditionsStruct
namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalGridConditions {
namespace Structs {
namespace ElectricalGridConditionsStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    // TODO: see #41307 issue for better codegen support for struct comparison
    // This operator is dangerous because it needs manual updates when struct fields are added
    /* uint32_t periodStart = static_cast<uint32_t>(0);
       DataModel::Nullable<uint32_t> periodEnd;
       int16_t gridCarbonIntensity     = static_cast<int16_t>(0);
       ThreeLevelEnum gridCarbonLevel  = static_cast<ThreeLevelEnum>(0);
       int16_t localCarbonIntensity    = static_cast<int16_t>(0);
       ThreeLevelEnum localCarbonLevel = static_cast<ThreeLevelEnum>(0);
    */

    // Validate that all expected fields exist with correct types and names
    // This will fail compilation if fields are removed, renamed, or change type
    static_assert(std::is_same<decltype(Type{}.periodStart), uint32_t>::value,
                  "ElectricalGridConditionsStruct::Type::periodStart missing or type changed - update operator==");
    static_assert(std::is_same<decltype(Type{}.periodEnd), DataModel::Nullable<uint32_t>>::value,
                  "ElectricalGridConditionsStruct::Type::periodEnd missing or type changed - update operator==");
    static_assert(std::is_same<decltype(Type{}.gridCarbonIntensity), int16_t>::value,
                  "ElectricalGridConditionsStruct::Type::gridCarbonIntensity missing or type changed - update operator==");
    static_assert(std::is_same<decltype(Type{}.gridCarbonLevel), ThreeLevelEnum>::value,
                  "ElectricalGridConditionsStruct::Type::gridCarbonLevel missing or type changed - update operator==");
    static_assert(std::is_same<decltype(Type{}.localCarbonIntensity), int16_t>::value,
                  "ElectricalGridConditionsStruct::Type::localCarbonIntensity missing or type changed - update operator==");
    static_assert(std::is_same<decltype(Type{}.localCarbonLevel), ThreeLevelEnum>::value,
                  "ElectricalGridConditionsStruct::Type::localCarbonLevel missing or type changed - update operator==");

    // Note: Cannot reliably detect new fields being added due to C++ limitations

    return (lhs.periodStart == rhs.periodStart && lhs.periodEnd == rhs.periodEnd &&
            lhs.gridCarbonIntensity == rhs.gridCarbonIntensity && lhs.gridCarbonLevel == rhs.gridCarbonLevel &&
            lhs.localCarbonIntensity == rhs.localCarbonIntensity && lhs.localCarbonLevel == rhs.localCarbonLevel);
}

inline bool operator!=(const Type & lhs, const Type & rhs)
{
    return !(lhs == rhs);
}
} // namespace ElectricalGridConditionsStruct
} // namespace Structs
} // namespace ElectricalGridConditions
} // namespace Clusters
} // namespace app
} // namespace chip

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalGridConditions {

// Spec-defined constraints
constexpr uint8_t kMaxForecastEntries = 56;

class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    // For now this is a placeholder and the Delegate could be removed
    // There are no delegated methods since these are largely implemented in the
    // electrical-grid-conditions-server.cpp Instance class

protected:
    EndpointId mEndpointId = 0;
};

class Instance : public AttributeAccessInterface
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, BitMask<Feature> aFeatures) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id), mDelegate(aDelegate), mFeatures(aFeatures)
    {
        /* set the base class delegate's endpointId */
        mDelegate.SetEndpointId(aEndpointId);
        mEndpointId = aEndpointId;
    }

    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

    // Set attribute methods
    CHIP_ERROR SetLocalGenerationAvailable(DataModel::Nullable<bool>);
    CHIP_ERROR SetCurrentConditions(DataModel::Nullable<Structs::ElectricalGridConditionsStruct::Type>);
    CHIP_ERROR SetForecastConditions(const DataModel::List<const Structs::ElectricalGridConditionsStruct::Type> &);

    // Generate CurrentConditionsChanged events
    Protocols::InteractionModel::Status GenerateCurrentConditionsChangedEvent();

private:
    // Internal copy function for forecast conditions memory management
    CHIP_ERROR CopyForecastConditions(const DataModel::List<const Structs::ElectricalGridConditionsStruct::Type> & src);
    Delegate & mDelegate;
    BitMask<Feature> mFeatures;

    EndpointId mEndpointId;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    // NOTE there are no writable attributes

    // CommandHandlerInterface
    // NOTE there are no commands

    // Attribute storage
    DataModel::Nullable<bool> mLocalGenerationAvailable;
    DataModel::Nullable<Structs::ElectricalGridConditionsStruct::Type> mCurrentConditions;
    DataModel::List<const Structs::ElectricalGridConditionsStruct::Type> mForecastConditions;

    // Memory storage for forecast conditions - manages ownership and lifetime
    // of the forecast data to ensure it remains valid during cluster operation.
    // The Instance owns this memory and is responsible for its allocation/deallocation.
    Platform::ScopedMemoryBufferWithSize<Structs::ElectricalGridConditionsStruct::Type> mForecastConditionsStorage;
};

} // namespace ElectricalGridConditions
} // namespace Clusters
} // namespace app
} // namespace chip
