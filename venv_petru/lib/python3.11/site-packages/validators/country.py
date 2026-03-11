"""Country."""

# local
from validators.utils import validator

# fmt: off
_alpha3_to_alpha2 = {
    # A
    "ABW": "AW", "AFG": "AF", "AGO": "AO", "AIA": "AI", "ALB": "AL", "AND": "AD", "ANT": "AN",
    "ARE": "AE", "ARG": "AR", "ARM": "AM", "ASM": "AS", "ATA": "AQ", "ATF": "TF", "ATG": "AG",
    "AUS": "AU", "AUT": "AT", "AZE": "AZ",
    # B
    "BDI": "BI", "BEL": "BE", "BEN": "BJ", "BFA": "BF", "BGD": "BD", "BGR": "BG", "BHR": "BH",
    "BHS": "BS", "BIH": "BA", "BLR": "BY", "BLZ": "BZ", "BMU": "BM", "BOL": "BO", "BRA": "BR",
    "BRB": "BB", "BRN": "BN", "BTN": "BT", "BVT": "BV", "BWA": "BW",
    # C
    "CAF": "CF", "CAN": "CA", "CCK": "CC", "CHE": "CH", "CHL": "CL", "CHN": "CN", "CMR": "CM",
    "COD": "CD", "COG": "CG", "COK": "CK", "COL": "CO", "COM": "KM", "CPV": "CV", "CRI": "CR",
    "CUB": "CU", "CXR": "CX", "CYM": "KY", "CYP": "CY", "CZE": "CZ",
    # D
    "DEU": "DE", "DJI": "DJ", "DMA": "DM", "DNK": "DK", "DOM": "DO", "DZA": "DZ",
    # E
    "ECU": "EC", "EGY": "EG", "ERI": "ER", "ESH": "EH", "ESP": "ES", "EST": "EE", "ETH": "ET",
    # F
    "FIN": "FI", "FJI": "FJ", "FLK": "FK", "FRA": "FR", "FRO": "FO", "FSM": "FM",
    # G
    "GAB": "GA", "GBR": "GB", "GEO": "GE", "GGY": "GG", "GHA": "GH", "GIB": "GI", "GIN": "GN",
    "GLP": "GP", "GMB": "GM", "GNB": "GW", "GNQ": "GQ", "GRC": "GR", "GRD": "GD", "GRL": "GL",
    "GTM": "GT", "GUF": "GF", "GUM": "GU", "GUY": "GY",
    # H
    "HKG": "HK", "HMD": "HM", "HND": "HN", "HRV": "HR", "HTI": "HT", "HUN": "HU",
    # I
    "IDN": "ID", "IMN": "IM", "IND": "IN", "IOT": "IO", "IRL": "IE", "IRN": "IR", "IRQ": "IQ",
    "ISL": "IS", "ISR": "IL", "ITA": "IT",
    # J
    "JAM": "JM", "JEY": "JE", "JOR": "JO", "JPN": "JP",
    # K
    "KAZ": "KZ", "KEN": "KE", "KGZ": "KG", "KHM": "KH", "KIR": "KI", "KNA": "KN", "KOR": "KR",
    "KWT": "KW",
    # L
    "LAO": "LA", "LBN": "LB", "LBR": "LR", "LBY": "LY", "LCA": "LC", "LIE": "LI", "LKA": "LK",
    "LSO": "LS", "LTU": "LT", "LUX": "LU", "LVA": "LV",
    # M
    "MAC": "MO", "MAR": "MA", "MCO": "MC", "MDA": "MD", "MDG": "MG", "MDV": "MV", "MEX": "MX",
    "MHL": "MH", "MKD": "MK", "MLI": "ML", "MLT": "MT", "MMR": "MM", "MNE": "ME", "MNG": "MN",
    "MNP": "MP", "MOZ": "MZ", "MRT": "MR", "MSR": "MS", "MTQ": "MQ", "MUS": "MU", "MWI": "MW",
    "MYS": "MY", "MYT": "YT",
    # N
    "NAM": "NA", "NCL": "NC", "NER": "NE", "NFK": "NF", "NGA": "NG", "NIC": "NI", "NIU": "NU",
    "NLD": "NL", "NOR": "NO", "NPL": "NP", "NRU": "NR", "NZL": "NZ",
    # O
    "OMN": "OM",
    # P
    "PAK": "PK", "PAN": "PA", "PCN": "PN", "PER": "PE", "PHL": "PH", "PLW": "PW", "PNG": "PG",
    "POL": "PL", "PRI": "PR", "PRK": "KP", "PRT": "PT", "PRY": "PY", "PSE": "PS", "PYF": "PF",
    # Q
    "QAT": "QA",
    # R
    "REU": "RE", "ROU": "RO", "RUS": "RU", "RWA": "RW",
    # S
    "SAU": "SA", "SDN": "SD", "SEN": "SN", "SGP": "SG", "SGS": "GS", "SHN": "SH", "SJM": "SJ",
    "SLB": "SB", "SLE": "SL", "SLV": "SV", "SMR": "SM", "SOM": "SO", "SPM": "PM", "SRB": "RS",
    "STP": "ST", "SUR": "SR", "SVK": "SK", "SVN": "SI", "SWE": "SE", "SWZ": "SZ", "SYC": "SC",
    "SYR": "SY",
    # T
    "TCA": "TC", "TCD": "TD", "TGO": "TG", "THA": "TH", "TJK": "TJ", "TKL": "TK", "TKM": "TM",
    "TLS": "TL", "TON": "TO", "TTO": "TT", "TUN": "TN", "TUR": "TR", "TUV": "TV", "TWN": "TW",
    "TZA": "TZ",
    # U
    "UGA": "UG", "UKR": "UA", "UMI": "UM", "URY": "UY", "USA": "US", "UZB": "UZ",
    # V
    "VAT": "VA", "VCT": "VC", "VEN": "VE", "VGB": "VG", "VIR": "VI", "VNM": "VN", "VUT": "VU",
    # W
    "WLF": "WF", "WSM": "WS",
    # Y
    "YEM": "YE",
    # Z
    "ZAF": "ZA", "ZMB": "ZM", "ZWE": "ZW",
}
_calling_codes = {
    # A
    "ABW": "+297", "AFG": "+93", "AGO": "+244", "AIA": "+1-264", "ALB": "+355", "AND": "+376",
    "ANT": "+599", "ARE": "+971", "ARG": "+54", "ARM": "+374", "ASM": "+1-684", "ATA": "+672",
    "ATG": "+1-268", "AUS": "+61", "AUT": "+43", "AZE": "+994",
    # B
    "BDI": "+257", "BEL": "+32", "BEN": "+229", "BFA": "+226", "BGD": "+880", "BGR": "+359",
    "BHR": "+973", "BHS": "+1-242", "BIH": "+387", "BLR": "+375", "BLZ": "+501",
    "BMU": "+1-441", "BOL": "+591", "BRA": "+55", "BRB": "+1-246", "BRN": "+673", "BTN": "+975",
    "BWA": "+267",
    # C
    "CAF": "+236", "CAN": "+1", "CCK": "+61", "CHE": "+41", "CHL": "+56", "CHN": "+86",
    "CMR": "+237", "COD": "+243", "COG": "+242", "COK": "+682", "COL": "+57", "COM": "+269",
    "CPV": "+238", "CRI": "+506", "CUB": "+53", "CXR": "+61", "CYM": "+1-345", "CYP": "+357",
    "CZE": "+420",
    # D
    "DEU": "+49", "DJI": "+253", "DMA": "+1-767", "DNK": "+45", "DOM": "+1-809", "DZA": "+213",
    # E
    "ECU": "+593", "EGY": "+20", "ERI": "+291", "ESH": "+212", "ESP": "+34", "EST": "+372",
    "ETH": "+251",
    # F
    "FIN": "+358", "FJI": "+679", "FLK": "+500", "FRA": "+33", "FRO": "+298", "FSM": "+691",
    # G
    "GAB": "+241", "GBR": "+44", "GEO": "+995", "GGY": "+44-1481", "GHA": "+233", "GIB": "+350",
    "GIN": "+224", "GLP": "+590", "GMB": "+220", "GNB": "+245", "GNQ": "+240", "GRC": "+30",
    "GRD": "+1-473", "GRL": "+299", "GTM": "+502", "GUF": "+594", "GUM": "+1-671",
    "GUY": "+592",
    # H
    "HKG": "+852", "HMD": "+672", "HND": "+504", "HRV": "+385", "HTI": "+509", "HUN": "+36",
    # I
    "IDN": "+62", "IMN": "+44-1624", "IND": "+91", "IOT": "+246", "IRL": "+353", "IRN": "+98",
    "IRQ": "+964", "ISL": "+354", "ISR": "+972", "ITA": "+39",
    # J
    "JAM": "+1-876", "JEY": "+44-1534", "JOR": "+962", "JPN": "+81",
    # K
    "KAZ": "+7", "KEN": "+254", "KGZ": "+996", "KHM": "+855", "KIR": "+686", "KNA": "+1-869",
    "KOR": "+82", "KWT": "+965",
    # L
    "LAO": "+856", "LBN": "+961", "LBR": "+231", "LBY": "+218", "LCA": "+1-758", "LIE": "+423",
    "LKA": "+94", "LSO": "+266", "LTU": "+370", "LUX": "+352", "LVA": "+371",
    # M
    "MAC": "+853", "MAR": "+212", "MCO": "+377", "MDA": "+373", "MDG": "+261", "MDV": "+960",
    "MEX": "+52", "MHL": "+692", "MKD": "+389", "MLI": "+223", "MLT": "+356", "MMR": "+95",
    "MNE": "+382", "MNG": "+976", "MNP": "+1-670", "MOZ": "+258", "MRT": "+222",
    "MSR": "+1-664", "MTQ": "+596", "MUS": "+230", "MWI": "+265", "MYS": "+60", "MYT": "+262",
    # N
    "NAM": "+264", "NCL": "+687", "NER": "+227", "NFK": "+672", "NGA": "+234", "NIC": "+505",
    "NIU": "+683", "NLD": "+31", "NOR": "+47", "NPL": "+977", "NRU": "+674", "NZL": "+64",
    # O
    "OMN": "+968",
    # P
    "PAK": "+92", "PAN": "+507", "PCN": "+64", "PER": "+51", "PHL": "+63", "PLW": "+680",
    "PNG": "+675", "POL": "+48", "PRI": "+1-787", "PRK": "+850", "PRT": "+351", "PRY": "+595",
    "PSE": "+970", "PYF": "+689",
    # Q
    "QAT": "+974",
    # R
    "REU": "+262", "ROU": "+40", "RUS": "+7", "RWA": "+250",
    # S
    "SAU": "+966", "SDN": "+249", "SEN": "+221", "SGP": "+65", "SHN": "+290", "SJM": "+47",
    "SLB": "+677", "SLE": "+232", "SLV": "+503", "SMR": "+378", "SOM": "+252", "SPM": "+508",
    "SRB": "+381", "STP": "+239", "SUR": "+597", "SVK": "+421", "SVN": "+386", "SWE": "+46",
    "SWZ": "+268", "SYC": "+248", "SYR": "+963",
    # T
    "TCA": "+1-649", "TCD": "+235", "TGO": "+228", "THA": "+66", "TJK": "+992", "TKL": "+690",
    "TKM": "+993", "TLS": "+670", "TON": "+676", "TTO": "+1-868", "TUN": "+216", "TUR": "+90",
    "TUV": "+688", "TWN": "+886", "TZA": "+255",
    # U
    "UGA": "+256", "UKR": "+380", "UMI": "+1", "URY": "+598", "USA": "+1", "UZB": "+998",
    # V
    "VAT": "+379", "VCT": "+1-784", "VEN": "+58", "VGB": "+1-284", "VIR": "+1-340",
    "VNM": "+84", "VUT": "+678",
    # W
    "WLF": "+681", "WSM": "+685",
    # Y
    "YEM": "+967",
    # Z
    "ZAF": "+27", "ZMB": "+260", "ZWE": "+263"
}
_numeric = {
    "004", "008", "010", "012", "016", "020", "024", "028", "031", "032",
    "036", "040", "044", "048", "050", "051", "052", "056", "060", "064",
    "068", "070", "072", "074", "076", "084", "086", "090", "092", "096",
    "100", "104", "108", "112", "116", "120", "124", "132", "136", "140",
    "144", "148", "152", "156", "158", "162", "166", "170", "174", "175",
    "178", "180", "184", "188", "191", "192", "196", "203", "204", "208",
    "212", "214", "218", "222", "226", "231", "232", "233", "234", "238",
    "239", "242", "246", "248", "250", "254", "258", "260", "262", "266",
    "268", "270", "275", "276", "288", "292", "296", "300", "304", "308",
    "312", "316", "320", "324", "328", "332", "334", "340", "344", "348",
    "352", "356", "360", "364", "368", "372", "376", "380", "384", "388",
    "392", "398", "400", "404", "408", "410", "414", "417", "418", "422",
    "426", "428", "430", "434", "438", "440", "442", "446", "450", "454",
    "458", "462", "466", "470", "474", "478", "480", "484", "492", "496",
    "498", "499", "500", "504", "508", "512", "516", "520", "524", "528",
    "531", "533", "534", "535", "540", "548", "554", "558", "562", "566",
    "570", "574", "578", "580", "581", "583", "584", "585", "586", "591",
    "598", "600", "604", "608", "612", "616", "620", "624", "626", "630",
    "634", "638", "642", "643", "646", "652", "654", "659", "660", "662",
    "663", "666", "670", "674", "678", "682", "686", "688", "690", "694",
    "702", "703", "704", "705", "706", "710", "716", "724", "728", "729",
    "732", "740", "744", "748", "752", "756", "760", "762", "764", "768",
    "772", "776", "780", "784", "788", "792", "795", "796", "798", "800",
    "804", "807", "818", "826", "831", "832", "833", "834", "840", "850",
    "854", "858", "860", "862", "876", "882", "887", "894",
}
_currency_iso4217 = {
    # https://en.wikipedia.org/wiki/ISO_4217
    "AED", "AFN", "ALL", "AMD", "ANG", "AOA", "ARS", "AUD", "AWG", "AZN",
    "BAM", "BBD", "BDT", "BGN", "BHD", "BIF", "BMD", "BND", "BOB", "BOV", "BRL", "BSD", "BTN",
    "BWP", "BYN", "BZD",
    "CAD", "CDF", "CHE", "CHF", "CHW", "CKD", "CLF", "CLP", "CNY", "COP", "CRC", "CUC", "CUP",
    "CVE", "CZK",
    "DJF", "DKK", "DOP", "DZD",
    "EGP", "ERN", "ETB", "EUR",
    "FJD", "FKP",
    "GBP", "GEL", "GHS", "GIP", "GMD", "GNF", "GTQ", "GYD",
    "HKD", "HNL", "HRK", "HTG", "HUF",
    "IDR", "IEP", "ILS", "INR", "IQD", "IRR", "ISK",
    "JMD", "JOD", "JPY",
    "KES", "KGS", "KHR", "KID", "KMF", "KPW", "KRW", "KWD", "KYD", "KZT",
    "LAK", "LBP", "LKR", "LRD", "LSL", "LYD",
    "MAD", "MDL", "MGA", "MKD", "MMK", "MNT", "MOP", "MRU", "MUR", "MVR",
    "MWK", "MXN", "MYR", "MZN",
    "NAD", "NGN", "NIO", "NOK", "NPR", "NZD",
    "OMR",
    "PAB", "PEN", "PGK", "PHP", "PKR", "PLN", "PYG",
    "QAR",
    "RON", "RSD", "RUB", "RWF",
    "SAR", "SBD", "SCR", "SDG", "SEK", "SGD", "SHP", "SLL", "SOS",
    "SRD", "SSP", "STN", "SVC", "SYP", "SZL",
    "THB", "TJS", "TMT", "TND", "TOP", "TRY", "TTD", "TWD", "TZS",
    "UAH", "UGX", "USD", "UYU", "UZS",
    "VED", "VES", "VND", "VUV",
    "WST",
    "XAF", "XCD", "XDR", "XOF", "XPF",
    "YER",
    "ZAR", "ZMW", "ZWL"
}
_currency_symbols = {
    # https://en.wikipedia.org/wiki/Currency_sign_(generic)
    "؋", "฿", "₵", "₡", "¢", "$", "₫", "֏", "€", "ƒ", "₣", "₲", "₴", "₭", "₾", "£", "₺", "₼", "₦",
    "₱", "元", "圆", "圓", "﷼", "៛", "₽", "₹", "रू", "රු", "૱", "௹", "꠸", "Rs", "₪", "⃀" "৳", "₸",
    "₮", "₩", "¥", "円", "₿", "¤"
}
# fmt: on


def _get_code_type(format_type: str):
    """Returns the type of country code."""
    if format_type.isdecimal():
        return "numeric"
    if format_type.isalpha():
        if len(format_type) == 2:
            return "alpha2"
        if len(format_type) == 3:
            return "alpha3"
    return "invalid"


@validator
def calling_code(value: str, /):
    """Validates given calling code.

    This performs country's calling code validation.

    Examples:
        >>> calling_code('+91')
        True
        >>> calling_code('-31')
        ValidationError(func=calling_code, args={'value': '-31'})

    Args:
        value:
            Country's calling code string to validate.

    Returns:
        (Literal[True]): If `value` is a valid calling code.
        (ValidationError): If `value` is an invalid calling code.
    """
    if not value:
        return False

    return value in set(_calling_codes.values())


@validator
def country_code(value: str, /, *, iso_format: str = "auto", ignore_case: bool = False):
    """Validates given country code.

    This performs a case-sensitive [ISO 3166][1] country code validation.

    [1]: https://www.iso.org/iso-3166-country-codes.html

    Examples:
        >>> country_code('GB', iso_format='alpha3')
        ValidationError(func=country_code, args={'value': 'GB', 'iso_format': 'alpha3'})
        >>> country_code('USA')
        True
        >>> country_code('840', iso_format='numeric')
        True
        >>> country_code('iN', iso_format='alpha2')
        ValidationError(func=country_code, args={'value': 'iN', 'iso_format': 'alpha2'})
        >>> country_code('ZWE', iso_format='alpha3')
        True

    Args:
        value:
            Country code string to validate.
        iso_format:
            ISO format to be used. Available options are:
            `auto`, `alpha2`, `alpha3` and `numeric`.
        ignore_case:
            Enable/Disable case-sensitive matching.

    Returns:
        (Literal[True]): If `value` is a valid country code.
        (ValidationError): If `value` is an invalid country code.
    """
    if not value:
        return False

    if not (1 < len(value) < 4):
        return False

    if iso_format == "auto" and (iso_format := _get_code_type(value)) == "invalid":
        return False

    if iso_format == "alpha2":
        return (
            value.upper() in set(_alpha3_to_alpha2.values())
            if ignore_case
            else value in set(_alpha3_to_alpha2.values())
        )
    if iso_format == "alpha3":
        return value.upper() in _alpha3_to_alpha2 if ignore_case else value in _alpha3_to_alpha2

    return value in _numeric if iso_format == "numeric" else False


@validator
def currency(value: str, /, *, skip_symbols: bool = True, ignore_case: bool = False):
    """Validates given currency code.

    This performs [ISO 4217][1] currency code/symbol validation.

    [1]: https://www.iso.org/iso-4217-currency-codes.html

    Examples:
        >>> currency('USD')
        True
        >>> currency('ZWX')
        ValidationError(func=currency, args={'value': 'ZWX'})

    Args:
        value:
            Currency code/symbol string to validate.
        skip_symbols:
            Skip currency symbol validation.
        ignore_case:
            Enable/Disable case-sensitive matching.

    Returns:
        (Literal[True]): If `value` is a valid currency code.
        (ValidationError): If `value` is an invalid currency code.
    """
    if not value:
        return False

    if not skip_symbols and value in _currency_symbols:
        return True

    if len(value) != 3:
        return False

    return value.upper() in _currency_iso4217 if ignore_case else value in _currency_iso4217
