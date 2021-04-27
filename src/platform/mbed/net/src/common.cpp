#include "common.h"
#include <stdlib.h>
#include <string.h>

int convert_bsd_addr_to_mbed(SocketAddress * out, struct sockaddr * in)
{
    if (out == NULL || in == NULL)
    {
        return -1;
    }

    if (in->sa_family == AF_INET)
    {
        sockaddr_in * addr = reinterpret_cast<sockaddr_in *>(in);
        out->set_ip_bytes((const void *) addr->sin_addr.s4_addr, NSAPI_IPv4);
        out->set_port(ntohs(addr->sin_port));
    }
    else if (in->sa_family == AF_INET6)
    {
        sockaddr_in6 * addr = reinterpret_cast<sockaddr_in6 *>(in);
        out->set_ip_bytes((const void *) addr->sin6_addr.s6_addr, NSAPI_IPv6);
        out->set_port(ntohs(addr->sin6_port));
    }

    return 0;
}

int convert_mbed_addr_to_bsd(struct sockaddr * out, const SocketAddress * in)
{
    if (out == NULL || in == NULL)
    {
        return -1;
    }

    nsapi_version_t verison = in->get_ip_version();
    if (verison == NSAPI_IPv4)
    {
        sockaddr_in * addr = reinterpret_cast<sockaddr_in *>(out);
        addr->sin_family   = AF_INET;
        addr->sin_port     = htons(in->get_port());
        memcpy(&addr->sin_addr, in->get_ip_bytes(), NSAPI_IPv4_BYTES);
    }
    else if (verison == NSAPI_IPv6)
    {
        sockaddr_in6 * addr = reinterpret_cast<sockaddr_in6 *>(out);
        addr->sin6_family   = AF_INET;
        addr->sin6_port     = htons(in->get_port());
        memcpy(&addr->sin6_addr, in->get_ip_bytes(), NSAPI_IPv6_BYTES);
    }
    else
    {
        out->sa_family = PF_UNSPEC;
    }

    return 0;
}
