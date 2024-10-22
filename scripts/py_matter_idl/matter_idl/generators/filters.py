# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import re


def normalize_acronyms(s: str) -> str:
    """Replaces variations of acronyms when converting various words.

       Specifically when considering how to generate a CONST_CASE constant,
       strings such as WiFi should not be WI_FI but rather WIFI
    """
    return s.replace('WiFi', 'Wifi').replace('WI_FI', 'WIFI')


def lowfirst(s: str) -> str:
    """Make the first letter lowercase. """
    return s[0].lower() + s[1:]


def upfirst(s: str) -> str:
    """Make the first letter uppercase """
    return s[0].upper() + s[1:]


def lowfirst_except_acronym(s: str) -> str:
    """Make the first letter lowercase assuming the string is already in
       CamelCase.

       Differs from lowfirst because it checks the string for starting with
       several uppercase, which is the case for acronyms (HVAC, ACL, WIFI),
       in which case it will NOT lowercase first
    """
    if len(s) >= 2:
        if s[1].isupper():
            return s

    return lowfirst(s)


def to_snake_case(s: str) -> str:
    """convert to snake case; all words are seperated by underscore and are lower case
       examples:
        FooBarBaz --> foo_bar_baz
        foo BarBaz --> foo_bar_baz
        FOOBarBaz --> foo_bar_baz
        _fooBar_Baz_ --> foo_bar_baz
    """
    s = "" if s is None else str(s)

    s = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', s)
    s = re.sub(r'([a-z\d])([A-Z])', r'\1_\2', s)
    s = re.sub(r'[\s\-]+', '_', s)

    snake_case = s.lower()
    return snake_case.strip('_')


def to_constant_case(s: str) -> str:
    """convert to constant case; all words are seperated by underscore and are upper case
       similar to a snake case but with upper case
       examples:
       FooBarBaz --> FOO_BAR_BAZ
       foo BarBaz --> FOO_BAR_BAZ
       FOOBarBaz --> FOO_BAR_BAZ
    """
    snake_case = to_snake_case(s)
    constant_case = snake_case.upper()
    return constant_case


def to_spinal_case(s: str) -> str:
    """convert to spinal case; all words sperated by hypen and are lower case
        similar to a snake case but with hyphen seperator instead of underscore 
        examples:
        FooBarBaz --> foo-bar-baz
        foo BarBaz --> foo-bar-baz
        FOOBarBaz --> foo-bar-baz
    """
    snake_case = to_snake_case(s)
    return snake_case.replace('_', '-')


def to_pascal_case(s: str) -> str:
    """convert to pascal case; with no spaces or underscore between words, first letter of all words is uppercase
       examples:
        fooBarBaz --> FooBarBaz
        foo BarBaz --> FooBarBaz
        FOOBar_Baz --> FooBarBaz
    """

    snake_case = to_snake_case(s)
    snake_case_split = snake_case.split('_')
    pascal_case = ''.join(word.capitalize() for word in snake_case_split)

    return pascal_case


def to_camel_case(s) -> str:
    """convert to camel case; with no spaces or underscore between words, first word all lowercase and following words are uppercase
        same as pascal case but first letter is lower case
       examples:
        FooBarBaz --> fooBarBaz
        foo BarBaz --> fooBarBaz
        FOOBarBaz --> fooBarBaz
    """
    return lowfirst(to_pascal_case(s))


def RegisterCommonFilters(filtermap):
    """
    Register filters that are NOT considered platform-generator specific.

    Codegen often needs standardized names, like "method names are CamelCase"
    or "command names need-to-be-spinal-case" so these filters are often
    generally registered on all generators.
    """

    # General casing for output naming
    filtermap['camelcase'] = to_camel_case
    filtermap['capitalcase'] = upfirst
    filtermap['constcase'] = to_constant_case
    filtermap['pascalcase'] = to_pascal_case
    filtermap['snakecase'] = to_snake_case
    filtermap['spinalcase'] = to_spinal_case

    filtermap['normalize_acronyms'] = normalize_acronyms
    filtermap['lowfirst'] = lowfirst
    filtermap['lowfirst_except_acronym'] = lowfirst_except_acronym
    filtermap['upfirst'] = upfirst
