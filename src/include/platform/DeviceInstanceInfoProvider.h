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
#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {
namespace DeviceLayer {

class DeviceInstanceInfoProvider
{
public:
    DeviceInstanceInfoProvider()          = default;
    virtual ~DeviceInstanceInfoProvider() = default;

    /**
     * @brief Obtain the Serial Number from the device's factory data.
     *
     * The SerialNumber attribute specifies a human readable serial number
     *
     * @param[in, out] buf Buffer to copy string. This buffer must be null-terminated.
     * @param[in] bufSize Size of null-terminated input buffer. This size should be +1 higher than maximum possible string.
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) = 0;

    /**
     * @brief Obtain a manufacturing date from the device's factory data.
     *
     * The ManufacturingDate attribute specifies the date that the Node was manufactured.
     * The date format is ISO 8601 YYYYMMDD.
     *
     *
     * @param[in,out] year Reference to location where manufacturing year will be stored
     * @param[in, out] month Reference to location where manufacturing month will be stored
     * @param[in, out] day Reference to location where manufacturing day will be stored
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) = 0;

    /**
     * @brief Obtain a Firmware Version from the device's factory data.
     *
     * @param[in,out] hardwareVersion Reference to location where the hardware version integer will be copied
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) = 0;

    /**
     * @brief Obtain a Firmware Version String from the device's factory data.
     *
     * The HardwareVersionString can be used to provide a more user-friendly value than that
     * represented by the HardwareVersion attribute.
     *
     * @param[in, out] buf Buffer to copy string. This buffer must be null-terminated.
     * @param[in] bufSize Size of null-terminated input buffer. This size should be +1 higher than maximum possible string.
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) = 0;

    /**
     * @brief Obtain a Rotating Device ID Unique ID from the device's factory data.
     *
     * The unique identifier consists of a randomly-generated 128-bit or longer octet string which
     * was programmed during factory provisioning or delivered to the device by the vendor using
     * secure means after a software update
     *
     * @param[in,out] uniqueIdSpan Reference to location where the Rotating Device ID Unique ID will be copied
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) = 0;
};

/**
 * Instance getter for the global DeviceInstanceInfoProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global device attestation credentials provider. Assume never null.
 */
DeviceInstanceInfoProvider * GetDeviceInstanceInfoProvider();

/**
 * Instance setter for the global DeviceInstanceInfoProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the DeviceInstanceInfoProvider to start returning with the getter
 */
void SetDeviceInstanceInfoProvider(DeviceInstanceInfoProvider * provider);

} // namespace DeviceLayer
} // namespace chip
