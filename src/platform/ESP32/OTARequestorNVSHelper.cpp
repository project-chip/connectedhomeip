/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "nvs.h"
#include "nvs_flash.h"
#include <platform/ESP32/OTARequestorNVSHelper.h>

const char OTARequestorNVSHelper::namespaceStorage[] = "otarStorage";

const char OTARequestorNVSHelper::softwareVersionName[]   = "preVersion";
const char OTARequestorNVSHelper::otaProviderNodeId[]     = "providerNodeId";
const char OTARequestorNVSHelper::otaProviderFabIndex[]   = "providerFabInd";
const char OTARequestorNVSHelper::otaProviderEndpointId[] = "providerEndId";

esp_err_t OTARequestorNVSHelper::ReadSoftwareVersion(uint32_t & softwareVersion)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(OTARequestorNVSHelper::namespaceStorage, NVS_READONLY, &my_handle);
    if (err != ESP_OK)
        return err;

    err = nvs_get_u32(my_handle, OTARequestorNVSHelper::softwareVersionName, &softwareVersion);

    nvs_close(my_handle);
    return ((err==ESP_OK)?ESP_OK:err);
}

esp_err_t OTARequestorNVSHelper::ReadProviderLocation(chip::NodeId & nodeId, chip::FabricIndex & fabIndex,
                                                      chip::EndpointId & endpointId)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(OTARequestorNVSHelper::namespaceStorage, NVS_READONLY, &my_handle);
    if (err != ESP_OK)
        return err;

    err = nvs_get_u64(my_handle, OTARequestorNVSHelper::otaProviderNodeId, &nodeId);
    if (err != ESP_OK)
    {
        nvs_close(my_handle);
        return err;
    }

    err = nvs_get_u8(my_handle, OTARequestorNVSHelper::otaProviderFabIndex, &fabIndex);
    if (err != ESP_OK)
    {
        nvs_close(my_handle);
        return err;
    }

    err = nvs_get_u16(my_handle, OTARequestorNVSHelper::otaProviderEndpointId, &endpointId);

    nvs_close(my_handle);
    return ((err==ESP_OK)?ESP_OK:err);
}

esp_err_t OTARequestorNVSHelper::ClearAllData()
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(OTARequestorNVSHelper::namespaceStorage, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        return err;

    err = nvs_erase_all(my_handle);
    if (err != ESP_OK)
    {
        nvs_close(my_handle);
        return err;
    }

    err = nvs_commit(my_handle);

    nvs_close(my_handle);
    return ((err==ESP_OK)?ESP_OK:err);
}

esp_err_t OTARequestorNVSHelper::WriteSoftwareVersion(uint32_t softwareVersion)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(OTARequestorNVSHelper::namespaceStorage, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        return err;

    err = nvs_set_u32(my_handle, OTARequestorNVSHelper::softwareVersionName, softwareVersion);
    if (err != ESP_OK)
    {
        nvs_close(my_handle);
        return err;
    }

    err = nvs_commit(my_handle);

    nvs_close(my_handle);
    return ((err==ESP_OK)?ESP_OK:err);
}

esp_err_t OTARequestorNVSHelper::WriteProviderParameters(chip::NodeId nodeId, chip::FabricIndex fabIndex,
                                                         chip::EndpointId endpointId)
{

    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(OTARequestorNVSHelper::namespaceStorage, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        return err;

    err = nvs_set_u64(my_handle, OTARequestorNVSHelper::otaProviderNodeId, nodeId);
    if (err != ESP_OK)
    {
        nvs_close(my_handle);
        return err;
    }

    err = nvs_set_u8(my_handle, OTARequestorNVSHelper::otaProviderFabIndex, fabIndex);
    if (err != ESP_OK)
    {
        nvs_close(my_handle);
        return err;
    }

    err = nvs_set_u16(my_handle, OTARequestorNVSHelper::otaProviderEndpointId, endpointId);
    if (err != ESP_OK)
    {
        nvs_close(my_handle);
        return err;
    }

    err = nvs_commit(my_handle);
    return ((err==ESP_OK)?ESP_OK:err);
}
