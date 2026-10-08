"""
Property Test — Property 6: Generation script Accessors extraction

Feature: consolidate-overrides-and-generation-script

**Validates: Requirements 9.7, 10.4**

For any valid subset of cluster names from the full Accessors.cpp,
the generation script SHALL produce a Accessors-override.cpp that
contains exactly the namespace blocks for the specified clusters,
and no namespace blocks for clusters not in the subset.
"""

import os
import re
import sys
import tempfile

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
FULL_ACCESSORS = os.path.join(
    REPO_ROOT, "zzz_generated", "app-common", "app-common",
    "zap-generated", "attributes", "Accessors.cpp",
)

# Import the generation script
sys.path.insert(0, os.path.join(REPO_ROOT, "examples", "tv-casting-app", "tv-casting-common"))
from generate_slim_overrides import _extract_cluster_namespace_blocks, generate_accessors  # noqa: E402

# ---------------------------------------------------------------------------
# Discover valid cluster names (those that actually have namespace blocks)
# ---------------------------------------------------------------------------


def _discover_accessor_cluster_names(source_path):
    """Find cluster names that have top-level namespace blocks in Accessors.cpp.

    We use the same extraction logic as the generation script to identify
    which names are valid cluster-level namespaces (as opposed to attribute
    namespaces nested inside clusters).
    """
    with open(source_path, "r") as f:
        text = f.read()

    # Use a large set of candidate names to discover which ones produce blocks
    # We look for namespace X { patterns after namespace Clusters {
    clusters_match = re.search(r'namespace\s+Clusters\s*\{', text)
    if not clusters_match:
        return []

    # Find all namespace names that appear right after Clusters namespace
    # at the top level (not nested inside another cluster namespace)
    candidates = set()
    ns_pattern = re.compile(r'^namespace\s+(\w+)\s*\{', re.MULTILINE)
    for m in ns_pattern.finditer(text, clusters_match.end()):
        name = m.group(1)
        if name not in ("Attributes", "chip", "app", "Clusters"):
            candidates.add(name)

    # Filter to only names that the extraction function actually finds
    # (i.e., names that produce non-empty blocks)
    valid = []
    for name in sorted(candidates):
        blocks = _extract_cluster_namespace_blocks(text, [name])
        if blocks:
            valid.append(name)

    return valid


# Use the casting_clusters from the YAML config as our known-good cluster names
# These are the ones the script is designed to work with
CASTING_CLUSTERS = [
    "AccessControl", "AdministratorCommissioning", "BasicInformation",
    "Binding", "Descriptor", "GeneralCommissioning", "GeneralDiagnostics",
    "GroupKeyManagement", "Groupcast", "Groups", "Identify",
    "NetworkCommissioning", "OperationalCredentials",
    "AccountLogin", "ApplicationBasic", "ApplicationLauncher",
    "AudioOutput", "Channel", "ContentAppObserver", "ContentControl",
    "ContentLauncher", "KeypadInput", "LevelControl", "LowPower",
    "MediaInput", "MediaPlayback", "Messages", "OnOff",
    "TargetNavigator", "WakeOnLan",
]

# Verify these are all valid
_valid_set = set(_discover_accessor_cluster_names(FULL_ACCESSORS))
ALL_ACCESSOR_CLUSTERS = [c for c in CASTING_CLUSTERS if c in _valid_set]

# ---------------------------------------------------------------------------
# Strategies
# ---------------------------------------------------------------------------

accessor_cluster_subsets = st.lists(
    st.sampled_from(ALL_ACCESSOR_CLUSTERS),
    min_size=1,
    max_size=len(ALL_ACCESSOR_CLUSTERS),
    unique=True,
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _extract_output_namespace_clusters(text):
    """Extract top-level cluster names from the generated Accessors output.

    Uses the same extraction logic as the generation script to identify
    cluster namespace blocks, ensuring consistency.
    """
    clusters = set()
    clusters_match = re.search(r'namespace\s+Clusters\s*\{', text)
    if not clusters_match:
        return clusters

    end_match = re.search(r'}\s*//\s*namespace\s+Clusters', text, re.MULTILINE)
    if not end_match:
        return clusters

    body = text[clusters_match.end():end_match.start()]

    # Find namespace blocks at the top level by looking for patterns like:
    # namespace X {  (preceded by start-of-body or } // namespace Y)
    # We track brace depth: a namespace at depth 0 is a cluster namespace
    depth = 0
    i = 0
    while i < len(body):
        # Try to match a namespace declaration
        m = re.match(r'namespace\s+(\w+)\s*\{', body[i:])
        if m and depth == 0:
            name = m.group(1)
            if name not in ("Attributes", "chip", "app", "Clusters"):
                clusters.add(name)
            # Skip past the matched text but count the { we just saw
            depth += 1
            i += m.end()
            continue

        if body[i] == '{':
            depth += 1
        elif body[i] == '}':
            depth -= 1

        i += 1

    return clusters


# ---------------------------------------------------------------------------
# Property tests
# ---------------------------------------------------------------------------


@given(subset=accessor_cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_accessors_contains_only_requested_namespace_blocks(subset):
    """
    **Validates: Requirements 9.7, 10.4**

    For any subset of cluster names, the generated Accessors file
    SHALL contain namespace blocks only for those clusters.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "Accessors-override.cpp")
        generate_accessors(FULL_ACCESSORS, subset, output_path)

        with open(output_path, "r") as f:
            content = f.read()

        output_clusters = _extract_output_namespace_clusters(content)
        requested = set(subset)

        # All requested clusters should have namespace blocks
        missing = requested - output_clusters
        assert not missing, (
            f"Missing namespace blocks for requested clusters: {missing}"
        )

        # No extra clusters should have namespace blocks
        extra = output_clusters - requested
        assert not extra, (
            f"Namespace blocks found for clusters NOT in the subset: {extra}"
        )


@given(subset=accessor_cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_accessors_wrapped_in_chip_app_clusters_namespaces(subset):
    """
    **Validates: Requirements 9.7, 10.4**

    For any subset, the generated Accessors file SHALL wrap the cluster
    namespace blocks inside namespace chip { namespace app { namespace Clusters { }.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "Accessors-override.cpp")
        generate_accessors(FULL_ACCESSORS, subset, output_path)

        with open(output_path, "r") as f:
            content = f.read()

        assert "namespace chip {" in content, "Output must contain namespace chip {"
        assert "namespace app {" in content, "Output must contain namespace app {"
        assert "namespace Clusters {" in content, "Output must contain namespace Clusters {"
        assert "} // namespace Clusters" in content, "Output must close namespace Clusters"
        assert "} // namespace app" in content, "Output must close namespace app"
        assert "} // namespace chip" in content, "Output must close namespace chip"
