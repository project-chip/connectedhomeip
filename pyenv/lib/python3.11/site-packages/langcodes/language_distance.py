from .data_dicts import LANGUAGE_DISTANCES
from typing import Dict, Tuple


TagTriple = Tuple[str, str, str]
_DISTANCE_CACHE: Dict[Tuple[TagTriple, TagTriple], int] = {}
DEFAULT_LANGUAGE_DISTANCE = LANGUAGE_DISTANCES["*"]["*"]
DEFAULT_SCRIPT_DISTANCE = LANGUAGE_DISTANCES["*_*"]["*_*"]
DEFAULT_TERRITORY_DISTANCE = 4


# Territory clusters used in territory matching:
# Maghreb (the western Arab world)
MAGHREB = {"MA", "DZ", "TN", "LY", "MR", "EH"}

# United States and its territories
US = {"AS", "GU", "MH", "MP", "PR", "UM", "US", "VI"}

# Special Autonomous Regions of China
CNSAR = {"HK", "MO"}

LATIN_AMERICA = {
    "419",
    # Central America
    "013",
    "BZ",
    "CR",
    "SV",
    "GT",
    "HN",
    "MX",
    "NI",
    "PA",
    # South America
    "005",
    "AR",
    "BO",
    "BR",
    "CL",
    "CO",
    "EC",
    "FK",
    "GF",
    "GY",
    "PY",
    "PE",
    "SR",
    "UY",
    "VE",
}

# North and South America
AMERICAS = {
    "019",
    # Caribbean
    "029",
    "AI",
    "AG",
    "AW",
    "BS",
    "BB",
    "VG",
    "BQ",
    "KY",
    "CU",
    "CW",
    "DM",
    "DO",
    "GD",
    "GP",
    "HT",
    "JM",
    "MQ",
    "MS",
    "PR",
    "SX",
    "BL",
    "KN",
    "LC",
    "MF",
    "VC",
    "TT",
    "TC",
    "VI",
    # Northern America
    "021",
    "BM",
    "CA",
    "GL",
    "PM",
    "US",
    # North America as a whole
    "003",
} | LATIN_AMERICA


def tuple_distance_cached(desired: TagTriple, supported: TagTriple) -> int:
    """
    Takes in triples of (language, script, territory), which can be derived by
    'maximizing' a language tag. Returns a number from 0 to 135 indicating the
    'distance' between these for the purposes of language matching.
    """
    # First of all, if these are identical, return quickly:
    if supported == desired:
        return 0

    # If we've already figured it out, return the cached distance.
    if (desired, supported) in _DISTANCE_CACHE:
        return _DISTANCE_CACHE[desired, supported]
    else:
        result = _tuple_distance(desired, supported)
        _DISTANCE_CACHE[desired, supported] = result
        return result


def _get2(dictionary: dict, key1: str, key2: str, default):
    return dictionary.get(key1, {}).get(key2, default)


def _tuple_distance(desired: TagTriple, supported: TagTriple) -> int:
    desired_language, desired_script, desired_territory = desired
    supported_language, supported_script, supported_territory = supported
    distance = 0

    if desired_language != supported_language:
        distance += _get2(
            LANGUAGE_DISTANCES,
            desired_language,
            supported_language,
            DEFAULT_LANGUAGE_DISTANCE,
        )

    desired_script_pair = f"{desired_language}_{desired_script}"
    supported_script_pair = f"{supported_language}_{supported_script}"

    if desired_script != supported_script:
        # Scripts can match other scripts, but only when paired with a
        # language. For example, there is no reason to assume someone who can
        # read 'Latn' can read 'Cyrl', but there is plenty of reason to believe
        # someone who can read 'sr-Latn' can read 'sr-Cyrl' because Serbian is
        # a language written in two scripts.
        distance += _get2(
            LANGUAGE_DISTANCES,
            desired_script_pair,
            supported_script_pair,
            DEFAULT_SCRIPT_DISTANCE,
        )

    if desired_territory != supported_territory:
        # The rules for matching territories are too weird to implement the
        # general case efficiently. Instead of implementing all the possible
        # match rules the XML could define, instead we just reimplement the
        # rules of CLDR 36.1 here in code.

        tdist = DEFAULT_TERRITORY_DISTANCE
        if desired_script_pair == supported_script_pair:
            if desired_language == "ar":
                if (desired_territory in MAGHREB) != (supported_territory in MAGHREB):
                    tdist = 5
            elif desired_language == "en":
                if (desired_territory == "GB") and (supported_territory not in US):
                    tdist = 3
                elif (desired_territory not in US) and (supported_territory == "GB"):
                    tdist = 3
                elif (desired_territory in US) != (supported_territory in US):
                    tdist = 5
            # This is not a rule that's spelled out in CLDR, but is implied by things
            # about territory containment mentioned in other standards. Numeric values
            # for territories, like '003', represent broad regions that contain more
            # specific territories.
            #
            # 419 is the numeric value most often seen in language codes, particularly
            # 'es-419' for Latin American Spanish. If you have a language code that
            # differs only in that its territory is more specific, like 'es-PY', it should
            # be closer to a supported 'es-419' than anything with a territory difference.
            #
            # We can implement this for 419 without becoming responsible for keeping up
            # with which countries/territories/regions contain others in the general case.
            elif desired_territory in LATIN_AMERICA and supported_territory == "419":
                tdist = 1
            elif desired_language == "es" or desired_language == "pt":
                if (desired_territory in AMERICAS) != (supported_territory in AMERICAS):
                    tdist = 5
            elif desired_script_pair == "zh_Hant":
                if (desired_territory in CNSAR) != (supported_territory in CNSAR):
                    tdist = 5
        distance += tdist
    return distance
