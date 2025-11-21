from langcodes import Language


def test__hash__():
    en1 = Language.get("en")
    # Disable caching
    Language._INSTANCES = {}
    Language._PARSE_CACHE = {}
    en2 = Language.get("en")
    assert hash(en1) == hash(en2)

    # Again, disable caching
    Language._INSTANCES = {}
    Language._PARSE_CACHE = {}
    en_us = Language.get("en-US")
    assert hash(en1) != hash(en_us)
