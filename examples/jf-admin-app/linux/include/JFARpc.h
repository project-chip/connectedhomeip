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
#include <lib/core/DataModelTypes.h>

namespace chip {

class JFARpc
{
public:
    virtual ~JFARpc() {}
    virtual CHIP_ERROR GetICACCSRForJF(MutableByteSpan & icacCSR) = 0;

    /**
     * Request the JFC to cross-sign the commissionee's ICAC CSR under the
     * anchor root CA and return the resulting cross-signed ICAC.
     *
     * @param anchorFabricId  The FabricId of the anchor fabric, embedded as
     *                        matter-fabric-id in the cross-signed ICAC Subject DN.
     * @param icacCSR         The raw ICAC CSR bytes received from the commissionee.
     * @param crossSignedICAC Output buffer for the cross-signed ICAC (DER/TLV encoded).
     */
    virtual CHIP_ERROR GetCrossSignedICAC(FabricId anchorFabricId, ByteSpan icacCSR, MutableByteSpan & crossSignedICAC) = 0;

    virtual void CloseStreams() = 0;
};

} // namespace chip
