/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <platform/CHIPDeviceLayer.h>

#include "Device.h"

void DeviceManagerInit();

/**
 * @brief Adds a device to a dynamic endpoint.
 *
 * This function attempts to add a device to a dynamic endpoint. It tries to find an available
 * endpoint slot and retries the addition process up to a specified maximum number of times if
 * the endpoint already exists. If the addition is successful, it returns the index of the
 * dynamic endpoint; otherwise, it returns -1.
 *
 * @param dev A pointer to the device to be added.
 * @param ep A pointer to the endpoint type.
 * @param deviceTypeList A span containing the list of device types.
 * @param dataVersionStorage A span containing the data version storage.
 * @param parentEndpointId The parent endpoint ID. Defaults to an invalid endpoint ID.
 * @return int The index of the dynamic endpoint if successful, -1 otherwise.
 */
int AddDeviceEndpoint(Device * dev, EmberAfEndpointType * ep, const chip::Span<const EmberAfDeviceType> & deviceTypeList,
                      const chip::Span<chip::DataVersion> & dataVersionStorage,
                      chip::EndpointId parentEndpointId = chip::kInvalidEndpointId);

/**
 * @brief Removes a device from a dynamic endpoint.
 *
 * This function attempts to remove a device from a dynamic endpoint by iterating through the
 * available endpoints and checking if the device matches. If the device is found, it clears the
 * dynamic endpoint, logs the removal, and returns the index of the removed endpoint.
 * If the device is not found, it returns -1.
 *
 * @param dev A pointer to the device to be removed.
 * @return int The index of the removed dynamic endpoint if successful, -1 otherwise.
 */
int RemoveDeviceEndpoint(Device * dev);
