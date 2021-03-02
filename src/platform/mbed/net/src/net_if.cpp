#include <NetworkInterface.h>
#include <errno.h>
#include <net_if.h>

static struct if_nameindex * net_list;
static NetworkInterface * net_if = NetworkInterface::get_default_instance();
struct if_nameindex * mbed_if_nameindex(void)
{
    char name[IF_NAMESIZE];
    char * name_ptr = name;

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

    net_list = (struct if_nameindex *) calloc((MBED_NET_IF_LIST_SIZE), sizeof(struct if_nameindex));
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

char * mbed_if_indextoname(unsigned int ifindex, char * ifname)
{
    char * ret = NULL;
    int index  = ifindex - 1;

    if (index < 0 || index > MBED_MAX_INTERFACES_NUM || ifname == NULL || net_list == NULL)
    {
        errno = ENXIO;
        return NULL;
    }

    if (net_list[index].if_name != NULL)
    {
        ret = strncpy(ifname, net_list[index].if_name, IF_NAMESIZE);
    }

    return ret;
}

unsigned int mbed_if_nametoindex(const char * ifname)
{
    unsigned int index;
    if (ifname == NULL || net_list == NULL)
    {
        errno = ENXIO;
        return NULL;
    }

    for (index = 0; index < MBED_NET_IF_LIST_SIZE; ++index)
    {
        if (!strcmp(ifname, net_list[index].if_name))
        {
            return net_list[index].if_index;
        }
    }

    return 0;
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