/*
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

#pragma once

#include <lib/support/Span.h>

namespace chip {
namespace TestCerts {

// Cert Hierarchy
//
//                RootA             RootB
//                  |                 |
//        +---------+-+-------+       |
//        |           |       |       |
//      IA_A1       IA_A2  NodeA4   IA_B1
//        |           |               |
//     +--+--+        |               |
//     |     |        |               |
//  NodeA1  NodeA2  NodeA3          NodeB1

struct UnitTestCertAsset
{
    UnitTestCertAsset(ByteSpan cert, ByteSpan key) : mCert(cert), mKey(key) {}
    ByteSpan mCert;
    ByteSpan mKey;
};

UnitTestCertAsset GetRootACertAsset();
UnitTestCertAsset GetRootBCertAsset();
UnitTestCertAsset GetIAA1CertAsset();
UnitTestCertAsset GetIAA2CertAsset();
UnitTestCertAsset GetIAB1CertAsset();
UnitTestCertAsset GetNodeA1CertAsset();
UnitTestCertAsset GetNodeA2CertAsset();
UnitTestCertAsset GetNodeA3CertAsset();
UnitTestCertAsset GetNodeA4CertAsset();
UnitTestCertAsset GetNodeB1CertAsset();

} // namespace TestCerts
} // namespace chip
