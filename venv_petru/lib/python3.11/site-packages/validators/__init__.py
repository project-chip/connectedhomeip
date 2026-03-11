"""Validate Anything!"""

# local
from .between import between
from .card import amex, card_number, diners, discover, jcb, mastercard, mir, unionpay, visa
from .country import calling_code, country_code, currency
from .cron import cron
from .crypto_addresses import bsc_address, btc_address, eth_address, trx_address
from .domain import domain
from .email import email
from .encoding import base16, base32, base58, base64
from .finance import cusip, isin, sedol
from .hashes import md5, sha1, sha224, sha256, sha384, sha512
from .hostname import hostname
from .i18n import (
    es_cif,
    es_doi,
    es_nie,
    es_nif,
    fi_business_id,
    fi_ssn,
    fr_department,
    fr_ssn,
    ind_aadhar,
    ind_pan,
    ru_inn,
)
from .iban import iban
from .ip_address import ipv4, ipv6
from .length import length
from .mac_address import mac_address
from .slug import slug
from .url import url
from .utils import ValidationError, validator
from .uuid import uuid

__all__ = (
    # ...
    "between",
    # crypto_addresses
    "bsc_address",
    "btc_address",
    "eth_address",
    "trx_address",
    # cards
    "amex",
    "card_number",
    "diners",
    "discover",
    "jcb",
    "mastercard",
    "unionpay",
    "visa",
    "mir",
    # country
    "calling_code",
    "country_code",
    "currency",
    # ...
    "cron",
    # ...
    "domain",
    # ...
    "email",
    # encodings
    "base16",
    "base32",
    "base58",
    "base64",
    # finance
    "cusip",
    "isin",
    "sedol",
    # hashes
    "md5",
    "sha1",
    "sha224",
    "sha256",
    "sha384",
    "sha512",
    # ...
    "hostname",
    # i18n
    "es_cif",
    "es_doi",
    "es_nie",
    "es_nif",
    "fi_business_id",
    "fi_ssn",
    "fr_department",
    "fr_ssn",
    "ind_aadhar",
    "ind_pan",
    "ru_inn",
    # ...
    "iban",
    # ip_addresses
    "ipv4",
    "ipv6",
    # ...
    "length",
    # ...
    "mac_address",
    # ...
    "slug",
    # ...
    "url",
    # ...
    "uuid",
    # utils
    "ValidationError",
    "validator",
)

__version__ = "0.35.0"
