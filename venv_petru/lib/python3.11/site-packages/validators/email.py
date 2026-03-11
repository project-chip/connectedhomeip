"""eMail."""

# standard
import re

# local
from .hostname import hostname
from .utils import validator


@validator
def email(
    value: str,
    /,
    *,
    ipv6_address: bool = False,
    ipv4_address: bool = False,
    simple_host: bool = False,
    rfc_1034: bool = False,
    rfc_2782: bool = False,
):
    """Validate an email address.

    This was inspired from [Django's email validator][1].
    Also ref: [RFC 1034][2], [RFC 5321][3] and [RFC 5322][4].

    [1]: https://github.com/django/django/blob/main/django/core/validators.py#L174
    [2]: https://www.rfc-editor.org/rfc/rfc1034
    [3]: https://www.rfc-editor.org/rfc/rfc5321
    [4]: https://www.rfc-editor.org/rfc/rfc5322

    Examples:
        >>> email('someone@example.com')
        True
        >>> email('bogus@@')
        ValidationError(func=email, args={'value': 'bogus@@'})

    Args:
        value:
            eMail string to validate.
        ipv6_address:
            When the domain part is an IPv6 address.
        ipv4_address:
            When the domain part is an IPv4 address.
        simple_host:
            When the domain part is a simple hostname.
        rfc_1034:
            Allow trailing dot in domain name.
            Ref: [RFC 1034](https://www.rfc-editor.org/rfc/rfc1034).
        rfc_2782:
            Domain name is of type service record.
            Ref: [RFC 2782](https://www.rfc-editor.org/rfc/rfc2782).

    Returns:
        (Literal[True]): If `value` is a valid eMail.
        (ValidationError): If `value` is an invalid eMail.
    """
    if not value or value.count("@") != 1:
        return False

    username_part, domain_part = value.rsplit("@", 1)

    if len(username_part) > 64 or len(domain_part) > 253:
        # ref: RFC 1034 and 5231
        return False

    if ipv6_address or ipv4_address:
        if domain_part.startswith("[") and domain_part.endswith("]"):
            # ref: RFC 5321
            domain_part = domain_part.lstrip("[").rstrip("]")
        else:
            return False

    return (
        bool(
            hostname(
                domain_part,
                skip_ipv6_addr=not ipv6_address,
                skip_ipv4_addr=not ipv4_address,
                may_have_port=False,
                maybe_simple=simple_host,
                rfc_1034=rfc_1034,
                rfc_2782=rfc_2782,
            )
        )
        if re.match(
            # extended latin
            r"(^[\u0100-\u017F\u0180-\u024F\u00A0-\u00FF]"
            # dot-atom
            + r"|[\u0100-\u017F\u0180-\u024F\u00A0-\u00FF0-9a-z!#$%&'*+/=?^_`{}|~\-]+"
            + r"(\.[\u0100-\u017F\u0180-\u024F\u00A0-\u00FF0-9a-z!#$%&'*+/=?^_`{}|~\-]+)*$"
            # quoted-string
            + r'|^"('
            + r"[\u0100-\u017F\u0180-\u024F\u00A0-\u00FF\001-\010\013\014\016-\037"
            + r"!#-\[\]-\177]|\\[\011.]"
            + r')*")$',
            username_part,
            re.IGNORECASE,
        )
        else False
    )
