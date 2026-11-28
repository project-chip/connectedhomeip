/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {

class JFARpc
{
public:
    virtual ~JFARpc() {}
    virtual CHIP_ERROR GetICACCSRForJF(MutableByteSpan & icacCSR) = 0;
    // Cross-sign the JFA-A CSR using the Fabric A Root CA.
    // On success, icac contains the DER-encoded cross-signed ICAC certificate.
    virtual CHIP_ERROR GetCrossSignedICAC(FabricId anchorFabricId, ByteSpan csr, MutableByteSpan & icac) = 0;
    // Push JF materials to JFC-B so it can issue Fabric-A NOCs for Fabric-B devices.
    // crossSignedICAC: the CHIP TLV cross-signed ICAC (from JFC-B's ICAC CSR, signed by JFA-A).
    // anchorRCAC: the Fabric-A root CA in CHIP TLV format.
    virtual CHIP_ERROR PushJFMaterials(ByteSpan crossSignedICAC, ByteSpan anchorRCAC, ByteSpan anchorIPK) = 0;
    // Tell the paired commissioner (JFC-B) to discard previously received JF materials.
    virtual CHIP_ERROR ClearJFMaterials() = 0;
    // Ask JFC-B to issue a Fabric-A NOC from a Fabric-B device CSR, so the device can be
    // migrated into Fabric A. JFC-B derives the Fabric-A fabric id internally from the cross-signed
    // ICAC it holds (no fabric id is transmitted). On success:
    //   noc  - the CHIP TLV NOC signed by the anchor ICAC key.
    //   icac - the cross-signed ICAC (CHIP TLV) held by JFC-B.
    //   rcac - the Fabric-A root CA (CHIP TLV) held by JFC-B.
    //   ipk  - the Fabric-A IPK epoch key 0 held by JFC-B.
    // JFA-B uses icac/rcac/ipk to populate the AddTrustedRootCertificate and AddNOC commands.
    virtual CHIP_ERROR GetNOCForDevice(NodeId deviceNodeId, ByteSpan csr, MutableByteSpan & noc, MutableByteSpan & icac,
                                       MutableByteSpan & rcac, MutableByteSpan & ipk) = 0;
    virtual void CloseStreams()                                                       = 0;
};

} // namespace chip
