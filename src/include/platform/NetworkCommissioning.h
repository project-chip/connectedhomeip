/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *    @file
 *      This file defines the CHIP Device Network Provisioning object.
 *
 */

#pragma once

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/Optional.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/Variant.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/internal/DeviceNetworkInfo.h>

#include <app-common/zap-generated/cluster-enums.h>

#include <limits>
#include <utility>

namespace chip {
namespace DeviceLayer {
/**
 * We are using a namespace here, for most use cases, this namespace will be used by `using DeviceLayer::NetworkCommissioning`, but
 * this still worth a dedicated namespace since:
 *
 * - The BaseDriver / WirelessDriver is not expected to be implemented directly by users, the only occurrence is in the network
 *   commissioning cluster.
 * - We can safely name the drivers as WiFiDriver / ThreadDriver, it should not be ambiguous for most cases
 * - We can safely name the Status enum to Status, and some other structs -- if we are using using, then we should in the context of
 *   writing something dedicated to network commissioning, then a single word Status should be clear enough.
 */
namespace NetworkCommissioning {

inline constexpr size_t kMaxNetworkIDLen = 32;

// TODO: This is exactly the same as the one in GroupDataProvider, this could be moved to src/lib/support
template <typename T>
class Iterator
{
public:
    virtual ~Iterator() = default;
    /**
     *  @retval The number of entries in total that will be iterated.
     */
    virtual size_t Count() = 0;
    /**
     *   @param[out] item  Value associated with the next element in the iteration.
     *  @retval true if the next entry is successfully retrieved.
     *  @retval false if no more entries can be found.
     */
    virtual bool Next(T & item) = 0;
    /**
     * Release the memory allocated by this iterator.
     * Must be called before the pointer goes out of scope.
     */
    virtual void Release() = 0;

protected:
    Iterator() = default;
};

// The following structs follows the generated cluster object structs.
struct Network
{
    uint8_t networkID[kMaxNetworkIDLen];
    uint8_t networkIDLen = 0;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    Optional<Credentials::CertificateKeyIdStorage> networkIdentifier;
    Optional<Credentials::CertificateKeyIdStorage> clientIdentifier;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    bool connected = false;
};

static_assert(sizeof(Network::networkID) <= std::numeric_limits<decltype(Network::networkIDLen)>::max(),
              "Max length of networkID ssid exceeds the limit of networkIDLen field");

struct WiFiScanResponse
{
public:
    chip::BitFlags<app::Clusters::NetworkCommissioning::WiFiSecurityBitmap> security;
    uint8_t ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
    uint8_t ssidLen;
    uint8_t bssid[6];
    uint16_t channel;
    app::Clusters::NetworkCommissioning::WiFiBandEnum wiFiBand;
    int8_t rssi;
};

static_assert(sizeof(WiFiScanResponse::ssid) <= std::numeric_limits<decltype(WiFiScanResponse::ssidLen)>::max(),
              "Max length of WiFi ssid exceeds the limit of ssidLen field");

struct ThreadScanResponse
{
    uint16_t panId;
    uint64_t extendedPanId;
    char networkName[16];
    uint8_t networkNameLen;
    uint16_t channel;
    uint8_t version;
    uint64_t extendedAddress;
    int8_t rssi;
    uint8_t lqi;
};

static_assert(sizeof(ThreadScanResponse::networkName) <= std::numeric_limits<decltype(ThreadScanResponse::networkNameLen)>::max(),
              "Max length of WiFi credentials exceeds the limit of credentialsLen field");

using NetworkIterator            = Iterator<Network>;
using WiFiScanResponseIterator   = Iterator<WiFiScanResponse>;
using ThreadScanResponseIterator = Iterator<ThreadScanResponse>;
using Status                     = app::Clusters::NetworkCommissioning::NetworkCommissioningStatusEnum;
using WiFiBandEnum               = app::Clusters::NetworkCommissioning::WiFiBandEnum;
// For backwards compatibility with pre-rename enum values.
using WiFiBand           = WiFiBandEnum;
using WiFiSecurityBitmap = app::Clusters::NetworkCommissioning::WiFiSecurityBitmap;
// For backwards compatibility with pre-rename bitmap values.
using WiFiSecurity       = WiFiSecurityBitmap;
using ThreadCapabilities = app::Clusters::NetworkCommissioning::ThreadCapabilitiesBitmap;

// BaseDriver and WirelessDriver are the common interfaces for a network driver, platform drivers should not implement this
// directly, instead, users are expected to implement WiFiDriver, ThreadDriver and EthernetDriver.
namespace Internal {
class BaseDriver
{
public:
    class NetworkStatusChangeCallback
    {
    public:
        /**
         * @brief Callback for the network driver pushing the event of network status change to the network commissioning cluster.
         * The platforms is expected to push the status from operations such as autonomous connection after loss of connectivity or
         * during initial establishment.
         *
         * This function must be called in a thread-safe manner with CHIP stack.
         */
        virtual void OnNetworkingStatusChange(Status commissioningError, Optional<ByteSpan> networkId,
                                              Optional<int32_t> connectStatus) = 0;

        virtual ~NetworkStatusChangeCallback() = default;
    };

    /**
     * @brief Initializes the driver, this function will be called when initializing the network commissioning cluster.
     */
    virtual CHIP_ERROR Init(NetworkStatusChangeCallback * networkStatusChangeCallback) { return CHIP_NO_ERROR; }

    /**
     * @brief Shuts down the driver, this function will be called when shutting down the network commissioning cluster.
     */
    virtual void Shutdown() {}

    /**
     * @brief Returns maximum number of network configs can be added to the driver.
     */
    virtual uint8_t GetMaxNetworks() = 0;

    /**
     * @brief Returns an iterator for reading the networks, the user will always call NetworkIterator::Release. The iterator should
     * be consumed in the same context as calling GetNetworks(). Users must call Release() when the iterator goes out of scope.
     */
    virtual NetworkIterator * GetNetworks() = 0;

    /**
     * @brief Sets the status of the interface, this is an optional feature of a network driver.
     */
    virtual CHIP_ERROR SetEnabled(bool enabled) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }

    /**
     * @brief Returns the status of the interface, this is an optional feature of a network driver the driver will be enabled by
     * default.
     */
    virtual bool GetEnabled() { return true; };

    virtual ~BaseDriver() = default;
};

class WirelessDriver : public Internal::BaseDriver
{
public:
    class ConnectCallback
    {
    public:
        virtual void OnResult(Status commissioningError, CharSpan debugText, int32_t connectStatus) = 0;

        virtual ~ConnectCallback() = default;
    };

    /**
     * @brief Persists the network configurations. This function is expected to be called when CommissioningComplete event is fired.
     */
    virtual CHIP_ERROR CommitConfiguration() = 0;

    /**
     * @brief Reverts the network configurations to the last committed one. This function is expected to be called when failsafe
     * timeout reached.
     */
    virtual CHIP_ERROR RevertConfiguration() = 0;

    virtual uint8_t GetScanNetworkTimeoutSeconds()    = 0;
    virtual uint8_t GetConnectNetworkTimeoutSeconds() = 0;

    /**
     * @brief Remove a network from the device. The driver should fill the outDebugText field to pass any human-readable messages to
     * the client. The driver should reduce the size of outDebugText to 0 to omit it from the response when no debug text needs to
     * be delivered. On success, the driver should set outNetworkIndex to the index of the network just removed. The value of
     * network index is discarded on failure.
     *
     * Note: The capacity of outDebugText passed by network commissioning cluster can be configured via
     * CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE.
     */
    virtual Status RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex) = 0;

    /**
     * @brief Reorder the networks on the device. The driver should fill the outDebugText field to pass any human-readable messages
     * to the client. The driver should reduce the size of outDebugText to 0 to omit it from the response when no debug text needs
     * to be delivered.
     *
     * Note: The capacity of outDebugText passed by network commissioning cluster can be configured via
     * CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE.
     */
    virtual Status ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText) = 0;

    /**
     * @brief Initializes a network join. callback->OnResult must be called, on both success and error. Callback can be
     * called inside ConnectNetwork.
     */
    virtual void ConnectNetwork(ByteSpan networkId, ConnectCallback * callback) = 0;

#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    /**
     * @brief Disconnect from network, if currently connected.
     */
    virtual CHIP_ERROR DisconnectFromNetwork() { return CHIP_ERROR_NOT_IMPLEMENTED; }
#endif
};
} // namespace Internal

class WiFiDriver : public Internal::WirelessDriver
{
public:
    class ScanCallback
    {
    public:
        /**
         * Indicates the scan is finished, and accepts a iterator of networks discovered.
         * - networks can be nullptr when no networks discovered, or error occurred during scanning the networks.
         * OnFinished() must be called in a thread-safe manner with CHIP stack. (e.g. using ScheduleWork or ScheduleLambda)
         * - Users can assume the networks will always be used (and Release will be called) inside this function call. However, the
         * iterator might be not fully consumed (i.e. There are too many networks scanned to fit in the buffer for scan response
         * message.)
         */
        virtual void OnFinished(Status status, CharSpan debugText, WiFiScanResponseIterator * networks) = 0;

        virtual ~ScanCallback() = default;
    };

    /**
     * @brief Adds or updates a WiFi network on the device. The driver should fill the outDebugText field to pass any human-readable
     * messages to the client. The driver should reduce the size of outDebugText to 0 to omit it from the response when no debug
     * text needs to be delivered. On success, the driver should set outNetworkIndex to the index of the network just added or
     * updated. The value of network index is discarded on failure.
     *
     * Note: The capacity of outDebugText passed by network commissioning cluster can be configured via
     * CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE.
     */
    virtual Status AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                      uint8_t & outNetworkIndex) = 0;

    /**
     * @brief Initializes a WiFi network scan. callback->OnFinished must be called, on both success and error. Callback can
     * be called inside ScanNetworks.
     *
     * @param ssid        The interested SSID, the scanning SHALL be restricted to the given SSID if the ssid is not empty (i.e.
     *                    ssid.empty() is false).
     * @param callback    Callback that will be invoked upon finishing the scan
     */
    virtual void ScanNetworks(ByteSpan ssid, ScanCallback * callback) = 0;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    virtual bool SupportsPerDeviceCredentials() { return false; };

    /**
     * @brief Adds or updates a WiFi network with Per-Device Credentials on the device.
     *
     * @param ssid                          The SSID of the network to be added / updated.
     * @param networkIdentity               The Network Identity of the network, in compact-pdc-identity format.
     * @param clientIdentityNetworkIndex    If present, the index of the existing network configuration of which the Client
     *                                      Identity is to be re-used. Otherwise a new Client Identity shall be generated.
     * @param outStatus                     The application-level status code (Status::kSuccess on success).
     * @param outDebugText                  A debug text buffer that may be populated by the driver. The size of the span
     *                                      must be reduced to the length of text emitted (or 0, if none).
     * @param outClientIdentity             On success, the Client Identity that was generated or copied, depending on the
     *                                      presence of `clientIdentityNetworkIndex`.
     * @param outNextworkIndex              On success, the index of the network entry that was added or updated.
     *
     * @retval CHIP_NO_ERROR and outStatus == kSuccess on success.
     * @retval CHIP_NO_ERROR and outStatus != kSuccess for application-level errors. outDebugText should be populated.
     * @retval CHIP_ERROR_* on internal errors. None of the output parameters will be examined in this case.
     */
    virtual CHIP_ERROR AddOrUpdateNetworkWithPDC(ByteSpan ssid, ByteSpan networkIdentity,
                                                 Optional<uint8_t> clientIdentityNetworkIndex, Status & outStatus,
                                                 MutableCharSpan & outDebugText, MutableByteSpan & outClientIdentity,
                                                 uint8_t & outNetworkIndex)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    /**
     * @brief Retrieves the Network Identity associated with a network.
     *
     * @param networkIndex          The 0-based index of the network.
     * @param outNetworkIdentity    The output buffer to be populated with the Network
     *                              Identity in compact-pdc-identity TLV format.
     *
     * @return CHIP_NO_ERROR on success or a CHIP_ERROR on failure.
     */
    virtual CHIP_ERROR GetNetworkIdentity(uint8_t networkIndex, MutableByteSpan & outNetworkIdentity)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    /**
     * @brief Retrieves the Network Client Identity associated with a network.
     *
     * @param networkIndex          The 0-based index of the network.
     * @param outNetworkIdentity    The output buffer to be populated with the Network
     *                              Client Identity in compact-pdc-identity TLV format.
     *
     * @return CHIP_NO_ERROR on success or a CHIP_ERROR on failure.
     */
    virtual CHIP_ERROR GetClientIdentity(uint8_t networkIndex, MutableByteSpan & outClientIdentity)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    /**
     * @brief Signs the specified message with the private key of a Network Client Identity.
     */
    virtual CHIP_ERROR SignWithClientIdentity(uint8_t networkIndex, const ByteSpan & message,
                                              Crypto::P256ECDSASignature & outSignature)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    /**
     *  @brief Provide all the frequency bands supported by the Wi-Fi interface.
     *
     *  The default implementation returns the 2.4 GHz band support.
     *  Note: WiFi platforms should implement this function in their WiFiDriver to provide their complete device capabilities.
     *
     *  The returned bit mask has values of WiFiBandEnum packed into the bits. For example:
     *
     *    - Bit 0 = (WiFiBandEnum::k2g4 == 0) --> (1 << 0) == (1 << WiFiBandEnum::k2g4)
     *    - Bit 2 = (WiFiBandEnum::k5g == 2) --> (1 << 2) == (1 << WiFiBandEnum::k5g)
     *    - If both 2.4G and 5G are supported --> ((1 << k2g4) || (1 << k5g)) == (1 || 4) == 5
     *
     *  On error, return 0 (no bands supported). This should never happen... Note that
     *  certification tests will REQUIRE at least one bit set in the set.
     *
     *  @return a bitmask of supported Wi-Fi bands where each bit is associated with a WiFiBandEnum value.
     */
    virtual uint32_t GetSupportedWiFiBandsMask() const
    {
        // Default to 2.4G support (100% example platform coverage as of Matter 1.3) listed.
        return static_cast<uint32_t>(1UL << chip::to_underlying(WiFiBandEnum::k2g4));
    }

    ~WiFiDriver() override = default;
};

class ThreadDriver : public Internal::WirelessDriver
{
public:
    class ScanCallback
    {
    public:
        /**
         * Indicates the scan is finished, and accepts a iterator of networks discovered.
         * - networks can be nullptr when no networks discovered, or error occurred during scanning the networks.
         * OnFinished() must be called in a thread-safe manner with CHIP stack. (e.g. using ScheduleWork or ScheduleLambda)
         * - Users can assume the networks will always be used (and Release will be called) inside this function call. However, the
         * iterator might be not fully consumed (i.e. There are too many networks scanned to fit in the buffer for scan response
         * message.)
         */
        virtual void OnFinished(Status err, CharSpan debugText, ThreadScanResponseIterator * networks) = 0;

        virtual ~ScanCallback() = default;
    };

    /**
     * @brief Adds or updates a Thread network on the device. The driver should fill the outDebugText field to pass any
     * human-readable messages to the client. The driver should reduce the size of outDebugText to 0 to omit it from the response
     * when no debug text needs to be delivered. On success, the driver should set outNetworkIndex to the index of the network just
     * added or updated. The value of the network index is discarded on failure.
     *
     * Note: The capacity of outDebugText passed by network commissioning cluster can be configured via
     * CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE.
     */
    virtual Status AddOrUpdateNetwork(ByteSpan operationalDataset, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex) = 0;

    /**
     * @brief Initializes a Thread network scan. callback->OnFinished must be called, on both success and error. Callback can
     * be called inside ScanNetworks.
     */
    virtual void ScanNetworks(ScanCallback * callback) = 0;

    /**
     * @brief Provide all of the Thread features supported by the Thread interface
     */
    virtual ThreadCapabilities GetSupportedThreadFeatures() = 0;

    /**
     * @brief Return the Thread version supported by the Thread interface
     */
    virtual uint16_t GetThreadVersion() = 0;

    ~ThreadDriver() override = default;
};

class EthernetDriver : public Internal::BaseDriver
{
    // Ethernet driver does not have any special operations.
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
