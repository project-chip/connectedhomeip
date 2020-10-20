#include "MdnsImpl.h"

namespace chip {
namespace DeviceLayer {

void UpdateMdnsDataset(fd_set & /*readFdSet*/, fd_set & /*writeFdSet*/, fd_set & /*errorFdSet*/, int & /*maxFd*/,
                       timeval & /*timeout*/)
{
    // TODO @gjc13: Add implementation using mDNSResponder
}

void ProcessMdns(fd_set & /*readFdSet*/, fd_set & /*writeFdSet*/, fd_set & /*errorFdSet*/)
{
    // TODO @gjc13: Add implementation using mDNSResponder
}

} // namespace DeviceLayer
} // namespace chip
