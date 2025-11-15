import langcodes


def test_updated_iana():
    aqk = langcodes.get('aqk')
    assert aqk.language_name('en') == 'Aninka'


def test_cldr_v40():
    en = langcodes.get('en')
    assert en.language_name('dsb') == 'engelšćina'
