#include "MdnsImpl.h"

namespace chip {
namespace DeviceLayer {

void UpdateMdnsDataset(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet, int & maxFd, timeval & timeout)
{
    // TODO @gjc13: Add implementation using mDNSResponder
    (void) readFdSet;
    (void) writeFdSet;
    (void) errorFdSet;
    (void) maxFd;
    (void) timeout;
}

void ProcessMdns(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet)
{
    // TODO @gjc13: Add implementation using mDNSResponder
    (void) readFdSet;
    (void) writeFdSet;
    (void) errorFdSet;
}

} // namespace DeviceLayer
} // namespace chip
