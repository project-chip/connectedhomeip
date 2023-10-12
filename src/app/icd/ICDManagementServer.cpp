#include "ICDManagementServer.h"

using namespace chip;
using namespace chip::Protocols;

namespace chip {

ICDManagementServer ICDManagementServer::mInstance;

Status ICDManagementServer::RegisterClient(PersistentStorageDelegate & storage, FabricIndex fabric_index, chip::NodeId node_id,
                                           uint64_t monitored_subject, chip::ByteSpan key,
                                           Optional<chip::ByteSpan> verification_key, bool is_admin)
{
    ICDMonitoringTable table(storage, fabric_index, GetClientsSupportedPerFabric(), mSymmetricKeystore);

    // Get current entry, if exists
    ICDMonitoringEntry entry(mSymmetricKeystore);
    CHIP_ERROR err = table.Find(node_id, entry);
    if (CHIP_NO_ERROR == err)
    {
        // Existing entry: Validate Key if, and only if, the ISD has NOT administrator permissions
        if (!is_admin)
        {
            VerifyOrReturnError(verification_key.HasValue(), InteractionModel::Status::Failure);
            VerifyOrReturnError(entry.IsKeyEquivalent(verification_key.Value()), InteractionModel::Status::Failure);
        }
    }
    else if (CHIP_ERROR_NOT_FOUND == err)
    {
        // New entry
        VerifyOrReturnError(entry.index < table.Limit(), InteractionModel::Status::ResourceExhausted);
    }
    else
    {
        // Error
        return InteractionModel::Status::Failure;
    }

    // Save
    entry.checkInNodeID    = node_id;
    entry.monitoredSubject = monitored_subject;
    err                    = entry.SetKey(key);
    VerifyOrReturnError(CHIP_ERROR_INVALID_ARGUMENT != err, InteractionModel::Status::ConstraintError);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);
    err = table.Set(entry.index, entry);

    // Delete key upon failure to prevent key storage leakage.
    if (err != CHIP_NO_ERROR)
    {
        entry.DeleteKey();
    }

    VerifyOrReturnError(CHIP_ERROR_INVALID_ARGUMENT != err, InteractionModel::Status::ConstraintError);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);

    return InteractionModel::Status::Success;
}

Status ICDManagementServer::UnregisterClient(PersistentStorageDelegate & storage, FabricIndex fabric_index, chip::NodeId node_id,
                                             Optional<chip::ByteSpan> verificationKey, bool is_admin)
{
    ICDMonitoringTable table(storage, fabric_index, GetClientsSupportedPerFabric(), mSymmetricKeystore);

    // Get current entry, if exists
    ICDMonitoringEntry entry(mSymmetricKeystore);
    CHIP_ERROR err = table.Find(node_id, entry);
    VerifyOrReturnError(CHIP_ERROR_NOT_FOUND != err, InteractionModel::Status::NotFound);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);

    // Existing entry: Validate Key if, and only if, the ISD has NOT administrator permissions
    if (!is_admin)
    {
        VerifyOrReturnError(verificationKey.HasValue(), InteractionModel::Status::Failure);
        VerifyOrReturnError(entry.IsKeyEquivalent(verificationKey.Value()), InteractionModel::Status::Failure);
    }

    err = table.Remove(entry.index);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);

    return InteractionModel::Status::Success;
}

Status ICDManagementServer::StayActiveRequest(FabricIndex fabric_index)
{
    // TODO: Implementent stay awake logic for end device
    return InteractionModel::Status::UnsupportedCommand;
}

} // namespace chip
