/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      This file defines chip vendor identifiers.
 *
 *      Vendor identifiers are assigned and administered by CHIP.
 *
 */

#ifndef CHIP_VENDOR_IDENTIFIERS_HPP
#define CHIP_VENDOR_IDENTIFIERS_HPP

namespace chip {

//
// CHIP Vendor Identifiers (16 bits max)
//

enum ChipVendorId
{
    kChipVendor_Common       = 0x0000,
    kChipVendor_NestLabs     = 0x235A,
    kChipVendor_NotSpecified = 0xFFFF
};

}; // namespace chip

#endif // CHIP_VENDOR_IDENTIFIERS_HPP
