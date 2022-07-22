/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
