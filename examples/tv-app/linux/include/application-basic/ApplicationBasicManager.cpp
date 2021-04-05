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

#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

#include <app/util/af.h>
#include <app/util/basic-types.h>

#include <inipp/inipp.h>

#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

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
    if (strcmp(application->vendorName, "\0") != 0)
    {
        EmberAfStatus vendorNameStatus =
            emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_VENDOR_NAME_ATTRIBUTE_ID,
                                        (uint8_t *) &application->vendorName, ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
        assert(vendorNameStatus == EMBER_ZCL_STATUS_SUCCESS);
    }

    if (application->vendorId != 0)
    {
        EmberAfStatus vendorIdStatus =
            emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_VENDOR_ID_ATTRIBUTE_ID,
                                        (uint8_t *) &application->vendorId, ZCL_INT16U_ATTRIBUTE_TYPE);
        assert(vendorIdStatus == EMBER_ZCL_STATUS_SUCCESS);
    }

    if (strcmp(application->name, "\0"))
    {
        EmberAfStatus nameStatus =
            emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_NAME_ATTRIBUTE_ID,
                                        (uint8_t *) &application->name, ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
        assert(nameStatus == EMBER_ZCL_STATUS_SUCCESS);
    }

    if (application->productId != 0)
    {
        EmberAfStatus productIdStatus =
            emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_PRODUCT_ID_ATTRIBUTE_ID,
                                        (uint8_t *) &application->productId, ZCL_INT16U_ATTRIBUTE_TYPE);
        assert(productIdStatus == EMBER_ZCL_STATUS_SUCCESS);
    }

    if (strcmp(application->id, "\0"))
    {
        EmberAfStatus idStatus =
            emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_ID_ATTRIBUTE_ID,
                                        (uint8_t *) &application->id, ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
        assert(idStatus == EMBER_ZCL_STATUS_SUCCESS);
    }

    if (application->catalogVendorId != 0)
    {
        EmberAfStatus catalogVendorIdStatus =
            emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_CATALOG_VENDOR_ID_ATTRIBUTE_ID,
                                        (uint8_t *) &application->catalogVendorId, ZCL_INT16U_ATTRIBUTE_TYPE);
        assert(catalogVendorIdStatus == EMBER_ZCL_STATUS_SUCCESS);
    }

    EmberAfStatus applicationStatus =
        emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_STATUS_ATTRIBUTE_ID,
                                    (uint8_t *) &application->status, ZCL_ENUM8_ATTRIBUTE_TYPE);
    assert(applicationStatus == EMBER_ZCL_STATUS_SUCCESS);
}

Application ApplicationBasicManager::getApplicationForEndpoint(chip::EndpointId endpoint)
{
    Application app = {};
    uint16_t size   = static_cast<uint16_t>(sizeof(app.name));

    string section = "app" + std::to_string(endpoint);
    assert(es->get(section, "name", app.name, size) == CHIP_NO_ERROR);
    assert(es->get(section, "vendorName", app.vendorName, size) == CHIP_NO_ERROR);
    assert(es->get(section, "id", app.id, size) == CHIP_NO_ERROR);
    assert(es->get(section, "catalogVendorId", app.catalogVendorId) == CHIP_NO_ERROR);
    assert(es->get(section, "productId", app.productId) == CHIP_NO_ERROR);
    assert(es->get(section, "vendorId", app.vendorId) == CHIP_NO_ERROR);

    return app;
}
