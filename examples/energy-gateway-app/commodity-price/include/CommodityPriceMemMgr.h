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

#include <app/clusters/commodity-price-server/commodity-price-server.h>
#include <lib/core/CHIPError.h>

#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityPrice {

/*
 * The PriceForecast data structure defined as:
 *
 * DataModel::List<const Structs::CommodityPriceStruct::Type> priceForecast;
 *
 * contains a list of up to (50) CommodityPriceStructs which in turn contains a
 * list of CommodityPriceComponentStructs (max 10).
 *
 * This means that somewhere the following memory needs to be allocated in the
 * case where the Components is at its maximum size:
 *
 * CommodityPriceStruct::Type mPriceEntries[kMaxCommodityPriceEntries][kMaxComponentsPerPriceEntry]
 *
 * This is 1680B. //TODO finish this comment block
 *
 * However it is likely the number of 'Components' sent will be considerably
 * less than 10. To avoid allocating the maximum possible Target size, each
 * List<CommodityPriceComponentStruct> is allocated separately. This class
 * handles that allocation.
 *
 * When iterating through the price entries, an index in this list is
 * kept and the PriceForecastMemMgr::PreparePriceEntry must be called so
 * this object knows which price entry it is tracking. This will free any
 * previous memory allocated for the price entry in this object.
 *
 * There are then three usage cases:
 *
 * 1. When loading the Target from persistent storage. In this scenario, it is
 *    not known upfront how many chargingTargets are associated with this day
 *    schedule so PriceForecastMemMgr::AddChargingTarget() needs to be called
 *    as each individual chargingTarget is loaded from persistent data.
 *
 *    Once the chargingTargets for the day schedule have been loaded,
 *    PriceForecastMemMgr::AllocAndCopy() is called to allocate the memory to
 *    store the chargingTargets and the chargingTargets are copied.
 *
 * 2. When updating a Target and a day schedule is unaffected, the
 *    chargingTargets associated with day schedule need copying. The following
 *    should be called:
 *
 *    PriceForecastMemMgr::AllocAndCopy(const DataModel::List<const Structs::ChargingTargetStruct::Type> & chargingTargets)
 *
 * 3. When in SetTargets, a new list of chargingTargets needs to be added to a
 *    day schedule, the following should be called:
 *
 *    PriceForecastMemMgr::AllocAndCopy(const DataModel::DecodableList<Structs::ChargingTargetStruct::DecodableType> &
 * chargingTargets)
 *
 * Having allocated and copied the chargingTargets accordingly, they can be
 * added to a DataModel::List<const Structs::ChargingTargetStruct::Type> as
 * follows:
 *
 * chargingTargetsList = DataModel::List<Structs::ChargingTargetStruct::Type>(PriceForecastMemMgr::GetChargingTargets(),
 * PriceForecastMemMgr::GetNumDailyChargingTargets());
 *
 * All memory allocated by this object is released When the
 * PriceForecastMemMgr destructor is called.
 *
 */

class PriceForecastMemMgr
{
public:
    PriceForecastMemMgr();
    ~PriceForecastMemMgr();

    /**
     * @brief This method prepares a new price entry. Subsequent calls to
     *        GetPriceComponents and the AllocAndCopy methods below will
     *        reference this price entry.
     *
     * @param priceEntryIdx  - The new price entry index
     */
    void PreparePriceEntry(uint16_t priceEntryIdx);

    /**
     * @brief Called as each individual price component is loaded.
     *
     * @param component - The price component that will be added into the
     *                    current priceEntry
     */
    void AddPriceComponent(const CommodityPrice::Structs::CommodityPriceComponentStruct::Type & component);

    /**
     * @brief Called to allocate and copy the priceComponent added via
     *        AddPriceComponent into the current price entry as set by
     *        PreparePriceEntry().
     */
    CHIP_ERROR AllocAndCopy();

    /**
     * @brief Called to allocate and copy the priceComponent list into the
     *        current price entry as set set by PreparePriceEntry(). If an
     *        attempt is made to add more than kMaxComponentsPerPriceEntry
     *        priceComponents for the current priceEntry, then the price
     *        component is not added and an error message is printed.
     *
     * @param priceComponents  - The price Component list to add into the current
     *                           day schedule
     */
    CHIP_ERROR AllocAndCopy(const DataModel::List<const Structs::CommodityPriceComponentStruct::Type> & priceComponents);

    /**
     * @brief Called to allocate and copy the priceComponent list into the
     *        current price entry as set set by PreparePriceEntry().
     *
     * @param priceComponents  - The price Component list to add into the current
     *                           day schedule
     */
    CHIP_ERROR
    AllocAndCopy(const DataModel::DecodableList<Structs::CommodityPriceComponentStruct::DecodableType> & priceComponents);

    /**
     * @brief Returns the list of priceComponents associated with the current price
     * entry.
     *
     * @return Pointer to list of priceComponents associated with the current price
     * entry.
     */
    CommodityPrice::Structs::CommodityPriceComponentStruct::Type * GetPriceComponents() const;

    /**
     * @brief Returns the number of priceComponents associated with current price entry.
     *
     * @return Returns the number of priceComponents associated with current price entry.
     */
    uint16_t GetNumPriceComponents() const;

private:
    CommodityPrice::Structs::CommodityPriceComponentStruct::Type * mpListOfPriceEntries[kMaxCommodityPriceEntries];
    CommodityPrice::Structs::CommodityPriceComponentStruct::Type mPriceComponents[kMaxComponentsPerPriceEntry];
    uint16_t mPriceEntryIdx;
    uint16_t mNumPriceComponents;
};

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
