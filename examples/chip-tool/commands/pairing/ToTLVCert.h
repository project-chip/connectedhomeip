/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <lib/support/Span.h>
#include <string>

CHIP_ERROR ToBase64(const chip::ByteSpan & input, std::string & outputAsPrefixedBase64);
CHIP_ERROR ToTLVCert(const chip::ByteSpan & derEncodedCertificate, std::string & tlvCertAsPrefixedBase64);
