#include "ProvisionManager.h"
#include "ProvisionStorageFlash.h"
#include "RttStreamChannel.h"

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

RttStreamChannel sProvisionChannel;
FlashStorage sProvisionStore;
Manager sProvisionManager(sProvisionStore, sProvisionChannel);

Manager & Manager::GetInstance() { return sProvisionManager; }

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
