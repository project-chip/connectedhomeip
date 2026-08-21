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

#include <app/CASESessionManager.h>
#include <app/CommandSender.h>
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
 *   client.Init(Server::GetInstance().GetCASESessionManager(), kCameraAvStreamManagementEndpoint);
 *   cluster.SetCameraClient(&client);
 */
class DefaultAvAnalysisCameraClient : public AvAnalysisCameraClient, public CommandSender::Callback
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
     * @param aCameraEndpoint     Endpoint on the camera hosting the CameraAVStreamManagement cluster.
     *                            The EstablishAnalysisStream command only carries the camera's NodeID
     *                            (spec 11.9.8), so the endpoint is configuration.
     */
    CHIP_ERROR Init(CASESessionManager * aCASESessionManager, EndpointId aCameraEndpoint);

    /**
     * Declares whether the camera's CameraAVStreamManagement cluster has the Watermark and
     * OnScreenDisplay features. VideoStreamAllocate requires the WatermarkEnabled/OSDEnabled fields
     * to be present exactly when the respective feature is supported (feature-conditional
     * conformance), so the client must know them. Defaults to absent.
     * TODO: discover from the camera's FeatureMap instead of configuration.
     */
    void SetCameraVideoTraits(bool aHasWatermark, bool aHasOSD)
    {
        mCameraHasWatermark = aHasWatermark;
        mCameraHasOSD       = aHasOSD;
    }

    // AvAnalysisCameraClient
    CHIP_ERROR RequestVideoStreamAllocation(const ScopedNodeId & aCameraNode, Callback & aCallback) override;
    CHIP_ERROR RequestVideoStreamDeallocation(const ScopedNodeId & aCameraNode, uint16_t aAnalysisStreamId,
                                              Callback & aCallback) override;
    void Cancel() override;

    // CommandSender::Callback
    void OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath, const StatusIB & aStatusIB,
                    TLV::TLVReader * apData) override;
    void OnError(const CommandSender * apCommandSender, CHIP_ERROR aError) override;
    void OnDone(CommandSender * apCommandSender) override;

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
     * Fills a profile from this client's configuration
     */
    void FillProfileFromConfiguration(CameraProfile & outProfile) const;

    /**
     * Builds the VideoStreamAllocate request (StreamUsage Analysis) from a camera profile.
     */
    static CameraAvStreamManagement::Commands::VideoStreamAllocate::Type BuildAllocateRequest(const CameraProfile & aProfile);

private:
    enum class PendingRequest : uint8_t
    {
        kNone,
        kAllocate,
        kDeallocate,
    };

    CHIP_ERROR StartRequest(PendingRequest aRequest, const ScopedNodeId & aCameraNode, uint16_t aAnalysisStreamId,
                            Callback & aCallback);
    CHIP_ERROR SendPendingCommand(Messaging::ExchangeManager & aExchangeMgr, const SessionHandle & aSessionHandle);
    void FinishRequest(Protocols::InteractionModel::Status aStatus, uint16_t aStreamId);

    static void OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    CASESessionManager * mCASESessionManager = nullptr;
    EndpointId mCameraEndpoint               = kInvalidEndpointId;
    bool mCameraHasWatermark                 = false;
    bool mCameraHasOSD                       = false;

    PendingRequest mPendingRequest = PendingRequest::kNone;
    uint16_t mPendingStreamId      = 0;
    Callback * mPendingCallback    = nullptr;
    bool mResponseDelivered        = false;
    CameraProfile mProfile;
    // Session/exchange manager held across the discovery phase of the pending request
    SessionHolder mSessionHolder;
    Messaging::ExchangeManager * mExchangeMgr = nullptr;
    std::unique_ptr<CommandSender> mCommandSender;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailureCallback;
};

} // namespace Clusters
} // namespace app
} // namespace chip
