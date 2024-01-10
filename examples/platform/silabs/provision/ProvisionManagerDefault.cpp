#include "ProvisionManager.h"
#include "ProvisionStorageDefault.h"
#include "RttStreamChannel.h"

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

RttStreamChannel sProvisionChannel;
DefaultStorage sProvisionStore;
Manager sProvisionManager(sProvisionStore, sProvisionChannel);

Manager & Manager::GetInstance() { return sProvisionManager; }

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
