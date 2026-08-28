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

#include <app/BufferedReadCallback.h>
#include <app/CASESessionManager.h>
#include <app/CommandSender.h>
#include <app/ReadClient.h>
#include <app/clusters/av-analysis-server/AvAnalysisCameraClient.h>
#include <clusters/CameraAvStreamManagement/Commands.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/TypeTraits.h>
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
        static constexpr uint16_t kDefaultMinFrameRate  = 15;
        static constexpr uint16_t kDefaultMaxFrameRate  = 30;
        static constexpr uint16_t kDefaultMinWidth      = 640;
        static constexpr uint16_t kDefaultMinHeight     = 480;
        static constexpr uint16_t kDefaultMaxWidth      = 1920;
        static constexpr uint16_t kDefaultMaxHeight     = 1080;
        static constexpr uint32_t kDefaultMinBitRateBps = 500000;
        static constexpr uint32_t kDefaultMaxBitRateBps = 2000000;

        // Discovered per request, invalid until the Descriptor read finds the AVSM endpoint
        EndpointId avsmEndpoint = kInvalidEndpointId;
        bool hasWatermark       = false;
        bool hasOSD             = false;

        uint16_t minFrameRate  = kDefaultMinFrameRate;
        uint16_t maxFrameRate  = kDefaultMaxFrameRate;
        uint16_t minWidth      = kDefaultMinWidth;
        uint16_t minHeight     = kDefaultMinHeight;
        uint16_t maxWidth      = kDefaultMaxWidth;
        uint16_t maxHeight     = kDefaultMaxHeight;
        uint32_t minBitRateBps = kDefaultMinBitRateBps;
        uint32_t maxBitRateBps = kDefaultMaxBitRateBps;
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
     * One camera request, from the first CASE attempt to the camera's answer.
     *
     * Reaching the camera is a multi-step asynchronous sequence, so the phase says which step is
     * outstanding and therefore which callback is legitimate; the rest is the state that lives
     * exactly as long as the request does. Exactly one request exists at a time.
     *
     *   kIdle -> kConnecting -> kDiscoveringEndpoint -> kDiscoveringCapabilities -> kInvoking -> kIdle
     *                                    (a deallocation skips the capabilities read)
     */
    class Request
    {
    public:
        enum class Phase : uint8_t
        {
            kIdle,                    // No request in flight
            kConnecting,              // Awaiting a CASE session with the camera
            kDiscoveringEndpoint,     // Wildcard Descriptor ServerList read locating the AVSM endpoint
            kDiscoveringCapabilities, // Targeted read of the AVSM capability attributes
            kInvoking,                // Command sent, awaiting the camera's answer
        };

        enum class CommandType : uint8_t
        {
            kVideoStreamAllocate,
            kVideoStreamDeallocate,
        };

        Phase GetPhase() const { return mPhase; }
        bool InPhase(Phase aPhase) const { return mPhase == aPhase; }
        bool InFlight() const { return mPhase != Phase::kIdle; }
        void Advance(Phase aPhase) { mPhase = aPhase; }

        CommandType GetCommandType() const { return mCommandType; }
        uint16_t VideoStreamId() const { return mVideoStreamId; }
        CameraProfile & Profile() { return mProfile; }
        const CameraProfile & Profile() const { return mProfile; }

        /**
         * Starts a request in kConnecting. The callback is delivered exactly once, by TakeCallback().
         */
        void Begin(CommandType aCommandType, uint16_t aVideoStreamId, AvAnalysisCameraClient::Callback & aCallback)
        {
            Reset();
            mPhase         = Phase::kConnecting;
            mCommandType   = aCommandType;
            mVideoStreamId = aVideoStreamId;
            mCallback      = &aCallback;
        }

        /**
         * Returns the callback owed the outcome, or nullptr when it was already delivered (or the
         * request was cancelled). Delivering is therefore exactly-once by construction.
         */
        AvAnalysisCameraClient::Callback * TakeCallback()
        {
            auto * callback = mCallback;
            mCallback       = nullptr;
            return callback;
        }

        // Session held from kConnecting until the request ends
        void HoldSession(const SessionHandle & aSession, Messaging::ExchangeManager & aExchangeMgr)
        {
            mSessionHolder.Grab(aSession);
            mExchangeMgr = &aExchangeMgr;
        }
        bool HasSession() const { return mSessionHolder && mExchangeMgr != nullptr; }

        Optional<SessionHandle> Session() const { return mSessionHolder.Get(); }
        Messaging::ExchangeManager & ExchangeManager() const { return *mExchangeMgr; }

        // The sender this request invoked with, if any. A CommandSender callback that does not
        // carry this pointer belongs to an interaction this request has already finished with.
        void SetInvokedSender(CommandSender * aSender) { mInvokedSender = aSender; }
        bool WasInvokedBy(const CommandSender * aSender) const { return mInvokedSender == aSender; }

        // Discovery outcome; the error only annotates the log, the flag fails an allocation early
        void SetDiscoveryError(CHIP_ERROR aError) { mDiscoveryError = aError; }
        CHIP_ERROR DiscoveryError() const { return mDiscoveryError; }
        void SetAnalysisUsageSupported(bool aSupported) { mAnalysisUsageSupported = aSupported; }
        bool AnalysisUsageSupported() const { return mAnalysisUsageSupported; }

        /**
         * Which capability attributes the camera has reported. All of them are Fixed and mandatory
         * for a camera that serves video (VideoSensorParams and RateDistortionTradeOffPoints under
         * the AVSM Video feature, SupportedStreamUsages unconditionally), so a missing one means a
         * lost report or a non-conformant camera, not an attribute the camera opted out of. An
         * allocation built on the built-in defaults instead would send constraints we invented, and
         * without the FeatureMap it would omit WatermarkEnabled/OSDEnabled where the camera
         * requires them, which the camera must reject.
         */
        enum class Capability : uint8_t
        {
            kFeatureMap                   = 0x01,
            kVideoSensorParams            = 0x02,
            kRateDistortionTradeOffPoints = 0x04,
            kSupportedStreamUsages        = 0x08,
        };
        static constexpr uint8_t kAllCapabilities = 0x0F;

        void MarkCapabilityReported(Capability aCapability)
        {
            mCapabilitiesSeen = static_cast<uint8_t>(mCapabilitiesSeen | to_underlying(aCapability));
        }
        bool AllCapabilitiesReported() const { return mCapabilitiesSeen == kAllCapabilities; }
        uint8_t MissingCapabilities() const { return static_cast<uint8_t>(kAllCapabilities & ~mCapabilitiesSeen); }

        void Reset() { *this = Request{}; }

    private:
        Phase mPhase                                 = Phase::kIdle;
        CommandType mCommandType                     = CommandType::kVideoStreamAllocate;
        uint16_t mVideoStreamId                      = 0;
        AvAnalysisCameraClient::Callback * mCallback = nullptr;
        CameraProfile mProfile;
        SessionHolder mSessionHolder;
        Messaging::ExchangeManager * mExchangeMgr = nullptr;
        CommandSender * mInvokedSender            = nullptr;
        CHIP_ERROR mDiscoveryError                = CHIP_NO_ERROR;
        bool mAnalysisUsageSupported              = true;
        uint8_t mCapabilitiesSeen                 = 0;
    };

    /**
     * The request in flight
     */
    Request & CurrentRequest() { return mRequest; }

    /**
     * Profile being assembled for the pending request.
     */
    const CameraProfile & CurrentProfile() const { return mRequest.Profile(); }

    /**
     * Reconciles the profile's bounds once every capability report is in, so the result does not
     * depend on the order they arrived in.
     *
     * Which side yields depends on where the bound came from. The resolution and frame-rate maximums
     * are the camera's own VideoSensorParams, a hard ceiling, so a minimum above them yields - asking
     * for more than the sensor can produce would be rejected. Nothing publishes a maximum bit rate,
     * so ours is only a preference and it rises to meet a camera-reported minimum instead.
     */
    static void NormalizeProfile(CameraProfile & aProfile);

    /**
     * Builds the VideoStreamAllocate request (StreamUsage Analysis) from a camera profile.
     */
    static CameraAvStreamManagement::Commands::VideoStreamAllocate::Type BuildAllocateRequest(const CameraProfile & aProfile);

    /**
     * Discovery report decoders, dispatched from OnAttributeData by discovery phase
     */
    void HandleServerListReport(const ConcreteDataAttributePath & aPath, TLV::TLVReader & aData);
    void HandleCapabilityReport(const ConcreteDataAttributePath & aPath, TLV::TLVReader & aData);

private:
    CHIP_ERROR StartRequest(Request::CommandType aCommandType, const ScopedNodeId & aCameraNode, uint16_t aVideoStreamId,
                            AvAnalysisCameraClient::Callback & aCallback);
    CHIP_ERROR SendPendingCommand();
    CHIP_ERROR AddPendingCommandData();
    CHIP_ERROR SendDiscoveryRead(AttributePathParams * aPaths, size_t aPathCount);
    void StartCapabilitiesRead();
    void ResetReadClient();
    void CompleteEndpointDiscovery();
    void CompleteCapabilityDiscovery();
    void FinishRequest(Protocols::InteractionModel::Status aStatus, uint16_t aStreamId);

    static void OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    CASESessionManager * mCASESessionManager = nullptr;

    Request mRequest;

    Platform::UniquePtr<BufferedReadCallback> mReadCallback;
    Platform::UniquePtr<ReadClient> mReadClient;
    Platform::UniquePtr<CommandSender> mCommandSender;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailureCallback;
};

} // namespace Clusters
} // namespace app
} // namespace chip
