#include <app-common/zap-generated/callback.h>

using namespace chip;
void __attribute__((weak)) emberAfThirdClusterInitCallback(EndpointId endpoint)
{
    // To prevent warning
    (void) endpoint;
}
