#include "IcdManagementServer.h"
#include <app/icd/IcdMonitoringTable.h>

using namespace chip;
using namespace chip::Protocols;

namespace chip {

IcdManagementServer IcdManagementServer::mInstance;

Status IcdManagementServer::RegisterClient(PersistentStorageDelegate & storage, FabricIndex fabric_index, chip::NodeId node_id,
                                           uint64_t monitored_subject, chip::ByteSpan key,
                                           Optional<chip::ByteSpan> verification_key, bool is_admin)
{
    IcdMonitoringTable table(storage, fabric_index, GetClientsSupportedPerFabric());

    // Get current entry, if exists
    IcdMonitoringEntry entry;
    CHIP_ERROR err = table.Find(node_id, entry);
    if (CHIP_NO_ERROR == err)
    {
        // Existing entry: Validate Key if, and only if, the ISD has NOT administrator permissions
        if (!is_admin)
        {
            VerifyOrReturnError(verification_key.HasValue(), InteractionModel::Status::Failure);
            VerifyOrReturnError(verification_key.Value().data_equal(entry.key), InteractionModel::Status::Failure);
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
    entry.key              = key;
    err                    = table.Set(entry.index, entry);
    VerifyOrReturnError(CHIP_ERROR_INVALID_ARGUMENT != err, InteractionModel::Status::ConstraintError);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);

    return InteractionModel::Status::Success;
}

Status IcdManagementServer::UnregisterClient(PersistentStorageDelegate & storage, FabricIndex fabric_index, chip::NodeId node_id,
                                             Optional<chip::ByteSpan> verificationKey, bool is_admin)
{
    IcdMonitoringTable table(storage, fabric_index, GetClientsSupportedPerFabric());

    // Get current entry, if exists
    IcdMonitoringEntry entry;
    CHIP_ERROR err = table.Find(node_id, entry);
    VerifyOrReturnError(CHIP_ERROR_NOT_FOUND != err, InteractionModel::Status::NotFound);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);

    // Existing entry: Validate Key if, and only if, the ISD has NOT administrator permissions
    if (!is_admin)
    {
        VerifyOrReturnError(verificationKey.HasValue(), InteractionModel::Status::Failure);
        VerifyOrReturnError(verificationKey.Value().data_equal(entry.key), InteractionModel::Status::Failure);
    }

    err = table.Remove(entry.index);
    VerifyOrReturnError(CHIP_NO_ERROR == err, InteractionModel::Status::Failure);

    return InteractionModel::Status::Success;
}

Status IcdManagementServer::StayActiveRequest(FabricIndex fabric_index)
{
    // TODO: Implementent stay awake logic for end device
    return InteractionModel::Status::UnsupportedCommand;
}

} // namespace chip
