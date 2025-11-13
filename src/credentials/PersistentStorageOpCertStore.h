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

#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>

#include "OperationalCertificateStore.h"

namespace chip {
namespace Credentials {

/**
 * @brief OperationalCertificateStore implementation making use of PersistentStorageDelegate
 *        to load/store certificates. This is the legacy behavior of `FabricTable` prior
 *        to refactors to use `OperationalCertificateStore` and exists as a baseline example
 *        of how to use the interface.
 */
class PersistentStorageOpCertStore final : public OperationalCertificateStore
{
public:
    PersistentStorageOpCertStore() {}
    virtual ~PersistentStorageOpCertStore() { Finish(); }

    // Non-copyable
    PersistentStorageOpCertStore(PersistentStorageOpCertStore const &) = delete;
    void operator=(PersistentStorageOpCertStore const &)               = delete;

    /**
     * @brief Initialize the certificate store to map to a given storage delegate.
     *
     * @param storage Pointer to persistent storage delegate to use. Must outlive this instance.
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if already initialized
     */
    CHIP_ERROR Init(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(mStorage == nullptr, CHIP_ERROR_INCORRECT_STATE);
        RevertPendingOpCerts();
        mStorage = storage;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Finalize the certificate store, so that subsequent operations fail
     */
    void Finish()
    {
        VerifyOrReturn(mStorage != nullptr);

        RevertPendingOpCerts();
        mStorage = nullptr;
    }

    bool HasPendingRootCert() const override;
    bool HasPendingNocChain() const override;
    bool HasPendingVidVerificationElements() const override;

    bool HasCertificateForFabric(FabricIndex fabricIndex, CertChainElement element) const override;

    CHIP_ERROR AddNewTrustedRootCertForFabric(FabricIndex fabricIndex, const ByteSpan & rcac) override;
    CHIP_ERROR AddNewOpCertsForFabric(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac) override;
    CHIP_ERROR UpdateOpCertsForFabric(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac) override;
    CHIP_ERROR UpdateVidVerificationSignerCertForFabric(FabricIndex fabricIndex, ByteSpan vvsc) override;
    CHIP_ERROR UpdateVidVerificationStatementForFabric(FabricIndex fabricIndex, ByteSpan vidVerificationStatement) override;

    CHIP_ERROR CommitOpCertsForFabric(FabricIndex fabricIndex) override;
    CHIP_ERROR RemoveOpCertsForFabric(FabricIndex fabricIndex) override;

    void RevertPendingOpCertsExceptRoot() override
    {
        mPendingIcac.Free();
        mPendingNoc.Free();

        if (mPendingRcac.Get() == nullptr)
        {
            mPendingFabricIndex = kUndefinedFabricIndex;
        }
        mStateFlags.Clear(StateFlags::kAddNewOpCertsCalled);
        mStateFlags.Clear(StateFlags::kUpdateOpCertsCalled);
        RevertVidVerificationStatement();
    }

    void RevertPendingOpCerts() override
    {
        RevertPendingOpCertsExceptRoot();

        // Clear the rest statelessly
        mPendingRcac.Free();
        mPendingFabricIndex = kUndefinedFabricIndex;
        mStateFlags.ClearAll();
    }

    CHIP_ERROR GetCertificate(FabricIndex fabricIndex, CertChainElement element, MutableByteSpan & outCertificate) const override;
    CHIP_ERROR GetVidVerificationElement(FabricIndex fabricIndex, VidVerificationElement element,
                                         MutableByteSpan & outElement) const override;

protected:
    enum class StateFlags : uint8_t
    {
        // Below are flags to assist interlock logic
        kAddNewOpCertsCalled             = (1u << 0),
        kAddNewTrustedRootCalled         = (1u << 1),
        kUpdateOpCertsCalled             = (1u << 2),
        kVidVerificationStatementUpdated = (1u << 3),
        kVvscUpdated                     = (1u << 4),
    };

    // Returns CHIP_ERROR_NOT_FOUND if a pending certificate couldn't be found, otherwise status of pending copy
    CHIP_ERROR GetPendingCertificate(FabricIndex fabricIndex, CertChainElement element, MutableByteSpan & outCertificate) const;

    // Returns true if any pending or persisted state exists for the fabricIndex, false if nothing at all is found.
    bool HasAnyCertificateForFabric(FabricIndex fabricIndex) const;

    // Returns true if there is stored or pending NOC chain .
    bool HasNocChainForFabric(FabricIndex fabricIndex) const;

    // Returns CHIP_NO_ERROR if all assumptions for VID Verification update operations are OK.
    CHIP_ERROR BasicVidVerificationAssumptionsAreMet(FabricIndex fabricIndex) const;

    // Commits pending VID Verification data (called only from CommitOpCertsForFabric).
    CHIP_ERROR CommitVidVerificationForFabric(FabricIndex fabricIndex);

    // Reverts pending VID Verification data if any.
    void RevertVidVerificationStatement()
    {
        mPendingVvsc.Free();
        mPendingVidVerificationStatement.Free();
        mStateFlags.Clear(StateFlags::kVidVerificationStatementUpdated);
        mStateFlags.Clear(StateFlags::kVvscUpdated);
    }

    PersistentStorageDelegate * mStorage = nullptr;

    // This pending fabric index is `kUndefinedFabricIndex` if there are no pending certs at all for the fabric
    FabricIndex mPendingFabricIndex = kUndefinedFabricIndex;

    Platform::ScopedMemoryBufferWithSize<uint8_t> mPendingRcac;
    Platform::ScopedMemoryBufferWithSize<uint8_t> mPendingIcac;
    Platform::ScopedMemoryBufferWithSize<uint8_t> mPendingNoc;
    Platform::ScopedMemoryBufferWithSize<uint8_t> mPendingVvsc;
    Platform::ScopedMemoryBufferWithSize<uint8_t> mPendingVidVerificationStatement;

    BitFlags<StateFlags> mStateFlags;
};

} // namespace Credentials
} // namespace chip
