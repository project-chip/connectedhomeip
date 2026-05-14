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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeBase::Attributes;

using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters::ModeBase {

// A set of pointers to all initialised ModeBase instances. It provides a way to access all ModeBase derived clusters.
// todo change once there is a clear public interface for the OnOff cluster data dependencies (#27508)
static IntrusiveList<Instance> gModeBaseInstances;

IntrusiveList<Instance> & GetModeBaseInstanceList()
{
    return gModeBaseInstances;
}

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature) :
    mDelegate(aDelegate), mClusterPath(aEndpointId, aClusterId), mFeature(aFeature)
{}

Instance::~Instance()
{
    // Call Deinit() to clean up and unregister the cluster if it is constructed.
    // Return value is safely ignored because we are in the destructor and this is just
    // a cleanup and we do not want to Log the error because Deinit() is part of the API
    // and could be called directly by the app.
    // If the cluster is not constructed, Deinit() will return CHIP_ERROR_INCORRECT_STATE.
    RETURN_SAFELY_IGNORED Deinit();
}

CHIP_ERROR Instance::Init()
{
    const EmberAfCluster * cluster = emberAfFindServerCluster(mClusterPath.mEndpointId, mClusterPath.mClusterId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);

    if (emberAfContainsAttribute(mClusterPath.mEndpointId, mClusterPath.mClusterId, StartUpMode::Id))
    {
        mOptionalAttributeSet.Set<StartUpMode::Id>();
    }

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
                                    .diagnosticDataProvider           = diagnosticDataProvider };
    mCluster.Create(mClusterPath.mEndpointId, mClusterPath.mClusterId, config);
    RegisterThisInstance();
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

CHIP_ERROR Instance::Deinit()
{
    if (mDelegate)
    {
        mDelegate->SetInstance(nullptr);
    }
    UnregisterThisInstance();
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    return CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster()));
}

Protocols::InteractionModel::Status Instance::UpdateCurrentMode(uint8_t aNewMode)
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().UpdateCurrentMode(aNewMode);
}

Protocols::InteractionModel::Status Instance::UpdateStartUpMode(DataModel::Nullable<uint8_t> aNewStartUpMode)
{
    VerifyOrDie(mCluster.IsConstructed());
    return mCluster.Cluster().UpdateStartUpMode(aNewStartUpMode);
}

Protocols::InteractionModel::Status Instance::UpdateOnMode(DataModel::Nullable<uint8_t> aNewOnMode)
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
    RETURN_SAFELY_IGNORED Deinit();
}

} // namespace chip::app::Clusters::ModeBase

// void MatterModeBaseClusterInitCallback(EndpointId) {}
// void MatterModeBaseClusterShutdownCallback(EndpointId, MatterClusterShutdownType) {}
