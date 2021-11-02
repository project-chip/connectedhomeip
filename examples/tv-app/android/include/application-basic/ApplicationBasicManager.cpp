/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "ApplicationBasicManager.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app/Command.h>
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <lib/support/ZclString.h>

#include <inipp/inipp.h>

using namespace chip;

CHIP_ERROR ApplicationBasicManager::Init()
{
    CHIP_ERROR err                                       = CHIP_NO_ERROR;
    EndpointConfigurationStorage & endpointConfiguration = EndpointConfigurationStorage::GetInstance();
    err                                                  = endpointConfiguration.Init();
    SuccessOrExit(err);
    es = &endpointConfiguration;
exit:
    return err;
}

void ApplicationBasicManager::store(chip::EndpointId endpoint, Application * application)
{
    uint8_t bufferMemory[64];
    MutableByteSpan zclString(bufferMemory);

    MakeZclCharString(zclString, application->vendorName);
    EmberAfStatus vendorNameStatus =
        emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_VENDOR_NAME_ATTRIBUTE_ID,
                                    zclString.data(), ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
    if (vendorNameStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store vendor name attribute.");
    }

    EmberAfStatus vendorIdStatus =
        emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_VENDOR_ID_ATTRIBUTE_ID,
                                    (uint8_t *) &application->vendorId, ZCL_INT16U_ATTRIBUTE_TYPE);
    if (vendorIdStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store vendor id attribute.");
    }

    MakeZclCharString(zclString, application->name);
    EmberAfStatus nameStatus =
        emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_NAME_ATTRIBUTE_ID, zclString.data(),
                                    ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
    if (nameStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store name attribute.");
    }

    EmberAfStatus productIdStatus =
        emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_PRODUCT_ID_ATTRIBUTE_ID,
                                    (uint8_t *) &application->productId, ZCL_INT16U_ATTRIBUTE_TYPE);
    if (productIdStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store product id attribute.");
    }

    MakeZclCharString(zclString, application->id);
    EmberAfStatus idStatus =
        emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_ID_ATTRIBUTE_ID, zclString.data(),
                                    ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
    if (idStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store id attribute.");
    }

    EmberAfStatus catalogVendorIdStatus =
        emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_CATALOG_VENDOR_ID_ATTRIBUTE_ID,
                                    (uint8_t *) &application->catalogVendorId, ZCL_INT16U_ATTRIBUTE_TYPE);
    if (catalogVendorIdStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store catalog vendor id attribute.");
    }

    EmberAfStatus applicationStatus =
        emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_STATUS_ATTRIBUTE_ID,
                                    (uint8_t *) &application->status, ZCL_ENUM8_ATTRIBUTE_TYPE);
    if (applicationStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store status attribute.");
    }
}

Application ApplicationBasicManager::getApplicationForEndpoint(chip::EndpointId endpoint)
{
    Application app = {};
    uint16_t size   = static_cast<uint16_t>(sizeof(app.name));

    std::string section = "endpoint" + std::to_string(endpoint);

    CHIP_ERROR err = es->get(section, "name", app.name, size);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to get app name. Error:%s", chip::ErrorStr(err));
    }

    err = es->get(section, "vendorName", app.vendorName, size);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to get app vendor name. Error:%s", chip::ErrorStr(err));
    }

    err = es->get(section, "id", app.id, size);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to get app id. Error:%s", chip::ErrorStr(err));
    }

    err = es->get(section, "catalogVendorId", app.catalogVendorId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to get app catalog vendor id. Error:%s", chip::ErrorStr(err));
    }

    err = es->get(section, "productId", app.productId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to get app product id. Error:%s", chip::ErrorStr(err));
    }

    err = es->get(section, "vendorId", app.vendorId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to get app vendor id. Error:%s", chip::ErrorStr(err));
    }

    return app;
}

bool applicationBasicClusterChangeApplicationStatus(EmberAfApplicationBasicStatus status, chip::EndpointId endpoint)
{
    // TODO: Insert code here
    ChipLogProgress(Zcl, "Sent an application status change request %d for endpoint %d", status, endpoint);
    return true;
}
