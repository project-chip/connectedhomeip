#!/usr/bin/env python3
# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Generation script for TV Casting App slim override files.

Parses full zap-generated source files and extracts only the sections
needed by the casting-relevant clusters, producing slim override files
that reduce binary size.

Usage:
    python3 generate_slim_overrides.py <cluster-config.yaml> [--output-dir <dir>] [--chip-root <dir>]
"""

import argparse
import logging
import os
import re
import sys

import yaml

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

CHIP_COPYRIGHT_HEADER = """\
/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */"""


CHIP_COPYRIGHT_HEADER_ALL_RIGHTS = """\
/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */"""

GNI_COPYRIGHT_HEADER = """\
# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License."""


# ---------------------------------------------------------------------------
# CLI and YAML parsing (Task 8.1)
# ---------------------------------------------------------------------------

def parse_args(argv=None):
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(
        description="Generate slim override files for the TV Casting App."
    )
    parser.add_argument(
        "config",
        help="Path to the cluster config YAML file (e.g. casting-cluster-config.yaml)",
    )
    parser.add_argument(
        "--output-dir",
        default=".",
        help="Directory to write generated files (default: current directory)",
    )
    parser.add_argument(
        "--chip-root",
        default=None,
        help="Path to the CHIP SDK root directory (for finding full generated source files)",
    )
    return parser.parse_args(argv)


def load_config(config_path):
    """Load and validate the cluster config YAML file.

    Returns a dict with keys: casting_clusters, tlv_decoder_clusters, cluster_servers.
    """
    if not os.path.isfile(config_path):
        logger.error("config file not found: %s", config_path)
        sys.exit(1)

    try:
        with open(config_path) as f:
            config = yaml.safe_load(f)
    except yaml.YAMLError:
        logger.exception("invalid YAML in %s", config_path)
        sys.exit(1)

    if not isinstance(config, dict):
        logger.error("config file must be a YAML mapping, got %s", type(config).__name__)
        sys.exit(1)

    casting_clusters = config.get("casting_clusters", [])
    tlv_decoder_clusters = config.get("tlv_decoder_clusters", [])
    cluster_servers = config.get("cluster_servers", [])

    if not isinstance(casting_clusters, list):
        logger.error("'casting_clusters' must be a list")
        sys.exit(1)
    if not isinstance(tlv_decoder_clusters, list):
        logger.error("'tlv_decoder_clusters' must be a list")
        sys.exit(1)
    if not isinstance(cluster_servers, list):
        logger.error("'cluster_servers' must be a list")
        sys.exit(1)

    return {
        "casting_clusters": casting_clusters,
        "tlv_decoder_clusters": tlv_decoder_clusters,
        "cluster_servers": cluster_servers,
    }


# ---------------------------------------------------------------------------
# Cluster-objects extraction (Task 8.2)
# ---------------------------------------------------------------------------

def _extract_cluster_name_from_include(line):
    """Extract the cluster name from an #include <clusters/ClusterName/...> line.

    Returns the cluster name string, or None if the line doesn't match.
    """
    m = re.match(r'#include\s+<clusters/([^/]+)/', line)
    if m:
        return m.group(1)
    return None


def _extract_command_function_case_blocks(full_text, func_name, cluster_names):
    """Extract case blocks for the given clusters from a Command* function.

    Parses the function body's switch statement and returns only the case
    blocks for clusters in cluster_names. Returns the full function text
    with only matching case blocks.
    """
    cluster_set = set(cluster_names)

    # Find the function in the text
    func_pattern = re.compile(
        r'^bool\s+' + re.escape(func_name) + r'\s*\(ClusterId\s+aCluster,\s*CommandId\s+aCommand\)',
        re.MULTILINE
    )
    match = func_pattern.search(full_text)
    if not match:
        return None

    # Find the opening brace of the function
    brace_pos = full_text.index('{', match.end())

    # Extract the full function body using brace balancing
    depth = 0
    func_end = brace_pos
    for i in range(brace_pos, len(full_text)):
        if full_text[i] == '{':
            depth += 1
        elif full_text[i] == '}':
            depth -= 1
            if depth == 0:
                func_end = i + 1
                break

    func_body = full_text[brace_pos:func_end]

    # Parse case blocks: case Clusters::ClusterName::Id: { ... }
    case_pattern = re.compile(r'case\s+Clusters::(\w+)::Id:\s*\{')
    case_blocks = []

    for case_match in case_pattern.finditer(func_body):
        cluster_name = case_match.group(1)
        # Find the start of the line containing the case statement
        line_start = func_body.rfind('\n', 0, case_match.start())
        block_start = line_start + 1 if line_start >= 0 else case_match.start()
        # Extract the full case block using brace balancing
        brace_start = func_body.index('{', case_match.end() - 1)
        depth = 0
        block_end = brace_start
        for i in range(brace_start, len(func_body)):
            if func_body[i] == '{':
                depth += 1
            elif func_body[i] == '}':
                depth -= 1
                if depth == 0:
                    block_end = i + 1
                    break

        if cluster_name in cluster_set:
            case_blocks.append(func_body[block_start:block_end])

    return case_blocks


def _reindent_case_block(block_text):
    """Re-indent a case block to have consistent 4-space indentation.

    The case keyword should be at 4-space indent (inside a switch inside a function).
    All other lines maintain their relative indentation to the case line.
    """
    block_lines = block_text.splitlines()
    if not block_lines:
        return ""

    # Find the indentation of the first line (the case line)
    first_line = block_lines[0]
    current_indent = len(first_line) - len(first_line.lstrip())

    # Target indent for case is 4 spaces
    target_indent = 4
    indent_diff = target_indent - current_indent

    result_lines = []
    for line in block_lines:
        if line.strip() == "":
            result_lines.append("")
        else:
            line_indent = len(line) - len(line.lstrip())
            new_indent = max(0, line_indent + indent_diff)
            result_lines.append(" " * new_indent + line.lstrip())

    return "\n".join(result_lines)


def generate_cluster_objects(full_source_path, casting_clusters, output_path):
    """Generate cluster-objects-override.cpp from the full cluster-objects.cpp.

    Extracts #include directives for casting clusters, plus shared/Structs.ipp,
    and the three Command* functions with only casting-relevant case blocks.
    """
    if not os.path.isfile(full_source_path):
        logger.error("full cluster-objects.cpp not found: %s", full_source_path)
        sys.exit(1)

    with open(full_source_path) as f:
        full_text = f.read()

    full_lines = full_text.splitlines()

    cluster_set = set(casting_clusters)
    matched_clusters = set()

    # Collect all #include <clusters/...> lines
    include_lines = []
    for line in full_lines:
        name = _extract_cluster_name_from_include(line)
        if name is not None:
            if name == "shared":
                # Always include shared/Structs.ipp — we'll add it at the end
                continue
            if name in cluster_set:
                include_lines.append(line)
                matched_clusters.add(name)

    # Warn about unmatched clusters
    for cluster in casting_clusters:
        if cluster not in matched_clusters:
            logger.warning("cluster '%s' not found in %s", cluster, full_source_path)

    # Extract Command* function case blocks
    timed_blocks = _extract_command_function_case_blocks(full_text, "CommandNeedsTimedInvoke", casting_clusters)
    scoped_blocks = _extract_command_function_case_blocks(full_text, "CommandIsFabricScoped", casting_clusters)
    large_blocks = _extract_command_function_case_blocks(full_text, "CommandHasLargePayload", casting_clusters)

    # Build output
    lines = []
    lines.append(CHIP_COPYRIGHT_HEADER)
    lines.append("")
    lines.append("#include <app-common/zap-generated/cluster-objects.h>")
    lines.append("")

    # Add includes grouped by cluster (sorted for determinism)
    for line in include_lines:
        lines.append(line)

    # Always add shared/Structs.ipp
    lines.append("")
    lines.append("#include <clusters/shared/Structs.ipp>")

    # Add Command* functions
    lines.append("")
    lines.append("namespace chip {")
    lines.append("namespace app {")
    lines.append("")

    # CommandNeedsTimedInvoke
    lines.append("bool CommandNeedsTimedInvoke(ClusterId aCluster, CommandId aCommand)")
    lines.append("{")
    lines.append("    switch (aCluster)")
    lines.append("    {")
    if timed_blocks:
        for block in timed_blocks:
            lines.append(_reindent_case_block(block))
    lines.append("    default:")
    lines.append("        break;")
    lines.append("    }")
    lines.append("    return false;")
    lines.append("}")
    lines.append("")

    # CommandIsFabricScoped
    lines.append("bool CommandIsFabricScoped(ClusterId aCluster, CommandId aCommand)")
    lines.append("{")
    lines.append("    switch (aCluster)")
    lines.append("    {")
    if scoped_blocks:
        for block in scoped_blocks:
            lines.append(_reindent_case_block(block))
    lines.append("    default:")
    lines.append("        break;")
    lines.append("    }")
    lines.append("    return false;")
    lines.append("}")
    lines.append("")

    # CommandHasLargePayload
    lines.append("bool CommandHasLargePayload(ClusterId aCluster, CommandId aCommand)")
    lines.append("{")
    if large_blocks:
        lines.append("    switch (aCluster)")
        lines.append("    {")
        for block in large_blocks:
            lines.append(_reindent_case_block(block))
        lines.append("    default:")
        lines.append("        break;")
        lines.append("    }")
    else:
        lines.append("    // None of the casting-relevant clusters have commands flagged as large payload.")
        lines.append("    (void) aCluster;")
        lines.append("    (void) aCommand;")
    lines.append("    return false;")
    lines.append("}")
    lines.append("")
    lines.append("} // namespace app")
    lines.append("} // namespace chip")
    lines.append("")

    with open(output_path, "w") as f:
        f.write("\n".join(lines))


# ---------------------------------------------------------------------------
# TLV decoder extraction (Task 8.3)
# ---------------------------------------------------------------------------

def _extract_switch_case_blocks(full_text, cluster_names):
    """Extract case blocks from the outer switch(mClusterId) for given clusters.

    Uses brace-balanced extraction to capture complete case blocks including
    all nested code. Returns a list of (cluster_name, block_text) tuples.
    """
    cluster_set = set(cluster_names)
    results = []

    # Pattern for outer case blocks: case app::Clusters::ClusterName::Id: {
    case_pattern = re.compile(r'(\s*case\s+app::Clusters::(\w+)::Id:\s*\{)')

    for match in case_pattern.finditer(full_text):
        cluster_name = match.group(2)
        if cluster_name not in cluster_set:
            continue

        # Find the opening brace of this case block
        block_start = match.start()
        brace_pos = full_text.index('{', match.end() - 1)

        # Brace-balanced extraction
        depth = 0
        block_end = brace_pos
        for i in range(brace_pos, len(full_text)):
            if full_text[i] == '{':
                depth += 1
            elif full_text[i] == '}':
                depth -= 1
                if depth == 0:
                    block_end = i + 1
                    break

        # Capture through the "break;" after the closing brace
        rest = full_text[block_end:block_end + 50]
        break_match = re.match(r'\s*\n\s*break;\s*\n', rest)
        if break_match:
            block_end += break_match.end()

        block_text = full_text[block_start:block_end]
        results.append((cluster_name, block_text))

    return results


def generate_tlv_decoder(full_source_path, cluster_names, output_path, decoder_type):
    """Generate a slim TLV decoder file (attribute or event).

    Args:
        full_source_path: Path to the full zap-generated decoder .cpp
        cluster_names: List of cluster names to include
        output_path: Path to write the slim decoder
        decoder_type: "attribute" or "event"
    """
    if not os.path.isfile(full_source_path):
        logger.error("full TLV decoder not found: %s", full_source_path)
        sys.exit(1)

    with open(full_source_path) as f:
        full_text = f.read()

    case_blocks = _extract_switch_case_blocks(full_text, cluster_names)
    matched_clusters = {name for name, _ in case_blocks}

    # Warn about unmatched clusters
    for cluster in cluster_names:
        if cluster not in matched_clusters:
            logger.warning("cluster '%s' not found in %s", cluster, full_source_path)

    if decoder_type == "attribute":
        error_macro = "CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB"
        func_sig = "jobject DecodeAttributeValue(const app::ConcreteAttributePath & aPath, TLV::TLVReader & aReader, CHIP_ERROR * aError)"
        path_field = "mClusterId"
        includes = [
            "#include <controller/java/CHIPAttributeTLVValueDecoder.h>",
            "",
            "#include <app-common/zap-generated/cluster-objects.h>",
            "#include <app-common/zap-generated/ids/Attributes.h>",
            "#include <app-common/zap-generated/ids/Clusters.h>",
            "#include <app/data-model/DecodableList.h>",
            "#include <app/data-model/Decode.h>",
            "#include <jni.h>",
            "#include <lib/support/JniReferences.h>",
            "#include <lib/support/JniTypeWrappers.h>",
            "#include <lib/support/TypeTraits.h>",
        ]
    else:
        error_macro = "CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB"
        func_sig = "jobject DecodeEventValue(const app::ConcreteEventPath & aPath, TLV::TLVReader & aReader, CHIP_ERROR * aError)"
        path_field = "mClusterId"
        includes = [
            "#include <controller/java/CHIPAttributeTLVValueDecoder.h>",
            "",
            "#include <app-common/zap-generated/cluster-objects.h>",
            "#include <app-common/zap-generated/ids/Clusters.h>",
            "#include <app-common/zap-generated/ids/Events.h>",
            "#include <app/data-model/DecodableList.h>",
            "#include <app/data-model/Decode.h>",
            "#include <jni.h>",
            "#include <lib/support/JniReferences.h>",
            "#include <lib/support/JniTypeWrappers.h>",
            "#include <lib/support/TypeTraits.h>",
        ]

    lines = []
    lines.append(CHIP_COPYRIGHT_HEADER_ALL_RIGHTS)
    lines.append("")
    for inc in includes:
        lines.append(inc)
    lines.append("")
    lines.append("namespace chip {")
    lines.append("")
    lines.append(func_sig)
    lines.append("{")
    lines.append("    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();")
    lines.append("    CHIP_ERROR err = CHIP_NO_ERROR;")
    lines.append("")
    lines.append("    switch (aPath." + path_field + ")")
    lines.append("    {")

    # Add case blocks
    for cluster_name, block_text in case_blocks:
        # The block_text already has proper indentation from the source
        for bline in block_text.splitlines():
            lines.append(bline)

    # Add default case
    lines.append("    default:")
    lines.append(f"        *aError = {error_macro};")
    lines.append("        break;")
    lines.append("    }")
    lines.append("    return nullptr;")
    lines.append("}")
    lines.append("")
    lines.append("} // namespace chip")
    lines.append("")

    with open(output_path, "w") as f:
        f.write("\n".join(lines))


# ---------------------------------------------------------------------------
# Accessors extraction (Task 8.4)
# ---------------------------------------------------------------------------

def _extract_cluster_namespace_blocks(full_text, cluster_names):
    """Extract namespace ClusterName { ... } blocks from inside namespace Clusters {.

    Uses brace-balanced extraction. Returns a list of (cluster_name, block_text) tuples.
    """
    cluster_set = set(cluster_names)
    results = []

    # Find the start of namespace Clusters {
    clusters_ns_match = re.search(r'namespace\s+Clusters\s*\{', full_text)
    if not clusters_ns_match:
        return results

    search_start = clusters_ns_match.end()

    # Find all top-level namespace blocks inside Clusters
    # Pattern: namespace ClusterName {
    ns_pattern = re.compile(r'^(namespace\s+(\w+)\s*\{)', re.MULTILINE)

    for match in ns_pattern.finditer(full_text, search_start):
        cluster_name = match.group(2)

        # Skip non-cluster namespaces (Attributes, etc. are nested)
        if cluster_name in ("Attributes", "chip", "app"):
            continue

        if cluster_name not in cluster_set:
            continue

        # Find the opening brace
        brace_pos = full_text.index('{', match.end() - 1)

        # Brace-balanced extraction for the namespace block
        depth = 0
        block_end = brace_pos
        for i in range(brace_pos, len(full_text)):
            if full_text[i] == '{':
                depth += 1
            elif full_text[i] == '}':
                depth -= 1
                if depth == 0:
                    block_end = i + 1
                    break

        # Capture the closing comment: } // namespace ClusterName
        rest = full_text[block_end:block_end + 100]
        comment_match = re.match(r'\s*//\s*namespace\s+\w+\s*\n?', rest)
        if comment_match:
            block_end += comment_match.end()

        block_text = full_text[match.start():block_end]
        results.append((cluster_name, block_text))

    return results


def generate_accessors(full_source_path, casting_clusters, output_path):
    """Generate Accessors-override.cpp from the full Accessors.cpp.

    Extracts namespace blocks for casting clusters.
    """
    if not os.path.isfile(full_source_path):
        logger.error("full Accessors.cpp not found: %s", full_source_path)
        sys.exit(1)

    with open(full_source_path) as f:
        full_text = f.read()

    namespace_blocks = _extract_cluster_namespace_blocks(full_text, casting_clusters)
    matched_clusters = {name for name, _ in namespace_blocks}

    # Warn about unmatched clusters
    for cluster in casting_clusters:
        if cluster not in matched_clusters:
            logger.warning("cluster '%s' not found in %s", cluster, full_source_path)

    lines = []
    lines.append(CHIP_COPYRIGHT_HEADER)
    lines.append("")
    lines.append("#include <app-common/zap-generated/attributes/Accessors.h>")
    lines.append("")
    lines.append("#include <app-common/zap-generated/attribute-type.h>")
    lines.append("#include <app-common/zap-generated/ids/Attributes.h>")
    lines.append("#include <app-common/zap-generated/ids/Clusters.h>")
    lines.append("#include <app/util/attribute-table.h>")
    lines.append("#include <app/util/ember-strings.h>")
    lines.append("#include <lib/core/CHIPEncoding.h>")
    lines.append("#include <lib/support/logging/CHIPLogging.h>")
    lines.append("#include <lib/support/odd-sized-integers.h>")
    lines.append("")
    lines.append("namespace chip {")
    lines.append("namespace app {")
    lines.append("namespace Clusters {")
    lines.append("")

    for cluster_name, block_text in namespace_blocks:
        lines.append(block_text)
        lines.append("")

    lines.append("} // namespace Clusters")
    lines.append("} // namespace app")
    lines.append("} // namespace chip")
    lines.append("")

    with open(output_path, "w") as f:
        f.write("\n".join(lines))


# ---------------------------------------------------------------------------
# Cluster-servers.gni generation (Task 8.5)
# ---------------------------------------------------------------------------

def generate_cluster_servers_gni(cluster_servers, output_path):
    """Generate cluster-servers-override.gni from the cluster_servers list."""
    lines = []
    lines.append(GNI_COPYRIGHT_HEADER)
    lines.append("")
    lines.append("cluster_server_override_dirs = [")
    for server in cluster_servers:
        lines.append(f'  "{server}",')
    lines.append("]")
    lines.append("")

    with open(output_path, "w") as f:
        f.write("\n".join(lines))


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main(argv=None):
    args = parse_args(argv)

    config = load_config(args.config)
    output_dir = args.output_dir
    chip_root = args.chip_root

    # If chip_root not specified, try to infer from the config file location
    if chip_root is None:
        # Assume config is at examples/tv-casting-app/tv-casting-common/casting-cluster-config.yaml
        # So chip_root is 4 levels up
        config_dir = os.path.dirname(os.path.abspath(args.config))
        chip_root = os.path.normpath(os.path.join(config_dir, "..", "..", ".."))

    os.makedirs(output_dir, exist_ok=True)

    casting_clusters = config["casting_clusters"]
    tlv_decoder_clusters = config["tlv_decoder_clusters"]
    cluster_servers = config["cluster_servers"]

    # Full source paths
    cluster_objects_path = os.path.join(
        chip_root, "zzz_generated", "app-common", "app-common",
        "zap-generated", "cluster-objects.cpp"
    )
    attr_decoder_path = os.path.join(
        chip_root, "src", "controller", "java", "zap-generated",
        "CHIPAttributeTLVValueDecoder.cpp"
    )
    event_decoder_path = os.path.join(
        chip_root, "src", "controller", "java", "zap-generated",
        "CHIPEventTLVValueDecoder.cpp"
    )
    accessors_path = os.path.join(
        chip_root, "zzz_generated", "app-common", "app-common",
        "zap-generated", "attributes", "Accessors.cpp"
    )

    # Generate each file
    logger.info("Generating cluster-objects-override.cpp ...")
    generate_cluster_objects(
        cluster_objects_path,
        casting_clusters,
        os.path.join(output_dir, "cluster-objects-override.cpp"),
    )

    logger.info("Generating CHIPAttributeTLVValueDecoder-override.cpp ...")
    generate_tlv_decoder(
        attr_decoder_path,
        tlv_decoder_clusters,
        os.path.join(output_dir, "CHIPAttributeTLVValueDecoder-override.cpp"),
        "attribute",
    )

    logger.info("Generating CHIPEventTLVValueDecoder-override.cpp ...")
    generate_tlv_decoder(
        event_decoder_path,
        tlv_decoder_clusters,
        os.path.join(output_dir, "CHIPEventTLVValueDecoder-override.cpp"),
        "event",
    )

    logger.info("Generating Accessors-override.cpp ...")
    generate_accessors(
        accessors_path,
        casting_clusters,
        os.path.join(output_dir, "Accessors-override.cpp"),
    )

    logger.info("Generating cluster-servers-override.gni ...")
    generate_cluster_servers_gni(
        cluster_servers,
        os.path.join(output_dir, "cluster-servers-override.gni"),
    )

    logger.info("Done.")


if __name__ == "__main__":
    main()
