
/**
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

/**
 * @file CommodityTariffServer.h
 * @brief Header for Matter Commodity Tariff Cluster implementation
 *
 * This file contains the implementation of the Matter Commodity Tariff Cluster,
 * including attribute management, data structures, and server-side handling.
 */

#pragma once

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/StatusIB.h>
#include <app/reporting/reporting.h>
#include <cstdint>
#include <functional>

#include "CommodityTariffAttrsDataMgmt.h"

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

typedef uint32_t epoch_s; ///< Type alias for epoch timestamps in seconds

/**
 * @defgroup tariff_attributes Commodity Tariff Attribute Definitions
 * @{
 */

/**
 * @def COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
 * @brief Macro defining Primary attributes for Commodity Tariff
 *
 * Primary attributes represent the fundamental tariff configuration that can only
 * be changed by authorized tariff updates. These are typically set by utility providers.
 */
#define COMMODITY_TARIFF_PRIMARY_SCALAR_ATTRS                                                                                      \
    X(TariffUnit, DataModel::Nullable<Globals::TariffUnitEnum>)                                                                    \
    X(StartDate, DataModel::Nullable<uint32_t>)                                                                                    \
    X(DefaultRandomizationOffset, DataModel::Nullable<int16_t>)                                                                    \
    X(DefaultRandomizationType, DataModel::Nullable<DayEntryRandomizationTypeEnum>)

#define COMMODITY_TARIFF_PRIMARY_COMPLEX_ATTRIBUTES                                                                                \
    X(TariffInfo, DataModel::Nullable<Structs::TariffInformationStruct::Type>)                                                     \
    X(DayEntries, DataModel::List<Structs::DayEntryStruct::Type>)                                                                  \
    X(DayPatterns, DataModel::List<Structs::DayPatternStruct::Type>)                                                               \
    X(TariffComponents, DataModel::List<Structs::TariffComponentStruct::Type>)                                                     \
    X(TariffPeriods, DataModel::List<Structs::TariffPeriodStruct::Type>)                                                           \
    X(IndividualDays, DataModel::Nullable<DataModel::List<Structs::DayStruct::Type>>)                                              \
    X(CalendarPeriods, DataModel::Nullable<DataModel::List<Structs::CalendarPeriodStruct::Type>>)

#define COMMODITY_TARIFF_PRIMARY_ATTRIBUTES                                                                                        \
    COMMODITY_TARIFF_PRIMARY_SCALAR_ATTRS                                                                                          \
    COMMODITY_TARIFF_PRIMARY_COMPLEX_ATTRIBUTES

/**
 * @def COMMODITY_TARIFF_CURRENT_ATTRIBUTES
 * @brief Macro defining Current attributes for Commodity Tariff
 *
 * Current attributes represent the dynamically changing state of the tariff system,
 * automatically updated based on time context and primary attribute values.
 */
#define COMMODITY_TARIFF_CURRENT_ATTRIBUTES                                                                                        \
    X(CurrentDay, DataModel::Nullable<Structs::DayStruct::Type>)                                                                   \
    X(NextDay, DataModel::Nullable<Structs::DayStruct::Type>)                                                                      \
    X(CurrentDayEntry, DataModel::Nullable<Structs::DayEntryStruct::Type>)                                                         \
    X(NextDayEntry, DataModel::Nullable<Structs::DayEntryStruct::Type>)                                                            \
    X(CurrentDayEntryDate, DataModel::Nullable<uint32_t>)                                                                          \
    X(NextDayEntryDate, DataModel::Nullable<uint32_t>)                                                                             \
    X(CurrentTariffComponents, DataModel::List<Structs::TariffComponentStruct::Type>)                                              \
    X(NextTariffComponents, DataModel::Nullable<DataModel::List<Structs::TariffComponentStruct::Type>>)

/** @} */ // end of tariff_attributes

/**
 * @defgroup attribute_management Attribute Management Classes
 * @brief Macro-generated classes for type-safe attribute management
 *
 * These templates provide consistent attribute handling with:
 * - Type safety
 * - Change detection
 * - Validation
 * - Memory management
 * @{
 */

/**
 * @def X(attrName, attrType)
 * @brief Generates attribute-specific management classes
 *
 * For each attribute in COMMODITY_TARIFF_PRIMARY_ATTRIBUTES, creates a dedicated class that:
 * - Inherits from CTC_BaseDataClass<attrType>
 * - Provides type-specific storage management
 * - Enables attribute-specific validation
 *
 * Example generated class:
 * @code
 * class TariffUnitDataClass : public CTC_BaseDataClass<Nullable<Globals::TariffUnitEnum>> {
 * public:
 *     TariffUnitDataClass(Nullable<Globals::TariffUnitEnum>& storage)
 *         : CTC_BaseDataClass(storage) {}
 * };
 * @endcode
 */
#define X(attrName, attrType)                                                                                                      \
    class attrName##DataClass : public CTC_BaseDataClass<attrType>                                                                 \
    {                                                                                                                              \
    public:                                                                                                                        \
        attrName##DataClass(attrType & aValueStorage) : CTC_BaseDataClass<attrType>(aValueStorage, Attributes::attrName::Id) {}                              \
        ~attrName##DataClass() override = default;                                                                                 \
                                                                                                                                   \
    protected:                                                                                                                     \
        CHIP_ERROR Validate(const ValueType & aValue) const override;                                                              \
    };
COMMODITY_TARIFF_PRIMARY_SCALAR_ATTRS
#undef X

#define X(attrName, attrType)                                                                                                      \
    class attrName##DataClass : public CTC_BaseDataClass<attrType>                                                                 \
    {                                                                                                                              \
    public:                                                                                                                        \
        attrName##DataClass(attrType & aValueStorage) : CTC_BaseDataClass<attrType>(aValueStorage, Attributes::attrName::Id) {}                              \
        ~attrName##DataClass() override = default;                                                                                 \
                                                                                                                                   \
    protected:                                                                                                                     \
        CHIP_ERROR Validate(const ValueType & aValue) const override;                                                              \
        bool CompareStructValue(const PayloadType & source, const PayloadType & destination) const override;                       \
        void CleanupStructValue(PayloadType & aValue) override;                                                                    \
    };
COMMODITY_TARIFF_PRIMARY_COMPLEX_ATTRIBUTES
#undef X

/** @} */ // end of attribute_management

struct TariffUpdateCtx
{
    /* DayEntryIDs */
    std::unordered_set<uint32_t> DE_KeyIDs; /* Master - IDs of all given DayEntry items */

    std::unordered_set<uint32_t> DP_DE_IDs; /* IDs mentioned in DayPattern items */
    std::unordered_set<uint32_t> TP_DE_IDs; /* IDs mentioned in TariffPeriod items */
    std::unordered_set<uint32_t> ID_DE_IDs; /* IDs mentioned in IndividualDays items */

    /* TariffComponentIDs */
    std::unordered_set<uint32_t> TC_KeyIDs; /* Master - IDs of all given TariffComponent items */
    std::unordered_set<uint32_t> TP_TC_IDs; /* IDs mentioned in TariffPeriods items */

    /* DayPatternsIDs */
    std::unordered_set<uint32_t> DP_KeyIDs; /* Master - IDs of all given DayPattern items */
    std::unordered_set<uint32_t> CP_DP_IDs; /* IDs mentioned in CalendarPeriods items */

    BitMask<Feature> mFeature;
    EndpointId aEndpoint;
};

/**
 * @class CommodityTariffPrimaryData
 * @brief Container for primary tariff attribute storage
 *
 * This class holds the storage for all primary tariff attributes that define
 * the tariff configuration. It serves as the data backbone for tariff operations.
 */
class CommodityTariffPrimaryData
{
public:
    // Primary attribute storage
#define X(attrName, attrType) attrType m##attrName;
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X
};

/**
 * @class CommodityTariffDataProvider
 * @brief Core tariff data management and processing class
 *
 * This class provides:
 * - Storage for both primary and current attributes
 * - Tariff update processing pipeline
 * - Attribute access methods
 * - Data validation and change management
 */
class CommodityTariffDataProvider
{
public:
    CommodityTariffDataProvider()          = default;
    virtual ~CommodityTariffDataProvider() = default;

    /**
     * @brief Set the endpoint ID for this tariff instance
     * @param aEndpoint The Matter endpoint identifier
     */
    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    /**
     * @brief Set the current feature map for this tariff instance
     * @param aFeature The current feature map value
     */
    void SetFeatures(BitMask<Feature> aFeature) { mFeature = aFeature; }

    bool HasFeature(Feature aFeature) { return mFeature.Has(aFeature); }

    // Pure virtual interface methods
    virtual Protocols::InteractionModel::Status GetDayEntryById(DataModel::Nullable<uint32_t> aDayEntryId,
                                                                Structs::DayEntryStruct::Type & aDayEntry) = 0;

    virtual Protocols::InteractionModel::Status
    GetTariffComponentInfoById(DataModel::Nullable<uint32_t> aTariffComponentId, DataModel::Nullable<chip::CharSpan> & label,
                               DataModel::List<const uint32_t> & dayEntryIDs,
                               Structs::TariffComponentStruct::Type & aTariffComponent) = 0;

    /**
     * @brief Process incoming tariff data updates
     *
     * This method implements a three-phase update process:
     * 1. Initial validation (TariffDataUpd_Init)
     * 2. Cross-field validation (TariffDataUpd_CrossValidator)
     * 3. Commit or abort (TariffDataUpd_Commit/Abort)
     */
    void TariffDataUpdate();

    // Attribute accessors
#define X(attrName, attrType)                                                                                                      \
    attrType & Get##attrName() { return m##attrName##_MgmtObj.GetValue(); }
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X

#define X(attrName, attrType)                                                                                                      \
    attrType & Get##attrName() { return m##attrName; }
    COMMODITY_TARIFF_CURRENT_ATTRIBUTES
#undef X

private:
    // Current attribute storage
#define X(attrName, attrType) attrType m##attrName;
    COMMODITY_TARIFF_CURRENT_ATTRIBUTES
#undef X

    // Primary attribute storage and management
    CommodityTariffPrimaryData mTariffData;

    // Attribute management objects
#define X(attrName, attrType) attrName##DataClass m##attrName##_MgmtObj{ mTariffData.m##attrName };
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X

    // Primary attrs update pipeline methods
    bool TariffDataUpd_Init(TariffUpdateCtx & UpdCtx);
    void TariffDataUpd_Commit();
    void TariffDataUpd_Abort();
    bool TariffDataUpd_CrossValidator(TariffUpdateCtx & UpdCtx);

    // Current attrs (time depended) update methods
    void UpdateCurrentAttrs();
    static void MidnightTimerCallback(chip::System::Layer *, void * callbackContext);

protected:
    EndpointId mEndpointId = 0; ///< Associated Matter endpoint ID
    BitMask<Feature> mFeature;
};

/**
 * @class CommodityTariffServer
 * @brief Matter server implementation for the Commodity Tariff cluster
 *
 * This class handles:
 * - Attribute access interface
 * - Command handling
 * - Cluster feature management
 */
class CommodityTariffServer : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    /**
     * @brief Construct a new Commodity Tariff Server instance
     * @param aEndpointId The Matter endpoint ID
     * @param aProvider The data provider delegate
     * @param aFeature Bitmask of supported features
     */
    CommodityTariffServer(EndpointId aEndpointId, CommodityTariffDataProvider & aProvider, BitMask<Feature> aFeature);

    ~CommodityTariffServer() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    /**
     * @brief Check if a feature is supported
     * @param aFeature The feature to check
     * @return true if supported, false otherwise
     */
    bool HasFeature(Feature aFeature) const;

private:
    CommodityTariffDataProvider & mProvider;
    BitMask<Feature> mFeature;

    // AttributeAccessInterface implementation
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // CommandHandlerInterface implementation
    void InvokeCommand(HandlerContext & handlerContext) override;

    // Command handlers
    void HandleGetDayEntry(HandlerContext & ctx, const Commands::GetDayEntry::DecodableType & commandData);
    void HandleGetTariffComponent(HandlerContext & ctx, const Commands::GetTariffComponent::DecodableType & commandData);
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
