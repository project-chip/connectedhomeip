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


def try_apply_float_to_integer_fix(value):
    '''Fix math operations where values ends up beeing a float for integer types.

    For example one of the color control test configure the ColoremperatureMireds value to be:
        (ColorTempPhysicalMinMireds + ColorTempPhysicalMaxMireds)/2
    In this specific example it ends up as '32639.5', which is invalid.
    '''
    if isinstance(value, float):
        return int(value)
    return value


def try_apply_yaml_unrepresentable_integer_for_javascript_fixes(value):
    '''Fix up large integers that are represented within a string.

    JavaScript can not represent integers bigger than 9007199254740991. But some of the test may
    uses values that are bigger than this. The current way to workaround this limitation has
    been to write those numbers as strings by encapsulating them in "".
    Conversion happens on a best effort basis. For example, the type of value can be a string
    because a variable has been used, in which case, it would not convert to a proper int
    and we would fail at runtime.
    '''
    if type(value) is str:
        try:
            value = int(value)
        except ValueError:
            pass
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
    # Step 0: Check if this is already of type bytes
    if isinstance(s, bytes):
        return s

    # Step 1: handle explicit "hex:" prefix
    if s.startswith('hex:'):
        return binascii.unhexlify(s[4:])

    # Step 2: handle explicit "base64:" prefix
    if s.startswith('base64:'):
        return binascii.a2b_base64(s[7:])

    # Step 3: convert non-hex-prefixed to bytes
    # TODO(#23669): This does not properly support utf8 octet strings. We mimic
    # javascript codegen behavior. Behavior of javascript is:
    #   * Octet string character >= u+0200 errors out.
    #   * Any character greater than 0xFF has the upper bytes chopped off.
    as_bytes = [ord(c) for c in s]

    if any([value > 0x200 for value in as_bytes]):
        raise ValueError('Unsupported char in octet string %r' % as_bytes)
    accumulated_hex = ''.join([f"{(v & 0xFF):02x}" for v in as_bytes])
    return binascii.unhexlify(accumulated_hex)


def add_yaml_support_for_scientific_notation_without_dot(loader):
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


# This is a gross hack. The previous runner has a some internal states where an identity match one
# accessory. But this state may not exist in the runner (as in it prevent to have multiple node ids
# associated to a fabric...) so the 'nodeId' needs to be added back manually.
def try_update_yaml_node_id_test_runner_state(tests, config):
    default_identity = 'alpha'
    identities = {
        default_identity: None if 'nodeId' not in config else config['nodeId']}

    for test in tests:
        if not test.is_enabled:
            continue

        identity = test.identity

        if test.cluster == 'CommissionerCommands' or test.cluster == 'DelayCommands':
            if test.command == 'PairWithCode' or test.command == 'WaitForCommissionee':
                if test.responses_with_placeholders:
                    # It the test expects an error, we should not update the
                    # nodeId of the identity.
                    error = test.responses_with_placeholders[0].get('error')
                    if error:
                        continue

                for item in test.arguments_with_placeholders['values']:
                    if item['name'] == 'nodeId':
                        identities[identity] = item['value']
        elif identity is not None and identity in identities:
            node_id = identities[identity]
            test.node_id = node_id
        elif identity is None and default_identity in identities:
            node_id = identities[default_identity]
            test.node_id = node_id
