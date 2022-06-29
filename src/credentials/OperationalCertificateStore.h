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
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>

namespace chip {
namespace Credentials {

class OperationalCertificateStore
{
public:
    enum class CertChainElement : uint8_t
    {
        kRcac = 0,
        kIcac = 1,
        kNoc  = 2
    };

    virtual ~OperationalCertificateStore() {}

    // ==== API designed for commisionables to support fail-safe (although can be used by controllers) ====

    /**
     * @brief Returns true if a pending root certificate exists and is active from a previous
     *        `AddNewTrustedRootCertForFabric`.
     */
    virtual bool HasPendingRootCert() const = 0;

    /**
     * @brief Returns true if a pending operational certificate chain exists and is active from a previous
     *        `AddNewOpCertsForFabric` or `UpdateOpCertsForFabric`.
     */
    virtual bool HasPendingNocChain() const = 0;

    /**
     * @brief Returns whether a usable operational certificates chain exists for the given fabric.
     *
     * Returns true even if the certificates are not persisted yet. Only returns true if a certificate
     * is presently usable such that `GetCertificate` would succeed for the fabric.
     *
     * @param fabricIndex - FabricIndex for which availability of certificate will be checked.
     * @param element - Element of the certificate chain whose presence needs to be checked
     * @return true if there an active obtainable operational certificate of the given type for the given FabricIndex,
     *         false otherwise.
     */
    virtual bool HasCertificateForFabric(FabricIndex fabricIndex, CertChainElement element) const = 0;

    /**
     * @brief Add and temporarily activate a new Trusted Root Certificate for the given fabric
     *
     * The certificate is temporary until committed or reverted.
     * The certificate is committed to storage only on `CommitOpCertsForFabric`.
     * The certificate is destroyed if `RevertPendingOpCerts` is called before `CommitOpCertsForFabric`.
     *
     * Only one pending trusted root certificate is supported at a time and it is illegal
     * to call this method if there is already a persisted root certificate for the given
     * fabric.
     *
     * Uniqueness constraints for roots (see AddTrustedRootCertificate command in spec) are not
     * enforced by this method and must be done as a more holistic check elsewhere. Cryptographic
     * signature verification or path validation are not enforced by this method.
     *
     * If `UpdateOpCertsForFabric` had been called before this method, this method will return
     * CHIP_ERROR_INCORRECT_STATE since it is illegal to update trusted roots when updating an
     * existing NOC chain.
     *
     * @param fabricIndex - FabricIndex for which a new trusted root certificate should be added
     * @param rcac - Buffer containing the root certificate to add.
     *
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient memory to maintain the temporary root cert
     * @retval CHIP_ERROR_INVALID_ARGUMENT if the certificate is empty or too large
     * @retval CHIP_ERROR_INCORRECT_STATE if the certificate store is not properly initialized, if this method
     *                                    is called after `UpdateOpCertsForFabric`, or if there was
     *                                    already a pending or persisted root certificate for the given `fabricIndex`.
     * @retval other CHIP_ERROR value on internal errors
     */
    virtual CHIP_ERROR AddNewTrustedRootCertForFabric(FabricIndex fabricIndex, const ByteSpan & rcac) = 0;

    /**
     * @brief Add and temporarily activate an operational certificate chain for the given fabric.
     *
     * The certificate chain is temporary until committed or reverted.
     * The certificate chain is committed to storage on `CommitOpCertsForFabric`.
     * The certificate chain is destroyed if `RevertPendingOpCerts` is called before `CommitOpCertsForFabric`.
     *
     * Only one pending operational certificate chain is supported at a time and it is illegal
     * to call this method if there is already a persisted certificate chain for the given
     * fabric.
     *
     * Cryptographic signature verification or path validation are not enforced by this method.
     *
     * If `UpdateOpCertsForFabric` had been called before this method, this method will return
     * CHIP_ERROR_INCORRECT_STATE since it is illegal to add a certificate chain after
     * updating an existing NOC and before committing or reverting the update.
     *
     * If `AddNewTrustedRootCertForFabric` had not been called before this method, this method will
     * return CHIP_ERROR_INCORRECT_STATE since it is illegal in this implementation to store an
     * NOC chain without associated root.
     *
     * NOTE: The Matter spec allows AddNOC without AddTrustedRootCertificate if the NOC
     * chains to an existing root, to support root reuse. In this implementation, we expect each
     * fabric to store the root with the rest of the chain. Because of this, callers must ensure
     * that if an AddNOC command is done and no trusted root was added, that the requisite existing
     * root be "copied over" to match.
     *
     * @param fabricIndex - FabricIndex for which to add a new operational certificate chain
     * @param noc - Buffer containing the NOC certificate to add
     * @param icac - Buffer containing the ICAC certificate to add. If no ICAC is needed, `icac.empty()` must be true.
     *
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient memory to maintain the temporary `noc` and `icac` cert copies
     * @retval CHIP_ERROR_INVALID_ARGUMENT if either the noc or icac are invalid sizes
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if `fabricIndex` mismatches the one from a previous successful
     *                                         `AddNewTrustedRootCertForFabric`.
     * @retval CHIP_ERROR_INCORRECT_STATE if the certificate store is not properly initialized, if this method
     *                                    is called after `UpdateOpCertsForFabric`, or if there was
     *                                    already a pending or persisted operational cert chain for the given `fabricIndex`,
     *                                    or if AddNewTrustedRootCertForFabric had not yet been called for the given `fabricIndex`.
     * @retval other CHIP_ERROR value on internal errors
     */
    virtual CHIP_ERROR AddNewOpCertsForFabric(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac) = 0;

    /**
     * @brief Update and temporarily activate an existing operational certificate chain for the given fabric.
     *
     * The certificate chain is temporary until committed or reverted.
     * The certificate chain is committed to storage on `CommitOpCertsForFabric`.
     * The certificate chain is reverted to prior storage if `RevertPendingOpCerts` is called
     * before `CommitOpCertsForFabric`.
     *
     * Only one pending operational certificate chain is supported at a time and it is illegal
     * to call this method if there was not already a persisted certificate chain for the given
     * fabric.
     *
     * Cryptographic signature verification or path validation are not enforced by this method.
     *
     * If `AddNewOpCertsForFabric` had been called before this method, this method will return
     * CHIP_ERROR_INCORRECT_STATE since it is illegal to update a certificate chain after
     * adding an existing NOC and before committing or reverting the addition.
     *
     * If there is no existing persisted trusted root certificate and NOC chain for the given
     * fabricIndex, this method will return CHIP_ERROR_INCORRECT_STATE since it is
     * illegal in this implementation to store an NOC chain without associated root, and it is illegal
     * to update an opcert for a fabric not already configured.
     *
     * @param fabricIndex - FabricIndex for which to update the operational certificate chain
     * @param noc - Buffer containing the new NOC certificate to use
     * @param icac - Buffer containing the ICAC certificate to use. If no ICAC is needed, `icac.empty()` must be true.
     *
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_NO_MEMORY if there is insufficient memory to maintain the temporary `noc` and `icac` cert copies
     * @retval CHIP_ERROR_INVALID_ARGUMENT if either the noc or icac are invalid sizes
     * @retval CHIP_ERROR_INCORRECT_STATE if the certificate store is not properly initialized, if this method
     *                                    is called after `AddNewOpCertsForFabric`, or if there was
     *                                    already a pending cert chain for the given `fabricIndex`, or if there are
     *                                    no associated persisted root and NOC chain for for the given `fabricIndex`.
     * @retval other CHIP_ERROR value on internal errors
     */
    virtual CHIP_ERROR UpdateOpCertsForFabric(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac) = 0;

    /**
     * @brief Permanently commit the certificate chain last configured via successful calls to
     *        legal combinations of `AddNewTrustedRootCertForFabric`, `AddNewOpCertsForFabric` or
     *        `UpdateOpCertsForFabric`, replacing previously committed data, if any.
     *
     * This is to be used when CommissioningComplete is successfully received
     *
     * @param fabricIndex - FabricIndex for which to commit the certificate chain, used for security cross-checking
     *
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if the certificate store is not properly initialized,
     *                                    or if no valid pending state is available.
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if there is no pending certificate chain for `fabricIndex`
     * @retval other CHIP_ERROR value on internal storage errors
     */
    virtual CHIP_ERROR CommitOpCertsForFabric(FabricIndex fabricIndex) = 0;

    /**
     * @brief Permanently remove the certificate chain associated with a fabric.
     *
     * This is to be used for RemoveFabric. Removes both the pending operational cert chain
     * elements for the fabricIndex (if any) and the committed ones (if any).
     *
     * @param fabricIndex - FabricIndex for which to remove the operational cert chain
     *
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if the certificate store is not properly initialized.
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if there was no operational certificate data at all for `fabricIndex`
     * @retval other CHIP_ERROR value on internal storage errors
     */
    virtual CHIP_ERROR RemoveOpCertsForFabric(FabricIndex fabricIndex) = 0;

    /**
     * @brief Permanently release the operational certificate chain made via successful calls to
     *        legal combinations of `AddNewTrustedRootCertForFabric`, `AddNewOpCertsForFabric` or
     *        `UpdateOpCertsForFabric`, if any.
     *
     * This is to be used when a fail-safe expires prior to CommissioningComplete.
     *
     * This method cannot error-out and must always succeed, even on a no-op. This should
     * be safe to do given that `CommitOpCertsForFabric` must succeed to make an operation
     * certificate chain usable.
     */
    virtual void RevertPendingOpCerts() = 0;

    /**
     * @brief Same as RevertPendingOpCerts(), but leaves pending Trusted Root certs if they had
     *        been added. This is is an operation to support the complex error handling of
     *        AddNOC, where we don't want to have "sticking" ICAC/NOC after validation
     *        problems, but don't want to lose the RCAC given in an AddTrustedRootCertificate
     *        command.
     */
    virtual void RevertPendingOpCertsExceptRoot() = 0;

    /**
     * @brief Get the operational certificate element requested, giving the pending data or committed
     *        data depending on prior `AddNewTrustedRootCertForFabric`, `AddNewOpCertsForFabric` or
     *        `UpdateOpCertsForFabric` calls.
     *
     * On success, the `outCertificate` span is resized to the size of the actual certificate read-back.
     *
     * @param fabricIndex - fabricIndex for which to get the certificate
     * @param element - which element of the cerficate chain to get
     * @param outCertificate - buffer to contain the certificate obtained from persistent or temporary storage
     *
     * @retval CHIP_NO_ERROR on success.
     * @retval CHIP_ERROR_BUFFER_TOO_SMALL if `outCertificate` is too small to fit the certificate found.
     * @retval CHIP_ERROR_INCORRECT_STATE if the certificate store is not properly initialized.
     * @retval CHIP_ERROR_NOT_FOUND if the element cannot be found.
     * @retval CHIP_ERROR_INVALID_FABRIC_INDEX if the fabricIndex is invalid.
     * @retval other CHIP_ERROR value on internal storage errors.
     */
    virtual CHIP_ERROR GetCertificate(FabricIndex fabricIndex, CertChainElement element,
                                      MutableByteSpan & outCertificate) const = 0;
};

/**
 * @brief RAII class to operate on an OperationalCertificateStore with auto-revert if not committed.
 *
 * Use as:
 *
 * CHIP_ERROR FunctionWillReturnWithPendingReverted(....)
 * {
 *     OpCertStoreTransaction transaction(opCertStore);
 *
 *     ReturnErrorOnFailure(transaction->AddNewTrustedRootCertForFabric(...));
 *     ReturnErrorOnFailure(transaction->AddNewOpCertsForFabric(...));
 *     ReturnErrorOnFailure(transaction->CommitOpCertsForFabric(...));
 *
 *     return CHIP_NO_ERROR;
 * }
 */
class OpCertStoreTransaction
{
public:
    explicit OpCertStoreTransaction(OperationalCertificateStore & store) : mStore(store) {}
    ~OpCertStoreTransaction()
    {
        // This is a no-op if CommitOpCertsForFabric had been called on the store
        mStore.RevertPendingOpCerts();
    }

    // Non-copyable
    OpCertStoreTransaction(OpCertStoreTransaction const &) = delete;
    void operator=(OpCertStoreTransaction const &) = delete;

    OperationalCertificateStore * operator->() { return &mStore; }

private:
    OperationalCertificateStore & mStore;
};

} // namespace Credentials
} // namespace chip
