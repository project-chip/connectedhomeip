/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "CustomCSRResponseOperationalKeyStore.h"

namespace chip {

struct CSRResponseOptions
{
    bool csrIncorrectType                  = false;
    bool csrExistingKeyPair                = false;
    bool csrNonceIncorrectType             = false;
    bool csrNonceTooLong                   = false;
    bool csrNonceInvalid                   = false;
    bool nocsrElementsTooLong              = false;
    bool attestationSignatureIncorrectType = false;
    bool attestationSignatureInvalid       = false;
    CustomCSRResponseOperationalKeyStore badCsrOperationalKeyStoreForTest;
};

} // namespace chip
