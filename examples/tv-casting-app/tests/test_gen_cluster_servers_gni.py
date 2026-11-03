"""
Property Test — Property 9: Generation script cluster-servers.gni output

Feature: consolidate-overrides-and-generation-script

**Validates: Requirements 12.1, 12.2, 12.3**

For any list of server directory names, the generation script SHALL
produce a cluster-servers-override.gni file containing a GN list variable
cluster_server_override_dirs with exactly those directory names, formatted
with the standard CHIP copyright header.
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

# Import the generation script
sys.path.insert(0, os.path.join(REPO_ROOT, "examples", "tv-casting-app", "tv-casting-common"))
from generate_slim_overrides import generate_cluster_servers_gni  # noqa: E402

# ---------------------------------------------------------------------------
# Strategies
# ---------------------------------------------------------------------------

# Generate realistic server directory names
KNOWN_SERVERS = [
    "access-control-server",
    "administrator-commissioning-server",
    "basic-information",
    "bindings",
    "descriptor",
    "general-commissioning-server",
    "general-diagnostics-server",
    "group-key-mgmt-server",
    "groups-server",
    "identify-server",
    "network-commissioning",
    "operational-credentials-server",
    "content-app-observer",
    "door-lock-server",
    "fan-control-server",
    "level-control",
    "on-off-server",
    "color-control-server",
    "thermostat",
    "window-covering-server",
]

server_lists = st.lists(
    st.sampled_from(KNOWN_SERVERS),
    min_size=1,
    max_size=len(KNOWN_SERVERS),
    unique=True,
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _extract_gni_list_entries(text):
    """Extract directory names from the cluster_server_override_dirs GN list."""
    # Find the list variable
    m = re.search(r'cluster_server_override_dirs\s*=\s*\[', text)
    if not m:
        return None

    # Find the closing bracket
    bracket_start = text.index('[', m.start())
    bracket_end = text.index(']', bracket_start)
    list_body = text[bracket_start + 1:bracket_end]

    # Extract quoted strings
    entries = re.findall(r'"([^"]+)"', list_body)
    return entries


# ---------------------------------------------------------------------------
# Property tests
# ---------------------------------------------------------------------------


@given(servers=server_lists)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_gni_contains_exactly_requested_servers(servers):
    """
    **Validates: Requirements 12.1, 12.2, 12.3**

    The generated .gni file SHALL contain a cluster_server_override_dirs
    variable with exactly the requested directory names.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "cluster-servers-override.gni")
        generate_cluster_servers_gni(servers, output_path)

        with open(output_path, "r") as f:
            content = f.read()

        entries = _extract_gni_list_entries(content)
        assert entries is not None, (
            "Output must contain cluster_server_override_dirs = [...]"
        )

        assert entries == servers, (
            f"Expected servers {servers}, got {entries}"
        )


@given(servers=server_lists)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_gni_has_copyright_header(servers):
    """
    **Validates: Requirements 12.3**

    The generated .gni file SHALL start with the standard CHIP copyright
    header.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "cluster-servers-override.gni")
        generate_cluster_servers_gni(servers, output_path)

        with open(output_path, "r") as f:
            content = f.read()

        assert content.startswith("# Copyright"), (
            "GNI output must start with the copyright header"
        )
        assert "Apache License" in content


@given(servers=server_lists)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_gni_one_entry_per_line(servers):
    """
    **Validates: Requirements 12.3**

    The generated .gni file SHALL have one directory entry per line.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "cluster-servers-override.gni")
        generate_cluster_servers_gni(servers, output_path)

        with open(output_path, "r") as f:
            content = f.read()

        # Each server should appear on its own line as "  \"server-name\","
        for server in servers:
            pattern = rf'^\s*"{re.escape(server)}",'
            assert re.search(pattern, content, re.MULTILINE), (
                f"Server '{server}' should appear on its own line"
            )
