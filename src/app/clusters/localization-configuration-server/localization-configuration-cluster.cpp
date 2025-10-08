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

namespace {
class AutoReleaseIterator
{
public:
    using Iterator = DeviceLayer::DeviceInfoProvider::SupportedLocalesIterator;

    explicit AutoReleaseIterator(DeviceLayer::DeviceInfoProvider * provider) :
        mIterator(provider != nullptr ? provider->IterateSupportedLocales() : nullptr)
    {}
    ~AutoReleaseIterator()
    {
        if (mIterator != nullptr)
        {
            mIterator->Release();
        }
    }

    // Delete copy constructor and assignment
    AutoReleaseIterator(const AutoReleaseIterator &)             = delete;
    AutoReleaseIterator & operator=(const AutoReleaseIterator &) = delete;

    bool IsValid() const { return mIterator != nullptr; }
    bool Next(CharSpan & value) { return (mIterator == nullptr) ? false : mIterator->Next(value); }

private:
    Iterator * mIterator;
};
} // namespace

namespace chip::app::Clusters {

LocalizationConfigurationCluster::LocalizationConfigurationCluster(DeviceLayer::DeviceInfoProvider & aDeviceInfoProvider,
                                                                   CharSpan activeLocale) :
    DefaultServerCluster({ kRootEndpointId, LocalizationConfiguration::Id }),
    mDeviceInfoProvider(aDeviceInfoProvider)
{
    DataModel::ActionReturnStatus status = SetActiveLocale(activeLocale);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        char tempBuf[kActiveLocaleMaxLength];
        MutableCharSpan validLocale(tempBuf);
        if (GetDefaultLocale(validLocale))
        {
            status = SetActiveLocale(validLocale);
            if (status != Protocols::InteractionModel::Status::Success)
            {
                ChipLogError(AppServer, "Failed to set active locale on endpoint %u", kRootEndpointId);
            }
        }
    }
}

CHIP_ERROR LocalizationConfigurationCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    AttributePersistence persistence(context.attributeStorage);

    // Load the active locale from persistence if it exists, otherwise use the default locale and store it in persistence.
    Storage::String<kActiveLocaleMaxLength> storedLocale;
    if (persistence.LoadString({ mPath.mEndpointId, LocalizationConfiguration::Id, Attributes::ActiveLocale::Id }, storedLocale))
    {
        SetActiveLocale(storedLocale.Content());
    }
    else
    {
        ReturnErrorOnFailure(persistence.StoreString(
            { mPath.mEndpointId, LocalizationConfiguration::Id, Attributes::ActiveLocale::Id }, mActiveLocale));
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

DataModel::ActionReturnStatus LocalizationConfigurationCluster::SetActiveLocale(CharSpan activeLocale)
{
    if (!IsSupportedLocale(activeLocale))
    {
        return Status::ConstraintError;
    }

    VerifyOrReturnError(mActiveLocale.SetContent(activeLocale), Status::ConstraintError);
    if (mContext != nullptr)
    {
        Storage::String<kActiveLocaleMaxLength> shortString;
        VerifyOrReturnError(shortString.SetContent(activeLocale), Status::ConstraintError);
        AttributePersistence persistence(mContext->attributeStorage);
        if (persistence.StoreString({ mPath.mEndpointId, LocalizationConfiguration::Id, Attributes::ActiveLocale::Id },
                                    shortString) != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to store active locale in persistence");
            return Status::ConstraintError;
        }
    }

    return Status::Success;
}

CharSpan LocalizationConfigurationCluster::GetActiveLocale()
{
    return mActiveLocale.Content();
}

CHIP_ERROR LocalizationConfigurationCluster::ReadSupportedLocales(AttributeValueEncoder & aEncoder)
{
    AutoReleaseIterator it(&mDeviceInfoProvider);
    VerifyOrReturnValue(it.IsValid(), aEncoder.EncodeEmptyList());

    return aEncoder.EncodeList([&it](const auto & encoder) -> CHIP_ERROR {
        CharSpan supportedLocale;

        while (it.Next(supportedLocale))
        {
            ReturnErrorOnFailure(encoder.Encode(supportedLocale));
        }

        return CHIP_NO_ERROR;
    });
}

bool LocalizationConfigurationCluster::IsSupportedLocale(CharSpan newLangTag) const
{
    AutoReleaseIterator it(&mDeviceInfoProvider);
    if (!it.IsValid())
    {
        return false;
    }

    CharSpan outLocale;
    while (it.Next(outLocale))
    {
        if (outLocale.data_equal(newLangTag))
        {
            return true;
        }
    }

    return false;
}

bool LocalizationConfigurationCluster::GetDefaultLocale(MutableCharSpan & outLocale)
{
    AutoReleaseIterator it(&mDeviceInfoProvider);
    if (!it.IsValid())
    {
        return false;
    }

    CharSpan tempLocale;
    bool found = it.Next(tempLocale);
    VerifyOrReturnValue(CopyCharSpanToMutableCharSpan(tempLocale, outLocale) == CHIP_NO_ERROR, false);

    return found;
}
} // namespace chip::app::Clusters
