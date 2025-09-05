/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "app/clusters/ota-requestor/OTARequestorInterface.h"
#include "app/icd/server/ICDConfigurationData.h"
#include "app/icd/server/ICDNotifier.h"
#include "app/server/CommissioningWindowManager.h"
#include "app/server/Server.h"
#include "credentials/FabricTable.h"
#include "device_service/device_service.rpc.pb.h"
#include "platform/CommissionableDataProvider.h"
#include "platform/ConfigurationManager.h"
#include "platform/DiagnosticDataProvider.h"
#include "platform/PlatformManager.h"
#include "setup_payload/OnboardingCodesUtil.h"
#include <crypto/CHIPCryptoPAL.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

namespace chip {
namespace rpc {

namespace Internal {
// This class supports changing the commissionable data provider values at
// runtime using the RPCs.
// This class is LazyInit after getting or setting any of it's values. After
// this the class wraps the original CommissionableDataProvider, returning the
// original values for anything which has not been overwritten.
//
// NOTE: Values written do not persist across a reboot.
class CommissionableDataProviderRpcWrapper : public DeviceLayer::CommissionableDataProvider
{
public:
    CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) override
    {
        LazyInit();
        if (mPasscodeOverride.has_value())
        {
            setupPasscode = mPasscodeOverride.value();
            return CHIP_NO_ERROR;
        }
        if (mCommissionableDataProvider)
        {
            return mCommissionableDataProvider->GetSetupPasscode(setupPasscode);
        }
        return CHIP_ERROR_INTERNAL;
    }

    // NOTE: Changing the passcode will not change the verifier or anything else
    // this just changes the value returned from GetSetupPasscode.
    // Using this is completely optional, and only really useful for test
    // automation which can read the configured passcode for commissioning
    // after it is changed.
    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override
    {
        LazyInit();
        mPasscodeOverride = setupPasscode;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override
    {
        LazyInit();
        if (mDiscriminatorOverride.has_value())
        {
            setupDiscriminator = mDiscriminatorOverride.value();
            return CHIP_NO_ERROR;
        }
        if (mCommissionableDataProvider)
        {
            return mCommissionableDataProvider->GetSetupDiscriminator(setupDiscriminator);
        }
        return CHIP_ERROR_INTERNAL;
    }

    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override
    {
        LazyInit();
        mDiscriminatorOverride = setupDiscriminator;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override
    {
        LazyInit();
        if (mIterationCountOverride.has_value())
        {
            iterationCount = mIterationCountOverride.value();
            return CHIP_NO_ERROR;
        }
        if (mCommissionableDataProvider)
        {
            return mCommissionableDataProvider->GetSpake2pIterationCount(iterationCount);
        }
        return CHIP_ERROR_INTERNAL;
    }

    CHIP_ERROR SetSpake2pIterationCount(uint32_t iterationCount)
    {
        LazyInit();
        mIterationCountOverride = iterationCount;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & saltBuf) override
    {
        LazyInit();
        if (mSaltOverride.has_value())
        {
            if (mSaltOverride.value().size() > saltBuf.size())
            {
                return CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            std::copy(mSaltOverride.value().begin(), mSaltOverride.value().end(), saltBuf.begin());
            saltBuf.reduce_size(mSaltOverride.value().size());
            return CHIP_NO_ERROR;
        }
        if (mCommissionableDataProvider)
        {
            return mCommissionableDataProvider->GetSpake2pSalt(saltBuf);
        }
        return CHIP_ERROR_INTERNAL;
    }

    CHIP_ERROR SetSpake2pSalt(ByteSpan saltBuf)
    {
        LazyInit();
        if (sizeof(mSaltBuf) < saltBuf.size())
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        std::copy(saltBuf.begin(), saltBuf.end(), mSaltBuf);
        mSaltOverride = ByteSpan(mSaltBuf, saltBuf.size());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & outVerifierLen) override
    {
        LazyInit();
        if (mVerifierOverride.has_value())
        {
            outVerifierLen = mVerifierOverride.value().size();
            if (mVerifierOverride.value().size() > verifierBuf.size())
            {
                return CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            std::copy(mVerifierOverride.value().begin(), mVerifierOverride.value().end(), verifierBuf.begin());
            verifierBuf.reduce_size(mVerifierOverride.value().size());
            return CHIP_NO_ERROR;
        }
        if (mCommissionableDataProvider)
        {
            return mCommissionableDataProvider->GetSpake2pVerifier(verifierBuf, outVerifierLen);
        }

        return CHIP_ERROR_INTERNAL;
    }

    CHIP_ERROR SetSpake2pVerifier(ByteSpan verifierBuf)
    {
        LazyInit();
        if (sizeof(mVerifierBuf) < verifierBuf.size())
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        std::copy(verifierBuf.begin(), verifierBuf.end(), mVerifierBuf);
        mVerifierOverride = ByteSpan(mVerifierBuf, verifierBuf.size());
        return CHIP_NO_ERROR;
    }

private:
    std::optional<uint16_t> mDiscriminatorOverride;
    std::optional<uint32_t> mPasscodeOverride;
    Crypto::Spake2pVerifierSerialized mVerifierBuf;
    std::optional<ByteSpan> mVerifierOverride;
    uint8_t mSaltBuf[Crypto::kSpake2p_Max_PBKDF_Salt_Length];
    std::optional<ByteSpan> mSaltOverride;
    std::optional<uint32_t> mIterationCountOverride;
    DeviceLayer::CommissionableDataProvider * mCommissionableDataProvider = nullptr;

    void LazyInit()
    {
        if (!mCommissionableDataProvider)
        {
            mCommissionableDataProvider = DeviceLayer::GetCommissionableDataProvider();
            DeviceLayer::SetCommissionableDataProvider(this);
        }
    }
};
} // namespace Internal

class Device : public pw_rpc::nanopb::Device::Service<Device>
{
public:
    virtual ~Device() = default;

    virtual pw::Status FactoryReset(const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        DeviceLayer::ConfigurationMgr().InitiateFactoryReset();
        return pw::OkStatus();
    }

    virtual pw::Status Reboot(const chip_rpc_RebootRequest & request, pw_protobuf_Empty & response)
    {
        return pw::Status::Unimplemented();
    }

    virtual pw::Status TriggerOta(const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t) {
                chip::OTARequestorInterface * requestor = chip::GetRequestorInstance();
                if (requestor == nullptr)
                {
                    ChipLogError(SoftwareUpdate, "Can't get the CASESessionManager");
                }
                else
                {
                    requestor->TriggerImmediateQuery();
                }
            },
            reinterpret_cast<intptr_t>(nullptr));
        return pw::OkStatus();
#else  // CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        ChipLogError(AppServer, "Trigger OTA requested, but OTA requestor not compiled in.");
        return pw::Status::Unimplemented();
#endif // CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    }

    virtual pw::Status SetOtaMetadataForProvider(const chip_rpc_MetadataForProvider & request, pw_protobuf_Empty & response)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        chip::OTARequestorInterface * requestor = chip::GetRequestorInstance();
        if (requestor == nullptr)
        {
            ChipLogError(SoftwareUpdate, "Can't get the CASESessionManager");
            return pw::Status::Unavailable();
        }
        else if (sizeof(metadataForProviderBuffer) < request.tlv.size)
        {
            return pw::Status::ResourceExhausted();
        }
        memcpy(metadataForProviderBuffer, request.tlv.bytes, request.tlv.size);
        DeviceLayer::StackLock lock;
        requestor->SetMetadataForProvider(chip::ByteSpan(metadataForProviderBuffer, request.tlv.size));
        return pw::OkStatus();
#else  // CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        ChipLogError(AppServer, "OTA set metadata for provider requested, but OTA requestor not compiled in.");
        return pw::Status::Unimplemented();
#endif // CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    }

    virtual pw::Status SetPairingState(const chip_rpc_PairingState & request, pw_protobuf_Empty & response)
    {
        if (request.pairing_enabled && !chip::Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen())
        {
            DeviceLayer::StackLock lock;
            chip::ChipError err = chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified, "RPC SetPairingState failed to open commissioning window: %" CHIP_ERROR_FORMAT,
                             err.Format());
                return pw::Status::Internal();
            }
        }
        else if (!request.pairing_enabled &&
                 chip::Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen())
        {
            DeviceLayer::StackLock lock;
            chip::Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
        }
        return pw::OkStatus();
    }

    virtual pw::Status GetPairingState(const pw_protobuf_Empty & request, chip_rpc_PairingState & response)
    {
        response.pairing_enabled = DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled();
        return pw::OkStatus();
    }

    virtual pw::Status GetDeviceState(const pw_protobuf_Empty & request, chip_rpc_DeviceState & response)
    {
        uint64_t time_since_boot_sec;
        DeviceLayer::GetDiagnosticDataProvider().GetUpTime(time_since_boot_sec);
        response.time_since_boot_millis = time_since_boot_sec * 1000;
        size_t count                    = 0;
        DeviceLayer::StackLock lock;
        for (const FabricInfo & fabricInfo : Server::GetInstance().GetFabricTable())
        {
            if (count < MATTER_ARRAY_SIZE(response.fabric_info))
            {
                response.fabric_info[count].fabric_id = fabricInfo.GetFabricId();
                response.fabric_info[count].node_id   = fabricInfo.GetPeerId().GetNodeId();
                count++;
            }
        }
        response.fabric_info_count = count;
        return pw::OkStatus();
    }

    virtual pw::Status GetDeviceInfo(const pw_protobuf_Empty & request, chip_rpc_DeviceInfo & response)
    {

        uint16_t vendor_id;
        if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(vendor_id) == CHIP_NO_ERROR)
        {
            response.vendor_id = static_cast<uint32_t>(vendor_id);
        }
        else
        {
            return pw::Status::Internal();
        }

        uint16_t product_id;
        if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(product_id) == CHIP_NO_ERROR)
        {
            response.product_id = static_cast<uint32_t>(product_id);
        }
        else
        {
            return pw::Status::Internal();
        }

        uint32_t software_version;
        if (DeviceLayer::ConfigurationMgr().GetSoftwareVersion(software_version) == CHIP_NO_ERROR)
        {
            response.software_version = software_version;
        }
        else
        {
            return pw::Status::Internal();
        }

        if (DeviceLayer::ConfigurationMgr().GetSoftwareVersionString(response.software_version_string,
                                                                     sizeof(response.software_version_string)) != CHIP_NO_ERROR)
        {
            return pw::Status::Internal();
        }

        uint32_t code;
        if (DeviceLayer::GetCommissionableDataProvider()->GetSetupPasscode(code) == CHIP_NO_ERROR)
        {
            response.pairing_info.code = code;
            response.has_pairing_info  = true;
        }

        uint16_t discriminator;
        if (DeviceLayer::GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator) == CHIP_NO_ERROR)
        {
            response.pairing_info.discriminator = static_cast<uint32_t>(discriminator);
            response.has_pairing_info           = true;
        }

        if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetSerialNumber(response.serial_number, sizeof(response.serial_number)) !=
            CHIP_NO_ERROR)
        {
            response.serial_number[0] = '\0'; // optional serial field not set.
        }

        // Create buffer for QR code that can fit max size and null terminator.
        char qrCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
        chip::MutableCharSpan qrCodeText(qrCodeBuffer);
        if (GetQRCode(qrCodeText, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE)) == CHIP_NO_ERROR)
        {
            snprintf(response.pairing_info.qr_code, sizeof(response.pairing_info.qr_code), "%s", qrCodeText.data());
            GetQRCodeUrl(response.pairing_info.qr_code_url, sizeof(response.pairing_info.qr_code_url), qrCodeText);
            response.has_pairing_info = true;
        }

        return pw::OkStatus();
    }

    virtual pw::Status GetSpakeInfo(const pw_protobuf_Empty & request, chip_rpc_SpakeInfo & response)
    {
        size_t serializedVerifierLen = 0;
        MutableByteSpan verifierSpan{ response.verifier.bytes };
        if (DeviceLayer::GetCommissionableDataProvider()->GetSpake2pVerifier(verifierSpan, serializedVerifierLen) == CHIP_NO_ERROR)
        {
            response.verifier.size = verifierSpan.size();
            response.has_verifier  = true;
        }

        MutableByteSpan saltSpan{ response.salt.bytes };
        if (DeviceLayer::GetCommissionableDataProvider()->GetSpake2pSalt(saltSpan) == CHIP_NO_ERROR)
        {
            response.salt.size = saltSpan.size();
            response.has_salt  = true;
        }

        if (DeviceLayer::GetCommissionableDataProvider()->GetSpake2pIterationCount(response.iteration_count) == CHIP_NO_ERROR)
        {
            response.has_iteration_count = true;
        }

        return pw::OkStatus();
    }

    virtual pw::Status SetSpakeInfo(const chip_rpc_SpakeInfo & request, pw_protobuf_Empty & response)
    {
        if (request.has_salt)
        {
            mCommissionableDataProvider.SetSpake2pSalt(ByteSpan(request.salt.bytes, request.salt.size));
        }
        if (request.has_iteration_count)
        {
            mCommissionableDataProvider.SetSpake2pIterationCount(request.iteration_count);
        }
        if (request.has_verifier)
        {
            mCommissionableDataProvider.SetSpake2pVerifier(ByteSpan(request.verifier.bytes, request.verifier.size));
        }

        if (Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen() &&
            Server::GetInstance().GetCommissioningWindowManager().CommissioningWindowStatusForCluster() !=
                app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kEnhancedWindowOpen)
        {
            // Cache values before closing to restore them after restart.
            app::DataModel::Nullable<VendorId> vendorId = Server::GetInstance().GetCommissioningWindowManager().GetOpenerVendorId();
            app::DataModel::Nullable<FabricIndex> fabricIndex =
                Server::GetInstance().GetCommissioningWindowManager().GetOpenerFabricIndex();

            // Restart commissioning window to recache the spakeInfo values:
            {
                DeviceLayer::StackLock lock;
                Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
            }
            // Let other tasks possibly work since Commissioning window close/open are "heavy"
            if (Server::GetInstance().GetCommissioningWindowManager().CommissioningWindowStatusForCluster() !=
                    app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen &&
                !vendorId.IsNull() && !fabricIndex.IsNull())
            {
                DeviceLayer::StackLock lock;
                System::Clock::Seconds16 commissioningTimeout =
                    System::Clock::Seconds16(CHIP_DEVICE_CONFIG_DISCOVERY_TIMEOUT_SECS); // Use default for timeout for now.
                Server::GetInstance()
                    .GetCommissioningWindowManager()
                    .OpenBasicCommissioningWindowForAdministratorCommissioningCluster(commissioningTimeout, fabricIndex.Value(),
                                                                                      vendorId.Value());
            }
            else
            {
                DeviceLayer::StackLock lock;
                Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
            }
        }

        return pw::OkStatus();
    }

    // NOTE: Changing the passcode will not change the verifier or anything else
    // this just changes the value returned from GetSetupPasscode.
    // Using this is completely optional, and only really useful for test
    // automation which can read the configured passcode for commissioning
    // after it is changed.
    virtual pw::Status SetPairingInfo(const chip_rpc_PairingInfo & request, pw_protobuf_Empty & response)
    {
        if (mCommissionableDataProvider.SetSetupPasscode(request.code) != CHIP_NO_ERROR ||
            mCommissionableDataProvider.SetSetupDiscriminator(request.discriminator) != CHIP_NO_ERROR)
        {
            return pw::Status::Unknown();
        }
        return pw::OkStatus();
    }

    /**
     * @brief RPC to get the current ICD mode (SIT or LIT).
     *
     * This function is only available when CONFIG_CHIP_ICD_DSLS_SUPPORT is enabled.
     * It retrieves the current operating mode of the Intermittently Connected Device
     * from ICDConfigurationData and populates the response.
     *
     * @param request The empty RPC request message.
     * @param response The RPC response message to be populated with the current ICD mode.
     * @return pw::OkStatus on success, pw::Status::Unimplemented if the feature is disabled.
     */
    virtual pw::Status GetIcdMode(const pw_protobuf_Empty & request, chip_rpc_IcdDeviceMode & response)
    {
#if CONFIG_CHIP_ICD_DSLS_SUPPORT
        if (ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::LIT)
            response.icd_mode = chip_rpc_IcdMode_LIT_ICD;
        else
            response.icd_mode = chip_rpc_IcdMode_SIT_ICD;

        ChipLogDetail(AppServer, "GetIcdMode : %d", response.icd_mode);
        return pw::OkStatus();
#else  // CONFIG_CHIP_ICD_DSLS_SUPPORT
        ChipLogError(AppServer, "GetIcdMode is not supported");
        return pw::Status::Unimplemented();
#endif // CONFIG_CHIP_ICD_DSLS_SUPPORT
    }

    /**
     * @brief RPC to get the current ICD slow poll interval.
     *
     * This function is only available when CHIP_CONFIG_ENABLE_ICD_SERVER is enabled.
     * It retrieves the current slow poll interval of the Intermittently Connected Device
     * from ICDConfigurationData and populates the response.
     *
     * @param request The empty RPC request message.
     * @param response The RPC response message to be populated with the current slow poll interval in milliseconds.
     * @return pw::OkStatus on success, pw::Status::Unimplemented if the feature is disabled.
     */
    virtual pw::Status GetIcdSlowPollInterval(const pw_protobuf_Empty & request, chip_rpc_IcdSlowPollInterval & response)
    {
#if CHIP_CONFIG_ENABLE_ICD_SERVER
        response.slow_poll_interval_ms = ICDConfigurationData::GetInstance().GetSlowPollingInterval().count();
        ChipLogDetail(AppServer, "GetIcdSlowPollInterval : %u", response.slow_poll_interval_ms);
        return pw::OkStatus();
#else  // CHIP_CONFIG_ENABLE_ICD_SERVER
        ChipLogError(AppServer, "GetIcdSlowPollInterval is not supported");
        return pw::Status::Unimplemented();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
    }

    /**
     * @brief RPC to set the ICD slow poll interval.
     *
     * This function is only available when CHIP_CONFIG_ENABLE_ICD_SERVER is enabled.
     * It sets the slow poll interval for the Intermittently Connected Device.
     * The new interval is passed to ICDConfigurationData for validation and application.
     *
     * @param request The RPC request message containing the new slow poll interval in milliseconds.
     * @param response The empty RPC response message.
     * @return pw::OkStatus on success, pw::Status::InvalidArgument if the interval is invalid,
     *         or pw::Status::Unimplemented if the feature is disabled.
     */
    virtual pw::Status SetIcdSlowPollInterval(const chip_rpc_IcdSlowPollInterval & request, pw_protobuf_Empty & response)
    {
#if CHIP_CONFIG_ENABLE_ICD_SERVER
        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t arg) {
                uint32_t interval_ms = static_cast<uint32_t>(arg);
                ChipLogDetail(AppServer, "SetIcdSlowPollInterval : %u", interval_ms);
//                ICDConfigurationData::GetInstance().SetSlowPollingInterval(System::Clock::Milliseconds32(interval_ms));
            },
            static_cast<intptr_t>(request.slow_poll_interval_ms));
        return pw::OkStatus();
#else  // CHIP_CONFIG_ENABLE_ICD_SERVER
        ChipLogError(AppServer, "SetIcdSlowPollInterval is not supported");
        return pw::Status::Unimplemented();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
    }
    
    /**
     * @brief RPC to set the ICD mode to SIT or LIT.
     *
     * This function is only available when CONFIG_CHIP_ICD_DSLS_SUPPORT is enabled.
     * It schedules a work item to request a change in the ICD operating mode
     * based on the current ICD mode. If current is LIT mode will force the device
     * into SIT mode, while current being SIT mode will withdraw the SIT request,
     * allowing the device to enter LIT mode if other conditions are met.
     *
     * @param request The RPC request message containing the desired ICD mode.
     * @param response The empty RPC response message.
     * @return pw::OkStatus on success, pw::Status::Unimplemented if the feature is disabled.
     */
    virtual pw::Status SetIcdMode(const chip_rpc_IcdDeviceMode & request, pw_protobuf_Empty & response)
    {
#if CONFIG_CHIP_ICD_DSLS_SUPPORT
        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t) {
                if (ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::LIT) {
                    ChipLogDetail(AppServer, "SetIcdMode from LIT to SIT");
                    chip::app::ICDNotifier::GetInstance().NotifySITModeRequestNotification();
                } else {
                    ChipLogDetail(AppServer, "SetIcdMode from SIT to LIT");
                    chip::app::ICDNotifier::GetInstance().NotifySITModeRequestWithdrawal();
                }
            },
            reinterpret_cast<intptr_t>(nullptr));
        return pw::OkStatus();
#else  // CONFIG_CHIP_ICD_DSLS_SUPPORT
        ChipLogError(AppServer, "SetIcdMode is not supported");
        return pw::Status::Unimplemented();
#endif // CONFIG_CHIP_ICD_DSLS_SUPPORT
    }


    virtual pw::Status ShutdownAllSubscriptions(const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
#if CHIP_CONFIG_ENABLE_ICD_CIP
        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t) {
                chip::app::InteractionModelEngine::GetInstance()->ShutdownAllSubscriptionHandlers();
                ChipLogDetail(AppServer, "Being triggered to shutdown all subscriptions in server side");
            },
            reinterpret_cast<intptr_t>(nullptr));
        return pw::OkStatus();
#else  // CHIP_CONFIG_ENABLE_ICD_CIP
        ChipLogError(AppServer, "ShutdownAllSubscriptions is not supported");
        return pw::Status::Unimplemented();
#endif // CHIP_CONFIG_ENABLE_ICD_CIP
    }

    virtual pw::Status TriggerIcdCheckin(const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
#if CHIP_CONFIG_ENABLE_ICD_CIP
        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t) {
                ChipLogDetail(AppServer, "Being triggered to send ICD check-in message to subscriber");
                chip::app::ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
            },
            reinterpret_cast<intptr_t>(nullptr));
        return pw::OkStatus();
#else  // CHIP_CONFIG_ENABLE_ICD_CIP
        ChipLogError(AppServer, "TriggerIcdCheckin is not supported");
        return pw::Status::Unimplemented();
#endif // CHIP_CONFIG_ENABLE_ICD_CIP
    }

private:
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    static constexpr size_t kMaxMetadataForProviderLength = 512; // length defined in chip spec 11.20.6.7
    uint8_t metadataForProviderBuffer[kMaxMetadataForProviderLength];
#endif // CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    Internal::CommissionableDataProviderRpcWrapper mCommissionableDataProvider;
};

} // namespace rpc
} // namespace chip
