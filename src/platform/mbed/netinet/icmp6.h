#ifndef _NETINET_ICMP6_H_
#define _NETINET_ICMP6_H_

#define IPV6_UNICAST_HOPS 16
#define IPV6_MULTICAST_IF 17
#define IPV6_MULTICAST_HOPS 18
#define ICMP6_FILTER 1
struct icmp6_filter
{
};

void ICMP6_FILTER_SETPASSALL(struct icmp6_filter *) {}

void ICMP6_FILTER_SETBLOCKALL(struct icmp6_filter *) {}

void ICMP6_FILTER_SETPASS(int, struct icmp6_filter *) {}

#endif /* _NETINET_ICMP6_H_ */