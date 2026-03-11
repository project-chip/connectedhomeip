"""URI."""

# Read: https://stackoverflow.com/questions/176264
# https://www.rfc-editor.org/rfc/rfc3986#section-3

# local
from .email import email
from .url import url
from .utils import validator


def _file_url(value: str):
    if not value.startswith("file:///"):
        return False
    return True


def _ipfs_url(value: str):
    if not value.startswith("ipfs://"):
        return False
    return True


@validator
def uri(value: str, /):
    """Return whether or not given value is a valid URI.

    Examples:
        >>> uri('mailto:example@domain.com')
        True
        >>> uri('file:path.txt')
        ValidationError(func=uri, args={'value': 'file:path.txt'})

    Args:
        value:
            URI to validate.

    Returns:
        (Literal[True]): If `value` is a valid URI.
        (ValidationError): If `value` is an invalid URI.
    """
    if not value:
        return False

    # TODO: work on various validations

    # url
    if any(
        # fmt: off
        value.startswith(item)
        for item in {
            "ftp",
            "ftps",
            "git",
            "http",
            "https",
            "irc",
            "rtmp",
            "rtmps",
            "rtsp",
            "sftp",
            "ssh",
            "telnet",
        }
        # fmt: on
    ):
        return url(value)

    # email
    if value.startswith("mailto:"):
        return email(value[len("mailto:") :])

    # file
    if value.startswith("file:"):
        return _file_url(value)

    # ipfs
    if value.startswith("ipfs:"):
        return _ipfs_url(value)

    # magnet
    if value.startswith("magnet:?"):
        return True

    # telephone
    if value.startswith("tel:"):
        return True

    # data
    if value.startswith("data:"):
        return True

    # urn
    if value.startswith("urn:"):
        return True

    # urc
    if value.startswith("urc:"):
        return True

    return False
