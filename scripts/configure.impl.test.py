# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import importlib.util
import json
import os
import tempfile
import unittest

# configure.impl uses a non-package filename; import it by path
_spec = importlib.util.spec_from_file_location("configure_impl", os.path.join(os.path.dirname(__file__), "configure.impl.py"))
impl = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(impl)


# A small fixture modeled on real `gn args --list --json` output.
# Includes: bool (default true/false), string, integer, list, and a
# collision where stripping prefixes from two args yields the same alias.
FIXTURE = [
    {"name": "chip_enable_wifi", "comment": "Enable wifi.", "default": {
        "file": "//third_party/connectedhomeip/src/platform/device.gni", "line": 69, "value": "false"}},
    {"name": "chip_automation_logging", "default": {
        "file": "//third_party/connectedhomeip/src/lib/support/logging.gni", "line": 10, "value": "true"}},
    {"name": "chip_crypto", "comment": "Crypto implementation.", "default": {
        "file": "//third_party/connectedhomeip/src/crypto/crypto.gni", "line": 17, "value": "\"\""}},
    {"name": "chip_log_message_max_size", "default": {
        "file": "//third_party/connectedhomeip/src/lib/support/logging.gni", "line": 5, "value": "-1"}},
    {"name": "chip_project_config_include_dirs", "comment": "Extra include dirs.", "default": {
        "file": "//third_party/connectedhomeip/src/system/BUILD.gn", "line": 39, "value": "[]"}},
    # Collision: both reduce to alias "debug" when chip_ prefix is stripped
    {"name": "chip_use_debug", "default": {"file": "//third_party/connectedhomeip/BUILD.gn", "line": 1, "value": "false"}},
    {"name": "chip_config_debug", "default": {"file": "//third_party/connectedhomeip/BUILD.gn", "line": 2, "value": "false"}},
    # A project-level arg (not under //third_party/connectedhomeip/)
    {"name": "project_foo", "default": {"file": "//src/args.gni", "line": 1, "value": "\"bar\""}},
    # GN built-ins needed for triplet tests (no source file)
    {"name": "target_cpu", "default": {"value": "\"\""}},
    {"name": "target_os", "default": {"value": "\"\""}},
    {"name": "host_cpu", "default": {"value": "\"\""}},
    {"name": "host_os", "default": {"value": "\"\""}},
]


def make_processor(fixture=FIXTURE):
    with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as f:
        json.dump(fixture, f)
        f.flush()
        try:
            return impl.ProjectArgProcessor(f.name)
        finally:
            os.unlink(f.name)


class TestProjectArgProcessor(unittest.TestCase):

    def test_arg_types(self):
        p = make_processor()
        self.assertEqual(p.gn_args['chip_enable_wifi'].type, 'b')
        self.assertEqual(p.gn_args['chip_automation_logging'].type, 'b')
        self.assertEqual(p.gn_args['chip_crypto'].type, 's')
        self.assertEqual(p.gn_args['chip_log_message_max_size'].type, 'i')
        self.assertEqual(p.gn_args['chip_project_config_include_dirs'].type, '[')

    def test_aliases(self):
        p = make_processor()
        # chip_enable_wifi -> shortest unique alias is "wifi"
        self.assertEqual(p.gn_args['chip_enable_wifi'].alias, 'wifi')
        self.assertIs(p.options['wifi'], p.gn_args['chip_enable_wifi'])
        # longer aliases work too
        self.assertIs(p.options['enable-wifi'], p.gn_args['chip_enable_wifi'])
        self.assertIs(p.options['chip-enable-wifi'], p.gn_args['chip_enable_wifi'])
        # chip_crypto -> "crypto"
        self.assertEqual(p.gn_args['chip_crypto'].alias, 'crypto')
        # chip_log_message_max_size -> "log-message-max-size"
        self.assertEqual(p.gn_args['chip_log_message_max_size'].alias, 'log-message-max-size')

    def test_collision_excludes_ambiguous_alias(self):
        p = make_processor()
        # Both chip_use_debug and chip_config_debug would map to "debug"
        self.assertNotIn('debug', p.options)
        # But they get longer unique aliases
        self.assertEqual(p.gn_args['chip_use_debug'].alias, 'use-debug')
        self.assertEqual(p.gn_args['chip_config_debug'].alias, 'config-debug')

    def test_process_enable_parameter(self):
        p = make_processor()
        p.process_parameters(['--enable-wifi'])
        self.assertEqual(p.args['chip_enable_wifi'], 'true')

    def test_process_enable_parameter_disable(self):
        p = make_processor()
        p.process_parameters(['--enable-wifi=no'])
        self.assertEqual(p.args['chip_enable_wifi'], 'false')

    def test_process_enable_rejects_non_bool(self):
        p = make_processor()
        with self.assertRaises(SystemExit):
            p.process_parameters(['--enable-crypto'])

    def test_process_string_parameter(self):
        p = make_processor()
        p.process_parameters(['--crypto=openssl'])
        self.assertEqual(p.args['chip_crypto'], '"openssl"')

    def test_process_integer_parameter(self):
        p = make_processor()
        p.process_parameters(['--log-message-max-size=256'])
        self.assertEqual(p.args['chip_log_message_max_size'], '256')

    def test_process_triplet(self):
        p = make_processor()
        p.process_parameters(['--target=aarch64-linux'])
        self.assertEqual(p.args['target_cpu'], '"aarch64"')
        self.assertEqual(p.args['target_os'], '"linux"')

    def test_process_triplet_with_vendor(self):
        p = make_processor()
        p.process_parameters(['--build=arm-unknown-linux-gnueabihf'])
        self.assertEqual(p.args['host_cpu'], '"arm"')
        self.assertEqual(p.args['host_os'], '"linux"')

    def test_bool_arg_rejects_generic_parameter(self):
        p = make_processor()
        self.assertEqual(p.options['wifi'].type, 'b')
        with self.assertRaises(SystemExit):
            p.process_parameters(['--wifi=yes'])

    def test_unknown_option_warns(self):
        p = make_processor()
        # Unknown options produce a warning but don't fail
        p.process_parameters(['--enable-nonexistent'])
        self.assertEqual(len(p.args), 0)

    def test_invalid_argument_format(self):
        p = make_processor()
        with self.assertRaises(SystemExit):
            p.process_parameters(['not-an-option'])

    def test_multiple_parameters(self):
        p = make_processor()
        p.process_parameters(['--enable-wifi', '--crypto=boringssl', '--log-message-max-size=512'])
        self.assertEqual(p.args['chip_enable_wifi'], 'true')
        self.assertEqual(p.args['chip_crypto'], '"boringssl"')
        self.assertEqual(p.args['chip_log_message_max_size'], '512')


if __name__ == '__main__':
    unittest.main()
