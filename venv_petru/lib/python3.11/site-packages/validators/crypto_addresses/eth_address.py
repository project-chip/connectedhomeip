"""ETH Address."""

# standard
import re

# local
from validators.utils import validator

_keccak_flag = True
try:
    # external
    from eth_hash.auto import keccak
except ImportError:
    _keccak_flag = False


def _validate_eth_checksum_address(addr: str):
    """Validate ETH type checksum address."""
    addr = addr.replace("0x", "")
    addr_hash = keccak.new(addr.lower().encode("ascii")).digest().hex()  # type: ignore

    if len(addr) != 40:
        return False

    for i in range(0, 40):
        if (int(addr_hash[i], 16) > 7 and addr[i].upper() != addr[i]) or (
            int(addr_hash[i], 16) <= 7 and addr[i].lower() != addr[i]
        ):
            return False
    return True


@validator
def eth_address(value: str, /):
    """Return whether or not given value is a valid ethereum address.

    Full validation is implemented for ERC20 addresses.

    Examples:
        >>> eth_address('0x9cc14ba4f9f68ca159ea4ebf2c292a808aaeb598')
        True
        >>> eth_address('0x8Ba1f109551bD432803012645Ac136ddd64DBa72')
        ValidationError(func=eth_address, args={'value': '0x8Ba1f109551bD432803012645Ac136ddd64DBa72'})

    Args:
        value:
            Ethereum address string to validate.

    Returns:
        (Literal[True]): If `value` is a valid ethereum address.
        (ValidationError): If `value` is an invalid ethereum address.
    """  # noqa: E501
    if not _keccak_flag:
        raise ImportError(
            "Do `pip install validators[crypto-eth-addresses]` to perform `eth_address` validation."
        )

    if not value:
        return False

    return re.compile(r"^0x[0-9a-f]{40}$|^0x[0-9A-F]{40}$").match(
        value
    ) or _validate_eth_checksum_address(value)
