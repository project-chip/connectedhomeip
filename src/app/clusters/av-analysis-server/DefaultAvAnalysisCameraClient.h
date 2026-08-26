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

#pragma once

#include <memory>

#include <app/BufferedReadCallback.h>
#include <app/CASESessionManager.h>
#include <app/CommandSender.h>
#include <app/ReadClient.h>
#include <app/clusters/av-analysis-server/AvAnalysisCameraClient.h>
#include <clusters/CameraAvStreamManagement/Commands.h>
#include <lib/core/DataModelTypes.h>
#include <transport/SessionHolder.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * Production AvAnalysisCameraClient: reaches the camera over a CASE session obtained from the
 * CASESessionManager and sends the CameraAVStreamManagement commands with a CommandSender.
 *
 * One interaction is in flight at a time; a request made while another is pending fails with
 * CHIP_ERROR_BUSY.
 *
 * The application creates one instance at startup and hands it to the cluster:
 *   client.Init(Server::GetInstance().GetCASESessionManager());
 *   cluster.SetCameraClient(&client);
 */
class DefaultAvAnalysisCameraClient : public AvAnalysisCameraClient, public CommandSender::Callback, public ReadClient::Callback
{
public:
    /**
     * What the client knows about the camera when building requests toward it.
     */
    struct CameraProfile
    {
        EndpointId avsmEndpoint = kInvalidEndpointId;
        bool hasWatermark       = false;
        bool hasOSD             = false;
        uint16_t minFrameRate   = 0;
        uint16_t maxFrameRate   = 0;
        uint16_t minWidth       = 0;
        uint16_t minHeight      = 0;
        uint16_t maxWidth       = 0;
        uint16_t maxHeight      = 0;
        uint32_t minBitRateBps  = 0;
        uint32_t maxBitRateBps  = 0;
    };

    DefaultAvAnalysisCameraClient() :
        mOnConnectedCallback(OnDeviceConnected, this), mOnConnectionFailureCallback(OnDeviceConnectionFailure, this)
    {}

    /**
     * @param aCASESessionManager Used to reach the camera node; must outlive this instance.
     *
     * Everything camera-specific — the CameraAVStreamManagement endpoint, its features, and its
     * stream constraints — is discovered from the camera itself per request.
     */
    CHIP_ERROR Init(CASESessionManager * aCASESessionManager);

    // AvAnalysisCameraClient
    CHIP_ERROR RequestVideoStreamAllocation(const ScopedNodeId & aCameraNode,
                                            AvAnalysisCameraClient::Callback & aCallback) override;
    CHIP_ERROR RequestVideoStreamDeallocation(const ScopedNodeId & aCameraNode, uint16_t aVideoStreamId,
                                              AvAnalysisCameraClient::Callback & aCallback) override;
    void Cancel() override;

    // CommandSender::Callback
    void OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath, const StatusIB & aStatusIB,
                    TLV::TLVReader * apData) override;
    void OnError(const CommandSender * apCommandSender, CHIP_ERROR aError) override;
    void OnDone(CommandSender * apCommandSender) override;

    // ReadClient::Callback (profile discovery reads)
    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;
    void OnError(CHIP_ERROR aError) override;
    void OnDone(ReadClient * apReadClient) override;

protected:
    /**
     * Starts CASE session establishment toward the camera; the connected/connection-failure callbacks
     * continue the pending request. The default body is the expected behavior; virtual only so unit
     * tests can intercept the network boundary.
     */
    virtual void EstablishSession(const ScopedNodeId & aCameraNode)
    {
        mCASESessionManager->FindOrEstablishSession(aCameraNode, &mOnConnectedCallback, &mOnConnectionFailureCallback);
    }

    /**
     * Begins profile discovery for the pending request, once the camera session is up.
     */
    virtual void StartProfileDiscovery();

    /**
     * Completion of StartProfileDiscovery: on success the pending command is sent on the held session,
     * otherwise the request fails.
     */
    void OnProfileDiscoveryComplete(CHIP_ERROR aError);

    /**
     * Fills a profile with the built-in default stream constraints; discovery overrides them with
     * camera-reported values.
     */
    static void FillProfileDefaults(CameraProfile & outProfile);

    /**
     * Profile being assembled for the pending request.
     */
    const CameraProfile & CurrentProfile() const { return mProfile; }

    /**
     * Builds the VideoStreamAllocate request (StreamUsage Analysis) from a camera profile.
     */
    static CameraAvStreamManagement::Commands::VideoStreamAllocate::Type BuildAllocateRequest(const CameraProfile & aProfile);

    /**
     * Discovery report decoders, dispatched from OnAttributeData by discovery phase. Protected so
     * unit tests can feed crafted reports without a live read.
     */
    void HandleServerListReport(const ConcreteDataAttributePath & aPath, TLV::TLVReader & aData);
    void HandleCapabilityReport(const ConcreteDataAttributePath & aPath, TLV::TLVReader & aData);

    /**
     * False only when SupportedStreamUsages was read successfully and lacks Analysis.
     */
    bool AnalysisUsageSupported() const { return mAnalysisUsageSupported; }

private:
    enum class PendingRequest : uint8_t
    {
        kNone,
        kAllocate,
        kDeallocate,
    };

    enum class DiscoveryPhase : uint8_t
    {
        kIdle,
        kFindEndpoint,     // Wildcard Descriptor ServerList read locating the AVSM endpoint
        kReadCapabilities, // Targeted read of the AVSM capability attributes on that endpoint
    };

    CHIP_ERROR StartRequest(PendingRequest aRequest, const ScopedNodeId & aCameraNode, uint16_t aVideoStreamId,
                            AvAnalysisCameraClient::Callback & aCallback);
    CHIP_ERROR SendPendingCommand(Messaging::ExchangeManager & aExchangeMgr, const SessionHandle & aSessionHandle);
    CHIP_ERROR SendDiscoveryRead(AttributePathParams * aPaths, size_t aPathCount);
    void StartCapabilitiesRead();
    void FinishRequest(Protocols::InteractionModel::Status aStatus, uint16_t aStreamId);

    static void OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    CASESessionManager * mCASESessionManager = nullptr;

    PendingRequest mPendingRequest                      = PendingRequest::kNone;
    uint16_t mPendingVideoStreamId                      = 0;
    AvAnalysisCameraClient::Callback * mPendingCallback = nullptr;
    bool mResponseDelivered                             = false;
    CameraProfile mProfile;
    // Session/exchange manager held across the discovery phase of the pending request
    SessionHolder mSessionHolder;
    Messaging::ExchangeManager * mExchangeMgr = nullptr;
    // Discovery read state
    BufferedReadCallback mBufferedReadCallback{ *this };
    Platform::UniquePtr<ReadClient> mReadClient;
    CHIP_ERROR mDiscoveryError     = CHIP_NO_ERROR;
    DiscoveryPhase mDiscoveryPhase = DiscoveryPhase::kIdle;
    // False only when SupportedStreamUsages was read successfully and lacks Analysis
    bool mAnalysisUsageSupported = true;
    std::unique_ptr<CommandSender> mCommandSender;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailureCallback;
};

} // namespace Clusters
} // namespace app
} // namespace chip
