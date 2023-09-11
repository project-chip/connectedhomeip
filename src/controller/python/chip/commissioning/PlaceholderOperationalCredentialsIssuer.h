/*
 *
 * SPDX-FileCopyrightText: 2021-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *  @file
 *    This file contains class definition of an example operational certificate
 *    issuer for CHIP devices. The class can be used as a guideline on how to
 *    construct your own certificate issuer. It can also be used in tests and tools
 *    if a specific signing authority is not required.
 *
 *    NOTE: This class stores the encryption key in clear storage. This is not suited
 *          for production use. This should only be used in test tools.
 */

#pragma once

#include <controller/OperationalCredentialsDelegate.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CASEAuthTag.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

namespace chip {
namespace Python {

class DLL_EXPORT PlaceholderOperationalCredentialsIssuer : public Controller::OperationalCredentialsDelegate
{
public:
    PlaceholderOperationalCredentialsIssuer() {}
    ~PlaceholderOperationalCredentialsIssuer() override {}

    CHIP_ERROR GenerateNOCChain(const ByteSpan & csrElements, const ByteSpan & csrNonce, const ByteSpan & attestationSignature,
                                const ByteSpan & attestationChallenge, const ByteSpan & DAC, const ByteSpan & PAI,
                                Callback::Callback<Controller::OnNOCChainGeneration> * onCompletion) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    void SetNodeIdForNextNOCRequest(NodeId nodeId) override {}

    void SetFabricIdForNextNOCRequest(FabricId fabricId) override {}
};

} // namespace Python
} // namespace chip
