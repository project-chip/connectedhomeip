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

#include <CommodityTariffSamples.h>
#include <app/clusters/commodity-tariff-server/commodity-tariff-server.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {
static constexpr uint32_t kTimerPollIntervalInSec = 30;
using chip::Protocols::InteractionModel::Status;

class CommodityTariffDelegate : public CommodityTariff::Delegate
{
public:
    CommodityTariffDelegate();
    ~CommodityTariffDelegate() = default;

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
};

class CommodityTariffInstance : public CommodityTariff::Instance
{
public:
    CommodityTariffInstance(EndpointId aEndpointId, CommodityTariffDelegate & aDelegate, Feature aFeature) :
        CommodityTariff::Instance(aEndpointId, aDelegate, aFeature)
    {
        mCommodityTariffDelegate = &aDelegate;
    }

    // Delete copy constructor and assignment operator.
    CommodityTariffInstance(const CommodityTariffInstance &)             = delete;
    CommodityTariffInstance(const CommodityTariffInstance &&)            = delete;
    CommodityTariffInstance & operator=(const CommodityTariffInstance &) = delete;

    CHIP_ERROR Init();
    void Shutdown();
    CHIP_ERROR AppInit();

    CommodityTariffDelegate * GetDelegate() { return mCommodityTariffDelegate; };

private:
    CommodityTariffDelegate * mCommodityTariffDelegate;

    // Private methods for tariff time management
    void ScheduleTariffTimeUpdate();
    void CancelTariffTimeUpdate();
    void TariffTimeUpdCb();
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
