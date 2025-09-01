/*
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
#include <app/clusters/operational-credentials-server/operational-credentials-cluster.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

#include <app/server/Server.h>
#include <app/EventLogging.h>
#include <app/server/Dnssd.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include <app/reporting/reporting.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

LazyRegisteredServerCluster<OperationalCredentialsCluster> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned emberEndpointIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServer.Create(endpointId);
        return gServer.Registration();
    }

    ServerClusterInterface & FindRegistration(unsigned emberEndpointIndex) override { return gServer.Cluster(); }
    void ReleaseRegistration(unsigned emberEndpointIndex) override { gServer.Destroy(); }
};

void FailSafeCleanup(const chip::DeviceLayer::ChipDeviceEvent * event)
{
    ChipLogError(Zcl, "OpCreds: Proceeding to FailSafeCleanup on fail-safe expiry!");

    bool nocAddedDuringFailsafe          = event->FailSafeTimerExpired.addNocCommandHasBeenInvoked;
    bool nocUpdatedDuringFailsafe        = event->FailSafeTimerExpired.updateNocCommandHasBeenInvoked;
    bool nocAddedOrUpdatedDuringFailsafe = nocAddedDuringFailsafe || nocUpdatedDuringFailsafe;

    FabricIndex fabricIndex = event->FailSafeTimerExpired.fabricIndex;

    // Report Fabrics table change if SetVIDVerificationStatement had been called.
    // There are 4 cases:
    //   1- Fail-safe started, AddNOC/UpdateNOC for fabric A, VVS set for fabric A after that: Need to mark dirty here.
    //   2- Fail-safe started, UpdateNOC/AddNOC for fabric A, VVS set for fabric B after that: No need to mark dirty.
    //   3- Fail-safe started, no UpdateNOC/AddNOC, VVS set for fabric X: No need to mark dirty.
    //   4- ail-safe started, VVS set for fabric A, UpdateNOC for fabric A: No need to mark dirty.
    //
    // Right now we will mark dirty no matter what, as the state-keeping logic for cases 2-4 above
    // was very complex and more likely to be less maintainable than possibly over-reporting Fabrics
    // attribute in this corner case of fail-safe expiry.
    if (event->FailSafeTimerExpired.setVidVerificationStatementHasBeenInvoked)
    {
        // Opcreds cluster is always on Endpoint 0.
        // Only `Fabrics` attribute is reported since `NOCs` is not reportable (`C` quality).```
        MatterReportingAttributeChangeCallback(0, OperationalCredentials::Id, OperationalCredentials::Attributes::Fabrics::Id);
    }

    // If an AddNOC or UpdateNOC command has been successfully invoked, terminate all CASE sessions associated with the Fabric
    // whose Fabric Index is recorded in the Fail-Safe context (see ArmFailSafe Command) by clearing any associated Secure
    // Session Context at the Server.
    if (nocAddedOrUpdatedDuringFailsafe)
    {
        SessionManager & sessionMgr = Server::GetInstance().GetSecureSessionManager();
        sessionMgr.ExpireAllSessionsForFabric(fabricIndex);
    }

    auto & fabricTable = Server::GetInstance().GetFabricTable();
    fabricTable.RevertPendingFabricData();

    // If an AddNOC command had been successfully invoked, achieve the equivalent effect of invoking the RemoveFabric command
    // against the Fabric Index stored in the Fail-Safe Context for the Fabric Index that was the subject of the AddNOC
    // command.
    if (nocAddedDuringFailsafe)
    {
        CHIP_ERROR err = Server::GetInstance().GetFabricTable().Delete(fabricIndex);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "OpCreds: failed to delete fabric at index %u: %" CHIP_ERROR_FORMAT, fabricIndex, err.Format());
        }
    }

    if (nocUpdatedDuringFailsafe)
    {
        // Operational identities/records available may have changed due to NodeID update. Need to refresh all records.
        // The case of fabric removal that reverts AddNOC is handled by the `DeleteFabricFromTable` flow above.
        app::DnssdServer::Instance().StartServer();
    }
}


void OnPlatformEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        ChipLogError(Zcl, "OpCreds: Got FailSafeTimerExpired");
        FailSafeCleanup(event);
    }
}

} // namespace

class OpCredsFabricTableDelegate : public chip::FabricTable::Delegate
{
public:
    // Gets called when a fabric is about to be deleted
    void FabricWillBeRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        // The Leave event SHOULD be emitted by a Node prior to permanently leaving the Fabric.
        for (auto endpoint : EnabledEndpointsWithServerCluster(BasicInformation::Id))
        {
            // If Basic cluster is implemented on this endpoint
            BasicInformation::Events::Leave::Type event;
            event.fabricIndex = fabricIndex;
            EventNumber eventNumber;

            if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
            {
                ChipLogError(Zcl, "OpCredsFabricTableDelegate: Failed to record Leave event");
            }
        }

        // Try to send the queued events as soon as possible for this fabric. If the just emitted leave event won't
        // be sent this time, it will likely not be delivered at all for the following reasons:
        // - removing the fabric expires all associated ReadHandlers, so all subscriptions to
        //   the leave event will be cancelled.
        // - removing the fabric removes all associated access control entries, so generating
        //   subsequent reports containing the leave event will fail the access control check.
        EventReporter & eventReporter = InteractionModelEngine::GetInstance()->GetReportingEngine();

        // public interface of event delivery is through an event reporter.
        eventReporter.ScheduleUrgentEventDeliverySync(MakeOptional(fabricIndex));
    }

    // Gets called when a fabric is deleted
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        ChipLogProgress(Zcl, "OpCreds: Fabric index 0x%x was removed", static_cast<unsigned>(fabricIndex));

        // We need to withdraw the advertisement for the now-removed fabric, so need
        // to restart advertising altogether.
        app::DnssdServer::Instance().StartServer();

        EventManagement::GetInstance().FabricRemoved(fabricIndex);

        NotifyFabricTableChanged();
    }

    // Gets called when a fabric is added/updated, but not necessarily committed to storage
    void OnFabricUpdated(const FabricTable & fabricTable, FabricIndex fabricIndex) override { NotifyFabricTableChanged(); }

    // Gets called when a fabric in FabricTable is persisted to storage
    void OnFabricCommitted(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        const FabricInfo * fabric = fabricTable.FindFabricWithIndex(fabricIndex);
        // Safety check, but should not happen by the code paths involved
        VerifyOrReturn(fabric != nullptr);

        ChipLogProgress(Zcl,
                        "OpCreds: Fabric index 0x%x was committed to storage. Compressed Fabric Id 0x" ChipLogFormatX64
                        ", FabricId " ChipLogFormatX64 ", NodeId " ChipLogFormatX64 ", VendorId 0x%04X",
                        static_cast<unsigned>(fabric->GetFabricIndex()), ChipLogValueX64(fabric->GetCompressedFabricId()),
                        ChipLogValueX64(fabric->GetFabricId()), ChipLogValueX64(fabric->GetNodeId()), fabric->GetVendorId());
    }

private:
    void NotifyFabricTableChanged()
    {
        // Opcreds cluster is always on Endpoint 0
        MatterReportingAttributeChangeCallback(0, OperationalCredentials::Id,
                                               OperationalCredentials::Attributes::CommissionedFabrics::Id);
        MatterReportingAttributeChangeCallback(0, OperationalCredentials::Id, OperationalCredentials::Attributes::Fabrics::Id);
    }
};
OpCredsFabricTableDelegate gFabricDelegate;

void emberAfOperationalCredentialsClusterServerInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = OperationalCredentials::Id,
            .fixedClusterServerEndpointCount = 1,
            .maxEndpointCount                = 1,
            .fetchFeatureMap                 = false,
            .fetchOptionalAttributes         = false,
        },
        integrationDelegate);

    Server::GetInstance().GetFabricTable().AddFabricDelegate(&gFabricDelegate);
    DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler);
}

void MatterOperationalCredentialsClusterServerShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    DeviceLayer::PlatformMgrImpl().RemoveEventHandler(OnPlatformEventHandler);
    Server::GetInstance().GetFabricTable().RemoveFabricDelegate(&gFabricDelegate);
    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = OperationalCredentials::Id,
            .fixedClusterServerEndpointCount = 1,
            .maxEndpointCount                = 1,
        },
        integrationDelegate);
}

void MatterOperationalCredentialsPluginServerInitCallback() {}