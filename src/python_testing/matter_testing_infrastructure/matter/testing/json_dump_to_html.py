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
"""Convert a MatterTlvJson (.json) device dump into a self-contained interactive HTML viewer.

Uses only the Python 3 standard library (no third-party packages or virtual environment
required) and produces a single self-contained HTML file with no external CSS/JS/CDN
dependencies.
"""

import argparse
import base64
import copy
import dataclasses
import json
import logging
import pathlib
import sys
import xml.etree.ElementTree as ET
import zipfile
from importlib.resources.abc import Traversable
from typing import Any

LOGGER = logging.getLogger(__name__)

GLOBAL_ATTRIBUTE_IDS: dict[int, tuple[str, str]] = {
    0xFFF8: ("GeneratedCommandList", "list[command-id]"),
    0xFFF9: ("AcceptedCommandList", "list[command-id]"),
    0xFFFA: ("EventList", "list[event-id]"),
    0xFFFB: ("AttributeList", "list[attrib-id]"),
    0xFFFC: ("FeatureMap", "bitmap32"),
    0xFFFD: ("ClusterRevision", "uint16"),
}

# Self-contained HTML/CSS/JS template. The `__DUMP_JSON_PLACEHOLDER__` token is
# replaced at generation time with the JSON-serialized output of `parse_dump()`.
HTML_TEMPLATE = """<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Matter Device Data Model Dump</title>
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
      --accent: #10b981;
      --accent-bg: rgba(16, 185, 129, 0.14);
      --accent-border: rgba(16, 185, 129, 0.35);
      --font-sans: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
      --font-mono: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace;
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
    * { box-sizing: border-box; }
    body {
      margin: 0;
      padding: 24px;
      background-color: var(--background);
      color: var(--foreground);
      font-family: var(--font-sans);
      line-height: 1.5;
      -webkit-font-smoothing: antialiased;
    }
    button, input {
      font-family: inherit;
      color: inherit;
    }
    .container {
      max-width: 1280px;
      margin: 0 auto;
      display: flex;
      flex-direction: column;
      gap: 20px;
    }
    .card {
      background-color: var(--card);
      border: 1px solid var(--border);
      border-radius: 12px;
      padding: 20px;
      box-shadow: 0 1px 2px rgba(0, 0, 0, 0.05);
    }
    .header-top {
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      justify-content: space-between;
      gap: 16px;
    }
    .title-row {
      display: flex;
      align-items: center;
      gap: 10px;
      flex-wrap: wrap;
    }
    .title-row h1 {
      margin: 0;
      font-size: 20px;
      font-weight: 700;
    }
    h1, h2 { margin: 0; }
    .subtitle {
      margin: 4px 0 0 0;
      font-size: 12px;
      color: var(--muted-foreground);
    }
    .badge-primary {
      background-color: var(--primary);
      color: var(--primary-foreground);
      padding: 4px 10px;
      border-radius: 6px;
      font-size: 11px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.05em;
    }
    .badge {
      display: inline-flex;
      align-items: center;
      gap: 4px;
      padding: 2px 8px;
      border-radius: 6px;
      font-size: 12px;
      background-color: var(--secondary);
      border: 1px solid var(--border);
      color: var(--foreground);
    }
    .badge-accent {
      display: inline-flex;
      align-items: center;
      padding: 2px 8px;
      border-radius: 6px;
      font-size: 12px;
      font-weight: 600;
      background-color: var(--accent-bg);
      color: var(--accent);
      border: 1px solid var(--accent-border);
    }
    .btn {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      padding: 6px 12px;
      border-radius: 8px;
      background-color: var(--secondary);
      color: var(--foreground);
      border: 1px solid var(--border);
      font-size: 12px;
      font-weight: 500;
      cursor: pointer;
      transition: opacity 0.15s ease;
    }
    .btn:hover { opacity: 0.85; }
    .btn-active {
      background-color: var(--primary);
      color: var(--primary-foreground);
      border-color: var(--primary);
      font-weight: 700;
    }
    .info-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
      gap: 10px;
      margin-top: 16px;
      padding-top: 14px;
      border-top: 1px solid var(--border);
    }
    .info-box {
      background-color: var(--secondary);
      border: 1px solid var(--border);
      border-radius: 8px;
      padding: 10px;
    }
    .info-label {
      font-size: 11px;
      color: var(--muted-foreground);
    }
    .info-val {
      font-family: var(--font-mono);
      font-size: 12px;
      font-weight: 600;
      margin-top: 2px;
      word-break: break-word;
    }
    .search-input {
      width: 100%;
      margin-top: 14px;
      padding: 9px 14px;
      border-radius: 8px;
      background-color: var(--secondary);
      border: 1px solid var(--border);
      color: var(--foreground);
      font-size: 13px;
      outline: none;
    }
    .search-input::placeholder { color: var(--muted-foreground); }
    .main-grid {
      display: grid;
      grid-template-columns: 380px 1fr;
      gap: 20px;
      align-items: start;
    }
    @media (max-width: 960px) {
      .main-grid { grid-template-columns: 1fr; }
    }
    .sidebar {
      display: flex;
      flex-direction: column;
      gap: 10px;
    }
    .sidebar-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 0 4px;
      font-size: 11px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.05em;
      color: var(--muted-foreground);
    }
    .tree-scroll {
      display: flex;
      flex-direction: column;
      gap: 5px;
      max-height: 75vh;
      overflow-y: auto;
      padding-right: 4px;
    }
    .nav-item {
      width: 100%;
      text-align: left;
      padding: 7px 12px;
      border-radius: 7px;
      background-color: var(--card);
      border: 1px solid var(--border);
      cursor: pointer;
      transition: border-color 0.15s ease, opacity 0.15s ease;
    }
    .nav-item:hover { border-color: var(--primary); }
    .nav-item.selected {
      background-color: var(--primary);
      color: var(--primary-foreground);
      border-color: var(--primary);
    }
    .nav-row {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 6px;
    }
    .nav-left {
      display: flex;
      align-items: center;
      gap: 6px;
      min-width: 0;
    }
    .truncate {
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
    }
    .mono { font-family: var(--font-mono); }
    .muted { color: var(--muted-foreground); }
    .selected .muted { color: var(--primary-foreground); opacity: 0.85; }
    .pill-count {
      padding: 2px 6px;
      border-radius: 4px;
      font-family: var(--font-mono);
      font-size: 11px;
      background-color: var(--secondary);
      color: var(--muted-foreground);
      flex-shrink: 0;
    }
    .selected .pill-count {
      background-color: rgba(0, 0, 0, 0.22);
      color: var(--primary-foreground);
    }
    .tree-toggle {
      width: 20px;
      height: 20px;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      border-radius: 4px;
      border: none;
      background: transparent;
      cursor: pointer;
      font-family: var(--font-mono);
      font-size: 11px;
      padding: 0;
      flex-shrink: 0;
    }
    .tree-toggle:hover { background-color: rgba(0, 0, 0, 0.12); }
    .overview-branch {
      margin-left: 22px;
      padding-left: 16px;
      border-left: 2px solid var(--border);
      display: flex;
      flex-direction: column;
      gap: 10px;
      margin-top: 10px;
    }
    .overview-node {
      background-color: var(--card);
      border: 1px solid var(--border);
      border-radius: 10px;
      padding: 14px;
      cursor: pointer;
      transition: border-color 0.15s ease;
    }
    .overview-node:hover { border-color: var(--primary); }
    .flex-wrap-gap {
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      gap: 6px;
    }
    .section-divider {
      margin-top: 12px;
      padding-top: 12px;
      border-top: 1px solid var(--border);
    }
    .cluster-card {
      background-color: var(--card);
      border: 1px solid var(--border);
      border-radius: 12px;
      overflow: hidden;
      margin-top: 12px;
    }
    .cluster-header {
      width: 100%;
      text-align: left;
      padding: 14px 20px;
      background-color: var(--secondary);
      border: none;
      border-bottom: 1px solid var(--border);
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      justify-content: space-between;
      gap: 10px;
      cursor: pointer;
    }
    .cluster-body {
      padding: 18px 20px;
      display: flex;
      flex-direction: column;
      gap: 16px;
    }
    .cmd-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
      gap: 12px;
      padding-bottom: 14px;
      border-bottom: 1px solid var(--border);
    }
    table.attr-table {
      width: 100%;
      border-collapse: collapse;
      text-align: left;
      font-size: 12px;
    }
    table.attr-table th {
      padding: 8px 12px 8px 0;
      border-bottom: 1px solid var(--border);
      color: var(--muted-foreground);
      font-weight: 600;
    }
    table.attr-table td {
      padding: 10px 12px 10px 0;
      border-bottom: 1px solid var(--border);
      vertical-align: top;
    }
    table.attr-table tr:last-child td { border-bottom: none; }
    pre.json-pre {
      margin: 0;
      padding: 10px;
      border-radius: 8px;
      background-color: var(--secondary);
      border: 1px solid var(--border);
      font-family: var(--font-mono);
      font-size: 12px;
      overflow-x: auto;
      max-height: 200px;
    }
  </style>
</head>
<body>
  <div id="app" class="container"></div>

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
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#39;');
    }

    function renderValue(val) {
      if (val === null || val === undefined) {
        return `<span class="badge mono muted" style="font-style:italic">null</span>`;
      }
      if (typeof val === 'boolean') {
        const cls = val ? 'badge-accent mono' : 'badge mono muted';
        return `<span class="${cls}">${val}</span>`;
      }
      if (typeof val === 'number') {
        const hex = val >= 0 ? `0x${val.toString(16).toUpperCase()}` : '';
        return `<span class="mono">${val}${hex && val > 9 ? ` <span class="muted">(${hex})</span>` : ''}</span>`;
      }
      if (typeof val === 'string') {
        if (val === '') {
          return `<span class="mono muted" style="font-style:italic">"" (empty)</span>`;
        }
        return `<span class="mono" style="word-break:break-all">${escapeHtml(val)}</span>`;
      }
      if (Array.isArray(val)) {
        if (val.length === 0) {
          return `<span class="mono muted">[]</span>`;
        }
        const allPrimitive = val.every(item => typeof item !== 'object' || item === null);
        if (allPrimitive && val.length <= 16) {
          return `<div class="flex-wrap-gap">${val.map(item =>
            `<span class="badge mono">${escapeHtml(item)}</span>`
          ).join('')}</div>`;
        }
        return `<pre class="json-pre">${escapeHtml(JSON.stringify(val, null, 2))}</pre>`;
      }
      if (typeof val === 'object') {
        return `<pre class="json-pre">${escapeHtml(JSON.stringify(val, null, 2))}</pre>`;
      }
      return `<span class="mono">${escapeHtml(String(val))}</span>`;
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
        <div class="card">
          <div class="header-top">
            <div>
              <div class="title-row">
                <span class="badge-primary">Matter Data Model</span>
                <h1 style="font-size:20px;font-weight:700">${escapeHtml(vendorName)} — ${escapeHtml(productName)}</h1>
              </div>
              <p class="subtitle">
                Interactive runtime topology &amp; attribute dump across ${totalEndpoints} endpoints, ${totalClusters} cluster instances, and ${totalAttrs} attributes
              </p>
            </div>
            <div class="flex-wrap-gap">
              <button onclick="expandOrCollapseAll(false)" class="btn">Expand Clusters</button>
              <button onclick="expandOrCollapseAll(true)" class="btn">Collapse Clusters</button>
              <label class="btn" style="user-select:none">
                <input type="checkbox" ${showGlobalAttributes ? 'checked' : ''} onchange="showGlobalAttributes = this.checked; render();">
                <span>Show Global Attributes</span>
              </label>
            </div>
          </div>

          <div class="info-grid">
            <div class="info-box">
              <div class="info-label">Vendor ID</div>
              <div class="info-val">${escapeHtml(vid)}</div>
            </div>
            <div class="info-box">
              <div class="info-label">Product ID</div>
              <div class="info-val">${escapeHtml(pid)}</div>
            </div>
            <div class="info-box">
              <div class="info-label">Software Version</div>
              <div class="info-val">${escapeHtml(swVer)}</div>
            </div>
            <div class="info-box">
              <div class="info-label">Specification Version</div>
              <div class="info-val">${escapeHtml(specVer)}</div>
            </div>
            <div class="info-box">
              <div class="info-label">Data Model Revision</div>
              <div class="info-val">${escapeHtml(dmRev)}</div>
            </div>
          </div>

          <input
            id="search-input"
            type="text"
            value="${escapeHtml(searchQuery)}"
            oninput="searchQuery = this.value; renderSidebarAndMain();"
            placeholder="Filter endpoints, device labels (e.g., Closure, Oven, Chime), clusters, attributes, or commands..."
            class="search-input"
          />
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
        <div style="display:flex;flex-direction:column;gap:5px">
          <div
            onclick="selectEndpoint(${ep.id})"
            style="margin-left:${indentPx}px;width:calc(100% - ${indentPx}px)"
            class="nav-item ${isSelected ? 'selected' : ''}"
          >
            <div class="nav-row">
              <div class="nav-left">
                ${hasChildren ? `
                  <button
                    onclick="toggleTreeNode(event, ${ep.id})"
                    class="tree-toggle"
                    title="${isCollapsed ? 'Expand child endpoints' : 'Collapse child endpoints'}"
                  >${isCollapsed ? '▶' : '▼'}</button>
                ` : `<span class="mono muted" style="width:14px;text-align:center;font-size:11px">•</span>`}
                <span class="mono" style="font-weight:700;font-size:12px;flex-shrink:0">EP ${ep.id}</span>
                <span style="font-size:12px;font-weight:600;flex-shrink:0;margin-left:2px">${escapeHtml(dtNames)}</span>
                ${ep.label ? `
                  <span class="${isSelected ? 'pill-count' : 'badge-accent'} truncate" style="font-size:11px;padding:1px 7px;margin-left:6px;min-width:0;display:inline-block" title="${escapeHtml(ep.label)}">
                    "${escapeHtml(ep.label)}"
                  </span>
                ` : ''}
                ${ep.semantic_tags.length > 0 ? `
                  <span class="${isSelected ? 'pill-count' : 'badge mono'} truncate" style="font-size:10px;padding:1px 6px;margin-left:6px;min-width:0;display:inline-block" title="${escapeHtml(ep.semantic_tags.map(t => t.display).join(' • '))}">
                    🏷 ${escapeHtml(ep.semantic_tags.map(t => t.tag_name).join(', '))}
                  </span>
                ` : ''}
              </div>
              <span class="pill-count">${ep.clusters.length}c</span>
            </div>
          </div>

          ${(hasChildren && !isCollapsed) ? `
            <div style="display:flex;flex-direction:column;gap:5px">
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
        <div class="sidebar">
          <div class="sidebar-header">
            <span>Endpoint Hierarchy (${DUMP_DATA.endpoints.length})</span>
            <div class="flex-wrap-gap">
              <button onclick="expandOrCollapseTree(false)" class="btn" style="padding:2px 8px" title="Expand all tree nodes">+</button>
              <button onclick="expandOrCollapseTree(true)" class="btn" style="padding:2px 8px" title="Collapse bridged nodes">−</button>
            </div>
          </div>

          <button onclick="selectEndpoint('tree')" class="nav-item ${treeSelected ? 'selected' : ''}">
            <div class="nav-row">
              <span style="font-weight:600;font-size:13px">Topology Tree Overview</span>
              <span class="pill-count">Tree</span>
            </div>
            <div class="muted" style="font-size:11px;margin-top:2px">
              Interactive hierarchy of all ${DUMP_DATA.endpoints.length} endpoints
            </div>
          </button>

          <button onclick="selectEndpoint('all')" class="nav-item ${allSelected ? 'selected' : ''}">
            <div class="nav-row">
              <span style="font-weight:600;font-size:12px">All Endpoints (Full Cluster List)</span>
              <span class="pill-count">${DUMP_DATA.endpoints.length} EPs</span>
            </div>
          </button>

          <div class="tree-scroll">
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
        <div class="${depth > 0 ? 'overview-branch' : ''}" style="display:flex;flex-direction:column;gap:10px">
          <div onclick="selectEndpoint(${ep.id})" class="overview-node">
            <div class="nav-row" style="flex-wrap:wrap">
              <div class="flex-wrap-gap">
                <span class="badge mono" style="font-weight:700">EP ${ep.id}</span>
                ${ep.device_types.map(dt => `
                  <span style="font-weight:700;font-size:14px">
                    ${escapeHtml(dt.name)} <span class="mono muted" style="font-weight:400;font-size:12px">(${dt.hex}, Rev ${dt.revision})</span>
                  </span>
                `).join('<span class="muted">+</span>')}
                ${ep.label ? `
                  <span class="badge-accent">NodeLabel: "${escapeHtml(ep.label)}"</span>
                ` : ''}
                ${ep.semantic_tags.map(t => `
                  <span class="badge mono">🏷 ${escapeHtml(t.display)}</span>
                `).join('')}
              </div>

              <span class="mono muted" style="font-size:12px">
                ${ep.clusters.length} clusters ${ep.children.length > 0 ? `• ${ep.children.length} direct children` : ''} →
              </span>
            </div>

            ${nonDescriptorClusters.length > 0 ? `
              <div class="flex-wrap-gap" style="margin-top:10px">
                ${nonDescriptorClusters.map(cl => `
                  <span
                    onclick="event.stopPropagation(); jumpToCluster(${ep.id}, ${cl.id});"
                    class="badge"
                    style="cursor:pointer"
                  >
                    <span>${escapeHtml(cl.name)}</span>
                    <span class="mono muted">${cl.hex}</span>
                  </span>
                `).join('')}
              </div>
            ` : ''}
          </div>

          ${ep.children.length > 0 ? `
            <div style="display:flex;flex-direction:column;gap:10px">
              ${ep.children.map(childId => renderOverviewTreeNode(childId, depth + 1)).join('')}
            </div>
          ` : ''}
        </div>
      `;
    }

    function renderTopologyTreeOverview() {
      const rootEndpoints = DUMP_DATA.endpoints.filter(ep => ep.parent_id === null);
      return `
        <div class="card">
          <div style="padding-bottom:12px;border-bottom:1px solid var(--border);margin-bottom:14px">
            <h2 style="font-size:16px;font-weight:700">Device Endpoint Composition Tree</h2>
            <p class="subtitle">
              Reconstructed parent-child hierarchy from Descriptor <code>PartsList</code>, <code>BridgedDeviceBasicInformation::NodeLabel</code>, and <code>TagList</code> semantic tags. Click any endpoint or cluster pill to inspect.
            </p>
          </div>
          <div style="display:flex;flex-direction:column;gap:12px">
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
        <div style="margin-bottom:20px">
          <div class="card">
            ${ep.breadcrumbs.length > 1 ? `
              <div class="flex-wrap-gap" style="padding-bottom:12px;margin-bottom:12px;border-bottom:1px solid var(--border);font-size:12px">
                <span class="muted" style="font-weight:600">Hierarchy Path:</span>
                ${ep.breadcrumbs.map((b, idx) => {
                  const isLast = idx === ep.breadcrumbs.length - 1;
                  return `
                    <button
                      onclick="selectEndpoint(${b.id})"
                      class="btn mono ${isLast ? 'btn-active' : ''}"
                      style="padding:3px 8px"
                    >
                      EP ${b.id}: ${escapeHtml(b.summary)}
                    </button>
                    ${isLast ? '' : `<span class="muted">→</span>`}
                  `;
                }).join('')}
              </div>
            ` : ''}

            <div class="flex-wrap-gap">
              <span class="badge mono" style="font-size:14px;font-weight:700;padding:4px 12px">Endpoint ${ep.id}</span>
              ${ep.device_types.map(dt => `
                <span class="badge" style="font-weight:600">
                  ${escapeHtml(dt.name)} <span class="mono muted">${dt.hex} (Rev ${dt.revision})</span>
                </span>
              `).join('')}
              ${ep.label ? `
                <span class="badge-accent">NodeLabel: "${escapeHtml(ep.label)}"</span>
              ` : ''}
              ${ep.semantic_tags.map(t => `
                <span class="badge mono" style="font-weight:600">🏷 ${escapeHtml(t.display)}</span>
              `).join('')}
            </div>

            ${ep.children.length > 0 ? `
              <div class="section-divider">
                <div class="muted" style="font-size:12px;font-weight:600;margin-bottom:8px">
                  Direct Child Endpoints (${ep.children.length}) ${ep.parts_list.length !== ep.children.length ? `<span style="font-weight:400">(Descriptor PartsList has ${ep.parts_list.length} total descendants)</span>` : ''}:
                </div>
                <div class="flex-wrap-gap">
                  ${ep.children.map(childId => {
                    const childEp = EP_BY_ID[childId];
                    const childDt = childEp ? childEp.device_types.map(d => d.name).join(', ') : '';
                    const childLbl = childEp?.label ? ` "${childEp.label}"` : '';
                    const childTags = childEp?.semantic_tags?.length ? ` [${childEp.semantic_tags.map(t => t.tag_name).join(', ')}]` : '';
                    return `
                      <button onclick="selectEndpoint(${childId})" class="btn">
                        <span class="mono" style="font-weight:700">EP ${childId}</span>
                        <span>${escapeHtml(childDt + childLbl + childTags)} →</span>
                      </button>
                    `;
                  }).join('')}
                </div>
              </div>
            ` : ''}

            <div class="section-divider">
              <div class="muted" style="font-size:12px;font-weight:600;margin-bottom:8px">
                Server Clusters (${ep.server_list.length}) — click to jump:
              </div>
              <div class="flex-wrap-gap">
                ${ep.server_list.map(sc => `
                  <button onclick="jumpToCluster(${ep.id}, ${sc.id})" class="btn">
                    <span>${escapeHtml(sc.name)}</span>
                    <span class="mono muted">${sc.hex}</span>
                  </button>
                `).join('')}
              </div>
              ${ep.client_list.length > 0 ? `
                <div class="muted" style="font-size:12px;font-weight:600;margin:10px 0 6px 0">
                  Client Clusters (${ep.client_list.length}):
                </div>
                <div class="flex-wrap-gap">
                  ${ep.client_list.map(cc => `
                    <span class="badge">
                      ${escapeHtml(cc.name)} <span class="mono muted">${cc.hex}</span>
                    </span>
                  `).join('')}
                </div>
              ` : ''}
            </div>
          </div>

          <div>
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
        <div id="cluster-${key}" class="cluster-card">
          <button onclick="toggleCluster('${key}')" class="cluster-header">
            <div class="flex-wrap-gap">
              <span class="mono muted" style="font-size:12px">${isCollapsed ? '▶' : '▼'}</span>
              <span style="font-weight:700;font-size:15px">${escapeHtml(cl.name)}</span>
              <span class="badge mono muted">${cl.hex} (${cl.id})</span>
              ${cl.revision !== null ? `<span class="badge">Rev ${cl.revision}</span>` : ''}
              ${cl.features.map(f => `
                <span class="badge-accent mono">Feature: ${escapeHtml(f.code)} (${escapeHtml(f.name)})</span>
              `).join('')}
            </div>

            <div class="mono muted" style="font-size:12px">
              ${visibleAttrs.length} attrs • ${cl.accepted_commands.length} in / ${cl.generated_commands.length} out cmds
            </div>
          </button>

          ${isCollapsed ? '' : `
            <div class="cluster-body">
              ${(cl.accepted_commands.length > 0 || cl.generated_commands.length > 0) ? `
                <div class="cmd-grid">
                  <div>
                    <div class="muted" style="font-size:12px;font-weight:600;margin-bottom:6px">
                      Accepted Commands (Client → Server) [${cl.accepted_commands.length}]
                    </div>
                    ${cl.accepted_commands.length === 0 ? `<span class="muted" style="font-size:12px;font-style:italic">None</span>` : `
                      <div class="flex-wrap-gap">
                        ${cl.accepted_commands.map(cmd => `
                          <span class="badge">
                            <span class="mono muted">${cmd.hex}</span> ${escapeHtml(cmd.name)}
                          </span>
                        `).join('')}
                      </div>
                    `}
                  </div>
                  <div>
                    <div class="muted" style="font-size:12px;font-weight:600;margin-bottom:6px">
                      Generated Commands (Server → Client) [${cl.generated_commands.length}]
                    </div>
                    ${cl.generated_commands.length === 0 ? `<span class="muted" style="font-size:12px;font-style:italic">None</span>` : `
                      <div class="flex-wrap-gap">
                        ${cl.generated_commands.map(cmd => `
                          <span class="badge">
                            <span class="mono muted">${cmd.hex}</span> ${escapeHtml(cmd.name)}
                          </span>
                        `).join('')}
                      </div>
                    `}
                  </div>
                </div>
              ` : ''}

              ${visibleAttrs.length === 0 ? `
                <div class="muted" style="font-size:12px;font-style:italic">
                  No cluster-specific attributes (enable "Show Global Attributes" above to view global attributes).
                </div>
              ` : `
                <div style="overflow-x:auto">
                  <table class="attr-table">
                    <thead>
                      <tr>
                        <th style="width:110px">ID</th>
                        <th style="width:220px">Attribute Name</th>
                        <th style="width:150px">Type</th>
                        <th>Value</th>
                      </tr>
                    </thead>
                    <tbody>
                      ${visibleAttrs.map(attr => `
                        <tr>
                          <td class="mono muted" style="white-space:nowrap">
                            ${attr.hex} <span style="opacity:0.75">(${attr.id})</span>
                          </td>
                          <td style="font-weight:500">
                            ${escapeHtml(attr.name)}
                            ${attr.is_global ? `<span class="badge muted" style="margin-left:6px;font-size:10px">Global</span>` : ''}
                          </td>
                          <td class="mono muted" style="white-space:nowrap">
                            ${escapeHtml(attr.type)}
                          </td>
                          <td>
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
        <div class="card muted" style="text-align:center;padding:32px;font-size:14px">
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
        <div class="main-grid">
          <div id="sidebar-content">
            ${renderSidebar()}
          </div>
          <div id="main-content">
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


@dataclasses.dataclass
class StructFieldDef:
    field_id: int
    name: str
    field_type: str
    entry_type: str = ""


@dataclasses.dataclass
class AttributeDef:
    attr_id: int
    name: str
    attr_type: str
    entry_type: str = ""


@dataclasses.dataclass
class FeatureDef:
    bit: int
    code: str
    name: str


@dataclasses.dataclass
class ClusterDef:
    cluster_id: int
    name: str
    base_cluster_name: str = ""
    features: dict[int, FeatureDef] = dataclasses.field(default_factory=dict)
    attributes: dict[int, AttributeDef] = dataclasses.field(default_factory=dict)
    accepted_commands: dict[int, str] = dataclasses.field(default_factory=dict)
    generated_commands: dict[int, str] = dataclasses.field(default_factory=dict)
    structs: dict[str, dict[int, StructFieldDef]] = dataclasses.field(default_factory=dict)
    enums: dict[str, dict[int, str]] = dataclasses.field(default_factory=dict)


@dataclasses.dataclass
class NamespaceDef:
    ns_id: int
    name: str
    tags: dict[int, str] = dataclasses.field(default_factory=dict)


@dataclasses.dataclass
class DataModelMetadata:
    clusters: dict[int, ClusterDef] = dataclasses.field(default_factory=dict)
    device_types: dict[int, str] = dataclasses.field(default_factory=dict)
    namespaces: dict[int, NamespaceDef] = dataclasses.field(default_factory=dict)
    global_structs: dict[str, dict[int, StructFieldDef]] = dataclasses.field(default_factory=dict)
    global_enums: dict[str, dict[int, str]] = dataclasses.field(default_factory=dict)


def parse_int(val: str | None) -> int | None:
    if val is None:
        return None
    try:
        return int(val, 0)
    except ValueError:
        return None


def find_repo_root() -> pathlib.Path | None:
    """Locate the connectedhomeip repository root directory."""
    script_path = pathlib.Path(__file__).resolve()
    for parent in script_path.parents:
        if (parent / "data_model").is_dir() and (parent / "src").is_dir():
            return parent
    cwd = pathlib.Path.cwd()
    for candidate in [cwd, *cwd.parents]:
        if (candidate / "data_model").is_dir() and (candidate / "src").is_dir():
            return candidate
    return None


def find_data_model_dir(spec_version: int | None) -> Traversable | None:
    """Locate the appropriate data_model/<version> directory in the repository or installed package."""
    candidates: list[str] = []
    if isinstance(spec_version, int) and spec_version > 0:
        major = (spec_version >> 24) & 0xFF
        minor = (spec_version >> 16) & 0xFF
        dot = (spec_version >> 8) & 0xFF
        candidates.extend([f"{major}.{minor}.{dot}", f"{major}.{minor}"])
    candidates.extend(["1.7", "1.6.1", "1.6", "1.5.1", "1.5", "1.4.2", "1.4", "1.3", "1.2"])

    repo_root = find_repo_root()
    if repo_root is not None:
        dm_root = repo_root / "data_model"
        for cand in candidates:
            if (dm_root / cand / "clusters").is_dir():
                return dm_root / cand

    pkg_dm_root = pathlib.Path(__file__).resolve().parent / "data_model"
    if pkg_dm_root.is_dir():
        for cand in candidates:
            zip_file = pkg_dm_root / cand / "allfiles.zip"
            if zip_file.is_file():
                return zipfile.Path(zip_file)

    return None


def _iter_xml_files(directory: Traversable) -> list[Traversable]:
    if not directory.is_dir():
        return []
    return sorted((f for f in directory.iterdir() if f.name.endswith(".xml")), key=lambda p: p.name)


def _parse_xml_root(xml_file: Traversable) -> ET.Element | None:
    """Parse an XML file and return its root element, logging a warning on malformed XML."""
    try:
        with xml_file.open("r", encoding="utf-8") as f:
            return ET.parse(f).getroot()
    except ET.ParseError as exc:
        LOGGER.warning("Failed to parse data model XML %s: %s", xml_file, exc)
        return None


def _parse_structs_from_element(parent: ET.Element) -> dict[str, dict[int, StructFieldDef]]:
    structs: dict[str, dict[int, StructFieldDef]] = {}
    for struct_el in parent.findall("struct"):
        sname = struct_el.attrib.get("name", "")
        if not sname:
            continue
        fields: dict[int, StructFieldDef] = {}
        for field_el in struct_el.findall("field"):
            fid = parse_int(field_el.attrib.get("id"))
            fname = field_el.attrib.get("name", "")
            ftype = field_el.attrib.get("type", "")
            entry_el = field_el.find("entry")
            etype = entry_el.attrib.get("type", "") if entry_el is not None else ""
            if fid is not None and fname:
                fields[fid] = StructFieldDef(field_id=fid, name=fname, field_type=ftype, entry_type=etype)
        structs[sname] = fields
    return structs


def _parse_enums_from_element(parent: ET.Element) -> dict[str, dict[int, str]]:
    enums: dict[str, dict[int, str]] = {}
    for enum_el in parent.findall("enum"):
        ename = enum_el.attrib.get("name", "")
        if not ename:
            continue
        items: dict[int, str] = {}
        for item_el in enum_el.findall("item"):
            ival = parse_int(item_el.attrib.get("value"))
            iname = item_el.attrib.get("name", "")
            if ival is not None and iname:
                items[ival] = iname
        enums[ename] = items
    return enums


def _load_global_types(dm_dir: Traversable, meta: DataModelMetadata) -> None:
    """Load global struct and enum definitions from globals/Structs.xml and globals/Enums.xml."""
    globals_dir = dm_dir.joinpath("globals")
    structs_xml = globals_dir.joinpath("Structs.xml")
    if structs_xml.is_file():
        root = _parse_xml_root(structs_xml)
        if root is not None:
            meta.global_structs.update(_parse_structs_from_element(root))

    enums_xml = globals_dir.joinpath("Enums.xml")
    if enums_xml.is_file():
        root = _parse_xml_root(enums_xml)
        if root is not None:
            meta.global_enums.update(_parse_enums_from_element(root))


def _load_device_types(dm_dir: Traversable, meta: DataModelMetadata) -> None:
    """Load device type IDs and names from device_types/*.xml."""
    dt_dir = dm_dir.joinpath("device_types")
    for xml_path in _iter_xml_files(dt_dir):
        root = _parse_xml_root(xml_path)
        if root is None:
            continue
        dt_id = parse_int(root.attrib.get("id"))
        dt_name = root.attrib.get("name", "")
        if dt_id is not None and dt_name:
            meta.device_types[dt_id] = dt_name


def _load_namespaces(dm_dir: Traversable, meta: DataModelMetadata) -> None:
    """Load semantic tag namespaces and tag names from namespaces/*.xml."""
    ns_dir = dm_dir.joinpath("namespaces")
    for xml_path in _iter_xml_files(ns_dir):
        root = _parse_xml_root(xml_path)
        if root is None:
            continue
        ns_id = parse_int(root.attrib.get("id"))
        ns_name = root.attrib.get("name", "")
        if ns_id is None or not ns_name:
            continue
        tags: dict[int, str] = {}
        for tag_el in root.findall("./tags/tag"):
            tid = parse_int(tag_el.attrib.get("id"))
            tname = tag_el.attrib.get("name", "")
            if tid is not None and tname:
                tags[tid] = tname
        meta.namespaces[ns_id] = NamespaceDef(ns_id=ns_id, name=ns_name, tags=tags)


def _parse_cluster_xml(root: ET.Element) -> tuple[ClusterDef, list[tuple[int, str]]]:
    """Parse a single cluster XML element into a template ClusterDef and list of (cluster_id, name) pairs."""
    raw_cluster_name = root.attrib.get("name", "")
    clean_base_name = raw_cluster_name[:-8].strip() if raw_cluster_name.endswith(" Cluster") else raw_cluster_name
    cls_el = root.find("classification")
    base_cluster_name = ""
    if cls_el is not None and cls_el.attrib.get("hierarchy") == "derived":
        base_cluster_name = cls_el.attrib.get("baseCluster", "")

    features: dict[int, FeatureDef] = {}
    for feat_el in root.findall("./features/feature"):
        bit = parse_int(feat_el.attrib.get("bit"))
        code = feat_el.attrib.get("code", "")
        fname = feat_el.attrib.get("name", "")
        if bit is not None and code:
            mask = 1 << bit
            features[mask] = FeatureDef(bit=bit, code=code, name=fname or code)

    dtypes_el = root.find("dataTypes")
    structs = _parse_structs_from_element(dtypes_el) if dtypes_el is not None else {}
    enums = _parse_enums_from_element(dtypes_el) if dtypes_el is not None else {}

    attributes: dict[int, AttributeDef] = {}
    for attr_el in root.findall("./attributes/attribute"):
        aid = parse_int(attr_el.attrib.get("id"))
        aname = attr_el.attrib.get("name", "")
        atype = attr_el.attrib.get("type", "")
        entry_el = attr_el.find("entry")
        etype = entry_el.attrib.get("type", "") if entry_el is not None else ""
        if aid is not None and aname:
            attributes[aid] = AttributeDef(attr_id=aid, name=aname, attr_type=atype, entry_type=etype)

    accepted_cmds: dict[int, str] = {}
    generated_cmds: dict[int, str] = {}
    for cmd_el in root.findall("./commands/command"):
        cmd_id = parse_int(cmd_el.attrib.get("id"))
        cmd_name = cmd_el.attrib.get("name", "")
        direction = cmd_el.attrib.get("direction", "")
        if cmd_id is None or not cmd_name:
            continue
        # Note: Derived cluster XMLs may include `<command>` elements without a `direction`
        # attribute solely to override conformance on a command defined in the base cluster.
        # Only classify commands when `direction` is explicitly specified; directionless
        # conformance overrides inherit their direction and name from the base cluster.
        if direction == "responseFromServer":
            generated_cmds[cmd_id] = cmd_name
        elif direction == "commandToServer":
            accepted_cmds[cmd_id] = cmd_name

    template_def = ClusterDef(
        cluster_id=-1,
        name=clean_base_name,
        base_cluster_name=base_cluster_name,
        features=features,
        attributes=attributes,
        accepted_commands=accepted_cmds,
        generated_commands=generated_cmds,
        structs=structs,
        enums=enums,
    )

    cluster_ids: list[tuple[int, str]] = []
    cluster_id_els = root.findall("./clusterIds/clusterId")
    if cluster_id_els:
        for cid_el in cluster_id_els:
            cid = parse_int(cid_el.attrib.get("id"))
            cname = cid_el.attrib.get("name") or clean_base_name
            if cid is not None:
                cluster_ids.append((cid, cname))
    else:
        cid = parse_int(root.attrib.get("id"))
        if cid is not None:
            cluster_ids.append((cid, clean_base_name))

    return template_def, cluster_ids


def _load_clusters(dm_dir: Traversable, meta: DataModelMetadata) -> dict[str, ClusterDef]:
    """Load all cluster XML files and return base cluster templates keyed by cluster name."""
    clusters_dir = dm_dir.joinpath("clusters")
    base_templates: dict[str, ClusterDef] = {}
    for xml_path in _iter_xml_files(clusters_dir):
        root = _parse_xml_root(xml_path)
        if root is None:
            continue
        template_def, cluster_ids = _parse_cluster_xml(root)
        if template_def.name:
            base_templates[template_def.name] = template_def
        for cid, cname in cluster_ids:
            cdef = copy.deepcopy(template_def)
            cdef.cluster_id = cid
            cdef.name = cname
            meta.clusters[cid] = cdef
            base_templates.setdefault(cname, cdef)
    return base_templates


def _merge_from_base_cluster(cdef: ClusterDef, base_def: ClusterDef) -> None:
    """Merge inherited features, structs, enums, attributes, and commands from a base ClusterDef."""
    for mask, fdef in base_def.features.items():
        cdef.features.setdefault(mask, fdef)
    for sname, sfields in base_def.structs.items():
        if sname not in cdef.structs:
            cdef.structs[sname] = copy.deepcopy(sfields)
        else:
            for fid, field_def in sfields.items():
                cdef.structs[sname].setdefault(fid, copy.deepcopy(field_def))
                if not cdef.structs[sname][fid].field_type:
                    cdef.structs[sname][fid].field_type = field_def.field_type
                if not cdef.structs[sname][fid].entry_type:
                    cdef.structs[sname][fid].entry_type = field_def.entry_type
    for ename, eitems in base_def.enums.items():
        if ename not in cdef.enums:
            cdef.enums[ename] = dict(eitems)
        else:
            for k, v in eitems.items():
                cdef.enums[ename].setdefault(k, v)
    for aid, base_attr in base_def.attributes.items():
        if aid not in cdef.attributes:
            cdef.attributes[aid] = copy.deepcopy(base_attr)
        else:
            cur_attr = cdef.attributes[aid]
            if not cur_attr.attr_type:
                cur_attr.attr_type = base_attr.attr_type
            if not cur_attr.entry_type:
                cur_attr.entry_type = base_attr.entry_type
    for cmd_id, cmd_name in base_def.accepted_commands.items():
        cdef.accepted_commands.setdefault(cmd_id, cmd_name)
    for cmd_id, cmd_name in base_def.generated_commands.items():
        cdef.generated_commands.setdefault(cmd_id, cmd_name)


def _resolve_derived_clusters(meta: DataModelMetadata, base_templates: dict[str, ClusterDef]) -> None:
    """Resolve derived cluster inheritance from baseCluster templates (supporting multi-level chains)."""
    for cdef in meta.clusters.values():
        seen: set[str] = set()
        curr_base_name = cdef.base_cluster_name
        while curr_base_name and curr_base_name not in seen:
            seen.add(curr_base_name)
            base_def = base_templates.get(curr_base_name)
            if base_def is None:
                break
            _merge_from_base_cluster(cdef, base_def)
            curr_base_name = base_def.base_cluster_name


def load_data_model_metadata(dm_dir: Traversable | None) -> DataModelMetadata:
    """Load cluster, device type, namespace, struct, and enum definitions from canonical data_model XML files."""
    meta = DataModelMetadata()
    if dm_dir is None or not dm_dir.is_dir():
        return meta

    _load_global_types(dm_dir, meta)
    _load_device_types(dm_dir, meta)
    _load_namespaces(dm_dir, meta)
    base_templates = _load_clusters(dm_dir, meta)
    _resolve_derived_clusters(meta, base_templates)
    return meta


def decode_tlv_value(
    raw_type: str,
    raw_val: Any,
    type_name: str,
    entry_type: str,
    cluster_def: ClusterDef | None,
    meta: DataModelMetadata,
) -> Any:
    """Decode a MatterTlvJson value using XML struct and enum definitions when available."""
    if raw_type == "NULL" or raw_val is None:
        return None

    if raw_type == "BYTES" and isinstance(raw_val, str):
        try:
            decoded_bytes = base64.b64decode(raw_val)
            return "hex:" + decoded_bytes.hex()
        except Exception:
            return raw_val

    if raw_type.startswith("ARRAY") and isinstance(raw_val, list):
        sub_raw_type = raw_type.split("-", 1)[1] if "-" in raw_type else ""
        item_type_name = entry_type or type_name
        return [
            decode_tlv_value(sub_raw_type, item, item_type_name, "", cluster_def, meta)
            for item in raw_val
        ]

    if (raw_type == "STRUCT" or isinstance(raw_val, dict)) and isinstance(raw_val, dict):
        struct_fields: dict[int, StructFieldDef] | None = None
        if cluster_def and type_name in cluster_def.structs:
            struct_fields = cluster_def.structs[type_name]
        elif type_name in meta.global_structs:
            struct_fields = meta.global_structs[type_name]

        out_dict: dict[str, Any] = {}
        for k_str, v_item in raw_val.items():
            parts = k_str.split(":", 1)
            field_tag = parse_int(parts[0])
            sub_raw_type = parts[1] if len(parts) > 1 else ""
            if field_tag is not None and struct_fields and field_tag in struct_fields:
                fdef = struct_fields[field_tag]
                out_dict[fdef.name] = decode_tlv_value(
                    sub_raw_type, v_item, fdef.field_type, fdef.entry_type, cluster_def, meta
                )
            else:
                key_label = str(field_tag) if field_tag is not None else parts[0]
                out_dict[key_label] = decode_tlv_value(sub_raw_type, v_item, "", "", cluster_def, meta)
        return out_dict

    if isinstance(raw_val, int) and not isinstance(raw_val, bool) and type_name:
        enum_map: dict[int, str] | None = None
        if cluster_def and type_name in cluster_def.enums:
            enum_map = cluster_def.enums[type_name]
        elif type_name in meta.global_enums:
            enum_map = meta.global_enums[type_name]
        if enum_map and raw_val in enum_map:
            return f"{enum_map[raw_val]} ({raw_val})"

    return raw_val


def format_spec_version(spec_ver: int | None) -> str:
    if not isinstance(spec_ver, int) or spec_ver <= 0:
        return "Unknown"
    major = (spec_ver >> 24) & 0xFF
    minor = (spec_ver >> 16) & 0xFF
    dot = (spec_ver >> 8) & 0xFF
    return f"{major}.{minor}.{dot} (0x{spec_ver:08X})"


def _extract_spec_version(json_data: dict[Any, Any]) -> int | None:
    """Extract SpecificationVersion (attribute 21) from Endpoint 0 BasicInformation (cluster 40)."""
    ep0 = json_data.get("0") or json_data.get(0) or {}
    if isinstance(ep0, dict):
        basic_info_raw = ep0.get("40:STRUCT", {})
        if isinstance(basic_info_raw, dict):
            for k, v in basic_info_raw.items():
                if str(k).startswith("21:") and isinstance(v, int):
                    return v
    return None


def _reconstruct_endpoint_tree(
    endpoints_by_id: dict[int, dict[str, Any]],
    endpoints_out: list[dict[str, Any]],
) -> None:
    """Reconstruct the parent-child Endpoint Composition Tree and breadcrumb paths.

    Matter endpoints declare their descendants in Descriptor::PartsList using either the
    Full-Family Pattern (a parent lists all direct and indirect descendants) or the
    Tree Pattern (a parent lists only direct children), and Endpoint 0 lists all non-zero
    endpoints on the node. We compute the transitive reduction of the reachability graph
    so each endpoint is attached to its most immediate parent.
    """
    all_ep_ids = sorted(endpoints_by_id.keys())
    direct_edges: dict[int, set[int]] = {
        ep_id: set(ep["parts_list"]) & set(all_ep_ids) for ep_id, ep in endpoints_by_id.items()
    }
    if 0 in direct_edges:
        for ep_id in all_ep_ids:
            if ep_id != 0:
                direct_edges[0].add(ep_id)

    # Compute transitive reachability from each endpoint via DFS over PartsList edges.
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

    # Select the most immediate ancestor (leaf ancestor with no intermediate descendant) as parent_id.
    for ep_id in all_ep_ids:
        if ep_id == 0:
            continue
        ancestors = [u for u in all_ep_ids if ep_id in reachable[u]]
        if not ancestors:
            continue
        leaf_ancestors = [
            u for u in ancestors if not any(w != u and w in reachable[u] for w in ancestors)
        ]
        parent_id = min(leaf_ancestors, key=lambda u: (len(endpoints_by_id[u]["parts_list"]), -u))
        endpoints_by_id[ep_id]["parent_id"] = parent_id
        endpoints_by_id[parent_id]["children"].append(ep_id)

    for ep in endpoints_out:
        ep["children"].sort()

    # Build root-to-endpoint breadcrumb chains for UI navigation.
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


def parse_dump(json_data: dict[Any, Any]) -> dict[str, Any]:
    """Parse MatterTlvJson into enriched JSON-serializable dicts for injection into HTML_TEMPLATE."""
    # Step 1: Detect the device's Matter SpecificationVersion from Endpoint 0 BasicInformation
    # and load the matching data_model/<version> XML definitions.
    spec_version_raw = _extract_spec_version(json_data)
    dm_dir = find_data_model_dir(spec_version_raw)
    meta = load_data_model_metadata(dm_dir)

    def get_cluster_name(cid: int) -> str:
        if cid in meta.clusters:
            return meta.clusters[cid].name
        return f"Cluster 0x{cid:04X}"

    def get_device_type_name(dt_id: int) -> str:
        if dt_id in meta.device_types:
            return meta.device_types[dt_id]
        return f"DeviceType 0x{dt_id:04X}"

    def resolve_semantic_tag(tag_entry: dict[str, Any]) -> dict[str, Any]:
        mfg_code = tag_entry.get("0:UINT")
        ns_id = tag_entry.get("1:UINT", 0)
        tag_id = tag_entry.get("2:UINT", 0)
        label_str = tag_entry.get("3:STRING")
        ns_name = f"Namespace 0x{ns_id:02X}"
        tag_name = f"Tag 0x{tag_id:02X}"
        if isinstance(mfg_code, int):
            ns_name = f"Mfg 0x{mfg_code:04X} Namespace 0x{ns_id:02X}"
        elif ns_id in meta.namespaces:
            ns_name = meta.namespaces[ns_id].name
            if tag_id in meta.namespaces[ns_id].tags:
                tag_name = meta.namespaces[ns_id].tags[tag_id]
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

    # Note: All output structures (`endpoints_out`, `clusters_out`, `attributes_out`, `ep_obj`)
    # are constructed as plain JSON-serializable dicts so they can be serialized directly
    # via `json.dumps()` into `HTML_TEMPLATE` (`const DUMP_DATA = ...`).
    endpoints_out: list[dict[str, Any]] = []
    endpoints_by_id: dict[int, dict[str, Any]] = {}

    # Step 2: Iterate through all endpoints in numeric order and decode each cluster's attributes.
    for ep_key in sorted(
        json_data.keys(),
        key=lambda x: x if isinstance(x, int) else (parse_int(str(x)) or 0),
    ):
        ep_id = ep_key if isinstance(ep_key, int) else parse_int(str(ep_key))
        if ep_id is None:
            continue
        ep_clusters_raw = json_data[ep_key]
        if not isinstance(ep_clusters_raw, dict):
            continue

        clusters_out: list[dict[str, Any]] = []
        ep_device_types: list[dict[str, Any]] = []
        ep_server_list: list[dict[str, Any]] = []
        ep_client_list: list[dict[str, Any]] = []
        ep_parts_list: list[int] = []
        ep_semantic_tags: list[dict[str, Any]] = []
        ep_label = ""

        # Extract NodeLabel (attribute 5) from BridgedDeviceBasicInformation (57) or BasicInformation (40).
        for label_cid in (57, 40):
            c_raw = ep_clusters_raw.get(f"{label_cid}:STRUCT", {})
            if isinstance(c_raw, dict) and c_raw.get("5:STRING"):
                ep_label = str(c_raw["5:STRING"])
                break

        # Sort Descriptor (29) and BasicInformation (40/57) first, followed by remaining clusters by ID.
        def cluster_sort_key(item: tuple[str, Any]) -> tuple[int, int]:
            cid = parse_int(item[0].split(":", 1)[0]) or 0
            if cid == 29:
                return (0, cid)
            if cid in (40, 57):
                return (1, cid)
            return (2, cid)

        for cluster_key, cluster_attrs_raw in sorted(ep_clusters_raw.items(), key=cluster_sort_key):
            cid = parse_int(cluster_key.split(":", 1)[0])
            if cid is None or not isinstance(cluster_attrs_raw, dict):
                continue
            cname = get_cluster_name(cid)
            cluster_def = meta.clusters.get(cid)

            attributes_out: list[dict[str, Any]] = []
            feature_map_val = 0
            cluster_rev_val = None
            accepted_cmds_raw: list[int] = []
            generated_cmds_raw: list[int] = []

            # Sort cluster-specific attributes before global attributes (0xFFF8..0xFFFD).
            def attr_sort_key(item: tuple[str, Any]) -> tuple[int, int]:
                aid = parse_int(item[0].split(":", 1)[0]) or 0
                return (1 if aid in GLOBAL_ATTRIBUTE_IDS else 0, aid)

            # Step 2a: Decode each attribute's name, type, and TLV value, while capturing
            # global metadata attributes (FeatureMap, ClusterRevision, command lists) and
            # Descriptor composition lists (DeviceTypeList, ServerList, ClientList, PartsList, TagList).
            for attr_key, attr_val_raw in sorted(cluster_attrs_raw.items(), key=attr_sort_key):
                parts = attr_key.split(":", 1)
                aid = parse_int(parts[0])
                if aid is None:
                    continue
                raw_type = parts[1] if len(parts) > 1 else ""

                aname = ""
                atype = raw_type
                entry_type = ""
                if aid in GLOBAL_ATTRIBUTE_IDS:
                    aname, atype = GLOBAL_ATTRIBUTE_IDS[aid]
                elif cluster_def and aid in cluster_def.attributes:
                    adef = cluster_def.attributes[aid]
                    aname = adef.name
                    entry_type = adef.entry_type
                    if adef.attr_type == "list" and adef.entry_type:
                        atype = f"list[{adef.entry_type}]"
                    elif adef.attr_type:
                        atype = adef.attr_type
                if not aname:
                    aname = f"Attribute 0x{aid:04X}"

                decoded_val = decode_tlv_value(
                    raw_type,
                    attr_val_raw,
                    cluster_def.attributes[aid].attr_type if (cluster_def and aid in cluster_def.attributes) else "",
                    entry_type,
                    cluster_def,
                    meta,
                )

                if aid == 0xFFFC and isinstance(attr_val_raw, int):
                    feature_map_val = attr_val_raw
                elif aid == 0xFFFD and isinstance(attr_val_raw, int):
                    cluster_rev_val = attr_val_raw
                elif aid == 0xFFF9 and isinstance(attr_val_raw, list):
                    accepted_cmds_raw = [int(x) for x in attr_val_raw if isinstance(x, int)]
                elif aid == 0xFFF8 and isinstance(attr_val_raw, list):
                    generated_cmds_raw = [int(x) for x in attr_val_raw if isinstance(x, int)]

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

            # Step 2b: Resolve active FeatureMap bits and Accepted/Generated command IDs
            # against the cluster's XML definition.
            features_out = []
            if cluster_def and feature_map_val:
                for mask, feat in sorted(cluster_def.features.items()):
                    if feature_map_val & mask:
                        features_out.append({
                            "mask": f"0x{mask:04X}",
                            "code": feat.code,
                            "name": feat.name,
                        })

            accepted_cmds_out = []
            for cmd_id in accepted_cmds_raw:
                cmd_name = f"Command 0x{cmd_id:02X}"
                if cluster_def and cmd_id in cluster_def.accepted_commands:
                    cmd_name = cluster_def.accepted_commands[cmd_id]
                accepted_cmds_out.append({
                    "id": cmd_id,
                    "hex": f"0x{cmd_id:02X}",
                    "name": cmd_name,
                })

            generated_cmds_out = []
            for cmd_id in generated_cmds_raw:
                cmd_name = f"Command 0x{cmd_id:02X}"
                if cluster_def and cmd_id in cluster_def.generated_commands:
                    cmd_name = cluster_def.generated_commands[cmd_id]
                generated_cmds_out.append({
                    "id": cmd_id,
                    "hex": f"0x{cmd_id:02X}",
                    "name": cmd_name,
                })

            # Plain dict representing a single cluster instance for JSON serialization into HTML_TEMPLATE.
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

        # Plain dict representing an endpoint for JSON serialization into HTML_TEMPLATE.
        ep_obj: dict[str, Any] = {
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

    # Step 3: Reconstruct the parent-child Endpoint Composition Tree and breadcrumbs.
    _reconstruct_endpoint_tree(endpoints_by_id, endpoints_out)

    # Step 4: Extract node-level device summary from Endpoint 0 BasicInformation (cluster 40).
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


def _make_js_safe(obj: Any) -> Any:
    if isinstance(obj, bool):
        return obj
    if isinstance(obj, int) and abs(obj) > 0x1FFFFFFFFFFFFF:
        return f"{obj} (0x{obj:X})" if obj >= 0 else f"{obj} (-0x{-obj:X})"
    if isinstance(obj, list):
        return [_make_js_safe(x) for x in obj]
    if isinstance(obj, dict):
        return {k: _make_js_safe(v) for k, v in obj.items()}
    return obj


def generate_html_from_dump_dict(raw_dump: dict[Any, Any], html_path: pathlib.Path) -> None:
    enriched = parse_dump(raw_dump)
    safe_json = (
        json.dumps(_make_js_safe(enriched), separators=(",", ":"))
        .replace("<", "\\u003c")
        .replace(">", "\\u003e")
        .replace("&", "\\u0026")
    )
    html_content = HTML_TEMPLATE.replace("__DUMP_JSON_PLACEHOLDER__", safe_json)
    html_path.parent.mkdir(parents=True, exist_ok=True)
    with open(html_path, "w", encoding="utf-8") as f:
        f.write(html_content)


def generate_html_from_dump(json_path: pathlib.Path, html_path: pathlib.Path) -> None:
    with open(json_path, encoding="utf-8") as f:
        raw_dump = json.load(f)
    generate_html_from_dump_dict(raw_dump, html_path)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Convert a MatterTlvJson (.json) device dump into a self-contained interactive HTML file."
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
