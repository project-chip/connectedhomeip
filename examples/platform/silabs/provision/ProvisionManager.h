#pragma once

#include "ProvisionProtocol.h"
#include "ProvisionChannel.h"
#include "ProvisionStorage.h"
#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

class Manager
{
public:
    Manager():
#ifdef SILABS_PROVISION_PROTOCOL_V1
    mProtocol1(mStore),
#endif
    mProtocol2(mStore) {}

    CHIP_ERROR Init();
    bool Step();
    bool IsProvisionRequired();
    CHIP_ERROR SetProvisionRequired(bool required);
    Storage & GetStorage() { return mStore; }
    static Manager & GetInstance();

private:
    bool ProcessCommand(ByteSpan & request, MutableByteSpan & response);

    Storage mStore;
    Channel mChannel;
#ifdef SILABS_PROVISION_PROTOCOL_V1
    Protocol1 mProtocol1;
#endif
    Protocol2 mProtocol2;
    bool mProvisionRequested = true;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
