/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <lib/support/Span.h>

#include <stddef.h>

namespace chip {
namespace trace {
namespace logging {

/**
 * @brief This methods converts bytes representing a certificate to a printable string.
 *        The certificate can be a X.509v3 certificate in DER format or a Matter Operational Certificate.
 *
 * @param source       The source buffer containing a certificate as bytes.
 * @param destination  The destination buffer where the printable representation will be stored.
 *
 * @returns             A pointer to the destination buffer data.
 */
const char * ToCertificateString(const chip::ByteSpan & source, chip::MutableCharSpan destination);

/**
 * @brief This methods converts bytes representing a CMS envelope to a printable string.
 *
 * @param source       The source buffer containing a certificate as bytes.
 * @param destination  The destination buffer where the printable representation will be stored.
 *
 * @returns             A pointer to the destination buffer data.
 */
const char * ToCertificateRequestString(const chip::ByteSpan & source, chip::MutableCharSpan destination);

} // namespace logging
} // namespace trace
} // namespace chip
