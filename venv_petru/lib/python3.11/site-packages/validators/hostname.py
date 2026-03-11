"""Hostname."""

# standard
from functools import lru_cache
import re
from typing import Optional

from .domain import domain

# local
from .ip_address import ipv4, ipv6
from .utils import validator


@lru_cache
def _port_regex():
    """Port validation regex."""
    return re.compile(
        r"^\:(6553[0-5]|655[0-2][0-9]|65[0-4][0-9]{2}|"
        + r"6[0-4][0-9]{3}|[1-5][0-9]{4}|[1-9][0-9]{0,3})$",
    )


@lru_cache
def _simple_hostname_regex():
    """Simple hostname validation regex."""
    # {0,59} because two characters are already matched at
    # the beginning and at the end, making the range {1, 61}
    return re.compile(r"^(?!-)[a-z0-9](?:[a-z0-9-]{0,59}[a-z0-9])?(?<!-)$", re.IGNORECASE)


def _port_validator(value: str):
    """Returns host segment if port is valid."""
    if value.count("]:") == 1:
        # with ipv6
        host_seg, port_seg = value.rsplit(":", 1)
        if _port_regex().match(f":{port_seg}"):
            return host_seg.lstrip("[").rstrip("]")

    if value.count(":") == 1:
        # with ipv4 or simple hostname
        host_seg, port_seg = value.rsplit(":", 1)
        if _port_regex().match(f":{port_seg}"):
            return host_seg

    return None


@validator
def hostname(
    value: str,
    /,
    *,
    skip_ipv6_addr: bool = False,
    skip_ipv4_addr: bool = False,
    may_have_port: bool = True,
    maybe_simple: bool = True,
    consider_tld: bool = False,
    private: Optional[bool] = None,  # only for ip-addresses
    rfc_1034: bool = False,
    rfc_2782: bool = False,
):
    """Return whether or not given value is a valid hostname.

    Examples:
        >>> hostname("ubuntu-pc:443")
        True
        >>> hostname("this-pc")
        True
        >>> hostname("xn----gtbspbbmkef.xn--p1ai:65535")
        True
        >>> hostname("_example.com")
        ValidationError(func=hostname, args={'value': '_example.com'})
        >>> hostname("123.5.77.88:31000")
        True
        >>> hostname("12.12.12.12")
        True
        >>> hostname("[::1]:22")
        True
        >>> hostname("dead:beef:0:0:0:0000:42:1")
        True
        >>> hostname("[0:0:0:0:0:ffff:1.2.3.4]:-65538")
        ValidationError(func=hostname, args={'value': '[0:0:0:0:0:ffff:1.2.3.4]:-65538'})
        >>> hostname("[0:&:b:c:@:e:f::]:9999")
        ValidationError(func=hostname, args={'value': '[0:&:b:c:@:e:f::]:9999'})

    Args:
        value:
            Hostname string to validate.
        skip_ipv6_addr:
            When hostname string cannot be an IPv6 address.
        skip_ipv4_addr:
            When hostname string cannot be an IPv4 address.
        may_have_port:
            Hostname string may contain port number.
        maybe_simple:
            Hostname string maybe only hyphens and alpha-numerals.
        consider_tld:
            Restrict domain to TLDs allowed by IANA.
        private:
            Embedded IP address is public if `False`, private/local if `True`.
        rfc_1034:
            Allow trailing dot in domain/host name.
            Ref: [RFC 1034](https://www.rfc-editor.org/rfc/rfc1034).
        rfc_2782:
            Domain/Host name is of type service record.
            Ref: [RFC 2782](https://www.rfc-editor.org/rfc/rfc2782).

    Returns:
        (Literal[True]): If `value` is a valid hostname.
        (ValidationError): If `value` is an invalid hostname.
    """
    if not value:
        return False

    if may_have_port and (host_seg := _port_validator(value)):
        return (
            (_simple_hostname_regex().match(host_seg) if maybe_simple else False)
            or domain(host_seg, consider_tld=consider_tld, rfc_1034=rfc_1034, rfc_2782=rfc_2782)
            or (False if skip_ipv4_addr else ipv4(host_seg, cidr=False, private=private))
            or (False if skip_ipv6_addr else ipv6(host_seg, cidr=False))
        )

    return (
        (_simple_hostname_regex().match(value) if maybe_simple else False)
        or domain(value, consider_tld=consider_tld, rfc_1034=rfc_1034, rfc_2782=rfc_2782)
        or (False if skip_ipv4_addr else ipv4(value, cidr=False, private=private))
        or (False if skip_ipv6_addr else ipv6(value, cidr=False))
    )
