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
#include <lib/core/ClusterEnums.h>
#include <lib/support/Span.h>

namespace chip {
namespace DeviceLayer {

class DeviceInstanceInfoProvider
{
public:
    DeviceInstanceInfoProvider()          = default;
    virtual ~DeviceInstanceInfoProvider() = default;

    /**
     * @brief Obtain the Vendor Name from the device's factory data.
     *
     * @param[out] buf Buffer to copy string.
     *                 On CHIP_NO_ERROR return from this function this buffer will be null-terminated.
     *                 On error CHIP_ERROR_BUFFER_TOO_SMALL there is no guarantee that buffer will be null-terminated.
     * @param[in] bufSize Size of data, including the null terminator, that can be written to buf.
     *                    This size should be +1 higher than maximum possible string.
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetVendorName(char * buf, size_t bufSize) = 0;

    /**
     * @brief Obtain the Vendor Id from the device's factory data.
     *
     * @param[out] vendorId Reference to location where the vendor id integer will be copied
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetVendorId(uint16_t & vendorId) = 0;

    /**
     * @brief Obtain the Product Name from the device's factory data.
     *
     * @param[in, out] buf Buffer to copy string.
     *                 On CHIP_NO_ERROR return from this function this buffer will be null-terminated.
     *                 On error CHIP_ERROR_BUFFER_TOO_SMALL there is no guarantee that buffer will be null-terminated.
     * @param[in] bufSize Size of data, including the null terminator, that can be written to buf.
     *                    This size should be +1 higher than maximum possible string.
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetProductName(char * buf, size_t bufSize) = 0;

    /**
     * @brief Obtain the Product Id from the device's factory data.
     *
     * @param[out] productId Reference to location where the product id integer will be copied
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetProductId(uint16_t & productId) = 0;

    /**
     * @brief Obtain Part Number from the device factory data.
     *
     * @param[out] buf     Buffer to store the null-terminated result string.
     * @param[in] bufSize  Size of the buffer. The buffer should allow for fitting in Part Number
     *                     (max 32 characters) and the null terminator.
     **/
    virtual CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) = 0;

    /**
     * @brief Obtain Product URL from the device factory data.
     *
     * @param[out] buf     Buffer to store the null-terminated result string.
     * @param[in] bufSize  Size of the buffer. The buffer should allow for fitting in Product URL
     *                     (max 256 characters) and the null terminator.
     **/
    virtual CHIP_ERROR GetProductURL(char * buf, size_t bufSize) = 0;

    /**
     * @brief Obtain Product Label from the device factory data.
     *
     * @param[out] buf     Buffer to store the null-terminated result string.
     * @param[in] bufSize  Size of the buffer. The buffer should allow for fitting in Product Label
     *                     (max 64 characters) and the null terminator.
     **/
    virtual CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) = 0;

    /**
     * @brief Obtain the Serial Number from the device's factory data.
     *
     * The SerialNumber attribute specifies a human readable serial number
     *
     * @param[in, out] buf Buffer to copy string.
     *                 On CHIP_NO_ERROR return from this function this buffer will be null-terminated.
     *                 On error CHIP_ERROR_BUFFER_TOO_SMALL there is no guarantee that buffer will be null-terminated.
     * @param[in] bufSize Size of data, including the null terminator, that can be written to buf.
     *                    This size should be +1 higher than maximum possible string.
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) = 0;

    /**
     * @brief Obtain a manufacturing date from the device's factory data.
     *
     * The ManufacturingDate attribute specifies the date that the Node was manufactured.
     * Output values are returned in ISO 8601, where:
     *      The first month of the year is January and its returning value is equal to 1.
     *      The first day of a month starts from 1.
     *
     * @param[out] year Reference to location where manufacturing year will be stored
     * @param[out] month 1-based value [range 1-12] Reference to location where manufacturing month will be stored
     * @param[out] day 1-based value [range 1-31] Reference to location where manufacturing day will be stored
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) = 0;

    /**
     * @brief Obtain a Hardware Version from the device's factory data.
     *
     * @param[out] hardwareVersion Reference to location where the hardware version integer will be copied
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) = 0;

    /**
     * @brief Obtain a Hardware Version String from the device's factory data.
     *
     * The HardwareVersionString can be used to provide a more user-friendly value than that
     * represented by the HardwareVersion attribute.
     *
     * @param[in, out] buf Buffer to copy string.
     *                     On CHIP_NO_ERROR return from this function this buffer will be null-terminated.
     *                     On error CHIP_ERROR_BUFFER_TOO_SMALL there is no guarantee that buffer will be null-terminated.
     * @param[in] bufSize Size of data, including the null terminator, that can be written to buf.
     *                    This size should be +1 higher than maximum possible string.
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_BUFFER_TOO_SMALL if the buffer was too small to fit string and null
     * terminating. or another CHIP_ERROR from the underlying implementation if access fails.
     */
    virtual CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) = 0;

    /**
     * @brief Obtain a Rotating Device ID Unique ID from the device's factory data.
     *
     * The unique identifier consists of a randomly-generated 128-bit or longer octet string which
     * was programmed during factory provisioning or delivered to the device by the vendor using
     * secure means after a software update.
     *
     * @param[out] uniqueIdSpan Reference to location where the Rotating Device ID Unique ID will be copied
     *                          According to specification input size of span buffer should be declared with at least 16 Bytes
     * length The size of uniqueIdSpan is reduced to actual value on success
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) = 0;

    /**
     * @brief Obtain the product's finish from the device's factory data.
     *
     * If the product finish is not available, this should return
     * CHIP_ERROR_NOT_IMPLEMENTED, and the Basic Information ProductAppearance attribute should
     * not be implemented for the device.
     */
    virtual CHIP_ERROR GetProductFinish(app::Clusters::BasicInformation::ProductFinishEnum * finish)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * @brief Obtain the product's primary color from the device's factory data.
     *
     * If the primary color finish is not available or does not exist (e.g. the
     * device wants to return null for the color in the Basic Information
     * ProductAppearance attribute), this should return CHIP_ERROR_NOT_IMPLEMENTED.
     */
    virtual CHIP_ERROR GetProductPrimaryColor(app::Clusters::BasicInformation::ColorEnum * primaryColor)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

/**
 * Instance getter for the global DeviceInstanceInfoProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The pointer to global device instance info provider. Assume never null.
 */
DeviceInstanceInfoProvider * GetDeviceInstanceInfoProvider();

/**
 * Instance setter for the global DeviceInstanceInfoProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the DeviceInstanceInfoProvider pointer to start returning with the getter
 */
void SetDeviceInstanceInfoProvider(DeviceInstanceInfoProvider * provider);

} // namespace DeviceLayer
} // namespace chip
