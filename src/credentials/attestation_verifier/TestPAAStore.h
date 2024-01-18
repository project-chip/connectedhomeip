/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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
 *
 * Storage for test attestation PAA certificates. Contains the certs for
 * the 0xFFF1 vid PAA cert and the test PAA cert with no VID.
 *
 * These certs are used by the device attestation and crypto tests
 * and by the DefaultDeviceAttestationVerifier.
 */

#pragma once

#include <lib/support/Span.h>

namespace chip {
namespace TestCerts {

extern const ByteSpan sTestCert_PAA_FFF1_Cert;
extern const ByteSpan sTestCert_PAA_NoVID_Cert;

const Span<const ByteSpan> & GetTestPaaRootStore();

} // namespace TestCerts
} // namespace chip
