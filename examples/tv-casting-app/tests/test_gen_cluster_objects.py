"""
Property Test — Property 4: Generation script cluster-objects extraction

Feature: consolidate-overrides-and-generation-script

**Validates: Requirements 9.4, 10.4**

For any valid subset of cluster names from the full cluster-objects.cpp,
the generation script SHALL produce a cluster-objects-override.cpp that
contains exactly the #include <clusters/ClusterName/...> directives for
the specified clusters (plus shared/Structs.ipp), and no includes for
clusters not in the subset.
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
FULL_CLUSTER_OBJECTS = os.path.join(
    REPO_ROOT, "zzz_generated", "app-common", "app-common",
    "zap-generated", "cluster-objects.cpp",
)

# Import the generation script
sys.path.insert(0, os.path.join(REPO_ROOT, "examples", "tv-casting-app", "tv-casting-common"))
from generate_slim_overrides import generate_cluster_objects  # noqa: E402

# ---------------------------------------------------------------------------
# Discover all cluster names from the full source
# ---------------------------------------------------------------------------


def _discover_cluster_names(source_path):
    """Extract all unique cluster names from #include <clusters/X/...> lines."""
    names = set()
    with open(source_path, "r") as f:
        for line in f:
            m = re.match(r'#include\s+<clusters/([^/]+)/', line)
            if m and m.group(1) != "shared":
                names.add(m.group(1))
    return sorted(names)


ALL_CLUSTER_NAMES = _discover_cluster_names(FULL_CLUSTER_OBJECTS)

# ---------------------------------------------------------------------------
# Strategies
# ---------------------------------------------------------------------------

cluster_subsets = st.lists(
    st.sampled_from(ALL_CLUSTER_NAMES),
    min_size=1,
    max_size=len(ALL_CLUSTER_NAMES),
    unique=True,
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _extract_include_clusters(text):
    """Extract cluster names from #include <clusters/X/...> lines in output."""
    clusters = set()
    for line in text.splitlines():
        m = re.match(r'#include\s+<clusters/([^/]+)/', line)
        if m:
            clusters.add(m.group(1))
    return clusters


# ---------------------------------------------------------------------------
# Property tests
# ---------------------------------------------------------------------------


@given(subset=cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_cluster_objects_includes_only_requested_clusters(subset):
    """
    **Validates: Requirements 9.4, 10.4**

    For any subset of cluster names, the generated cluster-objects file
    SHALL contain #include directives only for those clusters plus shared.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "cluster-objects-override.cpp")
        generate_cluster_objects(FULL_CLUSTER_OBJECTS, subset, output_path)

        with open(output_path, "r") as f:
            content = f.read()

        included_clusters = _extract_include_clusters(content)

        # shared/Structs.ipp must always be present
        assert "shared" in included_clusters, (
            "Output must always include shared/Structs.ipp"
        )

        # Remove 'shared' for comparison
        included_clusters.discard("shared")
        requested = set(subset)

        # All requested clusters should be present
        missing = requested - included_clusters
        assert not missing, (
            f"Missing includes for requested clusters: {missing}"
        )

        # No extra clusters should be present
        extra = included_clusters - requested
        assert not extra, (
            f"Includes found for clusters NOT in the subset: {extra}"
        )


@given(subset=cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_cluster_objects_always_includes_shared_structs(subset):
    """
    **Validates: Requirements 9.4, 10.4**

    For any subset of cluster names, the generated cluster-objects file
    SHALL always include shared/Structs.ipp.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "cluster-objects-override.cpp")
        generate_cluster_objects(FULL_CLUSTER_OBJECTS, subset, output_path)

        with open(output_path, "r") as f:
            content = f.read()

        assert "#include <clusters/shared/Structs.ipp>" in content, (
            "Output must always include #include <clusters/shared/Structs.ipp>"
        )
