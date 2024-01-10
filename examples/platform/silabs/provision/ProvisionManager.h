#pragma once

#include "ProvisionCommands.h"
#include "ProvisionEncoder.h"
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
    Manager(Storage & store, Channel &channel): mStore(store), mChannel(channel) {}
    CHIP_ERROR Start();
    void SetChannel(Channel & ch)
    {
        mChannel = ch;
    }
    Storage & GetStorage() {
        return mStore;
    }

    void Run();
    bool ProvisionRequired();
    CHIP_ERROR RequestProvision();
    static Manager & GetInstance();

private:
    bool ProcessCommand(Encoder & input, Encoder & output);
    Command *DecodeCommand(Encoder & input);
    void EncodeHeader(Encoder & output, uint8_t cid, CHIP_ERROR err);

    Storage & mStore;
    Channel & mChannel;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
