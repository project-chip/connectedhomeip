/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "WiFiPAFConfig.h"
#include "WiFiPAFEndPoint.h"
#include "WiFiPAFLayerDelegate.h"
#include "WiFiPAFRole.h"
#include <lib/core/CHIPError.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace WiFiPAF {

/**
 *  @def NUM_PAFTP_SUPPORTED_PROTOCOL_VERSIONS
 *
 *  Number of unsigned 4-bit representations of supported transport protocol
 *  versions encapsulated in a BleTransportCapabilitiesRequest. Defined by CHIP
 *  over PAFTP protocol specification.
 */
#define NUM_PAFTP_SUPPORTED_PROTOCOL_VERSIONS 8

// Version(s) of the CHIP PAF Transport Protocol that this stack supports.
#define CHIP_PAF_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION kWiFiPAFTransportProtocolVersion_V1
#define CHIP_PAF_TRANSPORT_PROTOCOL_MAX_SUPPORTED_VERSION kWiFiPAFTransportProtocolVersion_V1

/// Enum defining versions of CHIP over PAF transport protocol.
typedef enum
{
    kWiFiPAFTransportProtocolVersion_None = 0,
    kWiFiPAFTransportProtocolVersion_V1   = 4 // PAFTP as defined by CHIP v1.5
} WiFiPAFTransportProtocolVersion;

inline constexpr size_t kCapabilitiesRequestMagicnumLength          = 2;
inline constexpr size_t kCapabilitiesRequestL2capMtuLength          = 2;
inline constexpr size_t kCapabilitiesRequestSupportedVersionsLength = 4;
inline constexpr size_t kCapabilitiesRequestWindowSizeLength        = 1;
constexpr size_t kCapabilitiesRequestLength = (kCapabilitiesRequestMagicnumLength + kCapabilitiesRequestL2capMtuLength +
                                               kCapabilitiesRequestSupportedVersionsLength + kCapabilitiesRequestWindowSizeLength);

inline constexpr size_t kCapabilitiesResponseMagicnumLength                = 2;
inline constexpr size_t kCapabilitiesResponseL2capMtuLength                = 2;
inline constexpr size_t kCapabilitiesResponseSelectedProtocolVersionLength = 1;
inline constexpr size_t kCapabilitiesResponseWindowSizeLength              = 1;
constexpr size_t kCapabilitiesResponseLength(kCapabilitiesResponseMagicnumLength + kCapabilitiesResponseL2capMtuLength +
                                             kCapabilitiesResponseSelectedProtocolVersionLength +
                                             kCapabilitiesResponseWindowSizeLength);

class PAFTransportCapabilitiesRequestMessage
{
public:
    /**
     * An array of size NUM_PAFTP_SUPPORTED_PROTOCOL_VERSIONS listing versions of the
     * PAF transport protocol that this node supports. Each protocol version is
     * specified as a 4-bit unsigned integer. A zero-value represents unused
     * array elements. Counting up from the zero-index, the first zero-value
     * specifies the end of the list of supported protocol versions.
     */
    uint8_t mSupportedProtocolVersions[(NUM_PAFTP_SUPPORTED_PROTOCOL_VERSIONS / 2) + (NUM_PAFTP_SUPPORTED_PROTOCOL_VERSIONS % 2)];

    /**
     *  The MTU that has been negotiated for this PAF connection. Specified in
     *  the PAFTransportCapabilitiesRequestMessage because the remote node may
     *  be unable to glean this info from its own PAF hardware/software stack
     *
     *  A value of 0 means that the central could not determine the negotiated
     *  PAF connection MTU.
     */
    uint16_t mMtu;

    /**
     *  The initial and maximum receive window size offered by the central
     */
    uint8_t mWindowSize;

    /**
     *  Set supported version value at given index in
     *  SupportedProtocolVersions. uint8_t version argument is truncated to 4
     *  least-significant bits. Index shall be 0 through number of
     *  SupportedProtocolVersions elements - 1.
     */
    void SetSupportedProtocolVersion(uint8_t index, uint8_t version);

    /// Must be able to reserve 20 byte data length in msgBuf.
    CHIP_ERROR Encode(const System::PacketBufferHandle & msgBuf) const;

    static CHIP_ERROR Decode(const System::PacketBufferHandle & msgBuf, PAFTransportCapabilitiesRequestMessage & msg);
};

class PAFTransportCapabilitiesResponseMessage
{
public:
    /**
     *  The lower 4 bits specify the PAF transport protocol version that the PAF
     *  peripheral has selected for this connection.
     *
     *  A value of kWiFiPAFTransportProtocolVersion_None means that no supported
     *  protocol version was found in the central's capabilities request. The
     *  central should unsubscribe after such a response has been sent to free
     *  up the peripheral for connections from devices with supported protocol
     *  versions.
     */
    uint8_t mSelectedProtocolVersion;

    /**
     *  PAF transport fragment size selected by peripheral in response to MTU
     *  value in PAFTransportCapabilitiesRequestMessage and its local
     *  observation of the PAF connection MTU.
     */
    uint16_t mFragmentSize;

    /**
     *  The initial and maximum receive window size offered by the peripheral
     */
    uint8_t mWindowSize;

    /// Must be able to reserve 20 byte data length in msgBuf.
    CHIP_ERROR Encode(const System::PacketBufferHandle & msgBuf) const;

    static CHIP_ERROR Decode(const System::PacketBufferHandle & msgBuf, PAFTransportCapabilitiesResponseMessage & msg);
};

/**
 *  The State of the Wi-Fi-PAF connection
 *
 */
enum class State
{
    kNotReady    = 0, /**< State before initialization. */
    kInitialized = 1, /**< State after class is connected and ready. */
    kConnected   = 2, /**< Endpoint connected. */
};

enum class PafInfoAccess
{
    kAccNodeInfo,
    kAccSessionId,
    kAccNodeId,
    kAccDisc,
};

class DLL_EXPORT WiFiPAFLayer
{
    friend class WiFiPAFEndPoint;
    friend class TestWiFiPAFLayer;

public:
    State mAppState                          = State::kNotReady;
    WiFiPAFLayerDelegate * mWiFiPAFTransport = nullptr;

    WiFiPAFLayer();
    static WiFiPAFLayer & GetWiFiPAFLayer();
    CHIP_ERROR Init(chip::System::Layer * systemLayer);

    typedef void (*OnCancelDeviceHandle)(uint32_t id, WiFiPAF::WiFiPafRole role);
    void Shutdown(OnCancelDeviceHandle OnCancelDevice);
    bool OnWiFiPAFMessageReceived(WiFiPAFSession & RxInfo, System::PacketBufferHandle && msg);
    CHIP_ERROR OnWiFiPAFMsgRxComplete(WiFiPAFSession & RxInfo, System::PacketBufferHandle && msg);
    State GetWiFiPAFState() { return mAppState; };
    void SetWiFiPAFState(State state);
    CHIP_ERROR SendMessage(WiFiPAF::WiFiPAFSession & TxInfo, chip::System::PacketBufferHandle && msg);
    CHIP_ERROR HandleWriteConfirmed(WiFiPAF::WiFiPAFSession & TxInfo, bool result);
    CHIP_ERROR NewEndPoint(WiFiPAFEndPoint ** retEndPoint, WiFiPAFSession & SessionInfo, WiFiPafRole role);
    typedef void (*OnSubscribeCompleteFunct)(void * appState);
    typedef void (*OnSubscribeErrorFunct)(void * appState, CHIP_ERROR err);
    CHIP_ERROR HandleTransportConnectionInitiated(WiFiPAF::WiFiPAFSession & SessionInfo,
                                                  OnSubscribeCompleteFunct OnSubscribeDoneFunc = nullptr, void * appState = nullptr,
                                                  OnSubscribeErrorFunct OnSubscribeErrFunc = nullptr);
    void OnEndPointConnectComplete(WiFiPAFEndPoint * endPoint, CHIP_ERROR err);

    static WiFiPAFTransportProtocolVersion
    GetHighestSupportedProtocolVersion(const PAFTransportCapabilitiesRequestMessage & reqMsg);

    CHIP_ERROR AddPafSession(PafInfoAccess accType, WiFiPAFSession & SessionInfo);
    CHIP_ERROR RmPafSession(PafInfoAccess accType, WiFiPAFSession & SessionInfo);
    WiFiPAFSession * GetPAFInfo(PafInfoAccess accType, WiFiPAFSession & SessionInfo);

private:
    void InitialPafInfo();
    void CleanPafInfo(WiFiPAFSession & SessionInfo);
    WiFiPAFSession mPafInfoVect[WIFIPAF_LAYER_NUM_PAF_ENDPOINTS];
    chip::System::Layer * mSystemLayer;
};

} /* namespace WiFiPAF */
} /* namespace chip */
