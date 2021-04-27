#include "net_dns.h"
#include <common.h>
#include <netsocket/NetworkInterface.h>

int mbed_getaddrinfo(const char * nodename, const char * servname, const struct addrinfo * hints, struct addrinfo ** res)
{
    SocketAddress sHints;
    SocketAddress * result;
    nsapi_value_or_error_t err;
    int ret = 0;
    *res    = NULL;
    struct addrinfo * tmp;

    if (nodename == nullptr || hints == nullptr || res == nullptr)
    {
        set_errno(EINVAL);
        return EAI_SYSTEM;
    }

    NetworkInterface * net = NetworkInterface::get_default_instance();
    if (net == nullptr)
    {
        set_errno(ENETUNREACH);
        return EAI_SYSTEM;
    }

    switch (hints->ai_family)
    {
    case AF_INET:
        sHints.set_addr({ NSAPI_IPv4, 0 });
        break;
    case AF_INET6:
        sHints.set_addr({ NSAPI_IPv6, 0 });
        break;
    default:
        return EAI_FAMILY;
    }

    err = net->getaddrinfo(nodename, &sHints, &result);
    if (err < 0)
    {
        switch (ret)
        {
        case NSAPI_ERROR_DNS_FAILURE:
            ret = EAI_FAIL;
        case NSAPI_ERROR_NO_MEMORY:
            ret = EAI_MEMORY;
        default:
            set_errno(EINVAL);
            ret = EAI_SYSTEM;
        }
    }
    else if (err == 0)
    {
        ret = EAI_NODATA;
    }
    else
    {
        for (int i = 0; i < err; i++)
        {
            tmp = new addrinfo;
            if (tmp == nullptr)
            {
                break;
            }

            tmp->ai_next = *res;
            *res         = tmp;

            tmp->ai_addr = new sockaddr;
            if (tmp->ai_addr == nullptr)
            {
                mbed_freeaddrinfo(tmp);
                break;
            }

            if (convert_mbed_addr_to_bsd(tmp->ai_addr, &result[i]) < 0)
            {
                mbed_freeaddrinfo(tmp);
                continue;
            }
        }
    }

    delete[] result;

    return ret;
}

void mbed_freeaddrinfo(struct addrinfo * ai)
{
    struct addrinfo * next;

    while (ai != NULL)
    {
        next = ai->ai_next;

        if (ai->ai_addr)
        {
            delete ai->ai_addr;
            ai->ai_addr = NULL;
        }

        delete ai;

        ai = next;
    }
}
