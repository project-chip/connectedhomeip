/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stddef.h>

#include <controller/python/chip/native/PyChipError.h>

extern "C" {
PyChipError pychip_ConvertX509CertToChipCert(const uint8_t * x509Cert, size_t x509CertLen, uint8_t * chipCert,
                                             size_t * chipCertLen);
PyChipError pychip_ConvertChipCertToX509Cert(const uint8_t * chipCert, size_t chipCertLen, uint8_t * x509Cert,
                                             size_t * x509CertLen);
}
