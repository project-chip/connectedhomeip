/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *
 */

#include "basic.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/EventLogging.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#include <platform/PlatformManager.h>

#include <cstddef>
#include <cstring>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Basic;
using namespace chip::DeviceLayer;

namespace {

class PlatformMgrDelegate : public DeviceLayer::PlatformManagerDelegate
{
    // Gets called by the current Node after completing a boot or reboot process.
    void OnStartUp(uint32_t softwareVersion) override
    {
        ChipLogProgress(Zcl, "PlatformMgrDelegate: OnStartUp");

        ForAllEndpointsWithServerCluster(
            Basic::Id,
            [](EndpointId endpoint, intptr_t context) -> Loop {
                // If Basic cluster is implemented on this endpoint
                Events::StartUp::Type event{ static_cast<uint32_t>(context) };
                EventNumber eventNumber;

                if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber, EventOptions::Type::kUrgent))
                {
                    ChipLogError(Zcl, "PlatformMgrDelegate: Failed to record StartUp event");
                }

                return Loop::Continue;
            },
            static_cast<intptr_t>(softwareVersion));
    }

    // Gets called by the current Node prior to any orderly shutdown sequence on a best-effort basis.
    void OnShutDown() override
    {
        ChipLogProgress(Zcl, "PlatformMgrDelegate: OnShutDown");

        ForAllEndpointsWithServerCluster(Basic::Id, [](EndpointId endpoint, intptr_t context) -> Loop {
            // If Basic cluster is implemented on this endpoint
            Events::ShutDown::Type event;
            EventNumber eventNumber;

            if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
            {
                ChipLogError(Zcl, "PlatformMgrDelegate: Failed to record ShutDown event");
            }

            return Loop::Continue;
        });
    }
};

PlatformMgrDelegate gPlatformMgrDelegate;

class BasicAttrAccess : public app::AttributeAccessInterface
{
public:
    BasicAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), app::Clusters::Basic::Id) {}
    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override
    {
        if (aPath.mAttributeId == app::Clusters::Basic::Attributes::NodeLabel::Id)
        {
            char nodeLabel[DeviceLayer::ConfigurationManager::kMaxNodeLabelLength + 1];
            if (ConfigurationMgr().GetNodeLabel(nodeLabel, sizeof(nodeLabel)) == CHIP_NO_ERROR)
            {
                CharSpan span(nodeLabel, strlen(nodeLabel));
                aEncoder.Encode(span);
            }
        }
        else if (aPath.mAttributeId == app::Clusters::Basic::Attributes::Location::Id)
        {
            char location[DeviceLayer::ConfigurationManager::kMaxLocationLength + 1];
            size_t codeLen = 0;
            if (ConfigurationMgr().GetCountryCode(location, sizeof(location), codeLen) == CHIP_NO_ERROR && codeLen > 0)
            {
                CharSpan span(location, codeLen);
                aEncoder.Encode(span);
            }
            else
            {
                // "XX" is the default value of the location attribute
                aEncoder.Encode(CharSpan("XX", strlen("XX")));
            }
        }
        else if (aPath.mAttributeId == app::Clusters::Basic::Attributes::LocalConfigDisabled::Id)
        {
            bool localConfigDisabled;
            if (ConfigurationMgr().GetLocalConfigDisabled(localConfigDisabled) == CHIP_NO_ERROR)
            {
                aEncoder.Encode(localConfigDisabled);
            }
        }

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Write(const app::ConcreteDataAttributePath & aPath, app::AttributeValueDecoder & aDecoder) override
    {
        CHIP_ERROR err;
        if (aPath.mAttributeId == app::Clusters::Basic::Attributes::NodeLabel::Id)
        {
            CharSpan span;
            err = aDecoder.Decode(span);
            if (err == CHIP_NO_ERROR)
            {
                ConfigurationMgr().StoreNodeLabel(span.data(), span.size());
            }
        }
        else if (aPath.mAttributeId == app::Clusters::Basic::Attributes::Location::Id)
        {
            CharSpan span;
            err = aDecoder.Decode(span);
            if (err == CHIP_NO_ERROR)
            {
                ConfigurationMgr().StoreCountryCode(span.data(), span.size());
            }
        }
        else if (aPath.mAttributeId == app::Clusters::Basic::Attributes::LocalConfigDisabled::Id)
        {
            bool localConfigDisabled;
            err = aDecoder.Decode(localConfigDisabled);
            if (err == CHIP_NO_ERROR)
            {
                ConfigurationMgr().StoreLocalConfigDisabled(localConfigDisabled);
            }
        }

        return CHIP_NO_ERROR;
    }
};

static BasicAttrAccess gBasicAttrAttrAccess;

} // anonymous namespace

void emberAfBasicClusterServerInitCallback(chip::EndpointId endpoint)
{
    EmberAfStatus status;

    char vendorName[DeviceLayer::ConfigurationManager::kMaxVendorNameLength + 1];
    if (ConfigurationMgr().GetVendorName(vendorName, sizeof(vendorName)) == CHIP_NO_ERROR)
    {
        status = Attributes::VendorName::Set(endpoint, chip::CharSpan(vendorName, strlen(vendorName)));
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Vendor Name: 0x%02x", status));
    }

    uint16_t vendorId;
    if (ConfigurationMgr().GetVendorId(vendorId) == CHIP_NO_ERROR)
    {
        status = Attributes::VendorID::Set(endpoint, vendorId);
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Vendor Id: 0x%02x", status));
    }

    char productName[DeviceLayer::ConfigurationManager::kMaxProductNameLength + 1];
    if (ConfigurationMgr().GetProductName(productName, sizeof(productName)) == CHIP_NO_ERROR)
    {
        status = Attributes::ProductName::Set(endpoint, chip::CharSpan(productName, strlen(productName)));
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Product Name: 0x%02x", status));
    }

    uint16_t productId;
    if (ConfigurationMgr().GetProductId(productId) == CHIP_NO_ERROR)
    {
        status = Attributes::ProductID::Set(endpoint, productId);
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Product Id: 0x%02x", status));
    }

    char hardwareVersionString[DeviceLayer::ConfigurationManager::kMaxHardwareVersionStringLength + 1];
    if (ConfigurationMgr().GetHardwareVersionString(hardwareVersionString, sizeof(hardwareVersionString)) == CHIP_NO_ERROR)
    {
        status = Attributes::HardwareVersionString::Set(endpoint, CharSpan(hardwareVersionString, strlen(hardwareVersionString)));
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Hardware Version String: 0x%02x", status));
    }

    uint16_t hardwareVersion;
    if (ConfigurationMgr().GetHardwareVersion(hardwareVersion) == CHIP_NO_ERROR)
    {
        status = Attributes::HardwareVersion::Set(endpoint, hardwareVersion);
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Hardware Version: 0x%02x", status));
    }

    char softwareVersionString[DeviceLayer::ConfigurationManager::kMaxSoftwareVersionLength + 1];
    if (ConfigurationMgr().GetSoftwareVersionString(softwareVersionString, sizeof(softwareVersionString)) == CHIP_NO_ERROR)
    {
        status = Attributes::SoftwareVersionString::Set(endpoint, CharSpan(softwareVersionString, strlen(softwareVersionString)));
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Software Version String: 0x%02x", status));
    }

    uint16_t softwareVersion;
    if (ConfigurationMgr().GetSoftwareVersion(softwareVersion) == CHIP_NO_ERROR)
    {
        status = Attributes::SoftwareVersion::Set(endpoint, softwareVersion);
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Software Version: 0x%02x", status));
    }

    char serialNumberString[DeviceLayer::ConfigurationManager::kMaxSerialNumberLength + 1];
    if (ConfigurationMgr().GetSerialNumber(serialNumberString, sizeof(serialNumberString)) == CHIP_NO_ERROR)
    {
        status = Attributes::SerialNumber::Set(endpoint, CharSpan(serialNumberString, strlen(serialNumberString)));
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Serial Number String: 0x%02x", status));
    }

    char manufacturingDateString[DeviceLayer::ConfigurationManager::kMaxManufacturingDateLength + 1];
    uint16_t manufacturingYear;
    uint8_t manufacturingMonth;
    uint8_t manufacturingDayOfMonth;
    if (ConfigurationMgr().GetManufacturingDate(manufacturingYear, manufacturingMonth, manufacturingDayOfMonth) == CHIP_NO_ERROR)
    {
        snprintf(manufacturingDateString, sizeof(manufacturingDateString), "%04" PRIu16 "-%02" PRIu16 "-%02" PRIu16,
                 manufacturingYear, manufacturingMonth, manufacturingDayOfMonth);
        status = Attributes::ManufacturingDate::Set(endpoint, CharSpan(manufacturingDateString, strlen(manufacturingDateString)));
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status,
                   ChipLogError(Zcl, "Error setting Manufacturing Date String: 0x%02x", status));
    }

    char partNumber[DeviceLayer::ConfigurationManager::kMaxPartNumberLength + 1];
    if (ConfigurationMgr().GetPartNumber(partNumber, sizeof(partNumber)) == CHIP_NO_ERROR)
    {
        status = Attributes::PartNumber::Set(endpoint, CharSpan(partNumber, strlen(partNumber)));
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Part Number: 0x%02x", status));
    }

    char productURL[DeviceLayer::ConfigurationManager::kMaxProductURLLength + 1];
    if (ConfigurationMgr().GetProductURL(productURL, sizeof(productURL)) == CHIP_NO_ERROR)
    {
        status = Attributes::ProductURL::Set(endpoint, CharSpan(productURL, strlen(productURL)));
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Product URL: 0x%02x", status));
    }

    char productLabel[DeviceLayer::ConfigurationManager::kMaxProductURLLength + 1];
    if (ConfigurationMgr().GetProductLabel(productLabel, sizeof(productLabel)) == CHIP_NO_ERROR)
    {
        status = Attributes::ProductLabel::Set(endpoint, CharSpan(productLabel, strlen(productLabel)));
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Product Label: 0x%02x", status));
    }

    bool reachable;
    if (ConfigurationMgr().GetReachable(reachable) == CHIP_NO_ERROR)
    {
        status = Attributes::Reachable::Set(endpoint, reachable);
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Reachable: 0x%02x", status));
    }

    char uniqueId[DeviceLayer::ConfigurationManager::kMaxUniqueIDLength + 1];
    if (ConfigurationMgr().GetUniqueId(uniqueId, sizeof(uniqueId)) == CHIP_NO_ERROR)
    {
        status = Attributes::UniqueID::Set(endpoint, CharSpan(uniqueId, strlen(uniqueId)));
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Unique Id: 0x%02x", status));
    }
}

void MatterBasicPluginServerInitCallback()
{
    PlatformMgr().SetDelegate(&gPlatformMgrDelegate);
    registerAttributeAccessOverride(&gBasicAttrAttrAccess);
}
