/*
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 * SPDX-FileCopyrightText: 2014-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines chip vendor identifiers.
 *
 *      Vendor identifiers are assigned and administered by CHIP.
 *
 */

#pragma once

#include <cstdint>

namespace chip {

//
// CHIP Vendor Identifiers (16 bits max)
//
// These values are under administration by Connectivity Standards Alliance (CSA).
// For new value allocation make a formal request to CSA.
// The database of currently allocated identifiers can be found in this document:
// https://groups.csa-iot.org/wg/members-all/document/10905
//
// As per specifications (section 2.5.2 Vendor Identifier)
enum VendorId : uint16_t
{
    Common       = 0x0000u,
    Unspecified  = 0x0000u,
    Apple        = 0x1349u,
    Google       = 0x6006u,
    TestVendor1  = 0xFFF1u,
    TestVendor2  = 0xFFF2u,
    TestVendor3  = 0xFFF3u,
    TestVendor4  = 0xFFF4u,
    NotSpecified = 0xFFFFu
};

constexpr bool IsTestVendorId(VendorId vid)
{
    return (vid >= TestVendor1 && vid <= TestVendor4);
}

constexpr bool IsVendorIdValidOperationally(uint16_t vendorId)
{
    return (vendorId != static_cast<uint16_t>(VendorId::Common)) && (vendorId <= static_cast<uint16_t>(VendorId::TestVendor4));
}

constexpr bool IsVendorIdValidOperationally(VendorId vendorId)
{
    return IsVendorIdValidOperationally(static_cast<uint16_t>(vendorId));
}

} // namespace chip
