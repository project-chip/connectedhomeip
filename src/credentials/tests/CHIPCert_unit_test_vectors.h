/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
