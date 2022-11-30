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

import stringcase
import re
from typing import Callable, List


def normalize_acronyms(s: str) -> str:
    """Replaces variations of acronyms when converting various words.

       Specifically when considering how to generate a CONST_CASE constant,
       strings such as WiFi should not be WI_FI but rather WIFI
    """
    return s.replace('WiFi', 'Wifi').replace('WI_FI', 'WIFI')


WORD_SPLIT_RE = re.compile('[- /_]')

def _splitWords(s: str) -> List[str]:
    """
    Split a string into its underlying words
    """
    return [word for word in WORD_SPLIT_RE.split(s) if word]


def _splitIntoWords(s: str, preserve_acronyms: bool, transform: Callable[[str], str]) -> List[str]:
    """
    Splits a list of items into transform(lowercase(word)) except for acronyms.
    """
    parts = []
    for word in _splitWords(s):
        if not preserve_acronyms or (word != word.upper()):
            word = transform(word.lower())
        parts.append(word)

    return parts


def asUpperCamelCase(s: str, preserve_acronyms: bool = True) -> str:
    """
    Generates a camel case string from a split word. Specifically:
      - words are considered separated by space/_/-/slash
      - each individual word will be be camel-cased
      - acronyms are considered anything that is FULLUPPERCASE

    Resulting camel case has all first letters of words as upper case.
    """
    return "".join(_splitIntoWords(s, preserve_acronyms=preserve_acronyms,transform=str.title))

def asLowerCamelCase(s: str, preserve_acronyms: bool = True) -> str:
    """
    Same as asUpperCamelCase except first word is lowercased EXCEPT if it is
    an acronym.
    """
    words = _splitIntoWords(s, preserve_acronyms=preserve_acronyms,transform=str.title)
    if not preserve_acronyms or words[0].upper() != words[0]:
        words[0] = words[0][0].lower() + words[0][1:]

    return ''.join(words)




def RegisterCommonFilters(filtermap):
    """
    Register filters that are NOT considered platform-generator specific.

    Codegen often needs standardized names, like "method names are CamelCase"
    or "command names need-to-be-spinal-case" so these filters are often
    generally registered on all generators.
    """

    # General casing for output naming
    filtermap['camelcase'] = stringcase.camelcase
    filtermap['capitalcase'] = stringcase.capitalcase
    filtermap['constcase'] = stringcase.constcase
    filtermap['pascalcase'] = stringcase.pascalcase
    filtermap['snakecase'] = stringcase.snakecase
    filtermap['spinalcase'] = stringcase.spinalcase

    filtermap['normalize_acronyms'] = normalize_acronyms


    # equivalence methods from zap templates. These are expected to
    # be identical with zap
    filtermap['asUpperCamelCase'] = asUpperCamelCase
