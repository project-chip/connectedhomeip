/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "TermsAndConditionsProvider.h"

#include <stdint.h>

#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {

/**
 * @brief Abstract interface for storing and retrieving terms and conditions acceptance status.
 *
 * This class defines the methods required to interact with the underlying storage system
 * for saving, retrieving, and deleting the user's acceptance of terms and conditions.
 */
class TermsAndConditionsStorageDelegate
{
public:
    virtual ~TermsAndConditionsStorageDelegate() = default;

    /**
     * @brief Deletes the persisted terms and conditions acceptance status from storage.
     *
     * This method deletes the stored record of the user's acceptance of the terms and conditions,
     * effectively resetting their acceptance status in the persistent storage.
     *
     * @retval CHIP_NO_ERROR if the record was successfully deleted.
     * @retval CHIP_ERROR_UNINITIALIZED if the storage delegate is not properly initialized.
     * @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR Delete() = 0;

    /**
     * @brief Retrieves the persisted terms and conditions acceptance status from storage.
     *
     * This method attempts to retrieve the previously accepted terms and conditions from
     * persistent storage. If no such record exists, it returns an empty `Optional`.
     *
     * @param[out] outTermsAndConditions The retrieved terms and conditions, if any exist.
     *
     * @retval CHIP_NO_ERROR if the terms were successfully retrieved.
     * @retval CHIP_ERROR_UNINITIALIZED if the storage delegate is not properly initialized.
     * @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR Get(Optional<TermsAndConditions> & outTermsAndConditions) = 0;

    /**
     * @brief Persists the user's acceptance of the terms and conditions.
     *
     * This method stores the provided terms and conditions acceptance status in persistent
     * storage, allowing the user's acceptance to be retrieved later.
     *
     * @param[in] inTermsAndConditions The terms and conditions to be saved.
     *
     * @retval CHIP_NO_ERROR if the terms were successfully stored.
     * @retval CHIP_ERROR_UNINITIALIZED if the storage delegate is not properly initialized.
     * @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR Set(const TermsAndConditions & inTermsAndConditions) = 0;
};

/**
 * @brief Default implementation of the TermsAndConditionsStorageDelegate using a persistent storage backend.
 *
 * This class provides an implementation of the TermsAndConditionsStorageDelegate interface, storing
 * and retrieving the user's terms and conditions acceptance from persistent storage. It requires a
 * PersistentStorageDelegate to interface with the storage system.
 */
class DefaultTermsAndConditionsStorageDelegate : public TermsAndConditionsStorageDelegate
{
public:
    /**
     * @brief Initializes the storage delegate with a persistent storage backend.
     *
     * This method initializes the storage delegate with the provided persistent storage delegate.
     * The storage delegate must be initialized before performing any operations.
     *
     * @param[in] inPersistentStorageDelegate The storage backend used for saving and retrieving data.
     *
     * @retval CHIP_NO_ERROR if the storage delegate was successfully initialized.
     * @retval CHIP_ERROR_INVALID_ARGUMENT if the provided storage delegate is null.
     */
    CHIP_ERROR Init(PersistentStorageDelegate * inPersistentStorageDelegate);

    CHIP_ERROR Delete() override;

    CHIP_ERROR Get(Optional<TermsAndConditions> & inTermsAndConditions) override;

    CHIP_ERROR Set(const TermsAndConditions & inTermsAndConditions) override;

private:
    PersistentStorageDelegate * mStorageDelegate = nullptr;
};

class DefaultTermsAndConditionsProvider : public TermsAndConditionsProvider
{
public:
    /**
     * @brief Initializes the TermsAndConditionsProvider.
     *
     * @param[in] inStorageDelegate Storage delegate dependency.
     * @param[in] inRequiredTermsAndConditions The required terms and conditions that must be met.
     */
    CHIP_ERROR Init(TermsAndConditionsStorageDelegate * inStorageDelegate,
                    const Optional<TermsAndConditions> & inRequiredTermsAndConditions);

    CHIP_ERROR CommitAcceptance() override;

    CHIP_ERROR GetAcceptance(Optional<TermsAndConditions> & outTermsAndConditions) const override;

    CHIP_ERROR GetAcknowledgementsRequired(bool & outAcknowledgementsRequired) const override;

    CHIP_ERROR GetRequirements(Optional<TermsAndConditions> & outTermsAndConditions) const override;

    CHIP_ERROR GetUpdateAcceptanceDeadline(Optional<uint32_t> & outUpdateAcceptanceDeadline) const override;

    CHIP_ERROR ResetAcceptance() override;

    CHIP_ERROR RevertAcceptance() override;

    CHIP_ERROR SetAcceptance(const Optional<TermsAndConditions> & inTermsAndConditions) override;

private:
    TermsAndConditionsStorageDelegate * mTermsAndConditionsStorageDelegate;
    Optional<TermsAndConditions> mTemporalAcceptance;
    Optional<TermsAndConditions> mRequiredAcknowledgements;
};

} // namespace app
} // namespace chip
