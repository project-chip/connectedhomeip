/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/mode-base-server/CodegenIntegration.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <platform/DiagnosticDataProvider.h>

#ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER
#include <app/clusters/on-off-server/on-off-server.h>
using namespace chip::app::Clusters::OnOff::Attributes;
#endif // MATTER_DM_PLUGIN_ON_OFF_SERVER

#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeBase::Attributes;

using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters::ModeBase {

namespace {

// A set of pointers to all initialised ModeBase instances. It provides a way to access all ModeBase derived clusters.
// TODO: change once there is a clear public interface for the OnOff cluster data dependencies (#27508)
IntrusiveList<Instance> gModeBaseInstances;

} // namespace

IntrusiveList<Instance> & GetModeBaseInstanceList()
{
    return gModeBaseInstances;
}

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature) :
    mDelegate(aDelegate), mClusterPath(aEndpointId, aClusterId), mFeature(aFeature)
{}

Instance::~Instance()
{
    // Call Shutdown() to clean up and unregister the cluster if it is constructed.
    Shutdown();
}

CHIP_ERROR Instance::Init()
{
    const EmberAfCluster * cluster = emberAfFindServerCluster(mClusterPath.mEndpointId, mClusterPath.mClusterId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);

    std::optional<uint32_t> clusterRevision;
    for (const auto & entry : kAliasedClusters)
    {
        if (entry.id == mClusterPath.mClusterId)
        {
            clusterRevision = entry.revision;
            break;
        }
    }
    VerifyOrReturnError(clusterRevision.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

    // Although StartUpMode attribute is optional, spec says that none of the aliased clusters supports it.
    VerifyOrReturnError(!emberAfContainsAttribute(mClusterPath.mEndpointId, mClusterPath.mClusterId, StartUpMode::Id),
                        CHIP_ERROR_INCORRECT_STATE);

    bool onOffValueForStartUp = false;

#ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER
    // OnMode with Power Up
    // If the On/Off feature is supported and the On/Off cluster attribute StartUpOnOff is present, with a
    // value of On (turn on at power up), then the CurrentMode attribute SHALL be set to the OnMode attribute
    // value when the server is supplied with power, except if the OnMode attribute is null.
    if (emberAfContainsServer(mClusterPath.mEndpointId, OnOff::Id) &&
        emberAfContainsAttribute(mClusterPath.mEndpointId, OnOff::Id, OnOff::Attributes::StartUpOnOff::Id) &&
        emberAfContainsAttribute(mClusterPath.mEndpointId, mClusterPath.mClusterId, ModeBase::Attributes::OnMode::Id) &&
        HasFeature(ModeBase::Feature::kOnOff))
    {
        bool startUpOnOffValue = false;
        if (!emberAfIsKnownVolatileAttribute(mClusterPath.mEndpointId, OnOff::Id, OnOff::Attributes::StartUpOnOff::Id) &&
            OnOffServer::Instance().getOnOffValueForStartUp(mClusterPath.mEndpointId, startUpOnOffValue) == Status::Success)
        {
            onOffValueForStartUp = startUpOnOffValue;
        }
    }
#else
    // Although ModeBase::Feature::kOnOff is optional, spec says that none of the aliased clusters supports it.
    VerifyOrReturnError(!HasFeature(ModeBase::Feature::kOnOff), CHIP_ERROR_INCORRECT_STATE);
#endif // MATTER_DM_PLUGIN_ON_OFF_SERVER

    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mDelegate->SetInstance(this);
    ReturnErrorOnFailure(mDelegate->Init());

    SafeAttributePersistenceProvider * safeAttributePersistenceProvider = GetSafeAttributePersistenceProvider();
    VerifyOrReturnError(safeAttributePersistenceProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider = DeviceLayer::GetDiagnosticDataProvider();

    ModeBaseCluster::Config config{ .feature                          = mFeature,
                                    .optionalAttributeSet             = mOptionalAttributeSet,
                                    .appDelegate                      = *mDelegate,
                                    .onOffValueForStartUp             = onOffValueForStartUp,
                                    .safeAttributePersistenceProvider = *safeAttributePersistenceProvider,
                                    .diagnosticDataProvider           = diagnosticDataProvider,
                                    .clusterRevision                  = clusterRevision.value() };
    mCluster.Create(mClusterPath.mEndpointId, mClusterPath.mClusterId, config);
    RegisterThisInstance();
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

Status Instance::UpdateCurrentMode(uint8_t aNewMode)
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().UpdateCurrentMode(aNewMode);
}

Status Instance::UpdateStartUpMode(DataModel::Nullable<uint8_t> aNewStartUpMode)
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().UpdateStartUpMode(aNewStartUpMode);
}

Status Instance::UpdateOnMode(DataModel::Nullable<uint8_t> aNewOnMode)
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().UpdateOnMode(aNewOnMode);
}

uint8_t Instance::GetCurrentMode() const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetCurrentMode();
}

DataModel::Nullable<uint8_t> Instance::GetStartUpMode() const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetStartUpMode();
}

DataModel::Nullable<uint8_t> Instance::GetOnMode() const
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetOnMode();
}

void Instance::ReportSupportedModesChange()
{
    VerifyOrDie(mCluster.IsConstructed());
    mCluster.Cluster().ReportSupportedModesChange();
}

bool Instance::IsSupportedMode(uint8_t mode)
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().IsSupportedMode(mode);
}

CHIP_ERROR Instance::GetModeValueByModeTag(uint16_t modeTag, uint8_t & value)
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().GetModeValueByModeTag(modeTag, value);
}

void Instance::RegisterThisInstance()
{
    if (!GetModeBaseInstanceList().Contains(this))
    {
        GetModeBaseInstanceList().PushBack(this);
    }
}

void Instance::UnregisterThisInstance()
{
    GetModeBaseInstanceList().Remove(this);
}

void Instance::Shutdown()
{
    if (mDelegate != nullptr)
    {
        mDelegate->SetInstance(nullptr);
    }
    UnregisterThisInstance();
    if (mCluster.IsConstructed())
    {
        LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster())));
        mCluster.Destroy();
    }
}

} // namespace chip::app::Clusters::ModeBase
