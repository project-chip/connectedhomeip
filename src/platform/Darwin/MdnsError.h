/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "DnssdImpl.h"

namespace chip {
namespace Dnssd {
namespace Error {

const char * ToString(DNSServiceErrorType errorCode);

CHIP_ERROR ToChipError(DNSServiceErrorType errorCode);

} // namespace Error
} // namespace Dnssd
} // namespace chip
