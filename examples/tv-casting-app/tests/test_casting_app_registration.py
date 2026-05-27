"""
Unit Tests — MCCastingApp Registration Logic

**Validates: Requirements 4.3, 4.4, 5.1, 5.2**

These tests parse `MCCastingApp.mm` to verify that the provider registration
logic in `initializeWithDataSource:` correctly handles three scenarios:

1. The optional `castingAppDidReceiveRequestForDeviceInstanceInfo:` method
   is not implemented by the data source — registration is skipped.
2. The optional method is implemented but returns nil — registration is skipped.
3. The `MCDeviceInstanceInfoProvider::Initialize()` call fails — an error is
   returned and initialization does not proceed.

Feature: ios-device-instance-info-provider
"""

import os
import re

import pytest

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
MC_CASTING_APP_MM = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "darwin",
    "MatterTvCastingBridge",
    "MatterTvCastingBridge",
    "MCCastingApp.mm",
)
MC_DATA_SOURCE_H = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "darwin",
    "MatterTvCastingBridge",
    "MatterTvCastingBridge",
    "MCDataSource.h",
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


def _extract_initialize_method_body(content: str) -> str:
    """
    Extract the body of the `initializeWithDataSource:` method from
    MCCastingApp.mm. Returns the full method body as a string.
    """
    # Find the method signature
    pattern = r"-\s*\(NSError\s*\*\)\s*initializeWithDataSource:\(id\)dataSource\s*\{"
    match = re.search(pattern, content)
    assert match is not None, (
        "Could not find initializeWithDataSource: method in MCCastingApp.mm"
    )

    # Extract the body by counting braces
    start = match.start()
    brace_count = 0
    body_start = None
    for i in range(match.end() - 1, len(content)):
        if content[i] == "{":
            if body_start is None:
                body_start = i
            brace_count += 1
        elif content[i] == "}":
            brace_count -= 1
            if brace_count == 0:
                return content[body_start: i + 1]

    raise AssertionError(
        "Could not find matching closing brace for initializeWithDataSource:"
    )


# ---------------------------------------------------------------------------
# Test: Registration skipped when optional method is not implemented
# ---------------------------------------------------------------------------


class TestRegistrationSkippedWhenMethodNotImplemented:
    """
    **Validates: Requirements 4.3, 5.1, 5.2**

    When the data source does not implement the optional
    `castingAppDidReceiveRequestForDeviceInstanceInfo:` method,
    the code must check `respondsToSelector:` and skip provider creation.
    """

    def test_responds_to_selector_check_exists(self):
        """
        The initializeWithDataSource: method must contain a
        `respondsToSelector:` check for the optional method before
        attempting to call it.
        """
        content = _read_file(MC_CASTING_APP_MM)
        body = _extract_initialize_method_body(content)

        pattern = r"respondsToSelector:\s*@selector\s*\(\s*castingAppDidReceiveRequestForDeviceInstanceInfo:\s*\)"
        match = re.search(pattern, body)
        assert match is not None, (
            "MCCastingApp.mm initializeWithDataSource: is missing "
            "respondsToSelector: check for castingAppDidReceiveRequestForDeviceInstanceInfo:"
        )

    def test_responds_to_selector_guards_method_call(self):
        """
        The call to `castingAppDidReceiveRequestForDeviceInstanceInfo:` must
        be inside the `if` block guarded by `respondsToSelector:`. This
        ensures the method is only called when the data source implements it.
        """
        content = _read_file(MC_CASTING_APP_MM)
        body = _extract_initialize_method_body(content)

        # The respondsToSelector check should appear BEFORE the method call
        selector_pattern = r"respondsToSelector:\s*@selector\s*\(\s*castingAppDidReceiveRequestForDeviceInstanceInfo:\s*\)"
        call_pattern = r"\[\s*dataSource\s+castingAppDidReceiveRequestForDeviceInstanceInfo:"

        selector_match = re.search(selector_pattern, body)
        call_match = re.search(call_pattern, body)

        assert selector_match is not None, (
            "Missing respondsToSelector: check"
        )
        assert call_match is not None, (
            "Missing call to castingAppDidReceiveRequestForDeviceInstanceInfo:"
        )
        assert selector_match.start() < call_match.start(), (
            "The respondsToSelector: check must appear before the method call "
            "to castingAppDidReceiveRequestForDeviceInstanceInfo:"
        )

    def test_provider_creation_inside_selector_guard(self):
        """
        The MCDeviceInstanceInfoProvider creation (heap allocation) must be
        inside the respondsToSelector guard block, so it is skipped when
        the method is not implemented.
        """
        content = _read_file(MC_CASTING_APP_MM)
        body = _extract_initialize_method_body(content)

        # Find the if-block that checks respondsToSelector
        selector_pattern = r"if\s*\(\s*\[dataSource\s+respondsToSelector:\s*@selector\s*\(\s*castingAppDidReceiveRequestForDeviceInstanceInfo:\s*\)\s*\]\s*\)"
        selector_match = re.search(selector_pattern, body)
        assert selector_match is not None, (
            "Missing if-block with respondsToSelector: check"
        )

        # The provider allocation should appear after this guard
        alloc_pattern = r"new\s+matter::casting::support::MCDeviceInstanceInfoProvider\s*\(\s*\)"
        alloc_match = re.search(alloc_pattern, body)
        assert alloc_match is not None, (
            "Missing MCDeviceInstanceInfoProvider heap allocation in initializeWithDataSource:"
        )
        assert alloc_match.start() > selector_match.start(), (
            "MCDeviceInstanceInfoProvider allocation must be inside the "
            "respondsToSelector: guard block"
        )

    def test_no_set_provider_without_guard(self):
        """
        The call to SetDeviceInstanceInfoProvider must be guarded by a
        nil check on the provider pointer, ensuring it is only called
        when a provider was actually created.
        """
        content = _read_file(MC_CASTING_APP_MM)
        body = _extract_initialize_method_body(content)

        # SetDeviceInstanceInfoProvider should be guarded by a nullptr check
        set_pattern = r"SetDeviceInstanceInfoProvider\s*\("
        set_match = re.search(set_pattern, body)
        assert set_match is not None, (
            "Missing SetDeviceInstanceInfoProvider call in initializeWithDataSource:"
        )

        # There should be a nullptr check before SetDeviceInstanceInfoProvider
        guard_pattern = r"if\s*\(\s*_deviceInstanceInfoProvider\s*!=\s*nullptr\s*\)"
        guard_match = re.search(guard_pattern, body)
        assert guard_match is not None, (
            "Missing nullptr guard before SetDeviceInstanceInfoProvider call. "
            "The provider registration must be conditional on the provider being created."
        )
        assert guard_match.start() < set_match.start(), (
            "The nullptr guard must appear before the SetDeviceInstanceInfoProvider call"
        )


# ---------------------------------------------------------------------------
# Test: Registration skipped when optional method returns nil
# ---------------------------------------------------------------------------


class TestRegistrationSkippedWhenMethodReturnsNil:
    """
    **Validates: Requirements 4.3, 5.1**

    When the optional method is implemented but returns nil, the code
    must check the return value and skip provider creation.
    """

    def test_nil_check_on_device_instance_info(self):
        """
        After calling castingAppDidReceiveRequestForDeviceInstanceInfo:,
        the code must check if the returned MCDeviceInstanceInfo is non-nil
        before creating the provider.
        """
        content = _read_file(MC_CASTING_APP_MM)
        body = _extract_initialize_method_body(content)

        # The code should store the result and check for nil
        # Pattern: MCDeviceInstanceInfo * deviceInstanceInfo = [dataSource ...]
        assignment_pattern = r"MCDeviceInstanceInfo\s*\*\s*(\w+)\s*=\s*\[dataSource\s+castingAppDidReceiveRequestForDeviceInstanceInfo:"
        assignment_match = re.search(assignment_pattern, body)
        assert assignment_match is not None, (
            "Missing assignment of castingAppDidReceiveRequestForDeviceInstanceInfo: "
            "return value to a local variable"
        )

        var_name = assignment_match.group(1)

        # There should be a nil check on the variable
        nil_check_pattern = rf"if\s*\(\s*{re.escape(var_name)}\s*!=\s*nil\s*\)"
        nil_check_match = re.search(nil_check_pattern, body)
        assert nil_check_match is not None, (
            f"Missing nil check on '{var_name}' after calling "
            "castingAppDidReceiveRequestForDeviceInstanceInfo:. "
            "Provider creation must be skipped when the method returns nil."
        )

    def test_provider_creation_inside_nil_check(self):
        """
        The MCDeviceInstanceInfoProvider creation must be inside the nil
        check block, so it is skipped when the method returns nil.
        """
        content = _read_file(MC_CASTING_APP_MM)
        body = _extract_initialize_method_body(content)

        # Find the variable name
        assignment_pattern = r"MCDeviceInstanceInfo\s*\*\s*(\w+)\s*=\s*\[dataSource\s+castingAppDidReceiveRequestForDeviceInstanceInfo:"
        assignment_match = re.search(assignment_pattern, body)
        assert assignment_match is not None
        var_name = assignment_match.group(1)

        # Find the nil check
        nil_check_pattern = rf"if\s*\(\s*{re.escape(var_name)}\s*!=\s*nil\s*\)"
        nil_check_match = re.search(nil_check_pattern, body)
        assert nil_check_match is not None

        # The provider allocation must come after the nil check
        alloc_pattern = r"new\s+matter::casting::support::MCDeviceInstanceInfoProvider\s*\(\s*\)"
        alloc_match = re.search(alloc_pattern, body)
        assert alloc_match is not None, (
            "Missing MCDeviceInstanceInfoProvider heap allocation"
        )
        assert alloc_match.start() > nil_check_match.start(), (
            "MCDeviceInstanceInfoProvider allocation must be inside the "
            f"nil check on '{var_name}'"
        )


# ---------------------------------------------------------------------------
# Test: Error returned when Initialize() fails
# ---------------------------------------------------------------------------


class TestErrorOnInitializeFailure:
    """
    **Validates: Requirements 4.4, 5.2**

    When MCDeviceInstanceInfoProvider::Initialize() fails (returns a
    non-success CHIP_ERROR), the code must return an error and not
    proceed with CastingApp::Initialize().
    """

    def test_initialize_return_value_checked(self):
        """
        The return value of MCDeviceInstanceInfoProvider::Initialize()
        must be checked. If it fails, the method should return an error.
        """
        content = _read_file(MC_CASTING_APP_MM)
        body = _extract_initialize_method_body(content)

        # Look for the Initialize call with error checking
        # Pattern: VerifyOrReturnValue(_deviceInstanceInfoProvider->Initialize(...) == CHIP_NO_ERROR, ...)
        # or: if (_deviceInstanceInfoProvider->Initialize(...) != CHIP_NO_ERROR) { return ...; }
        verify_pattern = r"VerifyOrReturnValue\s*\(\s*_deviceInstanceInfoProvider->Initialize\s*\("
        if_pattern = r"_deviceInstanceInfoProvider->Initialize\s*\([^)]*\)\s*!=\s*CHIP_NO_ERROR"

        verify_match = re.search(verify_pattern, body)
        if_match = re.search(if_pattern, body)

        assert verify_match is not None or if_match is not None, (
            "MCCastingApp.mm does not check the return value of "
            "MCDeviceInstanceInfoProvider::Initialize(). If initialization "
            "fails, the method must return an error."
        )

    def test_initialize_failure_returns_error(self):
        """
        When Initialize() fails, the code must return an NSError
        (via MCErrorUtils) rather than proceeding with CastingApp::Initialize().
        """
        content = _read_file(MC_CASTING_APP_MM)
        body = _extract_initialize_method_body(content)

        # The VerifyOrReturnValue macro returns the second argument on failure.
        # Check that the second argument is an error conversion.
        verify_pattern = (
            r"VerifyOrReturnValue\s*\(\s*"
            r"_deviceInstanceInfoProvider->Initialize\s*\([^)]*\)\s*==\s*CHIP_NO_ERROR\s*,"
            r"\s*\[MCErrorUtils\s+NSErrorFromChipError:"
        )
        match = re.search(verify_pattern, body)
        assert match is not None, (
            "MCCastingApp.mm must return an NSError (via MCErrorUtils) when "
            "MCDeviceInstanceInfoProvider::Initialize() fails. Expected pattern: "
            "VerifyOrReturnValue(_deviceInstanceInfoProvider->Initialize(...) == CHIP_NO_ERROR, "
            "[MCErrorUtils NSErrorFromChipError:...])"
        )

    def test_initialize_failure_before_casting_app_initialize(self):
        """
        The MCDeviceInstanceInfoProvider::Initialize() check must occur
        before CastingApp::Initialize() is called, ensuring that a
        validation failure prevents further initialization.
        """
        content = _read_file(MC_CASTING_APP_MM)
        body = _extract_initialize_method_body(content)

        # Find the provider Initialize check
        provider_init_pattern = r"_deviceInstanceInfoProvider->Initialize\s*\("
        provider_init_match = re.search(provider_init_pattern, body)
        assert provider_init_match is not None, (
            "Missing MCDeviceInstanceInfoProvider::Initialize() call"
        )

        # Find CastingApp::Initialize()
        casting_init_pattern = r"CastingApp::GetInstance\(\)->Initialize\s*\("
        casting_init_match = re.search(casting_init_pattern, body)
        assert casting_init_match is not None, (
            "Missing CastingApp::Initialize() call"
        )

        assert provider_init_match.start() < casting_init_match.start(), (
            "MCDeviceInstanceInfoProvider::Initialize() must be called before "
            "CastingApp::Initialize() so that validation failures prevent "
            "further initialization"
        )


# ---------------------------------------------------------------------------
# Test: SetDeviceInstanceInfoProvider called after CastingApp::Initialize()
# ---------------------------------------------------------------------------


class TestProviderRegistrationOrder:
    """
    **Validates: Requirements 4.3, 5.2**

    The custom DeviceInstanceInfoProvider must be registered via
    SetDeviceInstanceInfoProvider() AFTER CastingApp::Initialize()
    succeeds, as specified in the design document.
    """

    def test_set_provider_after_casting_app_initialize(self):
        """
        SetDeviceInstanceInfoProvider must be called after
        CastingApp::Initialize() to override the default provider
        that is set up during chip stack initialization.
        """
        content = _read_file(MC_CASTING_APP_MM)
        body = _extract_initialize_method_body(content)

        casting_init_pattern = r"CastingApp::GetInstance\(\)->Initialize\s*\("
        set_provider_pattern = r"SetDeviceInstanceInfoProvider\s*\(\s*_deviceInstanceInfoProvider\s*\)"

        casting_init_match = re.search(casting_init_pattern, body)
        set_provider_match = re.search(set_provider_pattern, body)

        assert casting_init_match is not None, (
            "Missing CastingApp::Initialize() call"
        )
        assert set_provider_match is not None, (
            "Missing SetDeviceInstanceInfoProvider() call"
        )
        assert casting_init_match.start() < set_provider_match.start(), (
            "SetDeviceInstanceInfoProvider() must be called AFTER "
            "CastingApp::Initialize() so the custom provider overrides "
            "the default one set during chip stack initialization"
        )


# ---------------------------------------------------------------------------
# Test: MCDataSource protocol declares the method as @optional
# ---------------------------------------------------------------------------


class TestDataSourceProtocolDeclaration:
    """
    **Validates: Requirements 5.1, 5.2**

    The MCDataSource protocol must declare the device instance info
    method as @optional to maintain backward compatibility.
    """

    def test_optional_method_declared(self):
        """
        MCDataSource.h must contain an @optional section with the
        castingAppDidReceiveRequestForDeviceInstanceInfo: method.
        """
        content = _read_file(MC_DATA_SOURCE_H)

        # Find @optional section
        optional_match = re.search(r"@optional", content)
        assert optional_match is not None, (
            "MCDataSource.h is missing @optional section"
        )

        # The method declaration should appear after @optional
        method_pattern = r"-\s*\(\s*MCDeviceInstanceInfo\s*\*\s*_Nullable\s*\)\s*castingAppDidReceiveRequestForDeviceInstanceInfo:\s*\(\s*id\s+_Nonnull\s*\)\s*sender\s*;"
        method_match = re.search(method_pattern, content)
        assert method_match is not None, (
            "MCDataSource.h is missing the declaration of "
            "castingAppDidReceiveRequestForDeviceInstanceInfo: method"
        )
        assert method_match.start() > optional_match.start(), (
            "castingAppDidReceiveRequestForDeviceInstanceInfo: must be "
            "declared after @optional to ensure backward compatibility"
        )

    def test_no_required_methods_added(self):
        """
        The new method must not be in the @required section. All existing
        required methods must remain unchanged.
        """
        content = _read_file(MC_DATA_SOURCE_H)

        # Split content at @optional to get the required section
        optional_pos = content.find("@optional")
        assert optional_pos != -1, "Missing @optional section"

        required_section = content[:optional_pos]

        # The device instance info method should NOT appear in the required section
        method_pattern = r"castingAppDidReceiveRequestForDeviceInstanceInfo:"
        assert re.search(method_pattern, required_section) is None, (
            "castingAppDidReceiveRequestForDeviceInstanceInfo: must NOT be "
            "in the required section of MCDataSource protocol"
        )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    pytest.main([__file__, "-v"])
