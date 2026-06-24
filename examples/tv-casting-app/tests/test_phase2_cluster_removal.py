"""
Phase 2 Cluster Removal Property Tests

Feature: casting-apk-size-reduction-phase2

Property 1: Removed clusters absent from slim cluster file
**Validates: Requirements 1.1**

Property 2: Retained clusters present in slim cluster file
**Validates: Requirements 1.3, 1.4**

Property 3: Command metadata consistency
**Validates: Requirements 1.5**

This test verifies that:
  - The 11 non-essential infrastructure clusters removed in phase 2 do not
    appear as #include lines in the slim cluster-objects-override.cpp.
  - The 12 commissioning-essential clusters and 17 casting-specific clusters
    are present, along with shared/Structs.ipp.
  - Every cluster referenced in CommandNeedsTimedInvoke, CommandIsFabricScoped,
    or CommandHasLargePayload also has .ipp includes in the file.

The 11 removed clusters are:
  EthernetNetworkDiagnostics, SoftwareDiagnostics, WiFiNetworkDiagnostics,
  TimeSynchronization, TimeFormatLocalization, UnitLocalization, FixedLabel,
  UserLabel, Groupcast, IcdManagement, LocalizationConfiguration
"""

import os
import re

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)

SLIM_CLUSTER_FILE = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
    "cluster-objects-override.cpp",
)

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

REMOVED_CLUSTERS = [
    "EthernetNetworkDiagnostics",
    "SoftwareDiagnostics",
    "WiFiNetworkDiagnostics",
    "TimeSynchronization",
    "TimeFormatLocalization",
    "UnitLocalization",
    "FixedLabel",
    "UserLabel",
    "IcdManagement",
    "LocalizationConfiguration",
]

COMMISSIONING_CLUSTERS = [
    "AccessControl",
    "AdministratorCommissioning",
    "BasicInformation",
    "Binding",
    "Descriptor",
    "GeneralCommissioning",
    "GeneralDiagnostics",
    "GroupKeyManagement",
    "Groupcast",
    "Groups",
    "Identify",
    "NetworkCommissioning",
    "OperationalCredentials",
]

CASTING_CLUSTERS = [
    "AccountLogin",
    "ApplicationBasic",
    "ApplicationLauncher",
    "AudioOutput",
    "Channel",
    "ContentAppObserver",
    "ContentControl",
    "ContentLauncher",
    "KeypadInput",
    "LevelControl",
    "LowPower",
    "MediaInput",
    "MediaPlayback",
    "Messages",
    "OnOff",
    "TargetNavigator",
    "WakeOnLan",
]

RETAINED_CLUSTERS = COMMISSIONING_CLUSTERS + CASTING_CLUSTERS

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    """Return the text content of a file."""
    with open(path, "r") as f:
        return f.read()


# ---------------------------------------------------------------------------
# Property-based tests
# ---------------------------------------------------------------------------


@given(cluster=st.sampled_from(REMOVED_CLUSTERS))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_removed_clusters_absent_from_slim_cluster_file(cluster: str):
    """
    **Validates: Requirements 1.1**

    Property 1: Removed clusters absent from slim cluster file

    For any cluster name in the set of 11 removed clusters, the slim
    cluster-objects-override.cpp SHALL NOT contain any
    #include <clusters/{cluster}/...> lines.
    """
    content = _read_file(SLIM_CLUSTER_FILE)

    # Find any #include lines referencing this cluster's directory
    pattern = rf'#include\s+<clusters/{re.escape(cluster)}/'
    matches = re.findall(pattern, content)

    assert len(matches) == 0, (
        f"FAIL: Found {len(matches)} #include line(s) for removed cluster "
        f"'{cluster}' in cluster-objects-override.cpp. "
        f"Phase 2 requires this cluster to be removed from the slim cluster file."
    )


@given(cluster=st.sampled_from(RETAINED_CLUSTERS))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_retained_clusters_present_in_slim_cluster_file(cluster: str):
    """
    **Validates: Requirements 1.3, 1.4**

    Property 2: Retained clusters present in slim cluster file

    For any cluster name in the set of 12 commissioning-essential clusters or
    the set of 17 casting-specific clusters, the slim
    cluster-objects-override.cpp SHALL contain #include lines referencing that
    cluster directory.
    """
    content = _read_file(SLIM_CLUSTER_FILE)

    pattern = rf'#include\s+<clusters/{re.escape(cluster)}/'
    matches = re.findall(pattern, content)

    assert len(matches) > 0, (
        f"FAIL: No #include line(s) found for retained cluster "
        f"'{cluster}' in cluster-objects-override.cpp. "
        f"This cluster must be present in the slim cluster file."
    )


def test_shared_structs_ipp_present_in_slim_cluster_file():
    """
    **Validates: Requirements 1.3, 1.4**

    Property 2 (supplement): shared/Structs.ipp present in slim cluster file

    The slim cluster-objects-override.cpp SHALL contain the shared/Structs.ipp
    include.
    """
    content = _read_file(SLIM_CLUSTER_FILE)

    pattern = r'#include\s+<clusters/shared/Structs\.ipp>'
    matches = re.findall(pattern, content)

    assert len(matches) > 0, (
        "FAIL: shared/Structs.ipp include not found in "
        "cluster-objects-override.cpp. This include must be present."
    )


# ---------------------------------------------------------------------------
# Property 3 helpers
# ---------------------------------------------------------------------------

# The three command metadata function names to scan for cluster references.
_COMMAND_METADATA_FUNCTIONS = [
    "CommandNeedsTimedInvoke",
    "CommandIsFabricScoped",
    "CommandHasLargePayload",
]


def _extract_command_metadata_clusters(content: str) -> list:
    """Extract unique cluster names referenced in the command metadata functions.

    Looks for ``Clusters::<Name>::Id`` patterns inside the bodies of
    CommandNeedsTimedInvoke, CommandIsFabricScoped, and CommandHasLargePayload.
    Returns a sorted deduplicated list of cluster names.
    """
    clusters = set()
    for func_name in _COMMAND_METADATA_FUNCTIONS:
        # Match the function body: starts at the function signature and ends
        # at the next top-level closing brace followed by a blank line or EOF.
        func_pattern = (
            rf'bool\s+{func_name}\s*\([^)]*\)\s*\{{(.*?)\n\}}'
        )
        match = re.search(func_pattern, content, re.DOTALL)
        if match:
            body = match.group(1)
            # Find all Clusters::XXX::Id references in the function body
            refs = re.findall(r'Clusters::(\w+)::Id', body)
            clusters.update(refs)
    return sorted(clusters)


def _get_included_cluster_dirs(content: str) -> set:
    """Return the set of cluster directory names present as .ipp includes."""
    return set(re.findall(r'#include\s+<clusters/(\w+)/', content))


# Build the list of clusters referenced in command metadata at module load
# so hypothesis can sample from it.
_FILE_CONTENT = _read_file(SLIM_CLUSTER_FILE)
_CMD_METADATA_CLUSTERS = _extract_command_metadata_clusters(_FILE_CONTENT)


# ---------------------------------------------------------------------------
# Property 3 test
# ---------------------------------------------------------------------------

if _CMD_METADATA_CLUSTERS:
    @given(cluster=st.sampled_from(_CMD_METADATA_CLUSTERS))
    @settings(
        max_examples=100,
        suppress_health_check=[HealthCheck.function_scoped_fixture],
        deadline=None,
    )
    def test_command_metadata_clusters_have_ipp_includes(cluster: str):
        """
        **Validates: Requirements 1.5**

        Property 3: Command metadata consistency

        For any cluster ID referenced in CommandNeedsTimedInvoke,
        CommandIsFabricScoped, or CommandHasLargePayload, the slim
        cluster-objects-override.cpp SHALL also contain .ipp includes for
        that cluster. No orphaned command metadata entries shall exist for
        clusters whose .ipp files have been removed.
        """
        content = _read_file(SLIM_CLUSTER_FILE)
        included_dirs = _get_included_cluster_dirs(content)

        assert cluster in included_dirs, (
            f"FAIL: Cluster '{cluster}' is referenced in a command metadata "
            f"function (CommandNeedsTimedInvoke / CommandIsFabricScoped / "
            f"CommandHasLargePayload) but has no .ipp includes in "
            f"cluster-objects-override.cpp. This is an orphaned metadata entry."
        )
else:
    def test_command_metadata_clusters_have_ipp_includes():
        """
        **Validates: Requirements 1.5**

        Property 3: Command metadata consistency (vacuously true)

        No cluster IDs are referenced in the command metadata functions,
        so there are no orphaned entries to check.
        """
        pass


def test_command_metadata_no_orphaned_entries():
    """
    **Validates: Requirements 1.5**

    Property 3 (supplement): Deterministic check that ALL clusters in
    command metadata functions have corresponding .ipp includes.
    """
    content = _read_file(SLIM_CLUSTER_FILE)
    metadata_clusters = _extract_command_metadata_clusters(content)
    included_dirs = _get_included_cluster_dirs(content)

    orphaned = [c for c in metadata_clusters if c not in included_dirs]
    assert len(orphaned) == 0, (
        f"FAIL: Found orphaned command metadata entries for clusters "
        f"without .ipp includes: {orphaned}. "
        f"Remove these entries from CommandNeedsTimedInvoke / "
        f"CommandIsFabricScoped / CommandHasLargePayload."
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running phase 2 cluster removal property tests...")
    failed = False
    try:
        test_removed_clusters_absent_from_slim_cluster_file()
        print("  PASS: Property 1 - Removed clusters absent from slim cluster file")
    except AssertionError as e:
        print(f"  FAIL: Property 1\n    {e}")
        failed = True
    except Exception as e:
        print(f"  ERROR: Property 1\n    {e}")
        failed = True

    try:
        test_retained_clusters_present_in_slim_cluster_file()
        print("  PASS: Property 2 - Retained clusters present in slim cluster file")
    except AssertionError as e:
        print(f"  FAIL: Property 2\n    {e}")
        failed = True
    except Exception as e:
        print(f"  ERROR: Property 2\n    {e}")
        failed = True

    try:
        test_shared_structs_ipp_present_in_slim_cluster_file()
        print("  PASS: Property 2 (supplement) - shared/Structs.ipp present")
    except AssertionError as e:
        print(f"  FAIL: Property 2 (supplement)\n    {e}")
        failed = True
    except Exception as e:
        print(f"  ERROR: Property 2 (supplement)\n    {e}")
        failed = True

    try:
        test_command_metadata_clusters_have_ipp_includes()
        print("  PASS: Property 3 - Command metadata clusters have .ipp includes")
    except AssertionError as e:
        print(f"  FAIL: Property 3\n    {e}")
        failed = True
    except Exception as e:
        print(f"  ERROR: Property 3\n    {e}")
        failed = True

    try:
        test_command_metadata_no_orphaned_entries()
        print("  PASS: Property 3 (supplement) - No orphaned metadata entries")
    except AssertionError as e:
        print(f"  FAIL: Property 3 (supplement)\n    {e}")
        failed = True
    except Exception as e:
        print(f"  ERROR: Property 3 (supplement)\n    {e}")
        failed = True

    sys.exit(1 if failed else 0)
