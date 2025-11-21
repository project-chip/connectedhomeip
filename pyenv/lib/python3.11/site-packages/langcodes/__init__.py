"""
langcodes knows what languages are. It knows the standardized codes that
refer to them, such as `en` for English, `es` for Spanish and `hi` for Hindi.
Often, it knows what these languages are called *in* a language, and that
language doesn't have to be English.

See README.md for the main documentation, or read it on GitHub at
https://github.com/LuminosoInsight/langcodes/ . For more specific documentation
on the functions in langcodes, scroll down and read the docstrings.

Some of these functions, particularly those that work with the names of
languages, require the `language_data` module to be installed.
"""
from operator import itemgetter
from typing import Any, List, Tuple, Dict, Sequence, Iterable, Optional, Mapping, Union
import warnings
import sys

from langcodes.tag_parser import LanguageTagError, parse_tag, normalize_characters
from langcodes.language_distance import tuple_distance_cached
from langcodes.data_dicts import (
    ALL_SCRIPTS,
    DEFAULT_SCRIPTS,
    LANGUAGE_REPLACEMENTS,
    LANGUAGE_ALPHA3,
    LANGUAGE_ALPHA3_BIBLIOGRAPHIC,
    TERRITORY_REPLACEMENTS,
    NORMALIZED_MACROLANGUAGES,
    LIKELY_SUBTAGS,
    VALIDITY,
)

# When we're getting natural language information *about* languages, it's in
# English if you don't specify the language.
DEFAULT_LANGUAGE = 'en'


LANGUAGE_NAME_IMPORT_MESSAGE = """
Looking up language names now requires the `language_data` package.

Install it with:
    pip install language_data
Or as an optional feature of langcodes:
    pip install langcodes[data]
"""


class Language:
    """
    The Language class defines the results of parsing a language tag.
    Language objects have the following attributes, any of which may be
    unspecified (in which case their value is None):

    - *language*: the code for the language itself.
    - *script*: the 4-letter code for the writing system being used.
    - *territory*: the 2-letter or 3-digit code for the country or similar territory
      of the world whose usage of the language appears in this text.
    - *extlangs*: a list of more specific language codes that follow the language
      code. (This is allowed by the language code syntax, but deprecated.)
    - *variants*: codes for specific variations of language usage that aren't
      covered by the *script* or *territory* codes.
    - *extensions*: information that's attached to the language code for use in
      some specific system, such as Unicode collation orders.
    - *private*: a code starting with `x-` that has no defined meaning.

    The `Language.get` method converts a string to a Language instance.
    It's also available at the top level of this module as the `get` function.
    """

    ATTRIBUTES = [
        'language',
        'extlangs',
        'script',
        'territory',
        'variants',
        'extensions',
        'private',
    ]

    # When looking up "likely subtags" data, we try looking up the data for
    # increasingly less specific versions of the language code.
    BROADER_KEYSETS = [
        {'language', 'script', 'territory'},
        {'language', 'territory'},
        {'language', 'script'},
        {'language'},
        {'script'},
        {},
    ]

    MATCHABLE_KEYSETS = [
        {'language', 'script', 'territory'},
        {'language', 'script'},
        {'language'},
    ]

    # Values cached at the class level
    _INSTANCES: Dict[tuple, 'Language'] = {}
    _PARSE_CACHE: Dict[Tuple[str, bool], 'Language'] = {}

    def __init__(
        self,
        language: Optional[str] = None,
        extlangs: Optional[Sequence[str]] = None,
        script: Optional[str] = None,
        territory: Optional[str] = None,
        variants: Optional[Sequence[str]] = None,
        extensions: Optional[Sequence[str]] = None,
        private: Optional[str] = None,
    ):
        """
        The constructor for Language objects.

        It's inefficient to call this directly, because it can't return
        an existing instance. Instead, call Language.make(), which
        has the same signature.
        """
        self.language = language
        self.extlangs = extlangs
        self.script = script
        self.territory = territory
        self.variants = variants
        self.extensions = extensions
        self.private = private

        # Cached values
        self._simplified: 'Language' = None
        self._searchable: 'Language' = None
        self._broader: List[str] = None
        self._assumed: 'Language' = None
        self._filled: 'Language' = None
        self._macrolanguage: Optional['Language'] = None
        self._str_tag: str = None
        self._dict: dict = None
        self._disp_separator: str = None
        self._disp_pattern: str = None

        # Make sure the str_tag value is cached
        self.to_tag()

    @classmethod
    def make(
        cls,
        language: Optional[str] = None,
        extlangs: Optional[Sequence[str]] = None,
        script: Optional[str] = None,
        territory: Optional[str] = None,
        variants: Optional[Sequence[str]] = None,
        extensions: Optional[Sequence[str]] = None,
        private: Optional[str] = None,
    ) -> 'Language':
        """
        Create a Language object by giving any subset of its attributes.

        If this value has been created before, return the existing value.
        """
        values = (
            language,
            tuple(extlangs or ()),
            script,
            territory,
            tuple(variants or ()),
            tuple(extensions or ()),
            private,
        )
        if values in cls._INSTANCES:
            return cls._INSTANCES[values]

        instance = cls(
            language=language,
            extlangs=extlangs,
            script=script,
            territory=territory,
            variants=variants,
            extensions=extensions,
            private=private,
        )
        cls._INSTANCES[values] = instance
        return instance

    @staticmethod
    def get(tag: Union[str, 'Language'], normalize=True) -> 'Language':
        """
        Create a Language object from a language tag string.

        If normalize=True, non-standard or overlong tags will be replaced as
        they're interpreted. This is recommended.

        Here are several examples of language codes, which are also test cases.
        Most language codes are straightforward, but these examples will get
        pretty obscure toward the end.

        >>> Language.get('en-US')
        Language.make(language='en', territory='US')

        >>> Language.get('zh-Hant')
        Language.make(language='zh', script='Hant')

        >>> Language.get('und')
        Language.make()

        This function is idempotent, in case you already have a Language object:

        >>> Language.get(Language.get('en-us'))
        Language.make(language='en', territory='US')

        The non-code 'root' is sometimes used to represent the lack of any
        language information, similar to 'und'.

        >>> Language.get('root')
        Language.make()

        By default, getting a Language object will automatically convert
        deprecated tags:

        >>> Language.get('iw')
        Language.make(language='he')

        >>> Language.get('in')
        Language.make(language='id')

        One type of deprecated tag that should be replaced is for sign
        languages, which used to all be coded as regional variants of a
        fictitious global sign language called 'sgn'. Of course, there is no
        global sign language, so sign languages now have their own language
        codes.

        >>> Language.get('sgn-US')
        Language.make(language='ase')

        >>> Language.get('sgn-US', normalize=False)
        Language.make(language='sgn', territory='US')

        'en-gb-oed' is a tag that's grandfathered into the standard because it
        has been used to mean "spell-check this with Oxford English Dictionary
        spelling", but that tag has the wrong shape. We interpret this as the
        new standardized tag 'en-gb-oxendict', unless asked not to normalize.

        >>> Language.get('en-gb-oed')
        Language.make(language='en', territory='GB', variants=['oxendict'])

        >>> Language.get('en-gb-oed', normalize=False)
        Language.make(language='en-gb-oed')

        'zh-min-nan' is another oddly-formed tag, used to represent the
        Southern Min language, which includes Taiwanese as a regional form. It
        now has its own language code.

        >>> Language.get('zh-min-nan')
        Language.make(language='nan')

        The vague tag 'zh-min' is now also interpreted as 'nan', with a private
        extension indicating that it had a different form:

        >>> Language.get('zh-min')
        Language.make(language='nan', private='x-zh-min')

        Occasionally Wiktionary will use 'extlang' tags in strange ways, such
        as using the tag 'und-ibe' for some unspecified Iberian language.

        >>> Language.get('und-ibe')
        Language.make(extlangs=['ibe'])

        Here's an example of replacing multiple deprecated tags.

        The language tag 'sh' (Serbo-Croatian) ended up being politically
        problematic, and different standards took different steps to address
        this. The IANA made it into a macrolanguage that contains 'sr', 'hr',
        and 'bs'. Unicode further decided that it's a legacy tag that should
        be interpreted as 'sr-Latn', which the language matching rules say
        is mutually intelligible with all those languages.

        We complicate the example by adding on the territory tag 'QU', an old
        provisional tag for the European Union, which is now standardized as
        'EU'.

        >>> Language.get('sh-QU')
        Language.make(language='sr', script='Latn', territory='EU')
        """
        if isinstance(tag, Language):
            if not normalize:
                # shortcut: we have the tag already
                return tag

            # We might need to normalize this tag. Convert it back into a
            # string tag, to cover all the edge cases of normalization in a
            # way that we've already solved.
            tag = tag.to_tag()

        if (tag, normalize) in Language._PARSE_CACHE:
            return Language._PARSE_CACHE[tag, normalize]

        data: Dict[str, Any] = {}

        # If the complete tag appears as something to normalize, do the
        # normalization right away. Smash case and convert underscores to
        # hyphens when checking, because the case normalization that comes from
        # parse_tag() hasn't been applied yet.

        tag_lower = normalize_characters(tag)
        if normalize and tag_lower in LANGUAGE_REPLACEMENTS:
            tag = LANGUAGE_REPLACEMENTS[tag_lower]

        components = parse_tag(tag)

        for typ, value in components:
            if typ == 'extlang' and normalize and 'language' in data:
                # smash extlangs when possible
                minitag = f"{data['language']}-{value}"
                norm = LANGUAGE_REPLACEMENTS.get(normalize_characters(minitag))
                if norm is not None:
                    data.update(Language.get(norm, normalize).to_dict())
                else:
                    data.setdefault('extlangs', []).append(value)
            elif typ in {'extlang', 'variant', 'extension'}:
                data.setdefault(typ + 's', []).append(value)
            elif typ == 'language':
                if value == 'und':
                    pass
                elif normalize:
                    replacement = LANGUAGE_REPLACEMENTS.get(value.lower())
                    if replacement is not None:
                        # parse the replacement if necessary -- this helps with
                        # Serbian and Moldovan
                        data.update(Language.get(replacement, normalize).to_dict())
                    else:
                        data['language'] = value
                else:
                    data['language'] = value
            elif typ == 'territory':
                if normalize:
                    data['territory'] = TERRITORY_REPLACEMENTS.get(value.lower(), value)
                else:
                    data['territory'] = value
            elif typ == 'grandfathered':
                # If we got here, we got a grandfathered tag but we were asked
                # not to normalize it, or the CLDR data doesn't know how to
                # normalize it. The best we can do is set the entire tag as the
                # language.
                data['language'] = value
            else:
                data[typ] = value

        result = Language.make(**data)
        Language._PARSE_CACHE[tag, normalize] = result
        return result

    def to_tag(self) -> str:
        """
        Convert a Language back to a standard language tag, as a string.
        This is also the str() representation of a Language object.

        >>> Language.make(language='en', territory='GB').to_tag()
        'en-GB'

        >>> Language.make(language='yue', script='Hant', territory='HK').to_tag()
        'yue-Hant-HK'

        >>> Language.make(script='Arab').to_tag()
        'und-Arab'

        >>> str(Language.make(territory='IN'))
        'und-IN'
        """
        if self._str_tag is not None:
            return self._str_tag
        subtags = ['und']
        if self.language:
            subtags[0] = self.language
        if self.extlangs:
            for extlang in sorted(self.extlangs):
                subtags.append(extlang)
        if self.script:
            subtags.append(self.script)
        if self.territory:
            subtags.append(self.territory)
        if self.variants:
            for variant in sorted(self.variants):
                subtags.append(variant)
        if self.extensions:
            for ext in self.extensions:
                subtags.append(ext)
        if self.private:
            subtags.append(self.private)
        self._str_tag = '-'.join(subtags)
        return self._str_tag

    def simplify_script(self) -> 'Language':
        """
        Remove the script from some parsed language data, if the script is
        redundant with the language.

        >>> Language.make(language='en', script='Latn').simplify_script()
        Language.make(language='en')

        >>> Language.make(language='yi', script='Latn').simplify_script()
        Language.make(language='yi', script='Latn')

        >>> Language.make(language='yi', script='Hebr').simplify_script()
        Language.make(language='yi')
        """
        if self._simplified is not None:
            return self._simplified

        if self.language and self.script:
            if DEFAULT_SCRIPTS.get(self.language) == self.script:
                result = self.update_dict({'script': None})
                self._simplified = result
                return self._simplified

        self._simplified = self
        return self._simplified

    def assume_script(self) -> 'Language':
        """
        Fill in the script if it's missing, and if it can be assumed from the
        language subtag. This is the opposite of `simplify_script`.

        >>> Language.make(language='en').assume_script()
        Language.make(language='en', script='Latn')

        >>> Language.make(language='yi').assume_script()
        Language.make(language='yi', script='Hebr')

        >>> Language.make(language='yi', script='Latn').assume_script()
        Language.make(language='yi', script='Latn')

        This fills in nothing when the script cannot be assumed -- such as when
        the language has multiple scripts, or it has no standard orthography:

        >>> Language.make(language='sr').assume_script()
        Language.make(language='sr')

        >>> Language.make(language='eee').assume_script()
        Language.make(language='eee')

        It also dosn't fill anything in when the language is unspecified.

        >>> Language.make(territory='US').assume_script()
        Language.make(territory='US')
        """
        if self._assumed is not None:
            return self._assumed
        if self.language and not self.script:
            try:
                self._assumed = self.update_dict(
                    {'script': DEFAULT_SCRIPTS[self.language]}
                )
            except KeyError:
                self._assumed = self
        else:
            self._assumed = self
        return self._assumed

    def prefer_macrolanguage(self) -> 'Language':
        """
        BCP 47 doesn't specify what to do with macrolanguages and the languages
        they contain. The Unicode CLDR, on the other hand, says that when a
        macrolanguage has a dominant standardized language, the macrolanguage
        code should be used for that language. For example, Mandarin Chinese
        is 'zh', not 'cmn', according to Unicode, and Malay is 'ms', not 'zsm'.

        This isn't a rule you'd want to follow in all cases -- for example, you may
        want to be able to specifically say that 'ms' (the Malay macrolanguage)
        contains both 'zsm' (Standard Malay) and 'id' (Indonesian). But applying
        this rule helps when interoperating with the Unicode CLDR.

        So, applying `prefer_macrolanguage` to a Language object will
        return a new object, replacing the language with the macrolanguage if
        it is the dominant language within that macrolanguage. It will leave
        non-dominant languages that have macrolanguages alone.

        >>> Language.get('arb').prefer_macrolanguage()
        Language.make(language='ar')

        >>> Language.get('cmn-Hant').prefer_macrolanguage()
        Language.make(language='zh', script='Hant')

        >>> Language.get('yue-Hant').prefer_macrolanguage()
        Language.make(language='yue', script='Hant')
        """
        if self._macrolanguage is not None:
            return self._macrolanguage
        language = self.language or 'und'
        if language in NORMALIZED_MACROLANGUAGES:
            self._macrolanguage = self.update_dict(
                {'language': NORMALIZED_MACROLANGUAGES[language]}
            )
        else:
            self._macrolanguage = self
        return self._macrolanguage

    def to_alpha3(self, variant: str = 'T') -> str:
        """
        Get the three-letter language code for this language, even if it's
        canonically written with a two-letter code.

        These codes are the 'alpha3' codes defined by ISO 639-2.

        When this function returns, it always returns a 3-letter string. If
        there is no known alpha3 code for the language, it raises a LookupError.

        In cases where the distinction matters, we default to the 'terminology'
        code. You can pass `variant='B'` to get the 'bibliographic' code instead.
        For example, the terminology code for German is 'deu', while the
        bibliographic code is 'ger'.

        (The confusion between these two sets of codes is a good reason to avoid
        using alpha3 codes. Every language that has two different alpha3 codes
        also has an alpha2 code that's preferred, such as 'de' for German.)

        >>> Language.get('fr').to_alpha3()
        'fra'
        >>> Language.get('fr-CA').to_alpha3()
        'fra'
        >>> Language.get('fr').to_alpha3(variant='B')
        'fre'
        >>> Language.get('de').to_alpha3(variant='T')
        'deu'
        >>> Language.get('ja').to_alpha3()
        'jpn'
        >>> Language.get('un').to_alpha3()
        Traceback (most recent call last):
            ...
        LookupError: 'un' is not a known language code, and has no alpha3 code.


        All valid two-letter language codes have corresponding alpha3 codes,
        even the un-normalized ones. If they were assigned an alpha3 code by ISO
        before they were assigned a normalized code by CLDR, these codes may be
        different:

        >>> Language.get('tl', normalize=False).to_alpha3()
        'tgl'
        >>> Language.get('tl').to_alpha3()
        'fil'
        >>> Language.get('sh', normalize=False).to_alpha3()
        'hbs'


        Three-letter codes are preserved, even if they're unknown:

        >>> Language.get('qqq').to_alpha3()
        'qqq'
        >>> Language.get('und').to_alpha3()
        'und'
        """
        variant = variant.upper()
        if variant not in 'BT':
            raise ValueError("Variant must be 'B' or 'T'")

        language = self.language
        if language is None:
            return 'und'
        elif len(language) == 3:
            return language
        else:
            if variant == 'B' and language in LANGUAGE_ALPHA3_BIBLIOGRAPHIC:
                return LANGUAGE_ALPHA3_BIBLIOGRAPHIC[language]
            elif language in LANGUAGE_ALPHA3:
                return LANGUAGE_ALPHA3[language]
            else:
                raise LookupError(
                    f"{language!r} is not a known language code, "
                    "and has no alpha3 code."
                )

    def broader_tags(self) -> List[str]:
        """
        Iterate through increasingly general tags for this language.

        This isn't actually that useful for matching two arbitrary language tags
        against each other, but it is useful for matching them against a known
        standardized form, such as in the CLDR data.

        The list of broader versions to try appears in UTR 35, section 4.3,
        "Likely Subtags".

        >>> Language.get('nn-Latn-NO-x-thingy').broader_tags()
        ['nn-Latn-NO-x-thingy', 'nn-Latn-NO', 'nn-NO', 'nn-Latn', 'nn', 'und-Latn', 'und']

        >>> Language.get('arb-Arab').broader_tags()
        ['arb-Arab', 'ar-Arab', 'arb', 'ar', 'und-Arab', 'und']
        """
        if self._broader is not None:
            return self._broader
        self._broader = [self.to_tag()]
        seen = set([self.to_tag()])
        for keyset in self.BROADER_KEYSETS:
            for start_language in (self, self.prefer_macrolanguage()):
                filtered = start_language._filter_attributes(keyset)
                tag = filtered.to_tag()
                if tag not in seen:
                    self._broader.append(tag)
                    seen.add(tag)
        return self._broader

    def broaden(self) -> 'List[Language]':
        """
        Like `broader_tags`, but returrns Language objects instead of strings.
        """
        return [Language.get(tag) for tag in self.broader_tags()]

    def maximize(self) -> 'Language':
        """
        The Unicode CLDR contains a "likelySubtags" data file, which can guess
        reasonable values for fields that are missing from a language tag.

        This is particularly useful for comparing, for example, "zh-Hant" and
        "zh-TW", two common language tags that say approximately the same thing
        via rather different information. (Using traditional Han characters is
        not the same as being in Taiwan, but each implies that the other is
        likely.)

        These implications are provided in the CLDR supplemental data, and are
        based on the likelihood of people using the language to transmit text
        on the Internet. (This is why the overall default is English, not
        Chinese.)

        It's important to recognize that these tags amplify majorities, and
        that not all language support fits into a "likely" language tag.

        >>> str(Language.get('zh-Hant').maximize())
        'zh-Hant-TW'
        >>> str(Language.get('zh-TW').maximize())
        'zh-Hant-TW'
        >>> str(Language.get('ja').maximize())
        'ja-Jpan-JP'
        >>> str(Language.get('pt').maximize())
        'pt-Latn-BR'
        >>> str(Language.get('und-Arab').maximize())
        'ar-Arab-EG'
        >>> str(Language.get('und-CH').maximize())
        'de-Latn-CH'

        As many standards are, this is US-centric:

        >>> str(Language.make().maximize())
        'en-Latn-US'

        "Extlangs" have no likely-subtags information, so they will give
        maximized results that make no sense:

        >>> str(Language.get('und-ibe').maximize())
        'en-ibe-Latn-US'
        """
        if self._filled is not None:
            return self._filled

        for tag in self.broader_tags():
            if tag in LIKELY_SUBTAGS:
                result = Language.get(LIKELY_SUBTAGS[tag], normalize=False)
                result = result.update(self)
                self._filled = result
                return result

        raise RuntimeError(
            "Couldn't fill in likely values. This represents a problem with "
            "the LIKELY_SUBTAGS data."
        )

    # Support an old, wordier name for the method
    fill_likely_values = maximize

    def match_score(self, supported: 'Language') -> int:
        """
        DEPRECATED: use .distance() instead, which uses newer data and is _lower_
        for better matching languages.
        """
        warnings.warn(
            "`match_score` is deprecated because it's based on deprecated CLDR info. "
            "Use `distance` instead, which is _lower_ for better matching languages. ",
            DeprecationWarning,
        )
        return 100 - min(self.distance(supported), 100)

    def distance(self, supported: 'Language', ignore_script: bool = False) -> int:
        """
        Suppose that `self` is the language that the user desires, and
        `supported` is a language that is actually supported.

        This method returns a number from 0 to 134 measuring the 'distance'
        between the languages (lower numbers are better). This is not a
        symmetric relation. If `ignore_script` is `True`, the script will
        not be used in the comparison, possibly resulting in a smaller
        'distance'.

        The language distance is not really about the linguistic similarity or
        history of the languages; instead, it's based largely on sociopolitical
        factors, indicating which language speakers are likely to know which
        other languages in the present world. Much of the heuristic is about
        finding a widespread 'world language' like English, Chinese, French, or
        Russian that speakers of a more localized language will accept.

        A version that works on language tags, as strings, is in the function
        `tag_distance`. See that function for copious examples.
        """
        if supported == self:
            return 0

        # CLDR has realized that these matching rules are undermined when the
        # unspecified language 'und' gets maximized to 'en-Latn-US', so this case
        # is specifically not maximized:
        if self.language is None and self.script is None and self.territory is None:
            desired_triple = ('und', 'Zzzz', 'ZZ')
        else:
            desired_complete = self.prefer_macrolanguage().maximize()

            desired_triple = (
                desired_complete.language,
                None if ignore_script else desired_complete.script,
                desired_complete.territory,
            )

        if (
            supported.language is None
            and supported.script is None
            and supported.territory is None
        ):
            supported_triple = ('und', 'Zzzz', 'ZZ')
        else:
            supported_complete = supported.prefer_macrolanguage().maximize()

            supported_triple = (
                supported_complete.language,
                None if ignore_script else supported_complete.script,
                supported_complete.territory,
            )

        return tuple_distance_cached(desired_triple, supported_triple)

    def is_valid(self) -> bool:
        """
        Checks whether the language, script, territory, and variants
        (if present) are all tags that have meanings assigned by IANA.
        For example, 'ja' (Japanese) is a valid tag, and 'jp' is not.

        The data is current as of CLDR 40.

        >>> Language.get('ja').is_valid()
        True
        >>> Language.get('jp').is_valid()
        False
        >>> Language.get('en-001').is_valid()
        True
        >>> Language.get('en-000').is_valid()
        False
        >>> Language.get('en-Latn').is_valid()
        True
        >>> Language.get('en-Latnx').is_valid()
        False
        >>> Language.get('und').is_valid()
        True
        >>> Language.get('en-GB-oxendict').is_valid()
        True
        >>> Language.get('en-GB-oxenfree').is_valid()
        False
        >>> Language.get('x-heptapod').is_valid()
        True

        Some scripts are, confusingly, not included in CLDR's 'validity' pattern.
        If a script appears in the IANA registry, we consider it valid.

        >>> Language.get('ur-Aran').is_valid()
        True
        >>> Language.get('cu-Cyrs').is_valid()
        True

        A language tag with multiple extlangs will parse, but is not valid.
        The only allowed example is 'zh-min-nan', which normalizes to the
        language 'nan'.

        >>> Language.get('zh-min-nan').is_valid()
        True
        >>> Language.get('sgn-ase-bfi').is_valid()
        False

        These examples check that duplicate tags are not valid:

        >>> Language.get('de-1901').is_valid()
        True
        >>> Language.get('de-1901-1901').is_valid()
        False
        >>> Language.get('en-a-bbb-c-ddd').is_valid()
        True
        >>> Language.get('en-a-bbb-a-ddd').is_valid()
        False

        Of course, you should be prepared to catch a failure to parse the
        language code at all:

        >>> Language.get('C').is_valid()
        Traceback (most recent call last):
        ...
        langcodes.tag_parser.LanguageTagError: Expected a language code, got 'c'
        """
        if self.extlangs is not None:
            # An erratum to BCP 47 says that tags with more than one extlang are
            # invalid.
            if len(self.extlangs) > 1:
                return False

        subtags = [self.language, self.script, self.territory]
        checked_subtags = []
        if self.variants is not None:
            subtags.extend(self.variants)
        for subtag in subtags:
            if subtag is not None:
                checked_subtags.append(subtag)
                if not subtag.startswith('x-') and not VALIDITY.match(subtag):
                    if subtag not in ALL_SCRIPTS:
                        return False

        # We check extensions for validity by ensuring that there aren't
        # two extensions introduced by the same letter. For example, you can't
        # have two 'u-' extensions.
        if self.extensions:
            checked_subtags.extend([extension[:2] for extension in self.extensions])
        if len(set(checked_subtags)) != len(checked_subtags):
            return False
        return True

    def has_name_data(self) -> bool:
        """
        Return True when we can name languages in this language. Requires
        `language_data` to be installed.

        This is true when the language, or one of its 'broader' versions, is in
        the list of CLDR target languages.

        >>> Language.get('fr').has_name_data()
        True
        >>> Language.get('so').has_name_data()
        True
        >>> Language.get('enc').has_name_data()
        False
        >>> Language.get('und').has_name_data()
        False
        """
        try:
            from language_data.name_data import LANGUAGES_WITH_NAME_DATA
        except ImportError:
            print(LANGUAGE_NAME_IMPORT_MESSAGE, file=sys.stdout)
            raise

        matches = set(self.broader_tags()) & LANGUAGES_WITH_NAME_DATA
        return bool(matches)

    # These methods help to show what the language tag means in natural
    # language. They actually apply the language-matching algorithm to find
    # the right language to name things in.

    def _get_name(
        self, attribute: str, language: Union[str, 'Language'], max_distance: int
    ) -> str:
        try:
            from language_data.names import code_to_names
        except ImportError:
            print(LANGUAGE_NAME_IMPORT_MESSAGE, file=sys.stdout)
            raise

        assert attribute in self.ATTRIBUTES
        if isinstance(language, str):
            language = Language.get(language)

        attr_value = getattr(self, attribute)
        if attr_value is None:
            if attribute == 'language':
                attr_value = 'und'
            else:
                return None
        names = code_to_names(attr_value)

        result = self._best_name(names, language, max_distance)
        if result is not None:
            return result
        else:
            # Construct a string like "Unknown language [zzz]"
            placeholder = None
            if attribute == 'language':
                placeholder = 'und'
            elif attribute == 'script':
                placeholder = 'Zzzz'
            elif attribute == 'territory':
                placeholder = 'ZZ'

            unknown_name = None
            if placeholder is not None:
                names = code_to_names(placeholder)
                unknown_name = self._best_name(names, language, max_distance)
            if unknown_name is None:
                unknown_name = 'Unknown language subtag'
            return f'{unknown_name} [{attr_value}]'

    def _best_name(
        self, names: Mapping[str, str], language: 'Language', max_distance: int
    ):
        matchable_languages = set(language.broader_tags())
        possible_languages = [
            key for key in sorted(names.keys()) if key in matchable_languages
        ]

        target_language, score = closest_match(
            language, possible_languages, max_distance
        )
        if target_language in names:
            return names[target_language]
        else:
            return names.get(DEFAULT_LANGUAGE)

    def language_name(
        self,
        language: Union[str, 'Language'] = DEFAULT_LANGUAGE,
        max_distance: int = 25,
    ) -> str:
        """
        Give the name of the language (not the entire tag, just the language part)
        in a natural language. The target language can be given as a string or
        another Language object.

        By default, things are named in English:

        >>> Language.get('fr').language_name()
        'French'
        >>> Language.get('el').language_name()
        'Greek'

        But you can ask for language names in numerous other languages:

        >>> Language.get('fr').language_name('fr')
        'français'
        >>> Language.get('el').language_name('fr')
        'grec'

        Why does everyone get Slovak and Slovenian confused? Let's ask them.

        >>> Language.get('sl').language_name('sl')
        'slovenščina'
        >>> Language.get('sk').language_name('sk')
        'slovenčina'
        >>> Language.get('sl').language_name('sk')
        'slovinčina'
        >>> Language.get('sk').language_name('sl')
        'slovaščina'
        """
        return self._get_name('language', language, max_distance)

    def display_name(
        self,
        language: Union[str, 'Language'] = DEFAULT_LANGUAGE,
        max_distance: int = 25,
    ) -> str:
        """
        It's often helpful to be able to describe a language code in a way that a user
        (or you) can understand, instead of in inscrutable short codes. The
        `display_name` method lets you describe a Language object *in a language*.

        The `.display_name(language, min_score)` method will look up the name of the
        language. The names come from the IANA language tag registry, which is only in
        English, plus CLDR, which names languages in many commonly-used languages.

        The default language for naming things is English:

            >>> Language.make(language='fr').display_name()
            'French'

            >>> Language.make().display_name()
            'Unknown language'

            >>> Language.get('zh-Hans').display_name()
            'Chinese (Simplified)'

            >>> Language.get('en-US').display_name()
            'English (United States)'

        But you can ask for language names in numerous other languages:

            >>> Language.get('fr').display_name('fr')
            'français'

            >>> Language.get('fr').display_name('es')
            'francés'

            >>> Language.make().display_name('es')
            'lengua desconocida'

            >>> Language.get('zh-Hans').display_name('de')
            'Chinesisch (Vereinfacht)'

            >>> Language.get('en-US').display_name('zh-Hans')
            '英语（美国）'
        """
        reduced = self.simplify_script()
        language = Language.get(language)
        language_name = reduced.language_name(language, max_distance)
        extra_parts = []

        if reduced.script is not None:
            extra_parts.append(reduced.script_name(language, max_distance))
        if reduced.territory is not None:
            extra_parts.append(reduced.territory_name(language, max_distance))

        if extra_parts:
            clarification = language._display_separator().join(extra_parts)
            pattern = language._display_pattern()
            return pattern.format(language_name, clarification)
        else:
            return language_name

    def _display_pattern(self) -> str:
        """
        Get the pattern, according to CLDR, that should be used for clarifying
        details of a language code.
        """
        # Technically we are supposed to look up this pattern in each language.
        # Practically, it's the same in every language except Chinese, where the
        # parentheses are full-width.
        if self._disp_pattern is not None:
            return self._disp_pattern
        if self.distance(Language.get('zh')) <= 25 or self.distance(Language.get('zh-Hant')) <= 25:
            self._disp_pattern = "{0}（{1}）"
        else:
            self._disp_pattern = "{0} ({1})"
        return self._disp_pattern

    def _display_separator(self) -> str:
        """
        Get the symbol that should be used to separate multiple clarifying
        details -- such as a comma in English, or an ideographic comma in
        Japanese.

        Requires that `language_data` is installed.
        """
        try:
            from language_data.names import DISPLAY_SEPARATORS
        except ImportError:
            print(LANGUAGE_NAME_IMPORT_MESSAGE, file=sys.stdout)
            raise

        if self._disp_separator is not None:
            return self._disp_separator
        matched, _dist = closest_match(self, DISPLAY_SEPARATORS.keys())
        self._disp_separator = DISPLAY_SEPARATORS[matched]
        return self._disp_separator

    def autonym(self, max_distance: int = 9) -> str:
        """
        Give the display name of this language *in* this language.
        Requires that `language_data` is installed.

        >>> Language.get('fr').autonym()
        'français'
        >>> Language.get('es').autonym()
        'español'
        >>> Language.get('ja').autonym()
        '日本語'

        This uses the `display_name()` method, so it can include the name of a
        script or territory when appropriate.

        >>> Language.get('en-AU').autonym()
        'English (Australia)'
        >>> Language.get('sr-Latn').autonym()
        'srpski (latinica)'
        >>> Language.get('sr-Cyrl').autonym()
        'српски (ћирилица)'
        >>> Language.get('pa').autonym()
        'ਪੰਜਾਬੀ'
        >>> Language.get('pa-Arab').autonym()
        'پنجابی (عربی)'

        This only works for language codes that CLDR has locale data for. You
        can't ask for the autonym of 'ja-Latn' and get 'nihongo (rōmaji)'.
        """
        lang = self.prefer_macrolanguage()
        return lang.display_name(language=lang, max_distance=max_distance)

    def script_name(
        self,
        language: Union[str, 'Language'] = DEFAULT_LANGUAGE,
        max_distance: int = 25,
    ) -> str:
        """
        Describe the script part of the language tag in a natural language.
        Requires that `language_data` is installed.
        """
        return self._get_name('script', language, max_distance)

    def territory_name(
        self,
        language: Union[str, 'Language'] = DEFAULT_LANGUAGE,
        max_distance: int = 25,
    ) -> str:
        """
        Describe the territory part of the language tag in a natural language.
        Requires that `language_data` is installed.
        """
        return self._get_name('territory', language, max_distance)

    def region_name(
        self,
        language: Union[str, 'Language'] = DEFAULT_LANGUAGE,
        max_distance: int = 25,
    ) -> str:
        warnings.warn(
            "`region_name` has been renamed to `territory_name` for consistency",
            DeprecationWarning,
        )
        return self.territory_name(language, max_distance)

    @property
    def region(self):
        warnings.warn(
            "The `region` property has been renamed to `territory` for consistency",
            DeprecationWarning,
        )
        return self.territory

    def variant_names(
        self,
        language: Union[str, 'Language'] = DEFAULT_LANGUAGE,
        max_distance: int = 25,
    ) -> Sequence[str]:
        """
        Deprecated in version 3.0.

        We don't store names for variants anymore, so this just returns the list
        of variant codes, such as ['oxendict'] for en-GB-oxendict.
        """
        warnings.warn(
            "variant_names is deprecated and just returns the variant codes",
            DeprecationWarning,
        )
        return self.variants or []

    def describe(
        self,
        language: Union[str, 'Language'] = DEFAULT_LANGUAGE,
        max_distance: int = 25,
    ) -> dict:
        """
        Return a dictionary that describes a given language tag in a specified
        natural language. Requires that `language_data` is installed.

        See `language_name` and related methods for more specific versions of this.

        The desired `language` will in fact be matched against the available
        options using the matching technique that this module provides. We can
        illustrate many aspects of this by asking for a description of Shavian
        script (a phonetic script for English devised by author George Bernard
        Shaw), and where you might find it, in various languages.

        >>> shaw = Language.make(script='Shaw').maximize()
        >>> shaw.describe('en')
        {'language': 'English', 'script': 'Shavian', 'territory': 'United Kingdom'}

        >>> shaw.describe('fr')
        {'language': 'anglais', 'script': 'shavien', 'territory': 'Royaume-Uni'}

        >>> shaw.describe('es')
        {'language': 'inglés', 'script': 'shaviano', 'territory': 'Reino Unido'}

        >>> shaw.describe('pt')
        {'language': 'inglês', 'script': 'shaviano', 'territory': 'Reino Unido'}

        >>> shaw.describe('uk')
        {'language': 'англійська', 'script': 'шоу', 'territory': 'Велика Британія'}

        >>> shaw.describe('arb')
        {'language': 'الإنجليزية', 'script': 'الشواني', 'territory': 'المملكة المتحدة'}

        >>> shaw.describe('th')
        {'language': 'อังกฤษ', 'script': 'ซอเวียน', 'territory': 'สหราชอาณาจักร'}

        >>> shaw.describe('zh-Hans')
        {'language': '英语', 'script': '萧伯纳式文', 'territory': '英国'}

        >>> shaw.describe('zh-Hant')
        {'language': '英文', 'script': '簫柏納字符', 'territory': '英國'}

        >>> shaw.describe('ja')
        {'language': '英語', 'script': 'ショー文字', 'territory': 'イギリス'}

        When we don't have a localization for the language, we fall back on English,
        because the IANA provides names for all known codes in English.

        >>> shaw.describe('lol')
        {'language': 'English', 'script': 'Shavian', 'territory': 'United Kingdom'}

        When the language tag itself is a valid tag but with no known meaning, we
        say so in the appropriate language.

        >>> Language.get('xyz-ZY').display_name()
        'Unknown language [xyz] (Unknown Region [ZY])'

        >>> Language.get('xyz-ZY').display_name('es')
        'lengua desconocida [xyz] (Región desconocida [ZY])'
        """
        names = {}
        if self.language:
            names['language'] = self.language_name(language, max_distance)
        if self.script:
            names['script'] = self.script_name(language, max_distance)
        if self.territory:
            names['territory'] = self.territory_name(language, max_distance)
        return names

    def speaking_population(self) -> int:
        """
        Get an estimate of how many people in the world speak this language,
        derived from CLDR data. Requires that `language_data` is installed.

        Only the language and territory codes will be considered. If a
        territory code is included, the population will count only the
        speakers of the language in that territory.

        Script subtags are disregarded, because it doesn't make sense to ask
        how many people speak in a particular writing script.

        >>> Language.get('es').speaking_population()
        493528077
        >>> Language.get('pt').speaking_population()
        237496885
        >>> Language.get('es-BR').speaking_population()
        76218
        >>> Language.get('pt-BR').speaking_population()
        192661560
        >>> Language.get('vo').speaking_population()
        0
        """
        try:
            from language_data.population_data import LANGUAGE_SPEAKING_POPULATION
        except ImportError:
            print(LANGUAGE_NAME_IMPORT_MESSAGE, file=sys.stdout)
            raise

        lang = self._filter_attributes(['language', 'territory'])
        return LANGUAGE_SPEAKING_POPULATION.get(str(lang), 0)

    def writing_population(self) -> int:
        """
        Get an estimate of how many people in the world read and write
        this language, derived from CLDR data. Requires that `language_data`
        is installed.

        For many languages that aren't typically written, this is an
        overestimate, according to CLDR -- the data often includes people who
        speak that language but write in a different language.

        Only the language, script, and territory codes will be considered.
        If a territory code is included, the population will count only the
        speakers of the language in that territory.

        >>> all = Language.get('zh').writing_population()
        >>> all
        1240841517

        >>> traditional = Language.get('zh-Hant').writing_population()
        >>> traditional
        36863340

        >>> simplified = Language.get('zh-Hans').writing_population()
        >>> all == traditional + simplified
        True

        >>> Language.get('zh-Hant-HK').writing_population()
        6439733
        >>> Language.get('zh-Hans-HK').writing_population()
        338933

        Note that if you want to get the total Chinese writing population
        of Hong Kong, you need to avoid normalization that would interpret
        'zh-HK' as 'zh-Hant-HK'.

        >>> Language.get('zh-HK', normalize=False).writing_population()
        6778666

        Unknown or unspecified language codes get a population of 0.

        >>> Language.get('xyz').writing_population()
        0

        >>> Language.get('und').writing_population()
        0
        """
        try:
            from language_data.population_data import LANGUAGE_WRITING_POPULATION
        except ImportError:
            print(LANGUAGE_NAME_IMPORT_MESSAGE, file=sys.stdout)
            raise

        lang = self._filter_attributes(['language', 'script', 'territory'])
        if str(lang) in LANGUAGE_WRITING_POPULATION:
            return LANGUAGE_WRITING_POPULATION[str(lang)]
        else:
            lang = lang.simplify_script()
            return LANGUAGE_WRITING_POPULATION.get(str(lang), 0)

    @staticmethod
    def find_name(
        tagtype: str, name: str, language: Optional[Union[str, 'Language']] = None
    ) -> 'Language':
        """
        Find the subtag of a particular `tagtype` that has the given `name`.
        Requires that `language_data` is installed.

        The default language, "und", will allow matching names in any language,
        so you can get the code 'fr' by looking up "French", "Français", or
        "francés".

        Occasionally, names are ambiguous in a way that can be resolved by
        specifying what name the language is supposed to be in. For example,
        there is a language named 'Malayo' in English, but it's different from
        the language named 'Malayo' in Spanish (which is Malay). Specifying the
        language will look up the name in a trie that is only in that language.

        In a previous version, we thought we were going to deprecate the
        `language` parameter, as there weren't significant cases of conflicts
        in names of things between languages. Well, we got more data, and
        conflicts in names are everywhere.

        Specifying the language that the name should be in is still not
        required, but it will help to make sure that names can be
        round-tripped.

        >>> Language.find_name('language', 'francés')
        Language.make(language='fr')

        >>> Language.find_name('territory', 'United Kingdom')
        Language.make(territory='GB')

        >>> Language.find_name('script', 'Arabic')
        Language.make(script='Arab')

        >>> Language.find_name('language', 'norsk bokmål')
        Language.make(language='nb')

        >>> Language.find_name('language', 'norsk')
        Language.make(language='no')

        >>> Language.find_name('language', 'norsk', 'en')
        Traceback (most recent call last):
            ...
        LookupError: Can't find any language named 'norsk'

        >>> Language.find_name('language', 'norsk', 'no')
        Language.make(language='no')

        >>> Language.find_name('language', 'malayo', 'en')
        Language.make(language='mbp')

        >>> Language.find_name('language', 'malayo', 'es')
        Language.make(language='ms')

        Some langauge names resolve to more than a language. For example,
        the name 'Brazilian Portuguese' resolves to a language and a territory,
        and 'Simplified Chinese' resolves to a language and a script. In these
        cases, a Language object with multiple subtags will be returned.

        >>> Language.find_name('language', 'Brazilian Portuguese', 'en')
        Language.make(language='pt', territory='BR')

        >>> Language.find_name('language', 'Simplified Chinese', 'en')
        Language.make(language='zh', script='Hans')

        A small amount of fuzzy matching is supported: if the name can be
        shortened to match a single language name, you get that language.
        This allows, for example, "Hakka dialect" to match "Hakka".

        >>> Language.find_name('language', 'Hakka dialect')
        Language.make(language='hak')
        """
        try:
            from language_data.names import name_to_code
        except ImportError:
            print(LANGUAGE_NAME_IMPORT_MESSAGE, file=sys.stdout)
            raise

        # No matter what form of language we got, normalize it to a single
        # language subtag
        if isinstance(language, Language):
            language = language.language
        elif isinstance(language, str):
            language = get(language).language
        if language is None:
            language = 'und'

        code = name_to_code(tagtype, name, language)
        if code is None:
            raise LookupError(f"Can't find any {tagtype} named {name!r}")
        if '-' in code:
            return Language.get(code)
        else:
            data = {tagtype: code}
            return Language.make(**data)

    @staticmethod
    def find(
        name: str, language: Optional[Union[str, 'Language']] = None
    ) -> 'Language':
        """
        A concise version of `find_name`, used to get a language tag by its
        name in a natural language. The language can be omitted in the large
        majority of cases, where the language name is not ambiguous.

        >>> Language.find('Türkçe')
        Language.make(language='tr')
        >>> Language.find('brazilian portuguese')
        Language.make(language='pt', territory='BR')
        >>> Language.find('simplified chinese')
        Language.make(language='zh', script='Hans')

        Some language names are ambiguous: for example, there is a language
        named 'Fala' in English (with code 'fax'), but 'Fala' is also the
        Kwasio word for French. In this case, specifying the language that
        the name is in is necessary for disambiguation.

        >>> Language.find('fala')
        Language.make(language='fr')
        >>> Language.find('fala', 'nmg')
        Language.make(language='fr')
        >>> Language.find('fala', 'en')
        Language.make(language='fax')
        """
        return Language.find_name('language', name, language)

    def to_dict(self) -> dict:
        """
        Get a dictionary of the attributes of this Language object, which
        can be useful for constructing a similar object.
        """
        if self._dict is not None:
            return self._dict

        result = {}
        for key in self.ATTRIBUTES:
            value = getattr(self, key)
            if value:
                result[key] = value
        self._dict = result
        return result

    def update(self, other: 'Language') -> 'Language':
        """
        Update this Language with the fields of another Language.
        """
        return Language.make(
            language=other.language or self.language,
            extlangs=other.extlangs or self.extlangs,
            script=other.script or self.script,
            territory=other.territory or self.territory,
            variants=other.variants or self.variants,
            extensions=other.extensions or self.extensions,
            private=other.private or self.private,
        )

    def update_dict(self, newdata: dict) -> 'Language':
        """
        Update the attributes of this Language from a dictionary.
        """
        return Language.make(
            language=newdata.get('language', self.language),
            extlangs=newdata.get('extlangs', self.extlangs),
            script=newdata.get('script', self.script),
            territory=newdata.get('territory', self.territory),
            variants=newdata.get('variants', self.variants),
            extensions=newdata.get('extensions', self.extensions),
            private=newdata.get('private', self.private),
        )

    @staticmethod
    def _filter_keys(d: dict, keys: Iterable[str]) -> dict:
        """
        Select a subset of keys from a dictionary.
        """
        return {key: d[key] for key in keys if key in d}

    def _filter_attributes(self, keyset: Iterable[str]) -> 'Language':
        """
        Return a copy of this object with a subset of its attributes set.
        """
        filtered = self._filter_keys(self.to_dict(), keyset)
        return Language.make(**filtered)

    def _searchable_form(self) -> 'Language':
        """
        Convert a parsed language tag so that the information it contains is in
        the best form for looking up information in the CLDR.
        """
        if self._searchable is not None:
            return self._searchable

        self._searchable = (
            self._filter_attributes({'language', 'script', 'territory'})
            .simplify_script()
            .prefer_macrolanguage()
        )
        return self._searchable

    def __eq__(self, other):
        if self is other:
            return True
        if not isinstance(other, Language):
            return False
        return self._str_tag == other._str_tag

    def __hash__(self) -> int:
        return hash(self._str_tag)

    def __getitem__(self, key: str) -> Optional[Union[str, List[str]]]:
        if key in self.ATTRIBUTES:
            return getattr(self, key)
        else:
            raise KeyError(key)

    def __contains__(self, key: str) -> bool:
        return key in self.ATTRIBUTES and getattr(self, key)

    def __repr__(self) -> str:
        items = []
        for attr in self.ATTRIBUTES:
            if getattr(self, attr):
                value = getattr(self, attr)
                items.append(f'{attr}={value!r}')
        joined = ', '.join(items)
        return f"Language.make({joined})"

    def __str__(self) -> str:
        return self.to_tag()


# Make the get(), find(), and find_name() functions available at the top level
get = Language.get
find = Language.find
find_name = Language.find_name

# Make the Language object available under the old name LanguageData
LanguageData = Language


def standardize_tag(tag: Union[str, Language], macro: bool = False) -> str:
    """
    Standardize a language tag:

    - Replace deprecated values with their updated versions (if those exist)
    - Remove script tags that are redundant with the language
    - If *macro* is True, use a macrolanguage to represent the most common
      standardized language within that macrolanguage. For example, 'cmn'
      (Mandarin) becomes 'zh' (Chinese), and 'arb' (Modern Standard Arabic)
      becomes 'ar' (Arabic).
    - Format the result according to the conventions of BCP 47

    Macrolanguage replacement is not required by BCP 47, but it is required
    by the Unicode CLDR.

    >>> standardize_tag('en_US')
    'en-US'

    >>> standardize_tag('en-Latn')
    'en'

    >>> standardize_tag('en-uk')
    'en-GB'

    >>> standardize_tag('eng')
    'en'

    >>> standardize_tag('arb-Arab', macro=True)
    'ar'

    >>> standardize_tag('sh-QU')
    'sr-Latn-EU'

    >>> standardize_tag('sgn-US')
    'ase'

    >>> standardize_tag('zh-cmn-hans-cn')
    'zh-Hans-CN'

    >>> standardize_tag('zsm', macro=True)
    'ms'

    >>> standardize_tag('ja-latn-hepburn')
    'ja-Latn-hepburn'

    >>> standardize_tag('spa-latn-mx')
    'es-MX'

    If the tag can't be parsed according to BCP 47, this will raise a
    LanguageTagError (a subclass of ValueError):

    >>> standardize_tag('spa-mx-latn')
    Traceback (most recent call last):
        ...
    langcodes.tag_parser.LanguageTagError: This script subtag, 'latn', is out of place. Expected variant, extension, or end of string.
    """
    langdata = Language.get(tag, normalize=True)
    if macro:
        langdata = langdata.prefer_macrolanguage()

    return langdata.simplify_script().to_tag()


def tag_is_valid(tag: Union[str, Language]) -> bool:
    """
    Determines whether a string is a valid language tag. This is similar to
    Language.get(tag).is_valid(), but can return False in the case where
    the tag doesn't parse.

    >>> tag_is_valid('ja')
    True
    >>> tag_is_valid('jp')
    False
    >>> tag_is_valid('spa-Latn-MX')
    True
    >>> tag_is_valid('spa-MX-Latn')
    False
    >>> tag_is_valid('')
    False
    >>> tag_is_valid('C.UTF-8')
    False
    """
    try:
        langdata = Language.get(tag)
        return langdata.is_valid()
    except LanguageTagError:
        return False


def tag_match_score(
    desired: Union[str, Language], supported: Union[str, Language]
) -> int:
    """
    DEPRECATED: use .distance() instead, which uses newer data and is _lower_
    for better matching languages.

    Return a number from 0 to 100 indicating the strength of match between the
    language the user desires, D, and a supported language, S. Higher numbers
    are better. A reasonable cutoff for not messing with your users is to
    only accept scores of 75 or more.

    A score of 100 means the languages are the same, possibly after normalizing
    and filling in likely values.
    """
    warnings.warn(
        "tag_match_score is deprecated because it's based on deprecated CLDR info. "
        "Use tag_distance instead, which is _lower_ for better matching languages. ",
        DeprecationWarning,
    )
    desired_ld = Language.get(desired)
    supported_ld = Language.get(supported)
    return desired_ld.match_score(supported_ld)


def tag_distance(desired: Union[str, Language], supported: Union[str, Language], ignore_script: bool = False) -> int:
    """
    Tags that expand to the same thing when likely values are filled in get a
    distance of 0.

    >>> tag_distance('en', 'en')
    0
    >>> tag_distance('en', 'en-US')
    0
    >>> tag_distance('zh-Hant', 'zh-TW')
    0
    >>> tag_distance('ru-Cyrl', 'ru')
    0

    As a specific example, Serbo-Croatian is a politically contentious idea,
    but in CLDR, it's considered equivalent to Serbian in Latin characters.

    >>> tag_distance('sh', 'sr-Latn')
    0

    ... which is very similar to Croatian but sociopolitically not the same.

    >>> tag_distance('sh', 'hr')
    9

    Unicode reorganized its distinction between 'no' (Norwegian) and 'nb'
    (Norwegian Bokmål) in 2021. 'no' is preferred in most contexts, and the more
    specific 'nb' is a distance of 1 from it:

    >>> tag_distance('nb', 'no')
    1

    These distances can be asymmetrical: this data includes the fact that speakers
    of Swiss German (gsw) know High German (de), but not at all the other way around.

    The difference seems a little bit extreme, but the asymmetry is certainly
    there. And if your text is tagged as 'gsw', it must be that way for a
    reason.

    >>> tag_distance('gsw', 'de')
    8
    >>> tag_distance('de', 'gsw')
    84

    Unconnected languages get a distance of 80 to 134.

    >>> tag_distance('en', 'zh')
    134
    >>> tag_distance('es', 'fr')
    84
    >>> tag_distance('fr-CH', 'de-CH')
    80

    Different local variants of the same language get a distance from 3 to 5.
    >>> tag_distance('zh-HK', 'zh-MO')   # Chinese is similar in Hong Kong and Macao
    4
    >>> tag_distance('en-AU', 'en-GB')   # Australian English is similar to British English
    3
    >>> tag_distance('en-IN', 'en-GB')   # Indian English is also similar to British English
    3
    >>> tag_distance('es-PE', 'es-419')  # Peruvian Spanish is Latin American Spanish
    1
    >>> tag_distance('es-419', 'es-PE')  # but Latin American Spanish is not necessarily Peruvian
    4
    >>> tag_distance('es-ES', 'es-419')  # Spanish in Spain is further from Latin American Spanish
    5
    >>> tag_distance('en-US', 'en-GB')   # American and British English are somewhat different
    5
    >>> tag_distance('es-MX', 'es-ES')   # Mexican Spanish is different from Spanish Spanish
    5
    >>> # European Portuguese is different from the most common form (Brazilian Portuguese)
    >>> tag_distance('pt', 'pt-PT')
    5

    >>> # Serbian has two scripts, and people might prefer one but understand both
    >>> tag_distance('sr-Latn', 'sr-Cyrl')
    5

    A distance of 10 is used for matching a specific language to its
    more-commonly-used macrolanguage tag.

    >>> tag_distance('arz', 'ar')  # Egyptian Arabic to Modern Standard Arabic
    10
    >>> tag_distance('wuu', 'zh')  # Wu Chinese to (Mandarin) Chinese
    10

    Higher distances can arrive due to particularly contentious differences in
    the script for writing the language, where people who understand one script
    can learn the other but may not be happy with it. This specifically applies
    to Chinese.

    >>> tag_distance('zh-TW', 'zh-CN')
    54
    >>> tag_distance('zh-Hans', 'zh-Hant')
    54
    >>> tag_distance('zh-CN', 'zh-HK')
    54
    >>> tag_distance('zh-CN', 'zh-TW')
    54
    >>> tag_distance('zh-Hant', 'zh-Hans')
    54

    This distance range also applies to the differences between Norwegian
    Bokmål, Nynorsk, and Danish.

    >>> tag_distance('no', 'da')
    12
    >>> tag_distance('no', 'nn')
    20

    Differences of 20 to 50 can represent substantially different languages,
    in cases where speakers of the first may understand the second for demographic
    reasons.

    >>> tag_distance('eu', 'es')  # Basque to Spanish
    20
    >>> tag_distance('af', 'nl')  # Afrikaans to Dutch
    24
    >>> tag_distance('mr', 'hi')  # Marathi to Hindi
    30
    >>> tag_distance('ms', 'id')  # Malay to Indonesian
    34
    >>> tag_distance('mg', 'fr')  # Malagasy to French
    34
    >>> tag_distance('ta', 'en')  # Tamil to English
    44

    A complex example is the tag 'yue' for Cantonese. Written Chinese is usually
    presumed to be Mandarin Chinese, but colloquial Cantonese can be written as
    well. (Some things could not be written any other way, such as Cantonese
    song lyrics.)

    The difference between Cantonese and Mandarin also implies script and
    territory differences by default, adding to the distance.

    >>> tag_distance('yue', 'zh')
    64

    When the supported script is a different one than desired, this is usually
    a major difference with score of 50 or more.

    >>> tag_distance('ja', 'ja-Latn-US-hepburn')
    54

    If `ignore_script` is used, the script difference is ignored and a smaller
    difference with lower score will be found.

    >>> tag_distance('ja', 'ja-Latn-hepburn', ignore_script=True)
    0

    >>> # You can read the Shavian script, right?
    >>> tag_distance('en', 'en-Shaw')
    54
    """
    desired_obj = Language.get(desired)
    supported_obj = Language.get(supported)
    return desired_obj.distance(supported_obj, ignore_script)


def best_match(
    desired_language: Union[str, Language],
    supported_languages: Sequence[str],
    min_score: int = 75,
) -> Tuple[str, int]:
    """
    DEPRECATED: use .closest_match() instead. This function emulates the old
    matching behavior by subtracting the language distance from 100.

    You have software that supports any of the `supported_languages`. You want
    to use `desired_language`. This function lets you choose the right language,
    even if there isn't an exact match.

    Returns:

    - The best-matching language code, which will be one of the
      `supported_languages` or 'und'
    - The score of the match, from 0 to 100; higher is better.

    `min_score` sets the minimum match score. If all languages match with a lower
    score than that, the result will be 'und' with a score of 0.
    """
    max_distance = 100 - min_score
    supported, distance = closest_match(
        desired_language, supported_languages, max_distance
    )
    score = max(0, 100 - distance)
    return supported, score


def closest_match(
    desired_language: Union[str, Language],
    supported_languages: Sequence[str],
    max_distance: int = 25,
    ignore_script: bool = False,
) -> Tuple[str, int]:
    """
    You have software that supports any of the `supported_languages`. You want
    to use `desired_language`. This function lets you choose the right language,
    even if there isn't an exact match.

    Returns:

    - The best-matching language code, which will be one of the
      `supported_languages` or 'und' for no match
    - The distance of the match, which is 0 for a perfect match and increases
      from there (see `tag_distance`)

    `max_distance` sets the maximum match distance. If all matches are farther
    than that, the result will be 'und' with a distance of 1000. The default
    value is 25, and raising it can cause data to be processed in significantly
    the wrong language. The documentation for `tag_distance` describes the
    distance values in more detail.

    `ignore_script` makes the matching ignore scripts, allowing matches to be
    found when they wouldn't otherwise be due to different scripts.

    When there is a tie for the best matching language, the first one in the
    tie will be used.

    >>> closest_match('fr', ['de', 'en', 'fr'])
    ('fr', 0)

    >>> closest_match('pt', ['pt-BR', 'pt-PT'])
    ('pt-BR', 0)

    >>> closest_match('en-AU', ['en-GB', 'en-US'])
    ('en-GB', 3)

    >>> closest_match('af', ['en', 'nl', 'zu'])
    ('nl', 24)

    >>> closest_match('ja', ['ja-Latn-hepburn', 'en'])
    ('und', 1000)

    >>> closest_match('ja', ['ja-Latn-hepburn', 'en'], ignore_script=True)
    ('ja-Latn-hepburn', 0)
    """
    desired_language = str(desired_language)

    # Quickly return if the desired language is directly supported
    if desired_language in supported_languages:
        return desired_language, 0

    # Reduce the desired language to a standard form that could also match
    desired_language = standardize_tag(desired_language)
    if desired_language in supported_languages:
        return desired_language, 0

    match_distances = [
        (supported, tag_distance(desired_language, supported, ignore_script))
        for supported in supported_languages
    ]
    match_distances = [
        (supported, distance)
        for (supported, distance) in match_distances
        if distance <= max_distance
    ] + [('und', 1000)]

    match_distances.sort(key=itemgetter(1))
    return match_distances[0]


def closest_supported_match(
    desired_language: Union[str, Language],
    supported_languages: Sequence[str],
    max_distance: int = 25,
) -> Optional[str]:
    """
    Wraps `closest_match` with a simpler return type. Returns the language
    tag of the closest match if there is one, or None if there is not.

    >>> closest_supported_match('fr', ['de', 'en', 'fr'])
    'fr'

    >>> closest_supported_match('pt', ['pt-BR', 'pt-PT'])
    'pt-BR'

    >>> closest_supported_match('en-AU', ['en-GB', 'en-US'])
    'en-GB'

    >>> closest_supported_match('und', ['en', 'und'])
    'und'

    >>> closest_supported_match('af', ['en', 'nl', 'zu'])
    'nl'

    >>> print(closest_supported_match('af', ['en', 'nl', 'zu'], max_distance=10))
    None
    """
    code, distance = closest_match(desired_language, supported_languages, max_distance)
    if distance == 1000:
        return None
    else:
        return code
