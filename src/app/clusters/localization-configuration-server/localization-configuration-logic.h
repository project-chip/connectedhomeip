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

#include <app/AttributeValueEncoder.h>
#include <app/persistence/String.h>
#include <lib/support/Span.h>
#include <platform/DeviceInfoProvider.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

class LocalizationConfigurationServerLogic
{
public:
    LocalizationConfigurationServerLogic(DeviceLayer::DeviceInfoProvider & aDeviceInfoProvider) :
        mDeviceInfoProvider(aDeviceInfoProvider)
    {}

    /*
     * Set the active locale.
     *
     * @param activeLocale The active locale to set.
     * @return InteractionModel::Status::Success on success, InteractionModel::Status::ConstraintError if the locale is not valid or
     * supportedLocale.
     */
    Protocols::InteractionModel::Status SetActiveLocale(CharSpan activeLocale);

    /*
     * Get the active locale.
     *
     * @return The active locale.
     */
    CharSpan GetActiveLocale();

    /*
     * Read the supported locales.
     *
     * @param aEncoder The encoder to write the supported locales to.
     * @return CHIP_NO_ERROR on success.
     */
    CHIP_ERROR ReadSupportedLocales(AttributeValueEncoder & aEncoder);

    /*
     * Check if a locale is supported.
     *
     * @param newLangTag The locale to check.
     * @return true if the locale is supported, false otherwise.
     */
    bool IsSupportedLocale(CharSpan newLangTag);

private:
    Storage::String<35> mActiveLocale;
    DeviceLayer::DeviceInfoProvider & mDeviceInfoProvider;
};
} // namespace Clusters
} // namespace app
} // namespace chip
