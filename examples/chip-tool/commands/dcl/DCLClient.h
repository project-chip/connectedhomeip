/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/Optional.h>

#include <json/json.h>
#include <string>

namespace chip {
namespace tool {
namespace dcl {
class DCLClient
{
public:
    DCLClient(Optional<const char *> hostname, Optional<uint16_t> port);

    /**
     * @brief Retrieves the model information from the DCL based on the onboarding payload.
     *
     * This function uses the onboarding payload (a QR Code or Manual Code) to fetch the model information.
     * It constructs an HTTPS request to retrieve the model data associated with the specified vendor ID and product ID from the
     * payload.
     *
     * @param[in]  onboardingPayload A null-terminated string containing the onboarding payload.
     *                               This can either start with a QR Code prefix or be a Manual Code.
     * @param[out] outModel          A Json::Value object to store the retrieved model information.
     *                               If the vendor and product IDs are missing, this will be set to null.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, error code otherwise.
     */
    CHIP_ERROR Model(const char * onboardingPayload, Json::Value & outModel);

    /**
     * @brief Retrieves the model information from the DCL using vendor ID and product ID.
     *
     * This function constructs an HTTPS request to retrieve the model data associated with the specified vendor ID and product ID.
     *
     * @param[in]  vendorId   The vendor ID of the model (must not be 0).
     * @param[in]  productId  The product ID of the model (must not be 0).
     * @param[out] outModel   A Json::Value object to store the retrieved model information.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, error code otherwise.
     */
    CHIP_ERROR Model(const VendorId vendorId, const uint16_t productId, Json::Value & outModel);

    /**
     * @brief Retrieves the Terms and Conditions from the DCL based on the onboarding payload.
     *
     * This function uses the onboarding payload (a QR Code or Manual Code) to fetch the model information.
     * If the model includes enhanced setup flow options, it requests and validates the associated Terms
     * and Conditions data. If enhanced setup flow is not enabled, the output `tc` is set to null.
     *
     * @param[in]  onboardingPayload A null-terminated string containing the onboarding payload.
     *                               This can either start with a QR Code prefix or be a Manual Code.
     * @param[out] outTc             A Json::Value object to store the retrieved Terms and Conditions data.
     *                               If enhanced setup flow options are not enabled, this will be set to null.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, error code otherwise.
     */
    CHIP_ERROR TermsAndConditions(const char * onboardingPayload, Json::Value & outTc);

    /**
     * @brief Retrieves the Terms and Conditions from the DCL using vendor ID and product ID.
     *
     * This function first retrieves the model information using the specified vendor ID and product ID.
     * If the model includes enhanced setup flow options, it fetches the Terms and Conditions, validates the data, and returns it.
     *
     * @param[in]  vendorId   The vendor ID of the model (must not be 0).
     * @param[in]  productId  The product ID of the model (must not be 0).
     * @param[out] outTc      A Json::Value object to store the retrieved Terms and Conditions data.
     *                        If enhanced setup flow options are not enabled, this will be set to null.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, error code otherwise.
     */
    CHIP_ERROR TermsAndConditions(const chip::VendorId vendorId, const uint16_t productId, Json::Value & outTc);

private:
    std::string mHostName;
    uint16_t mPort;
};
} // namespace dcl
} // namespace tool
} // namespace chip
