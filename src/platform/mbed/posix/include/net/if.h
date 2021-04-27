#ifndef MBED_POSIX_NET_IF_H
#define MBED_POSIX_NET_IF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Standard interface flags */
#define IFF_UP 0x1           /* interface is up		*/
#define IFF_BROADCAST 0x2    /* broadcast address valid	*/
#define IFF_DEBUG 0x4        /* turn on debugging		*/
#define IFF_LOOPBACK 0x8     /* is a loopback net		*/
#define IFF_POINTOPOINT 0x10 /* interface is has p-p link	*/
#define IFF_NOTRAILERS 0x20  /* avoid use of trailers	*/
#define IFF_RUNNING 0x40     /* interface RFC2863 OPER_UP	*/
#define IFF_NOARP 0x80       /* no ARP protocol		*/
#define IFF_PROMISC 0x100    /* receive all packets		*/
#define IFF_ALLMULTI 0x200   /* receive all multicast packets*/

#define IFF_MASTER 0x400 /* master of a load balancer 	*/
#define IFF_SLAVE 0x800  /* slave of a load balancer	*/

#define IFF_MULTICAST 0x1000 /* Supports multicast		*/

#define IFF_PORTSEL 0x2000   /* can set media type		*/
#define IFF_AUTOMEDIA 0x4000 /* auto media select active	*/
#define IFF_DYNAMIC 0x8000   /* dialup device with changing addresses*/

#define IFF_LOWER_UP 0x10000 /* driver signals L1 up		*/
#define IFF_DORMANT 0x20000  /* driver signals dormant	*/

#define IFF_ECHO 0x40000 /* echo sent packets		*/

struct if_nameindex
{
    unsigned int if_index; /* Index of interface (1, 2, ...) */
    char * if_name;        /* Null-terminated name ("eth0", etc.) */
};

struct if_nameindex * if_nameindex(void);
char * if_indextoname(unsigned int ifindex, char * ifname);
unsigned int if_nametoindex(const char * ifname);
void if_freenameindex(struct if_nameindex * ptr);

#ifdef __cplusplus
}
#endif

#endif /* MBED_POSIX_NET_IF_H */
