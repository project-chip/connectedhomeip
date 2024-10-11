/*
 *
 *    Copyright (c) 2021-2024 Project CHIP Authors
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

#include <limits>

#include "network-commissioning.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/clusters/general-commissioning-server/general-commissioning-server.h>
#include <app/data-model/Nullable.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <credentials/CHIPCert.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/SafeInt.h>
#include <lib/support/SortUtils.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConnectivityManager.h>
#include <platform/DeviceControlServer.h>
#include <platform/PlatformManager.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <tracing/macros.h>

#include <array>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

using namespace Credentials;
using namespace DataModel;
using namespace DeviceLayer::NetworkCommissioning;

namespace {

// For WiFi and Thread scan results, each item will cost ~60 bytes in TLV, thus 15 is a safe upper bound of scan results.
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP || CHIP_DEVICE_CONFIG_ENABLE_THREAD
constexpr size_t kMaxNetworksInScanResponse = 15;
#endif

constexpr uint16_t kCurrentClusterRevision = 2;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
constexpr size_t kPossessionNonceSize = 32;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

// Note: CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE can be 0, this disables debug text
using DebugTextStorage = std::array<char, CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE>;

enum ValidWiFiCredentialLength
{
    kOpen      = 0,
    kWEP64     = 5,
    kMinWPAPSK = 8,
    kMaxWPAPSK = 63,
    kWPAPSKHex = 64,
};

template <typename T, typename Func>
static void EnumerateAndRelease(Iterator<T> * iterator, Func func)
{
    if (iterator != nullptr)
    {
        T element;
        while (iterator->Next(element) && func(element) == Loop::Continue)
        {
            /* continue */
        }
        iterator->Release();
    }
}

template <typename T>
static size_t CountAndRelease(Iterator<T> * iterator)
{
    size_t count = 0;
    if (iterator != nullptr)
    {
        count = iterator->Count();
        iterator->Release();
    }

    return count;
}

BitFlags<Feature> WiFiFeatures(WiFiDriver * driver)
{
    BitFlags<Feature> features = Feature::kWiFiNetworkInterface;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    features.Set(Feature::kPerDeviceCredentials, driver->SupportsPerDeviceCredentials());
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    return features;
}

/// Performs an auto-release of the given item, generally an `Iterator` type
/// like Wifi or Thread scan results.
template <typename T>
class AutoRelease
{
public:
    AutoRelease(T * iterator) : mValue(iterator) {}
    ~AutoRelease()
    {
        if (mValue != nullptr)
        {
            mValue->Release();
        }
    }

private:
    T * mValue;
};

/// Convenience macro to auto-create a variable for you to release the given name at
/// the exit of the current scope.
#define DEFER_AUTO_RELEASE(name) AutoRelease autoRelease##__COUNTER__(name)

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP

/// Handles encoding a WifiScanResponseIterator into a TLV response structure
class WifiScanResponseToTLV : public chip::app::DataModel::EncodableToTLV
{
public:
    WifiScanResponseToTLV(Status status, CharSpan debugText, WiFiScanResponseIterator * networks) :
        mStatus(status), mDebugText(debugText), mNetworks(networks)
    {}

    CHIP_ERROR EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag) const override;

private:
    Status mStatus;
    CharSpan mDebugText;
    WiFiScanResponseIterator * mNetworks;
};

CHIP_ERROR WifiScanResponseToTLV::EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag) const
{
    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outerType));

    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kNetworkingStatus), mStatus));
    if (mDebugText.size() != 0)
    {
        ReturnErrorOnFailure(
            DataModel::Encode(writer, TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kDebugText), mDebugText));
    }

    {
        TLV::TLVType listContainerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kWiFiScanResults),
                                                   TLV::kTLVType_Array, listContainerType));

        if ((mStatus == Status::kSuccess) && (mNetworks != nullptr))
        {
            WiFiScanResponse scanResponse;
            size_t networksEncoded = 0;
            while (mNetworks->Next(scanResponse))
            {
                Structs::WiFiInterfaceScanResultStruct::Type result;
                result.security = scanResponse.security;
                result.ssid     = ByteSpan(scanResponse.ssid, scanResponse.ssidLen);
                result.bssid    = ByteSpan(scanResponse.bssid, sizeof(scanResponse.bssid));
                result.channel  = scanResponse.channel;
                result.wiFiBand = scanResponse.wiFiBand;
                result.rssi     = scanResponse.rssi;
                ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), result));

                ++networksEncoded;
                if (networksEncoded >= kMaxNetworksInScanResponse)
                {
                    break;
                }
            }
        }

        ReturnErrorOnFailure(writer.EndContainer(listContainerType));
    }

    return writer.EndContainer(outerType);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

/// Handles encoding a ThreadScanResponseIterator into a TLV response structure.
class ThreadScanResponseToTLV : public chip::app::DataModel::EncodableToTLV
{
public:
    ThreadScanResponseToTLV(Status status, CharSpan debugText, ThreadScanResponseIterator * networks) :
        mStatus(status), mDebugText(debugText), mNetworks(networks)
    {}

    CHIP_ERROR EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag) const override;

private:
    Status mStatus;
    CharSpan mDebugText;
    ThreadScanResponseIterator * mNetworks;

    /// Fills up scanResponseArray with valid and de-duplicated thread responses from mNetworks.
    /// Handles sorting and keeping only larger rssi
    ///
    /// Returns the valid list of scan responses into `validResponses`, which is only valid
    /// as long as scanResponseArray is valid.
    CHIP_ERROR LoadResponses(Platform::ScopedMemoryBuffer<ThreadScanResponse> & scanResponseArray,
                             Span<ThreadScanResponse> & validResponses) const;
};

CHIP_ERROR ThreadScanResponseToTLV::LoadResponses(Platform::ScopedMemoryBuffer<ThreadScanResponse> & scanResponseArray,
                                                  Span<ThreadScanResponse> & validResponses) const
{
    VerifyOrReturnError(scanResponseArray.Alloc(chip::min(mNetworks->Count(), kMaxNetworksInScanResponse)), CHIP_ERROR_NO_MEMORY);

    ThreadScanResponse scanResponse;
    size_t scanResponseArrayLength = 0;
    for (; mNetworks != nullptr && mNetworks->Next(scanResponse);)
    {
        if ((scanResponseArrayLength == kMaxNetworksInScanResponse) &&
            (scanResponseArray[scanResponseArrayLength - 1].rssi > scanResponse.rssi))
        {
            continue;
        }

        bool isDuplicated = false;

        for (size_t i = 0; i < scanResponseArrayLength; i++)
        {
            if ((scanResponseArray[i].panId == scanResponse.panId) &&
                (scanResponseArray[i].extendedPanId == scanResponse.extendedPanId))
            {
                if (scanResponseArray[i].rssi < scanResponse.rssi)
                {
                    scanResponseArray[i] = scanResponseArray[--scanResponseArrayLength];
                }
                else
                {
                    isDuplicated = true;
                }
                break;
            }
        }

        if (isDuplicated)
        {
            continue;
        }

        if (scanResponseArrayLength < kMaxNetworksInScanResponse)
        {
            scanResponseArrayLength++;
        }
        scanResponseArray[scanResponseArrayLength - 1] = scanResponse;

        // TODO: this is a sort (insertion sort even, so O(n^2)) in a O(n) loop.
        ///      There should be some better alternatives to not have some O(n^3) processing complexity.
        Sorting::InsertionSort(scanResponseArray.Get(), scanResponseArrayLength,
                               [](const ThreadScanResponse & a, const ThreadScanResponse & b) -> bool { return a.rssi > b.rssi; });
    }

    validResponses = Span<ThreadScanResponse>(scanResponseArray.Get(), scanResponseArrayLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadScanResponseToTLV::EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag) const
{
    Platform::ScopedMemoryBuffer<ThreadScanResponse> responseArray;
    Span<ThreadScanResponse> responseSpan;

    ReturnErrorOnFailure(LoadResponses(responseArray, responseSpan));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outerType));

    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kNetworkingStatus), mStatus));
    if (mDebugText.size() != 0)
    {
        ReturnErrorOnFailure(
            DataModel::Encode(writer, TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kDebugText), mDebugText));
    }

    {
        TLV::TLVType listContainerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kThreadScanResults),
                                                   TLV::kTLVType_Array, listContainerType));

        for (const ThreadScanResponse & response : responseSpan)
        {
            Structs::ThreadInterfaceScanResultStruct::Type result;
            uint8_t extendedAddressBuffer[Thread::kSizeExtendedPanId];

            Encoding::BigEndian::Put64(extendedAddressBuffer, response.extendedAddress);
            result.panId           = response.panId;
            result.extendedPanId   = response.extendedPanId;
            result.networkName     = CharSpan(response.networkName, response.networkNameLen);
            result.channel         = response.channel;
            result.version         = response.version;
            result.extendedAddress = ByteSpan(extendedAddressBuffer);
            result.rssi            = response.rssi;
            result.lqi             = response.lqi;

            ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), result));
        }

        ReturnErrorOnFailure(writer.EndContainer(listContainerType));
    }

    return writer.EndContainer(outerType);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

} // namespace

#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
Instance::NetworkInstanceList Instance::sInstances;
#endif

Instance::Instance(EndpointId aEndpointId, WiFiDriver * apDelegate) :
    CommandHandlerInterface(Optional<EndpointId>(aEndpointId), Id), AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id),
    mEndpointId(aEndpointId), mFeatureFlags(WiFiFeatures(apDelegate)), mpWirelessDriver(apDelegate), mpBaseDriver(apDelegate)
{
    mpDriver.Set<WiFiDriver *>(apDelegate);
}

Instance::Instance(EndpointId aEndpointId, ThreadDriver * apDelegate) :
    CommandHandlerInterface(Optional<EndpointId>(aEndpointId), Id), AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id),
    mEndpointId(aEndpointId), mFeatureFlags(Feature::kThreadNetworkInterface), mpWirelessDriver(apDelegate),
    mpBaseDriver(apDelegate)
{
    mpDriver.Set<ThreadDriver *>(apDelegate);
}

Instance::Instance(EndpointId aEndpointId, EthernetDriver * apDelegate) :
    CommandHandlerInterface(Optional<EndpointId>(aEndpointId), Id), AttributeAccessInterface(Optional<EndpointId>(aEndpointId), Id),
    mEndpointId(aEndpointId), mFeatureFlags(Feature::kEthernetNetworkInterface), mpWirelessDriver(nullptr), mpBaseDriver(apDelegate)
{}

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this)));
    ReturnErrorOnFailure(mpBaseDriver->Init(this));
    mLastNetworkingStatusValue.SetNull();
    mLastConnectErrorValue.SetNull();
    mLastNetworkIDLen = 0;
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    if (!sInstances.Contains(this))
    {
        sInstances.PushBack(this);
    }
#endif
    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    if (sInstances.Contains(this))
    {
        sInstances.Remove(this);
    }
#endif
    mpBaseDriver->Shutdown();
}

#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
void Instance::SendNonConcurrentConnectNetworkResponse()
{
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        return;
    }

#if CONFIG_NETWORK_LAYER_BLE
    DeviceLayer::ConnectivityMgr().GetBleLayer()->IndicateBleClosing();
#endif // CONFIG_NETWORK_LAYER_BLE
    ChipLogProgress(NetworkProvisioning, "Non-concurrent mode. Send ConnectNetworkResponse(Success)");
    Commands::ConnectNetworkResponse::Type response;
    response.networkingStatus = NetworkCommissioning::Status::kSuccess;
    commandHandle->AddResponse(mPath, response);
}
#endif // CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION

void Instance::SetLastNetworkingStatusValue(Attributes::LastNetworkingStatus::TypeInfo::Type networkingStatusValue)
{
    if (mLastNetworkingStatusValue.Update(networkingStatusValue))
    {
        MatterReportingAttributeChangeCallback(mEndpointId, Clusters::NetworkCommissioning::Id,
                                               Attributes::LastNetworkingStatus::TypeInfo::GetAttributeId());
    }
}

void Instance::SetLastConnectErrorValue(Attributes::LastConnectErrorValue::TypeInfo::Type connectErrorValue)
{
    if (mLastConnectErrorValue.Update(connectErrorValue))
    {
        MatterReportingAttributeChangeCallback(mEndpointId, Clusters::NetworkCommissioning::Id,
                                               Attributes::LastConnectErrorValue::TypeInfo::GetAttributeId());
    }
}

void Instance::SetLastNetworkId(ByteSpan lastNetworkId)
{
    ByteSpan prevLastNetworkId{ mLastNetworkID, mLastNetworkIDLen };
    VerifyOrReturn(lastNetworkId.size() <= kMaxNetworkIDLen);
    VerifyOrReturn(!prevLastNetworkId.data_equal(lastNetworkId));

    memcpy(mLastNetworkID, lastNetworkId.data(), lastNetworkId.size());
    mLastNetworkIDLen = static_cast<uint8_t>(lastNetworkId.size());
    MatterReportingAttributeChangeCallback(mEndpointId, Clusters::NetworkCommissioning::Id,
                                           Attributes::LastNetworkID::TypeInfo::GetAttributeId());
}

void Instance::ReportNetworksListChanged() const
{
    MatterReportingAttributeChangeCallback(mEndpointId, Clusters::NetworkCommissioning::Id,
                                           Attributes::Networks::TypeInfo::GetAttributeId());
}

void Instance::InvokeCommand(HandlerContext & ctxt)
{
    if (mAsyncCommandHandle.Get() != nullptr)
    {
        // We have a command processing in the backend, reject all incoming commands.
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Busy);
        ctxt.SetCommandHandled();
        return;
    }

    // Since mPath is used for building the response command, and we have checked that we are not pending the response of another
    // command above. So it is safe to set the mPath here and not clear it when return.
    mPath = ctxt.mRequestPath;

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::ScanNetworks::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));
        HandleCommand<Commands::ScanNetworks::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleScanNetworks(ctx, req); });
        return;

    case Commands::AddOrUpdateWiFiNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface));
        HandleCommand<Commands::AddOrUpdateWiFiNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleAddOrUpdateWiFiNetwork(ctx, req); });
        return;

    case Commands::AddOrUpdateThreadNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kThreadNetworkInterface));
        HandleCommand<Commands::AddOrUpdateThreadNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleAddOrUpdateThreadNetwork(ctx, req); });
        return;

    case Commands::RemoveNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));
        HandleCommand<Commands::RemoveNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleRemoveNetwork(ctx, req); });
        return;

    case Commands::ConnectNetwork::Id: {
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));

        HandleCommand<Commands::ConnectNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleConnectNetwork(ctx, req); });
        return;
    }

    case Commands::ReorderNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));
        HandleCommand<Commands::ReorderNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleReorderNetwork(ctx, req); });
        return;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    case Commands::QueryIdentity::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kPerDeviceCredentials));
        HandleCommand<Commands::QueryIdentity::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleQueryIdentity(ctx, req); });
        return;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    }
}

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::MaxNetworks::Id:
        return aEncoder.Encode(mpBaseDriver->GetMaxNetworks());

    case Attributes::Networks::Id:
        return aEncoder.EncodeList([this](const auto & encoder) {
            CHIP_ERROR err = CHIP_NO_ERROR;
            Structs::NetworkInfoStruct::Type networkForEncode;
            EnumerateAndRelease(mpBaseDriver->GetNetworks(), [&](const Network & network) {
                networkForEncode.networkID = ByteSpan(network.networkID, network.networkIDLen);
                networkForEncode.connected = network.connected;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
                // These fields are both optional and nullable in NetworkInfoStruct.
                // If PDC is supported, the fields are always present but may be null.
                if (mFeatureFlags.Has(Feature::kPerDeviceCredentials))
                {
                    networkForEncode.networkIdentifier = MakeOptional(Nullable<ByteSpan>(network.networkIdentifier));
                    networkForEncode.clientIdentifier  = MakeOptional(Nullable<ByteSpan>(network.clientIdentifier));
                }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

                err = encoder.Encode(networkForEncode);
                return (err == CHIP_NO_ERROR) ? Loop::Continue : Loop::Break;
            });
            return err;
        });

    case Attributes::ScanMaxTimeSeconds::Id:
        if (mpWirelessDriver != nullptr)
        {
            return aEncoder.Encode(mpWirelessDriver->GetScanNetworkTimeoutSeconds());
        }
        return CHIP_NO_ERROR;

    case Attributes::ConnectMaxTimeSeconds::Id:
        if (mpWirelessDriver != nullptr)
        {
            return aEncoder.Encode(mpWirelessDriver->GetConnectNetworkTimeoutSeconds());
        }
        return CHIP_NO_ERROR;

    case Attributes::InterfaceEnabled::Id:
        return aEncoder.Encode(mpBaseDriver->GetEnabled());

    case Attributes::LastNetworkingStatus::Id:
        return aEncoder.Encode(mLastNetworkingStatusValue);

    case Attributes::LastNetworkID::Id:
        if (mLastNetworkIDLen == 0)
        {
            return aEncoder.EncodeNull();
        }
        else
        {
            return aEncoder.Encode(ByteSpan(mLastNetworkID, mLastNetworkIDLen));
        }

    case Attributes::LastConnectErrorValue::Id:
        return aEncoder.Encode(mLastConnectErrorValue);

    case Attributes::FeatureMap::Id:
        return aEncoder.Encode(mFeatureFlags);

    case Attributes::ClusterRevision::Id:
        return aEncoder.Encode(kCurrentClusterRevision);

    case Attributes::SupportedWiFiBands::Id: {
#if (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
        // TODO https://github.com/project-chip/connectedhomeip/issues/31431
        // This is a case of shared zap config where mandatory wifi attributes are enabled for a thread platform (e.g
        // all-cluster-app). Real world product must only enable the attributes tied to the network technology supported by their
        // product. Temporarily return an list of 1 element of value 0 when wifi is not supported or WiFiNetworkInterface is not
        // enabled until a solution is implemented with the attribute list.
        // Final implementation will return UnsupportedAttribute if we get here without the needed WiFi support .
        // VerifyOrReturnError(mFeatureFlags.Has(Feature::kWiFiNetworkInterface), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
        if (mFeatureFlags.Has(Feature::kWiFiNetworkInterface))
        {
            return aEncoder.EncodeList([this](const auto & encoder) {
                uint32_t bands = mpDriver.Get<WiFiDriver *>()->GetSupportedWiFiBandsMask();

                // Extract every band from the bitmap of supported bands, starting positionally on the right.
                for (uint32_t band_bit_pos = 0; band_bit_pos < std::numeric_limits<uint32_t>::digits; ++band_bit_pos)
                {
                    uint32_t band_mask = static_cast<uint32_t>(1UL << band_bit_pos);
                    if ((bands & band_mask) != 0)
                    {
                        ReturnErrorOnFailure(encoder.Encode(static_cast<WiFiBandEnum>(band_bit_pos)));
                    }
                }
                return CHIP_NO_ERROR;
            });
        }
#endif
        return aEncoder.EncodeList([](const auto & encoder) {
            WiFiBandEnum bands = WiFiBandEnum::k2g4;
            ReturnErrorOnFailure(encoder.Encode(bands));
            return CHIP_NO_ERROR;
        });
    }
    break;
    case Attributes::SupportedThreadFeatures::Id: {
        // TODO https://github.com/project-chip/connectedhomeip/issues/31431
        BitMask<ThreadCapabilities> ThreadCapabilities = 0;
#if (CHIP_DEVICE_CONFIG_ENABLE_THREAD)
        // This is a case of shared zap config where mandatory thread attributes are enabled for a wifi platform (e.g
        // all-cluster-app). Real world product must only enable the attributes tied to the network technology supported by their
        // product. Temporarily encode a value of 0 reflecting no thread capabilities hen CHIP_DEVICE_CONFIG_ENABLE_THREAD or
        // ThreadNetworkInterface are not enabled until a solution is implemented with the attribute list.
        // Final implementation will return UnsupportedAttribute if we get here without the needed thread support
        // VerifyOrReturnError(mFeatureFlags.Has(Feature::kThreadNetworkInterface), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
        if (mFeatureFlags.Has(Feature::kThreadNetworkInterface))
        {
            ThreadCapabilities = mpDriver.Get<ThreadDriver *>()->GetSupportedThreadFeatures();
        }
#endif
        return aEncoder.Encode(ThreadCapabilities);
    }
    break;
    case Attributes::ThreadVersion::Id: {
        // TODO https://github.com/project-chip/connectedhomeip/issues/31431ß
        uint16_t threadVersion = 0;
#if (CHIP_DEVICE_CONFIG_ENABLE_THREAD)
        // This is a case of shared zap config where mandatory thread attributes are enabled for a wifi platform (e.g
        // all-cluster-app) Real world product must only enable the attributes tied to the network technology supported by their
        // product. Temporarily encode a value of 0 reflecting no thread version when CHIP_DEVICE_CONFIG_ENABLE_THREAD or
        // ThreadNetworkInterface are not enabled until a solution is implemented with the attribute list.
        // Final implementation will return UnsupportedAttribute if we get here without the needed thread support
        // VerifyOrReturnError(mFeatureFlags.Has(Feature::kThreadNetworkInterface), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
        if (mFeatureFlags.Has(Feature::kThreadNetworkInterface))
        {
            threadVersion = mpDriver.Get<ThreadDriver *>()->GetThreadVersion();
        }
#endif
        return aEncoder.Encode(threadVersion);
    }
    break;
    default:
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::InterfaceEnabled::Id:
        bool value;
        ReturnErrorOnFailure(aDecoder.Decode(value));
        return mpBaseDriver->SetEnabled(value);
    default:
        return CHIP_IM_GLOBAL_STATUS(InvalidAction);
    }
}

void Instance::OnNetworkingStatusChange(Status aCommissioningError, Optional<ByteSpan> aNetworkId, Optional<int32_t> aConnectStatus)
{
    if (aNetworkId.HasValue())
    {
        if (aNetworkId.Value().size() > kMaxNetworkIDLen)
        {
            ChipLogError(DeviceLayer, "Overly large network ID received when calling OnNetworkingStatusChange");
        }
        else
        {
            SetLastNetworkId(aNetworkId.Value());
        }
    }

    SetLastNetworkingStatusValue(MakeNullable(aCommissioningError));
    if (aConnectStatus.HasValue())
    {
        SetLastConnectErrorValue(MakeNullable(aConnectStatus.Value()));
    }
    else
    {
        SetLastConnectErrorValue(NullNullable);
    }
}

void Instance::HandleScanNetworks(HandlerContext & ctx, const Commands::ScanNetworks::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleScanNetwork", "NetworkCommissioning");

    mScanningWasDirected = false;
    if (mFeatureFlags.Has(Feature::kWiFiNetworkInterface))
    {
        ByteSpan ssid;
        if (req.ssid.HasValue())
        {
            const auto & nullableSSID = req.ssid.Value();
            if (!nullableSSID.IsNull())
            {
                ssid = nullableSSID.Value();
                if (ssid.empty())
                {
                    // Normalize empty span value to null ByteSpan.
                    // Spec 7.17.1. Empty string is an equivalent of null.
                    ssid = ByteSpan();
                }
            }
        }
        if (ssid.size() > DeviceLayer::Internal::kMaxWiFiSSIDLength)
        {
            // Clients should never use too large a SSID.
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
            SetLastNetworkingStatusValue(MakeNullable(Status::kUnknownError));
            return;
        }

        mScanningWasDirected        = !ssid.empty();
        mCurrentOperationBreadcrumb = req.breadcrumb;
        mAsyncCommandHandle         = CommandHandler::Handle(&ctx.mCommandHandler);
        ctx.mCommandHandler.FlushAcksRightAwayOnSlowCommand();
        mpDriver.Get<WiFiDriver *>()->ScanNetworks(ssid, this);
    }
    else if (mFeatureFlags.Has(Feature::kThreadNetworkInterface))
    {
        // NOTE: the following lines were commented out due to issue #32875. In short, a popular
        // commissioner is passing a null SSID argument and this logic breaks interoperability as a result.
        // The spec has some inconsistency on this which also needs to be fixed. The commissioner maker is
        // fixing its code and will return to un-comment this code, with that work tracked by Issue #32887.
        //
        // SSID present on Thread violates the `[WI]` conformance.
        // if (req.ssid.HasValue())
        // {
        //     ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        //     return;
        // }

        mCurrentOperationBreadcrumb = req.breadcrumb;
        mAsyncCommandHandle         = CommandHandler::Handle(&ctx.mCommandHandler);
        ctx.mCommandHandler.FlushAcksRightAwayOnSlowCommand();
        mpDriver.Get<ThreadDriver *>()->ScanNetworks(this);
    }
    else
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCommand);
    }
}

namespace {
void FillDebugTextAndNetworkIndex(Commands::NetworkConfigResponse::Type & response, MutableCharSpan debugText, uint8_t networkIndex)
{
    if (!debugText.empty())
    {
        response.debugText.SetValue(debugText);
    }
    if (response.networkingStatus == Status::kSuccess)
    {
        response.networkIndex.SetValue(networkIndex);
    }
}

bool CheckFailSafeArmed(CommandHandlerInterface::HandlerContext & ctx)
{
    auto & failSafeContext = chip::Server::GetInstance().GetFailSafeContext();

    if (failSafeContext.IsFailSafeArmed(ctx.mCommandHandler.GetAccessingFabricIndex()))
    {
        return true;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::FailsafeRequired);
    return false;
}

} // namespace

void Instance::HandleAddOrUpdateWiFiNetwork(HandlerContext & ctx, const Commands::AddOrUpdateWiFiNetwork::DecodableType & req)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    MATTER_TRACE_SCOPE("HandleAddOrUpdateWiFiNetwork", "NetworkCommissioning");

    VerifyOrReturn(CheckFailSafeArmed(ctx));

    if (req.ssid.empty() || req.ssid.size() > DeviceLayer::Internal::kMaxWiFiSSIDLength)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError, "ssid");
        return;
    }

    // Presence of a Network Identity indicates we're configuring for Per-Device Credentials
    if (req.networkIdentity.HasValue())
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        if (mFeatureFlags.Has(Feature::kWiFiNetworkInterface))
        {
            HandleAddOrUpdateWiFiNetworkWithPDC(ctx, req);
            return;
        }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

    // Spec 11.8.8.4
    // Valid Credentials length are:
    // - 0 bytes: Unsecured (open) connection
    // - 5 bytes: WEP-64 passphrase
    // - 10 hexadecimal ASCII characters: WEP-64 40-bit hex raw PSK
    // - 13 bytes: WEP-128 passphrase
    // - 26 hexadecimal ASCII characters: WEP-128 104-bit hex raw PSK
    // - 8..63 bytes: WPA/WPA2/WPA3 passphrase
    // - 64 bytes: WPA/WPA2/WPA3 raw hex PSK
    // Note 10 hex WEP64 and 13 bytes / 26 hex WEP128 passphrase are covered by 8~63 bytes WPA passphrase, so we don't check WEP64
    // hex and WEP128 passphrase.
    if (req.credentials.size() == ValidWiFiCredentialLength::kOpen || req.credentials.size() == ValidWiFiCredentialLength::kWEP64 ||
        (req.credentials.size() >= ValidWiFiCredentialLength::kMinWPAPSK &&
         req.credentials.size() <= ValidWiFiCredentialLength::kMaxWPAPSK))
    {
        // Valid length, the credentials can have any characters.
    }
    else if (req.credentials.size() == ValidWiFiCredentialLength::kWPAPSKHex)
    {
        for (size_t d = 0; d < req.credentials.size(); d++)
        {
            if (!isxdigit(req.credentials.data()[d]))
            {
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
                return;
            }
        }
    }
    else
    {
        // Invalid length
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
        return;
    }

    Commands::NetworkConfigResponse::Type response;
    DebugTextStorage debugTextBuffer;
    MutableCharSpan debugText(debugTextBuffer);
    uint8_t outNetworkIndex = 0;
    response.networkingStatus =
        mpDriver.Get<WiFiDriver *>()->AddOrUpdateNetwork(req.ssid, req.credentials, debugText, outNetworkIndex);
    FillDebugTextAndNetworkIndex(response, debugText, outNetworkIndex);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    if (response.networkingStatus == Status::kSuccess)
    {
        UpdateBreadcrumb(req.breadcrumb);
        ReportNetworksListChanged();
    }
#endif
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
void Instance::HandleAddOrUpdateWiFiNetworkWithPDC(HandlerContext & ctx,
                                                   const Commands::AddOrUpdateWiFiNetwork::DecodableType & req)
{
    // Credentials must be empty when configuring for PDC, it's only present to keep the command shape compatible.
    if (!req.credentials.empty())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError, "credentials");
        return;
    }

    auto && networkIdentity = req.networkIdentity.Value(); // presence checked by caller
    if (networkIdentity.size() > kMaxCHIPCompactNetworkIdentityLength ||
        Credentials::ValidateChipNetworkIdentity(networkIdentity) != CHIP_NO_ERROR)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError, "networkIdentity");
        return;
    }

    if (req.clientIdentifier.HasValue() && req.clientIdentifier.Value().size() != CertificateKeyId::size())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError, "clientIdentifier");
        return;
    }

    bool provePossession = req.possessionNonce.HasValue();
    if (provePossession && req.possessionNonce.Value().size() != kPossessionNonceSize)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError, "possessionNonce");
        return;
    }

    auto err = CHIP_NO_ERROR;
    {
        auto driver = mpDriver.Get<WiFiDriver *>();

        // If the client is requesting re-use of a Client Identity, find the existing network it belongs to
        Optional<uint8_t> clientIdentityNetworkIndex;
        if (req.clientIdentifier.HasValue())
        {
            CertificateKeyId clientIdentifier(req.clientIdentifier.Value().data());
            uint8_t networkIndex = 0;
            EnumerateAndRelease(driver->GetNetworks(), [&](const Network & network) {
                if (network.clientIdentifier.HasValue() && clientIdentifier.data_equal(network.clientIdentifier.Value()))
                {
                    clientIdentityNetworkIndex.SetValue(networkIndex);
                    return Loop::Break;
                }
                networkIndex++;
                return Loop::Continue;
            });
            if (!clientIdentityNetworkIndex.HasValue())
            {
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::NotFound, "clientIdentifier");
                return;
            }
        }

        // Allocate a buffer to hold the client identity, and leave enough room to append the possession nonce if needed.
        chip::Platform::ScopedMemoryBuffer<uint8_t> identityBuffer;
        size_t identityBufferSize = kMaxCHIPCompactNetworkIdentityLength + (provePossession ? kPossessionNonceSize : 0);
        VerifyOrExit(identityBuffer.Alloc(identityBufferSize), /**/);

        // Add/Update the network at the driver level
        MutableByteSpan clientIdentity(identityBuffer.Get(), kMaxCHIPCompactNetworkIdentityLength);
        Optional<P256ECDSASignature> possessionSignature;
        Status status = Status::kUnknownError;
        DebugTextStorage debugTextBuffer;
        MutableCharSpan debugText(debugTextBuffer);
        uint8_t networkIndex;
        SuccessOrExit(err = driver->AddOrUpdateNetworkWithPDC(req.ssid, networkIdentity, clientIdentityNetworkIndex, status,
                                                              debugText, clientIdentity, networkIndex));

        Commands::NetworkConfigResponse::Type response;
        response.networkingStatus = status;
        FillDebugTextAndNetworkIndex(response, debugText, networkIndex);

        if (status == Status::kSuccess)
        {
            response.clientIdentity.SetValue(clientIdentity);

            if (provePossession)
            {
                // PossessionSignature TBS message = (NetworkClientIdentity || PossessionNonce)
                memcpy(clientIdentity.end(), req.possessionNonce.Value().data(), kPossessionNonceSize);
                ByteSpan tbsMessage(clientIdentity.data(), clientIdentity.size() + kPossessionNonceSize);
                SuccessOrExit(err = driver->SignWithClientIdentity(networkIndex, tbsMessage, possessionSignature.Emplace()));
                response.possessionSignature.SetValue(possessionSignature.Value().Span());
            }

            ReportNetworksListChanged();
            UpdateBreadcrumb(req.breadcrumb);
        }

        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AddOrUpdateWiFiNetwork with PDC failed: %" CHIP_ERROR_FORMAT, err.Format());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Failure);
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

void Instance::HandleAddOrUpdateThreadNetwork(HandlerContext & ctx, const Commands::AddOrUpdateThreadNetwork::DecodableType & req)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    MATTER_TRACE_SCOPE("HandleAddOrUpdateThreadNetwork", "NetworkCommissioning");

    VerifyOrReturn(CheckFailSafeArmed(ctx));

    Commands::NetworkConfigResponse::Type response;
    DebugTextStorage debugTextBuffer;
    MutableCharSpan debugText(debugTextBuffer);
    uint8_t outNetworkIndex = 0;
    response.networkingStatus =
        mpDriver.Get<ThreadDriver *>()->AddOrUpdateNetwork(req.operationalDataset, debugText, outNetworkIndex);
    FillDebugTextAndNetworkIndex(response, debugText, outNetworkIndex);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    if (response.networkingStatus == Status::kSuccess)
    {
        ReportNetworksListChanged();
        UpdateBreadcrumb(req.breadcrumb);
    }
#endif
}

void Instance::UpdateBreadcrumb(const Optional<uint64_t> & breadcrumb)
{
    VerifyOrReturn(breadcrumb.HasValue());
    GeneralCommissioning::SetBreadcrumb(breadcrumb.Value());
}

void Instance::CommitSavedBreadcrumb()
{
    // We rejected the command when there is another ongoing command, so mCurrentOperationBreadcrumb reflects the breadcrumb
    // argument in the only background command.
    UpdateBreadcrumb(mCurrentOperationBreadcrumb);
    mCurrentOperationBreadcrumb.ClearValue();
}

void Instance::HandleRemoveNetwork(HandlerContext & ctx, const Commands::RemoveNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleRemoveNetwork", "NetworkCommissioning");

    VerifyOrReturn(CheckFailSafeArmed(ctx));

    Commands::NetworkConfigResponse::Type response;
    DebugTextStorage debugTextBuffer;
    MutableCharSpan debugText(debugTextBuffer);
    uint8_t outNetworkIndex   = 0;
    response.networkingStatus = mpWirelessDriver->RemoveNetwork(req.networkID, debugText, outNetworkIndex);
    FillDebugTextAndNetworkIndex(response, debugText, outNetworkIndex);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    if (response.networkingStatus == Status::kSuccess)
    {
        ReportNetworksListChanged();
        UpdateBreadcrumb(req.breadcrumb);

        // If no networks are left, clear-out errors;
        if (CountAndRelease(mpBaseDriver->GetNetworks()) == 0)
        {
            SetLastNetworkId(ByteSpan{});
            SetLastConnectErrorValue(NullNullable);
            SetLastNetworkingStatusValue(NullNullable);
        }
    }
}

void Instance::HandleConnectNetwork(HandlerContext & ctx, const Commands::ConnectNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleConnectNetwork", "NetworkCommissioning");
    if (req.networkID.size() > kMaxNetworkIDLen)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError);
        return;
    }

    VerifyOrReturn(CheckFailSafeArmed(ctx));

    mConnectingNetworkIDLen = static_cast<uint8_t>(req.networkID.size());
    memcpy(mConnectingNetworkID, req.networkID.data(), mConnectingNetworkIDLen);
    mAsyncCommandHandle         = CommandHandler::Handle(&ctx.mCommandHandler);
    mCurrentOperationBreadcrumb = req.breadcrumb;

#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    // Per spec, lingering connections on any other interfaces need to be disconnected at this point.
    for (auto & node : sInstances)
    {
        Instance * instance = static_cast<Instance *>(&node);
        if (instance != this)
        {
            instance->DisconnectLingeringConnection();
        }
    }

    mpWirelessDriver->ConnectNetwork(req.networkID, this);
#else
    // In Non-concurrent mode postpone the final execution of ConnectNetwork until the operational
    // network has been fully brought up and kOperationalNetworkStarted is delivered.
    // mConnectingNetworkIDLen and mConnectingNetworkID contain the received SSID
    // As per spec, send the ConnectNetworkResponse(Success) prior to releasing the commissioning channel
    SendNonConcurrentConnectNetworkResponse();
#endif
}

void Instance::HandleNonConcurrentConnectNetwork()
{
    ByteSpan nonConcurrentNetworkID = ByteSpan(mConnectingNetworkID, mConnectingNetworkIDLen);
    ChipLogProgress(NetworkProvisioning, "Non-concurrent mode, Connect to Network SSID=%.*s", mConnectingNetworkIDLen,
                    mConnectingNetworkID);
    mpWirelessDriver->ConnectNetwork(nonConcurrentNetworkID, this);
}

void Instance::HandleReorderNetwork(HandlerContext & ctx, const Commands::ReorderNetwork::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleReorderNetwork", "NetworkCommissioning");
    Commands::NetworkConfigResponse::Type response;
    DebugTextStorage debugTextBuffer;
    MutableCharSpan debugText(debugTextBuffer);
    response.networkingStatus = mpWirelessDriver->ReorderNetwork(req.networkID, req.networkIndex, debugText);
    FillDebugTextAndNetworkIndex(response, debugText, req.networkIndex);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    if (response.networkingStatus == Status::kSuccess)
    {
        ReportNetworksListChanged();
        UpdateBreadcrumb(req.breadcrumb);
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
void Instance::HandleQueryIdentity(HandlerContext & ctx, const Commands::QueryIdentity::DecodableType & req)
{
    MATTER_TRACE_SCOPE("HandleQueryIdentity", "NetworkCommissioning");

    if (req.keyIdentifier.size() != CertificateKeyId::size())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError, "keyIdentifier");
        return;
    }
    CertificateKeyId keyIdentifier(req.keyIdentifier.data());

    bool provePossession = req.possessionNonce.HasValue();
    if (provePossession && req.possessionNonce.Value().size() != kPossessionNonceSize)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::ConstraintError, "possessionNonce");
        return;
    }

    auto err      = CHIP_NO_ERROR;
    auto status   = Protocols::InteractionModel::Status::Success;
    auto driver   = mpDriver.Get<WiFiDriver *>();
    auto networks = driver->GetNetworks();
    VerifyOrExit(networks != nullptr && networks->Count() > 0, status = Protocols::InteractionModel::Status::NotFound);

    {
        // Allocate a buffer to hold the identity, and leave enough room to append the possession nonce if needed.
        chip::Platform::ScopedMemoryBuffer<uint8_t> identityBuffer;
        size_t identityBufferSize = kMaxCHIPCompactNetworkIdentityLength + (provePossession ? kPossessionNonceSize : 0);
        VerifyOrExit(identityBuffer.Alloc(identityBufferSize), /**/);

        MutableByteSpan identity(identityBuffer.Get(), kMaxCHIPCompactNetworkIdentityLength);
        Optional<P256ECDSASignature> possessionSignature;

        Network network;
        for (uint8_t networkIndex = 0;; networkIndex++)
        {
            VerifyOrExit(networks->Next(network), status = Protocols::InteractionModel::Status::NotFound);

            if (network.clientIdentifier.HasValue() && keyIdentifier.data_equal(network.clientIdentifier.Value()))
            {
                SuccessOrExit(err = driver->GetClientIdentity(networkIndex, identity));
                if (provePossession)
                {
                    // PossessionSignature TBS message = (NetworkClientIdentity || PossessionNonce)
                    memcpy(identity.end(), req.possessionNonce.Value().data(), kPossessionNonceSize);
                    ByteSpan tbsMessage(identity.data(), identity.size() + kPossessionNonceSize);
                    SuccessOrExit(err = driver->SignWithClientIdentity(networkIndex, tbsMessage, possessionSignature.Emplace()));
                }
                break;
            }
            if (!provePossession && // Proof-of-possession is not possible for network identities
                network.networkIdentifier.HasValue() && keyIdentifier.data_equal(network.networkIdentifier.Value()))
            {
                SuccessOrExit(err = driver->GetNetworkIdentity(networkIndex, identity));
                break;
            }
        }

        Commands::QueryIdentityResponse::Type response;
        response.identity = identity;
        if (possessionSignature.HasValue())
        {
            response.possessionSignature.SetValue(possessionSignature.Value().Span());
        }
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }

exit:
    if (networks != nullptr)
    {
        networks->Release();
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "QueryIdentity failed: %" CHIP_ERROR_FORMAT, err.Format());
        status = Protocols::InteractionModel::Status::Failure;
    }
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
void Instance::DisconnectLingeringConnection()
{
    bool haveConnectedNetwork = false;
    EnumerateAndRelease(mpBaseDriver->GetNetworks(), [&](const Network & network) {
        if (network.connected)
        {
            haveConnectedNetwork = true;
            return Loop::Break;
        }
        return Loop::Continue;
    });

    // If none of the configured networks is `connected`, we may have a
    // lingering connection to a different network that we need to disconnect.
    // Note: The driver may or may not be actually connected
    if (!haveConnectedNetwork)
    {
        LogErrorOnFailure(mpWirelessDriver->DisconnectFromNetwork());
    }
}
#endif

void Instance::OnResult(Status commissioningError, CharSpan debugText, int32_t interfaceStatus)
{
    auto commandHandleRef = std::move(mAsyncCommandHandle);

    // In Non-concurrent mode the commandHandle will be null here, the ConnectNetworkResponse
    // has already been sent and the BLE will have been stopped, however the other functionality
    // is still required
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    auto commandHandle = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        // When the platform shutted down, interaction model engine will invalidate all commandHandle to avoid dangling references.
        // We may receive the callback after it and should make it noop.
        return;
    }
#endif // CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION

    Commands::ConnectNetworkResponse::Type response;
    response.networkingStatus = commissioningError;
    if (!debugText.empty())
    {
        response.debugText.SetValue(debugText);
    }
    if (commissioningError == Status::kSuccess)
    {
        DeviceLayer::DeviceControlServer::DeviceControlSvr().PostConnectedToOperationalNetworkEvent(
            ByteSpan(mLastNetworkID, mLastNetworkIDLen));
        SetLastConnectErrorValue(NullNullable);
    }
    else
    {
        response.errorValue.SetNonNull(interfaceStatus);
        SetLastConnectErrorValue(MakeNullable(interfaceStatus));
    }

    SetLastNetworkId(ByteSpan{ mConnectingNetworkID, mConnectingNetworkIDLen });
    SetLastNetworkingStatusValue(MakeNullable(commissioningError));

#if CONFIG_NETWORK_LAYER_BLE && !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    ChipLogProgress(NetworkProvisioning, "Non-concurrent mode, ConnectNetworkResponse will NOT be sent");
    // Do not send the ConnectNetworkResponse if in non-concurrent mode
    // TODO(#30576) raised to modify CommandHandler to notify it if no response required
    // -----> Is this required here: commandHandle->FinishCommand();
#else
    commandHandle->AddResponse(mPath, response);
#endif // CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION

    if (commissioningError == Status::kSuccess)
    {
        CommitSavedBreadcrumb();
    }
}

void Instance::OnFinished(Status status, CharSpan debugText, ThreadScanResponseIterator * networks)
{
    DEFER_AUTO_RELEASE(networks);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        // When the platform shutted down, interaction model engine will invalidate all commandHandle to avoid dangling references.
        // We may receive the callback after it and should make it noop.
        return;
    }

    SetLastNetworkingStatusValue(MakeNullable(status));

    ThreadScanResponseToTLV responseBuilder(status, debugText, networks);
    commandHandle->AddResponse(mPath, Commands::ScanNetworksResponse::Id, responseBuilder);

    if (status == Status::kSuccess)
    {
        CommitSavedBreadcrumb();
    }
#endif
}

void Instance::OnFinished(Status status, CharSpan debugText, WiFiScanResponseIterator * networks)
{
    DEFER_AUTO_RELEASE(networks);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        // When the platform shutted down, interaction model engine will invalidate all commandHandle to avoid dangling references.
        // We may receive the callback after it and should make it noop.
        return;
    }

    // If drivers are failing to respond NetworkNotFound on empty results, force it for them.
    bool resultsMissing = !networks || (networks->Count() == 0);
    if ((status == Status::kSuccess) && mScanningWasDirected && resultsMissing)
    {
        status = Status::kNetworkNotFound;
    }

    SetLastNetworkingStatusValue(MakeNullable(status));

    WifiScanResponseToTLV responseBuilder(status, debugText, networks);
    commandHandle->AddResponse(mPath, Commands::ScanNetworksResponse::Id, responseBuilder);

    if (status == Status::kSuccess)
    {
        CommitSavedBreadcrumb();
    }
#endif
}

void Instance::OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    Instance * this_ = reinterpret_cast<Instance *>(arg);

    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        this_->OnCommissioningComplete();
    }
    else if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        this_->OnFailSafeTimerExpired();
    }
    else if ((event->Type == DeviceLayer::DeviceEventType::kWiFiDeviceAvailable) ||
             (event->Type == DeviceLayer::DeviceEventType::kOperationalNetworkStarted))

    {
        // In Non-Concurrent mode connect the operational channel, as BLE has been stopped
        this_->HandleNonConcurrentConnectNetwork();
    }
}

void Instance::OnCommissioningComplete()
{
    VerifyOrReturn(mpWirelessDriver != nullptr);

    ChipLogDetail(Zcl, "Commissioning complete, notify platform driver to persist network credentials.");
    mpWirelessDriver->CommitConfiguration();
}

void Instance::OnFailSafeTimerExpired()
{
    VerifyOrReturn(mpWirelessDriver != nullptr);

    ChipLogDetail(Zcl, "Failsafe timeout, tell platform driver to revert network credentials.");
    mpWirelessDriver->RevertConfiguration();
    mAsyncCommandHandle.Release();

    // Mark the network list changed since `mpWirelessDriver->RevertConfiguration()` may have updated it.
    ReportNetworksListChanged();

    // If no networks are left, clear-out errors;
    if (mpBaseDriver && (CountAndRelease(mpBaseDriver->GetNetworks()) == 0))
    {
        SetLastNetworkId(ByteSpan{});
        SetLastConnectErrorValue(NullNullable);
        SetLastNetworkingStatusValue(NullNullable);
    }
}

CHIP_ERROR Instance::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{
    using namespace Clusters::NetworkCommissioning::Commands;

    if (mFeatureFlags.Has(Feature::kThreadNetworkInterface))
    {
        for (auto && cmd : {
                 ScanNetworks::Id,
                 AddOrUpdateThreadNetwork::Id,
                 RemoveNetwork::Id,
                 ConnectNetwork::Id,
                 ReorderNetwork::Id,
             })
        {
            VerifyOrExit(callback(cmd, context) == Loop::Continue, /**/);
        }
    }
    else if (mFeatureFlags.Has(Feature::kWiFiNetworkInterface))
    {
        for (auto && cmd : {
                 ScanNetworks::Id,
                 AddOrUpdateWiFiNetwork::Id,
                 RemoveNetwork::Id,
                 ConnectNetwork::Id,
                 ReorderNetwork::Id,
             })
        {
            VerifyOrExit(callback(cmd, context) == Loop::Continue, /**/);
        }
    }

    if (mFeatureFlags.Has(Feature::kPerDeviceCredentials))
    {
        VerifyOrExit(callback(QueryIdentity::Id, context) == Loop::Continue, /**/);
    }

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{
    using namespace Clusters::NetworkCommissioning::Commands;

    if (mFeatureFlags.HasAny(Feature::kWiFiNetworkInterface, Feature::kThreadNetworkInterface))
    {
        for (auto && cmd : { ScanNetworksResponse::Id, NetworkConfigResponse::Id, ConnectNetworkResponse::Id })
        {
            VerifyOrExit(callback(cmd, context) == Loop::Continue, /**/);
        }
    }

    if (mFeatureFlags.Has(Feature::kPerDeviceCredentials))
    {
        VerifyOrExit(callback(QueryIdentityResponse::Id, context) == Loop::Continue, /**/);
    }

exit:
    return CHIP_NO_ERROR;
}

bool NullNetworkDriver::GetEnabled()
{
    // Disable the interface and it cannot be enabled since there are no physical interfaces.
    return false;
}

uint8_t NullNetworkDriver::GetMaxNetworks()
{
    // The minimal value of MaxNetworks should be 1 per spec.
    return 1;
}

NetworkIterator * NullNetworkDriver::GetNetworks()
{
    // Instance::Read accepts nullptr as an empty NetworkIterator.
    return nullptr;
}

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterNetworkCommissioningPluginServerInitCallback()
{
    // Nothing to do, the server init routine will be done in Instance::Init()
}
