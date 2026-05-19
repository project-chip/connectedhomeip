"""
Property Test — Property 8: Generation script output preamble

Feature: consolidate-overrides-and-generation-script

**Validates: Requirements 11.2, 11.3**

For any valid cluster configuration, each generated slim file SHALL
begin with the CHIP copyright header and SHALL contain the required
#include directives for that file type.
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
# Discover cluster names
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
    max_size=min(len(ALL_OBJ_CLUSTERS), 15),
    unique=True,
)

attr_cluster_subsets = st.lists(
    st.sampled_from(ALL_ATTR_CLUSTERS),
    min_size=1,
    max_size=min(len(ALL_ATTR_CLUSTERS), 15),
    unique=True,
)

event_cluster_subsets = st.lists(
    st.sampled_from(ALL_EVENT_CLUSTERS),
    min_size=1,
    max_size=min(len(ALL_EVENT_CLUSTERS), 15),
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
def test_cluster_objects_has_copyright_and_includes(subset):
    """
    **Validates: Requirements 11.2, 11.3**

    The generated cluster-objects file SHALL start with the CHIP copyright
    header and contain the required #include directive.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "cluster-objects-override.cpp")
        generate_cluster_objects(FULL_CLUSTER_OBJECTS, subset, output_path)

        with open(output_path, "r") as f:
            content = f.read()

        # Must start with copyright header
        assert content.startswith("/*"), (
            "Output must start with the CHIP copyright header (/*)"
        )
        assert "Copyright (c)" in content, (
            "Output must contain copyright notice"
        )
        assert "Apache License" in content, (
            "Output must contain Apache License reference"
        )

        # Must contain the required include
        assert "#include <app-common/zap-generated/cluster-objects.h>" in content, (
            "Output must include cluster-objects.h"
        )


@given(subset=attr_cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_attr_decoder_has_copyright_and_includes(subset):
    """
    **Validates: Requirements 11.2, 11.3**

    The generated attribute TLV decoder SHALL start with the CHIP copyright
    header and contain the required #include directives.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "CHIPAttributeTLVValueDecoder-override.cpp")
        generate_tlv_decoder(FULL_ATTR_DECODER, subset, output_path, "attribute")

        with open(output_path, "r") as f:
            content = f.read()

        assert content.startswith("/*"), (
            "Output must start with the CHIP copyright header"
        )
        assert "Copyright (c)" in content
        assert "Apache License" in content

        # Required includes for attribute decoder
        assert "#include <controller/java/CHIPAttributeTLVValueDecoder.h>" in content
        assert "#include <app-common/zap-generated/cluster-objects.h>" in content
        assert "#include <app-common/zap-generated/ids/Attributes.h>" in content
        assert "#include <app-common/zap-generated/ids/Clusters.h>" in content
        assert "#include <jni.h>" in content


@given(subset=event_cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_event_decoder_has_copyright_and_includes(subset):
    """
    **Validates: Requirements 11.2, 11.3**

    The generated event TLV decoder SHALL start with the CHIP copyright
    header and contain the required #include directives.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "CHIPEventTLVValueDecoder-override.cpp")
        generate_tlv_decoder(FULL_EVENT_DECODER, subset, output_path, "event")

        with open(output_path, "r") as f:
            content = f.read()

        assert content.startswith("/*"), (
            "Output must start with the CHIP copyright header"
        )
        assert "Copyright (c)" in content
        assert "Apache License" in content

        # Required includes for event decoder
        assert "#include <controller/java/CHIPAttributeTLVValueDecoder.h>" in content
        assert "#include <app-common/zap-generated/cluster-objects.h>" in content
        assert "#include <app-common/zap-generated/ids/Clusters.h>" in content
        assert "#include <app-common/zap-generated/ids/Events.h>" in content
        assert "#include <jni.h>" in content


@given(subset=obj_cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_accessors_has_copyright_and_includes(subset):
    """
    **Validates: Requirements 11.2, 11.3**

    The generated Accessors file SHALL start with the CHIP copyright
    header and contain the required #include directives.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "Accessors-override.cpp")
        generate_accessors(FULL_ACCESSORS, subset, output_path)

        with open(output_path, "r") as f:
            content = f.read()

        assert content.startswith("/*"), (
            "Output must start with the CHIP copyright header"
        )
        assert "Copyright (c)" in content
        assert "Apache License" in content

        # Required includes for Accessors
        assert "#include <app-common/zap-generated/attributes/Accessors.h>" in content
        assert "#include <app-common/zap-generated/attribute-type.h>" in content
        assert "#include <app-common/zap-generated/ids/Attributes.h>" in content
        assert "#include <app-common/zap-generated/ids/Clusters.h>" in content


@given(servers=server_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_cluster_servers_gni_has_copyright(servers):
    """
    **Validates: Requirements 11.2**

    The generated cluster-servers.gni SHALL start with the CHIP copyright
    header (GNI format with # comments).
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
