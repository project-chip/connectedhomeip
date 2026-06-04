"""
Property Test — Property 7: Generation script idempotency

Feature: consolidate-overrides-and-generation-script

**Validates: Requirements 11.4**

For any valid cluster configuration and the same full generated source
files, running the generation script twice SHALL produce byte-identical
output files.
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
FULL_ATTR_DECODER = os.path.join(
    REPO_ROOT, "src", "controller", "java", "zap-generated",
    "CHIPAttributeTLVValueDecoder.cpp",
)
FULL_EVENT_DECODER = os.path.join(
    REPO_ROOT, "src", "controller", "java", "zap-generated",
    "CHIPEventTLVValueDecoder.cpp",
)
FULL_ACCESSORS = os.path.join(
    REPO_ROOT, "zzz_generated", "app-common", "app-common",
    "zap-generated", "attributes", "Accessors.cpp",
)

# Import the generation script
sys.path.insert(0, os.path.join(REPO_ROOT, "examples", "tv-casting-app", "tv-casting-common"))
from generate_slim_overrides import (generate_accessors, generate_cluster_objects, generate_cluster_servers_gni,  # noqa: E402
                                     generate_tlv_decoder)

# ---------------------------------------------------------------------------
# Discover cluster names from full sources
# ---------------------------------------------------------------------------


def _discover_cluster_names(source_path):
    names = set()
    with open(source_path, "r") as f:
        for line in f:
            m = re.match(r'#include\s+<clusters/([^/]+)/', line)
            if m and m.group(1) != "shared":
                names.add(m.group(1))
    return sorted(names)


def _discover_tlv_cluster_names(source_path):
    names = set()
    with open(source_path, "r") as f:
        for line in f:
            m = re.search(r'case\s+app::Clusters::(\w+)::Id:', line)
            if m:
                names.add(m.group(1))
    return sorted(names)


ALL_OBJ_CLUSTERS = _discover_cluster_names(FULL_CLUSTER_OBJECTS)
ALL_ATTR_CLUSTERS = _discover_tlv_cluster_names(FULL_ATTR_DECODER)
ALL_EVENT_CLUSTERS = _discover_tlv_cluster_names(FULL_EVENT_DECODER)

SAMPLE_SERVERS = [
    "access-control-server", "basic-information", "bindings",
    "descriptor", "groups-server", "identify-server",
]

# ---------------------------------------------------------------------------
# Strategies
# ---------------------------------------------------------------------------

obj_cluster_subsets = st.lists(
    st.sampled_from(ALL_OBJ_CLUSTERS),
    min_size=1,
    max_size=min(len(ALL_OBJ_CLUSTERS), 20),
    unique=True,
)

attr_cluster_subsets = st.lists(
    st.sampled_from(ALL_ATTR_CLUSTERS),
    min_size=1,
    max_size=min(len(ALL_ATTR_CLUSTERS), 20),
    unique=True,
)

event_cluster_subsets = st.lists(
    st.sampled_from(ALL_EVENT_CLUSTERS),
    min_size=1,
    max_size=min(len(ALL_EVENT_CLUSTERS), 20),
    unique=True,
)

server_subsets = st.lists(
    st.sampled_from(SAMPLE_SERVERS),
    min_size=1,
    max_size=len(SAMPLE_SERVERS),
    unique=True,
)

# ---------------------------------------------------------------------------
# Property tests
# ---------------------------------------------------------------------------


@given(subset=obj_cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_cluster_objects_idempotent(subset):
    """
    **Validates: Requirements 11.4**

    Running generate_cluster_objects twice with the same inputs
    SHALL produce byte-identical output.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        path1 = os.path.join(tmpdir, "run1.cpp")
        path2 = os.path.join(tmpdir, "run2.cpp")

        generate_cluster_objects(FULL_CLUSTER_OBJECTS, subset, path1)
        generate_cluster_objects(FULL_CLUSTER_OBJECTS, subset, path2)

        with open(path1, "rb") as f1, open(path2, "rb") as f2:
            assert f1.read() == f2.read(), (
                "cluster-objects output is not idempotent"
            )


@given(subset=attr_cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_attr_decoder_idempotent(subset):
    """
    **Validates: Requirements 11.4**

    Running generate_tlv_decoder (attribute) twice with the same inputs
    SHALL produce byte-identical output.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        path1 = os.path.join(tmpdir, "run1.cpp")
        path2 = os.path.join(tmpdir, "run2.cpp")

        generate_tlv_decoder(FULL_ATTR_DECODER, subset, path1, "attribute")
        generate_tlv_decoder(FULL_ATTR_DECODER, subset, path2, "attribute")

        with open(path1, "rb") as f1, open(path2, "rb") as f2:
            assert f1.read() == f2.read(), (
                "attribute TLV decoder output is not idempotent"
            )


@given(subset=event_cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_event_decoder_idempotent(subset):
    """
    **Validates: Requirements 11.4**

    Running generate_tlv_decoder (event) twice with the same inputs
    SHALL produce byte-identical output.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        path1 = os.path.join(tmpdir, "run1.cpp")
        path2 = os.path.join(tmpdir, "run2.cpp")

        generate_tlv_decoder(FULL_EVENT_DECODER, subset, path1, "event")
        generate_tlv_decoder(FULL_EVENT_DECODER, subset, path2, "event")

        with open(path1, "rb") as f1, open(path2, "rb") as f2:
            assert f1.read() == f2.read(), (
                "event TLV decoder output is not idempotent"
            )


@given(subset=obj_cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_accessors_idempotent(subset):
    """
    **Validates: Requirements 11.4**

    Running generate_accessors twice with the same inputs
    SHALL produce byte-identical output.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        path1 = os.path.join(tmpdir, "run1.cpp")
        path2 = os.path.join(tmpdir, "run2.cpp")

        generate_accessors(FULL_ACCESSORS, subset, path1)
        generate_accessors(FULL_ACCESSORS, subset, path2)

        with open(path1, "rb") as f1, open(path2, "rb") as f2:
            assert f1.read() == f2.read(), (
                "Accessors output is not idempotent"
            )


@given(servers=server_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_cluster_servers_gni_idempotent(servers):
    """
    **Validates: Requirements 11.4**

    Running generate_cluster_servers_gni twice with the same inputs
    SHALL produce byte-identical output.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        path1 = os.path.join(tmpdir, "run1.gni")
        path2 = os.path.join(tmpdir, "run2.gni")

        generate_cluster_servers_gni(servers, path1)
        generate_cluster_servers_gni(servers, path2)

        with open(path1, "rb") as f1, open(path2, "rb") as f2:
            assert f1.read() == f2.read(), (
                "cluster-servers.gni output is not idempotent"
            )
