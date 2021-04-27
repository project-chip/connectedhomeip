#ifndef MBED_POSIX_NETDB_H
#define MBED_POSIX_NETDB_H

#ifdef __cplusplus
extern "C" {
#endif

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

/* input flags for struct addrinfo */
#define AI_PASSIVE 0x01
#define AI_CANONNAME 0x02
#define AI_NUMERICHOST 0x04
#define AI_NUMERICSERV 0x08
#define AI_V4MAPPED 0x10
#define AI_ALL 0x20
#define AI_ADDRCONFIG 0x40

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

int getaddrinfo(const char * nodename, const char * servname, const struct addrinfo * hints, struct addrinfo ** res);
void freeaddrinfo(struct addrinfo * ai);

#ifdef __cplusplus
}
#endif

#endif /* MBED_POSIX_NETDB_H */
