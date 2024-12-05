#include <app/SafeAttributePersistenceProvider.h>

namespace chip {
namespace app {

namespace {

SafeAttributePersistenceProvider * gSafeAttributeSaver = nullptr;

} // anonymous namespace

SafeAttributePersistenceProvider * GetSafeAttributePersistenceProvider()
{
    return gSafeAttributeSaver;
}

void SetSafeAttributePersistenceProvider(SafeAttributePersistenceProvider * aProvider)
{
    VerifyOrReturn(aProvider != nullptr);
    gSafeAttributeSaver = aProvider;
}

} // namespace app
} // namespace chip
