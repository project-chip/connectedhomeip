#include "ThreadDiscoveryCode.h"

#include <assert.h>
#include <endian.h>

#include <lib/support/CodeUtils.h>

namespace chip {
namespace Thread {

DiscoveryCode::DiscoveryCode(uint8_t discriminator, uint32_t pincode, [[maybe_unused]] uint8_t version)
{
    assert(version == 0);

    // Only use 4 bits version
    mCode = (0x4a191b4dULL << 32) | (pincode << 4) | discriminator;
}

std::vector<uint8_t> DiscoveryCode::ToByteArray() const
{
    uint64_t code = htobe64(mCode);

    return std::vector<uint8_t>(reinterpret_cast<const uint8_t *>(&code), reinterpret_cast<const uint8_t *>(&code) + sizeof(code));
}

} // namespace Thread
} // namespace chip
