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
#include <lib/core/Optional.h>

#include <json/json.h>

namespace chip {
namespace tool {
namespace dcl {
/**
 * Display the terms and conditions to the user and prompt for acceptance.
 *
 * @param[in] tc The terms and conditions JSON object.
 * @param[out] outVersion The schema version of the terms and conditions.
 * @param[out] outUserResponse The user response as a bitfield where each bit corresponds to the ordinal of the text.
 * @param[in] countryCode The country code to use for the terms and conditions. If not provided, the default country will be used.
 * @param[in] languageCode The language code to use for the terms and conditions. If not provided, the default language will be
 * used.
 *
 * @return CHIP_NO_ERROR on success, error code otherwise.
 */
CHIP_ERROR DisplayTermsAndConditions(const Json::Value & tc, uint16_t & outVersion, uint16_t & outUserResponse,
                                     Optional<const char *> countryCode  = NullOptional,
                                     Optional<const char *> languageCode = NullOptional);
} // namespace dcl
} // namespace tool
} // namespace chip
