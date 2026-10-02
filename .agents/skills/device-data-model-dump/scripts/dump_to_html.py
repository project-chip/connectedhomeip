#!/usr/bin/env python3
#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
"""Convert a MatterTlvJson (.json) device dump into an interactive HTML viewer."""

import argparse
import base64
import json
import pathlib
import sys
from enum import Enum
from typing import Any

import matter.tlv
from matter.clusters.ClusterObjects import ALL_ATTRIBUTES, ALL_CLUSTERS, ClusterObject
from matter.clusters.Types import Nullable, NullValue
from matter.testing.spec_parsing import (PrebuiltDataModelDirectory, build_xml_clusters, build_xml_device_types,
                                         build_xml_namespaces, dm_from_spec_version)

GLOBAL_ATTRIBUTE_IDS = {
    0xFFF8: "GeneratedCommandList",
    0xFFF9: "AcceptedCommandList",
    0xFFFA: "EventList",
    0xFFFB: "AttributeList",
    0xFFFC: "FeatureMap",
    0xFFFD: "ClusterRevision",
}


def json_tlv_to_python_tlv(key_with_type: str, value: Any) -> tuple[int | str, Any]:
    """Convert a MatterTlvJson key/value pair back into raw Python TLV types."""
    parts = key_with_type.split(":", 1)
    raw_key = parts[0]
    key: int | str = int(raw_key) if raw_key.isdigit() else raw_key
    type_str = parts[1] if len(parts) > 1 else ""

    def convert_val(t: str, v: Any) -> Any:
        if t == "UINT":
            return matter.tlv.uint(v)
        if t == "INT":
            return int(v)
        if t == "BOOL":
            return bool(v)
        if t == "FLOAT":
            return matter.tlv.float32(v)
        if t == "DOUBLE":
            return float(v)
        if t == "STRING":
            return str(v)
        if t == "BYTES":
            return base64.b64decode(v)
        if t == "NULL":
            return None
        if t == "STRUCT" and isinstance(v, dict):
            return {
                sub_k: sub_v
                for k_str, val_item in v.items()
                for sub_k, sub_v in [json_tlv_to_python_tlv(k_str, val_item)]
            }
        if t.startswith("ARRAY"):
            sub_t = t.split("-", 1)[1] if "-" in t else ""
            if not isinstance(v, list):
                return []
            return [convert_val(sub_t, item) for item in v]
        return v

    return key, convert_val(type_str, value)


def format_decoded_value(val: Any) -> Any:
    """Convert decoded ClusterObject / Enum / bytes values into JSON-serializable structures."""
    if val is NullValue or isinstance(val, Nullable) and val is NullValue:
        return None
    if isinstance(val, Enum):
        return f"{val.name} ({val.value})"
    if isinstance(val, bytes):
        try:
            return "hex:" + val.hex()
        except Exception:
            return str(val)
    if isinstance(val, ClusterObject):
        result = {}
        for field in val.descriptor.Fields:
            field_val = getattr(val, field.Label, None)
            result[field.Label] = format_decoded_value(field_val)
        return result
    if isinstance(val, list):
        return [format_decoded_value(item) for item in val]
    if isinstance(val, dict):
        return {str(k): format_decoded_value(v) for k, v in val.items()}
    return val


def format_spec_version(spec_ver: int | None) -> str:
    if not spec_ver:
        return "Unknown"
    major = (spec_ver >> 24) & 0xFF
    minor = (spec_ver >> 16) & 0xFF
    dot = (spec_ver >> 8) & 0xFF
    return f"{major}.{minor}.{dot} (0x{spec_ver:08X})"


def parse_dump(json_data: dict[str, Any]) -> dict[str, Any]:
    """Parse MatterTlvJson into enriched endpoint tree, cluster, attribute, and command metadata."""
    spec_version_raw = None
    ep0 = json_data.get("0", {})
    basic_info_raw = ep0.get("40:STRUCT", {})
    for k, v in basic_info_raw.items():
        if k.startswith("21:"):
            spec_version_raw = v
            break

    dm_dir = PrebuiltDataModelDirectory.k1_5
    if spec_version_raw is not None:
        try:
            detected = dm_from_spec_version(matter.tlv.uint(spec_version_raw))
            if detected is not None:
                dm_dir = detected
        except Exception:
            pass

    xml_clusters, _ = build_xml_clusters(dm_dir)
    xml_device_types, _ = build_xml_device_types(dm_dir)
    xml_namespaces, _ = build_xml_namespaces(dm_dir)

    def get_cluster_name(cid: int) -> str:
        if cid in xml_clusters:
            return xml_clusters[cid].name
        if cid in ALL_CLUSTERS:
            return ALL_CLUSTERS[cid].__name__
        return f"Cluster 0x{cid:04X}"

    def get_device_type_name(dt_id: int) -> str:
        if dt_id in xml_device_types:
            return xml_device_types[dt_id].name
        return f"DeviceType 0x{dt_id:04X}"

    def resolve_semantic_tag(tag_entry: dict[str, Any]) -> dict[str, Any]:
        ns_id = tag_entry.get("1:UINT", 0)
        tag_id = tag_entry.get("2:UINT", 0)
        label_str = tag_entry.get("3:STRING")
        ns_name = f"Namespace 0x{ns_id:02X}"
        tag_name = f"Tag 0x{tag_id:02X}"
        if ns_id in xml_namespaces:
            ns_name = xml_namespaces[ns_id].name
            if tag_id in xml_namespaces[ns_id].tags:
                tag_name = xml_namespaces[ns_id].tags[tag_id].name
        display = f"{ns_name}: {tag_name}"
        if label_str:
            display += f" ({label_str})"
        return {
            "namespace_id": ns_id,
            "namespace_name": ns_name,
            "tag_id": tag_id,
            "tag_name": tag_name,
            "label": label_str,
            "display": display,
        }

    endpoints_out = []
    endpoints_by_id: dict[int, dict[str, Any]] = {}

    for ep_str in sorted(json_data.keys(), key=lambda x: int(x, 0)):
        ep_id = int(ep_str, 0)
        ep_clusters_raw = json_data[ep_str]

        clusters_out = []
        ep_device_types = []
        ep_server_list = []
        ep_client_list = []
        ep_parts_list = []
        ep_semantic_tags = []
        ep_label = ""

        # Extract NodeLabel from BasicInformation (40) or BridgedDeviceBasicInformation (57)
        for label_cid in (57, 40):
            c_raw = ep_clusters_raw.get(f"{label_cid}:STRUCT", {})
            if "5:STRING" in c_raw and c_raw["5:STRING"]:
                ep_label = str(c_raw["5:STRING"])
                break

        def cluster_sort_key(item: tuple[str, Any]) -> tuple[int, int]:
            cid = int(item[0].split(":", 1)[0])
            if cid == 29:
                return (0, cid)
            if cid in (40, 57):
                return (1, cid)
            return (2, cid)

        for cluster_key, cluster_attrs_raw in sorted(ep_clusters_raw.items(), key=cluster_sort_key):
            cid = int(cluster_key.split(":", 1)[0])
            cname = get_cluster_name(cid)
            xml_cluster = xml_clusters.get(cid)

            attributes_out = []
            feature_map_val = 0
            cluster_rev_val = None
            accepted_cmds_raw: list[int] = []
            generated_cmds_raw: list[int] = []

            def attr_sort_key(item: tuple[str, Any]) -> tuple[int, int]:
                aid = int(item[0].split(":", 1)[0])
                return (1 if aid in GLOBAL_ATTRIBUTE_IDS else 0, aid)

            for attr_key, attr_val_raw in sorted(cluster_attrs_raw.items(), key=attr_sort_key):
                aid, py_tlv_val = json_tlv_to_python_tlv(attr_key, attr_val_raw)
                assert isinstance(aid, int)
                raw_type = attr_key.split(":", 1)[1] if ":" in attr_key else ""

                aname = GLOBAL_ATTRIBUTE_IDS.get(aid)
                atype = raw_type
                if xml_cluster and aid in xml_cluster.attributes:
                    aname = xml_cluster.attributes[aid].name
                    atype = xml_cluster.attributes[aid].datatype or raw_type
                elif cid in ALL_ATTRIBUTES and aid in ALL_ATTRIBUTES[cid]:
                    aname = ALL_ATTRIBUTES[cid][aid].__name__
                if not aname:
                    aname = f"Attribute 0x{aid:04X}"

                decoded_val = format_decoded_value(py_tlv_val)
                if cid in ALL_ATTRIBUTES and aid in ALL_ATTRIBUTES[cid]:
                    try:
                        attr_cls = ALL_ATTRIBUTES[cid][aid]
                        obj_val = attr_cls.FromTagDictOrRawValue(py_tlv_val)
                        decoded_val = format_decoded_value(obj_val)
                    except Exception:
                        pass

                if aid == 0xFFFC and isinstance(attr_val_raw, int):
                    feature_map_val = attr_val_raw
                elif aid == 0xFFFD and isinstance(attr_val_raw, int):
                    cluster_rev_val = attr_val_raw
                elif aid == 0xFFF9 and isinstance(attr_val_raw, list):
                    accepted_cmds_raw = [int(x) for x in attr_val_raw]
                elif aid == 0xFFF8 and isinstance(attr_val_raw, list):
                    generated_cmds_raw = [int(x) for x in attr_val_raw]

                if cid == 29:
                    if aid == 0 and isinstance(attr_val_raw, list):
                        for dt_entry in attr_val_raw:
                            if isinstance(dt_entry, dict):
                                dt_id = dt_entry.get("0:UINT", 0)
                                dt_rev = dt_entry.get("1:UINT", 0)
                                ep_device_types.append({
                                    "id": dt_id,
                                    "hex": f"0x{dt_id:04X}",
                                    "name": get_device_type_name(dt_id),
                                    "revision": dt_rev,
                                })
                    elif aid == 1 and isinstance(attr_val_raw, list):
                        ep_server_list = [
                            {"id": int(x), "hex": f"0x{int(x):04X}", "name": get_cluster_name(int(x))}
                            for x in attr_val_raw
                        ]
                    elif aid == 2 and isinstance(attr_val_raw, list):
                        ep_client_list = [
                            {"id": int(x), "hex": f"0x{int(x):04X}", "name": get_cluster_name(int(x))}
                            for x in attr_val_raw
                        ]
                    elif aid == 3 and isinstance(attr_val_raw, list):
                        ep_parts_list = sorted(int(x) for x in attr_val_raw)
                    elif aid == 4 and isinstance(attr_val_raw, list):
                        for tag_entry in attr_val_raw:
                            if isinstance(tag_entry, dict):
                                ep_semantic_tags.append(resolve_semantic_tag(tag_entry))

                attributes_out.append({
                    "id": aid,
                    "hex": f"0x{aid:04X}",
                    "name": aname,
                    "type": atype,
                    "is_global": aid in GLOBAL_ATTRIBUTE_IDS,
                    "value": decoded_val,
                })

            features_out = []
            if xml_cluster and feature_map_val:
                for mask, feat in sorted(xml_cluster.features.items()):
                    if feature_map_val & mask:
                        features_out.append({
                            "mask": f"0x{mask:04X}",
                            "code": feat.code,
                            "name": feat.name,
                        })

            accepted_cmds_out = []
            for cmd_id in accepted_cmds_raw:
                cmd_name = f"Command 0x{cmd_id:02X}"
                if xml_cluster and cmd_id in xml_cluster.accepted_commands:
                    cmd_name = xml_cluster.accepted_commands[cmd_id].name
                accepted_cmds_out.append({
                    "id": cmd_id,
                    "hex": f"0x{cmd_id:02X}",
                    "name": cmd_name,
                })

            generated_cmds_out = []
            for cmd_id in generated_cmds_raw:
                cmd_name = f"Command 0x{cmd_id:02X}"
                if xml_cluster and cmd_id in xml_cluster.generated_commands:
                    cmd_name = xml_cluster.generated_commands[cmd_id].name
                generated_cmds_out.append({
                    "id": cmd_id,
                    "hex": f"0x{cmd_id:02X}",
                    "name": cmd_name,
                })

            clusters_out.append({
                "id": cid,
                "hex": f"0x{cid:04X}",
                "name": cname,
                "revision": cluster_rev_val,
                "feature_map": feature_map_val,
                "feature_map_hex": f"0x{feature_map_val:04X}",
                "features": features_out,
                "accepted_commands": accepted_cmds_out,
                "generated_commands": generated_cmds_out,
                "attributes": attributes_out,
            })

        ep_obj = {
            "id": ep_id,
            "label": ep_label,
            "device_types": ep_device_types,
            "semantic_tags": ep_semantic_tags,
            "server_list": ep_server_list,
            "client_list": ep_client_list,
            "parts_list": ep_parts_list,
            "parent_id": None,
            "children": [],
            "breadcrumbs": [],
            "clusters": clusters_out,
        }
        endpoints_out.append(ep_obj)
        endpoints_by_id[ep_id] = ep_obj

    # Reconstruct the true parent-child Endpoint Composition Tree via transitive reduction of PartsList
    # Build reachability: u -> v if v in PartsList(u) (or u == 0 and v != 0)
    all_ep_ids = sorted(endpoints_by_id.keys())
    direct_edges: dict[int, set[int]] = {
        ep_id: set(ep["parts_list"]) & set(all_ep_ids) for ep_id, ep in endpoints_by_id.items()
    }
    if 0 in direct_edges:
        for ep_id in all_ep_ids:
            if ep_id != 0:
                direct_edges[0].add(ep_id)

    # Compute transitive closure of reachability (Avoiding cycles if any malformed dump)
    reachable: dict[int, set[int]] = {ep_id: set() for ep_id in all_ep_ids}
    for ep_id in all_ep_ids:
        visited: set[int] = set()
        stack = list(direct_edges[ep_id])
        while stack:
            curr = stack.pop()
            if curr != ep_id and curr not in visited:
                visited.add(curr)
                stack.extend(direct_edges.get(curr, set()) - visited)
        reachable[ep_id] = visited

    # For each endpoint v != 0, its direct parent is the ancestor p that cannot reach any other ancestor of v
    for ep_id in all_ep_ids:
        if ep_id == 0:
            continue
        ancestors = [u for u in all_ep_ids if ep_id in reachable[u]]
        if not ancestors:
            continue
        # Leaf ancestors (those that do not reach any other ancestor of ep_id)
        leaf_ancestors = [
            u for u in ancestors if not any(w != u and w in reachable[u] for w in ancestors)
        ]
        parent_id = min(leaf_ancestors, key=lambda u: (len(endpoints_by_id[u]["parts_list"]), -u))
        endpoints_by_id[ep_id]["parent_id"] = parent_id
        endpoints_by_id[parent_id]["children"].append(ep_id)

    for ep in endpoints_out:
        ep["children"].sort()

    # Compute breadcrumb chain from root to each endpoint
    for ep in endpoints_out:
        chain = []
        curr_id: int | None = ep["id"]
        seen: set[int] = set()
        while curr_id is not None and curr_id not in seen and curr_id in endpoints_by_id:
            seen.add(curr_id)
            curr_ep = endpoints_by_id[curr_id]
            dt_label = ", ".join(dt["name"] for dt in curr_ep["device_types"]) or f"EP {curr_id}"
            if curr_ep["label"]:
                dt_label += f' ("{curr_ep["label"]}")'
            elif curr_ep["semantic_tags"]:
                dt_label += f' [{", ".join(t["tag_name"] for t in curr_ep["semantic_tags"])}]'
            chain.append({"id": curr_id, "summary": dt_label})
            curr_id = curr_ep["parent_id"]
        chain.reverse()
        ep["breadcrumbs"] = chain

    # Extract device summary from Endpoint 0 BasicInformation (Cluster 40)
    device_info: dict[str, Any] = {}
    if 0 in endpoints_by_id:
        for cl in endpoints_by_id[0]["clusters"]:
            if cl["id"] == 40:
                for attr in cl["attributes"]:
                    device_info[attr["name"]] = attr["value"]
    device_info["SpecificationVersionFormatted"] = format_spec_version(spec_version_raw)

    return {
        "device_info": device_info,
        "endpoints": endpoints_out,
    }


HTML_TEMPLATE = """<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Matter Device Data Model Dump</title>
  <script src="https://www.gstatic.com/antigravity/web/dev/tailwindcss.min.js"></script>
  <style>
    :root {
      --background: #f8fafc;
      --foreground: #0f172a;
      --card: #ffffff;
      --secondary: #f1f5f9;
      --muted-foreground: #64748b;
      --border: #e2e8f0;
      --primary: #2563eb;
      --primary-foreground: #ffffff;
    }
    @media (prefers-color-scheme: dark) {
      :root {
        --background: #0f172a;
        --foreground: #f8fafc;
        --card: #1e293b;
        --secondary: #334155;
        --muted-foreground: #94a3b8;
        --border: #334155;
        --primary: #3b82f6;
        --primary-foreground: #ffffff;
      }
    }
  </style>
</head>
<body class="bg-[var(--background)] text-[var(--foreground)] antialiased p-6">
  <div id="app" class="max-w-7xl mx-auto space-y-6"></div>

  <script>
    const DUMP_DATA = __DUMP_JSON_PLACEHOLDER__;
    const EP_BY_ID = Object.fromEntries(DUMP_DATA.endpoints.map(ep => [ep.id, ep]));

    let selectedView = DUMP_DATA.endpoints.length > 2 ? 'tree' : (DUMP_DATA.endpoints[1]?.id ?? DUMP_DATA.endpoints[0]?.id ?? 0);
    let searchQuery = '';
    let showGlobalAttributes = false;
    const collapsedClusters = new Set();
    const collapsedTreeNodes = new Set();

    function escapeHtml(str) {
      return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
    }

    function renderValue(val) {
      if (val === null || val === undefined) {
        return `<span class="px-2 py-0.5 rounded bg-[var(--secondary)] text-[var(--muted-foreground)] font-mono text-xs italic">null</span>`;
      }
      if (typeof val === 'boolean') {
        const cls = val
          ? 'bg-emerald-500/15 text-emerald-500 border border-emerald-500/30'
          : 'bg-[var(--secondary)] text-[var(--muted-foreground)] border border-[var(--border)]';
        return `<span class="px-2 py-0.5 rounded font-mono text-xs font-medium ${cls}">${val}</span>`;
      }
      if (typeof val === 'number') {
        const hex = val >= 0 ? `0x${val.toString(16).toUpperCase()}` : '';
        return `<span class="font-mono text-xs">${val}${hex && val > 9 ? ` <span class="text-[var(--muted-foreground)]">(${hex})</span>` : ''}</span>`;
      }
      if (typeof val === 'string') {
        if (val === '') {
          return `<span class="text-[var(--muted-foreground)] font-mono text-xs italic">"" (empty)</span>`;
        }
        return `<span class="font-mono text-xs break-all">${escapeHtml(val)}</span>`;
      }
      if (Array.isArray(val)) {
        if (val.length === 0) {
          return `<span class="text-[var(--muted-foreground)] font-mono text-xs">[]</span>`;
        }
        const allPrimitive = val.every(item => typeof item !== 'object' || item === null);
        if (allPrimitive && val.length <= 16) {
          return `<div class="flex flex-wrap gap-1">${val.map(item =>
            `<span class="px-1.5 py-0.5 rounded bg-[var(--secondary)] border border-[var(--border)] font-mono text-xs">${escapeHtml(item)}</span>`
          ).join('')}</div>`;
        }
        return `<pre class="font-mono text-xs bg-[var(--secondary)] border border-[var(--border)] rounded-lg p-2.5 overflow-x-auto max-h-48">${escapeHtml(JSON.stringify(val, null, 2))}</pre>`;
      }
      if (typeof val === 'object') {
        return `<pre class="font-mono text-xs bg-[var(--secondary)] border border-[var(--border)] rounded-lg p-2.5 overflow-x-auto max-h-48">${escapeHtml(JSON.stringify(val, null, 2))}</pre>`;
      }
      return `<span class="font-mono text-xs">${escapeHtml(String(val))}</span>`;
    }

    function endpointMatchesSearch(ep, query) {
      if (!query) return true;
      const q = query.toLowerCase();
      if (`ep ${ep.id}`.includes(q) || `endpoint ${ep.id}`.includes(q) || String(ep.id) === q) return true;
      if (ep.label && ep.label.toLowerCase().includes(q)) return true;
      if (ep.device_types.some(dt => dt.name.toLowerCase().includes(q) || dt.hex.toLowerCase().includes(q))) return true;
      if (ep.semantic_tags.some(t => t.display.toLowerCase().includes(q))) return true;
      return ep.clusters.some(cl => matchesSearch(cl, query));
    }

    function subtreeMatchesSearch(epId, query) {
      if (!query) return true;
      const ep = EP_BY_ID[epId];
      if (!ep) return false;
      if (endpointMatchesSearch(ep, query)) return true;
      return ep.children.some(cid => subtreeMatchesSearch(cid, query));
    }

    function matchesSearch(cluster, query) {
      if (!query) return true;
      const q = query.toLowerCase();
      if (cluster.name.toLowerCase().includes(q) || cluster.hex.toLowerCase().includes(q) || String(cluster.id).includes(q)) {
        return true;
      }
      if (cluster.attributes.some(a => a.name.toLowerCase().includes(q) || a.hex.toLowerCase().includes(q) || String(a.value).toLowerCase().includes(q))) {
        return true;
      }
      if (cluster.accepted_commands.some(c => c.name.toLowerCase().includes(q) || c.hex.toLowerCase().includes(q))) {
        return true;
      }
      if (cluster.generated_commands.some(c => c.name.toLowerCase().includes(q) || c.hex.toLowerCase().includes(q))) {
        return true;
      }
      return false;
    }

    function selectEndpoint(viewOrEpId) {
      selectedView = viewOrEpId;
      render();
    }

    function toggleTreeNode(event, epId) {
      event.stopPropagation();
      if (collapsedTreeNodes.has(epId)) {
        collapsedTreeNodes.delete(epId);
      } else {
        collapsedTreeNodes.add(epId);
      }
      render();
    }

    function expandOrCollapseTree(collapse) {
      if (collapse) {
        for (const ep of DUMP_DATA.endpoints) {
          if (ep.children.length > 0 && ep.id !== 0 && ep.id !== 1) {
            collapsedTreeNodes.add(ep.id);
          }
        }
      } else {
        collapsedTreeNodes.clear();
      }
      render();
    }

    function toggleCluster(key) {
      if (collapsedClusters.has(key)) {
        collapsedClusters.delete(key);
      } else {
        collapsedClusters.add(key);
      }
      render();
    }

    function jumpToCluster(epId, clusterId) {
      selectedView = epId;
      const key = `${epId}-${clusterId}`;
      collapsedClusters.delete(key);
      render();
      const el = document.getElementById(`cluster-${key}`);
      if (el) {
        el.scrollIntoView({ behavior: 'smooth', block: 'start' });
      }
    }

    function expandOrCollapseAll(collapse) {
      for (const ep of DUMP_DATA.endpoints) {
        for (const cl of ep.clusters) {
          const key = `${ep.id}-${cl.id}`;
          if (collapse) {
            collapsedClusters.add(key);
          } else {
            collapsedClusters.delete(key);
          }
        }
      }
      render();
    }

    function renderHeader() {
      const info = DUMP_DATA.device_info || {};
      const vendorName = info.VendorName || 'Matter Device';
      const productName = info.ProductName || 'Data Model Dump';
      const vid = info.VendorID !== undefined ? `0x${Number(info.VendorID).toString(16).toUpperCase().padStart(4, '0')} (${info.VendorID})` : 'N/A';
      const pid = info.ProductID !== undefined ? `0x${Number(info.ProductID).toString(16).toUpperCase().padStart(4, '0')} (${info.ProductID})` : 'N/A';
      const swVer = info.SoftwareVersionString ? `${info.SoftwareVersionString} (${info.SoftwareVersion ?? ''})` : (info.SoftwareVersion ?? 'N/A');
      const dmRev = info.DataModelRevision ?? 'N/A';
      const specVer = info.SpecificationVersionFormatted || 'N/A';

      const totalEndpoints = DUMP_DATA.endpoints.length;
      const totalClusters = DUMP_DATA.endpoints.reduce((acc, ep) => acc + ep.clusters.length, 0);
      const totalAttrs = DUMP_DATA.endpoints.reduce((acc, ep) => acc + ep.clusters.reduce((cAcc, cl) => cAcc + cl.attributes.length, 0), 0);

      return `
        <div class="bg-[var(--card)] border border-[var(--border)] rounded-xl p-5 shadow-sm space-y-4">
          <div class="flex flex-col md:flex-row md:items-center md:justify-between gap-4">
            <div>
              <div class="flex items-center gap-2.5">
                <span class="px-2.5 py-1 rounded-md bg-[var(--primary)] text-[var(--primary-foreground)] font-semibold text-xs uppercase tracking-wider">Matter Data Model</span>
                <h1 class="text-xl font-bold text-[var(--foreground)]">${escapeHtml(vendorName)} — ${escapeHtml(productName)}</h1>
              </div>
              <p class="text-xs text-[var(--muted-foreground)] mt-1">
                Interactive runtime topology &amp; attribute dump across ${totalEndpoints} endpoints, ${totalClusters} cluster instances, and ${totalAttrs} attributes
              </p>
            </div>
            <div class="flex flex-wrap items-center gap-2">
              <button onclick="expandOrCollapseAll(false)" class="px-3 py-1.5 rounded-lg bg-[var(--secondary)] text-[var(--foreground)] border border-[var(--border)] text-xs font-medium hover:opacity-80 transition">
                Expand Clusters
              </button>
              <button onclick="expandOrCollapseAll(true)" class="px-3 py-1.5 rounded-lg bg-[var(--secondary)] text-[var(--foreground)] border border-[var(--border)] text-xs font-medium hover:opacity-80 transition">
                Collapse Clusters
              </button>
              <label class="flex items-center gap-2 px-3 py-1.5 rounded-lg bg-[var(--secondary)] border border-[var(--border)] text-xs font-medium cursor-pointer select-none">
                <input type="checkbox" ${showGlobalAttributes ? 'checked' : ''} onchange="showGlobalAttributes = this.checked; render();" class="rounded">
                <span>Show Global Attributes</span>
              </label>
            </div>
          </div>

          <div class="grid grid-cols-2 sm:grid-cols-3 lg:grid-cols-5 gap-3 pt-2 border-t border-[var(--border)]">
            <div class="bg-[var(--secondary)] rounded-lg p-2.5 border border-[var(--border)]">
              <div class="text-xs text-[var(--muted-foreground)]">Vendor ID</div>
              <div class="font-mono text-xs font-semibold mt-0.5">${escapeHtml(vid)}</div>
            </div>
            <div class="bg-[var(--secondary)] rounded-lg p-2.5 border border-[var(--border)]">
              <div class="text-xs text-[var(--muted-foreground)]">Product ID</div>
              <div class="font-mono text-xs font-semibold mt-0.5">${escapeHtml(pid)}</div>
            </div>
            <div class="bg-[var(--secondary)] rounded-lg p-2.5 border border-[var(--border)]">
              <div class="text-xs text-[var(--muted-foreground)]">Software Version</div>
              <div class="font-mono text-xs font-semibold mt-0.5">${escapeHtml(swVer)}</div>
            </div>
            <div class="bg-[var(--secondary)] rounded-lg p-2.5 border border-[var(--border)]">
              <div class="text-xs text-[var(--muted-foreground)]">Specification Version</div>
              <div class="font-mono text-xs font-semibold mt-0.5">${escapeHtml(specVer)}</div>
            </div>
            <div class="bg-[var(--secondary)] rounded-lg p-2.5 border border-[var(--border)]">
              <div class="text-xs text-[var(--muted-foreground)]">Data Model Revision</div>
              <div class="font-mono text-xs font-semibold mt-0.5">${escapeHtml(dmRev)}</div>
            </div>
          </div>

          <div class="relative">
            <input
              id="search-input"
              type="text"
              value="${escapeHtml(searchQuery)}"
              oninput="searchQuery = this.value; renderSidebarAndMain();"
              placeholder="Filter endpoints, device labels (e.g., Closure, Oven, Chime), clusters, attributes, or commands..."
              class="w-full px-3.5 py-2 rounded-lg bg-[var(--secondary)] border border-[var(--border)] text-sm text-[var(--foreground)] placeholder-[var(--muted-foreground)] focus:outline-none"
            />
          </div>
        </div>
      `;
    }

    function renderSidebarTreeNode(epId, depth) {
      const ep = EP_BY_ID[epId];
      if (!ep || !subtreeMatchesSearch(epId, searchQuery)) return '';

      const isSelected = selectedView === ep.id;
      const hasChildren = ep.children.length > 0;
      const isCollapsed = collapsedTreeNodes.has(ep.id) && !searchQuery;
      const dtNames = ep.device_types.map(dt => dt.name).join(', ') || 'No Device Type';
      const indentPx = depth * 14;

      return `
        <div class="space-y-1">
          <div
            onclick="selectEndpoint(${ep.id})"
            style="margin-left: ${indentPx}px"
            class="cursor-pointer p-2.5 rounded-xl border transition ${isSelected ? 'bg-[var(--primary)] text-[var(--primary-foreground)] border-[var(--primary)] shadow-sm' : 'bg-[var(--card)] text-[var(--foreground)] border-[var(--border)] hover:opacity-90'}"
          >
            <div class="flex items-center justify-between gap-1.5">
              <div class="flex items-center gap-1.5 min-w-0">
                ${hasChildren ? `
                  <button
                    onclick="toggleTreeNode(event, ${ep.id})"
                    class="w-5 h-5 flex items-center justify-center rounded hover:bg-black/10 font-mono text-xs shrink-0"
                    title="${isCollapsed ? 'Expand child endpoints' : 'Collapse child endpoints'}"
                  >
                    ${isCollapsed ? '▶' : '▼'}
                  </button>
                ` : `<span class="w-3 shrink-0 text-center font-mono text-xs opacity-50">•</span>`}
                <span class="font-mono font-bold text-xs shrink-0">EP ${ep.id}</span>
                <span class="text-xs font-semibold truncate">${escapeHtml(dtNames)}</span>
              </div>
              <span class="px-1.5 py-0.5 rounded text-xs font-mono shrink-0 ${isSelected ? 'bg-black/20 text-[var(--primary-foreground)]' : 'bg-[var(--secondary)] text-[var(--muted-foreground)]'}">
                ${ep.clusters.length}c
              </span>
            </div>

            ${ep.label ? `
              <div class="mt-1 pl-5 flex items-center gap-1.5">
                <span class="px-2 py-0.5 rounded text-xs font-medium truncate ${isSelected ? 'bg-black/20 text-[var(--primary-foreground)]' : 'bg-emerald-500/15 text-emerald-500 border border-emerald-500/30'}">
                  "${escapeHtml(ep.label)}"
                </span>
              </div>
            ` : ''}

            ${ep.semantic_tags.length > 0 ? `
              <div class="mt-1 pl-5 flex flex-wrap gap-1">
                ${ep.semantic_tags.map(t => `
                  <span class="px-1.5 py-0.5 rounded text-xs font-mono ${isSelected ? 'bg-black/20 text-[var(--primary-foreground)]' : 'bg-[var(--secondary)] text-[var(--muted-foreground)] border border-[var(--border)]'}">
                    🏷 ${escapeHtml(t.tag_name)}
                  </span>
                `).join('')}
              </div>
            ` : ''}
          </div>

          ${(hasChildren && !isCollapsed) ? `
            <div class="space-y-1">
              ${ep.children.map(childId => renderSidebarTreeNode(childId, depth + 1)).join('')}
            </div>
          ` : ''}
        </div>
      `;
    }

    function renderSidebar() {
      const treeSelected = selectedView === 'tree';
      const allSelected = selectedView === 'all';
      const rootEndpoints = DUMP_DATA.endpoints.filter(ep => ep.parent_id === null);

      return `
        <div class="space-y-2.5">
          <div class="flex items-center justify-between px-1">
            <span class="text-xs font-semibold uppercase tracking-wider text-[var(--muted-foreground)]">
              Endpoint Hierarchy (${DUMP_DATA.endpoints.length})
            </span>
            <div class="flex items-center gap-1">
              <button onclick="expandOrCollapseTree(false)" class="px-1.5 py-0.5 rounded bg-[var(--secondary)] border border-[var(--border)] text-xs text-[var(--muted-foreground)] hover:text-[var(--foreground)]" title="Expand all tree nodes">+</button>
              <button onclick="expandOrCollapseTree(true)" class="px-1.5 py-0.5 rounded bg-[var(--secondary)] border border-[var(--border)] text-xs text-[var(--muted-foreground)] hover:text-[var(--foreground)]" title="Collapse bridged nodes">−</button>
            </div>
          </div>

          <button
            onclick="selectEndpoint('tree')"
            class="w-full text-left p-3 rounded-xl border transition ${treeSelected ? 'bg-[var(--primary)] text-[var(--primary-foreground)] border-[var(--primary)] shadow-sm' : 'bg-[var(--card)] text-[var(--foreground)] border-[var(--border)] hover:opacity-90'}"
          >
            <div class="flex items-center justify-between">
              <span class="font-semibold text-sm">Topology Tree Overview</span>
              <span class="px-2 py-0.5 rounded text-xs font-mono ${treeSelected ? 'bg-black/20 text-[var(--primary-foreground)]' : 'bg-[var(--secondary)] text-[var(--muted-foreground)]'}">
                Tree
              </span>
            </div>
            <div class="text-xs mt-1 ${treeSelected ? 'opacity-90' : 'text-[var(--muted-foreground)]'}">
              Interactive hierarchy of all ${DUMP_DATA.endpoints.length} endpoints
            </div>
          </button>

          <button
            onclick="selectEndpoint('all')"
            class="w-full text-left p-2.5 rounded-xl border transition ${allSelected ? 'bg-[var(--primary)] text-[var(--primary-foreground)] border-[var(--primary)] shadow-sm' : 'bg-[var(--card)] text-[var(--foreground)] border-[var(--border)] hover:opacity-90'}"
          >
            <div class="flex items-center justify-between">
              <span class="font-semibold text-xs">All Endpoints (Full Cluster List)</span>
              <span class="px-2 py-0.5 rounded text-xs font-mono ${allSelected ? 'bg-black/20 text-[var(--primary-foreground)]' : 'bg-[var(--secondary)] text-[var(--muted-foreground)]'}">
                ${DUMP_DATA.endpoints.length} EPs
              </span>
            </div>
          </button>

          <div class="space-y-1.5 max-h-[75vh] overflow-y-auto pr-1">
            ${rootEndpoints.map(ep => renderSidebarTreeNode(ep.id, 0)).join('')}
          </div>
        </div>
      `;
    }

    function renderOverviewTreeNode(epId, depth) {
      const ep = EP_BY_ID[epId];
      if (!ep || !subtreeMatchesSearch(epId, searchQuery)) return '';

      const nonDescriptorClusters = ep.clusters.filter(c => c.id !== 29);
      return `
        <div class="relative ${depth > 0 ? 'ml-6 pl-4 border-l-2 border-[var(--border)]' : ''} space-y-2.5">
          <div
            onclick="selectEndpoint(${ep.id})"
            class="cursor-pointer bg-[var(--card)] border border-[var(--border)] rounded-xl p-3.5 shadow-sm hover:border-[var(--primary)] transition"
          >
            <div class="flex flex-wrap items-center justify-between gap-2">
              <div class="flex flex-wrap items-center gap-2">
                <span class="px-2.5 py-0.5 rounded-md bg-[var(--secondary)] border border-[var(--border)] font-mono font-bold text-xs">
                  EP ${ep.id}
                </span>
                ${ep.device_types.map(dt => `
                  <span class="font-bold text-sm text-[var(--foreground)]">
                    ${escapeHtml(dt.name)} <span class="font-mono text-xs font-normal text-[var(--muted-foreground)]">(${dt.hex}, Rev ${dt.revision})</span>
                  </span>
                `).join('<span class="text-[var(--muted-foreground)]">+</span>')}
                ${ep.label ? `
                  <span class="px-2.5 py-0.5 rounded-md bg-emerald-500/15 text-emerald-500 border border-emerald-500/30 text-xs font-semibold">
                    NodeLabel: "${escapeHtml(ep.label)}"
                  </span>
                ` : ''}
                ${ep.semantic_tags.map(t => `
                  <span class="px-2 py-0.5 rounded-md bg-[var(--secondary)] border border-[var(--border)] font-mono text-xs">
                    🏷 ${escapeHtml(t.display)}
                  </span>
                `).join('')}
              </div>

              <span class="text-xs text-[var(--muted-foreground)] font-mono">
                ${ep.clusters.length} clusters ${ep.children.length > 0 ? `• ${ep.children.length} direct children` : ''} →
              </span>
            </div>

            ${nonDescriptorClusters.length > 0 ? `
              <div class="flex flex-wrap gap-1.5 mt-2.5">
                ${nonDescriptorClusters.map(cl => `
                  <span
                    onclick="event.stopPropagation(); jumpToCluster(${ep.id}, ${cl.id});"
                    class="px-2 py-0.5 rounded bg-[var(--secondary)] border border-[var(--border)] text-xs text-[var(--muted-foreground)] hover:text-[var(--foreground)] font-medium"
                  >
                    ${escapeHtml(cl.name)} <span class="font-mono opacity-75">${cl.hex}</span>
                  </span>
                `).join('')}
              </div>
            ` : ''}
          </div>

          ${ep.children.length > 0 ? `
            <div class="space-y-2.5">
              ${ep.children.map(childId => renderOverviewTreeNode(childId, depth + 1)).join('')}
            </div>
          ` : ''}
        </div>
      `;
    }

    function renderTopologyTreeOverview() {
      const rootEndpoints = DUMP_DATA.endpoints.filter(ep => ep.parent_id === null);
      return `
        <div class="bg-[var(--card)] border border-[var(--border)] rounded-xl p-5 shadow-sm space-y-4">
          <div class="flex flex-wrap items-center justify-between gap-2 border-b border-[var(--border)] pb-3">
            <div>
              <h2 class="text-base font-bold text-[var(--foreground)]">Device Endpoint Composition Tree</h2>
              <p class="text-xs text-[var(--muted-foreground)]">
                Reconstructed parent-child hierarchy from Descriptor <code>PartsList</code>, <code>BridgedDeviceBasicInformation::NodeLabel</code>, and <code>TagList</code> semantic tags. Click any endpoint or cluster pill to inspect.
              </p>
            </div>
          </div>
          <div class="space-y-3">
            ${rootEndpoints.map(ep => renderOverviewTreeNode(ep.id, 0)).join('')}
          </div>
        </div>
      `;
    }

    function renderEndpointSection(ep) {
      const filteredClusters = ep.clusters.filter(cl => matchesSearch(cl, searchQuery));
      if (searchQuery && filteredClusters.length === 0 && !endpointMatchesSearch(ep, searchQuery)) {
        return '';
      }
      const clustersToShow = filteredClusters.length > 0 ? filteredClusters : ep.clusters;

      return `
        <div class="space-y-4">
          <div class="bg-[var(--card)] border border-[var(--border)] rounded-xl p-5 shadow-sm space-y-4">
            ${ep.breadcrumbs.length > 1 ? `
              <div class="flex flex-wrap items-center gap-1.5 text-xs pb-3 border-b border-[var(--border)]">
                <span class="text-[var(--muted-foreground)] font-medium mr-1">Hierarchy Path:</span>
                ${ep.breadcrumbs.map((b, idx) => {
                  const isLast = idx === ep.breadcrumbs.length - 1;
                  return `
                    <button
                      onclick="selectEndpoint(${b.id})"
                      class="px-2 py-0.5 rounded border font-mono text-xs transition ${isLast ? 'bg-[var(--primary)] text-[var(--primary-foreground)] border-[var(--primary)] font-bold' : 'bg-[var(--secondary)] text-[var(--foreground)] border-[var(--border)] hover:opacity-80'}"
                    >
                      EP ${b.id}: ${escapeHtml(b.summary)}
                    </button>
                    ${isLast ? '' : `<span class="text-[var(--muted-foreground)]">→</span>`}
                  `;
                }).join('')}
              </div>
            ` : ''}

            <div class="flex flex-wrap items-center justify-between gap-3">
              <div class="flex flex-wrap items-center gap-2.5">
                <span class="px-3 py-1 rounded-lg bg-[var(--secondary)] border border-[var(--border)] font-mono font-bold text-sm">
                  Endpoint ${ep.id}
                </span>
                ${ep.device_types.map(dt => `
                  <span class="px-2.5 py-1 rounded-lg bg-[var(--secondary)] border border-[var(--border)] text-xs font-semibold">
                    ${escapeHtml(dt.name)} <span class="font-mono text-[var(--muted-foreground)]">${dt.hex} (Rev ${dt.revision})</span>
                  </span>
                `).join('')}
                ${ep.label ? `
                  <span class="px-2.5 py-1 rounded-lg bg-emerald-500/15 text-emerald-500 border border-emerald-500/30 text-xs font-semibold">
                    NodeLabel: "${escapeHtml(ep.label)}"
                  </span>
                ` : ''}
                ${ep.semantic_tags.map(t => `
                  <span class="px-2.5 py-1 rounded-lg bg-[var(--secondary)] border border-[var(--border)] font-mono text-xs font-semibold">
                    🏷 ${escapeHtml(t.display)}
                  </span>
                `).join('')}
              </div>
            </div>

            ${ep.children.length > 0 ? `
              <div class="space-y-2 pt-2 border-t border-[var(--border)]">
                <div class="text-xs font-semibold text-[var(--muted-foreground)]">
                  Direct Child Endpoints (${ep.children.length}) ${ep.parts_list.length !== ep.children.length ? `<span class="font-normal">(Descriptor PartsList has ${ep.parts_list.length} total descendants)</span>` : ''}:
                </div>
                <div class="flex flex-wrap gap-1.5">
                  ${ep.children.map(childId => {
                    const childEp = EP_BY_ID[childId];
                    const childDt = childEp ? childEp.device_types.map(d => d.name).join(', ') : '';
                    const childLbl = childEp?.label ? ` "${childEp.label}"` : '';
                    const childTags = childEp?.semantic_tags?.length ? ` [${childEp.semantic_tags.map(t => t.tag_name).join(', ')}]` : '';
                    return `
                      <button
                        onclick="selectEndpoint(${childId})"
                        class="px-2.5 py-1 rounded-lg bg-[var(--secondary)] border border-[var(--border)] text-xs font-medium hover:opacity-80 transition flex items-center gap-1.5"
                      >
                        <span class="font-mono font-bold">EP ${childId}</span>
                        <span>${escapeHtml(childDt + childLbl + childTags)} →</span>
                      </button>
                    `;
                  }).join('')}
                </div>
              </div>
            ` : ''}

            <div class="space-y-2 pt-2 border-t border-[var(--border)]">
              <div class="text-xs font-semibold text-[var(--muted-foreground)]">
                Server Clusters (${ep.server_list.length}) — click to jump:
              </div>
              <div class="flex flex-wrap gap-1.5">
                ${ep.server_list.map(sc => `
                  <button
                    onclick="jumpToCluster(${ep.id}, ${sc.id})"
                    class="px-2.5 py-1 rounded-lg bg-[var(--secondary)] border border-[var(--border)] text-xs font-medium hover:opacity-80 transition flex items-center gap-1.5"
                  >
                    <span>${escapeHtml(sc.name)}</span>
                    <span class="font-mono text-[var(--muted-foreground)]">${sc.hex}</span>
                  </button>
                `).join('')}
              </div>
              ${ep.client_list.length > 0 ? `
                <div class="text-xs font-semibold text-[var(--muted-foreground)] pt-2">
                  Client Clusters (${ep.client_list.length}):
                </div>
                <div class="flex flex-wrap gap-1.5">
                  ${ep.client_list.map(cc => `
                    <span class="px-2.5 py-1 rounded-lg bg-[var(--secondary)] border border-[var(--border)] text-xs font-medium">
                      ${escapeHtml(cc.name)} <span class="font-mono text-[var(--muted-foreground)]">${cc.hex}</span>
                    </span>
                  `).join('')}
                </div>
              ` : ''}
            </div>
          </div>

          <div class="space-y-3">
            ${clustersToShow.map(cl => renderClusterCard(ep.id, cl)).join('')}
          </div>
        </div>
      `;
    }

    function renderClusterCard(epId, cl) {
      const key = `${epId}-${cl.id}`;
      const isCollapsed = collapsedClusters.has(key);
      const visibleAttrs = cl.attributes.filter(a => showGlobalAttributes || !a.is_global);

      return `
        <div id="cluster-${key}" class="bg-[var(--card)] border border-[var(--border)] rounded-xl shadow-sm overflow-hidden">
          <button
            onclick="toggleCluster('${key}')"
            class="w-full text-left px-5 py-3.5 bg-[var(--secondary)] border-b border-[var(--border)] flex flex-wrap items-center justify-between gap-3 hover:opacity-95 transition"
          >
            <div class="flex flex-wrap items-center gap-2.5">
              <span class="font-mono text-xs text-[var(--muted-foreground)]">${isCollapsed ? '▶' : '▼'}</span>
              <span class="font-bold text-base text-[var(--foreground)]">${escapeHtml(cl.name)}</span>
              <span class="px-2 py-0.5 rounded bg-[var(--card)] border border-[var(--border)] font-mono text-xs text-[var(--muted-foreground)]">
                ${cl.hex} (${cl.id})
              </span>
              ${cl.revision !== null ? `
                <span class="px-2 py-0.5 rounded bg-[var(--card)] border border-[var(--border)] text-xs font-medium">
                  Rev ${cl.revision}
                </span>
              ` : ''}
              ${cl.features.map(f => `
                <span class="px-2 py-0.5 rounded bg-emerald-500/15 text-emerald-500 border border-emerald-500/30 text-xs font-mono font-medium">
                  Feature: ${escapeHtml(f.code)} (${escapeHtml(f.name)})
                </span>
              `).join('')}
            </div>

            <div class="flex items-center gap-2 text-xs text-[var(--muted-foreground)] font-mono">
              <span>${visibleAttrs.length} attrs</span>
              <span>•</span>
              <span>${cl.accepted_commands.length} in / ${cl.generated_commands.length} out cmds</span>
            </div>
          </button>

          ${isCollapsed ? '' : `
            <div class="p-5 space-y-4">
              ${(cl.accepted_commands.length > 0 || cl.generated_commands.length > 0) ? `
                <div class="grid grid-cols-1 md:grid-cols-2 gap-3 pb-3 border-b border-[var(--border)]">
                  <div>
                    <div class="text-xs font-semibold text-[var(--muted-foreground)] mb-1.5">
                      Accepted Commands (Client → Server) [${cl.accepted_commands.length}]
                    </div>
                    ${cl.accepted_commands.length === 0 ? `<span class="text-xs text-[var(--muted-foreground)] italic">None</span>` : `
                      <div class="flex flex-wrap gap-1.5">
                        ${cl.accepted_commands.map(cmd => `
                          <span class="px-2 py-1 rounded-md bg-[var(--secondary)] border border-[var(--border)] text-xs font-medium">
                            <span class="font-mono text-[var(--muted-foreground)]">${cmd.hex}</span> ${escapeHtml(cmd.name)}
                          </span>
                        `).join('')}
                      </div>
                    `}
                  </div>
                  <div>
                    <div class="text-xs font-semibold text-[var(--muted-foreground)] mb-1.5">
                      Generated Commands (Server → Client) [${cl.generated_commands.length}]
                    </div>
                    ${cl.generated_commands.length === 0 ? `<span class="text-xs text-[var(--muted-foreground)] italic">None</span>` : `
                      <div class="flex flex-wrap gap-1.5">
                        ${cl.generated_commands.map(cmd => `
                          <span class="px-2 py-1 rounded-md bg-[var(--secondary)] border border-[var(--border)] text-xs font-medium">
                            <span class="font-mono text-[var(--muted-foreground)]">${cmd.hex}</span> ${escapeHtml(cmd.name)}
                          </span>
                        `).join('')}
                      </div>
                    `}
                  </div>
                </div>
              ` : ''}

              ${visibleAttrs.length === 0 ? `
                <div class="text-xs text-[var(--muted-foreground)] italic">
                  No cluster-specific attributes (enable "Show Global Attributes" above to view global attributes).
                </div>
              ` : `
                <div class="overflow-x-auto">
                  <table class="w-full text-left border-collapse">
                    <thead>
                      <tr class="border-b border-[var(--border)] text-xs text-[var(--muted-foreground)]">
                        <th class="py-2 pr-4 font-semibold w-28">ID</th>
                        <th class="py-2 pr-4 font-semibold w-56">Attribute Name</th>
                        <th class="py-2 pr-4 font-semibold w-36">Type</th>
                        <th class="py-2 font-semibold">Value</th>
                      </tr>
                    </thead>
                    <tbody class="divide-y divide-[var(--border)] text-xs">
                      ${visibleAttrs.map(attr => `
                        <tr class="hover:bg-[var(--secondary)]/40">
                          <td class="py-2.5 pr-4 font-mono text-[var(--muted-foreground)] align-top whitespace-nowrap">
                            ${attr.hex} <span class="opacity-70">(${attr.id})</span>
                          </td>
                          <td class="py-2.5 pr-4 font-medium text-[var(--foreground)] align-top">
                            ${escapeHtml(attr.name)}
                            ${attr.is_global ? `<span class="ml-1.5 px-1.5 py-0.5 rounded bg-[var(--secondary)] text-[var(--muted-foreground)] text-xs">Global</span>` : ''}
                          </td>
                          <td class="py-2.5 pr-4 font-mono text-[var(--muted-foreground)] align-top whitespace-nowrap">
                            ${escapeHtml(attr.type)}
                          </td>
                          <td class="py-2.5 align-top">
                            ${renderValue(attr.value)}
                          </td>
                        </tr>
                      `).join('')}
                    </tbody>
                  </table>
                </div>
              `}
            </div>
          `}
        </div>
      `;
    }

    function renderMainContent() {
      if (selectedView === 'tree') {
        return renderTopologyTreeOverview();
      }
      const epsToRender = selectedView === 'all'
        ? DUMP_DATA.endpoints
        : DUMP_DATA.endpoints.filter(ep => ep.id === selectedView);

      const content = epsToRender.map(ep => renderEndpointSection(ep)).join('');
      return content || `
        <div class="bg-[var(--card)] border border-[var(--border)] rounded-xl p-8 text-center text-sm text-[var(--muted-foreground)]">
          No endpoints, clusters, attributes, or commands matched "${escapeHtml(searchQuery)}".
        </div>
      `;
    }

    function renderSidebarAndMain() {
      const sidebarEl = document.getElementById('sidebar-content');
      const mainEl = document.getElementById('main-content');
      if (sidebarEl) sidebarEl.innerHTML = renderSidebar();
      if (mainEl) mainEl.innerHTML = renderMainContent();
    }

    function render() {
      const app = document.getElementById('app');
      app.innerHTML = `
        ${renderHeader()}
        <div class="grid grid-cols-1 lg:grid-cols-12 gap-6 items-start">
          <div id="sidebar-content" class="lg:col-span-4">
            ${renderSidebar()}
          </div>
          <div id="main-content" class="lg:col-span-8 space-y-6">
            ${renderMainContent()}
          </div>
        </div>
      `;
    }

    render();
  </script>
</body>
</html>
"""


def generate_html_from_dump(json_path: pathlib.Path, html_path: pathlib.Path) -> None:
    with open(json_path, "r", encoding="utf-8") as f:
        raw_dump = json.load(f)

    enriched = parse_dump(raw_dump)
    html_content = HTML_TEMPLATE.replace(
        "__DUMP_JSON_PLACEHOLDER__", json.dumps(enriched, separators=(",", ":"))
    )
    with open(html_path, "w", encoding="utf-8") as f:
        f.write(html_content)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Convert a MatterTlvJson (.json) device dump into an interactive HTML file."
    )
    parser.add_argument("input_json", type=pathlib.Path, help="Path to input .json dump file")
    parser.add_argument(
        "-o",
        "--output",
        type=pathlib.Path,
        default=None,
        help="Path to output .html file (defaults to input path with .html suffix)",
    )
    args = parser.parse_args()

    if not args.input_json.exists():
        print(f"Error: Input file not found: {args.input_json}", file=sys.stderr)
        return 1

    output_html = args.output or args.input_json.with_suffix(".html")
    generate_html_from_dump(args.input_json, output_html)
    print(f"Generated interactive HTML data model viewer: {output_html}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
