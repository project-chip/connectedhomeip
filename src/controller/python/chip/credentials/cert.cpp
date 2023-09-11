/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cert.h"

#include <credentials/CHIPCert.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::Credentials;

PyChipError pychip_ConvertX509CertToChipCert(const uint8_t * x509Cert, size_t x509CertLen, uint8_t * chipCert, size_t * chipCertLen)
{
    MutableByteSpan output(chipCert, *chipCertLen);
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError((err = ConvertX509CertToChipCert(ByteSpan(x509Cert, x509CertLen), output)) == CHIP_NO_ERROR,
                        ToPyChipError(err));
    *chipCertLen = output.size();

    return ToPyChipError(err);
}

PyChipError pychip_ConvertChipCertToX509Cert(const uint8_t * chipCert, size_t chipCertLen, uint8_t * x509Cert, size_t * x509CertLen)
{
    MutableByteSpan output(x509Cert, *x509CertLen);
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError((err = ConvertChipCertToX509Cert(ByteSpan(chipCert, chipCertLen), output)) == CHIP_NO_ERROR,
                        ToPyChipError(err));
    *x509CertLen = output.size();

    return ToPyChipError(err);
}
