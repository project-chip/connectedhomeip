#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>

// Include door lock callbacks only when the server is enabled
#ifdef EMBER_AF_PLUGIN_DOOR_LOCK_SERVER
#include <app/clusters/door-lock-server/door-lock-server.h>

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const chip::Optional<chip::ByteSpan> & pinCode,
                                            chip::app::Clusters::DoorLock::DlOperationError & err)
{
    err = DlOperationError::kUnspecified;
    // TBD: LockManager, check pinCode, ...
    return DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kLocked);
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const chip::Optional<chip::ByteSpan> & pinCode,
                                              chip::app::Clusters::DoorLock::DlOperationError & err)
{
    err = DlOperationError::kUnspecified;
    // TBD: LockManager, check pinCode, ...
    return DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kUnlocked);
}
#endif /* EMBER_AF_PLUGIN_DOOR_LOCK_SERVER */
