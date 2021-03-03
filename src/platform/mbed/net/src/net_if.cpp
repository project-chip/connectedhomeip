#include <NetworkInterface.h>
#include <SocketAddress.h>
#include <errno.h>
#include <net/if.h>
#include <net_if.h>
#include <sys/ioctl.h>
struct if_nameindex * mbed_if_nameindex(void)
{
    char name[IF_NAMESIZE];
    char * name_ptr = name;

    NetworkInterface * net_if = NetworkInterface::get_default_instance();
    if (net_if == nullptr)
    {
        errno = ENOBUFS;
        return NULL;
    }

    name_ptr = net_if->get_interface_name(name_ptr);
    if (name_ptr == NULL)
    {
        errno = ENOBUFS;
        return NULL;
    }

    struct if_nameindex * net_list = (struct if_nameindex *) calloc((MBED_NET_IF_LIST_SIZE), sizeof(struct if_nameindex));
    if (net_list == NULL)
    {
        errno = ENOBUFS;
        return NULL;
    }

    memset(net_list, 0, sizeof(struct if_nameindex) * MBED_NET_IF_LIST_SIZE);

    net_list[0].if_name = (char *) malloc(IF_NAMESIZE);
    if (net_list[0].if_name == NULL)
    {
        errno = ENOBUFS;
        free(net_list);
        return NULL;
    }

    net_list[0].if_index = 1;
    strncpy(net_list[0].if_name, name_ptr, IF_NAMESIZE);

    return net_list;
}

void mbed_if_freenameindex(struct if_nameindex * ptr)
{
    struct if_nameindex * p;

    if (ptr == NULL)
    {
        return;
    }

    for (p = ptr; p->if_name != NULL; p++)
    {
        free(p->if_name);
    }

    free(ptr);
}

char * mbed_if_indextoname(unsigned int ifindex, char * ifname)
{
    char * ret = NULL;
    int index  = ifindex - 1;

    if (index < 0 || index > MBED_MAX_INTERFACES_NUM || ifname == NULL)
    {
        errno = ENXIO;
        return NULL;
    }

    struct if_nameindex * net_list = mbed_if_nameindex();
    if (net_list == NULL)
    {
        errno = ENXIO;
        return NULL;
    }

    if (net_list[index].if_name != NULL)
    {
        ret = strncpy(ifname, net_list[index].if_name, IF_NAMESIZE);
    }
    else
    {
        errno = ENXIO;
    }

    mbed_if_freenameindex(net_list);

    return ret;
}

unsigned int mbed_if_nametoindex(const char * ifname)
{
    unsigned int index, ret = 0;
    if (ifname == NULL)
    {
        errno = ENXIO;
        return NULL;
    }

    struct if_nameindex * net_list = mbed_if_nameindex();
    if (net_list == NULL)
    {
        errno = ENXIO;
        return NULL;
    }

    for (index = 0; index < MBED_NET_IF_LIST_SIZE; ++index)
    {
        if (!strcmp(ifname, net_list[index].if_name))
        {
            ret = net_list[index].if_index;
            break;
        }
    }

    mbed_if_freenameindex(net_list);

    return ret;
}

int mbed_getifaddrs(struct ifaddrs ** ifap)
{
    return 0;
}

void mbed_freeifaddrs(struct ifaddrs * ifp)
{
    return;
}

char * mbed_inet_ntop(sa_family_t family, const void * src, char * dst, size_t size)
{
    return NULL;
}

int mbed_inet_pton(sa_family_t family, const char * src, void * dst)
{
    return 0;
}

int mbed_ioctl(int fd, unsigned long request, void * param)
{
    int ret = 0;
    switch (request)
    {
    case SIOCGIFFLAGS: {
        if (param == NULL)
        {
            errno = EFAULT;
            ret   = -1;
            break;
        }
        struct ifreq * intfData = (struct ifreq *) param;
        short * flags           = &intfData->ifr_flags;
        *flags                  = 0;
        nsapi_error_t err;
        NetworkInterface * net_if = NetworkInterface::get_default_instance();
        if (net_if == nullptr)
        {
            errno = ENOTTY;
            ret   = -1;
            break;
        }

        err = net_if->connect();
        if (err != NSAPI_ERROR_OK)
        {
            errno = ENOTTY;
            ret   = -1;
            break;
        }

        nsapi_connection_status_t status = net_if->get_connection_status();
        if (status == NSAPI_STATUS_LOCAL_UP || status == NSAPI_STATUS_GLOBAL_UP)
        {
            *flags |= IFF_UP;
        }

        SocketAddress ip;
        SocketAddress netmask;
        bool isBroadcast         = true;
        unsigned int bytesNumber = 0;

        err = net_if->get_ip_address(&ip);
        if (err != NSAPI_ERROR_OK)
        {
            errno = ENOTTY;
            ret   = -1;
            break;
        }
        err = net_if->get_netmask(&netmask);
        if (err != NSAPI_ERROR_OK)
        {
            errno = ENOTTY;
            ret   = -1;
            break;
        }

        if (netmask.get_ip_version() == NSAPI_IPv6)
        {
            bytesNumber = NSAPI_IPv6_BYTES;
        }
        else
        {
            bytesNumber = NSAPI_IPv6_BYTES;
        }

        uint8_t * ip_bytes      = (uint8_t *) ip.get_ip_bytes();
        uint8_t * netmask_bytes = (uint8_t *) netmask.get_ip_bytes();
        uint8_t mask;
        for (int index = 0; index < bytesNumber; ++index)
        {
            mask = *ip_bytes & ~(*netmask_bytes);
            if (mask && mask != ~(*netmask_bytes))
            {
                isBroadcast = false;
                break;
            }
            ip_bytes++;
            netmask_bytes++;
        }

        if (isBroadcast)
        {
            *flags |= IFF_BROADCAST;
        }

        net_if->disconnect();

        break;
    }

    default:
        errno = ENOTTY;
        ret   = -1;
    }
    return ret;
}