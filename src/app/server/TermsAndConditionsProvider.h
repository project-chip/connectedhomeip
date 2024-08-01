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

#include <system/SystemConfig.h>

#include <stdint.h>

#include <lib/core/CHIPError.h>

namespace chip {
namespace app {

/**
 * @brief Data access layer for the required terms and conditions and the store for the user acceptance.
 */
class TermsAndConditionsProvider
{
public:
    /**
     * @brief Destructor.
     */
    virtual ~TermsAndConditionsProvider() = default;

    /**
     * @brief Sets the acceptance status of the required terms and conditions.
     */
    virtual CHIP_ERROR ClearAcceptance() = 0;

    /**
     * @brief Retrieves the acceptance status of the required terms and conditions.
     *
     * @param[out] outAcknowledgementsValue The bitmask of acknowledgements accepted.
     * @param[out] outAcknowledgementsVersionValue The version of the accepted acknowledgements.
     */
    virtual CHIP_ERROR GetAcceptance(uint16_t & outAcknowledgementsValue, uint16_t & outAcknowledgementsVersionValue) const = 0;

    /**
     * @brief Retrieves the requirements of the terms and conditions.
     *
     * @param[out] outAcknowledgementsValue The bitmask of required acknowledgements.
     * @param[out] outAcknowledgementsVersionValue The version of the required acknowledgements.
     */
    virtual CHIP_ERROR GetRequirements(uint16_t & outAcknowledgementsValue, uint16_t & outAcknowledgementsVersionValue) const = 0;

    /**
     * @brief Sets the acceptance status of the required terms and conditions.
     *
     * @param[in] inAcknowledgementsValue The bitmask of acknowledgements that was accepted.
     * @param[in] inAcknowledgementsVersionValue The version of the acknowledgements that was accepted.
     */
    virtual CHIP_ERROR SetAcceptance(uint16_t inAcknowledgementsValue, uint16_t inAcknowledgementsVersionValue) = 0;
};

}; // namespace app
}; // namespace chip
