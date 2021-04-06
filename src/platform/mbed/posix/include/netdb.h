
#ifndef MBED_POSIX_NETDB_H
#define MBED_POSIX_NETDB_H

#ifdef __cplusplus
extern "C" {
#endif
#include <net_dns.h>

/** Errors used by the DNS API functions, h_errno can be one of them */
#define EAI_NONAME 200
#define EAI_SERVICE 201
#define EAI_FAIL 202
#define EAI_MEMORY 203
#define EAI_FAMILY 204
#define EAI_NODATA 205
#define EAI_SYSTEM 206
#define EAI_ADDRFAMILY 207
#define EAI_AGAIN 208

#define HOST_NOT_FOUND 210
#define NO_DATA 211
#define NO_RECOVERY 212
#define TRY_AGAIN 213

/* input flags for struct addrinfo */
#define AI_PASSIVE 0x01
#define AI_CANONNAME 0x02
#define AI_NUMERICHOST 0x04
#define AI_NUMERICSERV 0x08
#define AI_V4MAPPED 0x10
#define AI_ALL 0x20
#define AI_ADDRCONFIG 0x40

struct hostent
{
    char * h_name;            /* Official name of the host. */
    char ** h_aliases;        /* A pointer to an array of pointers to alternative host names,
                                 terminated by a null pointer. */
    int h_addrtype;           /* Address type. */
    int h_length;             /* The length, in bytes, of the address. */
    char ** h_addr_list;      /* A pointer to an array of pointers to network addresses (in
                                 network byte order) for the host, terminated by a null pointer. */
#define h_addr h_addr_list[0] /* for backward compatibility */
};

struct addrinfo
{
    int ai_flags;              /* Input flags. */
    int ai_family;             /* Address family of socket. */
    int ai_socktype;           /* Socket type. */
    int ai_protocol;           /* Protocol of socket. */
    int ai_addrlen;            /* Length of socket address. */
    struct sockaddr * ai_addr; /* Socket address of socket. */
    char * ai_canonname;       /* Canonical name of service location. */
    struct addrinfo * ai_next; /* Pointer to next in list. */
};

#define NETDB_ELEM_SIZE (sizeof(struct addrinfo) + sizeof(struct sockaddr_storage) + DNS_MAX_NAME_LENGTH + 1)

int getaddrinfo(const char * nodename, const char * servname, const struct addrinfo * hints, struct addrinfo ** res);
void freeaddrinfo(struct addrinfo * ai);

#ifdef __cplusplus
}
#endif

#endif
