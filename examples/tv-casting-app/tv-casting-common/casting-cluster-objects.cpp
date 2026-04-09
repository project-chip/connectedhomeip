/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 * @file casting-cluster-objects.cpp
 *
 * Slim replacement for the generated cluster-objects.cpp that only includes
 * the cluster implementations (Attributes/Commands/Events/Structs .ipp files)
 * needed by the TV Casting App.  This covers:
 *
 *   - Casting-specific clusters (the clusters a casting client talks to on
 *     the remote TV/player device).
 *   - Infrastructure clusters required by the Matter controller and
 *     commissioning flow.
 *
 * All other clusters are excluded to reduce binary size.
 */

#include <app-common/zap-generated/cluster-objects.h>

// ---------------------------------------------------------------------------
// Infrastructure clusters (commissioning, diagnostics, core)
// ---------------------------------------------------------------------------
#include <clusters/AccessControl/Attributes.ipp>
#include <clusters/AccessControl/Commands.ipp>
#include <clusters/AccessControl/Events.ipp>
#include <clusters/AccessControl/Structs.ipp>

#include <clusters/AdministratorCommissioning/Attributes.ipp>
#include <clusters/AdministratorCommissioning/Commands.ipp>
#include <clusters/AdministratorCommissioning/Events.ipp>
#include <clusters/AdministratorCommissioning/Structs.ipp>

#include <clusters/BasicInformation/Attributes.ipp>
#include <clusters/BasicInformation/Commands.ipp>
#include <clusters/BasicInformation/Events.ipp>
#include <clusters/BasicInformation/Structs.ipp>

#include <clusters/Binding/Attributes.ipp>
#include <clusters/Binding/Commands.ipp>
#include <clusters/Binding/Events.ipp>
#include <clusters/Binding/Structs.ipp>

#include <clusters/Descriptor/Attributes.ipp>
#include <clusters/Descriptor/Commands.ipp>
#include <clusters/Descriptor/Events.ipp>
#include <clusters/Descriptor/Structs.ipp>

#include <clusters/EthernetNetworkDiagnostics/Attributes.ipp>
#include <clusters/EthernetNetworkDiagnostics/Commands.ipp>
#include <clusters/EthernetNetworkDiagnostics/Events.ipp>
#include <clusters/EthernetNetworkDiagnostics/Structs.ipp>

#include <clusters/FixedLabel/Attributes.ipp>
#include <clusters/FixedLabel/Commands.ipp>
#include <clusters/FixedLabel/Events.ipp>
#include <clusters/FixedLabel/Structs.ipp>

#include <clusters/GeneralCommissioning/Attributes.ipp>
#include <clusters/GeneralCommissioning/Commands.ipp>
#include <clusters/GeneralCommissioning/Events.ipp>
#include <clusters/GeneralCommissioning/Structs.ipp>

#include <clusters/GeneralDiagnostics/Attributes.ipp>
#include <clusters/GeneralDiagnostics/Commands.ipp>
#include <clusters/GeneralDiagnostics/Events.ipp>
#include <clusters/GeneralDiagnostics/Structs.ipp>

#include <clusters/GroupKeyManagement/Attributes.ipp>
#include <clusters/GroupKeyManagement/Commands.ipp>
#include <clusters/GroupKeyManagement/Events.ipp>
#include <clusters/GroupKeyManagement/Structs.ipp>

#include <clusters/Groupcast/Attributes.ipp>
#include <clusters/Groupcast/Commands.ipp>
#include <clusters/Groupcast/Events.ipp>
#include <clusters/Groupcast/Structs.ipp>

#include <clusters/Groups/Attributes.ipp>
#include <clusters/Groups/Commands.ipp>
#include <clusters/Groups/Events.ipp>
#include <clusters/Groups/Structs.ipp>

#include <clusters/IcdManagement/Attributes.ipp>
#include <clusters/IcdManagement/Commands.ipp>
#include <clusters/IcdManagement/Events.ipp>
#include <clusters/IcdManagement/Structs.ipp>

#include <clusters/Identify/Attributes.ipp>
#include <clusters/Identify/Commands.ipp>
#include <clusters/Identify/Events.ipp>
#include <clusters/Identify/Structs.ipp>

#include <clusters/LocalizationConfiguration/Attributes.ipp>
#include <clusters/LocalizationConfiguration/Commands.ipp>
#include <clusters/LocalizationConfiguration/Events.ipp>
#include <clusters/LocalizationConfiguration/Structs.ipp>

#include <clusters/NetworkCommissioning/Attributes.ipp>
#include <clusters/NetworkCommissioning/Commands.ipp>
#include <clusters/NetworkCommissioning/Events.ipp>
#include <clusters/NetworkCommissioning/Structs.ipp>

#include <clusters/OperationalCredentials/Attributes.ipp>
#include <clusters/OperationalCredentials/Commands.ipp>
#include <clusters/OperationalCredentials/Events.ipp>
#include <clusters/OperationalCredentials/Structs.ipp>

#include <clusters/SoftwareDiagnostics/Attributes.ipp>
#include <clusters/SoftwareDiagnostics/Commands.ipp>
#include <clusters/SoftwareDiagnostics/Events.ipp>
#include <clusters/SoftwareDiagnostics/Structs.ipp>

#include <clusters/TimeFormatLocalization/Attributes.ipp>
#include <clusters/TimeFormatLocalization/Commands.ipp>
#include <clusters/TimeFormatLocalization/Events.ipp>
#include <clusters/TimeFormatLocalization/Structs.ipp>

#include <clusters/TimeSynchronization/Attributes.ipp>
#include <clusters/TimeSynchronization/Commands.ipp>
#include <clusters/TimeSynchronization/Events.ipp>
#include <clusters/TimeSynchronization/Structs.ipp>

#include <clusters/UnitLocalization/Attributes.ipp>
#include <clusters/UnitLocalization/Commands.ipp>
#include <clusters/UnitLocalization/Events.ipp>
#include <clusters/UnitLocalization/Structs.ipp>

#include <clusters/UserLabel/Attributes.ipp>
#include <clusters/UserLabel/Commands.ipp>
#include <clusters/UserLabel/Events.ipp>
#include <clusters/UserLabel/Structs.ipp>

#include <clusters/WiFiNetworkDiagnostics/Attributes.ipp>
#include <clusters/WiFiNetworkDiagnostics/Commands.ipp>
#include <clusters/WiFiNetworkDiagnostics/Events.ipp>
#include <clusters/WiFiNetworkDiagnostics/Structs.ipp>

// ---------------------------------------------------------------------------
// Casting-specific clusters
// ---------------------------------------------------------------------------

#include <clusters/AccountLogin/Attributes.ipp>
#include <clusters/AccountLogin/Commands.ipp>
#include <clusters/AccountLogin/Events.ipp>
#include <clusters/AccountLogin/Structs.ipp>

#include <clusters/ApplicationBasic/Attributes.ipp>
#include <clusters/ApplicationBasic/Commands.ipp>
#include <clusters/ApplicationBasic/Events.ipp>
#include <clusters/ApplicationBasic/Structs.ipp>

#include <clusters/ApplicationLauncher/Attributes.ipp>
#include <clusters/ApplicationLauncher/Commands.ipp>
#include <clusters/ApplicationLauncher/Events.ipp>
#include <clusters/ApplicationLauncher/Structs.ipp>

#include <clusters/AudioOutput/Attributes.ipp>
#include <clusters/AudioOutput/Commands.ipp>
#include <clusters/AudioOutput/Events.ipp>
#include <clusters/AudioOutput/Structs.ipp>

#include <clusters/Channel/Attributes.ipp>
#include <clusters/Channel/Commands.ipp>
#include <clusters/Channel/Events.ipp>
#include <clusters/Channel/Structs.ipp>

#include <clusters/ContentAppObserver/Attributes.ipp>
#include <clusters/ContentAppObserver/Commands.ipp>
#include <clusters/ContentAppObserver/Events.ipp>
#include <clusters/ContentAppObserver/Structs.ipp>

#include <clusters/ContentControl/Attributes.ipp>
#include <clusters/ContentControl/Commands.ipp>
#include <clusters/ContentControl/Events.ipp>
#include <clusters/ContentControl/Structs.ipp>

#include <clusters/ContentLauncher/Attributes.ipp>
#include <clusters/ContentLauncher/Commands.ipp>
#include <clusters/ContentLauncher/Events.ipp>
#include <clusters/ContentLauncher/Structs.ipp>

#include <clusters/KeypadInput/Attributes.ipp>
#include <clusters/KeypadInput/Commands.ipp>
#include <clusters/KeypadInput/Events.ipp>
#include <clusters/KeypadInput/Structs.ipp>

#include <clusters/LevelControl/Attributes.ipp>
#include <clusters/LevelControl/Commands.ipp>
#include <clusters/LevelControl/Events.ipp>
#include <clusters/LevelControl/Structs.ipp>

#include <clusters/LowPower/Attributes.ipp>
#include <clusters/LowPower/Commands.ipp>
#include <clusters/LowPower/Events.ipp>
#include <clusters/LowPower/Structs.ipp>

#include <clusters/MediaInput/Attributes.ipp>
#include <clusters/MediaInput/Commands.ipp>
#include <clusters/MediaInput/Events.ipp>
#include <clusters/MediaInput/Structs.ipp>

#include <clusters/MediaPlayback/Attributes.ipp>
#include <clusters/MediaPlayback/Commands.ipp>
#include <clusters/MediaPlayback/Events.ipp>
#include <clusters/MediaPlayback/Structs.ipp>

#include <clusters/Messages/Attributes.ipp>
#include <clusters/Messages/Commands.ipp>
#include <clusters/Messages/Events.ipp>
#include <clusters/Messages/Structs.ipp>

#include <clusters/OnOff/Attributes.ipp>
#include <clusters/OnOff/Commands.ipp>
#include <clusters/OnOff/Events.ipp>
#include <clusters/OnOff/Structs.ipp>

#include <clusters/TargetNavigator/Attributes.ipp>
#include <clusters/TargetNavigator/Commands.ipp>
#include <clusters/TargetNavigator/Events.ipp>
#include <clusters/TargetNavigator/Structs.ipp>

#include <clusters/WakeOnLan/Attributes.ipp>
#include <clusters/WakeOnLan/Commands.ipp>
#include <clusters/WakeOnLan/Events.ipp>
#include <clusters/WakeOnLan/Structs.ipp>

// ---------------------------------------------------------------------------
// shared cluster utilities (always required)
// ---------------------------------------------------------------------------

#include <clusters/shared/Structs.ipp>

// ---------------------------------------------------------------------------
// Command metadata functions required by CodegenDataModelProvider.
//
// The full cluster-objects.cpp defines these for all ~200+ clusters.  Since
// we only compile the casting-relevant subset, we provide slim versions that
// cover only the clusters included above.  Any cluster not listed here
// falls through to the default `return false`.
// ---------------------------------------------------------------------------

namespace chip {
namespace app {

bool CommandNeedsTimedInvoke(ClusterId aCluster, CommandId aCommand)
{
    switch (aCluster)
    {
    case Clusters::AdministratorCommissioning::Id: {
        switch (aCommand)
        {
        case Clusters::AdministratorCommissioning::Commands::OpenCommissioningWindow::Id:
        case Clusters::AdministratorCommissioning::Commands::OpenBasicCommissioningWindow::Id:
        case Clusters::AdministratorCommissioning::Commands::RevokeCommissioning::Id:
            return true;
        default:
            return false;
        }
    }
    case Clusters::AccountLogin::Id: {
        switch (aCommand)
        {
        case Clusters::AccountLogin::Commands::GetSetupPIN::Id:
        case Clusters::AccountLogin::Commands::Login::Id:
        case Clusters::AccountLogin::Commands::Logout::Id:
            return true;
        default:
            return false;
        }
    }
    case Clusters::ContentControl::Id: {
        switch (aCommand)
        {
        case Clusters::ContentControl::Commands::UpdatePIN::Id:
        case Clusters::ContentControl::Commands::ResetPIN::Id:
        case Clusters::ContentControl::Commands::Enable::Id:
        case Clusters::ContentControl::Commands::Disable::Id:
            return true;
        default:
            return false;
        }
    }
    default:
        break;
    }
    return false;
}

bool CommandIsFabricScoped(ClusterId aCluster, CommandId aCommand)
{
    switch (aCluster)
    {
    case Clusters::Groups::Id: {
        switch (aCommand)
        {
        case Clusters::Groups::Commands::AddGroup::Id:
        case Clusters::Groups::Commands::ViewGroup::Id:
        case Clusters::Groups::Commands::GetGroupMembership::Id:
        case Clusters::Groups::Commands::RemoveGroup::Id:
        case Clusters::Groups::Commands::RemoveAllGroups::Id:
        case Clusters::Groups::Commands::AddGroupIfIdentifying::Id:
            return true;
        default:
            return false;
        }
    }
    case Clusters::GroupKeyManagement::Id: {
        switch (aCommand)
        {
        case Clusters::GroupKeyManagement::Commands::KeySetWrite::Id:
        case Clusters::GroupKeyManagement::Commands::KeySetRead::Id:
        case Clusters::GroupKeyManagement::Commands::KeySetRemove::Id:
        case Clusters::GroupKeyManagement::Commands::KeySetReadAllIndices::Id:
            return true;
        default:
            return false;
        }
    }
    case Clusters::OperationalCredentials::Id: {
        switch (aCommand)
        {
        case Clusters::OperationalCredentials::Commands::UpdateNOC::Id:
        case Clusters::OperationalCredentials::Commands::UpdateFabricLabel::Id:
        case Clusters::OperationalCredentials::Commands::SetVIDVerificationStatement::Id:
            return true;
        default:
            return false;
        }
    }
    case Clusters::TimeSynchronization::Id: {
        switch (aCommand)
        {
        case Clusters::TimeSynchronization::Commands::SetTrustedTimeSource::Id:
            return true;
        default:
            return false;
        }
    }
    case Clusters::IcdManagement::Id: {
        switch (aCommand)
        {
        case Clusters::IcdManagement::Commands::RegisterClient::Id:
        case Clusters::IcdManagement::Commands::UnregisterClient::Id:
            return true;
        default:
            return false;
        }
    }
    default:
        break;
    }
    return false;
}

bool CommandHasLargePayload(ClusterId aCluster, CommandId aCommand)
{
    // None of the casting-relevant clusters have commands flagged as large payload.
    (void) aCluster;
    (void) aCommand;
    return false;
}

} // namespace app
} // namespace chip
