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
 *          Defines the public interface for the Device Layer DiagnosticDataProvider object.
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <platform/CHIPDeviceBuildConfig.h>
#include <platform/GeneralFaults.h>

namespace chip {
namespace DeviceLayer {

// Maximum length of vendor defined name or prefix of the software thread that is
// static for the duration of the thread.
static constexpr size_t kMaxThreadNameLength = 32;

// 48-bit IEEE MAC Address or a 64-bit IEEE MAC Address (e.g. EUI-64).
constexpr size_t kMaxHardwareAddrSize = 8;

struct ThreadMetrics : public app::Clusters::SoftwareDiagnostics::Structs::ThreadMetrics::Type
{
    char NameBuf[kMaxThreadNameLength + 1];
    ThreadMetrics * Next; /* Pointer to the next structure.  */
};

struct NetworkInterface : public app::Clusters::GeneralDiagnostics::Structs::NetworkInterfaceType::Type
{
    char Name[Inet::InterfaceId::kMaxIfNameLength];
    uint8_t MacAddress[kMaxHardwareAddrSize];
    NetworkInterface * Next; /* Pointer to the next structure.  */
};

/**
 * Defines the General Diagnostics Delegate class to notify platform events.
 */
class GeneralDiagnosticsDelegate
{
public:
    virtual ~GeneralDiagnosticsDelegate() {}

    /**
     * @brief
     *   Called after the current device is rebooted.
     */
    virtual void OnDeviceRebooted() {}

    /**
     * @brief
     *   Called when the Node detects a hardware fault has been raised.
     */
    virtual void OnHardwareFaultsDetected(GeneralFaults<kMaxHardwareFaults> & previous, GeneralFaults<kMaxHardwareFaults> & current)
    {}

    /**
     * @brief
     *   Called when the Node detects a radio fault has been raised.
     */
    virtual void OnRadioFaultsDetected(GeneralFaults<kMaxRadioFaults> & previous, GeneralFaults<kMaxRadioFaults> & current) {}

    /**
     * @brief
     *   Called when the Node detects a network fault has been raised.
     */
    virtual void OnNetworkFaultsDetected(GeneralFaults<kMaxNetworkFaults> & previous, GeneralFaults<kMaxNetworkFaults> & current) {}
};

/**
 * Defines the Software Diagnostics Delegate class to notify software events.
 */
class SoftwareDiagnosticsDelegate
{
public:
    virtual ~SoftwareDiagnosticsDelegate() {}

    /**
     * @brief
     *   Called when a software fault that has taken place on the Node.
     */
    virtual void
    OnSoftwareFaultDetected(chip::app::Clusters::SoftwareDiagnostics::Structs::SoftwareFaultStruct::Type & softwareFault)
    {}
};

/**
 * Defines the WiFi Diagnostics Delegate class to notify WiFi network events.
 */
class WiFiDiagnosticsDelegate
{
public:
    virtual ~WiFiDiagnosticsDelegate() {}

    /**
     * @brief
     *   Called when the Node detects Node’s Wi-Fi connection has been disconnected.
     */
    virtual void OnDisconnectionDetected(uint16_t reasonCode) {}

    /**
     * @brief
     *   Called when the Node fails to associate or authenticate an access point.
     */
    virtual void OnAssociationFailureDetected(uint8_t associationFailureCause, uint16_t status) {}

    /**
     * @brief
     *   Called when the Node’s connection status to a Wi-Fi network has changed.
     */
    virtual void OnConnectionStatusChanged(uint8_t connectionStatus) {}
};

/**
 * Provides access to runtime and build-time configuration information for a chip device.
 */
class DiagnosticDataProvider
{
public:
    enum BootReasonType : uint8_t
    {
        Unspecified             = 0,
        PowerOnReboot           = 1,
        BrownOutReset           = 2,
        SoftwareWatchdogReset   = 3,
        HardwareWatchdogReset   = 4,
        SoftwareUpdateCompleted = 5,
        SoftwareReset           = 6,
    };

    void SetGeneralDiagnosticsDelegate(GeneralDiagnosticsDelegate * delegate) { mGeneralDiagnosticsDelegate = delegate; }
    GeneralDiagnosticsDelegate * GetGeneralDiagnosticsDelegate() const { return mGeneralDiagnosticsDelegate; }

    void SetSoftwareDiagnosticsDelegate(SoftwareDiagnosticsDelegate * delegate) { mSoftwareDiagnosticsDelegate = delegate; }
    SoftwareDiagnosticsDelegate * GetSoftwareDiagnosticsDelegate() const { return mSoftwareDiagnosticsDelegate; }

    void SetWiFiDiagnosticsDelegate(WiFiDiagnosticsDelegate * delegate) { mWiFiDiagnosticsDelegate = delegate; }
    WiFiDiagnosticsDelegate * GetWiFiDiagnosticsDelegate() const { return mWiFiDiagnosticsDelegate; }

    /**
     * General Diagnostics methods.
     */
    virtual CHIP_ERROR GetRebootCount(uint16_t & rebootCount);
    virtual CHIP_ERROR GetUpTime(uint64_t & upTime);
    virtual CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours);
    virtual CHIP_ERROR GetBootReason(uint8_t & bootReason);
    virtual CHIP_ERROR GetActiveHardwareFaults(GeneralFaults<kMaxHardwareFaults> & hardwareFaults);
    virtual CHIP_ERROR GetActiveRadioFaults(GeneralFaults<kMaxRadioFaults> & radioFaults);
    virtual CHIP_ERROR GetActiveNetworkFaults(GeneralFaults<kMaxNetworkFaults> & networkFaults);

    /*
     * Get the linked list of network interfaces of the current plaform. After usage, each caller of GetNetworkInterfaces
     * needs to release the network interface list it gets via ReleaseNetworkInterfaces.
     *
     */
    virtual CHIP_ERROR GetNetworkInterfaces(NetworkInterface ** netifpp);
    virtual void ReleaseNetworkInterfaces(NetworkInterface * netifp);

    /**
     * Software Diagnostics methods.
     */
    virtual CHIP_ERROR GetCurrentHeapFree(uint64_t & currentHeapFree);
    virtual CHIP_ERROR GetCurrentHeapUsed(uint64_t & currentHeapUsed);
    virtual CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark);

    /*
     * Get the linked list of thread metrics of the current plaform. After usage, each caller of GetThreadMetrics
     * needs to release the thread metrics list it gets via ReleaseThreadMetrics.
     *
     */
    virtual CHIP_ERROR GetThreadMetrics(ThreadMetrics ** threadMetricsOut);
    virtual void ReleaseThreadMetrics(ThreadMetrics * threadMetrics);

    /**
     * Ethernet network diagnostics methods
     */
    virtual CHIP_ERROR GetEthPHYRate(uint8_t & pHYRate);
    virtual CHIP_ERROR GetEthFullDuplex(bool & fullDuplex);
    virtual CHIP_ERROR GetEthCarrierDetect(bool & carrierDetect);
    virtual CHIP_ERROR GetEthTimeSinceReset(uint64_t & timeSinceReset);
    virtual CHIP_ERROR GetEthPacketRxCount(uint64_t & packetRxCount);
    virtual CHIP_ERROR GetEthPacketTxCount(uint64_t & packetTxCount);
    virtual CHIP_ERROR GetEthTxErrCount(uint64_t & txErrCount);
    virtual CHIP_ERROR GetEthCollisionCount(uint64_t & collisionCount);
    virtual CHIP_ERROR GetEthOverrunCount(uint64_t & overrunCount);
    virtual CHIP_ERROR ResetEthNetworkDiagnosticsCounts();

    /**
     * WiFi network diagnostics methods
     */
    virtual CHIP_ERROR GetWiFiBssId(ByteSpan & value);
    virtual CHIP_ERROR GetWiFiSecurityType(uint8_t & securityType);
    virtual CHIP_ERROR GetWiFiVersion(uint8_t & wiFiVersion);
    virtual CHIP_ERROR GetWiFiChannelNumber(uint16_t & channelNumber);
    virtual CHIP_ERROR GetWiFiRssi(int8_t & rssi);
    virtual CHIP_ERROR GetWiFiBeaconLostCount(uint32_t & beaconLostCount);
    virtual CHIP_ERROR GetWiFiBeaconRxCount(uint32_t & beaconRxCount);
    virtual CHIP_ERROR GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount);
    virtual CHIP_ERROR GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount);
    virtual CHIP_ERROR GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount);
    virtual CHIP_ERROR GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount);
    virtual CHIP_ERROR GetWiFiCurrentMaxRate(uint64_t & currentMaxRate);
    virtual CHIP_ERROR GetWiFiOverrunCount(uint64_t & overrunCount);
    virtual CHIP_ERROR ResetWiFiNetworkDiagnosticsCounts();

protected:
    // Construction/destruction limited to subclasses.
    DiagnosticDataProvider()          = default;
    virtual ~DiagnosticDataProvider() = default;

private:
    GeneralDiagnosticsDelegate * mGeneralDiagnosticsDelegate   = nullptr;
    SoftwareDiagnosticsDelegate * mSoftwareDiagnosticsDelegate = nullptr;
    WiFiDiagnosticsDelegate * mWiFiDiagnosticsDelegate         = nullptr;

    // No copy, move or assignment.
    DiagnosticDataProvider(const DiagnosticDataProvider &)  = delete;
    DiagnosticDataProvider(const DiagnosticDataProvider &&) = delete;
    DiagnosticDataProvider & operator=(const DiagnosticDataProvider &) = delete;
};

/**
 * Returns a reference to a DiagnosticDataProvider object.
 *
 * Applications should use this to access the features of the DiagnosticDataProvider.
 */
DiagnosticDataProvider & GetDiagnosticDataProvider();

/**
 * Sets a reference to a DiagnosticDataProvider object.
 *
 * This must be called before any calls to GetDiagnosticDataProvider. If a nullptr is passed in,
 * no changes will be made.
 */
void SetDiagnosticDataProvider(DiagnosticDataProvider * diagnosticDataProvider);

inline CHIP_ERROR DiagnosticDataProvider::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetThreadMetrics(ThreadMetrics ** threadMetricsOut)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline void DiagnosticDataProvider::ReleaseThreadMetrics(ThreadMetrics * threadMetrics) {}

inline CHIP_ERROR DiagnosticDataProvider::GetRebootCount(uint16_t & rebootCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetUpTime(uint64_t & upTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetBootReason(uint8_t & bootReason)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetActiveHardwareFaults(GeneralFaults<kMaxHardwareFaults> & hardwareFaults)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetActiveRadioFaults(GeneralFaults<kMaxRadioFaults> & radioFaults)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetActiveNetworkFaults(GeneralFaults<kMaxNetworkFaults> & networkFaults)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline void DiagnosticDataProvider::ReleaseNetworkInterfaces(NetworkInterface * netifp) {}

inline CHIP_ERROR DiagnosticDataProvider::GetEthPHYRate(uint8_t & pHYRate)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetEthFullDuplex(bool & fullDuplex)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetEthCarrierDetect(bool & carrierDetect)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetEthTimeSinceReset(uint64_t & timeSinceReset)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetEthPacketRxCount(uint64_t & packetRxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetEthPacketTxCount(uint64_t & packetTxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetEthTxErrCount(uint64_t & txErrCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetEthCollisionCount(uint64_t & collisionCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetEthOverrunCount(uint64_t & overrunCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::ResetEthNetworkDiagnosticsCounts()
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetWiFiBssId(ByteSpan & value)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetWiFiSecurityType(uint8_t & securityType)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetWiFiVersion(uint8_t & wiFiVersion)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetWiFiRssi(int8_t & rssi)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetWiFiBeaconRxCount(uint32_t & beaconRxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::GetWiFiOverrunCount(uint64_t & overrunCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline CHIP_ERROR DiagnosticDataProvider::ResetWiFiNetworkDiagnosticsCounts()
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace DeviceLayer
} // namespace chip
