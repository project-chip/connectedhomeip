import marisa_trie
import json
import xml.etree.ElementTree as ET
import os
from pathlib import Path
from collections import defaultdict, Counter

from language_data.names import normalize_name
from language_data.util import data_filename
from language_data.registry_parser import parse_registry

# Naming things is hard, especially languages
# ===========================================
#
# CLDR is supposed to avoid ambiguous language names, particularly among its
# core languages. But it seems that languages are incompletely disambiguated.
#
# It's convenient to be able to get a language by its name, without having to
# also refer to the language that the name is in. In most cases, we can do this
# unambiguously. With the disambiguations and overrides here, this will work
# in a lot of cases. However, some names such as 'Dongo', 'Fala', 'Malayo', and
# 'Tonga' are ambiguous in ways that can only be disambiguated by specifying
# the language the name is in.
#
# Ambiguous names can arise from:
#
# - Ambiguities in the scope of a name. These tend to span languages, and the
#   data files mask the fact that these names are generally ambiguous *within*
#   a language. This is why we have codes.
#
# - Names that just happen to be ambiguous between different things with
#   different etymologies.
#
# Most doubly-claimed language names have standard ways to disambiguate
# them in CLDR, but names such as 'Tonga' and 'Fala' have complex
# inter-language ambiguities.
#
# Our approach is:
#
# - Fix conflicts that seem to arise simply from errors in the data, by
#   overriding the data.
#
# - Fix ambiguities in scope by preferring one scope over another. For example,
#   "North America" could refer to a territory that includes Central America or
#   a territory that doesn't. In any such conflict, we choose to include Central
#   America.
#
# - Avoid ambiguities between different sources of data, by using an order
#   of precedence. CLDR data takes priority over IANA data, which takes priority
#   over Wiktionary data.
#
# - When ambiguity remains, that name is not resolvable to a language code.
#   Resolving the name might require a more specific name, or specifying the
#   language that the name is in.


AMBIGUOUS_PREFERENCES = {
    # Prefer 'Micronesia' to refer to the Federated States of Micronesia -
    # this seems to be poorly disambiguated in many languages, but we can't
    # do much with a code for the general region of Micronesia
    'FM': {'057'},
    # Prefer the country of South Africa over the general region of southern
    # Africa, in languages that don't distinguish them
    'ZA': {'018'},
    # Prefer territory 003 for 'North America', which includes Central America
    # and the Caribbean, over territory 021, which excludes them
    '003': {'021'},
    # Prefer territory 005 for 'Lulli-Amerihkká' (South America), over territory
    # 419, which includes Central America
    '005': {'419'},
    # If a name like "Amerika" is ambiguous between the Americas and the United
    # States of America, choose the Americas
    '019': {'US'},
    # Prefer 'Swiss German' to be a specific language
    'gsw': {'de-CH'},
    # Of the two countries named 'Congo', prefer the one with Kinshasa
    'CD': {'CG'},
    # Prefer Han script to not include bopomofo
    'Hani': {'Hanb'},
    # Prefer the specific language Tagalog over standard Filipino, because
    # the ambiguous name was probably some form of 'Tagalog'
    'tl': {'fil'},
    # Confusion between Ilokano and Hiligaynon
    'ilo': {'hil'},
    # Prefer Central Atlas Tamazight over Standard Moroccan Tamazight
    'tzm': {'zgh'},
    # Prefer the specific definition of Low Saxon
    'nds-NL': {'nds'},
    # Prefer the specific definition of Mandarin Chinese
    'cmn': {'zh'},
    # Prefer the territorially-specific definition of Dari
    'fa-AF': {'prs', 'fa', 'gbz'},
    # Ambiguity in the scope of Korean script (whether to include Han characters)
    'Kore': {'Hang'},
    # This ambiguity is kind of our fault, for adding an autonym for 'zsm'.
    # "Bahasa Malaysia" should still resolve to the more expected 'ms'.
    'ms': {'zsm'},
    # I think that the CLDR data for Mazanderani confuses Latvia and Lithuania,
    # and Wikipedia tells me it means Latvia. I should make this a CLDR issue
    'lv': {'lt'},
    'LV': {'LT'},
}

OVERRIDES = {
    # When I ask Wiktionary, it tells me that "Breatnais" is Scots Gaelic for
    # Welsh, not Breton, which is "Breatannais". This may be one of those
    # things that's not as standardized as it sounds, but let's at least agree
    # with Wiktionary and avoid a name conflict.
    ("gd", "br"): "Breatannais",
    # 'tagaloga' should be 'tl', not 'fil'
    ("eu", "tl"): "Tagaloga",
    ("eu", "fil"): "Filipinera",
    # 'Dakota' should be 'dak', not 'dar', which is "Dargwa"
    ("af", "dar"): "Dargwa",
    ("af-NA", "dar"): "Dargwa",
    # 'интерлингве' should be 'ie', not 'ia', which is 'интерлингва'
    ("az-Cyrl", "ia"): "интерлингва",
    # Don't confuse Samaritan Hebrew with Samaritan Aramaic
    ("en", "smp"): "Samaritan Hebrew",
    # Don't confuse the Mongol language of New Guinea with Mongolian
    ("en", "mgt"): "Mongol (New Guinea)",
    # Don't confuse Romang with Romani over the name 'Roma'
    ("en", "rmm"): "Romang",
    # 'Tai' is a large language family, and it should not refer exclusively and
    # unrelatedly to a language spoken by 900 people in New Guinea
    ("en", "taw"): "Kalam-Tai",
    # The code for Ladin -- the language that's almost certainly being named in
    # Friulian here -- is "lld". The given code of "lad" seems to be an error,
    # pointing to the Judeo-Spanish language Ladino, which would be less likely
    # to be what you mean when speaking Friulian.
    ("fur", "lad"): None,
    # The Amharic data in v39 appears to have switched the words for 'Western'
    # and 'Eastern'.
    ("am", "011"): "ምዕራባዊ አፍሪካ",  # Western Africa
    ("am", "014"): "ምስራቃዊ አፍሪካ",  # Eastern Africa
    ("am", "155"): "ምዕራባዊ አውሮፓ",  # Western Europe
    ("am", "151"): "ምስራቃዊ አውሮፓ",  # Eastern Europe
}


def resolve_name(key, vals, debug=False):
    """
    Given a name, and a number of possible values it could resolve to,
    find the single value it should resolve to, in the following way:

    - Apply the priority order
    - If names with the highest priority all agree, use that name
    - If there is disagreement that can be resolved by AMBIGUOUS_PREFERENCES,
      use that
    - Otherwise, don't resolve the name (and possibly show a debugging message
      when building the data)
    """
    max_priority = max([val[2] for val in vals])
    val_count = Counter([val[1] for val in vals if val[2] == max_priority])
    if len(val_count) == 1:
        unanimous = val_count.most_common(1)
        return unanimous[0][0]

    for pkey in val_count:
        if pkey in AMBIGUOUS_PREFERENCES:
            others = set(val_count)
            others.remove(pkey)
            if others == others & AMBIGUOUS_PREFERENCES[pkey]:
                if debug:
                    print("Resolved: {} -> {}".format(key, pkey))
                return pkey

    # In debug mode, show which languages vote for which name
    if debug and max_priority >= 0:
        votes = defaultdict(list)
        for voter, val, prio in vals:
            if prio == max_priority:
                votes[val].append(voter)

        print("{}:".format(key))
        for val, voters in sorted(votes.items()):
            print("\t{}: {}".format(val, ' '.join(voters)))

    # Don't use names that remain ambiguous
    return None


def resolve_names(name_dict, debug=False):
    resolved = {}
    for key, vals in sorted(name_dict.items()):
        resolved_name = resolve_name(key, vals, debug=debug)
        if resolved_name is not None:
            resolved[key] = resolved_name
    return resolved


def read_cldr_names(language, category):
    """
    Read CLDR's names for things in a particular language.
    """
    filename = data_filename(
        'cldr-json/cldr-json/cldr-localenames-full/main/{}/{}.json'.format(language, category)
    )
    fulldata = json.load(open(filename, encoding='utf-8'))
    data = fulldata['main'][language]['localeDisplayNames'][category]
    return data


def read_cldr_name_file(langcode, category):
    data = read_cldr_names(langcode, category)
    name_quads = []
    for subtag, name in sorted(data.items()):
        if (langcode, subtag) in OVERRIDES:
            name = OVERRIDES[langcode, subtag]
            if name is None:
                continue

        if subtag == name:
            # Default entries that map a language code to itself, which
            # an inattentive annotator just left there
            continue

        priority = 3
        if subtag.endswith('-alt-menu') and name == 'mandarin':
            # The -alt-menu entries are supposed to do things like alphabetize
            # "Mandarin Chinese" under "Chinese, Mandarin". A few languages
            # just put the string "mandarin" there, which seems wrong and
            # messes up our name lookups.
            continue

        # CLDR assigns multiple names to one code by adding -alt-* to
        # the end of the code. For example, the English name of 'az' is
        # Azerbaijani, but the English name of 'az-alt-short' is Azeri.
        if '-alt-' in subtag:
            subtag, _ = subtag.split('-alt-', 1)
            priority = 1

        if normalize_name(name) == normalize_name(subtag):
            # Giving the name "zh (Hans)" to "zh-Hans" is still lazy
            continue

        name_quads.append((langcode, subtag, name, priority))
    return name_quads


def read_iana_registry_names():
    language_quads = []
    script_quads = []
    territory_quads = []
    for entry in parse_registry():
        target = None
        if entry['Type'] == 'language':
            target = language_quads
        elif entry['Type'] == 'script':
            target = script_quads
        elif entry['Type'] == 'region':
            # IANA's terminology is 'region' where CLDR's is 'territory'
            target = territory_quads
        if target is not None:
            subtag = entry['Subtag']
            priority = 2
            if 'Deprecated' in entry:
                priority = 0
            if ('en', subtag) in OVERRIDES:
                target.append(('en', subtag, OVERRIDES['en', subtag], priority))
            else:
                for desc in entry['Description']:
                    target.append(('en', subtag, desc, priority))
    return language_quads, script_quads, territory_quads


def read_iana_registry_macrolanguages():
    macros = {}
    for entry in parse_registry():
        if entry['Type'] == 'language' and 'Macrolanguage' in entry:
            macros[entry['Subtag']] = entry['Macrolanguage']
    return macros


def read_iana_registry_replacements():
    replacements = {}
    for entry in parse_registry():
        if entry['Type'] == 'language' and 'Preferred-Value' in entry:
            # Replacements for language codes
            replacements[entry['Subtag']] = entry['Preferred-Value']
        elif 'Tag' in entry and 'Preferred-Value' in entry:
            # Replacements for entire tags
            replacements[entry['Tag'].lower()] = entry['Preferred-Value']
    return replacements


def read_csv_names(filename):
    data = open(filename, encoding='utf-8')
    quads = []
    for line in data:
        quad = line.rstrip().split(',', 3) + [True]
        quads.append(tuple(quad))
    return quads


def read_wiktionary_names(filename, language):
    data = open(filename, encoding='utf-8')
    quads = []
    for line in data:
        parts = line.rstrip().split('\t')
        code = parts[0]
        quads.append((language, code, parts[1], -1))
        names = [parts[1]]
        if len(parts) > 4 and parts[4]:
            names = parts[4].split(', ')
            for name in names:
                quads.append((language, code, name, -2))
    return quads


def update_names(names_fwd, names_rev, name_quads):
    for name_language, referent, name, priority in name_quads:
        # Get just the language from name_language, not the territory or script.
        short_language = name_language.split('-')[0]
        rev_all = names_rev.setdefault('und', {})
        rev_language = names_rev.setdefault(short_language, {})
        for rev_dict in (rev_all, rev_language):
            rev_dict.setdefault(normalize_name(name), []).append(
                (name_language, referent, priority)
            )

        names_for_referent = names_fwd.setdefault(referent, {})
        if name_language not in names_for_referent:
            names_for_referent[name_language] = name


def save_trie(mapping, filename):
    trie = marisa_trie.BytesTrie(
        (key, value.encode('utf-8')) for (key, value) in sorted(mapping.items())
    )
    trie.save(filename)


def save_reverse_name_tables(category, rev_dict):
    for language, lang_dict in rev_dict.items():
        os.makedirs(data_filename(f"trie/{language}"), exist_ok=True)
        save_trie(
            resolve_names(lang_dict, debug=True),
            data_filename(f"trie/{language}/name_to_{category}.marisa"),
        )


def get_name_languages():
    cldr_main_path = Path(data_filename("cldr-json/cldr-json/cldr-localenames-full/main"))
    languages = [
        subpath.name
        for subpath in sorted(cldr_main_path.iterdir())
        if subpath.name != 'root' and (subpath / 'languages.json').exists()
    ]
    return [language for language in languages if 'a' <= language[-1] <= 'z']


def get_population_data():
    import langcodes

    filename = data_filename("supplementalData.xml")
    root = ET.fromstring(open(filename).read())
    territories = root.findall("./territoryInfo/territory")

    language_population = defaultdict(int)
    language_writing_population = defaultdict(int)

    for territory in territories:
        t_code = territory.attrib['type']
        t_population = float(territory.attrib['population'])
        t_literacy_rate = float(territory.attrib['literacyPercent']) / 100

        for language in territory:
            attrs = language.attrib
            l_code = attrs['type'].replace('_', '-')
            l_proportion = float(attrs.get('populationPercent', 0)) / 100
            if 'writingPercent' in attrs:
                writing_prop = float(attrs['writingPercent']) / 100
            elif 'literacyPercent' in attrs:
                writing_prop = float(attrs['literacyPercent']) / 100
            else:
                writing_prop = t_literacy_rate

            l_population = t_population * l_proportion
            l_writing = t_population * l_proportion * writing_prop

            # Distinguish data in different territories, and also in different
            # scripts when necessary, while also accumulating more general data

            # We need to use maximize() on the bare language code, not just
            # assume_script(), because assumed defaults like 'zh-Hans' are unwritten
            # in the data. We need this if we want to count the relative use of
            # Simplified vs. Traditional Chinese, for example.
            written_ls = (
                langcodes.get(l_code).maximize()._filter_attributes(['language', 'script'])
            )
            written_lst = written_ls.update_dict({'territory': t_code})

            spoken_lt = written_lst._filter_attributes(['language', 'territory'])
            spoken_l = written_lst._filter_attributes(['language'])

            written_lt = written_lst._filter_attributes(['language', 'territory'])
            written_l = written_lst._filter_attributes(['language'])

            for lang in set([spoken_lt, spoken_l]):
                language_population[str(lang)] += int(round(l_population))

            for lang in set([written_lst, written_lt, written_ls, written_l]):
                language_writing_population[str(lang)] += int(round(l_writing))

    return language_population, language_writing_population


def write_python_dict(outfile, name, d):
    """
    Write Python code that initializes a given dictionary, with one value on
    each line.
    """
    print(f"{name} = {{", file=outfile)
    for key, value in sorted(d.items()):
        print(f"    {key!r}: {value!r},", file=outfile)
    print("}", file=outfile)


def write_python_set(outfile, name, s):
    print(f"{name} = {{", file=outfile)
    for key in sorted(set(s)):
        print(f"    {key!r},", file=outfile)
    print("}", file=outfile)


GENERATED_HEADER = "# This file is generated by build_data.py."


def build_data():
    language_names_rev = {}
    territory_names_rev = {}
    script_names_rev = {}
    names_fwd = {}

    override_language_data = read_csv_names(data_filename('override_language_names.csv'))
    update_names(names_fwd, language_names_rev, override_language_data)

    for langcode in get_name_languages():
        language_data = read_cldr_name_file(langcode, 'languages')
        update_names(names_fwd, language_names_rev, language_data)

        try:
            script_data = read_cldr_name_file(langcode, 'scripts')
            update_names(names_fwd, script_names_rev, script_data)
        except FileNotFoundError:
            pass

        try:
            territory_data = read_cldr_name_file(langcode, 'territories')
            update_names(names_fwd, territory_names_rev, territory_data)
        except FileNotFoundError:
            pass

    iana_languages, iana_scripts, iana_territories = read_iana_registry_names()
    update_names(names_fwd, language_names_rev, iana_languages)
    update_names(names_fwd, script_names_rev, iana_scripts)
    update_names(names_fwd, territory_names_rev, iana_territories)

    wiktionary_data = read_wiktionary_names(data_filename('wiktionary/codes-en.csv'), 'en')
    update_names(names_fwd, language_names_rev, wiktionary_data)

    extra_language_data = read_csv_names(data_filename('extra_language_names.csv'))
    update_names(names_fwd, language_names_rev, extra_language_data)

    save_reverse_name_tables('language', language_names_rev)
    save_reverse_name_tables('script', script_names_rev)
    save_reverse_name_tables('territory', territory_names_rev)

    # Get the list of languages where we have any name data. These are base
    # language codes (without scripts or territories) which contain a name for
    # themselves.
    name_languages = [
        langcode
        for langcode in get_name_languages()
        if '-' not in langcode and langcode in names_fwd and langcode in names_fwd[langcode]
    ]

    # Add the languages that have autonyms in extra_language_data, perhaps because
    # we specifically put them there to get their autonyms right
    name_languages += [lang1 for (lang1, lang2, _, _) in extra_language_data if lang1 == lang2]

    # Write the contents of name_data.py.
    with open('name_data.py', 'w', encoding='utf-8') as outfile:
        print(GENERATED_HEADER, file=outfile)
        write_python_set(outfile, 'LANGUAGES_WITH_NAME_DATA', name_languages)
        print(file=outfile)
        write_python_dict(outfile, 'CODE_TO_NAMES', names_fwd)

    language_population, language_writing_population = get_population_data()
    with open('population_data.py', 'w', encoding='utf-8') as outfile:
        print(GENERATED_HEADER, file=outfile)
        write_python_dict(outfile, 'LANGUAGE_SPEAKING_POPULATION', language_population)
        write_python_dict(outfile, 'LANGUAGE_WRITING_POPULATION', language_writing_population)


if __name__ == '__main__':
    build_data()
