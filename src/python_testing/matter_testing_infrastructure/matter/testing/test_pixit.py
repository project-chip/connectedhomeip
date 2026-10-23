#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

"""Unit tests for the pixit module (declarative PIXIT support)."""

import unittest

from matter.testing.harness_params import (HarnessParamDefinition, format_declared_parameters_for_failure,
                                           format_missing_test_parameters, get_harness_param_definitions, harness_params,
                                           resolve_harness_value, validate_harness_params)
from matter.testing.matter_test_config import MatterTestConfig
from matter.testing.pixit import (_PIXIT_NO_DEFAULT, PixitDefinition, _type_to_arg_flag, format_pixit_error,
                                  format_pixit_type_errors, format_pixit_value_for_dump, get_pixit_definitions, pixit,
                                  validate_pixit_types, validate_pixits)


class TestPixitDefinition(unittest.TestCase):
    """Tests for the PixitDefinition dataclass."""

    def test_creation_with_defaults(self):
        """Test PixitDefinition is created with correct default values."""
        pixit_def = PixitDefinition(name="test_param", type=str, description="A test parameter")
        self.assertEqual(pixit_def.name, "test_param")
        self.assertEqual(pixit_def.type, str)
        self.assertEqual(pixit_def.description, "A test parameter")
        self.assertTrue(pixit_def.required)
        self.assertIs(pixit_def.default, _PIXIT_NO_DEFAULT)
        self.assertFalse(pixit_def.sensitive)

    def test_creation_optional_with_default(self):
        """Test PixitDefinition for optional PIXIT with default value."""
        pixit_def = PixitDefinition(
            name="timeout", type=int, description="Timeout in seconds", required=False, default=30, sensitive=True
        )
        self.assertEqual(pixit_def.name, "timeout")
        self.assertEqual(pixit_def.type, int)
        self.assertFalse(pixit_def.required)
        self.assertEqual(pixit_def.default, 30)
        self.assertTrue(pixit_def.sensitive)


class TestRequiresPixitDecorator(unittest.TestCase):
    """Tests for the @pixit decorator."""

    def test_decorator_attaches_single_pixit(self):
        """Verify decorator attaches _pixit_definitions to function."""

        @pixit("app_path", str, "Path to application")
        def test_method(self):
            pass

        self.assertTrue(hasattr(test_method, "_pixit_definitions"))
        self.assertEqual(len(test_method._pixit_definitions), 1)
        self.assertEqual(test_method._pixit_definitions[0].name, "app_path")
        self.assertEqual(test_method._pixit_definitions[0].type, str)
        self.assertTrue(test_method._pixit_definitions[0].required)

    def test_multiple_decorators_stack(self):
        """Verify stacking multiple @pixit accumulates all definitions."""

        @pixit("param_a", str, "First parameter")
        @pixit("param_b", int, "Second parameter", required=False, default=42)
        @pixit("param_c", bool, "Third parameter")
        def test_method(self):
            pass

        defs = test_method._pixit_definitions
        self.assertEqual(len(defs), 3)
        names = [d.name for d in defs]
        self.assertIn("param_a", names)
        self.assertIn("param_b", names)
        self.assertIn("param_c", names)

    def test_decorator_preserves_function(self):
        """Verify the decorated function is still callable and returns correctly."""

        @pixit("x", int, "Some param")
        def my_func():
            return 42

        self.assertEqual(my_func(), 42)

    def test_optional_pixit_default(self):
        """Verify optional PIXIT has correct default value in definition."""

        @pixit("timeout", int, "Timeout", required=False, default=30)
        def test_method(self):
            pass

        pixit_def = test_method._pixit_definitions[0]
        self.assertFalse(pixit_def.required)
        self.assertEqual(pixit_def.default, 30)

    def test_required_pixit_ignores_default(self):
        """Verify that default value is stored even for required PIXITs (for informational purposes)."""

        @pixit("path", str, "Required path", required=True, default="/default/path")
        def test_method(self):
            pass

        pixit_def = test_method._pixit_definitions[0]
        self.assertTrue(pixit_def.required)
        # Default is stored but won't be used for validation since it's required
        self.assertEqual(pixit_def.default, "/default/path")


class TestGetPixitDefinitions(unittest.TestCase):
    """Tests for the get_pixit_definitions helper."""

    def test_returns_definitions_for_decorated_method(self):
        """Returns definitions from decorated method."""

        @pixit("x", int, "Test param")
        def test_method(self):
            pass

        defs = get_pixit_definitions(test_method)
        self.assertEqual(len(defs), 1)
        self.assertEqual(defs[0].name, "x")

    def test_returns_empty_list_for_undecorated_method(self):
        """Returns empty list for method without @pixit."""

        def test_method(self):
            pass

        defs = get_pixit_definitions(test_method)
        self.assertEqual(defs, [])


class TestValidatePixits(unittest.TestCase):
    """Tests for the validate_pixits function."""

    def test_all_required_present(self):
        """No missing PIXITs when all required values are provided."""
        pixit_defs = [
            PixitDefinition(name="app_path", type=str, description="Path", required=True),
            PixitDefinition(name="timeout", type=int, description="Timeout", required=True),
        ]
        user_params = {"app_path": "/some/path", "timeout": 30}
        missing, optional = validate_pixits(pixit_defs, user_params)
        self.assertEqual(len(missing), 0)
        self.assertEqual(len(optional), 0)

    def test_missing_required(self):
        """Detects missing required PIXITs."""
        pixit_defs = [
            PixitDefinition(name="app_path", type=str, description="Path", required=True),
            PixitDefinition(name="port", type=int, description="Port", required=True),
        ]
        user_params = {"app_path": "/some/path"}
        missing, optional = validate_pixits(pixit_defs, user_params)
        self.assertEqual(len(missing), 1)
        self.assertEqual(missing[0].name, "port")

    def test_multiple_missing_required(self):
        """Detects all missing required PIXITs at once."""
        pixit_defs = [
            PixitDefinition(name="a", type=str, description="A", required=True),
            PixitDefinition(name="b", type=int, description="B", required=True),
            PixitDefinition(name="c", type=bool, description="C", required=True),
        ]
        user_params = {}
        missing, optional = validate_pixits(pixit_defs, user_params)
        self.assertEqual(len(missing), 3)

    def test_optional_not_missing(self):
        """Optional PIXITs not in user_params are not reported as missing."""
        pixit_defs = [
            PixitDefinition(name="opt_param", type=int, description="Optional", required=False, default=10),
        ]
        user_params = {}
        missing, optional = validate_pixits(pixit_defs, user_params)
        self.assertEqual(len(missing), 0)
        self.assertEqual(len(optional), 1)
        self.assertEqual(optional[0].name, "opt_param")

    def test_none_value_treated_as_missing(self):
        """A PIXIT present in user_params but with None value is treated as missing."""
        pixit_defs = [
            PixitDefinition(name="app_path", type=str, description="Path", required=True),
        ]
        user_params = {"app_path": None}
        missing, optional = validate_pixits(pixit_defs, user_params)
        self.assertEqual(len(missing), 1)

    def test_mixed_required_and_optional(self):
        """Mix of required and optional PIXITs validated correctly."""
        pixit_defs = [
            PixitDefinition(name="required_a", type=str, description="Required A", required=True),
            PixitDefinition(name="optional_b", type=int, description="Optional B", required=False, default=5),
            PixitDefinition(name="required_c", type=str, description="Required C", required=True),
        ]
        user_params = {"required_a": "value"}
        missing, optional = validate_pixits(pixit_defs, user_params)
        self.assertEqual(len(missing), 1)
        self.assertEqual(missing[0].name, "required_c")
        self.assertEqual(len(optional), 1)
        self.assertEqual(optional[0].name, "optional_b")


class TestFormatPixitError(unittest.TestCase):
    """Tests for the format_pixit_error function."""

    def test_error_message_lists_missing_required(self):
        """Error message includes all missing required PIXITs."""
        missing = [
            PixitDefinition(name="app_path", type=str, description="Path to app"),
            PixitDefinition(name="port", type=int, description="Server port"),
        ]
        msg = format_pixit_error("test_TC_Example_1_1", missing, [])
        self.assertIn("app_path", msg)
        self.assertIn("port", msg)
        self.assertIn("Path to app", msg)
        self.assertIn("Server port", msg)
        self.assertIn("test_TC_Example_1_1", msg)

    def test_error_message_shows_arg_flags(self):
        """Error message shows correct --xxx-arg flags for each type."""
        missing = [
            PixitDefinition(name="path", type=str, description="A path"),
            PixitDefinition(name="count", type=int, description="A count"),
        ]
        msg = format_pixit_error("test_X", missing, [])
        self.assertIn("--string-arg", msg)
        self.assertIn("--int-arg", msg)

    def test_error_message_shows_optional_with_defaults(self):
        """Error message includes optional PIXITs with their defaults."""
        missing = [PixitDefinition(name="required", type=str, description="Req")]
        optional = [
            PixitDefinition(name="timeout", type=int, description="Timeout", required=False, default=30),
        ]
        msg = format_pixit_error("test_X", missing, optional)
        self.assertIn("Available optional PIXITs:", msg)
        self.assertNotIn("(with defaults)", msg)
        self.assertIn("timeout", msg)
        self.assertIn("30", msg)

    def test_error_message_optional_without_decorator_default(self):
        """Optional PIXITs without a decorator default omit the Default line."""
        optional = [
            PixitDefinition(name="note", type=str, description="Optional note", required=False),
        ]
        msg = format_pixit_error("test_X", [], optional)
        self.assertIn("Available optional PIXITs:", msg)
        self.assertIn("note", msg)
        self.assertNotIn("Default:", msg)

    def test_error_message_no_optional_section_when_none(self):
        """Optional section is omitted when there are no optional PIXITs."""
        missing = [PixitDefinition(name="req", type=str, description="Required")]
        msg = format_pixit_error("test_X", missing, [])
        self.assertNotIn("Available optional PIXITs", msg)


class TestTypeToArgFlag(unittest.TestCase):
    """Tests for the _type_to_arg_flag helper."""

    def test_int_flag(self):
        self.assertEqual(_type_to_arg_flag(int), "int-arg")

    def test_str_flag(self):
        self.assertEqual(_type_to_arg_flag(str), "string-arg")

    def test_bool_flag(self):
        self.assertEqual(_type_to_arg_flag(bool), "bool-arg")

    def test_float_flag(self):
        self.assertEqual(_type_to_arg_flag(float), "float-arg")

    def test_unknown_type_defaults_to_string(self):
        self.assertEqual(_type_to_arg_flag(set), "string-arg")

    def test_bytes_flag(self):
        self.assertEqual(_type_to_arg_flag(bytes), "hex-arg")

    def test_list_flag(self):
        self.assertEqual(_type_to_arg_flag(list), "json-arg")

    def test_dict_flag(self):
        self.assertEqual(_type_to_arg_flag(dict), "json-arg")

    def test_truly_unknown_type_defaults_to_string(self):
        self.assertEqual(_type_to_arg_flag(object), "string-arg")


class TestDecoratorPrecedence(unittest.TestCase):
    """Tests for decorator precedence after insert(0, ...) fix."""

    def test_outermost_decorator_first_in_list(self):
        """Verify outermost decorator's definition appears first in the list."""

        @pixit("outer", str, "Outer param")
        @pixit("inner", str, "Inner param")
        def test_method(self):
            pass

        defs = test_method._pixit_definitions
        self.assertEqual(len(defs), 2)
        # With insert(0, ...), outermost should be first
        self.assertEqual(defs[0].name, "outer")
        self.assertEqual(defs[1].name, "inner")


class TestGetPixitDefinitionsNone(unittest.TestCase):
    """Tests for get_pixit_definitions None safety."""

    def test_none_returns_empty_list(self):
        """get_pixit_definitions(None) should return empty list, not raise."""
        defs = get_pixit_definitions(None)
        self.assertEqual(defs, [])


class TestValidatePixitsDeduplication(unittest.TestCase):
    """Tests for validate_pixits deduplication behavior."""

    def test_duplicate_names_not_reported_multiple_times(self):
        """Duplicate PIXIT names should only be validated once."""
        pixit_defs = [
            PixitDefinition(name="app_path", type=str, description="Path v1", required=True),
            PixitDefinition(name="app_path", type=str, description="Path v2", required=True),
        ]
        user_params = {}
        missing, optional = validate_pixits(pixit_defs, user_params)
        # Should only report once despite duplicate definitions
        self.assertEqual(len(missing), 1)
        self.assertEqual(missing[0].name, "app_path")
        # First definition (effective one) should be used
        self.assertEqual(missing[0].description, "Path v1")


class TestHarnessParamsDecorator(unittest.TestCase):
    """Tests for @harness_params."""

    def test_unknown_name_raises(self):
        with self.assertRaises(ValueError) as ctx:

            @harness_params("not_a_real_param")
            def dummy(self):
                pass

        self.assertIn("not_a_real_param", str(ctx.exception))

    def test_decorator_attaches_definitions(self):
        @harness_params("discriminator", "passcode", optional=("endpoint",))
        def test_method(self):
            pass

        defs = get_harness_param_definitions(test_method)
        self.assertEqual(len(defs), 3)
        by_name = {d.name: d.required for d in defs}
        self.assertTrue(by_name["discriminator"])
        self.assertTrue(by_name["passcode"])
        self.assertFalse(by_name["endpoint"])

    def test_get_harness_param_definitions_none_safe(self):
        self.assertEqual(get_harness_param_definitions(None), [])


class TestValidateHarnessParams(unittest.TestCase):
    """Tests for validate_harness_params."""

    def test_discriminator_satisfied_with_list(self):
        cfg = MatterTestConfig()
        cfg.discriminators = [1234]
        cfg.setup_passcodes = [20202021]
        defs = [HarnessParamDefinition("discriminator", True)]
        missing, optional = validate_harness_params(defs, cfg)
        self.assertEqual(missing, [])
        self.assertEqual(optional, [])

    def test_discriminator_satisfied_with_qr(self):
        cfg = MatterTestConfig()
        cfg.qr_code_content = ["MT:fake"]
        defs = [HarnessParamDefinition("discriminator", True)]
        missing, optional = validate_harness_params(defs, cfg)
        self.assertEqual(missing, [])

    def test_discriminator_missing(self):
        cfg = MatterTestConfig()
        defs = [HarnessParamDefinition("discriminator", True)]
        missing, optional = validate_harness_params(defs, cfg)
        self.assertEqual(len(missing), 1)
        self.assertEqual(missing[0].name, "discriminator")

    def test_optional_not_reported_missing(self):
        cfg = MatterTestConfig()
        defs = [HarnessParamDefinition("wifi_ssid", False)]
        missing, optional = validate_harness_params(defs, cfg)
        self.assertEqual(missing, [])
        self.assertEqual(len(optional), 1)


class TestFormatMissingTestParameters(unittest.TestCase):
    """Tests for combined PIXIT + harness missing message."""

    @pixit("app_path", str, "Path", required=True)
    @harness_params("discriminator")
    def _combined_method(self):
        pass

    def test_pixit_only_missing(self):
        @pixit("key", str, "K", required=True)
        def m(self):
            pass

        cfg = MatterTestConfig()
        msg = format_missing_test_parameters("test_x", m, {}, cfg)
        self.assertIsNotNone(msg)
        self.assertIn("PIXIT", msg)
        self.assertIn("key", msg)
        self.assertNotIn("harness", msg.lower())

    def test_harness_only_missing(self):
        @harness_params("discriminator")
        def m(self):
            pass

        cfg = MatterTestConfig()
        msg = format_missing_test_parameters("test_x", m, {}, cfg)
        self.assertIsNotNone(msg)
        self.assertIn("harness", msg)
        self.assertIn("discriminator", msg)

    def test_combined_missing(self):
        cfg = MatterTestConfig()
        msg = format_missing_test_parameters("test_x", self._combined_method, {}, cfg)
        self.assertIsNotNone(msg)
        self.assertIn("app_path", msg)
        self.assertIn("discriminator", msg)

    def test_none_missing_returns_none(self):
        def m(self):
            pass

        cfg = MatterTestConfig()
        self.assertIsNone(format_missing_test_parameters("test_x", m, {}, cfg))


class TestFormatDeclaredParametersForFailure(unittest.TestCase):
    """Tests for failure dump of declared parameters."""

    @pixit("app_path", str, "Path", required=True)
    @harness_params("passcode")
    def _method(self):
        pass

    def test_empty_method_returns_empty(self):
        def m(self):
            pass

        self.assertEqual(format_declared_parameters_for_failure(m, {}, MatterTestConfig()), "")

    def test_includes_pixit_and_harness_labels(self):
        cfg = MatterTestConfig()
        cfg.discriminators = [1]
        cfg.setup_passcodes = [99]
        user = {"app_path": "/tmp/app"}
        out = format_declared_parameters_for_failure(self._method, user, cfg)
        self.assertIn("[pixit]", out)
        self.assertIn("[harness]", out)
        self.assertIn("app_path", out)
        self.assertIn("***REDACTED***", out)

    def test_sensitive_pixit_redacted_in_dump(self):
        @pixit("secret_token", str, "Token", required=True, sensitive=True)
        def m(self):
            pass

        user = {"secret_token": "super-secret"}
        out = format_declared_parameters_for_failure(m, user, MatterTestConfig())
        self.assertIn("***REDACTED***", out)
        self.assertNotIn("super-secret", out)

    def test_non_sensitive_pixit_shown_in_dump(self):
        @pixit("app_path", str, "Path", required=True)
        def m(self):
            pass

        user = {"app_path": "/tmp/app"}
        out = format_declared_parameters_for_failure(m, user, MatterTestConfig())
        self.assertIn("'/tmp/app'", out)


class TestResolveHarnessValue(unittest.TestCase):
    """Tests for resolve_harness_value."""

    def test_resolve_discriminator(self):
        cfg = MatterTestConfig()
        cfg.discriminators = [3840]
        self.assertEqual(resolve_harness_value("discriminator", cfg), 3840)

    def test_resolve_discriminator_qr_runtime_is_none(self):
        cfg = MatterTestConfig()
        cfg.qr_code_content = ["MT:fake"]
        self.assertIsNone(resolve_harness_value("discriminator", cfg))

    def test_discriminator_qr_display_placeholder_in_dump(self):
        @harness_params("discriminator")
        def m(self):
            pass

        cfg = MatterTestConfig()
        cfg.qr_code_content = ["MT:fake"]
        out = format_declared_parameters_for_failure(m, {}, cfg)
        self.assertIn("<from --qr-code>", out)

    def test_resolve_discriminator_manual_runtime_is_none(self):
        cfg = MatterTestConfig()
        cfg.manual_code = ["34970112332"]
        self.assertIsNone(resolve_harness_value("discriminator", cfg))

    def test_discriminator_manual_display_placeholder_in_dump(self):
        @harness_params("discriminator")
        def m(self):
            pass

        cfg = MatterTestConfig()
        cfg.manual_code = ["34970112332"]
        out = format_declared_parameters_for_failure(m, {}, cfg)
        self.assertIn("<from --manual-code>", out)

    def test_unknown_raises(self):
        with self.assertRaises(ValueError):
            resolve_harness_value("nope", MatterTestConfig())


class TestValidatePixitTypes(unittest.TestCase):
    """Tests for strict PIXIT type validation at setup."""

    def test_correct_int_type_passes(self):
        defs = [PixitDefinition(name="timeout_sec", type=int, description="Timeout")]
        user_params = {"timeout_sec": 30}
        self.assertEqual(validate_pixit_types(defs, user_params), [])

    def test_string_for_int_fails(self):
        defs = [PixitDefinition(name="timeout_sec", type=int, description="Timeout")]
        user_params = {"timeout_sec": "30"}
        errors = validate_pixit_types(defs, user_params)
        self.assertEqual(len(errors), 1)
        self.assertIn("timeout_sec", errors[0])
        self.assertIn("int-arg", errors[0])

    def test_non_numeric_string_for_int_fails(self):
        defs = [PixitDefinition(name="timeout_sec", type=int, description="Timeout")]
        user_params = {"timeout_sec": "not-a-number"}
        errors = validate_pixit_types(defs, user_params)
        self.assertEqual(len(errors), 1)
        self.assertIn("timeout_sec", errors[0])

    def test_string_for_bool_fails(self):
        defs = [PixitDefinition(name="flag", type=bool, description="Flag")]
        user_params = {"flag": "False"}
        errors = validate_pixit_types(defs, user_params)
        self.assertEqual(len(errors), 1)
        self.assertIn("bool-arg", errors[0])

    def test_correct_bool_type_passes(self):
        defs = [PixitDefinition(name="flag", type=bool, description="Flag")]
        user_params = {"flag": False}
        self.assertEqual(validate_pixit_types(defs, user_params), [])

    def test_format_pixit_type_errors(self):
        msg = format_pixit_type_errors("test_x", ["err one", "err two"])
        self.assertIn("test_x", msg)
        self.assertIn("err one", msg)

    def test_format_pixit_value_for_dump_sensitive(self):
        pdef = PixitDefinition(name="tok", type=str, description="Token", sensitive=True)
        self.assertEqual(format_pixit_value_for_dump(pdef, {"tok": "secret"}), "***REDACTED***")


class TestFormatMissingTestParametersTypes(unittest.TestCase):
    """Type errors surface via format_missing_test_parameters after presence passes."""

    def test_type_error_when_present_but_wrong_type(self):
        @pixit("count", int, "Count", required=True)
        def m(self):
            pass

        cfg = MatterTestConfig()
        msg = format_missing_test_parameters("test_x", m, {"count": "30"}, cfg)
        self.assertIsNotNone(msg)
        self.assertIn("count", msg)
        self.assertIn("int-arg", msg)


# NOTE: Tests for MatterBaseTest.pixit() default resolution live in
# src/python_testing/test_testing/TestPixitSupport.py. They import MatterBaseTest,
# which pulls in the compiled `matter` wheel, so they cannot run in this
# lightweight pigweed unit-test package and run in the REPL env (out/venv) instead.


if __name__ == "__main__":
    unittest.main()
