#include "netdb.h"
#include "netsocket/NetworkInterface.h"
#include <common.h>
#include <sys/socket.h>

int mbed_getaddrinfo(const char * nodename, const char * servname, const struct addrinfo * hints, struct addrinfo ** res)
{

    SocketAddress sHints;

    NetworkInterface * net = NetworkInterface::get_default_instance();
    if (hints->ai_family == AF_INET6)
    {
        sHints.set_addr({ NSAPI_IPv6, 0 });
    }
    else
    {
        sHints.set_addr({ NSAPI_IPv4, 0 });
    }

    SocketAddress * temp = new SocketAddress[5];
    if (!temp)
    {
        return -1;
    }

    int result = net->getaddrinfo(nodename, &sHints, &temp);

    if (result > 0)
    {

        for (int i = 0; i < result; i++)
        {
            res[i]          = new addrinfo;
            res[i]->ai_addr = new sockaddr;
            res[i]->ai_next = nullptr;
            if (i > 0)
            {
                res[i - 1]->ai_next = res[i];
            }
            if (convert_mbed_addr_to_bsd(res[i]->ai_addr, &temp[i]) < 0)
            {
                // to  do free res  to avoid memory leak
                delete[] temp;
                return -1;
            }
        }
    }
    delete[] temp;
    return 0;
}

void mbed_freeaddrinfo(struct addrinfo * ai)
{
    struct addrinfo * next;
    while (ai != NULL)
    {
        next = ai->ai_next;
        free(ai);
        ai = next;
    }
}