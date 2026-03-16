/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

/**
 *    @file
 *      Provides an implementation of Device Firmware Upgrade using SMP protocol
 *      over Bluetooth LE functionality for Telink platform.
 */

#pragma once

#include <lib/core/OTAImageHeader.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>

enum VerificationFailReason : unsigned char
{
    NO_FAIL          = 0,
    WRONG_PRODUCT_ID = 1 << 1,
    WRONG_VENDOR_ID  = 1 << 2,
    WRONG_VERSION    = 1 << 3,
};

using verificationFailCallback = void (*)(VerificationFailReason);

/**
 * @brief DFU over SMP helper class
 *
 * The purpose of this class is to enable Device Firmware Upgrade mechanism
 * using Simple Management Protocol (SMP) over Bluetooth LE. Besides
 * facilitating initialization of the SMP server, it is capable of requesting
 * BLE advertising in a way that is compatible with other application components
 * that use BLE, such as Matter BLE layer.
 */
class DFUOverSMP
{
public:
    /**
     * @brief Initialize DFU over SMP utility
     *
     * Initialize internal structures and register necessary commands in the SMP
     * server.
     */
    void Init();

    /**
     * @brief Set callback function for verification failing event
     *
     * Set a callback function that will be called if the image footer verification fails.
     * Callback function have to receive argument that will describe
     * what was the reason of the failure.
     */
    void SetFailCallback(verificationFailCallback cb);

    /**
     * @brief Start processing the footer of the image in slot 1.
     *
     * Starts footer check of the newly received image in the slot 1.
     */
    CHIP_ERROR ProcessImageFooter();

private:
    static constexpr uint16_t VERSION_STRING_MAX_LENGTH = 64;

    verificationFailCallback failCallback;

    friend DFUOverSMP & GetDFUOverSMP();
    static DFUOverSMP sDFUOverSMP;

    CHIP_ERROR GetDFUImageFooter(chip::OTAImageHeader & footer, const struct flash_area * fa);
    CHIP_ERROR GetDFUImageFooterOffset(unsigned int & footer_offset, const struct flash_area * fa);
    CHIP_ERROR CheckDFUImageFooter(chip::OTAImageHeader * imageHeader);
};

inline DFUOverSMP & GetDFUOverSMP()
{
    return DFUOverSMP::sDFUOverSMP;
}
