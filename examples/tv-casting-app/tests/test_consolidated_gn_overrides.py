"""
Property Test — Property 2: GN files resolve all overrides from the override directory

Feature: consolidate-overrides-and-generation-script

**Validates: Requirements 3.1, 3.2, 4.1, 4.2, 5.1, 14.4**

For each of the 5 well-known filenames, parse the corresponding GN consumer
file and verify it references `chip_data_model_overrides_dir` in its
conditional logic for selecting that source file, and falls back to the
default full generated source when the override dir is empty.

Well-known filenames and their consumers:
- cluster-objects-override.cpp -> src/app/common/BUILD.gn
- CHIPAttributeTLVValueDecoder-override.cpp -> src/controller/java/BUILD.gn
- CHIPEventTLVValueDecoder-override.cpp -> src/controller/java/BUILD.gn
- Accessors-override.cpp -> src/app/chip_data_model.gni
- cluster-servers-override.gni -> src/app/chip_data_model.gni
"""

import os
import re

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))

# Consumer files
COMMON_BUILD_GN = os.path.join(REPO_ROOT, "src", "app", "common", "BUILD.gn")
JAVA_BUILD_GN = os.path.join(REPO_ROOT, "src", "controller", "java", "BUILD.gn")
CHIP_DATA_MODEL_GNI = os.path.join(REPO_ROOT, "src", "app", "chip_data_model.gni")

# ---------------------------------------------------------------------------
# Well-known filenames and their consumer files
# ---------------------------------------------------------------------------

OVERRIDE_ENTRIES = [
    ("cluster-objects-override.cpp", COMMON_BUILD_GN),
    ("CHIPAttributeTLVValueDecoder-override.cpp", JAVA_BUILD_GN),
    ("CHIPEventTLVValueDecoder-override.cpp", JAVA_BUILD_GN),
    ("Accessors-override.cpp", CHIP_DATA_MODEL_GNI),
    ("cluster-servers-override.gni", CHIP_DATA_MODEL_GNI),
]

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


# ---------------------------------------------------------------------------
# Property-based test
# ---------------------------------------------------------------------------


@given(
    override_entry=st.sampled_from(OVERRIDE_ENTRIES),
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_gn_files_resolve_overrides_from_directory(override_entry):
    """
    **Validates: Requirements 3.1, 3.2, 4.1, 4.2, 5.1, 14.4**

    Property 2: For any of the 5 well-known override filenames, the
    corresponding GN consumer file SHALL reference
    `chip_data_model_overrides_dir` in its conditional logic for selecting
    that source file.
    """
    well_known_filename, consumer_file = override_entry

    content = _read_file(consumer_file)
    rel_path = os.path.relpath(consumer_file, REPO_ROOT)

    # The consumer file must reference chip_data_model_overrides_dir
    assert "chip_data_model_overrides_dir" in content, (
        f"`{rel_path}` does not reference `chip_data_model_overrides_dir` "
        f"at all — it should use the override directory for {well_known_filename}"
    )

    # The consumer file must reference the well-known filename
    assert well_known_filename in content, (
        f"`{rel_path}` does not reference `{well_known_filename}` — "
        f"the override file should be used when chip_data_model_overrides_dir is set"
    )

    # Verify the conditional pattern: if (chip_data_model_overrides_dir != "")
    conditional_pattern = r'if\s*\(\s*chip_data_model_overrides_dir\s*!=\s*""\s*\)'
    assert re.search(conditional_pattern, content), (
        f"`{rel_path}` does not contain the expected conditional "
        f'`if (chip_data_model_overrides_dir != "")` — '
        f"the override directory check is missing"
    )

    # Verify the well-known filename appears in a line that also references
    # chip_data_model_overrides_dir (i.e., it's resolved from the directory)
    dir_ref_pattern = (
        r"chip_data_model_overrides_dir.*" + re.escape(well_known_filename)
    )
    assert re.search(dir_ref_pattern, content), (
        f"`{rel_path}` references `{well_known_filename}` but not in "
        f"combination with `chip_data_model_overrides_dir` — the file should "
        f"be resolved from the override directory"
    )


@given(
    override_entry=st.sampled_from([
        ("cluster-objects-override.cpp", COMMON_BUILD_GN, "cluster-objects.cpp"),
        ("CHIPAttributeTLVValueDecoder-override.cpp", JAVA_BUILD_GN,
         "CHIPAttributeTLVValueDecoder.cpp"),
        ("CHIPEventTLVValueDecoder-override.cpp", JAVA_BUILD_GN,
         "CHIPEventTLVValueDecoder.cpp"),
        ("Accessors-override.cpp", CHIP_DATA_MODEL_GNI, "Accessors.cpp"),
    ]),
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_gn_files_have_fallback_to_default_source(override_entry):
    """
    **Validates: Requirements 3.1, 4.1, 4.2, 5.1**

    Property 2 (fallback): For each well-known override filename that has
    a corresponding full generated source, the GN consumer SHALL have an
    `else` branch that falls back to the default full generated source.
    """
    well_known_filename, consumer_file, fallback_filename = override_entry

    content = _read_file(consumer_file)
    rel_path = os.path.relpath(consumer_file, REPO_ROOT)

    # The consumer file must contain the fallback filename in an else branch
    assert fallback_filename in content, (
        f"`{rel_path}` does not reference the fallback file "
        f"`{fallback_filename}` — there should be an else branch that "
        f"compiles the full generated source when chip_data_model_overrides_dir "
        f"is empty"
    )

    # Verify there's an else clause after the override conditional
    # (simple check: "} else {" appears in the file)
    assert "} else {" in content or "} else if" in content, (
        f"`{rel_path}` does not contain an else clause — the override "
        f"conditional should have a fallback path"
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running GN override resolution property tests...")
    tests = [
        ("Property 2a: GN files resolve overrides from directory",
         test_gn_files_resolve_overrides_from_directory),
        ("Property 2b: GN files have fallback to default source",
         test_gn_files_have_fallback_to_default_source),
    ]
    all_passed = True
    for name, test_fn in tests:
        try:
            test_fn()
            print(f"  PASS: {name}")
        except AssertionError as e:
            print(f"  FAIL: {name}\n    {e}")
            all_passed = False
        except Exception as e:
            print(f"  ERROR: {name}\n    {e}")
            all_passed = False

    sys.exit(0 if all_passed else 1)
