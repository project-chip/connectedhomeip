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

#include <stdint.h>

#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {

/**
 * @brief Represents a pair of terms and conditions value and version.
 *
 * This class encapsulates terms and conditions with methods to validate a user's accepted value and version against required
 * criteria.
 */
class TermsAndConditions
{
public:
    TermsAndConditions(uint16_t inValue, uint16_t inVersion) : value(inValue), version(inVersion) {}

    bool operator==(const TermsAndConditions & other) const { return value == other.value && version == other.version; }
    bool operator!=(const TermsAndConditions & other) const { return !(*this == other); }

    /**
     * @brief Retrieves the terms and conditions value (accepted bits).
     *
     * @return The value of the terms and conditions.
     */
    uint16_t GetValue() const { return value; }

    /**
     * @brief Retrieves the terms and conditions version.
     *
     * @return The version of the terms and conditions.
     */
    uint16_t GetVersion() const { return version; }

    /**
     * @brief Validates the terms and conditions value.
     *
     * Checks whether all required bits are set in the accepted terms and conditions.
     *
     * @param acceptedTermsAndConditions The user's accepted terms and conditions.
     * @return True if all required bits are set, false otherwise.
     */
    bool ValidateValue(const TermsAndConditions & acceptedTermsAndConditions) const
    {
        // Check if all required bits are set in the user-accepted value.
        return ((value & acceptedTermsAndConditions.GetValue()) == value);
    }

    /**
     * @brief Validates the terms and conditions version.
     *
     * Checks whether the accepted version is greater than or equal to the required version.
     *
     * @param acceptedTermsAndConditions The user's accepted terms and conditions.
     * @return True if the accepted version is valid, false otherwise.
     */
    bool ValidateVersion(const TermsAndConditions & acceptedTermsAndConditions) const
    {
        // Check if the version is below the minimum required version.
        return (acceptedTermsAndConditions.GetVersion() >= version);
    }

    /**
     * @brief Validates the terms and conditions.
     *
     * Combines validation of both value and version to ensure compliance with requirements.
     *
     * @param acceptedTermsAndConditions The user's accepted terms and conditions.
     * @return True if both value and version validations pass, false otherwise.
     */
    bool Validate(const TermsAndConditions & acceptedTermsAndConditions) const
    {
        return ValidateVersion(acceptedTermsAndConditions) && ValidateValue(acceptedTermsAndConditions);
    }

private:
    const uint16_t value;
    const uint16_t version;
};

/**
 * @brief Data access layer for handling the required terms and conditions and managing user acceptance status.
 *
 * This class provides methods to manage the acceptance of terms and conditions, including storing, retrieving,
 * and verifying the acceptance status. It also supports temporary in-memory storage and persistent storage for
 * accepted terms and conditions.
 */
class TermsAndConditionsProvider
{
public:
    virtual ~TermsAndConditionsProvider() = default;

    /**
     * @brief Persists the acceptance of the terms and conditions.
     *
     * This method commits the in-memory acceptance status to persistent storage. It stores the acceptance
     * status in a permanent location and clears the temporary in-memory acceptance state after committing.
     *
     * @retval CHIP_NO_ERROR if the terms were successfully persisted.
     * @retval CHIP_ERROR_UNINITIALIZED if the module has not been initialized.
     * @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR CommitAcceptance() = 0;

    /**
     * @brief Retrieves the current acceptance status of the terms and conditions.
     *
     * This method checks the temporary in-memory acceptance state first. If no in-memory state is found,
     * it attempts to retrieve the acceptance status from persistent storage. If no terms have been accepted,
     * it returns an empty `Optional`.
     *
     * @param[out] outTermsAndConditions The current accepted terms and conditions, if any.
     *
     * @retval CHIP_NO_ERROR if the terms were successfully retrieved or no terms were found.
     * @retval CHIP_ERROR_UNINITIALIZED if the module has not been initialized.
     * @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR GetAcceptance(Optional<TermsAndConditions> & outTermsAndConditions) const = 0;

    /**
     * @brief Determines if acknowledgments are required.
     *
     * @param[out] outAcknowledgementsRequired True if acknowledgments are required, false otherwise.
     *
     * @retval CHIP_NO_ERROR if successful.
     * @retval CHIP_ERROR_UNINITIALIZED if the module has not been initialized.
     * @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR GetAcknowledgementsRequired(bool & outAcknowledgementsRequired) const = 0;

    /**
     * @brief Retrieves the requirements for the terms and conditions.
     *
     * This method retrieves the required terms and conditions that must be accepted by the user. These
     * requirements are set by the provider and used to validate the acceptance.
     *
     * @param[out] outTermsAndConditions The required terms and conditions.
     *
     * @retval CHIP_NO_ERROR if the required terms were successfully retrieved.
     * @retval CHIP_ERROR_UNINITIALIZED if the module has not been initialized.
     * @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR GetRequirements(Optional<TermsAndConditions> & outTermsAndConditions) const = 0;

    /**
     * @brief Retrieves the deadline for accepting updated terms and conditions.
     *
     * This method retrieves the deadline by which the user must accept updated terms and conditions.
     * If no deadline is set, it returns an empty `Optional`.
     *
     * @param[out] outUpdateAcceptanceDeadline The deadline (in seconds) by which updated terms must be accepted.
     *                                         Returns empty Optional if no deadline is set.
     *
     * @retval CHIP_NO_ERROR if the deadline was successfully retrieved or no deadline was found.
     * @retval CHIP_ERROR_UNINITIALIZED if the module has not been initialized.
     * @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR GetUpdateAcceptanceDeadline(Optional<uint32_t> & outUpdateAcceptanceDeadline) const = 0;

    /**
     * @brief Resets the persisted acceptance status.
     *
     * This method deletes the persisted acceptance of the terms and conditions from storage, effectively
     * resetting the stored acceptance status. Any in-memory temporary acceptance will also be cleared
     * through this method.
     *
     * @retval CHIP_NO_ERROR if the terms were successfully reset.
     * @retval CHIP_ERROR_UNINITIALIZED if the module has not been initialized.
     * @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR ResetAcceptance() = 0;

    /**
     * @brief Clears the in-memory temporary acceptance status.
     *
     * This method clears any temporary acceptance of the terms and conditions that is held in-memory. It does
     * not affect the persisted state stored in storage.
     *
     * @retval CHIP_NO_ERROR if the in-memory acceptance state was successfully cleared.
     * @retval CHIP_ERROR_UNINITIALIZED if the module has not been initialized.
     * @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR RevertAcceptance() = 0;

    /**
     * @brief Sets the temporary in-memory acceptance status of the terms and conditions.
     *
     * This method stores the provided terms and conditions acceptance status in-memory. It does not persist
     * the acceptance status to storage. To persist the acceptance, call `CommitAcceptance()` after this method.
     *
     * @param[in] inTermsAndConditions The terms and conditions to be accepted temporarily.
     *
     * @retval CHIP_NO_ERROR if the terms were successfully stored in-memory.
     * @retval CHIP_ERROR_INVALID_ARGUMENT if the provided terms and conditions are invalid.
     * @retval CHIP_ERROR_UNINITIALIZED if the module has not been initialized.
     * @retval CHIP_ERROR_* for other errors.
     */
    virtual CHIP_ERROR SetAcceptance(const Optional<TermsAndConditions> & inTermsAndConditions) = 0;
};

} // namespace app
} // namespace chip
