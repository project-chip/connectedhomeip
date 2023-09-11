/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <crypto/PersistentStorageOperationalKeystore.h>

namespace chip {

class CustomCSRResponseOperationalKeyStore : public PersistentStorageOperationalKeystore
{
public:
    CHIP_ERROR NewOpKeypairForFabric(FabricIndex fabricIndex, MutableByteSpan & outCertificateSigningRequest) override;

private:
    CHIP_ERROR ReuseOpKeypair(FabricIndex fabricIndex, MutableByteSpan & outCertificateSigningRequest);
};

} // namespace chip
