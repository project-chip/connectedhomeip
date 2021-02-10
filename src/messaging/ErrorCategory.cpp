/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *      This file implements the utility class for the CHIP Message Layer.
 */

#include <errno.h>

#include <messaging/ErrorCategory.h>

#include <inet/InetError.h>
#include <system/SystemConfig.h>
#include <system/SystemError.h>

namespace chip {
namespace Messaging {

bool IsIgnoredMulticastSendError(CHIP_ERROR err)
{
    return err == CHIP_NO_ERROR ||
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        err == System::MapErrorLwIP(ERR_RTE)
#else
        err == System::MapErrorPOSIX(ENETUNREACH) || err == System::MapErrorPOSIX(EADDRNOTAVAIL)
#endif
        ;
}

CHIP_ERROR FilterUDPSendError(CHIP_ERROR err, bool isMulticast)
{
    // Don't report certain types of routing errors when they occur while sending multicast packets.
    // These may indicate that the underlying interface doesn't support multicast (e.g. the loopback
    // interface on linux) or that the selected interface doesn't have an appropriate source address.
    if (isMulticast)
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        if (err == System::MapErrorLwIP(ERR_RTE))
        {
            err = CHIP_NO_ERROR;
        }
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
        if (err == System::MapErrorPOSIX(ENETUNREACH) || err == System::MapErrorPOSIX(EADDRNOTAVAIL))
        {
            err = CHIP_NO_ERROR;
        }
#endif
    }

    return err;
}

/**
 *  Checks if error, while sending, is critical enough to report to the application.
 *
 *  @param[in]    err      The #CHIP_ERROR being checked for criticality.
 *
 *  @return    true if the error is NOT critical; false otherwise.
 *
 */
bool IsSendErrorNonCritical(CHIP_ERROR err)
{
    return (err == INET_ERROR_NOT_IMPLEMENTED || err == INET_ERROR_OUTBOUND_MESSAGE_TRUNCATED ||
            err == INET_ERROR_MESSAGE_TOO_LONG || err == INET_ERROR_NO_MEMORY || CHIP_CONFIG_IsPlatformErrorNonCritical(err));
}

} // namespace Messaging
} // namespace chip
