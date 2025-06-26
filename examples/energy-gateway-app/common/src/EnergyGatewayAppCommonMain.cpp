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

#include "EnergyGatewayAppCommonMain.h"
#include "CommodityPriceMain.h"
#include "ElectricalGridConditionsMain.h"
#include "MeterIdentificationInstance.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/data-model/Nullable.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;

// Please refer to https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces
constexpr const uint8_t kNamespaceCommodityTariffCommodity = 0x0D;

// CommodityTariffCommodity Namespace: 0x0D
constexpr const uint8_t kTagElectricalEnergy = 0x00;
// -- Uncomment to use these - commented out to avoid unused warning
// constexpr const uint8_t kTagNaturalGas       = 0x01;
// constexpr const uint8_t kTagWater            = 0x02;

constexpr const uint8_t kNamespaceCommodityTariffChronology = 0x0B;
// CommodityTariffChronology Namespace: 0x0B
constexpr const uint8_t kTagCurrent = 0x00;
// -- Uncomment to use these - commented out to avoid unused warning
// constexpr const uint8_t kTagPrevious = 0x01;
// constexpr const uint8_t kUpcoming    = 0x02;

constexpr const uint8_t kNamespaceCommodityTariffFlow = 0x13;
// CommodityTariffFlow Namespace: 0x13
constexpr const uint8_t kTagImport = 0x00;
// -- Uncomment to use these - commented out to avoid unused warning
// constexpr const uint8_t kTagExport = 0x01;

const Clusters::Descriptor::Structs::SemanticTagStruct::Type electricalEnergyTariffTagList[] = {
    { .namespaceID = kNamespaceCommodityTariffCommodity, .tag = kTagElectricalEnergy },
    { .namespaceID = kNamespaceCommodityTariffChronology, .tag = kTagCurrent },
    { .namespaceID = kNamespaceCommodityTariffFlow, .tag = kTagImport }
};

/*
 *  @brief  Creates a Delegate and Instance for CommodityPrice clusters
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
void ElectricalEnergyTariffInit()
{
    EndpointId kElectricalEnergyTariffEndpointId = 1;

    VerifyOrDie(CommodityPriceInit(kElectricalEnergyTariffEndpointId) == CHIP_NO_ERROR);
    VerifyOrDie(ElectricalGridConditionsInit(kElectricalEnergyTariffEndpointId) == CHIP_NO_ERROR);

    // set the descriptor TagList to include "ElectricalEnergy" and "Current" (to indicate the ActiveTariff)
    SetTagList(kElectricalEnergyTariffEndpointId,
               Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(electricalEnergyTariffTagList));
}

void ElectricalEnergyTariffShutdown()
{
    ChipLogDetail(AppServer, "Energy Gateway App : ElectricalEnergyTariffShutdown()");
}
