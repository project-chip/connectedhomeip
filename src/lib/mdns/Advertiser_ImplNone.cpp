/* See Project CHIP LICENSE file for licensing information. */

#include "Advertiser.h"

#include <support/logging/CHIPLogging.h>

namespace chip {
namespace Mdns {
namespace {

class NoneAdvertiser : public ServiceAdvertiser
{
public:
    CHIP_ERROR Start(chip::Inet::InetLayer * inetLayet, uint16_t port) override
    {
        ChipLogError(Discovery, "mDNS advertising not available. mDNS start disabled.");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR Advertise(const OperationalAdvertisingParameters & params) override
    {
        ChipLogError(Discovery, "mDNS advertising not available. Operational Advertisement failed.");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR Advertise(const CommissionAdvertisingParameters & params) override
    {
        ChipLogError(Discovery, "mDNS advertising not available. Commisioning Advertisement failed.");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

NoneAdvertiser gAdvertiser;

} // namespace

ServiceAdvertiser & ServiceAdvertiser::Instance()
{
    return gAdvertiser;
}

} // namespace Mdns
} // namespace chip
