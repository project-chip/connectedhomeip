# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import stringcase


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
    filtermap['lowfirst'] = lowfirst
    filtermap['lowfirst_except_acronym'] = lowfirst_except_acronym
    filtermap['upfirst'] = upfirst
