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

#include "BridgedDevice.h"

#include <memory>

class BridgedDeviceManager
{
public:
    BridgedDeviceManager() = default;

    /**
     * @brief Initializes the BridgedDeviceManager.
     *
     * This function sets up the initial state of the BridgedDeviceManager, clearing
     * any existing devices and setting the starting dynamic endpoint ID.
     */
    void Init();

    /**
     * @brief Adds a device to a dynamic endpoint.
     *
     * This function attempts to add a device to a dynamic endpoint. It tries to find an available
     * endpoint slot and retries the addition process up to a specified maximum number of times if
     * the endpoint already exists. If the addition is successful, it returns the index of the
     * dynamic endpoint;
     *
     * Ensures that the device has a unique id:
     *   - device MUST set its unique id if any BEFORE calling this method
     *   - if no unique id (i.e. empty string), a new unique id will be generated
     *   - Add will fail if the unique id is not unique
     *
     * @param dev A pointer to the device to be added.
     * @param parentEndpointId The parent endpoint ID. Defaults to an invalid endpoint ID.
     * @return int The index of the dynamic endpoint if successful, nullopt otherwise
     */
    std::optional<unsigned> AddDeviceEndpoint(std::unique_ptr<BridgedDevice> dev,
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
    int RemoveDeviceEndpoint(BridgedDevice * dev);

    /**
     * @brief Gets a device from its endpoint ID.
     *
     * This function iterates through the available devices and returns the device that matches the
     * specified endpoint ID. If no device matches the endpoint ID, it returns nullptr.
     *
     * @param endpointId The endpoint ID of the device to be retrieved.
     * @return BridgedDevice* A pointer to the device if found, nullptr otherwise.
     */
    BridgedDevice * GetDevice(chip::EndpointId endpointId) const;

    /**
     * @brief Gets a device from its ScopedNodeId.
     *
     * This function iterates through the available devices and returns the device that matches the
     * specified ScopedNodeId. If no device matches the ScopedNodeId, it returns nullptr.
     *
     * @param scopedNodeId The ScopedNodeId of the device to be retrieved.
     * @return BridgedDevice* A pointer to the device if found, nullptr otherwise.
     */
    BridgedDevice * GetDeviceByScopedNodeId(chip::ScopedNodeId scopedNodeId) const;

    /**
     * @brief Removes a device from a dynamic endpoint by its ScopedNodeId.
     *
     * This function attempts to remove a device and the associated dynamic endpoint by iterating through
     * the available device and checking if the device matches the specified ScopedNodeId. If the device is
     * found, it removes the dynamic endpoint.
     *
     * @param scopedNodeId The ScopedNodeId of the device to be removed.
     * @return unsigned of the index of the removed dynamic endpoint if successful, nullopt otherwise.
     */
    std::optional<unsigned> RemoveDeviceByScopedNodeId(chip::ScopedNodeId scopedNodeId);

    /**
     * Finds the device with the given unique id (if any)
     */
    BridgedDevice * GetDeviceByUniqueId(const std::string & id);

private:
    friend BridgedDeviceManager & BridgeDeviceMgr();

    /**
     * Creates a new unique ID that is not used by any other mDevice
     */
    std::string GenerateUniqueId();

    static BridgedDeviceManager sInstance;

    chip::EndpointId mCurrentEndpointId;
    chip::EndpointId mFirstDynamicEndpointId;
    std::unique_ptr<BridgedDevice> mDevices[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT + 1];
};

/**
 * Returns the public interface of the BridgedDeviceManager singleton object.
 *
 * Applications should use this to access features of the BridgedDeviceManager
 * object.
 */
inline BridgedDeviceManager & BridgeDeviceMgr()
{
    return BridgedDeviceManager::sInstance;
}
