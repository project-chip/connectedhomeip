#### Content

This library defines methods for resolving addresses of CHIP Nodes/Peers.

While dnssd is used for discovery, that method generally can yield several IP
addresses due to:

-   multiple IP addresses per interface (e.g. LL, GUA, ULA or even IPv4)
-   multiple interfaces per device

The purpose of address resolution is to find a _single_ ip address to use for
the given lookup. It employs a set of heuristics to determine what the best IP
(the most likely to route correctly) is and allows custom implementations from
applications by not including the default implementation.
