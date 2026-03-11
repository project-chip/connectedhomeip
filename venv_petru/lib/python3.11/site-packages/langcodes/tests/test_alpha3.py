import string
import langcodes

def test_alpha2_to_alpha3():
    """
    Test that each valid alpha2 code has a corresponding, unique alpha3 code.
    """
    seen = set()
    for letter1 in string.ascii_lowercase:
        for letter2 in string.ascii_lowercase:
            code = letter1 + letter2
            language = langcodes.get(code, normalize=False)
            if language.is_valid():
                alpha3 = language.to_alpha3()

                # These four 2-letter codes exist only as aliases, and don't have
                # their own unique 3-letter codes. All other 2-letter codes should
                # uniquely map to 3-letter codes.
                if code not in {'in', 'iw', 'ji', 'jw'}:
                    assert alpha3 not in seen
                    seen.add(alpha3)
