#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>

using namespace ::chip;
using namespace chip::app::Clusters::DoorLock;

bool emberAfPluginDoorLockOnDoorLockCommand(EndpointId endpointId, const chip::Optional<ByteSpan> & pinCode, DlOperationError & err)
{
    return true;
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(EndpointId endpointId, const chip::Optional<ByteSpan> & pinCode,
                                              DlOperationError & err)
{
    return true;
}
