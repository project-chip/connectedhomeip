"""
Phase 2 ZAP Cluster Property Tests

Feature: casting-apk-size-reduction-phase2

Property 4: ZAP server-side disabled for removed clusters
**Validates: Requirements 2.1**

Property 5: ZAP server-side enabled for commissioning clusters
**Validates: Requirements 2.2**

This test verifies that:
  - For each of the 11 removed clusters, if a server-side entry exists in the
    ZAP file it has "enabled": 0 (absence also counts as disabled).
  - For each of the 12 commissioning-essential clusters, a server-side entry
    exists with "enabled": 1.

The 10 removed clusters (ZAP names use spaces):
  Ethernet Network Diagnostics, Software Diagnostics,
  Wi-Fi Network Diagnostics, Time Synchronization,
  Time Format Localization, Unit Localization, Fixed Label, User Label,
  ICD Management, Localization Configuration

The 13 commissioning clusters (ZAP names use spaces):
  Descriptor, Binding, Access Control, Basic Information,
  General Commissioning, Network Commissioning, General Diagnostics,
  Administrator Commissioning, Operational Credentials,
  Group Key Management, Groupcast, Identify, Groups
"""

import json
import os

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)

ZAP_FILE = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
    "tv-casting-app.zap",
)

# ---------------------------------------------------------------------------
# Constants — ZAP cluster names use spaces, not PascalCase
# ---------------------------------------------------------------------------

REMOVED_CLUSTERS_ZAP = [
    "Ethernet Network Diagnostics",
    "Software Diagnostics",
    "Wi-Fi Network Diagnostics",
    "Time Synchronization",
    "Time Format Localization",
    "Unit Localization",
    "Fixed Label",
    "User Label",
    "ICD Management",
    "Localization Configuration",
]

COMMISSIONING_CLUSTERS_ZAP = [
    "Descriptor",
    "Binding",
    "Access Control",
    "Basic Information",
    "General Commissioning",
    "Network Commissioning",
    "General Diagnostics",
    "Administrator Commissioning",
    "Operational Credentials",
    "Group Key Management",
    "Groupcast",
    "Identify",
    "Groups",
]

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _load_zap() -> dict:
    """Load and parse the ZAP JSON file."""
    with open(ZAP_FILE, "r") as f:
        return json.load(f)


def _get_server_clusters(zap_data: dict) -> list[dict]:
    """Return all server-side cluster entries across all endpoint types."""
    clusters = []
    for endpoint_type in zap_data.get("endpointTypes", []):
        for cluster in endpoint_type.get("clusters", []):
            if cluster.get("side") == "server":
                clusters.append(cluster)
    return clusters


# ---------------------------------------------------------------------------
# Property 4: ZAP server-side disabled for removed clusters
# ---------------------------------------------------------------------------


@given(cluster_name=st.sampled_from(REMOVED_CLUSTERS_ZAP))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_zap_server_side_disabled_for_removed_clusters(cluster_name: str):
    """
    **Validates: Requirements 2.1**

    Property 4: ZAP server-side disabled for removed clusters

    For any cluster name in the set of 11 removed clusters, if the ZAP file
    contains a server-side entry for that cluster, it SHALL have "enabled": 0.
    Absence of a server-side entry also satisfies this property.
    """
    zap_data = _load_zap()
    server_clusters = _get_server_clusters(zap_data)

    matching = [c for c in server_clusters if c["name"] == cluster_name]

    # If no server-side entry exists, the cluster is effectively disabled
    if not matching:
        return

    for entry in matching:
        assert entry.get("enabled") == 0, (
            f"FAIL: Server-side cluster '{cluster_name}' has "
            f"\"enabled\": {entry.get('enabled')} but should be 0 (disabled). "
            f"Phase 2 requires this cluster to be disabled in the ZAP file."
        )


# ---------------------------------------------------------------------------
# Property 5: ZAP server-side enabled for commissioning clusters
# ---------------------------------------------------------------------------


@given(cluster_name=st.sampled_from(COMMISSIONING_CLUSTERS_ZAP))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_zap_server_side_enabled_for_commissioning_clusters(cluster_name: str):
    """
    **Validates: Requirements 2.2**

    Property 5: ZAP server-side enabled for commissioning clusters

    For any cluster name in the set of 12 commissioning-essential clusters,
    the ZAP file SHALL contain a server-side entry with "enabled": 1.
    """
    zap_data = _load_zap()
    server_clusters = _get_server_clusters(zap_data)

    matching = [c for c in server_clusters if c["name"] == cluster_name]

    assert len(matching) > 0, (
        f"FAIL: No server-side entry found for commissioning cluster "
        f"'{cluster_name}' in tv-casting-app.zap. "
        f"Commissioning clusters must have a server-side entry."
    )

    for entry in matching:
        assert entry.get("enabled") == 1, (
            f"FAIL: Server-side cluster '{cluster_name}' has "
            f"\"enabled\": {entry.get('enabled')} but should be 1 (enabled). "
            f"Commissioning clusters must remain enabled."
        )
