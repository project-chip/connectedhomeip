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

#include <credentials/CHIPCert.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>

#include "PersistentStorageOpCertStore.h"

namespace chip {
namespace Credentials {

namespace {

using CertChainElement = OperationalCertificateStore::CertChainElement;

const char * GetStorageKeyForCert(DefaultStorageKeyAllocator & keyAllocator, FabricIndex fabricIndex, CertChainElement element)
{
    const char * storageKey = nullptr;

    switch (element)
    {
    case CertChainElement::kNoc:
        storageKey = keyAllocator.FabricNOC(fabricIndex);
        break;
    case CertChainElement::kIcac:
        storageKey = keyAllocator.FabricICAC(fabricIndex);
        break;
    case CertChainElement::kRcac:
        storageKey = keyAllocator.FabricRCAC(fabricIndex);
        break;
    default:
        break;
    }

    return storageKey;
}

bool StorageHasCertificate(PersistentStorageDelegate * storage, FabricIndex fabricIndex, CertChainElement element)
{
    DefaultStorageKeyAllocator keyAllocator;
    const char * storageKey = GetStorageKeyForCert(keyAllocator, fabricIndex, element);

    if (storageKey == nullptr)
    {
        return false;
    }

    // TODO(#16958): need to actually read the cert to know if it's there due to platforms not
    //               properly enforcing CHIP_ERROR_BUFFER_TOO_SMALL behavior needed by
    //               PersistentStorageDelegate.
    uint8_t placeHolderCertBuffer[kMaxCHIPCertLength];

    uint16_t keySize = sizeof(placeHolderCertBuffer);
    CHIP_ERROR err   = storage->SyncGetKeyValue(storageKey, &placeHolderCertBuffer[0], keySize);

    return (err == CHIP_NO_ERROR);
}

CHIP_ERROR LoadCertFromStorage(PersistentStorageDelegate * storage, FabricIndex fabricIndex, CertChainElement element,
                               MutableByteSpan & outCert)
{
    DefaultStorageKeyAllocator keyAllocator;
    const char * storageKey = GetStorageKeyForCert(keyAllocator, fabricIndex, element);

    uint16_t keySize = static_cast<uint16_t>(outCert.size());
    CHIP_ERROR err   = storage->SyncGetKeyValue(storageKey, outCert.data(), keySize);

    // Not finding an ICAC means we don't have one, so adjust to meet the API contract, where
    // outCert.empty() will be true;
    if ((element == CertChainElement::kIcac) && (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND))
    {
        outCert.reduce_size(0);
        return CHIP_ERROR_NOT_FOUND;
    }

    if (err == CHIP_NO_ERROR)
    {
        outCert.reduce_size(keySize);
    }
    else if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // Convert persisted storage error to CHIP_ERROR_NOT_FOUND so that
        // `PersistentStorageOpCertStore::GetCertificate` doesn't need to convert.
        err = CHIP_ERROR_NOT_FOUND;
    }

    return err;
}

CHIP_ERROR SaveCertToStorage(PersistentStorageDelegate * storage, FabricIndex fabricIndex, CertChainElement element,
                             const ByteSpan & cert)
{
    DefaultStorageKeyAllocator keyAllocator;
    const char * storageKey = GetStorageKeyForCert(keyAllocator, fabricIndex, element);

    // If provided an empty ICAC, we delete the ICAC key previously used. If not there, it's OK
    if ((element == CertChainElement::kIcac) && (cert.empty()))
    {
        CHIP_ERROR err = storage->SyncDeleteKeyValue(storageKey);
        if ((err == CHIP_NO_ERROR) || (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND))
        {
            return CHIP_NO_ERROR;
        }
        return err;
    }

    return storage->SyncSetKeyValue(storageKey, cert.data(), static_cast<uint16_t>(cert.size()));
}

CHIP_ERROR DeleteCertFromStorage(PersistentStorageDelegate * storage, FabricIndex fabricIndex, CertChainElement element)
{
    DefaultStorageKeyAllocator keyAllocator;
    const char * storageKey = GetStorageKeyForCert(keyAllocator, fabricIndex, element);
    return storage->SyncDeleteKeyValue(storageKey);
}

} // namespace

bool PersistentStorageOpCertStore::HasPendingRootCert() const
{
    if (mStorage == nullptr)
    {
        return false;
    }

    return (mPendingRcac.Get() != nullptr) && mStateFlags.Has(StateFlags::kAddNewTrustedRootCalled);
}

bool PersistentStorageOpCertStore::HasPendingNocChain() const
{
    if (mStorage == nullptr)
    {
        return false;
    }

    return (mPendingNoc.Get() != nullptr) && mStateFlags.HasAny(StateFlags::kAddNewOpCertsCalled, StateFlags::kUpdateOpCertsCalled);
}

bool PersistentStorageOpCertStore::HasCertificateForFabric(FabricIndex fabricIndex, CertChainElement element) const
{
    if ((mStorage == nullptr) || !IsValidFabricIndex(fabricIndex))
    {
        return false;
    }

    // FabricIndex matches pending, we MAY have some pending data
    if (fabricIndex == mPendingFabricIndex)
    {
        switch (element)
        {
        case CertChainElement::kRcac:
            if (mPendingRcac.Get() != nullptr)
            {
                return true;
            }
            break;
        case CertChainElement::kIcac:
            if (mPendingIcac.Get() != nullptr)
            {
                return true;
            }
            // If we have a pending NOC and no pending ICAC, don't delegate to storage, return not found here
            // since in the pending state, there truly is nothing.
            if (mPendingNoc.Get() != nullptr)
            {
                return false;
            }
            break;
        case CertChainElement::kNoc:
            if (mPendingNoc.Get() != nullptr)
            {
                return true;
            }
            break;
        default:
            return false;
        }
    }

    return StorageHasCertificate(mStorage, fabricIndex, element);
}

CHIP_ERROR PersistentStorageOpCertStore::AddNewTrustedRootCertForFabric(FabricIndex fabricIndex, const ByteSpan & rcac)
{
    ReturnErrorCodeIf(mStorage == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(!IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    ReturnErrorCodeIf(rcac.empty() || (rcac.size() > Credentials::kMaxCHIPCertLength), CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorCodeIf(mStateFlags.HasAny(StateFlags::kUpdateOpCertsCalled, StateFlags::kAddNewTrustedRootCalled,
                                         StateFlags::kAddNewOpCertsCalled),
                      CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(StorageHasCertificate(mStorage, fabricIndex, CertChainElement::kRcac), CHIP_ERROR_INCORRECT_STATE);

    Platform::ScopedMemoryBufferWithSize<uint8_t> rcacBuf;
    ReturnErrorCodeIf(!rcacBuf.Alloc(rcac.size()), CHIP_ERROR_NO_MEMORY);
    memcpy(rcacBuf.Get(), rcac.data(), rcac.size());

    mPendingRcac = std::move(rcacBuf);

    mPendingFabricIndex = fabricIndex;
    mStateFlags.Set(StateFlags::kAddNewTrustedRootCalled);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOpCertStore::AddNewOpCertsForFabric(FabricIndex fabricIndex, const ByteSpan & noc,
                                                                const ByteSpan & icac)
{
    ReturnErrorCodeIf(mStorage == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(!IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    ReturnErrorCodeIf(noc.empty() || (noc.size() > Credentials::kMaxCHIPCertLength), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(icac.size() > Credentials::kMaxCHIPCertLength, CHIP_ERROR_INVALID_ARGUMENT);

    // Can't have called UpdateOpCertsForFabric first, or called with pending certs
    ReturnErrorCodeIf(mStateFlags.HasAny(StateFlags::kUpdateOpCertsCalled, StateFlags::kAddNewOpCertsCalled),
                      CHIP_ERROR_INCORRECT_STATE);

    // Need to have trusted roots installed to make the chain valid
    ReturnErrorCodeIf(!mStateFlags.Has(StateFlags::kAddNewTrustedRootCalled), CHIP_ERROR_INCORRECT_STATE);

    // fabricIndex must match the current pending fabric
    ReturnErrorCodeIf(fabricIndex != mPendingFabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Can't have persisted NOC/ICAC for same fabric if adding
    ReturnErrorCodeIf(StorageHasCertificate(mStorage, fabricIndex, CertChainElement::kNoc), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(StorageHasCertificate(mStorage, fabricIndex, CertChainElement::kIcac), CHIP_ERROR_INCORRECT_STATE);

    Platform::ScopedMemoryBufferWithSize<uint8_t> nocBuf;
    ReturnErrorCodeIf(!nocBuf.Alloc(noc.size()), CHIP_ERROR_NO_MEMORY);
    memcpy(nocBuf.Get(), noc.data(), noc.size());

    Platform::ScopedMemoryBufferWithSize<uint8_t> icacBuf;
    if (icac.size() > 0)
    {
        ReturnErrorCodeIf(!icacBuf.Alloc(icac.size()), CHIP_ERROR_NO_MEMORY);
        memcpy(icacBuf.Get(), icac.data(), icac.size());
    }

    mPendingNoc  = std::move(nocBuf);
    mPendingIcac = std::move(icacBuf);

    mStateFlags.Set(StateFlags::kAddNewOpCertsCalled);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOpCertStore::UpdateOpCertsForFabric(FabricIndex fabricIndex, const ByteSpan & noc,
                                                                const ByteSpan & icac)
{
    ReturnErrorCodeIf(mStorage == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(!IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    ReturnErrorCodeIf(noc.empty() || (noc.size() > Credentials::kMaxCHIPCertLength), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(icac.size() > Credentials::kMaxCHIPCertLength, CHIP_ERROR_INVALID_ARGUMENT);

    // Can't have called AddNewOpCertsForFabric first, and should never get here after AddNewTrustedRootCertForFabric.
    ReturnErrorCodeIf(mStateFlags.HasAny(StateFlags::kAddNewOpCertsCalled, StateFlags::kAddNewTrustedRootCalled),
                      CHIP_ERROR_INCORRECT_STATE);

    // Can't have already pending NOC from UpdateOpCerts not yet committed
    ReturnErrorCodeIf(mStateFlags.Has(StateFlags::kUpdateOpCertsCalled), CHIP_ERROR_INCORRECT_STATE);

    // Need to have trusted roots installed to make the chain valid
    ReturnErrorCodeIf(!StorageHasCertificate(mStorage, fabricIndex, CertChainElement::kRcac), CHIP_ERROR_INCORRECT_STATE);

    // Must have persisted NOC for same fabric if updating
    ReturnErrorCodeIf(!StorageHasCertificate(mStorage, fabricIndex, CertChainElement::kNoc), CHIP_ERROR_INCORRECT_STATE);

    // Don't check for ICAC, we may not have had one before, but assume that if NOC is there, a
    // previous chain was at least partially there

    Platform::ScopedMemoryBufferWithSize<uint8_t> nocBuf;
    ReturnErrorCodeIf(!nocBuf.Alloc(noc.size()), CHIP_ERROR_NO_MEMORY);
    memcpy(nocBuf.Get(), noc.data(), noc.size());

    Platform::ScopedMemoryBufferWithSize<uint8_t> icacBuf;
    if (icac.size() > 0)
    {
        ReturnErrorCodeIf(!icacBuf.Alloc(icac.size()), CHIP_ERROR_NO_MEMORY);
        memcpy(icacBuf.Get(), icac.data(), icac.size());
    }

    mPendingNoc  = std::move(nocBuf);
    mPendingIcac = std::move(icacBuf);

    // For NOC update, UpdateOpCertsForFabric is what determines the pending fabric index,
    // not a previous AddNewTrustedRootCertForFabric call.
    mPendingFabricIndex = fabricIndex;

    mStateFlags.Set(StateFlags::kUpdateOpCertsCalled);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOpCertStore::CommitOpCertsForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    VerifyOrReturnError(HasPendingNocChain(), CHIP_ERROR_INCORRECT_STATE);
    if (HasPendingRootCert())
    {
        // Neither of these conditions should have occurred based on other interlocks, but since
        // committing certificates is a dangerous operation, we absolutely validate our assumptions.
        ReturnErrorCodeIf(mStateFlags.Has(StateFlags::kUpdateOpCertsCalled), CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(!mStateFlags.Has(StateFlags::kAddNewTrustedRootCalled), CHIP_ERROR_INCORRECT_STATE);
    }

    // TODO: Handle transaction marking to revert partial certs at next boot if we get interrupted by reboot.

    // Start committing NOC first so we don't have dangling roots if one was added.
    ByteSpan pendingNocSpan{ mPendingNoc.Get(), mPendingNoc.AllocatedSize() };
    CHIP_ERROR nocErr = SaveCertToStorage(mStorage, mPendingFabricIndex, CertChainElement::kNoc, pendingNocSpan);

    // ICAC storage handles deleting on empty/missing
    ByteSpan pendingIcacSpan{ mPendingIcac.Get(), mPendingIcac.AllocatedSize() };
    CHIP_ERROR icacErr = SaveCertToStorage(mStorage, mPendingFabricIndex, CertChainElement::kIcac, pendingIcacSpan);

    CHIP_ERROR rcacErr = CHIP_NO_ERROR;
    if (HasPendingRootCert())
    {
        ByteSpan pendingRcacSpan{ mPendingRcac.Get(), mPendingRcac.AllocatedSize() };
        rcacErr = SaveCertToStorage(mStorage, mPendingFabricIndex, CertChainElement::kRcac, pendingRcacSpan);
    }

    // Remember which was the first error, and if any error occurred.
    CHIP_ERROR stickyErr = nocErr;
    stickyErr            = (stickyErr != CHIP_NO_ERROR) ? stickyErr : icacErr;
    stickyErr            = (stickyErr != CHIP_NO_ERROR) ? stickyErr : rcacErr;

    if (stickyErr != CHIP_NO_ERROR)
    {
        // On Adds rather than updates, remove anything possibly stored for the new fabric on partial
        // failure.
        if (mStateFlags.Has(StateFlags::kAddNewOpCertsCalled))
        {
            (void) DeleteCertFromStorage(mStorage, mPendingFabricIndex, CertChainElement::kNoc);
            (void) DeleteCertFromStorage(mStorage, mPendingFabricIndex, CertChainElement::kIcac);
        }
        if (mStateFlags.Has(StateFlags::kAddNewTrustedRootCalled))
        {
            (void) DeleteCertFromStorage(mStorage, mPendingFabricIndex, CertChainElement::kRcac);
        }
        if (mStateFlags.Has(StateFlags::kUpdateOpCertsCalled))
        {
            // Can't do anything to clean-up here, but pretty sure the fabric is broken now...
            // TODO: Handle transaction marking to revert certs if somehow failing store on update by pre-backing-up opcerts
        }

        return stickyErr;
    }

    // If we got here, we succeeded and can reset the pending certs: next `GetCertificate` will use the stored certs
    RevertPendingOpCerts();
    return CHIP_NO_ERROR;
}

bool PersistentStorageOpCertStore::HasAnyCertificateForFabric(FabricIndex fabricIndex) const
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), false);

    bool rcacMissing = !StorageHasCertificate(mStorage, fabricIndex, CertChainElement::kRcac);
    bool icacMissing = !StorageHasCertificate(mStorage, fabricIndex, CertChainElement::kIcac);
    bool nocMissing  = !StorageHasCertificate(mStorage, fabricIndex, CertChainElement::kNoc);
    bool anyPending  = (mPendingRcac.Get() != nullptr) || (mPendingIcac.Get() != nullptr) || (mPendingNoc.Get() != nullptr);

    if (rcacMissing && icacMissing && nocMissing && !anyPending)
    {
        return false;
    }

    return true;
}

CHIP_ERROR PersistentStorageOpCertStore::RemoveOpCertsForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // If there was *no* state, pending or persisted, we have an error
    ReturnErrorCodeIf(!HasAnyCertificateForFabric(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Clear any pending state
    RevertPendingOpCerts();

    // Remove all persisted certs for the given fabric, blindly
    CHIP_ERROR nocErr  = DeleteCertFromStorage(mStorage, fabricIndex, CertChainElement::kNoc);
    CHIP_ERROR icacErr = DeleteCertFromStorage(mStorage, fabricIndex, CertChainElement::kIcac);
    CHIP_ERROR rcacErr = DeleteCertFromStorage(mStorage, fabricIndex, CertChainElement::kRcac);

    // Ignore missing cert errors
    nocErr  = (nocErr == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND) ? CHIP_NO_ERROR : nocErr;
    icacErr = (icacErr == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND) ? CHIP_NO_ERROR : icacErr;
    rcacErr = (rcacErr == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND) ? CHIP_NO_ERROR : rcacErr;

    // Find the first error and return that
    CHIP_ERROR stickyErr = nocErr;
    stickyErr            = (stickyErr != CHIP_NO_ERROR) ? stickyErr : icacErr;
    stickyErr            = (stickyErr != CHIP_NO_ERROR) ? stickyErr : rcacErr;

    return stickyErr;
}

CHIP_ERROR PersistentStorageOpCertStore::GetPendingCertificate(FabricIndex fabricIndex, CertChainElement element,
                                                               MutableByteSpan & outCertificate) const
{
    if (fabricIndex != mPendingFabricIndex)
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    // FabricIndex matches pending, we MAY have some pending data
    switch (element)
    {
    case CertChainElement::kRcac:
        if (mPendingRcac.Get() != nullptr)
        {
            ByteSpan rcacSpan{ mPendingRcac.Get(), mPendingRcac.AllocatedSize() };
            return CopySpanToMutableSpan(rcacSpan, outCertificate);
        }
        break;
    case CertChainElement::kIcac:
        if (mPendingIcac.Get() != nullptr)
        {
            ByteSpan icacSpan{ mPendingIcac.Get(), mPendingIcac.AllocatedSize() };
            return CopySpanToMutableSpan(icacSpan, outCertificate);
        }
        break;
    case CertChainElement::kNoc:
        if (mPendingNoc.Get() != nullptr)
        {
            ByteSpan nocSpan{ mPendingNoc.Get(), mPendingNoc.AllocatedSize() };
            return CopySpanToMutableSpan(nocSpan, outCertificate);
        }
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR PersistentStorageOpCertStore::GetCertificate(FabricIndex fabricIndex, CertChainElement element,
                                                        MutableByteSpan & outCertificate) const
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Handle case of pending data
    CHIP_ERROR err = GetPendingCertificate(fabricIndex, element, outCertificate);
    if ((err == CHIP_NO_ERROR) || (err != CHIP_ERROR_NOT_FOUND))
    {
        // Found in pending, or got a deeper error: return the pending cert status.
        return err;
    }

    // If we have a pending NOC and no pending ICAC, don't delegate to storage, return not found here
    // since in the pending state, there truly is nothing.

    if ((err == CHIP_ERROR_NOT_FOUND) && (element == CertChainElement::kIcac) && (mPendingNoc.Get() != nullptr))
    {
        // Don't delegate to storage if we just have a pending NOC and are missing the ICAC
        return CHIP_ERROR_NOT_FOUND;
    }

    // Not found in pending, let's look in persisted
    return LoadCertFromStorage(mStorage, fabricIndex, element, outCertificate);
}

} // namespace Credentials
} // namespace chip
