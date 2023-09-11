/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/address_resolve/AddressResolve.h>

namespace chip {
namespace AddressResolve {

// Provide as storage location to make clang happy.
constexpr uint32_t NodeLookupRequest::kMinLookupTimeMsDefault;
constexpr uint32_t NodeLookupRequest::kMaxLookupTimeMsDefault;

// Placed here so we guarantee that the address resolve static library
// has at least a single cpp file to compile.
Resolver::~Resolver() {}

} // namespace AddressResolve
} // namespace chip
