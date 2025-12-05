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
#include <array>
#include <cstdint>
#include <functional>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

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

    void SetTariffUpdCb(std::function<void(bool, const AttributeId *, size_t)> cb) { mTariffDataUpdatedCb = cb; }

    // Attribute accessors - manually defined for each attribute
    DataModel::Nullable<Globals::TariffUnitEnum> & GetTariffUnit() { return mTariffUnit_MgmtObj.GetValue(); }
    DataModel::Nullable<uint32_t> & GetStartDate() { return mStartDate_MgmtObj.GetValue(); }
    DataModel::Nullable<int16_t> & GetDefaultRandomizationOffset() { return mDefaultRandomizationOffset_MgmtObj.GetValue(); }
    DataModel::Nullable<DayEntryRandomizationTypeEnum> & GetDefaultRandomizationType()
    {
        return mDefaultRandomizationType_MgmtObj.GetValue();
    }
    DataModel::Nullable<Structs::TariffInformationStruct::Type> & GetTariffInfo() { return mTariffInfo_MgmtObj.GetValue(); }
    DataModel::Nullable<DataModel::List<Structs::DayEntryStruct::Type>> & GetDayEntries() { return mDayEntries_MgmtObj.GetValue(); }
    DataModel::Nullable<DataModel::List<Structs::DayPatternStruct::Type>> & GetDayPatterns()
    {
        return mDayPatterns_MgmtObj.GetValue();
    }
    DataModel::Nullable<DataModel::List<Structs::TariffComponentStruct::Type>> & GetTariffComponents()
    {
        return mTariffComponents_MgmtObj.GetValue();
    }
    DataModel::Nullable<DataModel::List<Structs::TariffPeriodStruct::Type>> & GetTariffPeriods()
    {
        return mTariffPeriods_MgmtObj.GetValue();
    }
    DataModel::Nullable<DataModel::List<Structs::DayStruct::Type>> & GetIndividualDays()
    {
        return mIndividualDays_MgmtObj.GetValue();
    }
    DataModel::Nullable<DataModel::List<Structs::CalendarPeriodStruct::Type>> & GetCalendarPeriods()
    {
        return mCalendarPeriods_MgmtObj.GetValue();
    }

    enum CommodityTariffAttrTypeEnum
    {
        kTariffUnit,
        kStartDate,
        kDefaultRandomizationOffset,
        kDefaultRandomizationType,
        kTariffInfo,
        kDayEntries,
        kDayPatterns,
        kTariffComponents,
        kTariffPeriods,
        kIndividualDays,
        kCalendarPeriods,
        kAttrMax
    };

    /**
     * @brief Get the management object for a given attribute type enum
     * @param aType The attribute type enum to retrieve
     * @return Reference to the corresponding management object
     */
    CommodityTariffAttrsDataMgmt::CTC_BaseDataClassBase & GetMgmtObj(CommodityTariffAttrTypeEnum aType);

    /**
     * @brief Process incoming tariff data updates
     *
     * This method implements a three-phase update process:
     * 1. Initial validation (TariffDataUpd_Init)
     * 2. Cross-field validation (TariffDataUpd_CrossValidator)
     * 3. Commit or abort (TariffDataUpd_Commit/Abort)
     */
    void TariffDataUpdate(uint32_t aNowTimestamp);

    // Primary attrs update pipeline methods
    CHIP_ERROR TariffDataUpd_Init(TariffUpdateCtx & UpdCtx);
    CHIP_ERROR TariffDataUpd_CrossValidator(TariffUpdateCtx & UpdCtx);
    void TariffDataUpd_Finish(bool is_success);

    void TryToactivateDelayedTariff(uint32_t now);
    void CleanupTariffData();

private:
    // Primary attribute storage and management
    TariffUnitDataClass mTariffUnit_MgmtObj{};
    StartDateDataClass mStartDate_MgmtObj{};
    DefaultRandomizationOffsetDataClass mDefaultRandomizationOffset_MgmtObj{};
    DefaultRandomizationTypeDataClass mDefaultRandomizationType_MgmtObj{};
    TariffInfoDataClass mTariffInfo_MgmtObj{};
    DayEntriesDataClass mDayEntries_MgmtObj{};
    DayPatternsDataClass mDayPatterns_MgmtObj{};
    TariffComponentsDataClass mTariffComponents_MgmtObj{};
    TariffPeriodsDataClass mTariffPeriods_MgmtObj{};
    IndividualDaysDataClass mIndividualDays_MgmtObj{};
    CalendarPeriodsDataClass mCalendarPeriods_MgmtObj{};

protected:
    EndpointId mEndpointId = 0; ///< Associated Matter endpoint ID
    BitMask<Feature> mFeature;
    std::function<void(bool, const AttributeId *, size_t)> mTariffDataUpdatedCb;
    bool DelayedTariffUpdateIsActive = false;
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
        mDelegate.SetTariffUpdCb([this](bool is_erased, const AttributeId * updatedAttrIds, size_t count) {
            this->TariffDataUpdatedCb(is_erased, updatedAttrIds, count);
        });
        mDelegate.SetFeatures(aFeature);
    }

    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

    void AttributeUpdCb(AttributeId aAttrId)
    {
        ChipLogProgress(NotSpecified, "EGW-CTC: The value for attribute (Id %" PRIu32 ") updated", aAttrId);
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, aAttrId);
    }

    void TariffTimeAttrsSync();

    /**
     * @struct CurrentTariffAttrsCtx
     * @brief Context for current tariff attributes
     */
    struct CurrentTariffAttrsCtx
    {
        Delegate * mTariffProvider;
        EndpointId mEndpointId;
    };

private:
    Delegate & mDelegate;
    BitMask<Feature> mFeature;

    EndpointId mEndpointId;

    // Current attributes storage
    DataModel::Nullable<Structs::DayStruct::Type> mCurrentDay;
    DataModel::Nullable<Structs::DayStruct::Type> mNextDay;
    DataModel::Nullable<Structs::DayEntryStruct::Type> mCurrentDayEntry;
    DataModel::Nullable<Structs::DayEntryStruct::Type> mNextDayEntry;
    DataModel::Nullable<uint32_t> mCurrentDayEntryDate;
    DataModel::Nullable<uint32_t> mNextDayEntryDate;

    TariffComponentsDataClass mCurrentTariffComponents_MgmtObj{};
    TariffComponentsDataClass mNextTariffComponents_MgmtObj{};

    // Attribute accessors
    DataModel::Nullable<Structs::DayStruct::Type> & GetCurrentDay() { return mCurrentDay; }
    DataModel::Nullable<Structs::DayStruct::Type> & GetNextDay() { return mNextDay; }
    DataModel::Nullable<Structs::DayEntryStruct::Type> & GetCurrentDayEntry() { return mCurrentDayEntry; }
    DataModel::Nullable<Structs::DayEntryStruct::Type> & GetNextDayEntry() { return mNextDayEntry; }
    DataModel::Nullable<uint32_t> & GetCurrentDayEntryDate() { return mCurrentDayEntryDate; }
    DataModel::Nullable<uint32_t> & GetNextDayEntryDate() { return mNextDayEntryDate; }

    template <typename T>
    CHIP_ERROR SetValue(T & currValue, T & newValue, uint32_t attrId);

    CHIP_ERROR SetCurrentDay(DataModel::Nullable<Structs::DayStruct::Type> & newValue)
    {
        return SetValue(mCurrentDay, newValue, Attributes::CurrentDay::Id);
    }
    CHIP_ERROR SetNextDay(DataModel::Nullable<Structs::DayStruct::Type> & newValue)
    {
        return SetValue(mNextDay, newValue, Attributes::NextDay::Id);
    }
    CHIP_ERROR SetCurrentDayEntry(DataModel::Nullable<Structs::DayEntryStruct::Type> & newValue)
    {
        return SetValue(mCurrentDayEntry, newValue, Attributes::CurrentDayEntry::Id);
    }
    CHIP_ERROR SetNextDayEntry(DataModel::Nullable<Structs::DayEntryStruct::Type> & newValue)
    {
        return SetValue(mNextDayEntry, newValue, Attributes::NextDayEntry::Id);
    }
    CHIP_ERROR SetCurrentDayEntryDate(DataModel::Nullable<uint32_t> & newValue)
    {
        return SetValue(mCurrentDayEntryDate, newValue, Attributes::CurrentDayEntryDate::Id);
    }
    CHIP_ERROR SetNextDayEntryDate(DataModel::Nullable<uint32_t> & newValue)
    {
        return SetValue(mNextDayEntryDate, newValue, Attributes::NextDayEntryDate::Id);
    }

    DataModel::Nullable<DataModel::List<Structs::TariffComponentStruct::Type>> & GetCurrentTariffComponents()
    {
        return mCurrentTariffComponents_MgmtObj.GetValue();
    }
    DataModel::Nullable<DataModel::List<Structs::TariffComponentStruct::Type>> & GetNextTariffComponents()
    {
        return mNextTariffComponents_MgmtObj.GetValue();
    }

    CurrentTariffAttrsCtx mServerTariffAttrsCtx;

    void TariffDataUpdatedCb(bool is_erased, const AttributeId * aUpdatedAttrIds, size_t aCount);
    void ResetCurrentAttributes();

    // AttributeAccessInterface implementation
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // CommandHandlerInterface implementation
    void InvokeCommand(HandlerContext & handlerContext) override;

    // Command handlers
    void HandleGetDayEntry(HandlerContext & ctx, const Commands::GetDayEntry::DecodableType & commandData);
    void HandleGetTariffComponent(HandlerContext & ctx, const Commands::GetTariffComponent::DecodableType & commandData);

    // Current attrs (time depended) update methods
    void InitCurrentAttrs();
    CHIP_ERROR UpdateCurrentAttrs();
    CHIP_ERROR UpdateDayInformation(uint32_t now);
    CHIP_ERROR UpdateDayEntryInformation(uint32_t now);
    void DeinitCurrentAttrs();
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
