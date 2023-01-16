#
#    Copyright (c) 2022 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the 'License');
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an 'AS IS' BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

import binascii
import re

'''Fixes certain value formats known to exist in YAML tests

Some of the YAML test files contains some values that has been crafted to avoid some limitations
of the original JavaScript parser and the C++ generated bits. This file contains functions to
changes and convert those YAML values them back to something agnostic.
'''


def try_apply_yaml_cpp_longlong_limitation_fix(value):
    '''Fix a known nasty hack in order that was used to work around compiler issue for C++.

    When -9223372036854775808 was provided compiler would give a warning saying it is not a valid
    way to write a long long in C++.
    '''
    if value == "-9223372036854775807LL - 1":
        value = -9223372036854775808
    return value


def try_apply_yaml_unrepresentable_integer_for_javascript_fixes(value):
    '''Fix up large integers that are represented within a string.

    JavaScript can not represent integers bigger than 9007199254740991. But some of the test may
    uses values that are bigger than this. The current way to workaround this limitation has
    been to write those numbers as strings by encapsulating them in "".
    '''
    if type(value) is str:
        value = int(value)
    return value


def try_apply_yaml_float_written_as_strings(value):
    '''Fix up floats that are represented within a string.'''
    if type(value) is str:
        value = float(value)
    return value


def convert_yaml_octet_string_to_bytes(s: str) -> bytes:
    '''Convert YAML octet string body to bytes.

    This handles any c-style hex escapes (e.g. \x5a) and hex: prefix
    '''
    # Step 1: handle explicit "hex:" prefix
    if s.startswith('hex:'):
        return binascii.unhexlify(s[4:])

    # Step 2: convert non-hex-prefixed to bytes
    # TODO(#23669): This does not properly support utf8 octet strings. We mimic
    # javascript codegen behavior. Behavior of javascript is:
    #   * Octet string character >= u+0200 errors out.
    #   * Any character greater than 0xFF has the upper bytes chopped off.
    as_bytes = [ord(c) for c in s]

    if any([value > 0x200 for value in as_bytes]):
        raise ValueError('Unsupported char in octet string %r' % as_bytes)
    accumulated_hex = ''.join([f"{(v & 0xFF):02x}" for v in as_bytes])
    return binascii.unhexlify(accumulated_hex)


def try_add_yaml_support_for_scientific_notation_without_dot(loader):
    regular_expression = re.compile(u'''^(?:
     [-+]?(?:[0-9][0-9_]*)\\.[0-9_]*(?:[eE][-+]?[0-9]+)?
    |[-+]?(?:[0-9][0-9_]*)(?:[eE][-+]?[0-9]+)
    |\\.[0-9_]+(?:[eE][-+][0-9]+)?
    |[-+]?[0-9][0-9_]*(?::[0-5]?[0-9])+\\.[0-9_]*
    |[-+]?\\.(?:inf|Inf|INF)
    |\\.(?:nan|NaN|NAN))$''', re.X)

    loader.add_implicit_resolver(
        u'tag:yaml.org,2002:float',
        regular_expression,
        list(u'-+0123456789.'))
    return loader


# This is a gross hack. The previous runner has a some internal states where an identity match one
# accessory. But this state may not exist in the runner (as in it prevent to have multiple node ids
# associated to a fabric...) so the 'nodeId' needs to be added back manually.
def try_update_yaml_node_id_test_runner_state(tests, config):
    identities = {
        'alpha': None if 'nodeId' not in config else config['nodeId']}

    for test in tests:
        if not test.is_enabled:
            continue

        identity = test.identity

        if test.cluster == 'CommissionerCommands':
            if test.command == 'PairWithCode':
                for item in test.arguments_with_placeholders['values']:
                    if item['name'] == 'nodeId':
                        identities[identity] = item['value']
        elif identity is not None and identity in identities:
            node_id = identities[identity]
            test.node_id = node_id
