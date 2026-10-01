#pragma once

#include <app/clusters/door-lock-server/CodegenIntegration.h>
#include <lib/core/DataModelTypes.h>

class LockEndpoint;

namespace LockApp {

/**
 * DoorLock::Delegate forwarding to the per-endpoint LockEndpoint managed by
 * LockManager.
 *
 * The cluster instance is constructed before the application endpoint data
 * (LockEndpoint) exists, so the forwarder resolves the LockEndpoint at call
 * time through LockManager. Platform glue installs the forwarder with
 * DoorLock::SetDelegate before the endpoint starts, and
 * LockManager::InitEndpoint makes it available.
 *
 * When no endpoint is available yet the answers are the neutral "no reader
 * configuration" defaults.
 */
class LockAppDoorLockDelegate : public chip::app::Clusters::DoorLock::Delegate
{
public:
    explicit LockAppDoorLockDelegate(chip::EndpointId endpointId) : mEndpointId(endpointId) {}

    bool HandleDoorLockCommand(chip::EndpointId endpointId, const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                               const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                               const chip::Optional<chip::ByteSpan> & pinCode,
                               chip::app::Clusters::DoorLock::OperationErrorEnum & err) override;
    bool HandleDoorUnlockCommand(chip::EndpointId endpointId, const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                 const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                 const chip::Optional<chip::ByteSpan> & pinCode,
                                 chip::app::Clusters::DoorLock::OperationErrorEnum & err) override;
    bool HandleDoorUnboltCommand(chip::EndpointId endpointId, const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                 const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                 const chip::Optional<chip::ByteSpan> & pinCode,
                                 chip::app::Clusters::DoorLock::OperationErrorEnum & err) override;

    CHIP_ERROR GetAliroReaderVerificationKey(chip::MutableByteSpan & verificationKey) override;
    CHIP_ERROR GetAliroReaderGroupIdentifier(chip::MutableByteSpan & groupIdentifier) override;
    CHIP_ERROR GetAliroReaderGroupSubIdentifier(chip::MutableByteSpan & groupSubIdentifier) override;
    CHIP_ERROR GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index,
                                                                           chip::MutableByteSpan & protocolVersion) override;
    CHIP_ERROR GetAliroGroupResolvingKey(chip::MutableByteSpan & groupResolvingKey) override;
    CHIP_ERROR GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, chip::MutableByteSpan & protocolVersion) override;
    uint8_t GetAliroBLEAdvertisingVersion() override;
    uint16_t GetNumberOfAliroCredentialIssuerKeysSupported() override;
    uint16_t GetNumberOfAliroEndpointKeysSupported() override;
    CHIP_ERROR SetAliroReaderConfig(const chip::ByteSpan & signingKey, const chip::ByteSpan & verificationKey,
                                    const chip::ByteSpan & groupIdentifier,
                                    const chip::Optional<chip::ByteSpan> & groupResolvingKey) override;
    CHIP_ERROR ClearAliroReaderConfig() override;

private:
    LockEndpoint * Resolve() const;

    chip::EndpointId mEndpointId;
};

} // namespace LockApp
