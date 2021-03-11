#include "net_if.h"
#include "common.h"
#include "net_socket.h"
#include <SocketAddress.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

struct if_nameindex * mbed_if_nameindex(void)
{
    char name[IF_NAMESIZE];
    char * name_ptr                = name;
    unsigned int if_counter        = 0;
    struct if_nameindex * net_list = NULL;

    NetworkInterface * net_if = NetworkInterface::get_default_instance();
    if (net_if == nullptr)
    {
        set_errno(ENOBUFS);
        goto exit;
    }

    name_ptr = net_if->get_interface_name(name_ptr);
    if (name_ptr == NULL)
    {
        set_errno(ENOBUFS);
        goto exit;
    }

    net_list = (struct if_nameindex *) realloc(net_list, (if_counter + 2) * sizeof(struct if_nameindex));
    if (net_list == NULL)
    {
        set_errno(ENOBUFS);
        goto exit;
    }

    memset(&net_list[if_counter + 1], 0, sizeof(struct if_nameindex));

    net_list[if_counter].if_name = (char *) malloc(IF_NAMESIZE);
    if (net_list[if_counter].if_name == NULL)
    {
        set_errno(ENOBUFS);
        free(net_list);
        net_list = NULL;
        goto exit;
    }

    strncpy(net_list[if_counter].if_name, name_ptr, IF_NAMESIZE);
    net_list[if_counter].if_index = if_counter + 1;
    if_counter++;

exit:
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
    struct if_nameindex *net_list, *p;

    if (ifname == NULL)
    {
        set_errno(ENXIO);
        goto exit;
    }

    net_list = mbed_if_nameindex();
    if (net_list == NULL)
    {
        set_errno(ENXIO);
        goto exit;
    }

    for (p = net_list; p->if_name != NULL; p++)
    {
        if (p->if_index == ifindex)
        {
            ret = strncpy(ifname, p->if_name, IF_NAMESIZE);
            break;
        }
    }

    mbed_if_freenameindex(net_list);

exit:
    return ret;
}

unsigned int mbed_if_nametoindex(const char * ifname)
{
    unsigned int ret = 0;
    struct if_nameindex *net_list, *p;

    if (ifname == NULL)
    {
        set_errno(ENXIO);
        goto exit;
    }

    net_list = mbed_if_nameindex();
    if (net_list == NULL)
    {
        set_errno(ENXIO);
        goto exit;
    }

    for (p = net_list; p->if_name != NULL; p++)
    {
        if (!strcmp(ifname, p->if_name))
        {
            ret = p->if_index;
            break;
        }
    }

    mbed_if_freenameindex(net_list);

exit:
    return ret;
}

int mbed_getifaddrs(struct ifaddrs ** ifap)
{
    char name[IF_NAMESIZE];
    char * name_ptr = name;
    SocketAddress ip;
    SocketAddress netmask;
    nsapi_version_t ipVersion;
    nsapi_error_t err;

    struct ifaddrs * tmp;

    if (ifap == NULL)
    {
        set_errno(ENOBUFS);
        return -1;
    }

    *ifap = NULL;

    NetworkInterface * net_if = NetworkInterface::get_default_instance();
    if (net_if == nullptr)
    {
        set_errno(ENETUNREACH);
        return -1;
    }

    name_ptr = net_if->get_interface_name(name_ptr);
    if (name_ptr == NULL)
    {
        set_errno(ENOTTY);
        return -1;
    }

    err = net_if->connect();
    if (err != NSAPI_ERROR_OK)
    {
        set_errno(ENOTTY);
        return -1;
    }

    net_if->get_ip_address(&ip);
    ipVersion = ip.get_ip_version();
    net_if->get_netmask(&netmask);

    net_if->disconnect();

    tmp = (struct ifaddrs *) calloc(1, sizeof(struct ifaddrs));
    if (tmp == NULL)
    {
        set_errno(ENOBUFS);
        return -1;
    }

    memset(tmp, 0, sizeof(struct ifaddrs));

    tmp->ifa_next = *ifap;
    *ifap         = tmp;

    tmp->ifa_name = (char *) malloc(IF_NAMESIZE);
    if (tmp->ifa_name == NULL)
    {
        set_errno(ENOBUFS);
        mbed_freeifaddrs(tmp);
        return -1;
    }

    memset(tmp->ifa_name, 0, IF_NAMESIZE);

    if (name_ptr != NULL)
    {
        strncpy(tmp->ifa_name, name_ptr, IF_NAMESIZE);
    }

    tmp->ifa_addr = (struct sockaddr *) malloc(sizeof(struct sockaddr));
    if (tmp->ifa_addr == NULL)
    {
        set_errno(ENOBUFS);
        mbed_freeifaddrs(tmp);
        return -1;
    }

    memset(tmp->ifa_addr, 0, sizeof(struct sockaddr));

    tmp->ifa_netmask = (struct sockaddr *) malloc(sizeof(struct sockaddr));
    if (tmp->ifa_netmask == NULL)
    {
        set_errno(ENOBUFS);
        mbed_freeifaddrs(tmp);
        return -1;
    }

    memset(tmp->ifa_netmask, 0, sizeof(struct sockaddr));

    if (ipVersion == NSAPI_IPv4)
    {
        tmp->ifa_addr->sa_family    = AF_INET;
        tmp->ifa_netmask->sa_family = AF_INET;
        memcpy(&tmp->ifa_addr->data[2], ip.get_ip_bytes(), NSAPI_IPv4_BYTES);
        memcpy(&tmp->ifa_netmask->data[2], netmask.get_ip_bytes(), NSAPI_IPv4_BYTES);
    }
    else if (ipVersion == NSAPI_IPv6)
    {
        tmp->ifa_addr->sa_family    = PF_INET6;
        tmp->ifa_netmask->sa_family = PF_INET6;
        memcpy(tmp->ifa_addr->data, ip.get_ip_bytes(), NSAPI_IPv6_BYTES);
        memcpy(tmp->ifa_netmask->data, netmask.get_ip_bytes(), NSAPI_IPv6_BYTES);
    }
    else
    {
        tmp->ifa_addr->sa_family    = PF_UNSPEC;
        tmp->ifa_netmask->sa_family = PF_UNSPEC;
    }

    return 0;
}

void mbed_freeifaddrs(struct ifaddrs * ifp)
{
    struct ifaddrs * next;

    while (ifp != NULL)
    {
        next = ifp->ifa_next;

        if (ifp->ifa_name)
        {
            free(ifp->ifa_name);
            ifp->ifa_name = NULL;
        }

        if (ifp->ifa_addr)
        {
            free(ifp->ifa_addr);
            ifp->ifa_addr = NULL;
        }

        if (ifp->ifa_netmask)
        {
            free(ifp->ifa_netmask);
            ifp->ifa_netmask = NULL;
        }

        free(ifp);

        ifp = next;
    }

    return;
}

static char * inet_ntop4(const void * src, char * dst, size_t size)
{
    char tmp[sizeof "255.255.255.255"];
    int l;

    uint8_t * buf = (uint8_t *) src;

    l = snprintf(tmp, sizeof(tmp), "%u.%u.%u.%u", buf[0], buf[1], buf[2], buf[3]);
    if (l <= 0 || l >= size)
    {
        set_errno(ENOSPC);
        return (NULL);
    }
    strlcpy(dst, tmp, size);
    return (dst);
}

static char * inet_ntop6(const void * src, char * dst, size_t size)
{
    /*
     * Note that int32_t and int16_t need only be "at least" large enough
     * to contain a value of the specified size.  On some systems, like
     * Crays, there is no such thing as an integer variable with 16 bits.
     * Keep this in mind if you think this function should have been coded
     * to use pointer overlays.  All the world's not a VAX.
     */
    char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"];
    char *tp, *ep;
    struct
    {
        int base, len;
    } best, cur;
    u_int words[IN6ADDRSZ / INT16SZ];
    int i;
    int advance;

    uint8_t * buf = (uint8_t *) src;

    /*
     * Preprocess:
     *	Copy the input (bytewise) array into a wordwise array.
     *	Find the longest run of 0x00's in buf[] for :: shorthanding.
     */
    memset(words, '\0', sizeof words);
    for (i = 0; i < IN6ADDRSZ; i++)
        words[i / 2] |= (buf[i] << ((1 - (i % 2)) << 3));
    best.base = -1;
    cur.base  = -1;
    for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
    {
        if (words[i] == 0)
        {
            if (cur.base == -1)
                cur.base = i, cur.len = 1;
            else
                cur.len++;
        }
        else
        {
            if (cur.base != -1)
            {
                if (best.base == -1 || cur.len > best.len)
                    best = cur;
                cur.base = -1;
            }
        }
    }
    if (cur.base != -1)
    {
        if (best.base == -1 || cur.len > best.len)
            best = cur;
    }
    if (best.base != -1 && best.len < 2)
        best.base = -1;

    /*
     * Format the result.
     */
    tp = tmp;
    ep = tmp + sizeof(tmp);
    for (i = 0; i < (IN6ADDRSZ / INT16SZ) && tp < ep; i++)
    {
        /* Are we inside the best run of 0x00's? */
        if (best.base != -1 && i >= best.base && i < (best.base + best.len))
        {
            if (i == best.base)
            {
                if (tp + 1 >= ep)
                {
                    set_errno(ENOSPC);
                    return (NULL);
                }

                *tp++ = ':';
            }
            continue;
        }
        /* Are we following an initial run of 0x00s or any real hex? */
        if (i != 0)
        {
            if (tp + 1 >= ep)
            {
                set_errno(ENOSPC);
                return (NULL);
            }
            *tp++ = ':';
        }
        /* Is this address an encapsulated IPv4? */
        if (i == 6 && best.base == 0 && (best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
        {
            if (!inet_ntop4(buf + 12, tp, (size_t)(ep - tp)))
            {
                return (NULL);
            }
            tp += strlen(tp);
            break;
        }
        advance = snprintf(tp, ep - tp, "%x", words[i]);
        if (advance <= 0 || advance >= ep - tp)
        {
            set_errno(ENOSPC);
            return (NULL);
        }
        tp += advance;
    }
    /* Was it a trailing run of 0x00's? */
    if (best.base != -1 && (best.base + best.len) == (IN6ADDRSZ / INT16SZ))
    {
        if (tp + 1 >= ep)
        {
            set_errno(ENOSPC);
            return (NULL);
        }
        *tp++ = ':';
    }
    if (tp + 1 >= ep)
    {
        set_errno(ENOSPC);
        return (NULL);
    }
    *tp++ = '\0';

    /*
     * Check for overflow, copy, and we're done.
     */
    if ((size_t)(tp - tmp) > size)
    {
        set_errno(ENOSPC);
        return (NULL);
    }
    strlcpy(dst, tmp, size);
    return (dst);
}

char * mbed_inet_ntop(sa_family_t family, const void * src, char * dst, size_t size)
{
    switch (family)
    {
    case AF_INET:
        return (inet_ntop4(src, dst, (size_t) size));
    case AF_INET6:
        return (inet_ntop6(src, dst, (size_t) size));
    default:
        set_errno(EAFNOSUPPORT);
        return (NULL);
    }
}

/* Return the value of CH as a hexademical digit, or -1 if it is a
   different type of character.  */
static int hex_digit_value(char ch)
{
    if ('0' <= ch && ch <= '9')
        return ch - '0';
    if ('a' <= ch && ch <= 'f')
        return ch - 'a' + 10;
    if ('A' <= ch && ch <= 'F')
        return ch - 'A' + 10;
    return -1;
}

static int inet_pton4(const char * src, void * dst)
{
    int saw_digit, octets, ch;
    unsigned char tmp[INADDRSZ], *tp;

    const char * end = src + strlen(src);

    saw_digit   = 0;
    octets      = 0;
    *(tp = tmp) = 0;
    while (src < end)
    {
        ch = *src++;
        if (ch >= '0' && ch <= '9')
        {
            unsigned int digit = *tp * 10 + (ch - '0');

            if (saw_digit && *tp == 0)
                return 0;
            if (digit > 255)
                return 0;
            *tp = digit;
            if (!saw_digit)
            {
                if (++octets > 4)
                    return 0;
                saw_digit = 1;
            }
        }
        else if (ch == '.' && saw_digit)
        {
            if (octets == 4)
                return 0;
            *++tp     = 0;
            saw_digit = 0;
        }
        else
            return 0;
    }
    if (octets < 4)
        return 0;
    memcpy((unsigned char *) dst, tmp, INADDRSZ);
    return 1;
}

static int inet_pton6(const char * src, void * dst)
{
    unsigned char tmp[IN6ADDRSZ], *tp, *endp, *colonp;
    const char * curtok;
    int ch;
    size_t xdigits_seen; /* Number of hex digits since colon.  */
    unsigned int val;

    const char * src_endp = src + strlen(src);

    tp     = (unsigned char *) memset(tmp, '\0', IN6ADDRSZ);
    endp   = tp + IN6ADDRSZ;
    colonp = NULL;

    /* Leading :: requires some special handling.  */
    if (src == src_endp)
        return 0;
    if (*src == ':')
    {
        ++src;
        if (src == src_endp || *src != ':')
            return 0;
    }

    curtok       = src;
    xdigits_seen = 0;
    val          = 0;
    while (src < src_endp)
    {
        ch        = *src++;
        int digit = hex_digit_value(ch);
        if (digit >= 0)
        {
            if (xdigits_seen == 4)
                return 0;
            val <<= 4;
            val |= digit;
            if (val > 0xffff)
                return 0;
            ++xdigits_seen;
            continue;
        }
        if (ch == ':')
        {
            curtok = src;
            if (xdigits_seen == 0)
            {
                if (colonp)
                    return 0;
                colonp = tp;
                continue;
            }
            else if (src == src_endp)
                return 0;
            if (tp + INT16SZ > endp)
                return 0;
            *tp++        = (unsigned char) (val >> 8) & 0xff;
            *tp++        = (unsigned char) val & 0xff;
            xdigits_seen = 0;
            val          = 0;
            continue;
        }
        if (ch == '.' && ((tp + INADDRSZ) <= endp) && inet_pton4(curtok, tp) > 0)
        {
            tp += INADDRSZ;
            xdigits_seen = 0;
            break; /* '\0' was seen by inet_pton4.  */
        }
        return 0;
    }
    if (xdigits_seen > 0)
    {
        if (tp + INT16SZ > endp)
            return 0;
        *tp++ = (unsigned char) (val >> 8) & 0xff;
        *tp++ = (unsigned char) val & 0xff;
    }
    if (colonp != NULL)
    {
        /* Replace :: with zeros.  */
        if (tp == endp)
            /* :: would expand to a zero-width field.  */
            return 0;
        size_t n = tp - colonp;
        memmove(endp - n, colonp, n);
        memset(colonp, 0, endp - n - colonp);
        tp = endp;
    }
    if (tp != endp)
        return 0;
    memcpy((unsigned char *) dst, tmp, IN6ADDRSZ);
    return 1;
}

int mbed_inet_pton(sa_family_t family, const char * src, void * dst)
{
    switch (family)
    {
    case AF_INET:
        return inet_pton4(src, dst);
    case AF_INET6:
        return inet_pton6(src, dst);
    default:
        set_errno(EAFNOSUPPORT);
        return -1;
    }
}

int mbed_ioctl(int fd, unsigned long request, void * param)
{
    int ret = 0;
    switch (request)
    {
    case SIOCGIFFLAGS: {
        if (param == NULL)
        {
            set_errno(EFAULT);
            ret = -1;
            break;
        }
        struct ifreq * intfData = (struct ifreq *) param;
        short * flags           = &intfData->ifr_flags;
        *flags                  = 0;
        nsapi_error_t err;
        NetworkInterface * net_if = NetworkInterface::get_default_instance();
        if (net_if == nullptr)
        {
            set_errno(ENOTTY);
            ret = -1;
            break;
        }

        err = net_if->connect();
        if (err != NSAPI_ERROR_OK)
        {
            set_errno(ENOTTY);
            ret = -1;
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
            set_errno(ENOTTY);
            ret = -1;
            break;
        }
        err = net_if->get_netmask(&netmask);
        if (err != NSAPI_ERROR_OK)
        {
            set_errno(ENOTTY);
            ret = -1;
            break;
        }

        if (netmask.get_ip_version() == NSAPI_IPv6)
        {
            bytesNumber = NSAPI_IPv6_BYTES;
        }
        else
        {
            bytesNumber = NSAPI_IPv4_BYTES;
        }

        uint8_t * ip_bytes      = (uint8_t *) ip.get_ip_bytes();
        uint8_t * netmask_bytes = (uint8_t *) netmask.get_ip_bytes();
        uint8_t mask, netmask_neg;
        for (int index = 0; index < bytesNumber; ++index)
        {
            netmask_neg = ~(*netmask_bytes);
            mask        = *ip_bytes & netmask_neg;
            if (mask && (mask != netmask_neg))
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
        set_errno(ENOTTY);
        ret = -1;
    }
    return ret;
}