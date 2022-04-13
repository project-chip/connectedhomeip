# LwIP changes for Matter

LwIP is one of the network layers used in the Matter platform. Although it has
some good IPv6 support, there are areas that are lacking that we should
implement for Matter. The recommendations here are listed roughly from most to
least important.

## Route Information Options (RIO)

The specification requires devices to store route options from Route Information
Options (RIO) sent in router advertisements. This functionality is not currently
present in upstream LwIP. The patch to add this is relatively small, but we may
need to upstream this in order to require its use in Matter. Platforms would
need to incorporate this into their own middleware

### Recommendation:

-   write a RIO patch, upstream to lwip
    -   Ensure patch is RFC compliant (especially re: expiry)
-   UPDATE: Patch is available at https://savannah.nongnu.org/patch/?10114

## Address Scopes

Link local addresses are less common on IPv4, which normally rely on NAT at the
router to do address translation. Matter mandates the use of IPv6 link local
addresses for communication to nodes on the same network (wifi or thread). When
there is more than one netif in the system (ex. loopback, softAP, STA), the link
local address needs more information to determine which link the address is
local to. This is normally added as the link local scope and can be seen on
addresses ex. `FE80::xxxx:xxxx:xxxx:xxxx%<scope>`, where the <scope> identifies
the netif (something like `%wlan0` or `%eno1` etc.).

Without this indicator, the link local address can only be resolved if there is
one netif. LwIP will also allow a direct address match to the netif source
address, but this does not scale well at all and is VERY racy. LwIP also
supports output to a specific netif, but this is not brought up to the socket
layer.

Upstream LwIP has support for IPv6 address scopes, but only as an option.
However, the code to support this is not present in the CHIP LwIP codebase.
Other platform versions assume this option is not present (ex. M5 has an
assertion on ip address sizes that disallow the use of a scope tag).

### Recommendation:

-   Ensure Matter SDK code works with scopes on our various platforms OR
    alternate: bring netif sendto up through the api / sockets layers
-   Audit Matter code to ensure LL addresses are properly scoped to their netif
    in all areas (DNS returned addresses especially)

## Duplicate address detection

The DAD in LwIP is actually implemented correctly right now, but there are
routers that incorrectly implement multicast for IPv6 and send packets back to
the sender. This triggers the LwIP DAD because it doesn’t check the source. This
can be fixed in the wifi layer as a filter, but it’s easy enough to add the fix
into the LwIP layer. This would help implementers so they don’t all have to
debug the same issues. Recommendation:

-   Create an LwIP patch to check NS/NA packets for source and discard if they
    originate from the same device. Upstream and offer patch to vendors.

## Timers, including TCP

lwIP uses on-demand timers for IGMP and MLD (see
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/lwip.html#esp-lwip-custom-modifications
for changes Espressif made to lwIP to help power usage on ESP32 and better
support IPv6), and also has several uncorrelated always-on timers for TCP. These
timers have caused power issues on some products.

### Recommendation:

-   Make sure to take-in Espressif improvements to timers (not sure they are
    upstreamed)
-   Look into supporting aligned TCP timers to aggregate multiple timers within
    a single wake

## pbuf management

Pool-based management has been a source of problems on several products, but
does have advantages over purely “heap” based allocation of pbufs as done in
ESP32 and many common lwIP stacks.

Overall, having the ability to instrument all PBUF allocations for usage (e.g.
Driver TX, Driver RX, Manual PacketBuffer allocation, internal TCP stack pbufs,
etc) would allow us to move towards a pool approach by allowing us to track the
following: Understanding of the overall memory usage of lwIP packet buffers over
time, helping debug issues related to out-of-pbuf or overly-long queuing. Keep
track of incoming packets dwelling and outgoing packets dwelling to start
dropping at ingress when running out of memory Overall, allow sizing of heap and
pools based on usage patterns.

### Recommendation:

-   Upstream a portable version of pbuf alloc/free accounting, allowing
    registration of instrumentation handlers.
-   Add support to account for high watermark of pbuf memory used and concurrent
    pbuf allocations.
-   Add more pbuf allocation types to allow finer-grained recording of “reason”
    for a pbuf alloc

## IPv6 Ping

Although ping is not required for Matter, it is very helpful for debugging
networking issues. Having a reliable ping would be beneficial for a lot of
developers.

LwIP will automatically respond to pings, but has no built-in way to send them.
The current ping implementation is a contrib app that only works for IPv4.
Extending the app is challenging for two reasons: 1) IPv6 checksum needs access
to the pbuf for calculation, which the app doesn’t have and 2) IPv6 has a lot
more ICMP traffic for SLAAC that the app would have to be updated to disregard.
Instead, it might be better to build this into the ICMP layer itself.

### Recommendation:

-   Add an ASYNC send_icmp6_ping function and add a hook to check ping
    responses. Upstream patch if possible. OR write an external ICMP6 ping util

## DNS

LwIP's DNS handling isn’t great and breaks down when the router supports
IPv4/IPv6. There is a single list of DNS servers, DHCP, SLAAC and DHCPv6 all
update the list without locks. Basically, whatever wrote to the list last gets
to set the list. Although there is handling for IP type (requesting A or `AAAA`
records), there isn’t handling to specify an IPv6 or IPv4 server specifically,
which can be challenging since not all servers serve all record types.

The design of the weave connectivity manager moves the DNS selection to the
upper layers by stopping lwip from directly changing the DNS list and hooking to
the DNS selection. This means the DNS selection policy isn’t hard-coded into the
lwip layer. This seems like a good model for CHIP going forward.

Additionally, we should ensure that CHIP uses non-blocking DNS APIs.

### Recommendation:

-   bug fix for DHCPv6 to avoid it setting bad addresses.
    -   note - fixed in
        https://git.savannah.nongnu.org/cgit/lwip.git/commit/?id=941300c21c45a4dbf1c074b29a9ca3c88c9f6553,
        but not yet released as a part of an official release.
-   Create a patch to add hooks to the SetDns and GetDns functions so logic for
    selecting the DNS server can be moved into the manager layer
