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
#include "../endpoint-configuration/EndpointConfiguration.h"

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

void logApp(const Application & a)
{
    std::cout << "------------ PRINT -----------" << std::endl;
    std::printf("catalogVendorId: %d\n", a.catalogVendorId);
    std::cout << "id: " << a.id << std::endl;
    std::cout << "name: " << a.name << std::endl;
    std::printf("productId: %d\n", a.productId);
    std::printf("vendorId: %d\n", a.vendorId);
    std::cout << "vendorName: " << a.vendorName << std::endl;
}

char * convertToChar(string appString)
{
    char * cstr = new char[appString.length() + 1];
    strcpy(cstr, appString.c_str());
    return cstr;
}

int ApplicationBasicManager::Init()
{
    return 0;
}

EmberAfStatus ApplicationBasicManager::store(chip::EndpointId endpoint, Application * application)
{
    if (strcmp(convertToChar(application->vendorName), "\0") != 0)
    {
#ifndef NDEBUG
        EmberAfStatus vendorNameStatus =
#endif // NDEBUG
            emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_VENDOR_NAME_ATTRIBUTE_ID,
                                        (uint8_t *) &application->name, ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
        assert(vendorNameStatus == EMBER_ZCL_STATUS_SUCCESS);
    }

    if (application->vendorId != 0)
    {
#ifndef NDEBUG
        EmberAfStatus vendorIdStatus =
#endif // NDEBUG
            emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_VENDOR_ID_ATTRIBUTE_ID,
                                        (uint8_t *) &application->vendorId, ZCL_INT16U_ATTRIBUTE_TYPE);
        assert(vendorIdStatus == EMBER_ZCL_STATUS_SUCCESS);
    }

    if (strcmp(convertToChar(application->name), "\0"))
    {
#ifndef NDEBUG
        EmberAfStatus nameStatus =
#endif // NDEBUG
            emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_NAME_ATTRIBUTE_ID,
                                        (uint8_t *) &application->name, ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
        assert(nameStatus == EMBER_ZCL_STATUS_SUCCESS);
    }

    if (application->productId != 0)
    {
#ifndef NDEBUG
        EmberAfStatus productIdStatus =
#endif // NDEBUG
            emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_PRODUCT_ID_ATTRIBUTE_ID,
                                        (uint8_t *) &application->productId, ZCL_INT16U_ATTRIBUTE_TYPE);
        assert(productIdStatus == EMBER_ZCL_STATUS_SUCCESS);
    }

    if (strcmp(convertToChar(application->id), "\0"))
    {
#ifndef NDEBUG
        EmberAfStatus idStatus =
#endif // NDEBUG
            emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_ID_ATTRIBUTE_ID,
                                        (uint8_t *) &application->id, ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
        assert(idStatus == EMBER_ZCL_STATUS_SUCCESS);
    }

    if (application->catalogVendorId != 0)
    {
#ifndef NDEBUG
        EmberAfStatus catalogVendorIdStatus =
#endif // NDEBUG
            emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_CATALOG_VENDOR_ID_ATTRIBUTE_ID,
                                        (uint8_t *) &application->catalogVendorId, ZCL_INT16U_ATTRIBUTE_TYPE);
        assert(catalogVendorIdStatus == EMBER_ZCL_STATUS_SUCCESS);
    }

#ifndef NDEBUG
    EmberAfStatus applicationStatus =
#endif // NDEBUG
        emberAfWriteServerAttribute(endpoint, ZCL_APPLICATION_BASIC_CLUSTER_ID, ZCL_APPLICATION_STATUS_ATTRIBUTE_ID,
                                    (uint8_t *) &application->status, ZCL_ENUM8_ATTRIBUTE_TYPE);
    assert(applicationStatus == EMBER_ZCL_STATUS_SUCCESS);

    return EMBER_ZCL_STATUS_SUCCESS;
}

Application getApplicationConfiguration(chip::EndpointId endpoint)
{
    inipp::Ini<char> ini;
    std::ifstream is(ENDPOINT_CONFIGURATION_PATH);
    Application app = {};
    if (is.good())
    {
        ini.parse(is);
        string section = "app" + std::to_string(endpoint);
        inipp::extract(ini.sections[section]["catalogVendorId"], app.catalogVendorId);
        inipp::extract(ini.sections[section]["id"], app.id);
        inipp::extract(ini.sections[section]["name"], app.name);
        inipp::extract(ini.sections[section]["productId"], app.productId);
        inipp::extract(ini.sections[section]["vendorId"], app.vendorId);
        inipp::extract(ini.sections[section]["vendorName"], app.vendorName);

        logApp(app);
    }

    return app;
}

/** @brief Application Basic Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfApplicationBasicClusterInitCallback(chip::EndpointId endpoint)
{
    Application application = getApplicationConfiguration(endpoint);
    ApplicationBasicManager().store(endpoint, &application);
}
