# LwIP patches

This directory contains patch files for LwIP that enable required or desired
functionality in Matter. Patches that are not yet integrated into mainline LwIP
are provided as a patch file. This file also contains a list of helpful patches
that may not be present in vendor forks of the LwIP codebase

## Un-merged patches

### Route information options (RIO)

Required for wifi devices to support communication with a thread device behind
an open thread border router.

-   patch file: rio_patch_updated.patch
-   savannah link: https://savannah.nongnu.org/patch/?10114

Troubleshooting: The patch uses the `ip6_addr_net_eq` function, which is a
recent API change on upstream LwIP. The previous version of this function is
`ip6_addr_netcmp`, so this function call may need to be replaced on older forks.

## Important upstream patches

### Malformed neighbor solicitation packet fix

-   issue raised here
    [Issue 9791](https://github.com/project-chip/connectedhomeip/issues/9791)
-   fixed in matter fork by
    [Issue 10160](https://github.com/project-chip/connectedhomeip/pull/10160)
-   fixed in upstream by
    [PATCH](https://git.savannah.nongnu.org/cgit/lwip.git/commit/?id=bc08c1d2b79b4763fc0f8f0bf0ed58e0c2899b3a)

### DHCPv6 DNS bug

There was a bug in the DHCPv6 code where if the router sent a DNS using DHCPv6
stateless, it would set the DNS server as an ipv4 address, which ended up being
garbage. This would invalidate the whole DNS table, and lwip does not have a
default backup.

-   fixed in upstream in this
    [commit](https://git.savannah.nongnu.org/cgit/lwip.git/commit/?id=941300c21c45a4dbf1c074b29a9ca3c88c9f6553)
