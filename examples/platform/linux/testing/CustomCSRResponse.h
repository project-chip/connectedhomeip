/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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
