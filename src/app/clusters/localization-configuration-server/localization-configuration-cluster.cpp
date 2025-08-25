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
#include <protocols/interaction_model/StatusCode.h>

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
    chip::app::Storage::String<35> storedLocale;
    (void) persistence.LoadString({ kRootEndpointId, LocalizationConfiguration::Id, Attributes::ActiveLocale::Id }, storedLocale);

    // Convert String to CharSpan and set it in the logic
    mLogic.SetActiveLocale(storedLocale.Content());

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
        return aEncoder.Encode(mLogic.GetActiveLocale());
    case SupportedLocales::Id:
        return mLogic.ReadSupportedLocales(aEncoder);
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
        CHIP_ERROR error = mLogic.SetActiveLocale(label);
        if (error != CHIP_NO_ERROR)
        {
            return Status::ConstraintError;
        }

        // Store the string in persistence
        Storage::String<35> shortString;
        shortString.SetContent(label);
        return persistence.StoreString(request.path, shortString);
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

} // namespace chip::app::Clusters
