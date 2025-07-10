
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

#include "CommodityTariffAttrsDataMgmt.h"

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <cstdint>

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
    X(DayEntries, DataModel::Nullable<DataModel::List<Structs::DayEntryStruct::Type>>)                                             \
    X(DayPatterns, DataModel::Nullable<DataModel::List<Structs::DayPatternStruct::Type>>)                                          \
    X(TariffComponents, DataModel::Nullable<DataModel::List<Structs::TariffComponentStruct::Type>>)                                \
    X(TariffPeriods, DataModel::Nullable<DataModel::List<Structs::TariffPeriodStruct::Type>>)                                      \
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
#define COMMODITY_TARIFF_CURRENT_SINGLE_ATTRIBUTES                                                                                 \
    X(CurrentDay, DataModel::Nullable<Structs::DayStruct::Type>)                                                                   \
    X(NextDay, DataModel::Nullable<Structs::DayStruct::Type>)                                                                      \
    X(CurrentDayEntry, DataModel::Nullable<Structs::DayEntryStruct::Type>)                                                         \
    X(NextDayEntry, DataModel::Nullable<Structs::DayEntryStruct::Type>)                                                            \
    X(CurrentDayEntryDate, DataModel::Nullable<uint32_t>)                                                                          \
    X(NextDayEntryDate, DataModel::Nullable<uint32_t>)

#define COMMODITY_TARIFF_CURRENT_LIST_ATTRIBUTES                                                                                   \
    X(CurrentTariffComponents, DataModel::Nullable<DataModel::List<Structs::TariffComponentStruct::Type>>)                         \
    X(NextTariffComponents, DataModel::Nullable<DataModel::List<Structs::TariffComponentStruct::Type>>)

#define COMMODITY_TARIFF_CURRENT_ATTRIBUTES                                                                                        \
    COMMODITY_TARIFF_CURRENT_SINGLE_ATTRIBUTES                                                                                     \
    COMMODITY_TARIFF_CURRENT_LIST_ATTRIBUTES

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
    class attrName##DataClass : public CommodityTariffAttrsDataMgmt::CTC_BaseDataClass<attrType>                                   \
    {                                                                                                                              \
    public:                                                                                                                        \
        attrName##DataClass() : CTC_BaseDataClass<attrType>(Attributes::attrName::Id) {}                                           \
        ~attrName##DataClass() override = default;                                                                                 \
                                                                                                                                   \
    protected:                                                                                                                     \
        CHIP_ERROR Validate(const ValueType & aValue) const override;                                                              \
    };
COMMODITY_TARIFF_PRIMARY_SCALAR_ATTRS
#undef X

#define X(attrName, attrType)                                                                                                      \
    class attrName##DataClass : public CommodityTariffAttrsDataMgmt::CTC_BaseDataClass<attrType>                                   \
    {                                                                                                                              \
    public:                                                                                                                        \
        attrName##DataClass() : CTC_BaseDataClass<attrType>(Attributes::attrName::Id) {}                                           \
        ~attrName##DataClass() override = default;                                                                                 \
        void CleanupExtEntry(PayloadType & entry)                                                                                  \
        {                                                                                                                          \
            CleanupStructValue(entry);                                                                                             \
        }                                                                                                                          \
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
    std::unordered_set<uint32_t> DayEntryKeyIDs; /* Master - IDs of all given DayEntry items */

    std::unordered_set<uint32_t> DayPatternsDayEntryIDs;    /* IDs mentioned in DayPattern items */
    std::unordered_set<uint32_t> TariffPeriodsDayEntryIDs;  /* IDs mentioned in TariffPeriod items */
    std::unordered_set<uint32_t> IndividualDaysDayEntryIDs; /* IDs mentioned in IndividualDays items */

    /* TariffComponentIDs */
    std::unordered_set<uint32_t> TariffComponentKeyIDs;           /* Master - IDs of all given TariffComponent items */
    std::unordered_set<uint32_t> TariffPeriodsTariffComponentIDs; /* IDs mentioned in TariffPeriods items */

    /* DayPatternsIDs */
    std::unordered_set<uint32_t> DayPatternKeyIDs;             /* Master - IDs of all given DayPattern items */
    std::unordered_set<uint32_t> CalendarPeriodsDayPatternIDs; /* IDs mentioned in CalendarPeriods items */

    BitMask<Feature> mFeature;
    EndpointId mEndpoint;
    bool mAnyHasChanged = false;
};

/**
 * @class Delegate
 * @brief Core tariff data management and processing class
 *
 * This class provides:
 * - Storage for both primary and current attributes
 * - Tariff update processing pipeline
 * - Attribute access methods
 * - Data validation and change management
 */
class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }
    /**
     * @brief Set the current feature map for this tariff instance
     * @param aFeature The current feature map value
     */
    void SetFeatures(BitMask<Feature> aFeature) { mFeature = aFeature; }

    bool HasFeature(Feature aFeature) { return mFeature.Has(aFeature); }

    void SetTariffUpdCb(std::function<void(bool)> cb) { mTariffDataUpdatedCb = cb; }

    /**
     * @brief Process incoming tariff data updates
     *
     * This method implements a three-phase update process:
     * 1. Initial validation (TariffDataUpd_Init)
     * 2. Cross-field validation (TariffDataUpd_CrossValidator)
     * 3. Commit or abort (TariffDataUpd_Commit/Abort)
     */
    void TariffDataUpdate()
    {
        TariffUpdateCtx UpdCtx = { .mFeature = mFeature, .mEndpoint = mEndpointId };

        if (!TariffDataUpd_Init(UpdCtx))
        {
            ChipLogError(NotSpecified, "EGW-CTC: New tariff data rejected due to internal inconsistencies");
        }
        else if (!TariffDataUpd_CrossValidator(UpdCtx))
        {
            ChipLogError(NotSpecified, "EGW-CTC: New tariff data rejected due to some cross-fields inconsistencies");
        }
        else
        {
            TariffDataUpd_Commit();

            if (UpdCtx.mAnyHasChanged)
            {
                ChipLogProgress(NotSpecified, "EGW-CTC: Tariff data applied");

                if (mTariffDataUpdatedCb != nullptr)
                {
                    mTariffDataUpdatedCb(false);
                }
            }
            else
            {
                ChipLogProgress(NotSpecified, "EGW-CTC: Tariff data does not change");
            }

            return;
        }

        TariffDataUpd_Abort();
    }

    // Attribute accessors
#define X(attrName, attrType)                                                                                                      \
    attrType & Get##attrName() { return m##attrName##_MgmtObj.GetValue(); }
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X

#define X(attrName, attrType)                                                                                                      \
    attrName##DataClass & Get##attrName##_MgmtObj() { return m##attrName##_MgmtObj; }
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X

    void CleanupTariffData()
    {
#define X(attrName, attrType) m##attrName##_MgmtObj.Cleanup();
        COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X
        if (mTariffDataUpdatedCb != nullptr)
        {
            mTariffDataUpdatedCb(true);
        }
    }

private:
    // Primary attribute storage and management
#define X(attrName, attrType) attrName##DataClass m##attrName##_MgmtObj{};
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X

    static void TariffDataUpd_AttrChangeCb(uint32_t aAttrId, void * CbCtx)
    {
        TariffUpdateCtx * UpdCtx = (TariffUpdateCtx *) CbCtx;
        ChipLogProgress(NotSpecified, "EGW-CTC: The value for attribute (Id %d) updated", aAttrId);
        MatterReportingAttributeChangeCallback(UpdCtx->mEndpoint, CommodityTariff::Id, aAttrId);
        UpdCtx->mAnyHasChanged = true;
    }

    // Primary attrs update pipeline methods
    bool TariffDataUpd_Init(TariffUpdateCtx & UpdCtx)
    {
#define X(attrName, attrType)                                                                                                      \
    if (m##attrName##_MgmtObj.UpdateBegin(&UpdCtx, TariffDataUpd_AttrChangeCb, true) != CHIP_NO_ERROR)                             \
    {                                                                                                                              \
        return false;                                                                                                              \
    }
        COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X
        return true;
    }

    virtual bool TariffDataUpd_CrossValidator(TariffUpdateCtx & UpdCtx) { return true; }

    void TariffDataUpd_Commit()
    {
#define X(attrName, attrType) m##attrName##_MgmtObj.UpdateCommit();
        COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X
    }

    void TariffDataUpd_Abort()
    {
#define X(attrName, attrType) m##attrName##_MgmtObj.UpdateEnd();
        COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X
    }

protected:
    EndpointId mEndpointId = 0; ///< Associated Matter endpoint ID
    BitMask<Feature> mFeature;
    std::function<void(bool)> mTariffDataUpdatedCb;
};

struct CurrentTariffAttrsCtx
{
    Delegate * mTariffProvider;
    EndpointId mEndpointId;

    std::map<uint32_t, const Structs::DayPatternStruct::Type *> DayPatternsMap;
    std::map<uint32_t, const Structs::DayEntryStruct::Type *> DayEntriesMap;
    std::map<uint32_t, const Structs::TariffComponentStruct::Type *> TariffComponentsMap;

    uint32_t mForwardAlarmTriggerTime;
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
class Instance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    /**
     * @brief Construct a new Commodity Tariff Server instance
     * @param aEndpointId The Matter endpoint ID
     * @param aProvider The data provider delegate
     * @param aFeature Bitmask of supported features
     */
    Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id), CommandHandlerInterface(MakeOptional(aEndpointId), Id),
        mDelegate(aDelegate), mFeature(aFeature)
    {
        /* set the base class delegates endpointId */
        mDelegate.SetEndpointId(aEndpointId);
        mEndpointId = aEndpointId;
        mDelegate.SetTariffUpdCb([this](bool is_erased) { this->TariffDataUpdatedCb(is_erased); });
        mDelegate.SetFeatures(aFeature);
    }

    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

    void TariffComponentUpd_AttrChangeCb(uint32_t aAttrId, void * CbCtx);

    /**
     * @brief Passes the specified time offset value to the context variable that is used to override the real-time stamp.
     * In depends on the time shift value may triggered DaysUpdating or DayEntryUpdating event handling.
     */
    void SetupTimeShiftOffset(uint32_t offset);

private:
    enum class UpdateEventCode
    {
        TariffErased,
        TariffUpdated,
        DaysUpdating,
        DayEntryUpdating
    };

    CurrentTariffAttrsCtx mServerTariffAttrsCtx;

    Delegate & mDelegate;
    BitMask<Feature> mFeature;

    EndpointId mEndpointId;

    // AttributeAccessInterface implementation
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // CommandHandlerInterface implementation
    void InvokeCommand(HandlerContext & handlerContext) override;

    // Command handlers
    void HandleGetDayEntry(HandlerContext & ctx, const Commands::GetDayEntry::DecodableType & commandData);
    void HandleGetTariffComponent(HandlerContext & ctx, const Commands::GetTariffComponent::DecodableType & commandData);

    // Current attributes storage
#define X(attrName, attrType)                                                                                                      \
    attrType m##attrName;                                                                                                          \
    attrType & Get##attrName() { return m##attrName; }                                                                             \
    CHIP_ERROR Set##attrName(attrType & newValue) { return SetValue(m##attrName, newValue, Attributes::attrName::Id); }
    COMMODITY_TARIFF_CURRENT_SINGLE_ATTRIBUTES
#undef X

#define X(attrName, attrType)                                                                                                      \
    TariffComponentsDataClass m##attrName##_MgmtObj{};                                                                             \
    attrType & Get##attrName() { return m##attrName##_MgmtObj.GetValue(); }
    COMMODITY_TARIFF_CURRENT_LIST_ATTRIBUTES
#undef X

    template <typename T>
    CHIP_ERROR SetValue(T & currValue, T & newValue, uint32_t attrId);

    void TariffDataUpdatedCb(bool is_erased);
    void ResetCurrentAttributes();

    // Current attrs (time depended) update methods
    void UpdateCurrentAttrs(UpdateEventCode aEvt);
    void ScheduleTariffActivation(uint32_t delay);
    void ScheduleMidnightUpdate();
    void ScheduleDayEntryUpdate(uint16_t minutesSinceMidnight);
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
