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
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/LocalizationConfiguration/ClusterId.h>
#include <lib/support/Span.h>
#include <lib/support/TypeTraits.h>
#include <platform/DeviceInfoProvider.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
constexpr size_t kActiveLocaleMaxLength = 35;
class LocalizationConfigurationCluster : public DefaultServerCluster
{
public:
    LocalizationConfigurationCluster(DeviceLayer::DeviceInfoProvider & aDeviceInfoProvider, CharSpan activeLocale);

    // Server cluster implementation
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & aEncoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & aDecoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    /*
     * Set the active locale.
     *
     * @param activeLocale The active locale to set.
     * @return Protocols::InteractionModel::Status::Success on success, Protocols::InteractionModel::Status::ConstraintError if the
     * locale is not valid or supportedLocale.
     */
    DataModel::ActionReturnStatus SetActiveLocale(CharSpan activeLocale);

    CharSpan GetActiveLocale();

private:
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
    bool IsSupportedLocale(CharSpan newLangTag) const;

    /**
     * @brief Get the default locale of the device.
     *
     * @param outLocale - the default locale of the device.
     * @return true if the default locale is found, false otherwise.
     */
    virtual bool GetDefaultLocale(MutableCharSpan & outLocale);

protected:
    Storage::String<kActiveLocaleMaxLength> mActiveLocale;
    DeviceLayer::DeviceInfoProvider & mDeviceInfoProvider;
};
} // namespace Clusters
} // namespace app
} // namespace chip
