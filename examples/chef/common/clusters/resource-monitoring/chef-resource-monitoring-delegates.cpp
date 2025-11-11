/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-server.h>
#include <lib/core/TLVReader.h>
#include <lib/support/BufferReader.h>
#include <lib/support/BufferWriter.h>
#include <resource-monitoring/chef-resource-monitoring-delegates.h>
#include <utility>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using namespace chip::app::Clusters::ActivatedCarbonFilterMonitoring;
using namespace chip::app::Clusters::HepaFilterMonitoring;
using chip::Protocols::InteractionModel::Status;

const chip::BitMask<ResourceMonitoring::Feature> gHepaFilterFeatureMap(ResourceMonitoring::Feature::kCondition,
                                                                       ResourceMonitoring::Feature::kWarning,
                                                                       ResourceMonitoring::Feature::kReplacementProductList);
const chip::BitMask<ResourceMonitoring::Feature> gActivatedCarbonFeatureMap(ResourceMonitoring::Feature::kCondition,
                                                                            ResourceMonitoring::Feature::kWarning,
                                                                            ResourceMonitoring::Feature::kReplacementProductList);

static std::unique_ptr<ActivatedCarbonFilterMonitoringDelegate> gActivatedCarbonFilterDelegate;
static std::unique_ptr<ResourceMonitoring::ChefResourceMonitorInstance> gActivatedCarbonFilterInstance;

static std::unique_ptr<HepaFilterMonitoringDelegate> gHepaFilterDelegate;
static std::unique_ptr<ResourceMonitoring::ChefResourceMonitorInstance> gHepaFilterInstance;

static ImmutableReplacementProductListManager sReplacementProductListManager;

//-- Activated Carbon Filter Monitoring delegate methods
CHIP_ERROR ActivatedCarbonFilterMonitoringDelegate::Init()
{
    ChipLogDetail(Zcl, "ActivatedCarbonFilterMonitoringDelegate::Init()");
    GetInstance()->SetReplacementProductListManagerInstance(&sReplacementProductListManager);
    return CHIP_NO_ERROR;
}

Status ActivatedCarbonFilterMonitoringDelegate::PreResetCondition()
{
    ChipLogDetail(Zcl, "ActivatedCarbonFilterMonitoringDelegate::PreResetCondition()");
    return Status::Success;
}

Status ActivatedCarbonFilterMonitoringDelegate::PostResetCondition()
{
    ChipLogDetail(Zcl, "ActivatedCarbonFilterMonitoringDelegate::PostResetCondition()");
    return Status::Success;
}

void ActivatedCarbonFilterMonitoring::Shutdown()
{
    gActivatedCarbonFilterInstance.reset();
    gActivatedCarbonFilterDelegate.reset();
}

//-- Hepa Filter Monitoring delegate methods
CHIP_ERROR HepaFilterMonitoringDelegate::Init()
{
    ChipLogDetail(Zcl, "HepaFilterMonitoringDelegate::Init()");
    GetInstance()->SetReplacementProductListManagerInstance(&sReplacementProductListManager);
    return CHIP_NO_ERROR;
}

Status HepaFilterMonitoringDelegate::PreResetCondition()
{
    ChipLogDetail(Zcl, "HepaFilterMonitoringDelegate::PreResetCondition()");
    return Status::Success;
}

Status HepaFilterMonitoringDelegate::PostResetCondition()
{
    ChipLogDetail(Zcl, "HepaFilterMonitoringDelegate::PostResetCondition()");
    return Status::Success;
}

void HepaFilterMonitoring::Shutdown()
{
    gHepaFilterInstance.reset();
    gHepaFilterDelegate.reset();
}

chip::Protocols::InteractionModel::Status
ChefResourceMonitorInstance::ExternalAttributeWrite(const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Success;
    AttributeId attributeId                 = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case HepaFilterMonitoring::Attributes::Condition::Id: {
        uint8_t newCondition = *(uint8_t *) buffer;
        ret                  = UpdateCondition(newCondition);
    }
    break;
    case HepaFilterMonitoring::Attributes::ChangeIndication::Id: {
        ResourceMonitoring::ChangeIndicationEnum newIndication =
            static_cast<ResourceMonitoring::ChangeIndicationEnum>(*(uint8_t *) buffer);
        ret = UpdateChangeIndication(newIndication);
    }
    break;
    case HepaFilterMonitoring::Attributes::InPlaceIndicator::Id: {
        bool newInPlaceIndicator = *(bool *) buffer;
        ret                      = UpdateInPlaceIndicator(newInPlaceIndicator);
    }
    break;
    case HepaFilterMonitoring::Attributes::LastChangedTime::Id: {
        // We already know the input is a buffer started with a uint16_t as the length
        chip::Encoding::LittleEndian::Reader bufReader(buffer, sizeof(uint16_t));
        uint16_t tlvLen;
        VerifyOrReturnError(CHIP_NO_ERROR == bufReader.Read16(&tlvLen).StatusCode(),
                            Protocols::InteractionModel::Status::UnsupportedWrite);

        // Read from TLV
        uint32_t newValue = 0;
        chip::TLV::TLVReader tlvReader;
        tlvReader.Init(buffer + sizeof(uint16_t), tlvLen);
        tlvReader.Next();
        tlvReader.Get(newValue);
        DataModel::Nullable<uint32_t> newLastChangedTime = DataModel::MakeNullable(newValue);
        ret                                              = UpdateLastChangedTime(newLastChangedTime);
    }
    break;
    case HepaFilterMonitoring::Attributes::DegradationDirection::Id:
    default: {
        ChipLogError(Zcl, "Unsupported External Attribute Write: %d", static_cast<int>(attributeId));
        ret = Protocols::InteractionModel::Status::UnsupportedWrite;
    }
    break;
    }

    return ret;
}

void emberAfActivatedCarbonFilterMonitoringClusterInitCallback(chip::EndpointId endpoint)
{
    VerifyOrDie(!gActivatedCarbonFilterInstance && !gActivatedCarbonFilterDelegate);

    bool bResetConditionCommandSupported = true; // The ResetCondition command is supported by the ResourceMonitor cluster
    gActivatedCarbonFilterDelegate       = std::make_unique<ActivatedCarbonFilterMonitoringDelegate>();
    gActivatedCarbonFilterInstance       = std::make_unique<ResourceMonitoring::ChefResourceMonitorInstance>(
        gActivatedCarbonFilterDelegate.get(), endpoint, ActivatedCarbonFilterMonitoring::Id,
        static_cast<uint32_t>(gActivatedCarbonFeatureMap.Raw()), ResourceMonitoring::DegradationDirectionEnum::kDown,
        bResetConditionCommandSupported);
    gActivatedCarbonFilterInstance->Init();
}

chip::Protocols::InteractionModel::Status
chefResourceMonitoringExternalWriteCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                            const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Success;
    AttributeId attributeId                 = attributeMetadata->attributeId;
    ChipLogProgress(Zcl, "chefResourceMonitoringExternalWriteCallback EP: %d, Cluster: %04x, Att: %04x", static_cast<int>(endpoint),
                    static_cast<int>(clusterId), static_cast<int>(attributeId));

    switch (clusterId)
    {
    case HepaFilterMonitoring::Id:
        ret = gHepaFilterInstance->ExternalAttributeWrite(attributeMetadata, buffer);
        break;
    case ActivatedCarbonFilterMonitoring::Id:
        ret = gActivatedCarbonFilterInstance->ExternalAttributeWrite(attributeMetadata, buffer);
        break;
    default:
        ret = Protocols::InteractionModel::Status::UnsupportedWrite;
        break;
    }

    return ret;
}

chip::Protocols::InteractionModel::Status
chefResourceMonitoringExternalReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                           const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                           uint16_t maxReadLength)
{
    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Success;
    AttributeId attributeId                 = attributeMetadata->attributeId;
    ChipLogProgress(Zcl, "chefResourceMonitoringExternalReadCallback EP: %d, Cluster: %d, Att: %d", static_cast<int>(endpoint),
                    static_cast<int>(clusterId), static_cast<int>(attributeId));

    switch (clusterId)
    {
    case HepaFilterMonitoring::Id:
        ret = gHepaFilterInstance->ExternalAttributeRead(attributeMetadata, buffer, maxReadLength);
        break;
    case ActivatedCarbonFilterMonitoring::Id:
        ret = gActivatedCarbonFilterInstance->ExternalAttributeRead(attributeMetadata, buffer, maxReadLength);
        break;
    default:
        ret = Protocols::InteractionModel::Status::UnsupportedRead;
        break;
    }

    return ret;
}

chip::Protocols::InteractionModel::Status
ChefResourceMonitorInstance::ExternalAttributeRead(const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                                   uint16_t maxReadLength)
{
    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Success;
    AttributeId attributeId                 = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case HepaFilterMonitoring::Attributes::Condition::Id: {
        *buffer = GetCondition();
    }
    break;
    case HepaFilterMonitoring::Attributes::ChangeIndication::Id: {
        ResourceMonitoring::ChangeIndicationEnum changeIndication = GetChangeIndication();
        // The underlying type of ResourceMonitoring::ChangeIndicationEnum is uint8_t
        *buffer = to_underlying(changeIndication);
    }
    break;
    case HepaFilterMonitoring::Attributes::InPlaceIndicator::Id: {
        *(bool *) buffer = GetInPlaceIndicator();
    }
    break;
    case HepaFilterMonitoring::Attributes::LastChangedTime::Id: {
        // Only LastChangedTime needs to handle Endianness
        DataModel::Nullable<uint32_t> lastChangedTime = GetLastChangedTime();
        chip::Encoding::LittleEndian::BufferWriter bufWriter(buffer, sizeof(uint16_t));

        bufWriter.Put32(lastChangedTime.IsNull() ? 0 : lastChangedTime.Value());
    }
    break;
    case HepaFilterMonitoring::Attributes::DegradationDirection::Id: {
        ResourceMonitoring::DegradationDirectionEnum degradationDirection = GetDegradationDirection();
        // The underlying type of ResourceMonitoring::DegradationDirectionEnum is uint8_t
        *buffer = to_underlying(degradationDirection);
    }
    break;
    default:
        ChipLogError(Zcl, "Unsupported External Attribute Read: %d", static_cast<int>(attributeId));
        ret = Protocols::InteractionModel::Status::UnsupportedRead;
        break;
    }

    return ret;
}

void emberAfHepaFilterMonitoringClusterInitCallback(chip::EndpointId endpoint)
{
    VerifyOrDie(!gHepaFilterInstance && !gHepaFilterDelegate);

    bool bResetConditionCommandSupported = true; // The ResetCondition command is supported by the ResourceMonitor cluster
    gHepaFilterDelegate                  = std::make_unique<HepaFilterMonitoringDelegate>();
    gHepaFilterInstance                  = std::make_unique<ResourceMonitoring::ChefResourceMonitorInstance>(
        gHepaFilterDelegate.get(), endpoint, HepaFilterMonitoring::Id, static_cast<uint32_t>(gHepaFilterFeatureMap.Raw()),
        ResourceMonitoring::DegradationDirectionEnum::kDown, bResetConditionCommandSupported);
    gHepaFilterInstance->Init();
}

CHIP_ERROR ImmutableReplacementProductListManager::Next(ReplacementProductStruct & item)
{
    if (mIndex >= kReplacementProductListMaxSize)
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    switch (mIndex)
    {
    case 0:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kUpc);
        item.SetProductIdentifierValue(CharSpan::fromCharString("111112222233"));
        break;
    case 1:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kGtin8);
        item.SetProductIdentifierValue(CharSpan::fromCharString("gtin8xxx"));
        break;
    case 2:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kEan);
        item.SetProductIdentifierValue(CharSpan::fromCharString("4444455555666"));
        break;
    case 3:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kGtin14);
        item.SetProductIdentifierValue(CharSpan::fromCharString("gtin14xxxxxxxx"));
        break;
    case 4:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kOem);
        item.SetProductIdentifierValue(CharSpan::fromCharString("oem20xxxxxxxxxxxxxxx"));
        break;
    default:
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    mIndex++;
    return CHIP_NO_ERROR;
}
