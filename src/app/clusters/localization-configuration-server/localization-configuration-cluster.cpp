/*
 *
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <app/clusters/localization-configuration-server/localization-configuration-cluster.h>
#include <app/persistence/AttributePersistence.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/String.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/LocalizationConfiguration/Metadata.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LocalizationConfiguration;
using namespace chip::app::Clusters::LocalizationConfiguration::Attributes;

using Protocols::InteractionModel::Status;

namespace chip::app::Clusters {

CHIP_ERROR LocalizationConfigurationCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    AttributePersistence persistence(context.attributeStorage);

    // Load the active locale from persistence
    chip::app::Storage::String<kActiveLocaleMaxLength> storedLocale;
    if (persistence.LoadString({ kRootEndpointId, LocalizationConfiguration::Id, Attributes::ActiveLocale::Id }, storedLocale))
    {
        SetActiveLocale(storedLocale.Content());
    }
    else
    {
        ChipLogProgress(AppServer, "Failed to load active locale from persistence, using default locale");
        ReturnErrorOnFailure(persistence.StoreString(
            { kRootEndpointId, LocalizationConfiguration::Id, Attributes::ActiveLocale::Id }, mActiveLocale));
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus LocalizationConfigurationCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                              AttributeValueEncoder & aEncoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        // No features defined for the cluster.
        return aEncoder.Encode<uint32_t>(0);
    case ClusterRevision::Id:
        return aEncoder.Encode(LocalizationConfiguration::kRevision);
    case ActiveLocale::Id:
        return aEncoder.Encode(GetActiveLocale());
    case SupportedLocales::Id:
        return ReadSupportedLocales(aEncoder);
    }
    return Status::UnsupportedAttribute;
}

DataModel::ActionReturnStatus LocalizationConfigurationCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                               AttributeValueDecoder & aDecoder)
{
    AttributePersistence persistence(mContext->attributeStorage);
    switch (request.path.mAttributeId)
    {
    case ActiveLocale::Id: {
        CharSpan label;
        ReturnErrorOnFailure(aDecoder.Decode(label));
        ChipLogProgress(AppServer, "Setting active locale to %s", label.data());
        // Store the string in persistence
        Storage::String<kActiveLocaleMaxLength> shortString;
        VerifyOrReturnError(shortString.SetContent(label), Status::Failure);
        ReturnValueOnFailure(persistence.StoreString(request.path, shortString), Status::Failure);

        return SetActiveLocale(label);
    }
    }
    return Status::UnsupportedAttribute;
}

CHIP_ERROR LocalizationConfigurationCluster::Attributes(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder attributeListBuilder(builder);
    return attributeListBuilder.Append(Span(kMandatoryMetadata), {}, {});
}

Protocols::InteractionModel::Status LocalizationConfigurationCluster::SetActiveLocale(CharSpan activeLocale)
{
    if (!IsSupportedLocale(activeLocale))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    mActiveLocale.SetContent(activeLocale);
    return Protocols::InteractionModel::Status::Success;
}

CharSpan LocalizationConfigurationCluster::GetActiveLocale()
{
    return mActiveLocale.Content();
}

CHIP_ERROR LocalizationConfigurationCluster::ReadSupportedLocales(AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err                                                 = CHIP_NO_ERROR;
    DeviceLayer::DeviceInfoProvider::SupportedLocalesIterator * it = mDeviceInfoProvider.IterateSupportedLocales();
    if (it)
    {
        err = aEncoder.EncodeList([&it](const auto & encoder) -> CHIP_ERROR {
            CharSpan supportedLocale;

            while (it->Next(supportedLocale))
            {
                ReturnErrorOnFailure(encoder.Encode(supportedLocale));
            }

            return CHIP_NO_ERROR;
        });

        it->Release();
    }
    else
    {
        err = aEncoder.EncodeEmptyList();
    }
    return err;
}

bool LocalizationConfigurationCluster::IsSupportedLocale(CharSpan newLangTag)
{
    DeviceLayer::DeviceInfoProvider::SupportedLocalesIterator * it;
    if ((it = mDeviceInfoProvider.IterateSupportedLocales()))
    {
        CharSpan outLocale;

        while (it->Next(outLocale))
        {
            if (outLocale.data_equal(newLangTag))
            {
                it->Release();
                return true;
            }
        }

        it->Release();
    }

    return false;
}

bool LocalizationConfigurationCluster::GetDefaultLocale(MutableCharSpan & outLocale)
{
    bool found = false;
    DeviceLayer::DeviceInfoProvider::SupportedLocalesIterator * it;
    if ((it = mDeviceInfoProvider.IterateSupportedLocales()))
    {
        CharSpan tempLocale;
        found = it->Next(tempLocale);
        CopyCharSpanToMutableCharSpan(tempLocale, outLocale);
        it->Release();
    }
    return found;
}
} // namespace chip::app::Clusters
